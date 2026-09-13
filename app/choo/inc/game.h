#pragma once
#include <atomic>
#include <entt/entt.hpp>
#include <memory>
#include "controller.h"

namespace xev {
struct Window;
struct Engine;
struct Renderer2D;
struct Renderer3D;
struct Scene;
struct Font;
}  // namespace xev

struct GUI;
struct Controller;

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
  void render(std::atomic<bool>& scene_ready);
  void handle_input(std::atomic<bool>& scene_ready);
  void update_camera();

 private:
  bool m_running{true};

  uint64_t m_tick{0};
  float m_dt{0.0f};

  bool m_isMouseCaptured{false};
  float m_mouseX{0.0f}, m_mouseY{0.0f};
  float m_mouseRelX{0.0f}, m_mouseRelY{0.0f};
  bool m_isMouseDown;

  Controller m_controller;
  GameState m_state{GameState::Hauptmenu};
  entt::registry m_registry;
  entt::entity m_player;
  entt::entity m_map;

  std::unique_ptr<xev::Window> m_window;
  std::unique_ptr<xev::Engine> m_engine;
  std::unique_ptr<xev::Renderer3D> m_renderer3D;
  std::unique_ptr<xev::Renderer2D> m_renderer2D;
  std::unique_ptr<xev::Scene> m_scene;
  std::unique_ptr<xev::Font> m_font;

  std::unique_ptr<GUI> m_gui;
};
