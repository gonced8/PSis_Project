#include "server_game.h"


int dimBoard(int argc, char *argv[]){
	int dim = DIM_BOARD;

	if(argc>1){
		sscanf(argv[1], "%d", &dim);
		if(dim<=0 || dim>DIM_LIM || (dim*dim)%2!= 0)
			dim = DIM_BOARD;
	}

	return dim;
}


T_init *thread_init_input(Players *players, Board *board){
	T_init *par = (T_init *) malloc(sizeof(T_init));

	par->players = players;
	par->board = board;

	return par;
}


T_recv *thread_receive_input(Players *players, Player *player, Board *board){
	T_recv *par = (T_recv *) malloc(sizeof(T_recv));

	par->players = players;
	par->player = player;
	par->board = board;

	return par;
}


void initBoard(Board *board, int dim, pthread_rwlock_t *rw){
	board->entry = init_board(dim);
	board->dim = dim;
	board->rw = rw;
	board->unsolved = dim*dim/2;

	return;
}


Entry **init_board(int dim){
	int count  = 0;
	int i, j;
	Entry **entry = (Entry **) malloc(sizeof(Entry *)*dim*dim);
	int pos[2];
	uint8_t color[3];
	char text[SIZE_TEXT] = {'\0'};
	int *array = shuffled_array(dim*dim);

	fill_color(WHITE, color);

	for (char c1 = 'a' ; c1 < ('a'+dim); c1++){
		for (char c2 = 'a' ; c2 < ('a'+dim); c2++){
			for(i=count; i<count+2; i++){
				j = array[i];
				matrix_conv(dim, j, &(pos[0]), &(pos[1]));
				text[0] = c1;
				text[1] = c2;
				entry[j] = (Entry *) malloc(sizeof(Entry));
				entry[j]->player = NULL;
				entry[j]->card = newCard(pos, color, text, color);
				printf("%d: %d %d -%s-\n", i, pos[0], pos[1], entry[j]->card->text);
			}
			count += 2;
			if (count == dim*dim){
				free((int *)array);
				return entry;
			}
		}
	}

	printf("Error initializing board\n");
	exit(-1);
}


void *init_players(void *par){
	Players *players = ((T_init *)par)->players;
	Board *board = ((T_init *)par)->board;
	free((T_init *)par);

	sem_t *sem = players->sem;
	int dim = board->dim;
	Player *player;
	Init init;
	Message *msg;
	T_recv *arg;
	bool flag;

	init.dim = dim;

	while(1){
		sem_wait(sem);
		printf("Semaphore down\n");

		pthread_rwlock_wrlock(players->rwlock);

		flag = true;
		for(player=players->start; flag && player!=NULL; player = player->next){
			switch(player->state){
				case NEW:
					players->count++;

					memcpy(init.color, player->color, 3*sizeof(uint8_t));
					msg = init2message(player->sock_fd, &init);
					sendToClient(msg);
					free((Message *)msg);

					sendBoard(board, player->sock_fd, 0);

					player->state = ONLINE;
					arg = thread_receive_input(players, player, board);
					pthread_create(&(player->thread), 0, receive_play, (void *)arg);

					printf("Created thread for player in socket %d\n", player->sock_fd);
					flag = false;
					break;

				case RESET:
					printf("Sending new board\n");
					sendBoard(board, player->sock_fd, 1);
					player->state = ONLINE;
					flag = false;
					break;
			}
		}

		pthread_rwlock_unlock(players->rwlock);
	}

	return NULL;
}


void sendBoard(Board *board, int sock_fd, bool all){
	int count = (board->dim)*(board->dim);
	Message *msg;
	int i = 0;

	for(i=0; i<count; i++){
		pthread_rwlock_rdlock(board->rw);

		if((board->entry[i]->player != NULL) || all){
			msg = card2message(sock_fd, board->entry[i]->card);
			printEntry(board->entry[i]);

			sendToClient(msg);
			free((Message *)msg);
		}

		pthread_rwlock_unlock(board->rw);
	}


	printf("Sent board to player in socket %d\n", sock_fd);

	return;
}


void printEntry(Entry *entry){
	// printf("player = %d\n", entry->player->sock_fd);
	printf("pos = [%d, %d]\n", entry->card->pos[0], entry->card->pos[1]);
	printf("color = [%d, %d, %d]\n", entry->card->color[0], entry->card->color[1], entry->card->color[2]);
	printf("text = \"%c%c%c\"\n", entry->card->text[0], entry->card->text[1], entry->card->text[2]);
	printf("tcolor = [%d, %d, %d]\n", entry->card->tcolor[0], entry->card->tcolor[1], entry->card->tcolor[2]);
	return;
}


void *receive_play(void *arg){
	Players *players = ((T_recv *)arg)->players;
	Player *player = ((T_recv *)arg)->player;
	Board *board = ((T_recv *)arg)->board;
	free((T_recv *)arg);

	int pos[2];
	Play play;
	play.code = WAIT;

	pthread_t id;
	Timer timer;
	init_timer(board, players, &play, &(player->state), &timer, &id);

	while(recvFromClient(player->sock_fd, pos)>0){
		printf("Player in socket %d: [%d, %d]\n", player->sock_fd, pos[0], pos[1]);
		game_rules(&play, board, players, player, &timer, pos);
	}

	player->state = OFFLINE;
	disconnect_player(players, player);
	close_timer(id, &timer);

	printf("Closed player thread\n");
	return NULL;
}


void disconnect_player(Players *players, Player *player){
	int sock = player->sock_fd;
	removePlayer(players, player);
	close(sock);
	printf("Closed socket %d\n", sock);
	pthread_rwlock_wrlock(players->rwlock);
	players->count--;
	pthread_rwlock_unlock(players->rwlock);
	printf("Removed player in socket %d\n", sock);

	return;
}


void broadcast(Players *players, Card *card){
	Player *player;

	Message *msg = card2message(0, card);

	pthread_rwlock_rdlock(players->rwlock);
	for(player=players->start; player!=NULL; player=player->next){
		msg->sock_fd = player->sock_fd;
		sendToClient(msg);
	}
	pthread_rwlock_unlock(players->rwlock);

	free((Message *)msg);

	return;
}


bool getEntry(Board *board, Player *player, char state, int play[2], Entry **entry){
	
	int index = linear_conv(board->dim, play[0], play[1]);
	
	if ( (index < 0) || (index >= board->dim*board->dim)){
		printf("INVALID INDEX : %i \n", index);
		printf("INVALID POSTION: [%i, %i]\n", play[0], play[1]);
		return true;
	}

	bool occupied = true;

	if(state != WRONG){
		pthread_rwlock_wrlock(board->rw);
		if(board->entry[index]->player == NULL){
			board->entry[index]->player = player;
			occupied = false;
		}
		pthread_rwlock_unlock(board->rw);
	}

	printEntry(board->entry[index]);

	if(occupied)
		printf("Ignored play\n");
	else{
		*entry = board->entry[index];
		//memcpy((*entry)->card->pos, play, 2*sizeof(int));
		memcpy((*entry)->card->color, player->color, 3*sizeof(uint8_t));
		//memcpy((*entry)->card->text, board->board[index].v, SIZE_TEXT);
	}

	return occupied;
}


void clearEntry(Board *board, Players *players, Entry *entry){
	uint8_t color[3];
	fill_color(WHITE, color);

	memcpy(entry->card->color, color, 3*sizeof(uint8_t));
	memcpy(entry->card->tcolor, color, 3*sizeof(uint8_t));
	broadcast(players, entry->card);

	pthread_rwlock_wrlock(board->rw);
	entry->player = NULL;
	pthread_rwlock_unlock(board->rw);

	return;
}


void game_rules(Play *play, Board *board, Players *players, Player *player, Timer *timer, int *pos){
	if(player->state != ONLINE){
		printf("Player not ready\n");
		return;
	}
	if(players->count<MIN_PLAYERS){
		printf("Less than %d players playing. Waiting.\n", MIN_PLAYERS);
		return;
	}

	Entry *entry;
	(*play).occupied = getEntry(board, player, (*play).code, pos, &entry);

	pthread_mutex_lock(&(timer->mux));
	switch(play->code){
		case WAIT:
			if(!(play->occupied)){
				play->entry1 = entry;
				play->code = FIRST;
				fill_color(GRAY, play->entry1->card->tcolor);
				broadcast(players, play->entry1->card);

				pthread_cond_signal(&(timer->cond));
				timer->timer = WAIT_TIME;
			}
			break;

		case FIRST:
			if(play->occupied){
				clearEntry(board, players, play->entry1);
				play->code = WAIT;

				pthread_cond_signal(&(timer->cond));
			}
			else{
				play->entry2 = entry;

				// compare cards texts
				if(0 == strcmp(play->entry1->card->text, play->entry2->card->text)){
					// correct
					play->code = CORRECT;
					fill_color(BLACK, play->entry1->card->tcolor);
					fill_color(BLACK, play->entry2->card->tcolor);
					broadcast(players, play->entry1->card);
					broadcast(players, play->entry2->card);

					player->score++;

					if(check_solved(board, players)){
						printf("reset timer\n");
						pthread_cond_signal(&(timer->cond));
						timer->timer = RESET_TIME;
					}
					else
						play->code = WAIT;
				}
				else{
					// wrong
					play->code = WRONG;
					fill_color(RED, play->entry1->card->tcolor);
					fill_color(RED, play->entry2->card->tcolor);
					broadcast(players, play->entry1->card);
					broadcast(players, play->entry2->card);

					pthread_cond_signal(&(timer->cond));
					timer->timer = WRONG_TIME;
				}
			}
			break;

		case WRONG:
			printf("Wrong pair. Wait.\n");
			break;
	}
	pthread_mutex_unlock(&(timer->mux));

	return;
}


void fill_color(char color, uint8_t array[3]){
	switch (color){
		case WHITE:
			array[0]=255;
			array[1]=255;
			array[2]=255;
			break;
		case GRAY:
			array[0]=200;
			array[1]=200;
			array[2]=200;
			break;
		case BLACK:
			array[0]=0;
			array[1]=0;
			array[2]=0;
			break;
		case RED:
			array[0]=255;
			array[1]=0;
			array[2]=0;
			break;
	}

	return;
}


void flush_socket(int sock_fd){
	struct pollfd pfd[1];
	int play[2];

	pfd->fd = sock_fd;
	pfd->events = POLLIN;
	pfd->revents = 0;

	while(poll(pfd, 1, 0)>0){
		recv(sock_fd, play, 2*sizeof(int), 0);
	}

	return;
}


void *timer_func(void *arg){
	Board *board = ((T_timer *)arg)->board;
	Players *players = ((T_timer *)arg)->players;
	Play *play = ((T_timer *)arg)->play;
	//char *state = ((T_timer *)arg)->state;
	int *timer = &(((T_timer *)arg)->timer->timer);
	pthread_mutex_t *mux = &(((T_timer *)arg)->timer->mux);
	pthread_cond_t *cond = &(((T_timer *)arg)->timer->cond);
	free((T_timer *) arg);

	struct timespec ts = {0,0};
	Player *player;

	pthread_mutex_lock(mux);

	*timer = 0;
	pthread_cond_signal(cond);

	while(*timer != -1){
		while(play->code==WAIT && *timer!=-1){
			*timer = 0;
			printf("sleep %d\n", *timer);
			pthread_cond_wait(cond, mux);
			printf("timer = %d\n", *timer);
		}

		printf("awake\n");
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += *timer;

		switch(play->code){
			case FIRST:
				printf("wait for 2nd card. 5s\n");
				if(pthread_cond_timedwait(cond, mux, &ts)==ETIMEDOUT && play->code==FIRST){
					printf("erased 1 card\n");
					clearEntry(board, players, play->entry1);

					play->code = WAIT;
					//*timer = 0;
				}
				break;

			case WRONG:
				printf("wrong cards. 2s\n");
				if(pthread_cond_timedwait(cond, mux, &ts)==ETIMEDOUT && play->code==WRONG){
					printf("erased 2 card\n");
					clearEntry(board, players, play->entry1);
					clearEntry(board, players, play->entry2);

					play->code = WAIT;
					//*timer = 0;
				}
				break;

			case CORRECT:
				play->code = WAIT;

				printf("Waiting 10s to reset board.\n");
				if(pthread_cond_timedwait(cond, mux, &ts)==ETIMEDOUT && play->code==WAIT){
					printf("Reseting game\n");

					pthread_rwlock_rdlock(players->rwlock);

					pthread_rwlock_wrlock(board->rw);
					clearBoard(board);
					initBoard(board, board->dim, board->rw);
					pthread_rwlock_unlock(board->rw);
					printf("New board created\n");

					for(player=players->start; player!=NULL; player=player->next){
						player->state = RESET;
						printf("Semaphore up\n");
						sem_post(players->sem);
					}

					pthread_rwlock_unlock(players->rwlock);
				}
				break;
		}

	}

	pthread_mutex_unlock(mux);

	printf("Exited timer_func\n");
	return NULL;
}


void init_timer(Board *board, Players *players, Play *play, char *state, Timer *timer, pthread_t *id){
	pthread_mutex_init(&(timer->mux), NULL);
	pthread_cond_init(&(timer->cond), NULL);

	T_timer *arg = (T_timer *) malloc(sizeof(T_timer));
	arg->board = board;
	arg->players = players;
	arg->play = play;
	arg->state = state;
	arg->timer = timer;

	pthread_mutex_lock(&(timer->mux));
	pthread_create(id, 0, timer_func, (void *)arg);
	pthread_cond_wait(&(timer->cond), &(timer->mux));
	pthread_mutex_unlock(&(timer->mux));
	printf("Timer initialized\n");

	return;
}


void close_timer(pthread_t id, Timer *timer){
	printf("Closing timer\n");

	pthread_mutex_lock(&(timer->mux));
	if(timer->timer==0){
		pthread_cond_signal(&(timer->cond));
	}
	timer->timer = -1;
	pthread_mutex_unlock(&(timer->mux));

	pthread_join(id, NULL);
	printf("Closed timer\n");
	return;
}


int check_solved(Board *board, Players *players){
	pthread_rwlock_wrlock(board->rw);
	board->unsolved --;
	pthread_rwlock_unlock(board->rw);

	if(board->unsolved>0)
		return 0;

	printf("Sending end\n");

	char won[SIZE_END];
	char lost[SIZE_END] = "You suck!";	// lost
	int score;

	if(best_score(players, &score))
		strcpy(won, "Not too shabby");	// tie
	else
		strcpy(won, "You da best!");	// won

	Message *msg_won = end2message(0, won);
	Message *msg_lost = end2message(0, lost);

	Player *player;

	pthread_rwlock_wrlock(players->rwlock);
	for(player=players->start; player!=NULL; player=player->next){
		if(player->score == score){
			msg_won->sock_fd = player->sock_fd;
			sendToClient(msg_won);
		}
		else{
			msg_lost->sock_fd = player->sock_fd;
			sendToClient(msg_lost);
		}
		player->score = 0;
	}
	pthread_rwlock_unlock(players->rwlock);

	free((Message *)msg_won);
	free((Message *)msg_lost);

	return 1;
}


bool best_score(Players *players, int *score){
	bool tie;
	*score = -1;	
	Player *player;

	pthread_rwlock_rdlock(players->rwlock);
	for(player=players->start; player!=NULL; player=player->next){
		if(player->score > *score){
			*score = player->score;
			tie = false;
		}
		else if(player->score == *score)
			tie = true;
	}
	pthread_rwlock_unlock(players->rwlock);

	return tie;
}


void clearBoard(Board *board){
	int i;
	for(i=0; i<board->dim*board->dim; i++){
		free((Card *)board->entry[i]->card);
		free((Entry *)board->entry[i]);
	}

	free((Entry **)board->entry);

	return;
}
