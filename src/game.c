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
#define BUTTON_HEIGHT 50

#define BLOCK_SIZE 24

#define GAME_BOARD_X (WINDOW_WIDTH/2 - BLOCK_COUNT_X*BLOCK_SIZE/2)
#define GAME_BOARD_Y 80
#define GAME_BOARD_W (BLOCK_COUNT_X*BLOCK_SIZE)
#define GAME_BOARD_H (BLOCK_COUNT_Y*BLOCK_SIZE)

#define TIME_STEP 0.5f

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
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            printf("Quitting...\n");
            state->playstate = STATE_QUITTING;
        }
    }
    DrawRectangle(button_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, color);
    DrawText(text, button_x + BUTTON_WIDTH/2 - text_w/2, 
             button_y + BUTTON_SIZE/2 + 5, BUTTON_SIZE, BLACK);
}

void PlayButton(struct GameState* state) {
    int button_x = WINDOW_WIDTH - 100 - 50;
    int button_y = WINDOW_HEIGHT - 86 - 80;
    char* text = "PLAY";
    char* restart = "REPLAY";
    if (state->playstate != STATE_MENU) {
        text = restart;
    }
    int text_w = MeasureText(text, BUTTON_SIZE);
    Color color = LIGHTGRAY;
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){button_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT})) {
        color = SKYBLUE;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            GameInit(state);
            state->playstate = STATE_PLAYING;
        }
    }
    DrawRectangle(button_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT, color);
    DrawText(text, button_x + BUTTON_WIDTH/2 - text_w/2, 
             button_y + BUTTON_SIZE/2 + 5, BUTTON_SIZE, BLACK);
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
    state->rows_cleared += ClearFullRows(state);
    GetNewActiveBlock(state);
    if (!TryMoveBlock(state, 0, 0, 0, &state->active_block)) {
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
        InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tetris");
    } else {
      // run on reload
    }

    while (!WindowShouldClose()) {
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
            DrawStats(state);
            #ifdef DEBUG
                DrawFPS(10, 10);
            #endif

            if (state->playstate == STATE_PLAYING) {
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
                if (IsKeyPressed(KEY_LEFT)) {
                    TryMoveBlock(state, -1, 0, 0, &state->active_block);
                    SetPreviewBlock(state, state->active_block.id);
                }
                else if (IsKeyPressed(KEY_RIGHT)) {
                    TryMoveBlock(state, 1, 0, 0, &state->active_block);
                    SetPreviewBlock(state, state->active_block.id);
                }
                else if (IsKeyPressed(KEY_UP)) {
                    TryMoveBlock(state, 0, 0, 1, &state->active_block);
                    SetPreviewBlock(state, state->active_block.id);
                }
                else if (IsKeyPressed(KEY_DOWN)) {
                    TryMoveBlock(state, 0, 0, -1, &state->active_block);
                    SetPreviewBlock(state, state->active_block.id);
                }
                else if (IsKeyPressed(KEY_SPACE)) {
                    while (TryMoveBlock(state, 0, 1, 0, &state->active_block)) {}
                    RunEndTurn(state);
                }
                if (GetTime() - state->last_time > TIME_STEP) {
                    state->last_time = GetTime();
                    bool moved = TryMoveBlock(state, 0, 1, 0, &state->active_block);
                    if (!moved) {
                        RunEndTurn(state);
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
            return state;
        }
        #ifdef DEBUG
            if (IsKeyPressed(KEY_R)) {
                printf("Reloading...\n");
                return state;
            }
        #endif
        if (state->playstate == STATE_QUITTING) {
            return state;
        } 
    }

    CloseWindow();
    state->playstate = STATE_QUITTING;

    return state;
}
