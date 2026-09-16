#include "game.h"
#include "server.h"
#include <string>

int main(int argc, char *argv[]) {
  if (argc > 1 && (std::string(argv[1]) == "--server" || std::string(argv[1]) == "-s")) {
    uint16_t port = (argc > 2) ? static_cast<uint16_t>(std::stoi(argv[2])) : 1906;
    Server server;
    return server.run(port);
  }

  Game game;
  game.run();
  return 0;
}
