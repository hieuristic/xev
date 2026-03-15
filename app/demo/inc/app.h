#pragma once
#include <xev/backend.h>
#include <xev/renderer2D.h>
#include <xev/renderer3D.h>
#include <xev/window.h>

class App {
 public:
  App();
  void draw();
  void run();

 public:
  enum ACTION { QUIT };
  std::unordered_map<ACTION, SDL_EventType> keymap;

 private:
  void handle_inputs();
  bool m_running;

  std::unique_ptr<xev::Window> m_window;
  std::unique_ptr<xev::Backend> m_backend;
  xev::unique_ptr<Renderer3D> m_renderer3D;
  xev::unique_ptr<Renderer2D> m_renderer2D;
}
