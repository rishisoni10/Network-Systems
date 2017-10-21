#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <error.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>	//inet_addr

#define	NUM_CLIENTS		(3)
#define BUFSIZE			(8096)			//4KB
char *ROOT;
FILE *fp;

int main(int argc, char const *argv[])
{
	struct sockaddr_in server, client;
	int sockfd, new_sockfd, c, *new_sockfd2, i;
	char *msg, PORT[6], client_reply[200];
	int clients[NUM_CLIENTS];
	long ret;
	char buffer[BUFSIZE+1], local_buffer[BUFSIZE+1], *substring, *file_name;
	char *data_buffer;
	int count = 0, file_len;
	size_t len;

	//Parse from ws.conf file instead
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
	
	while((new_sockfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&c)))
	{
		fflush(stdin);
		// printf("Connection accepted\n");
		ret = read(new_sockfd, buffer, BUFSIZE);
		if(ret == 0 || ret == -1)
		{
			perror("browser request read failed");
			exit(1);
		}

		//Extracting HTTP request from client
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
		
		// printf("%s", local_buffer);
		// while(count != i)
		// {
		// 	printf("%c", local_buffer[count]);
		// 	// printf("%s", local_buffer);
		// 	// printf("Loop\n");
		// 	count++;
		// 	// i--;
		// }

		if(strstr(local_buffer, "GET"))
		{
			// printf("GET found\n");
			substring = strstr(local_buffer, "index.html");
			// if (substring)
			// {
			// 	printf("index.html requested\n");
			// 	len = local_buffer - substring;
			// 	string2 = malloc(len + 1);
			// 	memcpy(string2, local_buffer, len);
			// 	string2[len] = 0;
			// }
			file_name = malloc(10);
			memcpy(file_name, substring, 10);
			printf("%s", file_name);
	     
	        // file_name[strcspn(file_name, " ")] = 0;
			fp = fopen(file_name, "r");		//open file in read mode
			
			if(fp == NULL)
			{
				perror("File does not exist");
				exit(1);
			}

			fseek(fp, 0, SEEK_END);
   			file_len = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			printf("%d\n", file_len);
			data_buffer = malloc(file_len);
			fread(data_buffer, 1, file_len, fp);

			printf("%s", data_buffer);

			write(new_sockfd, "HTTP/1.1 200 OK\n", 16);
			write(new_sockfd, "Content-Type: text/html\n", 24);
			write(new_sockfd, "Content-Length: 3391\n\n", 22);
			write(new_sockfd, data_buffer, 3391);
		}
		break;
		// write(new_sockfd, msg, strlen(msg));
	}

	if (new_sockfd < 0)
	{
		perror("accept failed");
		exit(1);
	}

	// printf("Connection accepted\n");
	close(sockfd);
	close(new_sockfd);
	// sleep(1);
	return 0;
}
