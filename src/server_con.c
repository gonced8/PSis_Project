#include "connection.h"


T_connections *thread_connections_input(Players *players, int dim){
	T_connections *par = (T_connections *) malloc(sizeof(T_connections));

	par->players = players;
	par->dim = dim;
	
	return par;
}


void *accept_connections(void *par){
	int server_fd;
	struct sockaddr_in server;
	int client_fd;
	struct sockaddr_in client;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	uint8_t color[3];
	Players *players = ((T_connections *)par)->players;
	int dim = dim;

	free((T_connections*) par);

	sem_t *sem = players->sem;

	signal(SIGPIPE, SIG_IGN);	// disables broken pipe signal

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(server_fd == -1){
		perror("socket");
		exit(-1);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr= INADDR_ANY;
	server.sin_port= htons(PORT);

	if(bind(server_fd, (struct sockaddr *)(&server), sizeof(server)) == -1){
		perror("bind");
		exit(-1);
	}
	printf("Socket created and binded \n");

	if(listen(server_fd, 5) == -1){
		perror("listen");
		exit(-1);
	}

	print_addr(); // prints server addresses


	printf("Waiting for players\n");
	while(1){
		client_fd  = accept(server_fd, (struct sockaddr *)(&client), &addrlen);

		if(client_fd == -1){
			perror("accept");
			exit(-1);
		}

		random_color(color);
		newPlayer(players, client_fd, client, color);
		sem_post(sem);		// +1 player unitialized, to send board to

		printf("Acceped connection in socket %d from %s:%hu\n", client_fd, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
	}
	
	return NULL;
}


void random_color(uint8_t color[3]){
	int i;
	for(i=0; i<3; i++)
		color[i] = (uint8_t) (rand()%256);

	return;
}


void initPlayers(Players *players, pthread_rwlock_t *rwlock, sem_t *sem){
	players->start = NULL;
	players->rwlock = rwlock;
	players->count = 0;
	players->sem = sem;

	return;
}


void newPlayer(Players *players, int sock_fd, struct sockaddr_in client, uint8_t color[3]){
	Player *player = (Player *) malloc(sizeof(Player));
   	player->sock_fd = sock_fd;
	player->client = client;
	memcpy(player->color, color, 3*sizeof(uint8_t));
	player->state = NEW;
	player->previous = NULL;
	player->score = 0;

	pthread_rwlock_wrlock(players->rwlock);

	if(players->start!=NULL)
		players->start->previous = player;

	player->next = players->start;
	players->start = player;

	pthread_rwlock_unlock(players->rwlock);

	return;
}


void removePlayer(Players *players, Player *player){
	pthread_rwlock_wrlock(players->rwlock);

	if(player->previous != NULL)
		player->previous->next = player->next;
	else
		players->start = player->next;

	if(player->next != NULL)
		player->next->previous = player->previous;

	pthread_rwlock_unlock(players->rwlock);

	free(player);

	return;	
}


int countPlayers(Players *players){
	int count = 0;
	Player *player;

	for(player = players->start; player!=NULL; player=player->next)
		count++;

	return count;
}


void print_addr(){
	struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    char buf[64];

	if(getifaddrs(&myaddrs) != 0){
        perror("getifaddrs");
        exit(1);
    }

	printf("===== Addresses =====\n");

	for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next){
        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        switch (ifa->ifa_addr->sa_family){
            case AF_INET:{
                struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                in_addr = &s4->sin_addr;
                break;
            }

            case AF_INET6:{
                struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                in_addr = &s6->sin6_addr;
                break;
            }

            default:
                continue;
        }

        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf)))
            printf("%s: inet_ntop failed!\n", ifa->ifa_name);
        else
            printf("%s: %s\n", ifa->ifa_name, buf);
    }

	printf("=====================\n");

    freeifaddrs(myaddrs);

	return;
}
