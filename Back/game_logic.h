#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

typedef enum {
    CELL_EMPTY = 0,
    CELL_X = 1,
    CELL_O = 2
} CellState;

typedef enum {
    GAME_CONTINUES = 0,
    WIN_X = 1,
    WIN_O = 2
} GameStatus;

typedef enum {
    MOVE_FAIL = 0,
    MOVE_SUCCESS = 1
} MoveResult;

typedef enum {
    BOT_EASY = 1,
    BOT_HARD = 2,
    BOT_IMPOSSIBLE = 3
} BotDifficulty;

typedef enum {
    PLAYER_HUMAN = 0,
    PLAYER_BOT = 1
} PlayerType;

typedef struct {
    PlayerType type;
    CellState symbol;
    
    union {
        char nickname[32];
        BotDifficulty difficulty;
    } info;
} PlayerInfo;

typedef struct figure_desk {
    int x;
    int y;
    CellState cell;
    struct figure_desk *next;
} figure_desk;

typedef struct {
    int x;
    int y;
} Move;


void init_game();
void cleanup_game();

MoveResult make_move(int x, int y, CellState player);
CellState get_cell_state(int x, int y);
const figure_desk* get_board_head();
GameStatus check_win(int last_x, int last_y);

Move bot_make_move(CellState bot_symbol, BotDifficulty difficulty);

int save_game(const char* filename);
int load_game(const char* filename);

#endif