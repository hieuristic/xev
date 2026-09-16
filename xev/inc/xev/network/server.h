#pragma once
#include <cstdint>
#include <functional>
#include <vector>

#include <xev/network/common.h>

class ISteamNetworkingSockets;

namespace xev::net {

using AboCallback =
    std::function<void(uint32_t senderId, const void* data, uint32_t size)>;

struct Server {
  bool start(uint16_t port = 1906);
  void stop();
  void poll();

  bool is_running() const { return m_isRunning; }
  uint32_t get_num_clients() const { return m_clients.size(); }

  void broadcast(const void* data,
                 uint32_t size,
                 uint32_t except = 0,
                 SendMode mode = SendMode::Fast);
  void send(uint32_t clientId,
            const void* data,
            uint32_t size,
            SendMode mode = SendMode::Fast);
  void abonnen(AboCallback cb);

 private:
  bool m_isRunning{false};
  uint32_t m_socket{0};
  uint32_t m_pollGroup{0};
  ISteamNetworkingSockets* m_interface{nullptr};
  std::vector<uint32_t> m_clients;
  AboCallback m_callback;
};

}  // namespace xev::net
