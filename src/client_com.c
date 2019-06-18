#include "communication.h"


Message *newMessage(int sock_fd){
	Message *msg = (Message *) malloc(sizeof(Message));
	msg->sock_fd = sock_fd;
	return msg;
}


char findType(Message *msg){
	return *(msg->buffer);
}


void printClientCard(Card *card){
	printf("pos = [%d, %d]\n", card->pos[0], card->pos[1]); 
	printf("color = [%d, %d, %d]\n", card->color[0], card->color[1], card->color[2]); 
	printf("text = \"%c%c%c\"\n", card->text[0], card->text[1], card->text[2]); 
	printf("tcolor = [%d, %d, %d]\n", card->tcolor[0], card->tcolor[1], card->tcolor[2]); 

	return;
}


Init *message2init(Message *msg){
	int i = sizeof(char);
	Init *init = (Init *) malloc(sizeof(Init));

	memcpy(&(init->dim), msg->buffer+i, sizeof(int));
	i += sizeof(int);

	memcpy(init->color, msg->buffer+i, 3*sizeof(uint8_t));
	i += 3*sizeof(uint8_t);

	// free(msg);

	return init;
}


Card *message2card(Message *msg){
	int i = sizeof(char);
	Card *card = (Card *) malloc(sizeof(Card));

	memcpy(card->pos, msg->buffer+i, 2*sizeof(int));
	i += 2*sizeof(int);

	memcpy(card->color, msg->buffer+i, 3*sizeof(uint8_t));
	i += 3*sizeof(uint8_t);

	memcpy(card->text, msg->buffer+i, SIZE_TEXT*sizeof(char));
	i += SIZE_TEXT*sizeof(char);
	// text doesn't end with '\0'
	
	memcpy(card->tcolor, msg->buffer+i, 3*sizeof(uint8_t));
	i += 3*sizeof(uint8_t);

	// free(msg);

	return card;
}


char *message2end(Message *msg){
	int i = sizeof(char);
	char *end = (char *) malloc(SIZE_END);

	memcpy(end, msg->buffer+i, SIZE_END);
	i += SIZE_END;

	end[SIZE_END-1] = '\0';
	
	// free(msg);

	return end;
}


int recvFromServer(Message *msg){
	int count;
	if((count = recv(msg->sock_fd, msg->buffer, SIZE_MSG_SERVER, 0)) > 0)
		printf("Received message (%c) from server\n", *(msg->buffer));
	else
		printf("Server disconnected\n");

	return count;
}


void *sendToServer(int server_fd, int play[2]){
	if(send(server_fd, play, 2*sizeof(int), 0)>0)
		printf("Sent message to server\n");
	else
		printf("Server disconnected\n");

	return NULL;
}
