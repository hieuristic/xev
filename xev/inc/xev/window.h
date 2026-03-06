#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <cstdint>

namespace xev {

class Window {
public:
  Window(std::string name);
  Window(std::string name, uint32_t width, uint32_t height);
  ~Window();

  SDL_Window* getNativeWindow() const { return m_window; }

private:
  void createWindow(std::string name, uint32_t width, uint32_t height);
  SDL_Window* m_window;
  uint32_t m_width;
  uint32_t m_height;
};

} // namespace xev
