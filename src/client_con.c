#include "connection.h"

void connect_server(int *fd, int argc, char *argv[]){
	struct sockaddr_in server;
	int fail;
	int port;

	*fd = socket(AF_INET, SOCK_STREAM, 0);

	/* Checks */
	if (*fd == -1){
		perror("socket: ");
		exit(-1);
    }

	printf("Socket created \n");

    server.sin_family = AF_INET;

	fail = 1;
	if(argc>1)
		if(inet_aton(argv[1], &(server.sin_addr)))
			fail = 0;
	if(fail)
		inet_aton(ADDRESS, &(server.sin_addr));

	fail = 1;
	if(argc>2)
		if(sscanf(argv[2], "%d", &port)==1){
			server.sin_port= htons(port);
			fail = 0;
		}
	if(fail)
		server.sin_port= htons(PORT);

	printf("Attempting connection to %s:%hu\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

    if(connect(*fd, (const struct sockaddr *) &server, sizeof(server)) == -1){
		printf("Error connecting\n");
		exit(-1);
	}

	printf("Client connected\n");
}
