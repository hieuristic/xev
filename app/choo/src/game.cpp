#include <SDL3/SDL.h>

#include <xev/engine.h>
#include <xev/filesystem/fs.h>
#include <xev/filesystem/loose.h>
#include <xev/frame_context.h>
#include <xev/global_descriptor_set.h>
#include <xev/hot_exec.h>
#include <xev/pipeline_manager.h>
#include <xev/renderer2D.h>
#include <xev/renderer3D.h>
#include <xev/resource/scene.h>
#include <xev/resource_manager.h>
#include <xev/ui/font.h>
#include <xev/window.h>

#include "game.h"
#include "gui.h"

Game::Game() : m_running(true) {
  m_window = std::make_unique<xev::Window>("demogame", 800, 600);

  SDL_SetWindowRelativeMouseMode(m_window->get_native(), m_isMouseCaptured);

  const char* base = SDL_GetBasePath();
  std::filesystem::path basePath = base ? std::filesystem::path(base) / ".."
                                        : std::filesystem::current_path();

  m_engine = std::make_unique<xev::Engine>(m_window->get_native());
  m_engine->init_file_system();
  m_engine->fileSys->mount(xev::LooseMount{basePath / "assets"});
  m_engine->init_swapchain();
  m_engine->init_resource_manager();
  m_engine->init_hot_exec();
  m_engine->init_global_descriptor_set();
  m_engine->init_pipeline_manager();
  m_engine->init_frame_context();

  m_renderer3D = std::make_unique<xev::Renderer3D>(*m_engine->pipelineManager);
  m_renderer2D = std::make_unique<xev::Renderer2D>(
      *m_engine->pipelineManager, *m_engine->resourceManager,
      m_engine->frameContext->get_num_frames());

  m_font = std::make_unique<xev::Font>(
      *m_engine->resourceManager, *m_engine->hotExec, *m_engine->fileSys,
      "fonts/akkurat.bin", "fonts/akkurat.json");
  m_font->bind(*m_engine->globalDescriptorSet);

  m_gui = std::make_unique<GUI>(static_cast<float>(m_window->width()),
                                static_cast<float>(m_window->height()),
                                *m_renderer2D, *m_font);
}

Game::~Game() {
  if (m_scene) {
    m_scene->destroy(*m_engine->resourceManager);
  }
}

void Game::run() {
  if (!m_running) return;

  while (m_running) {
    float mouseX{0.0f}, mouseY{0.0f};
    uint32_t mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
    bool mouseDown = (mouseButtons & SDL_BUTTON_LMASK) != 0;

    // input handling
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        m_running = false;
      }
      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && m_window &&
          event.window.windowID == SDL_GetWindowID(m_window->get_native())) {
        m_running = false;
      }
      if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE) {
          m_running = false;
        }
        if (event.key.key == SDLK_Q) {
          m_isMouseCaptured = !m_isMouseCaptured;
          SDL_SetWindowRelativeMouseMode(m_window->get_native(),
                                         m_isMouseCaptured);
        }
      }
    }

    // rendering
    VkCommandBuffer cmdbuf = m_engine->frameContext->acquire_frame();
    if (cmdbuf != VK_NULL_HANDLE) {
      const xev::Image& output_color =
          m_engine->frameContext->get_current_render_target();

      switch (m_state) {
        case GameState::Hauptmenu:
          m_gui->draw_hauptmenu(glm::vec2(mouseX, mouseY), mouseDown, m_state,
                                m_running);
          break;
        case GameState::Gameplay:
          m_gui->draw_gameplay();
          break;
      }

      m_renderer2D->draw(cmdbuf, output_color, *m_engine->globalDescriptorSet,
                         m_engine->frameContext->get_current_index(),
                         {0.1f, 0.1f, 0.1f, 1.0f});
      m_engine->submit_and_show(cmdbuf, output_color);
    }
  }
}
