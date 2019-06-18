#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include "bot_game.h"
#include "communication.h"
#include "connection.h"
#include "UI_library.h"

#define RANDOM 1
#define NUM_THREADS 1

int done = 0;
static void interrupt_handler(int signum);

int main(int argc, char *argv[]){
	/* Variables */
	pthread_t thread[NUM_THREADS];
	pthread_rwlock_t rwlock;
	int sock_fd;
	int max_pairs;
	int dim;
	uint8_t color[3];
	SDL_Event event;
	int max_list_index;
	int ui_mode;
	Bot_smart  bot_smart;
	Memory *memory;

	/* Program */
	signal(SIGINT, &interrupt_handler);

	if(RANDOM)
		srand(time(NULL));
	
	no_inputs(argc);

	pthread_rwlock_init(&rwlock, NULL);	// 0 = list and memory rwlock
	connect_server(&sock_fd, (argc-2), (argv+2));

	initBoard(sock_fd, &dim, color);
	
	max_list_index = dim*dim;
	max_pairs = max_list_index/2;
	printf("Dim = %d\nColor = [%d, %d, %d]\n", dim, color[0], color[1], color[2]);

	UI_mode(&ui_mode, argc, argv, dim);
	smart_bot_mode(&bot_smart, argc, argv);

	T_recv *rcv = thread_receive_input(sock_fd, ui_mode);

	/* Board memory */
	memory = bot_memory_init(max_pairs);

	/* List of available plays */
	pair_t* pair_list = init_list(dim);

	bot_smart.dim 	 		 = dim;
	bot_smart.receive 		 = rcv;
	bot_smart.memory  		 = memory;
	bot_smart.list    		 = &pair_list;
	bot_smart.rwlock    	 = &rwlock;
	bot_smart.max_list_index = &max_list_index;

	/* Receives cards from server */
	pthread_create(&thread[0], 0, receive_bot_events, (void *)&bot_smart);
	srand(time(NULL));

	/* Sends cards to server */
	while (!done){
		while (SDL_PollEvent(&event)){
			switch (event.type){
				case SDL_QUIT:
					done = SDL_TRUE;
					break;
			}
		}

		if(bot_smart.on){
			/* Max Priority for cards available from memory */
			switch(find_play_method(memory, max_pairs)){
			 	case MEMORY:
					memory = play_from_memory(memory, max_pairs, sock_fd, &rwlock);
					break;
				case LIST:
					play_from_list(&pair_list, sock_fd, &rwlock, &max_list_index);
					break;		
			}	
		}
		else{
			play_random(sock_fd, dim);
		}
	}
	
	
	close(sock_fd);

	printf("end\n");
	exit(0);

}


static void interrupt_handler(int signum){
	// 
	printf("\nCTRL+C PRESSED!!!\nEXITING GAME...\n");
	done = 1;
}
