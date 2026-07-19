#pragma once
#include "game_state.h"

namespace xev {
class Window;
class Engine;
class Renderer2D;
class Renderer3D;
class Scene;
}  // namespace xev

class Game {
 public:
  Game();
  ~Game();
  void run();

 private:
  bool m_running;
  GameState m_state;

  std::unique_ptr<xev::Window> m_window;
  std::unique_ptr<xev::Engine> m_engine;
  std::unique_ptr<xev::Renderer3D> m_renderer3D;
  std::unique_ptr<xev::Renderer2D> m_renderer2D;
  std::unique_ptr<xev::Scene> m_scene;
};
