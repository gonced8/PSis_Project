#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <pthread.h>
#include <time.h>
#include <asm/errno.h>

#include "../lib/board_library.h"
#include "../lib/UI_library.h"

pthread_cond_t cond;
pthread_mutex_t mux; 

void *thread_func(void *arg)
{
	
	play_response *response;

	response = (struct play_response *)arg;

	while(1){
		if (response->code == 1 && response->timer5sec_init == 1){
			struct timespec ts = {0,0};
			int rt = 0;

			clock_gettime(CLOCK_REALTIME, &ts);

		    ts.tv_sec += 5; // waits 5 seconds
			printf("entering mutex\n");
			pthread_mutex_lock(&mux);
			rt = pthread_cond_timedwait(&cond, &mux, &ts); // starts waiting for 5 sec
			if (rt == 0){
				printf("2ND CARD WAS PICK...\n");
				response->timer5sec_init = 0;  // redundant
			}else if (rt = ETIMEDOUT){
				printf("tired of waiting for the 2nd card...\n");
				paint_card(response->play1[0], response->play1[1] , 255, 255, 255); // PUT THE CARD DOWN
				response->timer5sec_init = 0;  // waits for another 1st pick
				response->play1_int[0]=-1;     // resets 1st pick
			} 
			pthread_mutex_unlock(&mux);
			printf("leaving mutex\n");
		}
	}
}

int main(){

	play_response resp;

	pthread_mutex_init(&mux, NULL);
	pthread_cond_init(&cond, NULL);  // created and initialized

	pthread_t thread_id;

	pthread_create(&thread_id, NULL, thread_func,  &resp.code);

	SDL_Event event;
	int done = 0;

	 if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}
	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}


	create_board_window(300, 300,  2);
	init_board(2);
	resp.play1_int[0]=-1;

	while (!done){
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					done = SDL_TRUE;
					break;
				}
				case SDL_MOUSEBUTTONDOWN:{
					int board_x, board_y;
					get_board_card(event.button.x, event.button.y, &board_x, &board_y);

					printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y);
					board_play(board_x, board_y, &resp);
					switch (resp.code) {
						case 1:
							paint_card(resp.play1[0], resp.play1[1] , 7, 200, 100);
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);
							resp.timer5sec_init = 1;
							break;
						case 3:
						  done = 1;
						case 2:
							paint_card(resp.play1[0], resp.play1[1] , 107, 200, 100);
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
							paint_card(resp.play2[0], resp.play2[1] , 107, 200, 100);
							write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
							pthread_cond_signal(&cond);
							break;
						case -2:
							paint_card(resp.play1[0], resp.play1[1] , 107, 200, 100);
							write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);
							paint_card(resp.play2[0], resp.play2[1] , 107, 200, 100);
							write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0);
							sleep(2);
							paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255);
							paint_card(resp.play2[0], resp.play2[1] , 255, 255, 255);
							break;
						case 0:
							paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255); // turn down the 1st card
							resp.play1_int[0]=-1;									  // resets 1st pick
							break;
						case 4:
							resp.play1_int[0]=-1;
							break;

					}
				}
			}
		}
	}
	printf("fim\n");
	close_board_windows();
}
