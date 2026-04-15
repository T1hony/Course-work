#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#define CELL_EMPTY 0
#define CELL_X 1
#define CELL_O 2

#define GAME_CONTINUES 0
#define WIN_X 1
#define WIN_O 2

#define MOVE_SUCCESS 1
#define MOVE_FAIL 0

typedef struct figure_desk {
    int x;
    int y;
    int cell;
    struct figure_desk *next;
} figure_desk;

typedef struct{
    int x;
    int y;
}Move;

void init_game();

void cleanup_game();

int make_move(int x, int y, int player);

int get_cell_state(int x, int y);

const figure_desk* get_board_head();

int check_win(int last_x, int last_y);

Move bot_make_move(int bot_symbol);

#endif 