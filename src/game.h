#include "raylib.h"
#include <stdbool.h>
#define BLOCK_COUNT_X 10
#define BLOCK_COUNT_Y 20
#define EXTRA_ABOVE 20

enum PlayState {
  STATE_MENU,
  STATE_PAUSED,
  STATE_PLAYING,
  STATE_GAME_OVER,
  STATE_QUITTING,
};

enum ButtonOption {
  BTN_NONE,
  BTN_PAUSE,
  BTN_PLAY,
  BTN_QUIT,
};

struct Block {
  int x;
  int y;
  int id;
  int rot;
};

#define SONG_COUNT 4

struct GameState {
  enum PlayState playstate;
  int next_block_id;
  unsigned int turn;
  unsigned int rows_cleared;
  double last_time;
  struct Block active_block;
  struct Block preview_block;
  char board[BLOCK_COUNT_Y + EXTRA_ABOVE][BLOCK_COUNT_X];

  enum ButtonOption gamepad_hilighted_btn;

  unsigned int current_song;
  float time_song_started;
  Music songs[SONG_COUNT];

  Sound move_down_sound;
  Sound move_horizontal_sound;
  Sound lock_sound;
  Sound reload_sound;
  Sound rotate_sound;
  Sound row_clear_sound;
  Sound game_over_sound;

  Sound play_button_sound;
  Sound quit_button_sound;
  Sound hover_button_sound;
  bool is_hovering_play_button;
  bool is_hovering_quit_button;
  bool is_hovering_pause_button;
};
