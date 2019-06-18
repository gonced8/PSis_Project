#ifndef UI_LIBRARY_H
#define UI_LIBRARY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


#define WIDTH 300
#define HEIGHT 300
#define DELAY 5


SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font;
int card_size[2];


void write_card(int board[2], char *text, uint8_t tcolor[3]);
void paint_card(int board[2], uint8_t color[3]);
void clear_card(int  board_x, int board_y);
void get_board_card(int mouse_x, int mouse_y, int *play);
void create_board_window(int width, int height, int dim);
void close_board_window();

#endif /* UI_LIBRARY_H */
