#pragma once
#include <memory>

namespace xev {
struct Window;
struct Engine;
struct Renderer2D;
struct Renderer3D;
struct Scene;
struct Font;
}  // namespace xev

struct GUI;

enum struct GameState : uint8_t {
  Hauptmenu,
  Gameplay,
  Loading,
  // GAME_CUTSCENE,
};

struct Game {
  Game();
  ~Game();
  void run();
  void draw_hauptmenu();
  void draw_gameplay();

 private:
  bool m_running{true};
  bool m_isMouseCaptured{false};
  GameState m_state{GameState::Hauptmenu};

  std::unique_ptr<xev::Window> m_window;
  std::unique_ptr<xev::Engine> m_engine;
  std::unique_ptr<xev::Renderer3D> m_renderer3D;
  std::unique_ptr<xev::Renderer2D> m_renderer2D;
  std::unique_ptr<xev::Scene> m_scene;
  std::unique_ptr<xev::Font> m_font;

  std::unique_ptr<GUI> m_gui;
};
