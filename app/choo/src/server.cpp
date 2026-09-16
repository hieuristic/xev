#include <atomic>
#include <chrono>
#include <csignal>
#include <string>
#include <thread>

#include <xev/logger.h>
#include <xev/network/network.h>
#include <xev/network/server.h>

#include "message.h"
#include "server.h"

int Server::run(uint16_t port) {
  xev::Network net;
  xev::net::Server server;

  if (!server.start(port)) return 1;

  server.abonnen(
      [&server](uint32_t senderId, const void* data, uint32_t size) {
        if (size >= sizeof(MsgTran)) {
          MsgTran t = *reinterpret_cast<const MsgTran*>(data);
          t.senderId = senderId;
          server.broadcast(&t, sizeof(t), senderId, xev::net::SendMode::Fast);
        }
      });

  XEV_INFO("[choo:server] running on port {}", port);
  while (server.is_running()) {
    server.poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return 0;
}
