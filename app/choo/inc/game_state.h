enum GameStateType {
  MAINMENU,
  GAMEPLAY,
  CUTSCENE,
};

struct GameState {
  GameState();

  GameStateType type;
};
