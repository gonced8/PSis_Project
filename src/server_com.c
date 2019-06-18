#include "communication.h"


Card *newCard(int pos[2], uint8_t color[3], char text[SIZE_TEXT], uint8_t tcolor[3]){
	Card *card = (Card *) malloc(sizeof(Card));

	memcpy(card->pos, pos, 2*sizeof(int));
	memcpy(card->color, color, 3*sizeof(uint8_t));
	memcpy(card->text, text, SIZE_TEXT); 
	memcpy(card->tcolor, tcolor, 3*sizeof(uint8_t));

	return card;
}


Message *init2message(int sock_fd, Init *init){
	int i = 1;
	Message *msg = (Message *) malloc(sizeof(Message));
	msg->sock_fd = sock_fd;

	memset(msg->buffer, INIT, sizeof(INIT));

	memcpy(msg->buffer+i, &(init->dim), sizeof(int));
	i += sizeof(int);
	
	memcpy(msg->buffer+i, init->color, 3*sizeof(uint8_t));
	i += sizeof(uint8_t);

	// free(init);

	return msg;
}


Message *card2message(int sock_fd, Card *card){
	int i = 1;
	Message *msg = (Message *) malloc(sizeof(Message));
	msg->sock_fd = sock_fd;
	
	memset(msg->buffer, CARD, sizeof(CARD));

	memcpy(msg->buffer+i, card->pos, 2*sizeof(int));
	i += 2*sizeof(int);

	memcpy(msg->buffer+i, card->color, 3*sizeof(uint8_t));
	i += 3*sizeof(uint8_t);

	memcpy(msg->buffer+i, card->text, SIZE_TEXT*sizeof(char));
	i += SIZE_TEXT*sizeof(char);

	memcpy(msg->buffer+i, card->tcolor, 3*sizeof(uint8_t));
	i += 3*sizeof(uint8_t);

	// free(card);

	return msg;
}


Message *end2message(int sock_fd, char *end){
	Message *msg = (Message *) malloc(sizeof(Message));
	msg->sock_fd = sock_fd;

	memset(msg->buffer, END, sizeof(END));

	memcpy(msg->buffer+1, end, SIZE_END);
	
	// free(end);

	return msg;
}


void *sendToClient(Message *msg){
	if(send(msg->sock_fd, msg->buffer, SIZE_MSG_SERVER, 0)>0)
		printf("Sent message (%c) to client in socket %d\n", *(msg->buffer), msg->sock_fd);
	else
		printf("Send: client in socket %d disconnected\n", msg->sock_fd);

	// free(msg);

	return NULL;
}


int recvFromClient(int client_fd, int *play){
	int count;
	if((count = recv(client_fd, play, 2*sizeof(int), 0)) > 0)
		printf("Received message from client in socket %d\n", client_fd);
	else
		printf("Recv: client in socket %d disconnected\n", client_fd);

	return count;
}
