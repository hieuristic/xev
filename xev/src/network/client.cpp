#include <steam/steamnetworkingsockets.h>
#include <xev/logger.h>
#include <xev/network/client.h>

namespace xev::net {

#define XEV_VALID_CONNECTION \
  (m_interface && m_connection != k_HSteamNetConnection_Invalid)

#define XEV_INVALID_CONNECTION \
  !(m_interface && m_connection != k_HSteamNetConnection_Invalid)

bool Client::connect(std::string_view host, uint16_t port) {
  m_interface = SteamNetworkingSockets();
  if (!m_interface) {
    XEV_ERROR("[net:client] failed to obtain interface");
    return false;
  }

  disconnect();

  SteamNetworkingIPAddr hostAddr;
  hostAddr.Clear();
  hostAddr.ParseString(std::string(host).c_str());
  hostAddr.m_port = port;

  m_connection = m_interface->ConnectByIPAddress(hostAddr, 0, nullptr);
  if (m_connection == k_HSteamNetConnection_Invalid) {
    XEV_ERROR("[net:client] failed to connect to {}", host);
    return false;
  }

#ifdef XEVDEBUG
  XEV_INFO("[net:client] connected to {}:{}", host, port);
#endif
  return true;
}

void Client::disconnect() {
  if (XEV_VALID_CONNECTION) {
    m_interface->CloseConnection(m_connection, 0, "client leave", true);
    m_connection = k_HSteamNetConnection_Invalid;
  }
  m_connected = false;
}

void Client::poll() {
  if (XEV_INVALID_CONNECTION) return;

  SteamNetConnectionInfo_t info;
  if (m_interface->GetConnectionInfo(m_connection, &info)) {
    switch (info.m_eState) {
      case k_ESteamNetworkingConnectionState_Connected:
        m_connected = true;
        break;
      case k_ESteamNetworkingConnectionState_ClosedByPeer:
      case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        m_connected = false;
        disconnect();
        return;
      default:
        break;
    }
  }

  m_interface->RunCallbacks();

  ISteamNetworkingMessage* pMsg = nullptr;
  while (m_interface->ReceiveMessagesOnConnection(m_connection, &pMsg, 1) > 0 &&
         pMsg) {
    if (m_onMessage && pMsg->m_cbSize > 0) {
      m_onMessage(reinterpret_cast<const uint8_t*>(pMsg->m_pData),
                  static_cast<uint64_t>(pMsg->m_cbSize));
    }
    pMsg->Release();
  }
}

int Client::get_ping() const {
  if (XEV_INVALID_CONNECTION) return -1;

  SteamNetConnectionRealTimeStatus_t status;
  if (m_interface->GetConnectionRealTimeStatus(m_connection, &status, 0,
                                               nullptr) == k_EResultOK) {
    return status.m_nPing;
  }
  return -1;
}

void Client::send(const void* data, uint32_t size, SendMode mode) {
  if (XEV_INVALID_CONNECTION || !m_connected) return;
  int flags = (mode == SendMode::Reliable)
                  ? k_nSteamNetworkingSend_Reliable
                  : k_nSteamNetworkingSend_UnreliableNoDelay;
  m_interface->SendMessageToConnection(m_connection, data, size, flags,
                                       nullptr);
}

}  // namespace xev::net
