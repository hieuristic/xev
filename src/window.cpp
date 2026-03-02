#include <xev/logger.h>
#include <xev/window.h>

namespace xev {

Window::Window(std::string name) { createWindow(name, 800, 600); }

Window::Window(std::string name, uint32_t width, uint32_t height) {
  createWindow(name, width, height);
}

Window::~Window() {
  if (m_window) {
    SDL_DestroyWindow(m_window);
    XEV_DEBUG("Window destroyed.");
  }
}

void Window::createWindow(std::string name, uint32_t width, uint32_t height) {
  m_width = width;
  m_height = height;
  m_window = SDL_CreateWindow(name.c_str(), width, height, SDL_WINDOW_VULKAN);
  if (!m_window) {
    XEV_ERROR("SDL_CreateWindow failed: {}", SDL_GetError());
  } else {
    XEV_DEBUG("Window '{}' created ({}x{}).", name, width, height);
  }
}

} // namespace xev
