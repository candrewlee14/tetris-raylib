#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "raylib.h"
#include "game.h"
#include "tetr.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define TITLE_SIZE 30
#define BUTTON_SIZE 20

#define BUTTON_WIDTH 120
#define BUTTON_HEIGHT 40
#define BUTTON_SPACING 15

#define BLOCK_SIZE 24

#define GAME_BOARD_X (WINDOW_WIDTH/2 - BLOCK_COUNT_X*BLOCK_SIZE/2)
#define GAME_BOARD_Y 80
#define GAME_BOARD_W (BLOCK_COUNT_X*BLOCK_SIZE)
#define GAME_BOARD_H (BLOCK_COUNT_Y*BLOCK_SIZE)

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
    GetNewActiveBlock(state);
    for (int i = 0; i < BLOCK_COUNT_Y + EXTRA_ABOVE; i++) {
        for (int j = 0; j < BLOCK_COUNT_X; j++) {
            state->board[i][j] = 0;
        }
    }
}


void DrawBlock(int x, int y, Color color) {
    DrawRectangle(x + 1, y + 1, BLOCK_SIZE - 2, BLOCK_SIZE - 2, color);
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
    for (int yb = 0; yb < size; yb++) {
        for (int xb = 0; xb < size; xb++) {
            if (tetr[pos_rot*size*size + yb*size + xb] > 0) {
                if (mask != NULL) {
                    if (!CheckCollisionPointRec((Vector2){x + xb*BLOCK_SIZE, y + yb*BLOCK_SIZE}, *mask)) {
                        continue;
                    }
                }
                DrawBlock(x + xb*BLOCK_SIZE, y + yb*BLOCK_SIZE, color);
                if (preview) {
                    DrawRectangle(x + xb*BLOCK_SIZE + 2, y + yb*BLOCK_SIZE + 2, 
                                  BLOCK_SIZE - 4, BLOCK_SIZE - 4, BLACK);
                }
            }
        }
    }
}

void DrawGameBoard(int x, int y, int w, int h, int border_size, struct GameState* state) {
    DrawRectangle(x - border_size, y - border_size, 
                  w + border_size*2, h + border_size*2, LIGHTGRAY);
    DrawRectangle(x, y, w, h, BLACK);
    for (int i = 0; i < BLOCK_COUNT_Y; i++) {
        for (int j = 0; j < BLOCK_COUNT_X; j++) {
            if (state->board[i + EXTRA_ABOVE][j] > 0) {
                DrawBlock(x + j*BLOCK_SIZE, y + i*BLOCK_SIZE, 
                          ColorFromId(state->board[i + EXTRA_ABOVE][j] - 1));
            }
        }
    }
}

void QuitButton(struct GameState* state) {
    int button_x = WINDOW_WIDTH - 100 - 50;
    int button_y = WINDOW_HEIGHT - 86;
    char* text = "QUIT";
    int text_w = MeasureText(text, BUTTON_SIZE);
    Color color = LIGHTGRAY;
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){button_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT})) {
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
    DrawRectangle(button_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, color);
    DrawText(text, button_x + BUTTON_WIDTH/2 - text_w/2, 
             button_y + BUTTON_HEIGHT/2 - BUTTON_SIZE/2 + 2, BUTTON_SIZE, BLACK);
}

void PlayButton(struct GameState* state) {
    int button_x = WINDOW_WIDTH - 100 - 50;
    int button_y = WINDOW_HEIGHT - 86 - (BUTTON_HEIGHT + BUTTON_SPACING);
    char* text = "PLAY";
    char* restart = "RESTART";
    if (state->playstate != STATE_MENU) {
        text = restart;
    }
    int text_w = MeasureText(text, BUTTON_SIZE);
    Color color = LIGHTGRAY;
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){button_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT})) {
        if (!state->is_hovering_play_button) {
            PlaySound(state->hover_button_sound);
        }
        state->is_hovering_play_button = true;
        color = SKYBLUE;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            GameInit(state);
            state->playstate = STATE_PLAYING;
            PlaySound(state->play_button_sound);
        }
    } else {
        state->is_hovering_play_button = false;
    }
    DrawRectangle(button_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, color);
    DrawText(text, button_x + BUTTON_WIDTH/2 - text_w/2, 
             button_y + BUTTON_HEIGHT/2 - BUTTON_SIZE/2 + 2, BUTTON_SIZE, BLACK);
}

void PauseButton(struct GameState* state) {
    if (state->playstate == STATE_MENU || state->playstate == STATE_GAME_OVER) {
        return;
    }
    int button_x = WINDOW_WIDTH - 100 - 50;
    int button_y = WINDOW_HEIGHT - 86 - (BUTTON_HEIGHT + BUTTON_SPACING) * 2;
    char* text = "PAUSE";
    if (state->playstate == STATE_PAUSED) {
        text = "RESUME";
    }
    int text_w = MeasureText(text, BUTTON_SIZE);
    Color color = LIGHTGRAY;
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){button_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT})) {
        if (!state->is_hovering_pause_button) {
            PlaySound(state->hover_button_sound);
        }
        state->is_hovering_pause_button = true;
        color = PURPLE;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (state->playstate == STATE_PAUSED) {
                state->playstate = STATE_PLAYING;
            } else {
                state->playstate = STATE_PAUSED;
            }
        }
    } else {
        state->is_hovering_pause_button = false;
    }
    if (IsKeyPressed(KEY_P) || 
        (IsGamepadAvailable(GAMEPAD) && IsGamepadButtonPressed(GAMEPAD, GAMEPAD_BUTTON_MIDDLE_RIGHT))
    ) {
        if (state->playstate == STATE_PAUSED) {
            state->playstate = STATE_PLAYING;
        } else {
            state->playstate = STATE_PAUSED;
        }
    }
    DrawRectangle(button_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, color);
    DrawText(text, button_x + BUTTON_WIDTH/2 - text_w/2, 
             button_y + BUTTON_HEIGHT/2 - BUTTON_SIZE/2 + 2, BUTTON_SIZE, BLACK);
}

void DrawStats(struct GameState* state) {
    int STATS_Y = 250;
    int TEXT_SIZE = 20;
    {

        char* text = "TURN:";
        DrawText(text, WINDOW_WIDTH - 150, STATS_Y, TEXT_SIZE, BLACK);
        const char* num_txt =  TextFormat("%i", state->turn + 1);
        int text_w = MeasureText(num_txt, TEXT_SIZE);
        DrawText(num_txt, WINDOW_WIDTH - 50 - text_w, STATS_Y + TEXT_SIZE + 5, 20, BLACK);
    }
    {
        char* text = "CLEARED:";
        int text_y = STATS_Y + 3 * TEXT_SIZE;
        const char* num_txt =  TextFormat("%i", state->rows_cleared);
        DrawText(text, WINDOW_WIDTH - 150, text_y, 20, BLACK);
        int text_w = MeasureText(num_txt, 20);
        DrawText(num_txt, WINDOW_WIDTH - 50 - text_w, text_y + TEXT_SIZE + 5, TEXT_SIZE, BLACK);
    }
}

void DrawNextTetri(struct GameState* state) {
    int x = WINDOW_WIDTH - 140;
    int y = 100;
    int w = 100;
    int h = 100;
    int text_w = MeasureText("NEXT", 20);
    DrawText("NEXT", x + w/2 - text_w/2, y - 30, 20, BLACK);
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
    x += w/2 - size*BLOCK_SIZE/2;
    y += h/2 - start_y*BLOCK_SIZE/2 - true_size_y*BLOCK_SIZE/2;
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

void* module_main(void* data) {
    struct GameState* state = (struct GameState*)data;
    if (state == NULL) {
        state = (struct GameState*)malloc(sizeof(struct GameState));
        GameInit(state);
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
        SetTargetFPS(60);
        InitSound(state);
        InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tetris");

        state->time_song_started = GetTime();
        PlayMusicStream(state->songs[state->current_song % SONG_COUNT]);
    } else {
      // run on reload
    }

    while (!WindowShouldClose()) {
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
                int text_w = MeasureText(text, TITLE_SIZE);
                DrawText(text, WINDOW_WIDTH/2 - text_w/2, 24, TITLE_SIZE, color);
            }
            DrawGameBoard(GAME_BOARD_X, GAME_BOARD_Y, GAME_BOARD_W, GAME_BOARD_H, 5, state);

            for (int i = 0; i < 7; i++) {
                DrawTetrimino(i, 50, 10 + i*80, 0, false, NULL );
            }
            DrawNextTetri(state);
            QuitButton(state);
            PlayButton(state);
            PauseButton(state);
            DrawStats(state);
            #ifdef DEBUG
                DrawFPS(10, 10);
            #endif

            if (state->playstate == STATE_PLAYING ||
                state->playstate == STATE_PAUSED) {
                DrawTetrimino(state->preview_block.id, 
                              GAME_BOARD_X + state->preview_block.x*BLOCK_SIZE, 
                              GAME_BOARD_Y + state->preview_block.y*BLOCK_SIZE, 
                              state->preview_block.rot,
                              true,
                              &(Rectangle){GAME_BOARD_X, GAME_BOARD_Y, GAME_BOARD_W, GAME_BOARD_H});
                DrawTetrimino(state->active_block.id, 
                              GAME_BOARD_X + state->active_block.x*BLOCK_SIZE, 
                              GAME_BOARD_Y + state->active_block.y*BLOCK_SIZE, 
                              state->active_block.rot,
                              false,
                              &(Rectangle){GAME_BOARD_X, GAME_BOARD_Y, GAME_BOARD_W, GAME_BOARD_H});
                if (state->playstate == STATE_PAUSED) {
                    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 100});
                    DrawText("PAUSED", WINDOW_WIDTH/2 - 50, WINDOW_HEIGHT/2 - 10, 20, WHITE);
                } else { // playing
                    bool gamepad_on = IsGamepadAvailable(GAMEPAD);
                    DrawText(TextFormat("GP%d: %s", GAMEPAD, GetGamepadName(GAMEPAD)), 10, 10, 10, BLACK);
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
