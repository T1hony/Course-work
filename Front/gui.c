#include "gui.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define CELL 40

static TTF_Font* font = NULL;
static SDL_Renderer* r;
static int cam_x = 0, cam_y = 0;

static int bot_x = 999999;
static int bot_y = 999999;

static char player1_name[32] = "Игрок 1";
static char player2_name[32] = "Игрок 2";
static int entering_first = 1;

static SDL_Window* win_ptr = NULL;

int get_text_width(const char* text) {
    if (!font || !text)  return 0;
    int w, h;
    TTF_GetStringSize(font, text, 0, &w, &h);
    return w;
}

void gui_init(SDL_Window* window, SDL_Renderer* renderer) {
    win_ptr = window;
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

void gui_handle_event(SDL_Event* e, GuiState* state, GameMode* mode, BotDifficulty* diff, int* bot_starts) {
    if (*state == GUI_NAME_INPUT) {
        if (e->type == SDL_EVENT_TEXT_INPUT) {
            char* target = entering_first ? player1_name : player2_name;
            if (strlen(target) < 31) strcat(target, e->text.text);
        }
        if (e->type == SDL_EVENT_KEY_DOWN) {
            char* target = entering_first ? player1_name : player2_name;
            if (e->key.key == SDLK_BACKSPACE && strlen(target) > 0) {
                target[strlen(target) - 1] = '\0';
            }
            if (e->key.key == SDLK_RETURN) { 
                if (entering_first) {
                    entering_first = 0; 
                } else {
                    init_game();
                    cam_x = 400 - (CELL / 2); 
                    cam_y = 300 - (CELL / 2);
                    *state = GUI_GAME;
                    SDL_StopTextInput(win_ptr);
                }
            }
        }
        return;
    }

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

            if (*mode == MODE_PVE && y > 370 && y < 420) {
                if (x > 100 && x < 260) *bot_starts = 0;
                if (x > 300 && x < 460) *bot_starts = 1;
            }
            
            int start_y = (*mode == MODE_PVP) ? 300 : ((*mode == MODE_PVE) ? 450 : 400);
            if (y > start_y && y < start_y + 60 && x > 300 && x < 500) {
                if (*mode == MODE_PVP) {
                    *state = GUI_NAME_INPUT;
                    entering_first = 1;
                    player1_name[0] = '\0';
                    player2_name[0] = '\0';
                    SDL_StartTextInput(win_ptr);
                } 
                else {
                    if (*mode == MODE_EVE) {
                        gui_set_player_names("Бот (X)", "Бот (O)");
                    }
                    init_game();
                    cam_x = 400 - (CELL / 2); 
                    cam_y = 300 - (CELL / 2);
                    *state = GUI_GAME;
                }
            }
        }
    }
}

Move gui_get_cell_from_mouse(int mx, int my) {
    Move m;
    m.x = (int)floor((double)(mx-cam_x) / CELL);
    m.y = (int)floor((double)(my-cam_y) / CELL);
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

    for(int i = 0; i < 3; i++) {
        SDL_SetRenderDrawColor(r, 50, 50, 50, 255);
        SDL_RenderFillRect(r, &b[i]);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_RenderRect(r, &b[i]);

        int text_w = get_text_width(t[i]);
        int centered_x = (int)b[i].x + ((int)b[i].w / 2) - (text_w / 2);
        
        draw_text(t[i], centered_x, 212 + i * 100, w);
    }
}

static void draw_settings(GameMode mode, BotDifficulty diff, int bot_starts) {
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
        if (mode == MODE_PVE) {
            draw_text("Кто ходит первым?", 100, 330, white);
            
            SDL_FRect b_h = {100, 370, 160, 50};
            SDL_SetRenderDrawColor(r, bot_starts == 0 ? 0 : 40, bot_starts == 0 ? 150 : 40, 40, 255);
            SDL_RenderFillRect(r, &b_h);
            draw_text("Игрок (X)", 125, 382, white);

            SDL_FRect b_b = {300, 370, 160, 50};
            SDL_SetRenderDrawColor(r, bot_starts == 1 ? 0 : 40, bot_starts == 1 ? 150 : 40, 40, 255);
            SDL_RenderFillRect(r, &b_b);
            draw_text("Бот (X)", 335, 382, white);
        }
    }

    int start_y = (mode == MODE_PVP) ? 300 : ((mode == MODE_PVE) ? 450 : 400);    SDL_FRect start = {300, (float)start_y, 200, 60};
    SDL_SetRenderDrawColor(r, 0, 180, 0, 255);
    SDL_RenderFillRect(r, &start);
    draw_text("ИГРАТЬ!", 355, start_y + 15, white);
    
    const char* esc_msg = "Нажми ESC для возврата в меню";
    int esc_x = 400 - (get_text_width(esc_msg) / 2);
    draw_text(esc_msg, esc_x, 530, gray);
}

static void draw_help() {
    SDL_Color y = {255, 255, 0, 255};
    draw_text("ПРАВИЛА ИГРЫ", 320, 50, y);
    draw_text("- Соберите 5 фигур в ряд", 50, 120, y);
    draw_text("- WASD для камеры, ЛКМ для хода", 50, 160, y);
    const char* esc_msg = "Нажми ESC для возврата в меню";
    int esc_x = 400 - (get_text_width(esc_msg) / 2);
    draw_text(esc_msg, esc_x, 530, y);}

static void draw_about() {
    SDL_Color w = {255, 255, 255, 255};
    
    const char* header = "О ПРОГРАММЕ";
    int header_x = 400 - (get_text_width(header) / 2);
    draw_text(header, header_x, 50, w);
    
    draw_text("Разработчик: Тимощенко Даниил Сергеевич", 50, 100, w);
    draw_text("Разработчик: Цацу Глеб Михалович", 50, 150, w);
    draw_text("Группа: 5131001/50603", 50, 200, w);
    draw_text("СПбПУ Петра Великого (Политех)", 50, 250, w);
    draw_text("Институт компбютерных наук и кибербезопасности(ИКНК)", 50, 300, w);
    draw_text("Высшая школа кибербезопасности и защиты информации", 50, 350, w);
    
    const char* year = "2026";
    int year_x = 400 - (get_text_width(year) / 2);
    draw_text(year, year_x, 420, w);
    
    const char* esc_msg = "Нажми ESC для возврата в меню";
    int esc_x = 400 - (get_text_width(esc_msg) / 2);
    draw_text(esc_msg, esc_x, 530, w);
}

void gui_set_player_names(const char* n1, const char* n2) {
    strncpy(player1_name, n1, 31);
    strncpy(player2_name, n2, 31);
}

static void draw_name_input() {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color green = {0, 255, 0, 255};
    SDL_Color gray = {150, 150, 150, 255};

    draw_text("ВВОД ИМЕН ИГРОКОВ", 280, 50, white);
    
    draw_text("Игрок 1 (X):", 100, 150, entering_first ? green : white);
    draw_text(player1_name, 300, 150, white);
    
    draw_text("Игрок 2 (O):", 100, 250, !entering_first ? green : white);
    draw_text(player2_name, 300, 250, white);
    
    draw_text("Нажмите ENTER, чтобы подтвердить", 220, 400, white);
    draw_text("Нажмите BACKSPACE, чтобы стереть", 220, 450, gray);
}

void gui_draw(GuiState state, GameStatus win_status, GameMode mode, BotDifficulty diff, int bot_starts) {
    switch (state) {
        case GUI_MENU: draw_menu(); break;
        case GUI_SETTINGS: draw_settings(mode, diff, bot_starts); break;        case GUI_NAME_INPUT: draw_name_input(); break;
        case GUI_HELP: draw_help(); break;
        case GUI_ABOUT: draw_about(); break;
        case GUI_GAME: draw_grid(); draw_marks(); break;
        case GUI_WIN_SCREEN:
            draw_grid(); 
            draw_marks();
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
            SDL_FRect f = {0, 0, 800, 600};
            SDL_RenderFillRect(r, &f);
            SDL_Color gold = {255, 215, 0, 255};
            char win_msg[64];
            snprintf(win_msg, 64, "ПОБЕДИТЕЛЬ: %s!", (win_status == WIN_X) ? player1_name : player2_name);
            int tw = get_text_width(win_msg);
            draw_text(win_msg, 400 - tw / 2, 250, gold);
            draw_text("ESC - в меню", 335, 320, gold);
            break;
    }
}
