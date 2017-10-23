#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>	//inet_addr

#define	NUM_CLIENTS		(1000)
#define BUFSIZE			(8096)			//4KB

int multi_clients[NUM_CLIENTS];
FILE *fp;

struct sockaddr_in server, client;
int sockfd, new_sockfd, c, *new_sockfd2;
int clients[NUM_CLIENTS];
int process_index;

void alarm_handler(int sig)
{
	if(sig == SIGALRM)
	{
		// signal(signum, alarm_handler);
	    printf("Socket Timed out\n");
	    printf("closing the socket %d\n",process_index); 
	    shutdown (multi_clients[process_index], SHUT_RDWR);         
	    close(multi_clients[process_index]);
	    multi_clients[process_index] = -1;
	    exit(0);    
	}

}

void signal_handler(int signum)
{
	if(signum == SIGINT)
	{
		printf("\nCleaning up...\n");
		if (fp != NULL)
		{	
			fclose(fp);
			printf("File closed\n");
		}
	    printf("Exiting....\n");
		close(multi_clients[process_index]);
	    close(sockfd);
	    // fclose(conf_fp);
	    exit(0);
	}
}

int file_size(FILE *file_fp)
{
	fseek(file_fp, 0, SEEK_END);
	int len = ftell(file_fp);
	fseek(file_fp, 0, SEEK_SET);
	return len;
}

void http_server_send(int sock_desc, char *server_msg, FILE *send_fp)
{
	char data_buffer[BUFSIZE];
	printf("Server msg:\n");
	int data_bytes;
	printf("%s\n", server_msg);
	int len = strlen(server_msg);
	if(send(sock_desc, server_msg, len, 0) == -1)
		printf("Send failed\n");
	
	// int file_len = file_size(send_fp);
	// data_buffer = malloc(file_len + 1);

	// fread(data_buffer, 1, file_len, send_fp);
	while((data_bytes = fread(data_buffer, 1, (int)BUFSIZE, send_fp))>0)
		send(sock_desc, data_buffer, (int)BUFSIZE, 0);
}

char* config_file(char *string)
{
	FILE *conf_fp;
	char filename[] = "ws.conf";
	// char buffer[4000];
	char *buffer;
	char *param, *tk;
	conf_fp = fopen(filename, "r");
	int len = file_size(conf_fp);
	buffer = malloc(len);
	fread(buffer, 1, len, conf_fp);
	if((param = strstr(buffer,string)) != NULL)
	{
		tk = strtok(param," \t\n");
		tk = strtok(NULL," \t\n");
	}
	fclose(conf_fp);
	return tk;
}

char* http_file_format(char *hint_string, char *search_string)
{
	char filename[] = "ws.conf";
	char *param, *tk;
	char *parse;
	FILE *conf_fp;
	char buffer[100000] = {0};

	conf_fp = fopen(filename, "r");
	if (conf_fp == NULL)
		printf("Error. Could not open file\n");

	int len = file_size(conf_fp);
	fread(buffer, 1, len, conf_fp);

	if((param = strstr(buffer, hint_string)) != NULL)
	{
		parse = strstr(param, search_string);
		tk = strtok(parse, " \t\n");
		tk = strtok(NULL, " \t\n");
	}
	fclose(conf_fp);
	return tk;
}

void server_response(int n, char* ROOT)
{
	char size[10], buffer[99999], *reqline[3], path[10000], hdr[10000], msg[99999], post_msg[99999]; 	//String parsing using an array of three char pointers (*reqline[3])
	int ret, len;
	FILE *res_fp;
	char def_page[30], file_name[1000], error_msg[1000], always_connect[1000000];
	char *token = malloc(30);
	char *file_type = malloc(40);
	char *content_type = malloc(50);
	char *http;
	char delimiter[] = ".";
	char *time_out_str = NULL;

	memset(&buffer, 0, sizeof(buffer));
	ret = recv(multi_clients[n], buffer, sizeof(buffer), 0);
	if(ret < 0)
	{
		printf("recv() error\n");
	}

	else
	{
		printf("%s", buffer);
		strcpy(msg, buffer);
		strcpy(post_msg, buffer);

		strcpy(always_connect, buffer);
		reqline[0] = strtok(buffer, " \t\n");
		// printf("reqline0: %s\n", reqline[0]);
		if(!strncmp(reqline[0], "GET\0", 4))
		{
			printf("GET Found\n");
			reqline[1] = strtok(NULL," \t");			//extract file 
			// printf("reqline1: %s\n", reqline[1]);
			reqline[2] = strtok(NULL," \t\n");			//extract HTTP version 
			// printf("reqline2: %s\n", reqline[2]);
			if (!strncmp(reqline[2], "HTTP/1.0", 8))
            {
              http = "HTTP/1.0 "; 
            }
            if(!strncmp(reqline[2], "HTTP/1.1", 8))
			{
               http = "HTTP/1.1 ";
               printf("Client request HTTP version is:%s\n", http);
            }

            //400 error: unsupported HTTP version is specified, 
            if(strncmp(reqline[2], "HTTP/1.0", 8) && strncmp(reqline[2], "HTTP/1.1", 8))
            {
            	printf("Client requests unsupported HTTP version\n");
            	// strcpy(hdr,http);
				sprintf(hdr,"HTTP/1.1 400 Bad Request\r\nContent-Type: text/html;\r\n\r\n<!DOCTYPE html>\r\n \
  					<html><head><title>Bad Request!</title></head>\r\n<body> \
  					<p><b>400: Bad Request. Reason - Invalid HTTP-version</b></p>\r\n \
  					</body></html>\r\n");
				send(multi_clients[n], hdr, strlen(hdr), 0);
            }
            else
            {
            	//If no page is specified, load default page
            	if(!strncmp(reqline[1], "/\0", 2))
            	{
            		printf("loading default page\n");
            		strcpy(def_page, "/");
            		char *tmp_ptr = config_file("DirectoryIndex");
            		//printf("Copied file is %s\n", tmp_ptr);
            		*(tmp_ptr + 10) = '\0';
            		strcat(def_page,tmp_ptr);
            		// printf("Concat file is %s\n", def_page);
            		strcpy(reqline[1], def_page);
            		// printf("reqline1 new: %s\n", reqline[1]);
            	}
            	strcpy(file_name, reqline[1]);
            	// printf("File name is %s\n", file_name);
            	strncpy(path, ROOT, strlen(ROOT));
            	strcat(path, file_name+1);
            	// printf("Path: %s\n", path);
            	strcpy(file_type, path);
            	token = strtok(file_type, ".\0 ");	//Finding the requested file type
            	token = strtok(NULL, ".\0");
            	// printf("file type is %s\n", token);

            	file_type = strcat(delimiter, token);
            	// printf("Final file format is:%s\n", file_type);
            	content_type = http_file_format("#Content-Type which the server handles", file_type);		//Checking whether requested file type is supported by server
            	// printf("content_type is %s\n", content_type);

            	res_fp = fopen(path, "r");
            	if(res_fp != NULL)
            	{
            		printf("File FOUND!\n");
            		strcpy(hdr, http);
            		strcat(hdr, "200 OK\n");
            		len = file_size(res_fp);
            		snprintf(size, sizeof(size), "%d", len); 		//Convert integer to string wit(hout itoa()
            		 // printf("Size in string is%s\n", size);
            		// sprintf(hdr, "Content-Size :%s\nContent-Type : %s\n\n", size, content_type);
            		strcat(hdr, "Content-Size :");
            		strcat(hdr, size);
            		strcat(hdr, "\n");
            		strcat(hdr, "Content-Type : ");
            		strcat(hdr, content_type);
            		strcat(hdr, "\n\n");
            		printf("hdr is %s\n", hdr);
            		http_server_send(multi_clients[n], hdr, res_fp);			//Sending header + data to client
            		fclose(res_fp);
            	}
            	
            	//error 404: File Not Found
            	else
				{
					printf("File NOT FOUND! :(\n");
					strcpy(hdr, http);
					strcat(hdr, "404 Not Found\nContent-Size : NONE\nContent-Type : Invalid\n\n");
					// strcat(hdr, "Content-Size : NONE\nContent-Type : Invalid\n\n");
					send(multi_clients[n], hdr, strlen(hdr), 0);
					strcpy(error_msg,"<HEAD><TITLE>404 File not found Reason</TITLE></HEAD>\n<html><BODY>404 File not found Request URL doesn't exist:");
					strcat(error_msg,path);
					// strcat(error_msg,"\n");
					strcat(error_msg,"\n</BODY></html>");
					send(multi_clients[n], error_msg, strlen(error_msg), 0);
            	}
            }

		}
		else if(!strncmp(reqline[0], "POST\0", 4))
		{
			printf("Post request\n");
			reqline[1] = strtok(NULL," \t");
			strcpy(file_name, reqline[1]);
        	// printf("File name is %s\n", file_name);
        	strncpy(path, ROOT, strlen(ROOT));
        	strcat(path, file_name+1);
        	// printf("Path: %s\n", path);
        	strcpy(file_type, path);
        	token = strtok(file_type, ".\0 ");
        	token = strtok(NULL, ".\0");
        	// printf("file type is %s\n", token);

        	file_type = strcat(delimiter, token);
        	// printf("Final file format is:%s\n", file_type);
        	content_type = http_file_format("#Content-Type which the server handles", file_type);
        	// printf("content_type is %s\n", content_type);

        	res_fp = fopen(path, "r");
        	if(res_fp != NULL)
        	{
        		printf("File FOUND!\n");
        		strcpy(hdr, http);
        		strcat(hdr, "200 OK\n");
        		len = file_size(res_fp);
        		snprintf(size, sizeof(size), "%d", len); //Convert interger to string
        		// printf("Size in string is%s\n", size);
        		strcat(hdr, "Content-Size :");
        		strcat(hdr, size);
        		strcat(hdr, "\n");
        		strcat(hdr, "Content-Type : ");
        		strcat(hdr, content_type);
        		// strcat(hdr, "\n\n");
	    		strcat(hdr, "\n\n<html><body><pre><h1>POSTDATA </h1></pre>");
        		http_server_send(multi_clients[n], hdr, res_fp);
        		fclose(res_fp);
            }

		}
		//501 error: Client requested method is not supported by server
		 else if (strcmp(reqline[0], "POST") && strcmp(reqline[0],"GET"))
        {
        	printf("Requested method %s not implemented\n", reqline[0]);
			memset(hdr, 0, sizeof(hdr));
			strcat(hdr, reqline[0]);
			strcat(hdr, " 501 Not Implemented\r\nContent-Type: text/html;\r\n\r\n<!DOCTYPE html>\r\n \
  					<html><head><title>Not Implemented method</title></head>\r\n<body> \
  					<p><b>501: Not Implemented method on server</b></p>\r\n \
  					</body></html>\r\n");
			send(multi_clients[n], hdr, strlen(hdr), 0);
        }
        // 500 error: Cannot allocate memory
        else
        {
        	memset(hdr, 0, sizeof(hdr));
			strcpy(hdr, "500 Internal Server Error\r\nContent-Type: text/html;\r\n\r\n<!DOCTYPE html>\r\n \
			<html><head><title>Aww, snap!</title></head>\r\n<body> \
			<p><b>500: Internal Server Error - Cannot allocate memory</b></p>\r\n \
			</body></html>\r\n");
			send(multi_clients[n], hdr, strlen(hdr), 0);
			exit(1);
        }

        //timeouts
         if(strstr(always_connect,"Connection: keep-alive")!= NULL)
        {
    	    // signal(SIGALRM, alarm_handler);
        	printf("Client requests to keep connection alive\n");
        	time_out_str = config_file("Keep-Alivetime");	//Extract server timeout from config file
        	printf("Timeout set is %d\n",atoi(time_out_str));
        	// printf("timer started\n");
        	int timeout = atoi(time_out_str);
        	alarm(timeout);
        }
        else
        {
      	  //Closing SOCKET
    		shutdown (clients[n], SHUT_RDWR);         
    		close(clients[n]);
        	clients[n]=-1;
        	exit(0);
        }
	}
}

int main(int argc, char const *argv[])
{
	//Registering signal handler
	signal(SIGINT, signal_handler);

	/*** Parsing from ws.conf file***/
	char *ROOT = malloc(100);
	char *PORT = malloc(16);
	char *temp = malloc(7);
	
	strcpy(temp, config_file(" "));
	strcpy(ROOT, config_file("DocumentRoot"));
	strcpy(PORT, config_file("Listen"));
	
	printf("Server started at port %s ", PORT);
	printf("with root directory %s\n", ROOT);
	
	int server_port = atoi(PORT);
	printf("Port number is %d\n", server_port);

    // signal(SIGALRM, timeout_handler);

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
	server.sin_port = htons(server_port);

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

	// Setting all elements to -1: Signifies that there are no clients connected
	for (int i = 0; i < NUM_CLIENTS; ++i)
	{
		multi_clients[i] = -1;
	}
    signal(SIGALRM, alarm_handler);


	process_index = 0;
	
	//Accept connections until terminated
	while(1)
	{
		multi_clients[process_index] = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&c);
		
		if(multi_clients[process_index] < 0)
		{
			perror("accept error");
		}
		else
		{
			if(fork() == 0)
		{
			// close(sockfd);		//child closes listening socket
			server_response(process_index, ROOT);
			// close(new_sockfd);
			exit(0);
		}
		// close(new_sockfd);	
		close(multi_clients[process_index]);
		}
		while(multi_clients[process_index] != -1)
			process_index = (process_index + 1) % NUM_CLIENTS;
	}
	return 0;
}
