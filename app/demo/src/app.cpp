#include "app.h"

App::App() {
  m_window = std::make_unique<Window>("demo app");
  m_backend = std::make_unique<Backend>(m_window.get_native());
  m_renderer3D = std::make_unique<Renderer3D>(m_window, m_window.get_size());
  m_renderer2D = std::make_unique<Renderer2D>(m_window, m_window.get_size());
}

void App::handle_inputs() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
      if (e.key.key == SDLK_ESCAPE | e.key.key == SDLK_Q) {
        m_running = false;
      }
    }
  }
}

void App::draw() {
  m_backend.enter_frame();

  // m_renderer3D.draw();
  // m_renderer2D.draw();

  m_backend.leave_frame();
}

void App::run() {
  while (m_running) {
    handle_inputs();
    draw();
  }
}
