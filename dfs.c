
/*
* @file dfs.c
* @brief Distributed file system TCP/IP server source file
*
* This source file creates distributed file system server, which accepts the following commands 
* from the distributed file system clients: PUT, MKDIR, GET, LIST
*
* Tools used: GCC Compiler, GDB
* Command to compile from source: make all
* Command to run: make run
*
* @author Rishi Soni
* @date November 12 2017
* @version 1.0
*
*/
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
#include <arpa/inet.h>  //inet_addr

int file_size(FILE *file_fp)
{
  fseek(file_fp, 0, SEEK_END);
  int len = ftell(file_fp);
  fseek(file_fp, 0, SEEK_SET);
  return len;
}

char* config_file(char *string)
{
    // printf("Entered config func\n");
    FILE *conf_fp;
    char filename[] = "dfs.conf";
    // char buffer[4000];
    char *buffer;
    char *param, *tk;
    conf_fp = fopen(filename, "r");

    int len = file_size(conf_fp);
    buffer = malloc(len);
    fread(buffer, 1, len, conf_fp);
    if((param = strstr(buffer,string)) != NULL)
    {
    tk = strtok(param," \n");
    // tk = strtok(NULL,":\0");
    }
    // printf("Token is:%s\n", tk);
    fclose(conf_fp);
    return tk;
}

int main(int argc, char const *argv[])
{
  int welcomeSocket, newSocket;
  char buffer[1024];
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  int port;
  char* recv_username = malloc(100);
  char* recv_password = malloc(100);
  char* req_username = malloc(100);
  char* req_password = malloc(100);
  char* token = malloc(100);


  /*---- Create the socket. The three arguments are: ----*/
  welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
  if(argc < 2)
  {
  	printf("Enter port number of server\n");
  	exit(1);
  }

  printf("Value of port is %s\n", argv[1]);
  port = atoi(argv[1]);
  printf("Port:%d\n",port);

  
  /*---- Configure settings of the server address struct ----*/
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(port);
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*---- Bind the address struct to the socket ----*/
  bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  /*---- Listen on the socket, with 5 max connection requests queued ----*/
  while(1)
  {
	  if(listen(welcomeSocket,4)==0)
	    printf("Listening\n");
	  else
	    printf("Error\n");

	  /*---- Accept call creates a new socket for the incoming connection ----*/
	  addr_size = sizeof(serverStorage);
	  newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

	  /*---- Receive message from the socket of the incoming connection ----*/
	  
	  recv(newSocket,buffer,1024,0);

	  printf("Received info is %s\n", buffer);
      recv_username = strtok(buffer,",");
      recv_password = strtok(NULL,",");
      printf("Received user name:%s\n", recv_username);
      printf("Received Password:%s\n", recv_password);
	 
      req_username = config_file(recv_username);
      printf("Username token:%s\n", req_username);
     
      req_password = config_file(recv_password);
      printf("Password token:%s\n", req_password);


      // printf("\n\n");
      if(req_password == NULL || req_username == NULL)
      {
        printf("Wrong credentials. Try Again\n");
      }
      else if(!strcmp(recv_username, req_username) && !strcmp(recv_password, req_password))
        printf("Client credentials verified\n");
      else
        printf("Problem hain\n");



	  // memset(buffer, 0, sizeof(buffer));
	}

  return 0;
}