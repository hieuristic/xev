#pragma once
#include <glm/glm.hpp>

namespace xev {

// functional points with functionality defined in application
// layer.
struct FuncPoint {
  FuncPoint(std::string name_, glm::vec3& position_)
      : name(std::move(name_)), position(position_) {}
  std::string name;
  glm::vec3 position;
};

}  // namespace xev
