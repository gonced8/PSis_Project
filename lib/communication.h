#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "board_library.h"
#include "connection.h"


#define SIZE_TEXT 3
#define SIZE_END 15
#define SIZE_MSG_SERVER (sizeof(char)+2*sizeof(int)+3*sizeof(uint8_t)+SIZE_TEXT*sizeof(char)+3*sizeof(uint8_t))

enum {INIT='i', CARD='c', END='e'};

typedef struct{
	int sock_fd;
	char buffer[SIZE_MSG_SERVER];
}Message;

typedef struct{
	int dim;
	uint8_t color[3];
}Init;

typedef struct{
	int pos[2];
	uint8_t color[3];
	char text[SIZE_TEXT];
	uint8_t tcolor[3];
}Card;

// typedef char End[SIZE_END];


Card *newCard(int pos[2], uint8_t color[3], char text[SIZE_TEXT], uint8_t tcolor[3]);
void printClientCard(Card *card);
Message *init2message(int sock_fd, Init *init);
Message *card2message(int sock_fd, Card *card);
Message *end2message(int sock_fd, char *end);
Message *newMessage(int sock_fd);
char findType(Message *msg);
Init *message2init(Message *msg);
Card *message2card(Message *msg);
char *message2end(Message *msg);
void *sendToClient(Message *msg);
void *sendToServer(int server_fd, int play[2]);
int recvFromClient(int client_fd, int play[2]);
int recvFromServer(Message *msg);

#endif	/* COMMUNICATION.H */
