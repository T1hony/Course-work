#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include "game_logic.h"

typedef enum {
    GUI_MENU,
    GUI_GAME,
    GUI_HELP,
    GUI_ABOUT,
    GUI_WIN_SCREEN,
    GUI_SETTINGS
} GuiState;

typedef enum {
    MODE_PVP,
    MODE_PVE,
    MODE_EVE
} GameMode;

void gui_init(SDL_Renderer* renderer);

void gui_handle_event(SDL_Event* e, GuiState* state, GameMode* mode, BotDifficulty* diff);

void gui_draw(GuiState state, GameStatus win_status, GameMode mode, BotDifficulty diff);

Move gui_get_cell_from_mouse(int mx, int my);

void gui_set_bot_move(int x, int y);

#endif