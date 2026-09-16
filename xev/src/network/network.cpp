#include <steam/steamnetworkingsockets.h>

#include <xev/logger.h>
#include <xev/network/network.h>

namespace xev {

Network::Network() {
  SteamDatagramErrMsg errMsg;
  if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
    XEV_ERROR("Failed to start networking! {}", errMsg);
  }
}

Network::~Network() {
  GameNetworkingSockets_Kill();
}

}  // namespace xev
