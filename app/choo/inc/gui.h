#pragma once
#include <xev/backend.h>

class GUI {
 public:
  GUI(const xev::Backend &backend);
  ~GUI();

 public:
  void create_initmenu();
  void create_mainmenu();
  void create_gameplay();
};
