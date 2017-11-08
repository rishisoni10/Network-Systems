
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

int nbytes;                        //number of bytes we receive in our message
char file_name[10];

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
/*
void put_file(char *file_name, int sockfd)
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
*/


int main(int argc, char const *argv[])
{
    int welcomeSocket, newSocket;
    char buffer[1024];
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    int port;
    char ACK;
    char* recv_username = malloc(100);
    char* recv_password = malloc(100);
    char* req_username = malloc(100);
    char* req_password = malloc(100);
    char* cpy_1 = malloc(100);
    char* cpy_2 = malloc(100);
    char* token = malloc(100);
    char* file_contents = NULL;
    FILE *fp = NULL;

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
            printf("Wrong credentials. Try Again\n");   // username/password not found in dfs.conf
        }
        else if(!strcmp(recv_username, req_username) && !strcmp(recv_password, req_password))
        {
            printf("Client credentials verified\n");
            ACK = '1';
            send(newSocket,&ACK,1,0);
            break; 
        }
        else
        {
            printf("Wrong credentials. Try Again....\n");
            ACK = '0';
            send(newSocket,&ACK,1,0);
        }
        // memset(buffer, 0, sizeof(buffer));
    }

    memset(buffer, 0, sizeof(buffer));
    recv(newSocket,buffer,1024,0);
    if(strstr(buffer, "put") != NULL)
    {
        printf("Found command\n");
        strcpy(file_name, (buffer + 3));
        memset(buffer,0, sizeof(buffer));
        strcpy(buffer, "Send file");
        strcat(buffer, file_name);
        printf("Sending the following string: %s\n", buffer);
        send(newSocket,buffer,1024,0);
        // put_file(file_name, sockfd, remote);
    }



    return 0;
}