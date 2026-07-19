#include "game.h"
#include <SDL3/SDL.h>
#include <xev/engine.h>
#include <xev/renderer2D.h>
#include <xev/renderer3D.h>
#include <xev/resource/scene.h>
#include <xev/window.h>

Game::Game() : m_running(true) {
  m_window = std::make_unique<xev::Window>("demogame", 800, 600);

  SDL_SetWindowRelativeMouseMode(m_window->get_native(), true);

  m_engine = std::make_unique<xev::Engine>(m_window->get_native());
  m_engine->init_swapchain();
  m_engine->init_resource_manager();
  m_engine->init_hot_exec();
  m_engine->init_global_descriptor_set();
  m_engine->init_pipeline_manager();
  m_engine->init_frame_context();

  m_renderer2D = std::make_unique<xev::Renderer3D>(
      *m_engine->pipeline_manager,
      m_engine->global_descriptor_set->get_layout());

  m_renderer3D = std::make_unique<xev::Renderer2D>(
      *m_engine->pipeline_manager,
      m_engine->global_descriptor_set->get_layout());
}

Game::~Game() {
  m_renderer.reset();
  m_scene.reset();
  m_shader.reset();
  m_backend.reset();
  m_window.reset();
  SDL_Quit();
}

void Game::run() {
  if (!m_running)
    return;

  XEV_INFO("TestApp loop started.");

  while (m_running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        m_running = false;
      }
      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && m_window &&
          event.window.windowID ==
              SDL_GetWindowID(m_window->getNativeWindow())) {
        m_running = false;
      }
    }

    if (m_renderer) {
      m_renderer->draw();
    }
  }
}

}  // namespace xev
