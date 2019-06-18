#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <poll.h>
#include <sys/errno.h>

#include "communication.h"
#include "connection.h"

#define DIM_BOARD 4
#define DIM_LIM 26
#define WRONG_TIME 2
#define WAIT_TIME 5
#define RESET_TIME 10
#define MIN_PLAYERS 2

enum {WHITE='w', GRAY='g', BLACK='b', RED='r'};
enum {WAIT='w', FIRST='f', WRONG='i', CORRECT='c', TIMER='t'};


typedef struct{
	Player *player;
	Card *card;
}Entry;

typedef struct{
	Entry *entry1;
	Entry *entry2;
	char code;
	bool occupied;
}Play;

typedef struct{
	Entry **entry;
	int dim;
	pthread_rwlock_t *rw;
	int unsolved;
}Board;

typedef struct{
	Players *players;
	Board *board;
}T_init;

typedef struct{
	Players *players;
	Player *player;
	Board *board;
}T_recv;

typedef struct{
	int timer;
	pthread_mutex_t mux;
	pthread_cond_t cond;
}Timer;

typedef struct{
	Board *board;
	Players *players;
	Play *play;
	char *state;
	Timer *timer;
}T_timer;


int dimBoard(int argc, char *argv[]);
T_init *thread_init_input(Players *players, Board *board);
T_recv *thread_receive_input(Players *players, Player *player, Board *board);
void initBoard(Board *board, int dim, pthread_rwlock_t *rw);
void sendBoard(Board *board, int sock_fd, bool all);
void printEntry(Entry *entry);
void *init_players(void *par);
void *receive_play(void *arg);
void disconnect_player(Players *players, Player *player);
void broadcast(Players *players, Card *card);
bool getEntry(Board *board, Player *player, char state, int play[2], Entry **entry);
void eraseEntry(Board *board, Entry **entry);
void game_rules(Play *play, Board *board, Players *players, Player *player, Timer *timer, int *pos);
void fill_color(char color, uint8_t tcolor[3]);
void flush_socket(int sock_fd);
Entry **init_board(int dim);
void clearEntry(Board *board, Players *players, Entry *entry);
void init_timer(Board *board, Players *players, Play *play, char *state, Timer *timer, pthread_t *id);
void *timer_func(void *arg);
void close_timer(pthread_t id, Timer *timer);
int check_solved(Board *board, Players *players);
bool best_score(Players *players, int *score);
void clearBoard(Board *board);

#endif /* SERVER_GAME_H */
