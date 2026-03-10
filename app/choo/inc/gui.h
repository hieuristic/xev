#pragma once
#include <xev/backend.h>

class GUI {
  GUI(const xev::Backend &backend, );
  ~GUI();

public:
  create_initmenu();
  create_mainmenu();
  create_gameplay();
}
