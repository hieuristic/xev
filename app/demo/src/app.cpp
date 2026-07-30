// In app.cpp let's implement the core logic for the demo:

#include "app.h"
#include <xev/frame_context.h>
#include <xev/globalDescriptorSet.h>
#include <xev/logger.h>
#include <xev/resourceManager.h>
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
  SDL_SetWindowRelativeMouseMode(m_window->get_native(), true);

  m_engine = std::make_unique<xev::Engine>(m_window->get_native());
  m_engine->init_swapchain();
  m_engine->init_resource_manager();
  m_engine->init_hot_exec();
  m_engine->init_global_descriptor_set();
  m_engine->init_pipeline_manager();
  m_engine->init_frame_context();

  m_renderer3D = std::make_unique<xev::Renderer3D>(
      *m_engine->pipelineManager, m_engine->globalDescriptorSet->get_layout());

  m_scene = std::make_unique<xev::Scene>();
  const char* base_path = SDL_GetBasePath();
  std::string base_path =
      SDL_GetBasePath() ? std::string(SDL_GetBasePath()) + "../" : "";
  std::string scene_path = base_path + "assets/models/sponza_full.glb";

  XEV_INFO("Loading Sponza...");
  m_scene->load_gltf(scene_path);
  // m_scene->create_test_triangle();
  m_scene->alloc(*m_engine->resourceManager);
  m_scene->upload(*m_engine->resourceManager, *m_engine->hotExec);
  m_scene->bind(*m_engine->globalDescriptorSet);
  // m_scene->active_cam.set_aspect(m_window->get_aspect());

  m_keystate = SDL_GetKeyboardState(NULL);
  m_last_time = SDL_GetTicks();

  m_renderer2D = std::make_unique<xev::Renderer2D>(
      *m_engine->pipelineManager, *m_engine->resourceManager,
      m_engine->globalDescriptorSet->get_layout(),
      m_engine->frameContext->get_num_frames());

  m_font = std::make_unique<xev::Font>();
  {
    std::string atlas_path = base_path + "assets/fonts/akkurat.bin";
    std::string config_path = base_path + "assets/fonts/akkurat.bin";
    m_font.load(*m_engine->resourceManager, atlas_path, config_path);
    m_font.bind(*m_engine->globalDescriptorSet);
  }
}

App::~App() {
  if (m_scene && m_engine->resourceManager) {
    m_scene->destroy(*m_engine->resourceManager);
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
  VkCommandBuffer cmdbuf = m_engine->frame_context->acquire_frame();

  if (cmdbuf != VK_NULL_HANDLE) {
    const xev::Image& output_color =
        m_engine->frame_context->get_current_render_target();
    const xev::Image& output_depth =
        m_engine->frame_context->get_current_render_depth();
    m_renderer3D->draw(cmdbuf, output_color, output_depth,
                       *m_engine->globalDescriptorSet, *m_scene,
                       m_scene->active_cam, {0.1f, 0.1f, 0.1f, 1.0f});

    glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(2.0, 2.0, 2.0));
    m_renderer2D->draw_text(m_font, "hello world", transform, 0.1);
    m_renderer2D->draw(cmdbuf, output_color, *m_engine->globalDescriptorSet,
                       {0.1f, 0.1f, 0.1f, 1.0f});

    m_engine->submit_and_show(cmdbuf, output_color);
  }
}

void App::run() {
  while (m_running) {
    handle_inputs();
    draw();
  }
}
