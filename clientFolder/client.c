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

#define MAXBUFSIZE	(143)

//Structure for transmitter and receiver packet. 1 packet = 576 bytes (MTU for IPv4)
typedef struct 
{
	int index;
	int buffer[MAXBUFSIZE];
}PACKET;

int main (int argc, char * argv[])
{	
	int prev_index;
	long num, num_bytes;
	char command[100];
	char data[512];
	int len;
	int errsv;									//Store errno
	int sbytes = -1;                             // number of bytes send by sendto()
	int rbytes = -1;                             // number of bytes send by recvfrom()
	int sockfd;                               //this will be our socket
	void *buffer;
	PACKET *pkt = NULL;;
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
		exit(1);
	}

	struct sockaddr_in from_addr;
	socklen_t addr_length = sizeof(struct sockaddr);
	
	memset(command, 0, sizeof(command));
	printf("Size of each packet is %ld\n", sizeof(pkt->buffer) + sizeof(pkt->index));

	// while(1)
	// {
	// 	printf("\r\n     *** Main Menu ***\n\r");
	// 	printf("Enter the following commands for file transfers / handling\n");
 //        printf("get <file_name>\n");
 //        printf("put <file_name>\n");
 //        printf("delete <file_name>\n");
 //        printf("ls\n");
 //        printf("exit\n");
 //        printf("Type in the command followed by the <file_name>, if the command requires\n");
 //        // scanf("%s", command);
 //        // gets(command);
 //        fgets(command, sizeof(command), stdin);
 //        printf("Sending entered command to server\n");
 //    	sbytes = sendto(sockfd, command, (sizeof(command) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));



	// }

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
		fp_temp = fopen("foo2_temp", "w");


		if(fp == NULL)
		{
			perror("could not open file");
			exit(1);
		}

	   	fseek(fp, 0, SEEK_END);
   		len = ftell(fp);
		printf("Length of file is %d bytes\n", len);
		sbytes = sendto(sockfd, &len, (sizeof(len) + 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
		fseek(fp, 0, SEEK_SET);
		printf("Size of data buffer is %ld\n", sizeof(pkt->buffer));
		// *buffer = malloc(sizeof(int) * (int)MAXBUFSIZE);
		pkt = (PACKET *)malloc(sizeof(pkt->buffer) + sizeof(pkt->index));
		num = (len / (sizeof(int)));
		// printf("Number of packets needed is %ld\n", num);
		pkt->index = 0;
		while(len > 0)
		{
			prev_index = pkt->index;
			printf("Entered while loop\n");
			pkt->index++;
			num_bytes = fread(pkt->buffer, 1, (int)MAXBUFSIZE, fp);
			// printf("File read done\n");
			if(pkt->index > prev_index)
			{
				fwrite(pkt->buffer, 1, num_bytes, fp_temp);
			}
			// printf("File write done\n");
			memset(pkt->buffer, 0, (int)MAXBUFSIZE);
			len = len - num_bytes;
		}
		// fread(buffer, 1, len, fp);
		// printf("Number of bytes read: %d\n",i);		
		// fwrite(buffer, 1, len, fp_temp);
		// sbytes = sendto(sockfd, buffer, &len, 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));

	}

	close(sockfd);
}