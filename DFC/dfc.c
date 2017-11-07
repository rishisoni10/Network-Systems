/*
* @file dfc.c
* @brief Distributed file system TCP/IP client source file
*
* This source file creates distributed file system client, which sends the following commands 
* to the distributed file system server (DFS): PUT, MKDIR, GET, LIST
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
  FILE *conf_fp;
  char filename[] = "dfc.conf";
  // char buffer[4000];
  char *buffer;
  char *param, *tk;
  conf_fp = fopen(filename, "r");

  int len = file_size(conf_fp);
  buffer = malloc(len);
  fread(buffer, 1, len, conf_fp);
  if((param = strstr(buffer,string)) != NULL)
  {
    tk = strtok(param,":\n");
    tk = strtok(NULL,":\n");
  }
  // printf("Token is:%s\n", tk);
  fclose(conf_fp);
  return tk;
}


int main()
{
  int clientSocket, clientSocket_1, clientSocket_2, clientSocket_3, clientSocket_4;
  char buffer[1024], buffer1[1024], buffer2[1024], buffer3[1024], buffer4[1024];
  struct sockaddr_in serverAddr1, serverAddr2, serverAddr3, serverAddr4;
  socklen_t addr_size;
  int s1_port, s2_port, s3_port, s4_port;
  char* key = NULL;


  /*** Parsing from dfc.conf file***/    
  char* username = malloc(100);
  char* password = malloc(100);
  char* PORT1 = malloc(10);
  char* PORT2 = malloc(10);
  char* PORT3 = malloc(10);
  char* PORT4 = malloc(10);
  char* temp = malloc(7);
  
  // strcpy(temp, config_file(" "));
  // strcpy(ROOT, config_file("DocumentRoot"));
  strcpy(PORT1, config_file("DFS1"));
  strcpy(PORT2, config_file("DFS2"));
  strcpy(PORT3, config_file("DFS3"));
  strcpy(PORT4, config_file("DFS4"));
  strcpy(username, config_file("Username"));
  strcpy(password, config_file("Password"));
  strcat(username, ",");
  strcat(username, password);

  // key = (char*) malloc(strlen(username));
  // strcpy(key, username);
  // printf("strcpy is %s", key);


  printf("Port1:%s Port2:%s Port3:%s Port4:%s\n", PORT1, PORT2, PORT3, PORT4);

  s1_port = atoi(PORT1);
  s2_port = atoi(PORT2);
  s3_port = atoi(PORT3);
  s4_port = atoi(PORT4);

  /*---- Create the socket. The three arguments are: ----*/
  clientSocket_1 = socket(PF_INET, SOCK_STREAM, 0);
  clientSocket_2 = socket(PF_INET, SOCK_STREAM, 0);
  clientSocket_3 = socket(PF_INET, SOCK_STREAM, 0);
  clientSocket_4 = socket(PF_INET, SOCK_STREAM, 0);
 
  
  /*---- Configure settings of the server address struct ----*/
  serverAddr1.sin_family = AF_INET;
  serverAddr2.sin_family = AF_INET;
  serverAddr3.sin_family = AF_INET;
  serverAddr4.sin_family = AF_INET;

  /* Set port number, using htons function to use proper byte order */
  serverAddr1.sin_port = htons(s1_port);
  serverAddr2.sin_port = htons(s2_port);
  serverAddr3.sin_port = htons(s3_port);
  serverAddr4.sin_port = htons(s4_port);

  /* Set IP address to localhost for all servers */
  serverAddr1.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverAddr2.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverAddr3.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverAddr4.sin_addr.s_addr = inet_addr("127.0.0.1");

  /* Set all bits of the padding field to 0 */
  memset(serverAddr1.sin_zero, '\0', sizeof serverAddr1.sin_zero);  
  memset(serverAddr2.sin_zero, '\0', sizeof serverAddr2.sin_zero);  
  memset(serverAddr3.sin_zero, '\0', sizeof serverAddr3.sin_zero);  
  memset(serverAddr4.sin_zero, '\0', sizeof serverAddr4.sin_zero);  

  /*---- Connect the socket to all servers using the address struct ----*/
  addr_size = sizeof(serverAddr1);
  connect(clientSocket_1, (struct sockaddr *) &serverAddr1, addr_size);
  connect(clientSocket_2, (struct sockaddr *) &serverAddr2, addr_size);
  connect(clientSocket_3, (struct sockaddr *) &serverAddr3, addr_size);
  connect(clientSocket_4, (struct sockaddr *) &serverAddr4, addr_size);


  /*---- Read the message from the server into the buffer ----*/
  // recv(clientSocket_1, buffer1, 1024, 0);
  // recv(clientSocket_2, buffer2, 1024, 0);
  // recv(clientSocket_3, buffer3, 1024, 0);
  // recv(clientSocket_4, buffer4, 1024, 0);
  // send(clientSocket_4,username,strlen(username)+1,0);
  // send(clientSocket_2,username,strlen(username)+1,0);
  // send(clientSocket_3,username,strlen(username)+1,0);
  send(clientSocket_1,username,strlen(username)+1,0);



  // send(clientSocket_1,username,strlen(username),0);


  /*---- Print the received message ----*/
  // printf("Data received: %s",buffer1);   
  // printf("Data received: %s",buffer2);   
  // printf("Data received: %s",buffer3);   
  // printf("Data received: %s",buffer4);   


  return 0;
}