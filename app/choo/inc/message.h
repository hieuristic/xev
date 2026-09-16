#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "character.h"

enum struct MessageType : uint8_t {
  Transform = 0,
  Disconnect = 1,
};

#pragma pack(push,1)
struct MsgTran {
  MessageType type{MessageType::Transform};
  uint32_t senderId{0};
  CharacterType character{CharacterType::Hieu};
  glm::vec3 pos{0.0f};
  glm::quat rot{1.0f, 0.0f, 0.0f, 0.0f};
};
struct MsgDisc {
  MessageType type{MessageType::Disconnect};
  uint32_t senderId{0};
};
#pragma pack(pop)
