#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUF_SIZE 500

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: ./client user \n");
        exit(EXIT_FAILURE);
    }

	//create socket
	int sfd; 
    sfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	int enable_broadcast = 1;

	//set socket option for broadcast
	setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, &enable_broadcast, sizeof(int));	

	//error handling if broadcast cannot be made
	if (setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, &enable_broadcast, sizeof(int)) != 0) {
		perror("setsockopt broadcast");
		exit(-1);
	}

	//set socket option for reuseaddr
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable_broadcast, sizeof(int));

	//error handling if reuseaddr cannot be made
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable_broadcast, sizeof(int)) != 0) {
		perror("setsockopt reuseaddr");
		exit(-1); 
	}

    //if socket unsuccessfully made
    if (sfd == -1) {
    	perror("socket");
        exit(-1);
    } 

	//create sockaddr_in struct
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = PF_INET;
	addr.sin_port = htons (8221);
	inet_pton(PF_INET, "10.10.13.255", &addr.sin_addr);

	//send input to broadcast
	int len = strlen(argv[1]);
	int sent = sendto(sfd, argv[1], len, 0, (struct sockaddr*) &addr, sizeof(struct sockaddr_in));

	//if input fails to send
	if (sent != len) {
		fprintf(stderr, "Error sending response\n");
		
	}

	printf("online: %s \n", argv[1]);
	sleep(2);
	printf("offline: %s \n", argv[1]);
	
    close(sfd);


}
