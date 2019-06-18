#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "UI_library.h"


void write_card(int board[2], char *text, uint8_t tcolor[3]){
	SDL_Rect rect;

	rect.x = board[0] * card_size[0];
	rect.y = board[1] * card_size[1];
	rect.w = card_size[0]+1;
	rect.h = card_size[1]+1;

	// int text_x = board[0] * card_size[0];
	// int text_y = board[1] * card_size[1];

	SDL_Color color = { tcolor[0], tcolor[1], tcolor[2] };
 	SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);

	SDL_Texture* Background_Tx = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface); // we got the texture now -> free surface


	SDL_RenderCopy(renderer, Background_Tx, NULL, &rect);
	SDL_DestroyTexture(Background_Tx);
	SDL_RenderPresent(renderer);
	SDL_Delay(DELAY);

	return;
}


void paint_card(int board[2], uint8_t color[3]){
	SDL_Rect rect;

	rect.x = board[0] * card_size[0];
	rect.y = board[1] * card_size[1];
	rect.w = card_size[0]+1;
	rect.h = card_size[1]+1;

	SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &rect);

	SDL_RenderPresent(renderer);
	SDL_Delay(DELAY);

	return;
}

/*
void clear_card(int  board_x, int board_y){
	paint_card(board_x, board_y , 255, 255, 255);
}
*/

void get_board_card(int mouse_x, int mouse_y, int *play){
	play[0] = mouse_x / card_size[0];
	play[1] = mouse_y / card_size[1];
}

void create_board_window(int width, int height, int dim){
	int n_ronw_cols = dim;
	int col_width = width /n_ronw_cols;
	int row_height = height /n_ronw_cols;
	int screen_width = n_ronw_cols * col_width +1;
	int screen_height = n_ronw_cols *row_height +1;
	int i;

	card_size[0] = col_width;
	card_size[1] = row_height;

	if(SDL_CreateWindowAndRenderer(screen_width, screen_height, 0, &window, &renderer)  != 0){
		printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError());
		exit(-1);
	}

	SDL_PumpEvents();

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

	for (i=0; i<n_ronw_cols+1; i++){
		SDL_RenderDrawLine(renderer, 0, i*row_height, screen_width, i*row_height);
		SDL_RenderDrawLine(renderer, i*col_width, 0, i*col_width, screen_height);
	}

	SDL_RenderPresent(renderer);
	SDL_Delay(DELAY);

	font = TTF_OpenFont("arial.ttf", card_size[1]);

	return;
}


void close_board_window(){
	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	if (window) {
		SDL_DestroyWindow(window);
	}
	if (font) {
		TTF_CloseFont(font);
	}
}
