#include "client_game.h"


T_recv *thread_receive_input(int sock_fd, int ui){
	T_recv *arg = (T_recv *) malloc(sizeof(T_recv));
	arg->sock_fd = sock_fd;
	arg->ui = ui;
	return arg;
}


void *initBoard(int server_fd, int *dim, uint8_t color[3]){
	Message *msg = newMessage(server_fd);
	recvFromServer(msg);

	Init *init = message2init(msg);
	*dim = init->dim;
	memcpy(color, init->color, 3*sizeof(uint8_t));

	free((Message *)msg);
	free((Init *)init);

	return NULL;
}


void *initUI(int dim){
	if(SDL_Init(SDL_INIT_VIDEO)<0){
		 printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		 exit(-1);
	}

	if(TTF_Init()==-1){
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	create_board_window(WIDTH, HEIGHT, dim);
	
	return NULL;	
}


void *receive_events(void *arg){
	int sock_fd = ((T_recv *)arg)->sock_fd;
	int ui = ((T_recv *)arg)->ui;
	free((T_recv *)arg);

	Message *msg = newMessage(sock_fd);
	Card *card;
	char *end;

	// Receives one message (mainly cards) 
	while(recvFromServer(msg)>0){
		switch(findType(msg)){
			case CARD:
				card = message2card(msg);
				printClientCard(card);
				if(ui){
					paint_card(card->pos, card->color);
					write_card(card->pos, card->text, card->tcolor);
				}
				free((Card *)card);
				break;

			case END:
				end = message2end(msg);
				printf("end = %s\n", end);
				free((char *)end);
				break;
		}
	}
	done = 1;
	free((Message *)msg);

	return NULL;
}