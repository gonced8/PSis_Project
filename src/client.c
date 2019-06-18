#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "client_game.h"
#include "communication.h"
#include "connection.h"
#include "UI_library.h"

int done = 0;

static void interrupt_handler(int signum);

int main(int argc, char *argv[]){
	/* Variables */
	struct sigaction action;
	pthread_t thread;
	int sock_fd;
	int dim;
	uint8_t color[3];
	SDL_Event event;
	int play[2];

	/* Program */
	action.sa_handler = &interrupt_handler;
	sigemptyset(&action.sa_mask);
	if(sigaction(SIGINT, &action, NULL)==-1){
		perror("Error while creating sigaction: ");
		return 1;
	}
	
	connect_server(&sock_fd, argc, argv);

	initBoard(sock_fd, &dim, color);
	printf("Dim = %d\nColor = [%d, %d, %d]\n", dim, color[0], color[1], color[2]);

	initUI(dim);

	T_recv *arg = thread_receive_input(sock_fd, 1);
	pthread_create(&thread, 0, receive_events, (void *)arg);

	while (!done){
		while (SDL_PollEvent(&event)){
			switch (event.type){
				case SDL_QUIT:
					done = SDL_TRUE;
					break;

				case SDL_MOUSEBUTTONDOWN:
					get_board_card(event.button.x, event.button.y, play);
					printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, play[0], play[1]);
					sendToServer(sock_fd, play);
					break;
			}
		}
	}

	close(sock_fd);

	printf("end\n");
	exit(0);
}



static void interrupt_handler(int signum){
	// 
	printf("CTRL+C PRESSED!!!\nEXITING GAME...\n");
	done = 1;
}
