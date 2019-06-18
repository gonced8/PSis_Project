#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

#include <stdio.h>

#include "communication.h"
#include "connection.h"
#include "UI_library.h"

extern int done;

typedef struct{
	int sock_fd;
	int ui;
}T_recv;


T_recv *thread_receive_input(int sock_fd, int ui);
void *initBoard(int server_fd, int *dim, uint8_t color[3]);
void *receive_events(void *arg);
void *initUI(int dim);

#endif /* CLIENT_GAME_H */
