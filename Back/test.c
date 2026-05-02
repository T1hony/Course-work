#include <stdio.h>
#include "game_logic.h"

int main() {
    init_game();
    printf("Game initialized!\n");

    make_move(0, 0, CELL_X);
    make_move(0, 1, CELL_O);
    make_move(1, 0, CELL_X);

    if (save_game("test_save.txt")) {
        printf("Game saved successfully!\n");
    }

    cleanup_game();
    return 0;
}