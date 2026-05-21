#pragma once
#include <SDL3/SDL.h>
#include <xev/engine.h>
#include <xev/renderer2D.h>
#include <xev/renderer3D.h>
#include <xev/resource/scene.h>
#include <xev/window.h>
#include "controller.h"

class App {
 public:
  App();
  ~App();

  void draw();
  void run();

 public:
  enum ACTION { QUIT };
  std::unordered_map<ACTION, SDL_EventType> keymap;

 private:
  void handle_inputs();
  bool m_running = true;

  uint64_t m_last_time = 0;
  const bool* m_keystate = nullptr;

  std::unique_ptr<xev::Window> m_window;
  std::unique_ptr<xev::Engine> m_engine;
  std::unique_ptr<xev::Renderer3D> m_renderer3D;
  std::unique_ptr<xev::Renderer2D> m_renderer2D;
  std::unique_ptr<xev::Scene> m_scene;
  Controller m_controller;
};
