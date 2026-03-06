#pragma once
#include <memory>
#include <xev/window.h>
#include <xev/backend.h>

namespace xev {

class Shader;
class Scene;
class Renderer;

class App {
public:
  App();
  virtual ~App();
  virtual void run();

protected:
  bool m_running;
  std::unique_ptr<Window> m_window;
  std::shared_ptr<Backend> m_backend;
  std::unique_ptr<Shader> m_shader;
  std::unique_ptr<Scene> m_scene;
  std::unique_ptr<Renderer> m_renderer;
};

} // namespace xev
