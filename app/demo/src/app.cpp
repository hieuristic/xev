// In app.cpp let's implement the core logic for the demo:

#include "app.h"
#include <xev/logger.h>
#include <xev/volk.h>

void compute_projection(const xev::Camera& cam,
                        const std::vector<glm::vec3> pos_world) {
  glm::mat4 proj_mat = cam.create_vp_mat();
  std::vector<glm::vec4> pos_clip;
  pos_clip.reserve(pos_world.size());
  for (const auto& p : pos_world) {
    glm::vec4 p_clip = proj_mat * glm::vec4(p, 1.0);
    XEV_INFO("clip coordinates: x={} y={} z={} w={}", p_clip.x, p_clip.y,
             p_clip.z, p_clip.w);
    pos_clip.emplace_back(p_clip);
  }
}

App::App() {
  m_window = std::make_unique<xev::Window>("Sponza Demo");
  m_backend = std::make_shared<xev::Backend>(m_window->get_native());
  SDL_SetWindowRelativeMouseMode(m_window->get_native(), true);

  auto size = m_window->get_size();
  m_renderer3D = std::make_unique<xev::Renderer3D>(
      m_backend, static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));

  m_scene = std::make_unique<xev::Scene>();
  const char* base_path = SDL_GetBasePath();
  std::string scene_path =
      base_path ? std::string(base_path) + "../assets/models/sponza_full.glb"
                : "assets/models/sponza_full.glb";

  XEV_INFO("Loading Sponza...");
  m_scene->load_gltf(scene_path);
  // XEV_INFO("Loading Triangle...");
  // m_scene->create_test_triangle();
  m_scene->reserve(*m_backend);
  m_scene->upload_meshes();
  m_scene->upload_textures();
  m_scene->active_cam.set_aspect(m_window->get_aspect());

  m_keystate = SDL_GetKeyboardState(NULL);
  m_last_time = SDL_GetTicks();
}

App::~App() {
  if (m_scene) {
    m_scene->destroy(*m_backend);
  }
}

void App::handle_inputs() {
  SDL_Event e;
  float xrel = 0;
  float yrel = 0;

  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_EVENT_QUIT:
        m_running = false;
        break;
      case SDL_EVENT_KEY_DOWN:
        if (e.key.key == SDLK_ESCAPE)
          m_running = false;
        break;
      case SDL_EVENT_MOUSE_MOTION:
        xrel = e.motion.xrel;
        yrel = e.motion.yrel;
        break;
    }
  }

  uint64_t current_time = SDL_GetTicks();
  float dt = (current_time - m_last_time) / 1000.0f;
  m_last_time = current_time;

  xev::Camera& cam = m_scene->active_cam;
  m_controller.update(dt, xrel, yrel, m_keystate, cam.pos, cam.rot);
  return;
}

void App::draw() {
  VkCommandBuffer cmd = m_backend->enter_frame();
  if (cmd != VK_NULL_HANDLE) {
    xev::FrameArg args;
    args.clear_color = {0.1f, 0.1f, 0.1f, 1.0f};
    args.copy_to_swapchain = true;
    const xev::Image& output_image =
        m_renderer3D->draw(cmd, *m_scene, m_scene->active_cam, args);
    m_backend->leave_frame(cmd, output_image, args);
  }
}

void App::run() {
  while (m_running) {
    handle_inputs();
    draw();
  }
}
