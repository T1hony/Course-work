#include "game_logic.h"
#include <stdlib.h> 
#include <stdio.h>
#include <stddef.h> 
#include <time.h>

static figure_desk* head = NULL;

static figure_desk* find_node(int x, int y) {
    figure_desk* current = head;
    while (current != NULL) {
        if (current->x == x && current->y == y) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static int count_in_direction(int start_x, int start_y, int dx, int dy, CellState player) {
    int count = 0;
    int cur_x = start_x + dx;
    int cur_y = start_y + dy;

    while (get_cell_state(cur_x, cur_y) == player) {
        count++;
        cur_x += dx;
        cur_y += dy;
    }
    return count;
}

void init_game() {
    cleanup_game();
    head = NULL;
    srand((unsigned int)time(NULL)); 
}

void cleanup_game() {
    figure_desk* current = head;
    while (current != NULL) {
        figure_desk* next_node = current->next;
        free(current);                          
        current = next_node;                    
    }
    head = NULL;
}

const figure_desk* get_board_head() {
    return head;
}

CellState get_cell_state(int x, int y) {
    figure_desk* node = find_node(x, y);
    if (node != NULL) {
        return node->cell;
    }
    return CELL_EMPTY;
}

MoveResult make_move(int x, int y, CellState player) {
    if (find_node(x, y) != NULL) {
        return MOVE_FAIL;
    }

    figure_desk* new_node = (figure_desk*)malloc(sizeof(figure_desk));
    if (new_node == NULL) {
        return MOVE_FAIL; 
    }

    new_node->x = x;
    new_node->y = y;
    new_node->cell = player;

    new_node->next = head;
    head = new_node;

    return MOVE_SUCCESS;
}

GameStatus check_win(int last_x, int last_y) {
    CellState player = get_cell_state(last_x, last_y);
    if (player == CELL_EMPTY) return GAME_CONTINUES;

    int directions[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };

    for (int i = 0; i < 4; i++) {
        int dx = directions[i][0];
        int dy = directions[i][1];

        int total = 1 + 
                    count_in_direction(last_x, last_y, dx, dy, player) + 
                    count_in_direction(last_x, last_y, -dx, -dy, player);

        if (total >= 5) {
            return (player == CELL_X) ? WIN_X : WIN_O;
        }
    }
    return GAME_CONTINUES;
}

int save_game(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) return 0;

    figure_desk* current = head;
    while (current != NULL) {
        fprintf(file, "%d %d %d\n", current->x, current->y, current->cell);
        current = current->next;
    }

    fclose(file);
    return 1;
}

int load_game(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        file = fopen(filename, "w");
        if (file != NULL) fclose(file);
        return 0; 
    }

    cleanup_game();

    int x, y, cell_val;
    while (fscanf(file, "%d %d %d", &x, &y, &cell_val) == 3) {
        figure_desk* new_node = (figure_desk*)malloc(sizeof(figure_desk));
        if (new_node != NULL) {
            new_node->x = x;
            new_node->y = y;
            new_node->cell = (CellState)cell_val;
            new_node->next = head;
            head = new_node;
        }
    }

    fclose(file);
    return 1;
}

static int get_candidate_moves(Move candidates[], int max_candidates) {
    int count = 0;
    figure_desk* current = head;
    int neighbors[8][2] = {{-1,-1}, {0,-1}, {1,-1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

    while (current != NULL) {
        for (int i = 0; i < 8; i++) {
            int tx = current->x + neighbors[i][0];
            int ty = current->y + neighbors[i][1];

            if (get_cell_state(tx, ty) == CELL_EMPTY) {
                int already_in = 0;
                for (int j = 0; j < count; j++) {
                    if (candidates[j].x == tx && candidates[j].y == ty) { already_in = 1; break; }
                }
                if (!already_in && count < max_candidates) {
                    candidates[count].x = tx;
                    candidates[count].y = ty;
                    count++;
                }
            }
        }
        current = current->next;
    }
    return count;
}

static int evaluate_line(int x, int y, int dx, int dy, CellState player) {
    int count = 1;
    int cur_x = x + dx; int cur_y = y + dy;
    int space_fwd = 0, space_bwd = 0;
    
    while (get_cell_state(cur_x, cur_y) == player) { count++; cur_x += dx; cur_y += dy; }
    if (get_cell_state(cur_x, cur_y) == CELL_EMPTY) space_fwd = 1;

    cur_x = x - dx; cur_y = y - dy;
    while (get_cell_state(cur_x, cur_y) == player) { count++; cur_x -= dx; cur_y -= dy; }
    if (get_cell_state(cur_x, cur_y) == CELL_EMPTY) space_bwd = 1;

if (count >= 5) {
        return 100000;
    }

    if (count == 4) {
        if (space_fwd && space_bwd) {
            return 10000;
        } else {
            return 1000;
        }
    }

    if (count == 3) {
        if (space_fwd && space_bwd) {
            return 500;
        } else {
            return 50;
        }
    }

    if (count == 2) {
        if (space_fwd && space_bwd) {
            return 10;
        } else {
            return 0;
        }
    }
    return 0;
}

static int get_cell_score(int x, int y, CellState bot_symbol, BotDifficulty difficulty) {
    CellState human_symbol = (bot_symbol == CELL_X) ? CELL_O : CELL_X;
    int total_score = 0;
    int dirs[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

    for (int i = 0; i < 4; i++) {
        int atk = evaluate_line(x, y, dirs[i][0], dirs[i][1], bot_symbol);
        int def = evaluate_line(x, y, dirs[i][0], dirs[i][1], human_symbol);

        if (difficulty == BOT_HARD) total_score += atk + (int)(def * 0.8); 
        else if (difficulty == BOT_IMPOSSIBLE) total_score += atk + (int)(def * 1.2); 
    }
    return total_score;
}

Move bot_make_move(CellState bot_symbol, BotDifficulty difficulty) {
    Move result = {0, 0};
    if (head == NULL) return result;

    Move candidates[1000];
    int count = get_candidate_moves(candidates, 1000);
    if (count == 0) return result;

    if (difficulty == BOT_EASY) {
        return candidates[rand() % count];
    }

    int best_score = -1;
    Move best_move = candidates[0];

    for (int i = 0; i < count; i++) {
        int score = get_cell_score(candidates[i].x, candidates[i].y, bot_symbol, difficulty);
        if (score > best_score || (score == best_score && (rand() % 2 == 0))) {
            best_score = score;
            best_move = candidates[i];
        }
    }
    return best_move;
}