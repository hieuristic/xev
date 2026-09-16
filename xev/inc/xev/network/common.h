#pragma once
#include <cstdint>
#include <functional>

namespace xev::net {

enum struct SendMode : uint8_t {
  Reliable,
  Fast,
};

using ClientMessageCallback = std::function<void(const uint8_t* data, uint64_t size)>;
using ServerMessageCallback =
    std::function<void(uint32_t clientID, const uint8_t* data, uint64_t size)>;

}  // namespace xev::net
