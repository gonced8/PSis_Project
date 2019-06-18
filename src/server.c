#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>

#include "board_library.h"
#include "connection.h"
#include "communication.h"
#include "server_game.h"

#define RANDOM 1
// #define NUM_MUX 1
#define NUM_RW 2
#define NUM_SEM 1
#define NUM_THREADS 2
#define NPLAYERS 1

int global_socket = -1;

static void interrupt_handler(int signum);

void init_mux(pthread_mutex_t *mux);
void init_rw(pthread_rwlock_t *rwlock);
void init_sem(sem_t **sem);


int main(int argc, char *argv[]){

	// pthread_mutex_t mux[NUM_MUX];
	pthread_rwlock_t rwlock[NUM_RW];
	sem_t *sem[NUM_SEM];
	pthread_t id[NUM_THREADS];
	int i;
	int dim;
	Board board;
	Players players;

	signal(SIGINT, &interrupt_handler);
	signal(SIGSEGV, SIG_IGN);

	if(RANDOM)
		srand(time(NULL));

	// init_mux(mux);
	init_rw(rwlock);
	init_sem(sem);

	// Reads board dimension
	dim = dimBoard(argc, argv);

	// Initializes board
	initBoard(&board, dim, &(rwlock[0]));
	
	// Accept connections
	initPlayers(&players, &(rwlock[1]), sem[0]);
	T_connections *par0 = thread_connections_input(&players, dim);
	pthread_create(&(id[0]), 0, accept_connections, (void *)par0);
	
	// Init players
	T_init *par1 = thread_init_input(&players, &board);
	pthread_create(&(id[1]), 0, init_players, (void *)par1);

	for(i=0; i<NUM_THREADS; i++)
		pthread_join(id[i], NULL);

	printf("Server end\n");
	exit(0);
}



void init_mux(pthread_mutex_t *mux){
	// pthread_mutex_init(&(mux[0]), NULL);	// 0 = Board card mutex
	// pthread_mutex_init(&(mux[1]), NULL);	// 1 = Players list mutex
	return;
}


void init_rw(pthread_rwlock_t *rwlock){
	pthread_rwlock_init(&(rwlock[0]), NULL);	// 0 = Board rwlock
	pthread_rwlock_init(&(rwlock[1]), NULL);	// 1 = Players list rwlock
	return;
}


void init_sem(sem_t **sem){
	// 0 = Unitialized players. Send board to.
	if((sem[0] = sem_open("/sem0", O_CREAT, 0644, 0)) == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	sem_unlink("/sem0");
	return;
}


static void interrupt_handler(int signum){
	printf("\nCTRL+C PRESSED!!!\n");
	if (global_socket!=-1){
		close(global_socket);
	}
	printf("Closed server\n");
	exit(0);
}
