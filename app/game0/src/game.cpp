#include <SDL3/SDL.h>
#include "app.h"
#include <xev/backend.h>
#include <xev/camera.h>
#include <xev/logger.h>
#include <xev/renderer.h>
#include <xev/scene.h>
#include <xev/shader.h>

namespace xev {

App::App() : m_running(true) {
  xev::InitLogger();

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    XEV_ERROR("SDL_Init failed: {}", SDL_GetError());
    m_running = false;
    return;
  }

  m_window = std::make_unique<Window>("test", 800, 600);
  if (!m_window->getNativeWindow()) {
    XEV_ERROR("Failed to create application window.");
    m_running = false;
  }

  m_backend = std::make_shared<Backend>(m_window->getNativeWindow());

  m_shader = std::make_unique<Shader>();
  std::string base_path(SDL_GetBasePath());
  m_shader->load(m_backend->get_device(), base_path + "shaders/mesh.spv",
                   S_GRAPHICS);
  if (m_shader->get_shader_type() == S_NULL) {
    XEV_ERROR("FAILED TEST SHADER LOADING!");
  } else {
    XEV_INFO("SUCCESS TEST SHADER LOADING!");
  }

  m_scene = std::make_unique<Scene>();
  m_scene->load_gltf(base_path + "../../assets/bunny.glb");

  XEV_INFO("Scene vertices: {}, faces: {}", m_scene->m_vert_buffer.size(),
           m_scene->m_face_buffer.size());

  m_renderer = std::make_unique<Renderer>(m_backend, std::move(m_shader), *m_scene);
}

App::~App() {
  m_renderer.reset();
  m_scene.reset();
  m_shader.reset();
  m_backend.reset();
  m_window.reset();
  SDL_Quit();
}

void App::run() {
    if (!m_running)
      return;

    XEV_INFO("TestApp loop started.");

    while (m_running) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
          m_running = false;
        }
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
            m_window && event.window.windowID ==
                SDL_GetWindowID(m_window->getNativeWindow())) {
          m_running = false;
        }
      }

      if (m_renderer) {
          m_renderer->draw();
      }
    }
}

} // namespace xev
