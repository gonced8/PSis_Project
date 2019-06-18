#ifndef BOT_GAME_H
#define BOT_GAME_H

#include <stdio.h>

#include "communication.h"
#include "connection.h"
#include "UI_library.h"
#include "client_game.h"


#define MEMORY     0
#define LIST       1
#define SLEEP_TIME 100000

typedef struct pair{
	int play[2];
	struct pair* next_pair;
}pair_t;

typedef struct Board_memory{
	char str[3];   // this holds str
	int xy1[2];
	int xy2[2];
	bool ready;
	bool sent;
	int  board_completed;
}Memory;

typedef struct{
	int dim;
	T_recv    *receive;
	Memory    *memory;
	pair_t    **list;
	pthread_rwlock_t *rwlock;
	int 	  *max_list_index;
	int       on;
}Bot_smart;

Memory* fill_memory_list(Memory *memory_bot, Card *card, int number_of_pairs, int *blank);
void *receive_bot_events(void *arg);
pair_t* init_list(int dim);
void print_list(pair_t * head);
pair_t* remove_by_value(pair_t *head, int pos[2], int retval[2], int* list_num);
int search_index(pair_t *head, int pos[2]);
pair_t* add_list(pair_t *head, int val[2]);
pair_t* remove_by_index(pair_t** head, int n, int retval[2], int* list_num);
Memory* bot_memory_init(int max_p);
int find_play_method(Memory *memory, int num_pairs);
pair_t* restart_game(Memory *memory, pair_t *list, int pairs_num, int board_dim, int ui, Message *msg, int *max_index);
Memory* bot_memory_reset(Memory *memory, int max_p);
Memory* bot_memory_init(int max_p);
int is_my_card(Card *card, uint8_t *my_color);
pair_t* update_list(pair_t *list, Card *card, int *max_list_index);
void print_memory(Memory *memory, int max_p);
Memory* play_from_memory(Memory *memory, int max_pairs, int sock_fd, pthread_rwlock_t* rwlock);
void play_from_list(pair_t **pair_list, int sock_fd, pthread_rwlock_t *rwlock, int *max_list_index);
void play_random(int sock_fd, int board_dim);
void smart_bot_mode(Bot_smart *bot, int argc, char *argv[]);
void UI_mode(int *mode, int argc, char *argv[], int dim);
void no_inputs(int argc);

#endif /* BOT_GAME_H */
