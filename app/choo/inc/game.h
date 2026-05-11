#pragma once
#include <memory>
#include <xev/backend.h>
#include <xev/camera.h>
#include <xev/logger.h>
#include <xev/renderer.h>
#include <xev/scene.h>
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
  void init_initmenu();
  void init_mainmenu();
  void init_gameplay();
  void exit_gameplay();
  void init_cutscene();
  void exit_cutscene();

private:
  bool m_running;
  GameState m_state;
  std::unique_ptr<xev::Window> m_window;
  std::shared_ptr<xev::Backend> m_backend;
  std::unique_ptr<xev::Scene> m_scene;
  std::unique_ptr<xev::Renderer> m_renderer;
};
