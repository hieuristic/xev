#include <xev/backend.h>

#include <choo/gui.h>

class GUI {
  GUI(const xev::Backend &backend, );
  ~GUI();

public:
  createInitMenu();
  createMainMenu();
  createGamePlay();
}
