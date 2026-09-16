#pragma once
#include <cstdint>
#include <functional>

#include <xev/network/common.h>

class ISteamNetworkingSockets;

namespace xev::net {

struct Client {
  bool connect(std::string_view host = "127.0.0.1", uint16_t port = 1906);
  void disconnect();
  void poll();

  bool is_connected() const { return m_connected; }
  int get_ping() const;

  void send(const void* data, uint32_t size, SendMode mode);
  void on_message(ClientMessageCallback cb) { m_onMessage = std::move(cb); }

 private:
  uint32_t m_connection{0};
  ISteamNetworkingSockets* m_interface{nullptr};
  bool m_connected{false};
  ClientMessageCallback m_onMessage;
};

}  // namespace xev::net
