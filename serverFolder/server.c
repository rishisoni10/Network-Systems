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

#define MAXBUFSIZE	(143)

int isfilepresent(const char *filename);

//Structure for transmitter and receiver packet. 1 packet = 576 bytes (MTU for IPv4)
typedef struct 
{
	int index;
	int buffer[MAXBUFSIZE];
}PACKET;


int main (int argc, char * argv[] )
{
	char file_name[10];
	char msg[100];
	int file_size[1], len;
	int sockfd;                           //This will be our socket
	struct sockaddr_in sin, remote;      //"Internet socket address structure"
	unsigned int remote_length;         //length of the sockaddr_in structure
	int nbytes;                        //number of bytes we receive in our message
	void *buffer;             //a buffer to store our received message
	PACKET pkt;
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

	// while(1)
	// {
	// 	memset(msg,0, sizeof(msg));
	// 	nbytes = recvfrom(sockfd, msg, (int)MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_length);
	// 	printf("Client: %s\n", msg);	
	// 	if(strstr(msg, "get") != NULL)
	// 	{
	// 		strcpy(file_name, (msg + 3));
	// 		// printf("File name is %s\n", file_name);
	// 		// result = isfilepresent(file_name);
	// 		fp = fopen(file_name, "r");
	// 		if(fp == NULL)
	// 		{
	// 			perror("could not open file");
	// 			exit(1);
	// 		}

	// 		fseek(fp, 0, SEEK_END);
	// 		file_size[0] = ftell(fp);
	// 		// sbytes = sendto(sockfd, file_size, (sizeof(file_size) + 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
	// 		fseek(fp, 0, SEEK_SET);
	// 		buffer = malloc(sizeof(file_size[0]));
	// 		fread(buffer, 1, file_size[0], fp);
	// 		printf("Sending requested file\n");
	// 		sbytes = sendto(sockfd, buffer, file_size[0], 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));

	// }
	memset(msg,0, sizeof(msg));
	// nbytes = recvfrom(sockfd, msg, (int)MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_length);
	nbytes = recvfrom(sockfd, msg, sizeof(pkt), 0, (struct sockaddr *)&remote, &remote_length);
	printf("Client: %s\n", msg);

	memset(msg,0, sizeof(msg));
	strcpy(msg, "Okay");
	nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));

	memset(file_size, 0, sizeof(file_size));
	nbytes = recvfrom(sockfd, file_size, sizeof(file_size), 0, (struct sockaddr *)&remote, &remote_length);
	printf("The size of the file to be received is %d\n", file_size[0]);
	len = file_size[0];
	
	buffer = malloc(len);		//Allocating a buffer large enough for incoming file
	printf("Size of buffer is %ld\n", sizeof(buffer));
	printf("Its length is %d\n", len);
	printf("Writing received data in foo2\n");
	fp = fopen("foo1", "w");

	
	nbytes = recvfrom(sockfd, buffer, len, 0, (struct sockaddr *)&remote, &remote_length);
	fwrite(buffer, 1, len, fp);

	close(sockfd);
}

int isfilepresent(const char *filename)
{

}
