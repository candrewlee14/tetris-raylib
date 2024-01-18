#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "raylib.h"
#include "game.h"
#include "tetr.h"

#define TITLE_SIZE_RATIO 0.05f

#define BUTTON_WIDTH_RATIO 0.2f
#define BUTTON_HEIGHT_RATIO 0.0666f
#define BUTTON_TEXT_SIZE_RATIO_TO_BUTTON 0.8f
#define BUTTON_SPACING_RATIO 0.025f

#define STATS_Y_RATIO 0.42f
#define STATS_X_RATIO 0.82f
#define BUTTONS_X_RATIO 0.82f
#define NEXT_TETRI_Y_RATIO 0.1f
#define TEXT_SIZE_RATIO 0.05f

#define BLOCK_SIZE_RATIO_TO_WINDOW_Y 0.043f
#define BOARD_X_RATIO 0.5f
#define BOARD_Y_RATIO 0.1f

#define GAMEPAD 0

#define TIME_STEP 0.5f
#define TIME_STEP_FAST 0.1f

void InitSound(struct GameState* state) {
    InitAudioDevice();
    // load same song twice cuz it's much shorter
    state->songs[0] = LoadMusicStream("./assets/very-lush-and-swag-loop-74140.mp3");
    state->songs[1] = LoadMusicStream("./assets/very-lush-and-swag-loop-74140.mp3");
    state->songs[2] = LoadMusicStream("./assets/song-2018-30015.mp3");
    state->songs[3] = LoadMusicStream("./assets/8_Bit_Retro_Funk-David_Renda.mp3");

    state->game_over_sound = LoadSound("./assets/videogame-death-sound.mp3");
    state->move_down_sound = LoadSound("./assets/one_beep-99630.mp3");
    state->lock_sound = LoadSound("./assets/deep-thud-8bit.mp3");
    state->row_clear_sound = LoadSound("./assets/8-bit-powerup-6768.mp3");
    state->rotate_sound = LoadSound("./assets/woosh-sfx-95844.mp3");
    state->move_horizontal_sound = LoadSound("./assets/impact-8-bit-retro.mp3");
    state->reload_sound = LoadSound("./assets/8-bit-laser-151672.mp3");
    state->play_button_sound = LoadSound("./assets/coin-collect-retro-8-bit-sound-effect.mp3");
    state->hover_button_sound = LoadSound("./assets/cymbal-83127.mp3");

    state->current_song = 0;
}

void DeinitSound(struct GameState* state) {
    for (int i = 0; i < SONG_COUNT; i++) {
        UnloadMusicStream(state->songs[i]);
    }
    UnloadSound(state->game_over_sound);
    UnloadSound(state->move_down_sound);
    UnloadSound(state->lock_sound);
    UnloadSound(state->row_clear_sound);
    UnloadSound(state->rotate_sound);
    UnloadSound(state->move_horizontal_sound);
    UnloadSound(state->reload_sound);
    UnloadSound(state->play_button_sound);
    UnloadSound(state->hover_button_sound);
    CloseAudioDevice();
}

bool TryMoveBlock(struct GameState* state, int dx, int dy, int drot, struct Block* block) {
    int x = block->x + dx;
    int y = block->y + dy;
    int rot = block->rot + drot;
    int id = block->id;
    struct TetriminoInfo info = GetTetriminoInfo(id);
    char* tetr = info.data;
    int size = info.size;
    int pos_rot = rot % 4;
    if (pos_rot < 0) {
        pos_rot += 4;
    }
    for (int yb = 0; yb < size; yb++) {
        for (int xb = 0; xb < size; xb++) {
            if (tetr[pos_rot*size*size + yb*size + xb] > 0) {
                if (x + xb < 0 || x + xb >= BLOCK_COUNT_X || y + yb >= BLOCK_COUNT_Y) {
                    return false;
                }
                if (state->board[EXTRA_ABOVE + y + yb][x + xb] > 0) {
                    return false;
                }
            }
        }
    }
    block->x = x;
    block->y = y;
    block->rot = rot;
    return true;
}

void SetPreviewBlock(struct GameState* state, int id) {
    state->preview_block.id = state->active_block.id;
    state->preview_block.x = state->active_block.x;
    state->preview_block.y = state->active_block.y;
    state->preview_block.rot = state->active_block.rot;

    while (TryMoveBlock(state, 0, 1, 0, &state->preview_block)) {}
}

void GetNewActiveBlock(struct GameState* state) {
    state->active_block.id = state->next_block_id;
    state->active_block.x = 3;
    state->active_block.y = -2;
    state->active_block.rot = 0;
    state->next_block_id = GetRandomValue(0, 6); 
    SetPreviewBlock(state, state->active_block.id);
}

void LockActiveBlock(struct GameState* state) {
    int x = state->active_block.x;
    int y = state->active_block.y;
    int rot = state->active_block.rot;
    int id = state->active_block.id;
    struct TetriminoInfo info = GetTetriminoInfo(id);
    char* tetr = info.data;
    int size = info.size;
    int pos_rot = rot % 4;
    if (pos_rot < 0) {
        pos_rot += 4;
    }
    for (int yb = 0; yb < size; yb++) {
        for (int xb = 0; xb < size; xb++) {
            if (tetr[pos_rot*size*size + yb*size + xb] > 0) {
                state->board[EXTRA_ABOVE + y + yb][x + xb] = id + 1;
            }
        }
    }
}

void GameInit(struct GameState* state) {
    state->playstate = STATE_MENU;
    state->rows_cleared = 0;
    state->turn = 0;
    state->last_time = GetTime();
    SetRandomSeed(GetTime() * 1000);
    state->next_block_id = GetRandomValue(0, 6); 
    for (int i = 0; i < BLOCK_COUNT_Y + EXTRA_ABOVE; i++) {
        for (int j = 0; j < BLOCK_COUNT_X; j++) {
            state->board[i][j] = 0;
        }
    }
    GetNewActiveBlock(state);
}


void DrawBlock(int x, int y, int block_size, Color color ) {
    DrawRectangle(x + 1, y + 1, block_size - 2, block_size - 2, color);
}

int ClearFullRows(struct GameState* state) {
    int cleared = 0;
    for (int i = 0; i < BLOCK_COUNT_Y + EXTRA_ABOVE; i++) {
        bool full = true;
        for (int j = 0; j < BLOCK_COUNT_X; j++) {
            if (state->board[i][j] == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            cleared++;
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < BLOCK_COUNT_X; j++) {
                    state->board[k][j] = state->board[k - 1][j];
                }
            }
            for (int j = 0; j < BLOCK_COUNT_X; j++) {
                state->board[0][j] = 0;
            }
        }
    }
    return cleared;
}

void DrawTetrimino(int id, int x, int y, int rot, bool preview, Rectangle* mask) {
    Color color;
    // tetr is a pointer to data of tetr
    // which is [4][size][size] chars
    char* tetr;
    int size;
    struct TetriminoInfo info = GetTetriminoInfo(id);
    tetr = info.data;
    size = info.size;
    color = ColorFromId(id);
    int pos_rot = rot % 4;
    if (pos_rot < 0) {
        pos_rot += 4;
    }
    int block_size = (int)(BLOCK_SIZE_RATIO_TO_WINDOW_Y * (float)GetRenderHeight());
    for (int yb = 0; yb < size; yb++) {
        for (int xb = 0; xb < size; xb++) {
            if (tetr[pos_rot*size*size + yb*size + xb] > 0) {
                if (mask != NULL) {
                    if (!CheckCollisionPointRec((Vector2){x + xb*block_size, y + yb*block_size}, *mask)) {
                        continue;
                    }
                }
                DrawBlock(x + xb*block_size, y + yb*block_size, block_size, color);
                if (preview) {
                    // draw black inner
                    DrawRectangle(x + xb*block_size + 2, y + yb*block_size + 2, 
                                  block_size - 4, block_size - 4, BLACK);
                }
            }
        }
    }
}

void DrawGameBoard(int x, int y, int w, int h, int border_size, struct GameState* state) {
    int block_size = (int)(BLOCK_SIZE_RATIO_TO_WINDOW_Y * (float)GetRenderHeight());
    DrawRectangle(x - border_size, y - border_size, 
                  w + border_size*2, h + border_size*2, LIGHTGRAY);
    DrawRectangle(x, y, w, h, BLACK);
    for (int i = 0; i < BLOCK_COUNT_Y; i++) {
        for (int j = 0; j < BLOCK_COUNT_X; j++) {
            if (state->board[i + EXTRA_ABOVE][j] > 0) {
                DrawBlock(x + j*block_size, y + i*block_size, block_size, 
                          ColorFromId(state->board[i + EXTRA_ABOVE][j] - 1));
            }
        }
    }
}

void QuitButton(struct GameState* state) {
    int window_width = GetRenderWidth();
    int window_height = GetRenderHeight();
    int button_height = (int)(BUTTON_HEIGHT_RATIO * (float)window_height);
    int button_width = (int)(BUTTON_WIDTH_RATIO * (float)window_width);
    int button_spacing = (int)(BUTTON_SPACING_RATIO * (float)window_height);
    int button_text_size = (int)(BUTTON_TEXT_SIZE_RATIO_TO_BUTTON * (float)button_height);

    int button_x = (int)(BUTTONS_X_RATIO * (float)window_width) - button_width / 2;
    int button_y = window_height - 86 ;

    char* text = "QUIT";
    int text_w = MeasureText(text, button_text_size);
    Color color = LIGHTGRAY;
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){button_x, button_y, button_width, button_height})) {
        color = PINK;
        if (!state->is_hovering_quit_button) {
            PlaySound(state->hover_button_sound);
        }
        state->is_hovering_quit_button = true;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            printf("Quitting...\n");
            state->playstate = STATE_QUITTING;
        }
    } else {
        state->is_hovering_quit_button = false;
    }
    if (state->gamepad_hilighted_btn == BTN_QUIT) {
        color = PINK;
        if (IsGamepadAvailable(GAMEPAD) && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
            printf("Quitting...\n");
            state->playstate = STATE_QUITTING;
        }
    }
    DrawRectangle(button_x, button_y, button_width, button_height, color);
    DrawText(text, button_x + button_width/2 - text_w/2, 
             button_y + button_height/2 - button_text_size/2 + 2, button_text_size, BLACK);
}

void PlayButton(struct GameState* state) {
    int window_width = GetRenderWidth();
    int window_height = GetRenderHeight();
    int button_height = (int)(BUTTON_HEIGHT_RATIO * (float)window_height);
    int button_width = (int)(BUTTON_WIDTH_RATIO * (float)window_width);
    int button_spacing = (int)(BUTTON_SPACING_RATIO * (float)window_height);
    int button_text_size = (int)(BUTTON_TEXT_SIZE_RATIO_TO_BUTTON * (float)button_height);

    int button_x = (int)(BUTTONS_X_RATIO * (float)window_width) - button_width / 2;
    int button_y = window_height - 86 - (button_height + button_spacing);
    char* text = "PLAY";
    char* restart = "RESTART";
    if (state->playstate != STATE_MENU) {
        text = restart;
    }
    int text_w = MeasureText(text, button_text_size);
    Color color = LIGHTGRAY;
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){button_x, button_y, button_width, button_height})) {
        if (!state->is_hovering_play_button) {
            PlaySound(state->hover_button_sound);
        }
        state->is_hovering_play_button = true;
        color = SKYBLUE;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            GameInit(state);
            state->playstate = STATE_PLAYING;
            state->gamepad_hilighted_btn = BTN_NONE;
            PlaySound(state->play_button_sound);
        }
    } else {
        state->is_hovering_play_button = false;
    }
    if (state->gamepad_hilighted_btn == BTN_PLAY) {
        color = SKYBLUE;
        if (IsGamepadAvailable(GAMEPAD) && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
            GameInit(state);
            state->playstate = STATE_PLAYING;
            state->gamepad_hilighted_btn = BTN_NONE;
            PlaySound(state->play_button_sound);
        }
    }
    DrawRectangle(button_x, button_y, button_width, button_height, color);
    DrawText(text, button_x + button_width/2 - text_w/2, 
             button_y + button_height/2 - button_text_size/2 + 2, button_text_size, BLACK);
}

void PauseButton(struct GameState* state) {
    int window_width = GetRenderWidth();
    int window_height = GetRenderHeight();
    int button_height = (int)(BUTTON_HEIGHT_RATIO * (float)window_height);
    int button_width = (int)(BUTTON_WIDTH_RATIO * (float)window_width);
    int button_text_size = (int)((float)button_height * BUTTON_TEXT_SIZE_RATIO_TO_BUTTON);
    int button_spacing = (int)(BUTTON_SPACING_RATIO * (float)window_height);

    int button_x = (int)(BUTTONS_X_RATIO * (float)window_width) - button_width / 2;
    int button_y = window_height - 86 - (button_height + button_spacing) * 2;

    if (state->playstate == STATE_MENU || state->playstate == STATE_GAME_OVER) {
        return;
    }
    char* text = "PAUSE";
    if (state->playstate == STATE_PAUSED) {
        text = "RESUME";
    }
    int text_w = MeasureText(text, button_text_size);
    Color color = LIGHTGRAY;
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){button_x, button_y, button_width, button_height})) {
        if (!state->is_hovering_pause_button) {
            PlaySound(state->hover_button_sound);
        }
        state->is_hovering_pause_button = true;
        color = PURPLE;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (state->playstate == STATE_PAUSED) {
                state->playstate = STATE_PLAYING;
                state->gamepad_hilighted_btn = BTN_NONE;
            } else {
                state->playstate = STATE_PAUSED;
                state->gamepad_hilighted_btn = BTN_PAUSE;
            }
        }
    } else {
        state->is_hovering_pause_button = false;
    }
    if (state->gamepad_hilighted_btn == BTN_PAUSE) {
        color = PURPLE;
        if (IsGamepadAvailable(GAMEPAD) && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
            if (state->playstate == STATE_PAUSED) {
                state->playstate = STATE_PLAYING;
                state->gamepad_hilighted_btn = BTN_NONE;
            } else {
                state->playstate = STATE_PAUSED;
                state->gamepad_hilighted_btn = BTN_PAUSE;
            }

        }
    }
    if (IsKeyPressed(KEY_P) || 
        (IsGamepadAvailable(GAMEPAD) && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_MIDDLE_RIGHT))
    ) {
        if (state->playstate == STATE_PAUSED) {
            state->playstate = STATE_PLAYING;
            state->gamepad_hilighted_btn = BTN_NONE;
        } else {
            state->playstate = STATE_PAUSED;
            state->gamepad_hilighted_btn = BTN_PAUSE;
        }
    }
    DrawRectangle(button_x, button_y, button_width, button_height, color);
    DrawText(text, button_x + button_width/2 - text_w/2, 
             button_y + button_height/2 - button_text_size/2 + 2, button_text_size, BLACK);
}

void DrawStats(struct GameState* state) {
    int window_width = GetRenderWidth();
    int window_height = GetRenderHeight();
    int stats_y = (int)(STATS_Y_RATIO * (float)window_height);
    int stats_x = (int)(STATS_X_RATIO * (float)window_width);
    int text_size = (int)(TEXT_SIZE_RATIO * (float)window_height);
    {
        char* text = "TURN:";
        DrawText(text, stats_x - 100, stats_y, text_size, BLACK);
        const char* num_txt =  TextFormat("%i", state->turn + 1);
        int text_w = MeasureText(num_txt, text_size);
        DrawText(num_txt, stats_x - text_w , stats_y + text_size + 5, text_size, BLACK);
    }
    {
        char* text = "CLEARED:";
        int text_y = stats_y + 3 * text_size;
        const char* num_txt =  TextFormat("%i", state->rows_cleared);
        DrawText(text, stats_x - 100, text_y, text_size, BLACK);
        int text_w = MeasureText(num_txt, text_size);
        DrawText(num_txt, stats_x - text_w, text_y + text_size + 5, text_size, BLACK);
    }
}

void DrawNextTetri(struct GameState* state) {
    int window_width = GetRenderWidth();
    int window_height = GetRenderHeight();
    int text_size = (int)(TEXT_SIZE_RATIO * (float)window_height);
    int block_size = (int)(BLOCK_SIZE_RATIO_TO_WINDOW_Y * (float)GetRenderHeight());
    int y = (int)(NEXT_TETRI_Y_RATIO * (float)window_height);
    int w = block_size * 4 + 20;
    int h = block_size * 4 + 20;
    int x = window_width - 250 - w/2;
    int text_w = MeasureText("NEXT", text_size);
    DrawText("NEXT", x + w/2 - text_w/2, y - text_size - 3, text_size, BLACK);
    DrawRectangle(x, y, w, h, BLACK);
    struct TetriminoInfo info = GetTetriminoInfo(state->next_block_id);
    int size = info.size;
    int true_size_y = 0; 
    bool found_row = false;
    int start_y = -1;
    for (int i = 0; i < size; i++) {
        bool empty = true;
        for (int j = 0; j < size; j++) {
            if (info.data[i*size + j] > 0) {
                found_row = true;
                if (start_y == -1) {
                    start_y = i;
                }
                empty = false;
                break;
            }
        }
        if (!empty || !found_row) {
            true_size_y++;
        }
    }
    x += w/2 - size*block_size/2;
    y += h/2 - start_y*block_size/2 - true_size_y*block_size/2;
    DrawTetrimino(state->next_block_id, x, y, 0, false, NULL);
}

void RunEndTurn(struct GameState* state) {
    state->turn++;
    LockActiveBlock(state);
    PlaySound(state->lock_sound);
    int turn_rows_cleared = ClearFullRows(state);
    state->rows_cleared += turn_rows_cleared;
    if (turn_rows_cleared > 0) {
        PlaySound(state->row_clear_sound);
    }
    GetNewActiveBlock(state);
    if (!TryMoveBlock(state, 0, 0, 0, &state->active_block)) {
        PlaySound(state->game_over_sound);
        state->playstate = STATE_GAME_OVER;
    }
}

void* module_main(bool fullscreen, void* data) {
    struct GameState* state = (struct GameState*)data;
    if (state == NULL) {
        state = (struct GameState*)malloc(sizeof(struct GameState));
        GameInit(state);
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
        SetTargetFPS(60);
        InitSound(state);
        InitWindow(1280, 800, "Tetris");
        if (fullscreen) {
            ToggleFullscreen();
        } 
        state->playstate = STATE_MENU;
        state->gamepad_hilighted_btn = BTN_PLAY;

        state->time_song_started = GetTime();
        PlayMusicStream(state->songs[state->current_song % SONG_COUNT]);
    } else {
      // run on reload
    }

    while (!WindowShouldClose()) {
        int window_width = GetRenderWidth();
        int window_height = GetRenderHeight();
        int block_size = (int)(BLOCK_SIZE_RATIO_TO_WINDOW_Y * (float)window_height);
        int game_board_w = block_size * BLOCK_COUNT_X;
        int game_board_h = block_size * BLOCK_COUNT_Y;
        int game_board_x = (int)(BOARD_X_RATIO * (float)window_width) - game_board_w/2;
        int game_board_y = (int)(BOARD_Y_RATIO * (float)window_height);

        unsigned int cur_song = state->current_song % SONG_COUNT;
        if (IsMusicStreamPlaying(state->songs[cur_song])) {
            float time_ran = GetTime() - state->time_song_started;
            if (time_ran >= GetMusicTimeLength(state->songs[cur_song])) {
                StopMusicStream(state->songs[cur_song]);
                cur_song = (cur_song + 1) % SONG_COUNT;
                state->current_song = cur_song;
                state->time_song_started = GetTime();
                PlayMusicStream(state->songs[cur_song]);
            }
        }
        UpdateMusicStream(state->songs[cur_song]);
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            {
                char* text = "TETRIS";
                Color color = BLACK;
                if (state->playstate == STATE_GAME_OVER) {
                    text = "GAME OVER";
                    color = RED;
                }
                int title_size = (int)(TITLE_SIZE_RATIO * (float)GetRenderHeight());
                int text_w = MeasureText(text, title_size);
                DrawText(text, window_width/2 - text_w/2, 24, title_size, color);
            }
            DrawGameBoard(game_board_x, game_board_y, game_board_w, game_board_h, 5, state);

            for (int i = 0; i < 12; i++) {
                DrawTetrimino(i % 7, 50, 10 + i*(block_size * 2 + 10), 0, false, NULL );
            }
            DrawNextTetri(state);
            DrawStats(state);
            #ifdef DEBUG
                DrawFPS(10, 10);
            #endif

            bool gamepad_on = IsGamepadAvailable(GAMEPAD);
            // handle gamepad menu navigation
            if (state->playstate == STATE_MENU || state->playstate == STATE_GAME_OVER) {
                if (gamepad_on && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
                    PlaySound(state->move_horizontal_sound);
                    state->gamepad_hilighted_btn++;
                    if (state->gamepad_hilighted_btn > BTN_QUIT) {
                        state->gamepad_hilighted_btn = BTN_PLAY;
                    } 
                } else if (gamepad_on && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
                    PlaySound(state->move_horizontal_sound);
                    state->gamepad_hilighted_btn--;
                    if (state->gamepad_hilighted_btn < BTN_PLAY) {
                        state->gamepad_hilighted_btn = BTN_QUIT;
                    } 
                }
            } else if (state->playstate == STATE_PAUSED) {
                if (gamepad_on && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
                    PlaySound(state->move_horizontal_sound);
                    state->gamepad_hilighted_btn++;
                    if (state->gamepad_hilighted_btn > BTN_QUIT) {
                        state->gamepad_hilighted_btn = BTN_PAUSE;
                    } 
                } else if (gamepad_on && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
                    PlaySound(state->move_horizontal_sound);
                    state->gamepad_hilighted_btn--;
                    if (state->gamepad_hilighted_btn < BTN_PAUSE) {
                        state->gamepad_hilighted_btn = BTN_QUIT;
                    } 
                }
            }
            // handle game play
            if (state->playstate == STATE_PLAYING ||
                state->playstate == STATE_PAUSED) {
                DrawTetrimino(state->preview_block.id, 
                              game_board_x + state->preview_block.x*block_size, 
                              game_board_y + state->preview_block.y*block_size, 
                              state->preview_block.rot,
                              true,
                              &(Rectangle){game_board_x, game_board_y, game_board_w, game_board_h});
                DrawTetrimino(state->active_block.id, 
                              game_board_x + state->active_block.x*block_size, 
                              game_board_y + state->active_block.y*block_size, 
                              state->active_block.rot,
                              false,
                              &(Rectangle){game_board_x, game_board_y, game_board_w, game_board_h});
                if (state->playstate == STATE_PAUSED) {
                    DrawRectangle(0, 0, window_width, window_height, (Color){0, 0, 0, 100});
                    char* text = "PAUSED";
                    int text_size = (int)(TEXT_SIZE_RATIO * (float)window_height);
                    int text_w = MeasureText(text, text_size);
                    DrawText("PAUSED", window_width/2 - text_w/2, window_height/2 - text_size/2, text_size, WHITE);

                } else { // playing
                    if (IsKeyPressed(KEY_LEFT) || 
                        (gamepad_on && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
                    ) {
                        if (TryMoveBlock(state, -1, 0, 0, &state->active_block)) {
                            PlaySound(state->move_horizontal_sound);
                            SetPreviewBlock(state, state->active_block.id);
                        };
                    }
                    else if (IsKeyPressed(KEY_RIGHT) || 
                        (gamepad_on && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
                    ) {
                        if (TryMoveBlock(state, 1, 0, 0, &state->active_block)) {
                            PlaySound(state->move_horizontal_sound);
                            SetPreviewBlock(state, state->active_block.id);
                        }
                    }
                    else if (IsKeyPressed(KEY_UP) || 
                        (gamepad_on && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
                    ) {
                        if (TryMoveBlock(state, 0, 0, 1, &state->active_block)) {
                            PlaySound(state->rotate_sound);
                            SetPreviewBlock(state, state->active_block.id);
                        }
                    }
                    else if (IsKeyPressed(KEY_DOWN) ||
                        (gamepad_on && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
                    ) {
                        if (TryMoveBlock(state, 0, 0, -1, &state->active_block)) {
                            PlaySound(state->rotate_sound);
                            SetPreviewBlock(state, state->active_block.id);
                        }
                    }
                    else if (IsKeyPressed(KEY_SPACE) || 
                        (gamepad_on && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
                    ) {
                        while (TryMoveBlock(state, 0, 1, 0, &state->active_block)) {}
                        RunEndTurn(state);
                    }
                    float time_step = TIME_STEP;
                    if (IsKeyDown(KEY_F) || 
                        (gamepad_on && IsGamepadButtonDown(GAMEPAD, GAMEPAD_BUTTON_LEFT_TRIGGER_2)) ||
                        (gamepad_on && IsGamepadButtonDown(GAMEPAD, GAMEPAD_BUTTON_RIGHT_TRIGGER_2))
                    ) {
                        time_step = TIME_STEP_FAST;
                    }
                    if (GetTime() - state->last_time > time_step) {
                        state->last_time = GetTime();
                        bool moved = TryMoveBlock(state, 0, 1, 0, &state->active_block);
                        if (!moved) {
                            RunEndTurn(state);
                        } else {
                            PlaySound(state->move_down_sound);
                        }
                    }
                }
            } else if (state->playstate == STATE_MENU) {
                if (GetTime() - state->last_time > TIME_STEP) {
                    state->last_time = GetTime();
                    state->next_block_id = GetRandomValue(0, 6);
                }
            }

            // put these after so starting the game doesn't immediately drop a block
            QuitButton(state);
            PlayButton(state);
            PauseButton(state);
        }    
        EndDrawing();
    
        // quit if button pressed
        if (IsKeyPressed(KEY_Q)) {
            printf("Quitting...\n");
            state->playstate = STATE_QUITTING;
            break;
        }
        #ifdef DEBUG
            if (IsKeyPressed(KEY_R)) {
                PlaySound(state->reload_sound);
                printf("Reloading...\n");
                return state;
            }
        #endif
        if (state->playstate == STATE_QUITTING) {
            return state;
        } 
    }

    CloseWindow();
    DeinitSound(state);
    state->playstate = STATE_QUITTING;

    return state;
}
