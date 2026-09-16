#include <steam/steamnetworkingsockets.h>
#include <xev/logger.h>
#include <xev/network/server.h>
#include <algorithm>

namespace xev::net {

bool Server::start(uint16_t port) {
  m_interface = SteamNetworkingSockets();
  if (!m_interface) {
    XEV_ERROR("[net:server] failed to obtain interface.");
    return false;
  }

  stop();

  SteamNetworkingIPAddr addr;
  addr.Clear();
  addr.m_port = port;

  auto statusCallback = [](SteamNetConnectionStatusChangedCallback_t* pInfo) {
    auto* self = reinterpret_cast<Server*>(pInfo->m_info.m_nUserData);
    if (!self) return;

    switch (pInfo->m_info.m_eState) {
      case k_ESteamNetworkingConnectionState_Connecting: {
        XEV_INFO("[net:server] Connection request from {}",
                 pInfo->m_info.m_szConnectionDescription);
        if (self->m_interface->AcceptConnection(pInfo->m_hConn) !=
            k_EResultOK) {
          self->m_interface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
          XEV_WARN("[xev:server] failed to accept connection");
          break;
        }
        self->m_interface->SetConnectionUserData(
            pInfo->m_hConn, reinterpret_cast<int64_t>(self));
        self->m_interface->SetConnectionPollGroup(pInfo->m_hConn,
                                                  self->m_pollGroup);
        self->m_clients.push_back(pInfo->m_hConn);
        XEV_INFO("[net:server] Client {} connected (total: {})", pInfo->m_hConn,
                 self->m_clients.size());
        break;
      }
      case k_ESteamNetworkingConnectionState_ClosedByPeer:
      case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
        XEV_INFO("[net:server] client {} disconnectd: {}", pInfo->m_hConn,
                 pInfo->m_info.m_szEndDebug);
        auto it = std::find(self->m_clients.begin(), self->m_clients.end(),
                            pInfo->m_hConn);
        if (it != self->m_clients.end()) {
          self->m_clients.erase(it);
        }
        self->m_interface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
        break;
      }
      default:
        break;
    }
  };

  SteamNetworkingConfigValue_t opts[2];
  opts[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                 (void*)+statusCallback);  // This cast lambda to c func pointer
  opts[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData,
                   reinterpret_cast<int64_t>(this));
  m_socket = m_interface->CreateListenSocketIP(addr, 2, opts);
  if (m_socket == k_HSteamListenSocket_Invalid) {
    XEV_ERROR("[net:server] failed to create socket at port {}", port);
    return false;
  }

  m_pollGroup = m_interface->CreatePollGroup();
  if (m_pollGroup == k_HSteamNetPollGroup_Invalid) {
    XEV_ERROR("[net:server] failed to create poll group.");
    m_interface->CloseListenSocket(m_socket);
    m_socket = k_HSteamListenSocket_Invalid;
    return false;
  }

  m_isRunning = true;
  XEV_INFO("[net:server] server running on port {}", port);
  return true;
}

void Server::stop() {
  if (!m_isRunning && m_socket == 0) return;
  m_isRunning = false;

  for (auto id : m_clients) {
    m_interface->CloseConnection(id, 0, "closing server", true);
  }
  m_clients.clear();

  if (m_socket != 0) {
    m_interface->CloseListenSocket(m_socket);
    m_socket = 0;
  }
  if (m_pollGroup != 0) {
    m_interface->DestroyPollGroup(m_pollGroup);
    m_pollGroup = 0;
  }
}

void Server::poll() {
  if (!m_isRunning || !m_interface) return;

  m_interface->RunCallbacks();

  while (m_isRunning) {
    ISteamNetworkingMessage* pMsg = nullptr;
    int numMsgs =
        m_interface->ReceiveMessagesOnPollGroup(m_pollGroup, &pMsg, 1);
    if (numMsgs <= 0 || !pMsg) break;
    if (pMsg->m_cbSize > 0 && m_callback) {
      m_callback(pMsg->m_conn, pMsg->m_pData,
                 static_cast<uint32_t>(pMsg->m_cbSize));
    }
    pMsg->Release();
  }
}

void Server::broadcast(const void* data,
                       uint32_t size,
                       uint32_t except,
                       SendMode mode) {
  if (!m_isRunning || !m_interface) return;

  int flags = (mode == SendMode::Reliable)
                  ? k_nSteamNetworkingSend_Reliable
                  : k_nSteamNetworkingSend_UnreliableNoDelay;

  for (auto id : m_clients) {
    if (id != except) {
      m_interface->SendMessageToConnection(id, data, size, flags, nullptr);
    }
  }
}

void Server::send(uint32_t clientId,
                  const void* data,
                  uint32_t size,
                  SendMode mode) {
  if (!m_isRunning || !m_interface) return;

  int flags = (mode == SendMode::Reliable)
                  ? k_nSteamNetworkingSend_Reliable
                  : k_nSteamNetworkingSend_UnreliableNoDelay;

  m_interface->SendMessageToConnection(clientId, data, size, flags, nullptr);
}

void Server::abonnen(AboCallback cb) {
  m_callback = std::move(cb);
}

}  // namespace xev::net
