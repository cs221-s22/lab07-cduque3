#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 1000

int main(int argc, char *argv[])
{
    if (argc != 1) {
        fprintf(stderr, "Usage: ./server\n");
        exit(EXIT_FAILURE);
    }

	struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // Datagram socket
    hints.ai_flags = AI_PASSIVE;    // Any IP address (DHCP)
    hints.ai_protocol = 0;          // Any protocol
    hints.ai_canonname = NULL; // canonical name
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

	//getaddrinfo() returns one or more addrinfo structs
	//each struct contains Internet address that can be specified in call to bind
	struct addrinfo *result;
    int s = getaddrinfo(NULL, "8221", &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
      Try each address until we successfully bind().
      If socket() or bind() fails, we close the socket
      and try the next address. 
    */

	struct addrinfo *rp;
	int sfd; 
    for (rp = result; rp != NULL; rp = rp->ai_next) {
    	//socket function creates unbound socket in specified domain, returns socket file descriptor
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue; 

		int enable_broadcast = 1;
		setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, &enable_broadcast, sizeof(int));

		if (setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, &enable_broadcast, sizeof(int)) == -1) {
			perror("setsockopt broadcast");
			break;
		}

		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable_broadcast, sizeof(int));
		
		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable_broadcast, sizeof(int)) != 0) {
			perror("setsockopt reuseaddr");
			exit(-1); 
		} 
        
		//bind function assigns address to unbound socket
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;  // Success
		}
		else {
			perror("bind");
		}
        close(sfd);
    }

    freeaddrinfo(result);  // No longer needed

    if (rp == NULL) {  // No address succeeded
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    // Read datagrams and echo them back to sender
	socklen_t peer_addr_len;
	struct sockaddr_storage peer_addr; 
	ssize_t nread; //ssize_t used for count of bytes / error indication
	char buf[BUF_SIZE];
    for (;;) {
        peer_addr_len = sizeof(peer_addr);
        nread = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (nread == -1)
            continue;  // Ignore failed request 

		
        char host[NI_MAXHOST], service[NI_MAXSERV];

        s = getnameinfo((struct sockaddr *) &peer_addr,
                peer_addr_len, host, NI_MAXHOST,
                service, NI_MAXSERV, NI_NUMERICSERV);
                
        if (s == 0) {
           printf("Presence: online %s on host host: %s \n", buf, host);
           sleep(2);
           printf("Presence: offline %s on host host: %s \n", buf, host); 
        }
        else {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s)); 
        }

		//sendto function sends message on socket
        if (sendto(sfd, buf, nread, 0,(struct sockaddr *) &peer_addr, peer_addr_len) != nread) {
            fprintf(stderr, "Error sending response\n");
        }
    }

    close(sfd); 
}
