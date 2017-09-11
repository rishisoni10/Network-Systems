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
#include <errno.h>
#include <string.h>

#define MAXBUFSIZE	(512)


int main (int argc, char * argv[])
{
	char command[100];
	char data[512];
	int len[1];
	int errsv;									//Store errno
	int sbytes = -1;                             // number of bytes send by sendto()
	int rbytes = -1;                             // number of bytes send by recvfrom()
	int sockfd;                               //this will be our socket
	void *buffer;
	FILE *fp;
	FILE *fp_temp;

	struct sockaddr_in remote;              //"Internet socket address structure"

	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/
	memset(&remote,0, sizeof(remote));
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address


	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create client socket");
	}

	struct sockaddr_in from_addr;
	socklen_t addr_length = sizeof(struct sockaddr);
	// memset(buffer, 0, sizeof(buffer));



	strcpy(command, "Sending a text file");
	printf("%s\n", command);
	sbytes = sendto(sockfd, command, (sizeof(command) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
	
	memset(command, 0, sizeof(command));
	rbytes = recvfrom(sockfd, command, (int)MAXBUFSIZE, 0, (struct sockaddr *)&from_addr, &addr_length);  
	printf("Server: %s\n", command);
	// memset(command, 0, sizeof(command));


	if(strcmp(command, "Okay") == 0)
	{
		memset(command, 0, sizeof(command));
		// sbytes = sendto(sockfd, len, (sizeof(command) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
		fp = fopen("foo2", "r");		//open file in read mode
		fp_temp = fopen("foo1_temp", "w+");


		if(fp == NULL)
		{
			perror("could not open file");
			exit(1);
		}

	   	fseek(fp, 0, SEEK_END);
   		len[0] = ftell(fp);
		printf("Length of file is %d bytes\n", len[0]);
		sbytes = sendto(sockfd, len, (sizeof(len) + 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
		fseek(fp, 0, SEEK_SET);
		// buffer = (char *)malloc(len[0]);
		buffer = malloc(len[0]);
		fread(buffer, 1, len[0], fp);
		// printf("Number of bytes read: %d\n",i);
		fwrite(buffer, 1, len[0], fp_temp);
		sbytes = sendto(sockfd, buffer, len[0], 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));

		// while(fgets(data, 512, fp) !=NULL)
		// {
		// 	sbytes = sendto(sockfd, data, (sizeof(data) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
		// 	// printf("%s", data);
		// 	memset(data, 0, sizeof(data));
		// }

	}

	close(sockfd);
}