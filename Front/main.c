#include <SDL3/SDL.h>
#include "game_logic.h"
#include "gui.h"
#include <stdio.h>
#include <string.h>

void write_log(const char* msg) {
    FILE* f = fopen("game.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        printf("=========================================\n");
        printf(" Бесконечные Крестики-Нолики (Гомоку)\n");
        printf("=========================================\n");
        printf(" Использование: game.exe [параметры]\n");
        printf(" Параметры:\n");
        printf("   --help    Показать эту справку\n");
        printf(" Управление:\n");
        printf("   ЛКМ       - Поставить знак\n");
        printf("   W,A,S,D   - Перемещение камеры\n");
        printf("   ESC       - Выход в меню\n");
        printf("=========================================\n");
        return 0;
    }

    write_log("--- Запуск программы ---");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

    SDL_Window* win = SDL_CreateWindow("Infinite Gomoku - Polytech Edition", 800, 600, 0);
    SDL_Renderer* r = SDL_CreateRenderer(win, NULL);

    gui_init(win, r);
    init_game();
    
    if (load_game("save.txt")) {
        write_log("Сохранение успешно загружено");
    } else {
        write_log("Создана новая игровая сессия");
    }

    GuiState state = GUI_MENU;
    GuiState last_state = GUI_MENU;
    GameMode mode = MODE_PVE;
    BotDifficulty diff = BOT_HARD;
    CellState current = CELL_X;
    GameStatus win_status = GAME_CONTINUES;
    int bot_starts = 0; 
    CellState human_symbol = CELL_X;
    CellState bot_symbol = CELL_O;
    SDL_Event e;
    int run = 1;

    while(run)
    {
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_EVENT_QUIT) run = 0;

            GuiState state_before_event = state;

            gui_handle_event(&e, &state, &mode, &diff, &bot_starts);

            if(e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && 
               state == GUI_GAME && 
               state_before_event == GUI_GAME && 
               mode != MODE_EVE &&               
               win_status == GAME_CONTINUES)
            {
                if (mode == MODE_PVP || (mode == MODE_PVE && current == human_symbol)) {
                    Move m = gui_get_cell_from_mouse(e.button.x, e.button.y);
                    if(make_move(m.x, m.y, current) == MOVE_SUCCESS) {
                        write_log("Игрок сделал ход");
                        win_status = check_win(m.x, m.y);
                        current = (current == CELL_X) ? CELL_O : CELL_X;
                    }
                }
            }

            if(e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE && state == GUI_WIN_SCREEN) {
                state = GUI_MENU;
                win_status = GAME_CONTINUES;
                write_log("Возврат в главное меню");
            }
        }

        if ((last_state == GUI_SETTINGS || last_state == GUI_NAME_INPUT) && state == GUI_GAME) {
            current = CELL_X;
            write_log("--- Начало новой партии ---");
            
            PlayerInfo p1, p2;
            char log_msg[128];
            p1.symbol = CELL_X;
            p2.symbol = CELL_O;
            
            if (mode == MODE_PVP) {
                p1.type = PLAYER_HUMAN; strcpy(p1.info.nickname, "Игрок 1");
                p2.type = PLAYER_HUMAN; strcpy(p2.info.nickname, "Игрок 2");
                sprintf(log_msg, "Участники: %s (X) против %s (O)", p1.info.nickname, p2.info.nickname);
                human_symbol = CELL_X; 
            } 
            else if (mode == MODE_PVE) {
                if (bot_starts) {
                    p1.type = PLAYER_BOT;   p1.info.difficulty = diff;
                    p2.type = PLAYER_HUMAN; strcpy(p2.info.nickname, "Человек");
                    sprintf(log_msg, "Участники: Бот (X) против Человека (O)");
                    gui_set_player_names("Бот", "Человек");
                    human_symbol = CELL_O;
                    bot_symbol = CELL_X;
                } else {
                    p1.type = PLAYER_HUMAN; strcpy(p1.info.nickname, "Человек");
                    p2.type = PLAYER_BOT;   p2.info.difficulty = diff;
                    sprintf(log_msg, "Участники: Человек (X) против Бота (O)");
                    gui_set_player_names("Человек", "Бот");
                    human_symbol = CELL_X;
                    bot_symbol = CELL_O;
                }
            } 
            else if (mode == MODE_EVE) {
                p1.type = PLAYER_BOT;   p1.info.difficulty = diff;
                p2.type = PLAYER_BOT;   p2.info.difficulty = diff;
                sprintf(log_msg, "Участники: Бот (X) против Бота (O), сложность %d", diff);
                gui_set_player_names("Бот (X)", "Бот (O)");
            }
            write_log(log_msg);
            
            if (mode == MODE_EVE) {
                make_move(0, 0, CELL_X);
                gui_set_bot_move(0, 0);
                current = CELL_O;
            } else if (mode == MODE_PVE && bot_starts) {
                make_move(0, 0, bot_symbol);
                gui_set_bot_move(0, 0);
                current = human_symbol; 
            }
        }
        last_state = state;

        if(state == GUI_GAME && win_status == GAME_CONTINUES)
        {
            if(mode == MODE_PVE && current == bot_symbol)
            {
                Move b = bot_make_move(bot_symbol, diff);
                if (make_move(b.x, b.y, bot_symbol) == MOVE_SUCCESS) {
                    gui_set_bot_move(b.x, b.y);
                    write_log("Бот сделал ход");
                    win_status = check_win(b.x, b.y);
                    current = human_symbol;
                }
            }

            if(mode == MODE_EVE)
            {
                Move b = bot_make_move(current, diff);
                if (make_move(b.x, b.y, current) == MOVE_SUCCESS) {
                    gui_set_bot_move(b.x, b.y);
                    win_status = check_win(b.x, b.y);
                    current = (current == CELL_X) ? CELL_O : CELL_X;
                }
                SDL_Delay(200);
            }
        }

        if (win_status != GAME_CONTINUES && state != GUI_WIN_SCREEN) {
            write_log(win_status == WIN_X ? "Результат: Победа Крестиков" : "Результат: Победа Ноликов");
            state = GUI_WIN_SCREEN; 
        }

        SDL_SetRenderDrawColor(r, 0, 0, 0, 255); 
        SDL_RenderClear(r);
        
        gui_draw(state, win_status, mode, diff, bot_starts);        
        SDL_RenderPresent(r);
    }

    save_game("save.txt");
    write_log("Игра сохранена. Завершение работы.");

    cleanup_game();
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    return 0;
}