#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define MAXBUFSIZE	(512)

int main (int argc, char * argv[] )
{

	int file_size[1], len;
	int sockfd;                           //This will be our socket
	struct sockaddr_in sin, remote;     //"Internet socket address structure"
	unsigned int remote_length;         //length of the sockaddr_in structure
	int nbytes;                        //number of bytes we receive in our message
	char buffer[MAXBUFSIZE];             //a buffer to store our received message
	FILE *fp;
	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	memset(&sin,0, sizeof(sin));                    //zero the struct
	sin.sin_family = AF_INET;                   //address family
	sin.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	sin.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine


	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create server socket");
	}


	/******************
	  Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("unable to bind socket\n");
	}

	remote_length = sizeof(remote);

	//waits for an incoming message
	// bzero(buffer,sizeof(buffer));
	// nbytes = recvfrom(sockfd, buffer, (int)MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_length);

	// printf("The client says %s\n", buffer);
	// printf("server: number of bytes sent: %d\n", nbytes);


	char msg[100];
	// nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));

	memset(buffer,0, sizeof(buffer));
	nbytes = recvfrom(sockfd, buffer, (int)MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_length);
	printf("Client: %s\n", buffer);

	strcpy(msg, "Okay");
	nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));

	memset(file_size, 0, sizeof(file_size));
	nbytes = recvfrom(sockfd, file_size, sizeof(file_size), 0, (struct sockaddr *)&remote, &remote_length);
	printf("The size of the file to be received is %d\n", file_size[0]);
	len = file_size[0];
	

	memset(buffer, 0, sizeof(buffer));
	printf("Writing received data in hello_server.txt\n");
	fp = fopen("hello_server.txt", "w");
	while(len > 0)
	{
		nbytes = recvfrom(sockfd, buffer, (int)MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_length);
		// printf("%s", buffer);
		fputs(buffer, fp);
		len = len - strlen(buffer);
		memset(buffer, 0, sizeof(buffer));
		printf("%d\n", len);	}

	close(sockfd);
}

