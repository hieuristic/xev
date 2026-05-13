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

  m_scene->load(*m_backend);

  std::vector<glm::vec3> p = {
      glm::vec3(0.0f, 0.5f, 1.0f),
      glm::vec3(-0.5f, -0.5f, 1.0f),
      glm::vec3(0.5f, -0.5f, 1.0f),
  };
  compute_projection(m_scene->m_active_cam, p);

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
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_EVENT_QUIT) {
      m_running = false;
    }
    if (e.type == SDL_EVENT_KEY_DOWN) {
      if (e.key.key == SDLK_ESCAPE) {
        m_running = false;
      }
    }
  }

  uint64_t current_time = SDL_GetTicks();
  float dt = (current_time - m_last_time) / 1000.0f;
  m_last_time = current_time;

  float speed = 5.0f * dt;
  if (m_keystate[SDL_SCANCODE_LSHIFT])
    speed *= 2.0f;

  xev::Camera& cam = m_scene->m_active_cam;

  // Get camera local axes
  glm::mat4 R = glm::mat4_cast(cam.rot);
  glm::vec3 forward = -glm::vec3(R[2]);  // -Z is forward in RDF
  glm::vec3 right = glm::vec3(R[0]);     // +X is right
  glm::vec3 up = glm::vec3(R[1]);        // -Y is up in RDF (Y is down)

  if (m_keystate[SDL_SCANCODE_W])
    cam.pos += forward * speed;
  if (m_keystate[SDL_SCANCODE_S])
    cam.pos -= forward * speed;
  if (m_keystate[SDL_SCANCODE_A])
    cam.pos -= right * speed;
  if (m_keystate[SDL_SCANCODE_D])
    cam.pos += right * speed;

  float theta = 0.01f * speed;
  glm::quat turn_up = glm::angleAxis(-theta, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::quat turn_down = glm::angleAxis(theta, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::quat turn_right = glm::angleAxis(theta, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::quat turn_left = glm::angleAxis(-theta, glm::vec3(0.0f, 1.0f, 0.0f));
  if (m_keystate[SDL_SCANCODE_K])
    cam.rot = turn_up * cam.rot;
  if (m_keystate[SDL_SCANCODE_J])
    cam.rot = turn_down * cam.rot;
  if (m_keystate[SDL_SCANCODE_L])
    cam.rot = turn_right * cam.rot;
  if (m_keystate[SDL_SCANCODE_H])
    cam.rot = turn_left * cam.rot;
  cam.rot = glm::normalize(cam.rot);

  // Q and E for up/down (world space)
  if (m_keystate[SDL_SCANCODE_Q])
    cam.pos -= glm::vec3(0.0f, 1.0f, 0.0f) * speed;  // Up (negative Y)
  if (m_keystate[SDL_SCANCODE_E])
    cam.pos += glm::vec3(0.0f, 1.0f, 0.0f) * speed;  // Down (positive Y)
}

void App::draw() {
  VkCommandBuffer cmd = m_backend->enter_frame();
  if (cmd != VK_NULL_HANDLE) {
    xev::FrameArg args;
    args.clear_color = {0.1f, 0.1f, 0.1f, 1.0f};
    args.copy_to_swapchain = true;
    const xev::Image& output_image =
        m_renderer3D->draw(cmd, *m_scene, m_scene->m_active_cam, args);
    m_backend->leave_frame(cmd, output_image, args);
  }
}

void App::run() {
  while (m_running) {
    handle_inputs();
    draw();
  }
}
