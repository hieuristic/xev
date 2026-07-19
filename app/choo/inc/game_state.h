enum GameStateType {
  MAINMENU,
  GAMEPLAY,
  CUTSCENE,
};

class GameState {
public:
  GameState();

  GameStateType type;
};
