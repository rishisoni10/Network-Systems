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
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>  //inet_addr

#define NUM_CLIENTS     (1000)
int multi_clients[NUM_CLIENTS];
int process_index;
int byte_len_1, byte_len_2;
int byte_len[4];
int ret;                        
char file_name[10];
char buffer[1024];
int welcomeSocket, newSocket;
char* recv_username = NULL;
char* recv_password = NULL;
char* req_username = NULL;
char* req_password = NULL;
char* cpy_1 = NULL;
char* cpy_2 = NULL;
char* token = NULL;
char* file_contents = NULL;
char* folder = NULL;
char* temp_buf = NULL;

char* part1_name = NULL;
char* file_part_1 = NULL;
int part1_name_len;

char* part2_name = NULL;
char* file_part_2 = NULL;
int part2_name_len;

FILE *fp = NULL;
char ACK = '0';

char* buf = NULL;
char* ptr = NULL;
size_t size;

int tmpSocket[NUM_CLIENTS];

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


int user_credentials_check(void)
{
    int flag = 0;
    recv(newSocket,buffer,1024,0);

/* ------User credentials check ----------*/
    printf("Received info is %s\n", buffer);
    recv_username = strtok(buffer,",");
    recv_password = strtok(NULL,",");
    printf("Received user name:%s\n", recv_username);
    printf("Received Password:%s\n", recv_password);

    fp = fopen("dfs.conf", "r");
    file_contents = malloc(file_size(fp));
    fread(file_contents, 1, file_size(fp), fp);
    printf("file contents:%s\n", file_contents);

    if((cpy_1 = strstr(file_contents, recv_username)) != NULL)
    {
        req_username = strtok(cpy_1, " ,\n");
        if(strcmp(req_username, recv_username) == 0)
            printf("Username checks out\n");
        else
            printf("Error in username\n");
    }
    else
        printf("Username not found\n");

    if((cpy_2 = strstr(file_contents, recv_password)) != NULL)
    {
        req_password = strtok(cpy_2, ",");
        // req_password = strtok(NULL, ",\n");
        if(strcmp(req_password, recv_password) == 0)
            printf("Password checks out\n");
        else
            printf("Error in Password:\n");
    }
    else
        printf("Password not found\n");

    if(req_password == NULL || req_username == NULL)
    {
        flag = 0;
        printf("Wrong credentials. Try Again\n");   // username/password not found in dfs.conf
        return flag;
    }
    else if(!strcmp(recv_username, req_username) && !strcmp(recv_password, req_password))
    {
        printf("Client credentials verified\n");
        ACK = '1';
        flag = 1;
        send(newSocket,&ACK,1,0);
        // break; 
        return flag;
    }
    else
    {
        printf("Invalid username/password. Try Again....\n");
        ACK = '0';
        flag = 0;
        send(newSocket,&ACK,1,0);
        return flag;
    }
}

/**
* @brief Receives file from server
*
* Given a file name, this function receives the file from the client
*
* @param *file_name     File name pointer
*        sockfd         Socket ID
*
* @return void 
*/

void put_file(char *file_name, int sockfd)
{
    int flag = user_credentials_check();
    struct stat st = {0};
    FILE* fp_part1 = NULL;
    FILE* fp_part2 = NULL;

    if(flag == 1)
    {
        // Username folder creation
        strcat(folder, "/");
        strcat(folder, req_username);
        strcat(folder, "/");

        if(stat(folder, &st) == -1)
        {
            mkdir(folder, 0700);
        }
        else
        {
            printf("Username folder already exists\n");
            printf("The folder path is:%s\n", folder);
        }

        // memset(file_part_1, 0, 1024);
        // memset(file_part_2, 0, 1024);
        memset(&byte_len_1, 0, 4);
        memset(&byte_len_2, 0, 4);
        memset(&part1_name_len, 0, 4);
        memset(&part2_name_len, 0, 4);
        memset(part1_name, 0, 150);
        memset(part2_name, 0, 150);

        printf("waiting for byte len of two parts\n");
        recv(sockfd, &byte_len_1, 4, 0);
        recv(sockfd, &byte_len_2, 4, 0);
        printf("byte len received\n");

        printf("Waiting for file part name lengths...\n");
        recv(sockfd, &part1_name_len, 4, 0);
        recv(sockfd, &part2_name_len, 4, 0);
        printf("file part name lengths received!!!\n");

        printf("Waiting for file part names...\n");
        recv(sockfd, part1_name, part1_name_len,0);
        recv(sockfd, part2_name, part2_name_len,0);
        printf("file part names received!!!\n");

        //Allocating appropriate memory for incoming file parts
        file_part_1 = malloc(byte_len_1);
        file_part_2 = malloc(byte_len_2);

        printf("Waiting to receive actual file parts....\n");
        recv(newSocket, file_part_1, byte_len_1, 0);
        recv(newSocket, file_part_2, byte_len_2, 0);
        printf("file parts received!!!\n");

        printf("File part1 name is:%s\n", part1_name);
        printf("File part1 size is:%d\n", byte_len_1);
        // printf("File part1 contents is:%s\n", file_part_1);

        printf("File part2 name is:%s\n", part2_name);
        printf("File part2 size is:%d\n", byte_len_2);
        // printf("File part2 contents is:%s\n", file_part_2);

        //Putting the file in the user folder
        memset(temp_buf, 0, 150);
        strcpy(temp_buf, folder);
        strcat(temp_buf, part1_name);

        fp_part1 = fopen(temp_buf, "w");
        if(fp_part1 == NULL)
        {
            perror("part1 file error");
        }
        fwrite(file_part_1, 1, byte_len_1, fp_part1);
        fclose(fp_part1);

        //Putting the file in the user folder
        memset(temp_buf, 0, 100);
        strcpy(temp_buf, folder);
        strcat(temp_buf, part2_name);

        fp_part2 = fopen(temp_buf, "w");
        if(fp_part2 == NULL)
        {
            perror("part2 file error");
        }
        fwrite(file_part_2, 1, byte_len_2, fp_part2);
        fclose(fp_part2);

        free(file_part_1);
        free(file_part_2);
    }
    else
        printf("User credentials not correct. Try again\n");
}



int main(int argc, char const *argv[])
{
    struct sockaddr_in serverAddr;
    struct sockaddr_in serverStorage;
    int addr_size;
    int port;
    recv_username = malloc(100);
    recv_password = malloc(100);
    req_username = malloc(100);
    req_password = malloc(100);
    cpy_1 = malloc(100);
    cpy_2 = malloc(100);
    token = malloc(100);
    folder = malloc(100);
    temp_buf = malloc(150);
    part1_name = malloc(150);
    part2_name = malloc(150);
    char* file_contents = NULL;
    FILE *fp = NULL;

    /*---- Create the socket. The three arguments are: ----*/
    welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
    
    if (setsockopt(welcomeSocket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if(argc < 3)
    {
    	printf("Enter port number / folder name of server\n");
    	exit(1);
    }

    // printf("Value of port is %s\n", argv[1]);
    port = atoi(argv[2]);
    printf("Port:%d\n",port);
    strcpy(folder, argv[1]+1);
    printf("Folder name is:%s\n", folder);


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

    /*---- Listen on the socket, with 1000 max connection requests queued ----*/
    listen(welcomeSocket,(int)NUM_CLIENTS);

        /*---- Accept call creates a new socket for the incoming connection ----*/
    addr_size = sizeof(struct sockaddr_in);
    // newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
    process_index = 0;
    while(1)
    {
        printf("Server menu:\n");
        memset(buffer, 0, sizeof(buffer));
        newSocket = accept(welcomeSocket, (struct sockaddr *)&serverStorage,(socklen_t*)&addr_size);
        printf("Value of tmpsocket: %d\n", newSocket);
        if(newSocket < 0)
        {
            perror("Accept error");
        }
        else if(fork() == 0)
        {
            close(welcomeSocket);   //child closes listening socket
            while(1)
            {
                printf("Before recv function\n");
                // close(welcomeSocket);   //child closes listening socket
                ret = recv(newSocket,buffer,1024,0);
                if(ret == 0 || ret < 0)
                {
                    perror("Recv Error");
                    exit(0);
                    break;
                }
                printf("After recv\n");
                if(strstr(buffer, "put") != NULL)
                {
                    printf("Found command\n");
                    strcpy(file_name, (buffer + 3));
                    memset(buffer,0, sizeof(buffer));
                    strcpy(buffer, "Send file");
                    strcat(buffer, file_name);
                    printf("Sending the following string: %s\n", buffer);
                    if(send(newSocket,buffer,1024,0) < 0)
                    {
                        perror("Send Error");
                        exit(0);
                        break;
                    }
                    printf("after send\n");

                    // int flag = user_credentials_check();
                    put_file(file_name, newSocket);
                    // exit(1);
                }

                if(strstr(buffer, "get") != NULL)
                {
                    printf("Found command\n");
                    strcpy(file_name, (buffer + 3));
                    memset(buffer,0, sizeof(buffer));
                    strcpy(buffer, "Sending file");
                    strcat(buffer, file_name);
                    printf("Sending the following string: %s\n", buffer);
                    if(send(newSocket,buffer,1024,0) < 0)
                    {
                        perror("Send Error");
                        exit(0);
                    }
                    printf("after send\n");
                    // put_file(file_name, newSocket);
                    // exit(1);
                }
                // close(newSocket);
                // exit(0);
            }
            printf("After child exit\n");
        }
        // printf("after child exit\n");
        // close(tmpSocket[process_index]);
    // process_index++;
    // while(tmpSocket[process_index] != -1)
        // process_index = (process_index + 1) % NUM_CLIENTS;
    }
    return 0;

}