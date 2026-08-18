#pragma once
#include <xev/backend.h>

struct GUI {
  GUI(const xev::Backend &backend);
  ~GUI();

  void create_initmenu();
  void create_mainmenu();
  void create_gameplay();
};
