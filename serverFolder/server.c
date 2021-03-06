/*
* @file server.c
* @brief UDP Server file
*
* This source file provides UDP/IP connection with a remote / local client for 
* file transfer. A reliability protocol has been applied over UDP's unreliable
* packet transmission protocol
*
* Tools used: GCC Compiler, GDB
* Command to compile from source: make
*
*
* @author Rishi Soni
* @date September 24 2017
* @version 1.0
*
*/

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
#include <time.h>

#define MAXBUFSIZE	(1024)

void put_file(char *, int, struct sockaddr_in);
void get_file(char *, int, struct sockaddr_in);
void list_directory(int, struct sockaddr_in);
void delete(char *);



//Structure for transmitter and receiver packet. 1 packet = 576 bytes (MTU for IPv4)
typedef struct 
{
	int index;
	int data_length;
	char buffer[MAXBUFSIZE];
}PACKET;

struct timeval timeout;
char file_name[10];
int pkt_size, key_len; 
char key[] = "123456789098765432123456789098765432123456789098765432123456789";
int pkt_size;
int file_size;
struct sockaddr_in remote;      //"Internet socket address structure"
unsigned int remote_length;         //length of the sockaddr_in structure
int nbytes;                        //number of bytes we receive in our message
PACKET *pkt = NULL, *pkt_ack = NULL;
FILE *fp;

int main (int argc, char * argv[] )
{
	int sockfd;                           //This will be our socket
	struct sockaddr_in sin;
	char msg[100];

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
	pkt_size = sizeof(pkt->buffer) + sizeof(pkt->index) + sizeof(pkt->data_length);
	int key_len = strlen(key);
	while(1)
	{
		printf("Server menu:\n");
		nbytes = recvfrom(sockfd, msg, (int)MAXBUFSIZE, 0, (struct sockaddr *)&remote, &remote_length);
		printf("Client: %s\n", msg);	
		if(strstr(msg, "get") != NULL)
		{
			printf("Found command\n");
			strcpy(file_name, (msg + 4));
			memset(msg,0, sizeof(msg));
			strcpy(msg, "Sending file");
			strcat(msg, file_name);
			printf("Sending the following string: %s\n", msg);
			nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
			get_file(file_name, sockfd, remote);
		}
		else if(strstr(msg, "put") != NULL)
		{
			printf("Found command\n");
			strcpy(file_name, (msg + 3));
			memset(msg,0, sizeof(msg));
			strcpy(msg, "Send file");
			strcat(msg, file_name);
			printf("Sending the following string: %s\n", msg);
			nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
			put_file(file_name, sockfd, remote);
		}

		else if(strstr(msg, "ls") != NULL)
		{
			printf("Found command\n");
			memset(msg,0, sizeof(msg));
			strcpy(msg, "Listing server directory contents");
			printf("Sending the following string: %s\n", msg);
			nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
			list_directory(sockfd, remote);
		}	

		else if(strstr(msg, "delete") != NULL)
		{
			printf("Found command\n");
			strcpy(file_name, (msg + 7));
			memset(msg,0, sizeof(msg));
			strcpy(msg, "Deleting ");
			strcat(msg, file_name);
			printf("Sending the following string: %s\n", msg);
			nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
			delete(file_name);
		}	

		else if(strstr(msg, "exit") != NULL)
		{
			memset(msg, 0, sizeof(msg));
			strcpy(msg, "Server is exiting");
			nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
			printf("Server stops\n");
			break;
		}
		else 
		{
			nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
			strcpy(msg, "Given command was not understood by server");
			printf("Given command was not understood by server\n");
			nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));			
		}
	}
	close(sockfd);
}

/**
* @brief Sends file to client
*
* Given a file name, this function sends the file to the client
*
* @param *file_name     File name pointer
*		 sockfd			Socket ID
*        remote         Socket parameters
*
* @return void 
*/
void get_file(char *file_name, int sockfd, struct sockaddr_in remote)
{
	char msg[100];
	int index_req, errsv, count, sent_index;
	long num_pkts, num_bytes;
	remote_length = sizeof(remote);
	nbytes = recvfrom(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&remote, &remote_length);
	if(strstr(msg, "Client Ready"))
	{
		printf("File name is %s\n", file_name);
		memset(msg, 0, sizeof(msg));
		fp = fopen(file_name, "r+");

		if(fp == NULL)
		{
			errsv = errno;
			printf("ERRNO is %d\n", errsv);
			perror("Could not open file");
			exit(1);
		}

		fseek(fp, 0, SEEK_END);
		file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		printf("Length of file is %d bytes\n", file_size);
		nbytes = sendto(sockfd, &file_size, (sizeof(file_size) + 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
		printf("Size of data buffer is %ld\n", sizeof(pkt->buffer));
		pkt = (PACKET *)malloc(pkt_size);
		pkt_ack = (PACKET *)malloc(pkt_size);

		num_pkts = file_size / pkt_size;
		printf("Number of packets needed is %ld\n", num_pkts);

		pkt->index = 0;
		count = 0;
		int loop_count = 0;


		//Setting timeout using setsockopt()
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;		//500ms timeout
		setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

		// Loop till entire file is transmitted
		while(file_size > 0)
		{
			loop_count = 0;

			pkt->index++;
			sent_index = pkt->index;

			//64-bit encryption. XOR every byte
			while(loop_count<pkt_size)
			{
				pkt->buffer[loop_count] ^= key[loop_count % (key_len-1)];
				++loop_count;
			}

			num_bytes = fread(pkt->buffer, 1, (int)MAXBUFSIZE, fp);
			pkt->data_length = num_bytes;

			nbytes = sendto(sockfd, pkt, pkt_size, 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
			printf("Sent index is %d\n", pkt->index);
			memset(pkt_ack, 0, pkt_size);
			nbytes = recvfrom(sockfd, pkt_ack, pkt_size, 0, (struct sockaddr *)&remote, &remote_length);
            
            //Loop till recvfrom error OR received packet != expected packet conditions are resolved
			while(nbytes < 0 || pkt_ack->index !=sent_index)
			{
				errsv = errno;
				memset(pkt_ack, 0, pkt_size);
				printf("The error number is %d\n", errsv);
				//Sending data packet again if timeout occurs
				nbytes = sendto(sockfd, pkt, pkt_size, 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
				printf("Send index is %d\n", pkt->index);
				nbytes = recvfrom(sockfd, pkt_ack, pkt_size, 0, (struct sockaddr *)&remote, &remote_length);
		
			}

			//Loop exits if correct ACK is received
			printf("ACK received\n");
			file_size = file_size - num_bytes;
			count++;
		}
		
		// fclose(fp);
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	}
	printf("Number of ACKs is %d\n", count);
}

/**
* @brief Receives file from server
*
* Given a file name, this function receives the file from the client
*
* @param *file_name     File name pointer
*        sockfd         Socket ID
*        remote         Socket parameters
*
* @return void 
*/
void put_file(char *file_name, int sockfd, struct sockaddr_in remote)
{
	char msg[100];
	int index_req, old_index;
	remote_length = sizeof(remote);
	memset(msg,0, sizeof(msg));
	nbytes = recvfrom(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&remote, &remote_length);
	printf("put function at server receives: %s\n", msg);

	if(!strcmp(msg, "Sending file"))
	{
		printf("Client reply received\n");
		memset(msg,0, sizeof(msg));
		strcpy(msg, "Okay");
		printf("Sending ACK: %s\n", msg);
		nbytes = sendto(sockfd, msg, (sizeof(msg) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
	}

	memset(&file_size, 0, sizeof(file_size));
	nbytes = recvfrom(sockfd, &file_size, sizeof(file_size), 0, (struct sockaddr *)&remote, &remote_length);
	printf("The size of the file to be received is %ld\n", file_size);
	
	pkt_size = sizeof(pkt->buffer) + sizeof(pkt->index) + sizeof(pkt->data_length);
	pkt = (PACKET *)malloc(pkt_size);

	printf("Writing received data in %s\n", file_name);
	fp = fopen(file_name+1, "w");

	index_req = 1;
	int loop_count = 0;


	//Setting the timeout using setsockopt()
	// timeout.tv_sec = 0;
	// timeout.tv_usec = 500000;  //500ms timeout
	// setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	//Loop till entire file has been received
	while(file_size > 0)
	{
		loop_count = 0;
		nbytes = recvfrom(sockfd, pkt, pkt_size, 0, (struct sockaddr *)&remote, &remote_length);
		if(pkt->index == index_req)
		{
			printf("Correct data received\n");
			printf("Received index is %d\n", pkt->index);

			//64-bit decryption. double XOR every byte in packet to recover original data
			while(loop_count<pkt_size)
			{
				pkt->buffer[loop_count] ^= key[loop_count % (key_len-1)] ^ key[loop_count % (key_len-1)];
				++loop_count;
			}

			fwrite(pkt->buffer, 1, pkt->data_length, fp);
			file_size = file_size - pkt->data_length;
			printf("Current file size is %ld\n", file_size);
			memset(pkt, 0, pkt_size);
			pkt->index = index_req;
			nbytes = sendto(sockfd, pkt, pkt_size, 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
			index_req++;
		}

		//If old packet has been received, send ACK of old packet
		else if (pkt->index < index_req)
		{
			printf("Old pkt received\n");
			printf("Received index: %d\n", pkt->index);
			old_index = pkt->index;
			memset(pkt, 0, pkt_size);
			pkt->index = old_index;
			nbytes = sendto(sockfd, pkt, pkt_size, 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
		}
		
		//Else, index=0. This makes sender loop till it receives correct ACK
		else
		{
			printf("Incorrect data received. Send data again\n");
			// strcpy(pkt->buffer, "Incorrect index. Send again");
			printf("Received index: %d\n", pkt->index);
			memset(pkt, 0, pkt_size);
			pkt->index = 0;
			nbytes = sendto(sockfd, pkt, pkt_size, 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));
			// printf("Incorrect Received index is %d\n", pkt->index);
		}
		memset(pkt, 0, pkt_size);
	}
	// timeout.tv_sec = 0;
	// timeout.tv_usec = 0;
	// setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	fclose(fp);
	printf("Client to Server file transfer complete\n");
}

/**
* @brief Lists current directory contents of server
*
* Given the socket parameters, this function displays the server directory
* contents
*
* @param *file_name     File name pointer
*        sockfd         Socket ID
*        remote         Socket parameters
*
* @return void 
*/
void list_directory(int sockfd, struct sockaddr_in remote)
{
	printf("Entering ls function\n");
	FILE *fp_ls;
	fp_ls = fopen("ls_dir", "w+");
	if(fp_ls == NULL)
	{
		int errsv = errno;
		printf("ERRNO is %d\n", errsv);
		perror("could not open file");
		exit(1);
	}

	system("ls -la> ls_dir");
	
	//Sending the directory list as a file to the client
	get_file("ls_dir", sockfd, remote);

	//Deleting the ls text file
	delete("ls_dir");
}


/**
* @brief Deletes file on server
*
* This function deletes the requested file on the server. 
*
* @param *file_name     File name pointer
* 
* @return void 
*/
void delete(char *file_name)
{
	if (remove(file_name) == 0)
      printf(" %s deleted successfully\n", file_name);
   else
      printf("Unable to delete the file");
}