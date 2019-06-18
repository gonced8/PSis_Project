#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <stdbool.h>
#include <signal.h>

#define ADDRESS "127.0.0.1"
#define PORT 3000

enum {NEW='n', ONLINE='c', OFFLINE='d', RESET='r'};

typedef struct Player{
	int sock_fd;
	struct sockaddr_in client;
	uint8_t color[3];
	char state;
	pthread_t thread;
	struct Player* previous;
	struct Player* next;
	int score;
}Player;

typedef struct{
	Player *start;
	pthread_rwlock_t *rwlock;
	int count;
	sem_t *sem;
}Players;

typedef struct{
	Players *players;
	int dim;
	sem_t *sem;
}T_connections;


void connect_server(int *fd, int argc, char *argv[]);

T_connections *thread_connections_input(Players *players, int dim);
void *accept_connections(void *par);
void random_color(uint8_t color[3]);
void initPlayers(Players *players, pthread_rwlock_t *rwlock, sem_t *sem);
void newPlayer(Players *players, int sock_fd, struct sockaddr_in client, uint8_t color[3]);
void removePlayer(Players *players, Player *player);
int countPlayers(Players *players);
void print_addr();

#endif /* CONNECTION_H */
