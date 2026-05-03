#include "gui.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>

#define CELL 40

static TTF_Font* font = NULL;
static SDL_Renderer* r;
static int cam_x = 0, cam_y = 0;

static int bot_x = 999999;
static int bot_y = 999999;

void gui_init(SDL_Renderer* renderer) {
    r = renderer;
    if (!TTF_Init()) {
        printf("Ошибка TTF_Init: %s\n", SDL_GetError());
    }
    font = TTF_OpenFont("arial.ttf", 24); 
    if (!font) {
        printf("Шрифт не найден!\n");
    }
}

void draw_text(const char* text, int x, int y, SDL_Color color) {
    if (!font || !text) return; 
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, 0, color); 
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FRect dst = { (float)x, (float)y, (float)surf->w, (float)surf->h };
    SDL_RenderTexture(r, tex, NULL, &dst);
    SDL_DestroySurface(surf);
    SDL_DestroyTexture(tex);
}

void draw_circle(SDL_Renderer* renderer, int center_x, int center_y, int radius) {
    int x = radius - 1, y = 0, tx = 1, ty = 1, error = tx - (radius * 2);
    while (x >= y) {
        SDL_RenderPoint(renderer, center_x + x, center_y - y);
        SDL_RenderPoint(renderer, center_x + x, center_y + y);
        SDL_RenderPoint(renderer, center_x - x, center_y - y);
        SDL_RenderPoint(renderer, center_x - x, center_y + y);
        SDL_RenderPoint(renderer, center_x + y, center_y - x);
        SDL_RenderPoint(renderer, center_x + y, center_y + x);
        SDL_RenderPoint(renderer, center_x - y, center_y - x);
        SDL_RenderPoint(renderer, center_x - y, center_y + x);
        if (error <= 0) { ++y; error += ty; ty += 2; }
        if (error > 0) { --x; tx += 2; error += tx - (radius * 2); }
    }
}

void gui_handle_event(SDL_Event* e, GuiState* state, GameMode* mode, BotDifficulty* diff) {
    if (e->type == SDL_EVENT_KEY_DOWN) {
        switch (e->key.key) {
            case SDLK_W: cam_y += 20; break;
            case SDLK_S: cam_y -= 20; break;
            case SDLK_A: cam_x += 20; break;
            case SDLK_D: cam_x -= 20; break;
            case SDLK_ESCAPE: if (*state != GUI_WIN_SCREEN) *state = GUI_MENU; break;
        }
    }
    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        int x = (int)e->button.x, y = (int)e->button.y;
        if (*state == GUI_MENU) {
            if (x > 300 && x < 500) {
                if (y > 200 && y < 250) *state = GUI_SETTINGS;
                else if (y > 300 && y < 350) *state = GUI_HELP;
                else if (y > 400 && y < 450) *state = GUI_ABOUT;
            }
        } else if (*state == GUI_SETTINGS) {
            if (y > 150 && y < 200) {
                if (x > 100 && x < 250) *mode = MODE_PVP;
                if (x > 300 && x < 450) *mode = MODE_PVE;
                if (x > 500 && x < 650) *mode = MODE_EVE;
            }
            if (*mode != MODE_PVP && y > 260 && y < 310) {
                if (x > 100 && x < 250) *diff = BOT_EASY;
                if (x > 300 && x < 450) *diff = BOT_HARD;
                if (x > 500 && x < 650) *diff = BOT_IMPOSSIBLE;
            }
            int start_y = (*mode == MODE_PVP) ? 300 : 400;
            if (y > start_y && y < start_y + 60 && x > 300 && x < 500) {
                init_game();
                cam_x = 400 - (CELL / 2); 
                cam_y = 300 - (CELL / 2);
                *state = GUI_GAME;
            }
        }
    }
}

Move gui_get_cell_from_mouse(int mx, int my) {
    Move m = { (mx - cam_x) / CELL, (my - cam_y) / CELL };
    return m;
}

void gui_set_bot_move(int x, int y) { bot_x = x; bot_y = y; }

static void draw_grid() {
    SDL_SetRenderDrawColor(r, 100, 100, 100, 255);
    for (int x = -800; x < 1600; x += CELL) {
        int sx = x + (cam_x % CELL);
        SDL_RenderLine(r, (float)sx, 0, (float)sx, 600);
    }
    for (int y = -600; y < 1200; y += CELL) {
        int sy = y + (cam_y % CELL);
        SDL_RenderLine(r, 0, (float)sy, 800, (float)sy);
    }
}

static void draw_marks() {
    const figure_desk* cur = get_board_head();
    while (cur) {
        int x = cur->x * CELL + cam_x, y = cur->y * CELL + cam_y;
        if (cur->x == bot_x && cur->y == bot_y) {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 255, 0, 0, 100);
            SDL_FRect rect = { (float)x, (float)y, (float)CELL, (float)CELL };
            SDL_RenderFillRect(r, &rect);
        }
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        if (cur->cell == CELL_X) {
            SDL_RenderLine(r, (float)x+5, (float)y+5, (float)x+CELL-5, (float)y+CELL-5);
            SDL_RenderLine(r, (float)x+CELL-5, (float)y+5, (float)x+5, (float)y+CELL-5);
        } else {
            draw_circle(r, x + CELL / 2, y + CELL / 2, 15);
            draw_circle(r, x + CELL / 2, y + CELL / 2, 14);
        }
        cur = cur->next;
    }
}

static void draw_menu() {
    SDL_Color w = {255, 255, 255, 255};
    SDL_FRect b[3] = {{300, 200, 200, 50}, {300, 300, 200, 50}, {300, 400, 200, 50}};
    char* t[] = {"НОВАЯ ИГРА", "СПРАВКА", "ОБ АВТОРЕ"};
    int tx[] = {335, 355, 345};
    for(int i=0; i<3; i++) {
        SDL_SetRenderDrawColor(r, 50, 50, 50, 255);
        SDL_RenderFillRect(r, &b[i]);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_RenderRect(r, &b[i]);
        draw_text(t[i], tx[i], 212 + i*100, w);
    }
}

static void draw_settings(GameMode mode, BotDifficulty diff) {
    SDL_Color white = {255, 255, 255, 255}, gray = {150, 150, 150, 255};
    draw_text("НАСТРОЙКИ ПАРТИИ", 280, 50, white);
    
    draw_text("Выберите режим:", 100, 110, white);
    char* modes[] = {"Игрок-Игрок", "Игрок-Бот", "Бот-Бот"};
    
    for(int i = 0; i < 3; i++) {
        SDL_FRect b = {100.0f + i * 200, 150, 160, 50};
        
        if (mode == i) {
            SDL_SetRenderDrawColor(r, 0, 150, 40, 255);
        } else {
            SDL_SetRenderDrawColor(r, 40, 40, 40, 255);
        }
        
        SDL_RenderFillRect(r, &b);
        draw_text(modes[i], 110 + i * 200, 162, white);
    }

    if (mode != MODE_PVP) {
        draw_text("Сложность бота:", 100, 220, white);
        
        SDL_FRect b_e = {100, 260, 160, 50};
        if (diff == BOT_EASY) {
            SDL_SetRenderDrawColor(r, 0, 150, 40, 255);
        } else {
            SDL_SetRenderDrawColor(r, 40, 40, 40, 255);
        }
        SDL_RenderFillRect(r, &b_e);
        draw_text("Легко", 145, 272, white);

        SDL_FRect b_m = {300, 260, 160, 50};
        if (diff == BOT_HARD) {
            SDL_SetRenderDrawColor(r, 0, 150, 40, 255);
        } else {
            SDL_SetRenderDrawColor(r, 40, 40, 40, 255);
        }
        SDL_RenderFillRect(r, &b_m);
        draw_text("Средне", 345, 272, white);

        SDL_FRect b_h = {500, 260, 160, 50};
        if (diff == BOT_IMPOSSIBLE) {
            SDL_SetRenderDrawColor(r, 0, 150, 40, 255);
        } else {
            SDL_SetRenderDrawColor(r, 40, 40, 40, 255);
        }
        SDL_RenderFillRect(r, &b_h);
        draw_text("Сложно", 540, 272, white);
    }

    int start_y = (mode == MODE_PVP) ? 300 : 400;
    SDL_FRect start = {300, (float)start_y, 200, 60};
    SDL_SetRenderDrawColor(r, 0, 180, 0, 255);
    SDL_RenderFillRect(r, &start);
    draw_text("ИГРАТЬ!", 355, start_y + 15, white);
    
    draw_text("Нажми ESC, чтобы вернуться", 240, 530, gray);
}

static void draw_help() {
    SDL_Color y = {255, 255, 0, 255};
    draw_text("ПРАВИЛА ИГРЫ", 320, 50, y);
    draw_text("- Соберите 5 фигур в ряд", 50, 120, y);
    draw_text("- WASD для камеры, ЛКМ для хода", 50, 160, y);
    draw_text("ESC - меню", 230, 500, y);
}

static void draw_about() {
    SDL_Color w = {255, 255, 255, 255};
    draw_text("О ПРОГРАММЕ", 330, 50, w);
    draw_text("Разработчик: Тимощенко Даниил Сергеевич", 50, 200, w);
    draw_text("Разработчик: Цацу Глеб Михалович", 50, 250, w);
    draw_text("Группа: 5131001/50603", 50, 300, w);
    draw_text("СПбПУ Петра Великого (Политех)", 50, 350, w);
    draw_text("Институт компбютерных наук и кибербезопасности(ИКНК)", 50, 400, w);
    draw_text(" Высшая школа кибербезопасности и защиты информации", 50, 450, w);
    draw_text("2026", 50, 500, w);
}

void gui_draw(GuiState state, GameStatus win_status, GameMode mode, BotDifficulty diff) {
    switch (state) {
        case GUI_MENU: draw_menu(); break;
        case GUI_SETTINGS: draw_settings(mode, diff); break;
        case GUI_HELP: draw_help(); break;
        case GUI_ABOUT: draw_about(); break;
        case GUI_GAME: draw_grid(); draw_marks(); break;
        case GUI_WIN_SCREEN:
            draw_grid(); draw_marks();
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
            SDL_FRect f = {0, 0, 800, 600};
            SDL_RenderFillRect(r, &f);
            SDL_Color gold = {255, 215, 0, 255};
            draw_text(win_status == WIN_X ? "ПОБЕДИЛИ КРЕСТИКИ!" : "ПОБЕДИЛИ НОЛИКИ!", 260, 250, gold);
            draw_text("ESC - в меню", 335, 320, gold);
            break;
    }
}