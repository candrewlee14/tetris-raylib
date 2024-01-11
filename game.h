#include <stdbool.h>
#define BLOCK_COUNT_X 10
#define BLOCK_COUNT_Y 20
#define EXTRA_ABOVE 20

enum PlayState {
  STATE_MENU,
  STATE_PLAYING,
  STATE_GAME_OVER,
  STATE_QUITTING,
};

struct Block {
  int x;
  int y;
  int id;
  int rot;
};

struct GameState {
  enum PlayState playstate;
  int next_block_id;
  unsigned int turn;
  unsigned int rows_cleared;
  double last_time;
  struct Block active_block;
  struct Block preview_block;
  char board[BLOCK_COUNT_Y + EXTRA_ABOVE][BLOCK_COUNT_X];
};
