#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.h>
#include <cstdint>
#include <string>

namespace xev {

class Window {
 public:
  Window(std::string name);
  Window(std::string name, uint32_t width, uint32_t height);
  ~Window();

  SDL_Window* get_native() const { return m_window; }
  glm::ivec2 get_size() const { return glm::ivec2(m_width, m_height); }

 private:
  void create_window(std::string name, uint32_t width, uint32_t height);
  SDL_Window* m_window;
  uint32_t m_width;
  uint32_t m_height;
};

}  // namespace xev
