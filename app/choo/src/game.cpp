#include <SDL3/SDL.h>

#include <xev/engine.h>
#include <xev/renderer2D.h>
#include <xev/renderer3D.h>
#include <xev/resource/scene.h>
#include <xev/ui/font.h>
#include <xev/window.h>

#include "game.h"
#include "gui.h"

Game::Game() : m_running(true) {
  m_window = std::make_unique<xev::Window>("demogame", 800, 600);

  SDL_SetWindowRelativeMouseMode(m_window->get_native(), true);

  const char* base = SDL_GetBasePath();
  std::filesystem::path basePath = base ? std::filesystem::path(base) / ".."
                                        : std::filesystem::current_path();

  m_engine = std::make_unique<xev::Engine>(m_window->get_native());
  m_engine->fileSys->mount(xev::LooseFile{basePath});
  m_engine->init_swapchain();
  m_engine->init_resource_manager();
  m_engine->init_hot_exec();
  m_engine->init_global_descriptor_set();
  m_engine->init_pipeline_manager();
  m_engine->init_frame_context();

  m_renderer3D = std::make_unique<xev::Renderer3D>(*m_engine->pipelineManager);
  m_renderer2D = std::make_unique<xev::Renderer2D>(
      *m_engine->pipeline_manager, *m_engine->resourceManager,
      m_engine->frameContext->get_num_frames());

  m_font = std::make_unique<xev::Font>(
      *m_engine->resourceManager, *m_engine->hotExec, *m_engine->fileSys,
      "assets/fonts/akkurat.bin", "assets/fonts/akkurat.json");
  m_font->bind(*m_engine->globalDescriptorSet);

  m_gui = std::make_unique<GUI>(*m_renderer2D, *m_font,
                                static_cast<float>(m_window.width),
                                static_cast<float>(m_window.height));
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
  if (!m_running) return;

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

    switch (m_state) {
      case STATE_HAUPTMENU:
        m_gui->draw_hauptmenu(800.0, 600.0, glm::vec2(mouseX, mouseY),
                              mouseDown, m_state, m_running);
        break;
      case STATE_GAMEPLAY:
        m_gui.draw_gameplay();
        break;
    }
  }
}

void Game::draw_hauptmenu() {
  m_renderer2D->draw_text(*m_font, ;
}

void Game::draw_gameplay() {
  ;
}

}  // namespace xev
