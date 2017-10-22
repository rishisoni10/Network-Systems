#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <error.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>	//inet_addr

#define	NUM_CLIENTS		(3)
#define BUFSIZE			(8096)			//4KB
char *ROOT;
FILE *fp;

struct sockaddr_in server, client;
int sockfd, new_sockfd, c, *new_sockfd2, i;
char *msg, PORT[6], client_reply[200];
int clients[NUM_CLIENTS];
long ret;
char buffer[BUFSIZE+1], local_buffer[BUFSIZE+1], *substring, *file_name;
char *data_buffer;
int count = 0, file_len;
size_t len;

void signal_handler(int signum)
{
	if(signum == SIGINT)
	{
		printf("Cleaning up...\n");
		if (fp != NULL)
		{	
			fclose(fp);
			printf("File closed\n");
		}
		else
			printf("File is already in closed state\n");

	    printf("Exiting....\n");
		close(new_sockfd);
	    close(sockfd);
	    exit(0);
	}
}


void server_response(int new_sockfd)
{
	ret = recv(new_sockfd, buffer, BUFSIZE, 0);
	if(ret == 0 || ret == -1)
	{
		perror("browser request read failed");
		exit(1);
	}

	//Extracting just the HTTP request from client
	for (i = 0; i < ret; i++)
	{
		if(buffer[i] == '\n')
		{
			local_buffer[i] = '\0';
			break;
		}
		else 
		{
			local_buffer[i] = buffer[i];
		}
	}
	
	if(strstr(local_buffer, "GET"))
	{
		// printf("GET found\n");
		substring = strcasestr(local_buffer, "index.html");
		// if (substring)
		// {
		// 	printf("index.html requested\n");
		// 	len = local_buffer - substring;
		// 	string2 = malloc(len + 1);
		// 	memcpy(string2, local_buffer, len);
		// 	string2[len] = 0;
		// }
		file_name = malloc(10);
		// printf("Copying file name\n");
		memcpy(file_name, substring, 10);
		printf("%s", file_name);
		fp = fopen(file_name, "r");		//open file in read mode
		
		if(fp == NULL)
		{
			perror("File does not exist");
			exit(1);
		}

		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// printf("%d\n", file_len);
		data_buffer = malloc(file_len);
		fread(data_buffer, 1, file_len, fp);

		// printf("%s", data_buffer);

		send(new_sockfd, "HTTP/1.1 200 OK\n", 16, 0);
		send(new_sockfd, "Content-Type: text/html\n", 24, 0);
		send(new_sockfd, "Content-Length: 3391\n\n", 22, 0);
		send(new_sockfd, data_buffer, 3391, 0);
	}
}

int main(int argc, char const *argv[])
{
	//Registering signal handler
	signal(SIGINT, signal_handler);

	/*** Parse from ws.conf file instead ***/
	ROOT = getenv("PWD"); //record current working directory
	strcpy(PORT, "80");

	//Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("Could not create socket");
		exit(1);
	}

	//Initialize socket parameters
	server.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(8081);

	//Bind socket to port and IP address
	if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("bind failed");
		exit(1);
	}

	//Listen
	listen(sockfd, (int)NUM_CLIENTS);

	//Accept incoming connection
	printf("Waiting for incoming connection\n");
	c = sizeof(struct sockaddr_in);
	memset(buffer, 0, sizeof(buffer));
	memset(local_buffer, 0, sizeof(local_buffer));
	new_sockfd = 1;
	
	while(new_sockfd)
	{
		new_sockfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&c);
		
		if(fork() == 0)
		{
			close(sockfd);		//child closes listening socket
			server_response(new_sockfd);
			close(new_sockfd);
			exit(0);
		}

		close(new_sockfd);	
		// printf("Response sent\n");
		// memset(buffer, 0, sizeof(buffer));
		// printf("Memory cleared\n");

	}
	return 0;
}
