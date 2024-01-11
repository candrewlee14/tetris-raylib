
#define TETR_1_COLOR SKYBLUE
#define TETR_1_SIZE 4
char tetrimino_1[4][TETR_1_SIZE][TETR_1_SIZE] = {
    {{0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0}},
    {{0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 0}},
    {{0, 0, 0, 0},
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0}},
    {{0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0}}
};

#define TETR_2_COLOR BLUE
#define TETR_2_SIZE 3
char tetrimino_2[4][TETR_2_SIZE][TETR_2_SIZE] = {
    {{2, 0, 0},
    {2, 2, 2},
    {0, 0, 0}},
    {{0, 2, 2},
    {0, 2, 0},
    {0, 2, 0}},
    {{0, 0, 0},
    {2, 2, 2},
    {0, 0, 2}},
    {{0, 2, 0},
    {0, 2, 0},
    {2, 2, 0}}
};

#define TETR_3_COLOR ORANGE
#define TETR_3_SIZE 3
char tetrimino_3[4][TETR_3_SIZE][TETR_3_SIZE] = {
    {{0, 0, 3},
    {3, 3, 3},
    {0, 0, 0}},
    {{0, 3, 0},
    {0, 3, 0},
    {0, 3, 3}},
    {{0, 0, 0},
    {3, 3, 3},
    {3, 0, 0}},
    {{3, 3, 0},
    {0, 3, 0},
    {0, 3, 0}}
};

#define TETR_4_COLOR YELLOW
#define TETR_4_SIZE 2
char tetrimino_4[4][TETR_4_SIZE][TETR_4_SIZE] = {
    {{4, 4},
    {4, 4}},
    {{4, 4},
    {4, 4}},
    {{4, 4},
    {4, 4}},
    {{4, 4},
    {4, 4}}
};

#define TETR_5_COLOR GREEN
#define TETR_5_SIZE 3
char tetrimino_5[4][TETR_5_SIZE][TETR_5_SIZE] = {
    {{0, 5, 5},
    {5, 5, 0},
    {0, 0, 0}},
    {{0, 5, 0},
    {0, 5, 5},
    {0, 0, 5}},
    {{0, 0, 0},
    {0, 5, 5},
    {5, 5, 0}},
    {{5, 0, 0},
    {5, 5, 0},
    {0, 5, 0}}
};

#define TETR_6_COLOR PURPLE
#define TETR_6_SIZE 3
char tetrimino_6[4][TETR_6_SIZE][TETR_6_SIZE] = {
    {{0, 6, 0},
    {6, 6, 6},
    {0, 0, 0}},
    {{0, 6, 0},
    {0, 6, 6},
    {0, 6, 0}},
    {{0, 0, 0},
    {6, 6, 6},
    {0, 6, 0}},
    {{0, 6, 0},
    {6, 6, 0},
    {0, 6, 0}}
};

#define TETR_7_COLOR RED
#define TETR_7_SIZE 3
char tetrimino_7[4][TETR_7_SIZE][TETR_7_SIZE] = {
    {{7, 7, 0},
    {0, 7, 7},
    {0, 0, 0}},
    {{0, 0, 7},
    {0, 7, 7},
    {0, 7, 0}},
    {{0, 0, 0},
    {7, 7, 0},
    {0, 7, 7}},
    {{0, 7, 0},
    {7, 7, 0},
    {7, 0, 0}}
};

struct TetriminoInfo {
    int size;
    char* data;
};

struct TetriminoInfo GetTetriminoInfo(int id) {
    struct TetriminoInfo info;
    switch (id % 7) {
        case 0: 
            info.size = TETR_1_SIZE;
            info.data = &tetrimino_1[0][0][0];
            break;
        case 1:
            info.size = TETR_2_SIZE;
            info.data = &tetrimino_2[0][0][0];
            break;
        case 2:
            info.size = TETR_3_SIZE;
            info.data = &tetrimino_3[0][0][0];
            break;
        case 3:
            info.size = TETR_4_SIZE;
            info.data = &tetrimino_4[0][0][0];
            break;
        case 4:
            info.size = TETR_5_SIZE;
            info.data = &tetrimino_5[0][0][0];
            break;
        case 5:
            info.size = TETR_6_SIZE;
            info.data = &tetrimino_6[0][0][0];
            break;
        case 6:
            info.size = TETR_7_SIZE;
            info.data = &tetrimino_7[0][0][0];
            break;
        default: break;
    }
    return info;
}

Color ColorFromId(int id) {
    switch (id) {
        case 0: return TETR_1_COLOR; break;
        case 1: return TETR_2_COLOR; break;
        case 2: return TETR_3_COLOR; break;
        case 3: return TETR_4_COLOR; break;
        case 4: return TETR_5_COLOR; break;
        case 5: return TETR_6_COLOR; break;
        case 6: return TETR_7_COLOR; break;
        default: return BLACK; break;
    }
}
