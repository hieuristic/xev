#pragma once
#include <memory>
#include <xev/backend.h>
#include <xev/camera.h>
#include <xev/logger.h>
#include <xev/renderer.h>
#include <xev/scene.h>
#include <xev/shader.h>
#include <xev/window.h>

enum GameState {
  INITMENU,
  MAINMENU,
  GAMEPLAY,
  CUTSCENE,
};

class Game {
public:
  Game(GameState state);
  ~Game();
  void run();

public:
  // game state
  initInitMenu();
  initMainMenu();
  initGamePlay();
  exitGamePlay();
  initCutscene();
  exitCutscene();

private:
  bool m_running;
  bool m_state;
  std::unique_ptr<Window> m_window;
  std::shared_ptr<Backend> m_backend;
  std::unique_ptr<Shader> m_shader;
  std::unique_ptr<Scene> m_scene;
  std::unique_ptr<Renderer> m_renderer;
};
