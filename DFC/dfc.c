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
#include <openssl/md5.h>

#define FILENAME_LEN        (50)



// void put_file(char*, char*);

int errsv = 0;
char filename[FILENAME_LEN];
int sending, receiving;
int string_a_len, string_b_len;

char command[100];
char command_2[100];
FILE *fp_part[4];        /* Array of file pointers */
char *file_parts[4];     /* Array of string pointers for split file names*/
char *PORTS[4];          /* Array of string pointers for port numbers*/
// int s1_port, s2_port, s3_port, s4_port;
int s_port[4];
int file_part_len[4];
int clientSocket[4];     /* Array of socket descriptors for 4 connections*/
struct sockaddr_in serverAddr[4];

char buffer[1024], buffer1[1024], buffer2[1024], buffer3[1024], buffer4[1024];
socklen_t addr_size;
char* username = NULL;
char* password = NULL;
char* reply = NULL;
char* subfolder = NULL;
char *P[4];              /* Array of pointers to split file contents*/
// size_t p_size[4];
// char* P2 = NULL;
// char* P3 = NULL;
// char* P4 = NULL;
// char *a;

void signal_handler(int signum)
{
    if(signum == SIGINT)
    {
        for (int i = 0; i < 4; ++i)
        {
            close(clientSocket[i]);
            if(P[i] != NULL)
                free(P[i]);
        }
    }
    exit(0);
}

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
    // char filename[] = "dfc.conf";
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

unsigned int MD5HASH(FILE *fp)
{
    int i, bytes;
    MD5_CTX mdContext;
    unsigned char mask;
    unsigned char c[MD5_DIGEST_LENGTH];
    unsigned char data[1024];
    unsigned char* md5_mask = malloc(sizeof(char) * 1);

    MD5_Init(&mdContext);
    while((bytes = fread(data, 1, 1024, fp)) != 0)
        MD5_Update(&mdContext, data, bytes) ;
    MD5_Final(c, &mdContext);

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        printf("%x",c[i]);;
    }

    printf("\n");

    //Extracting first byte of md5sum string
    md5_mask = strcpy(md5_mask, c+(MD5_DIGEST_LENGTH-1));
    for (i = 0; i < 1; i++)
    {
        printf("%d ", md5_mask[i]);
    }
    mask = 0;
    mask = md5_mask[0];
    printf("Mask is %d\n",mask);
    return mask;
}

void split_files(char *data_file_name)
{
    printf("In split files function\n");
    int i;
    char *a = NULL;
    char *tmp = NULL;
    FILE* fp = NULL;
    fp = fopen(data_file_name, "r");
    int file_string_len = strlen(data_file_name);
    if (fp == NULL)
    {
       errsv = errno;
       printf("ERRNO is %d\n", errsv);
       perror("could not open file");
       exit(1);    
    }

    int len = file_size(fp);
    a = (char*)malloc(2);
    tmp = (char*)calloc(file_string_len+4, 1);
    for(i = 0; i < 4; i++)
    {
        strcpy(tmp, ".");
        file_parts[i] = (char*)malloc(file_string_len+5);
        strcat(tmp, data_file_name);
        strcpy(file_parts[i], tmp);
        // printf("copied:%s\n", file_parts[i]);
        strcat(file_parts[i], ".");
        sprintf(a, "%d", i+1);
        strcat(file_parts[i], a);
        printf("file part name is: %s\n", file_parts[i]);
        memset(tmp, 0, strlen(tmp)+1);
    }

    /* Splitting file in 4 parts, equal/unequal, depending on mod(4) operation result */
    printf("\nLength of file is %d bytes\n", len);
    if (len % 4 == 0)
    {
        for(i = 0; i < 4; i++)
        {
            file_part_len[i] = len/4;
            P[i] = malloc(file_part_len[i]);
        }
    }

    else if(len % 4 == 1)
    {
        i = 0;
        while(i < 3)
        {
            file_part_len[i] = len/4;
            P[i] = malloc(file_part_len[i]);
            i++;
        }

        file_part_len[i] = (len/4) + 1;
        P[i] = malloc(file_part_len[i]);
    }

    else if(len % 4 == 2)
    {
        i = 0;
        while(i < 2)
        {
            file_part_len[i] = len/4;
            P[i] = malloc(file_part_len[i]);
            i++;
        }
        while(i < 4)
        {
            file_part_len[i] = (len/4) + 1;
            P[i] = malloc(file_part_len[i]);
            i++;
        }
    }

    else if(len % 4 == 3)
    {
        i = 0;
        while(i < 3)
        {
            file_part_len[i] = (len/4) + 1;
            P[i] = malloc(file_part_len[i]);
            i++;   
        }
        file_part_len[i] = (len/4);
        P[i] = malloc(file_part_len[i]);
    }
    
    /*Reading "file_part_len[i]" number of bytes and storing them in different buffers, P[i]*/
    for(i = 0; i < 4; i++)
    {
        fread(P[i], 1, file_part_len[i], fp);
    }
    fclose(fp);
}


void send_part(int socket, int part_a, int part_b)
{
    //Part 1 and 2 
    if(part_a == 1 && part_b == 2)
    {
        printf("sending part 1 and 2\n");

        //Sending file size before sending file part
        send(socket, &file_part_len[0], sizeof(int), 0);
        send(socket, &file_part_len[1], sizeof(int), 0);

        string_a_len = strlen(file_parts[0])+1;
        string_b_len = strlen(file_parts[1])+1;
        //Sending file part name lengths before sending file part
        send(socket, &string_a_len, sizeof(int), 0);
        send(socket, &string_b_len, sizeof(int), 0);

        //Sending file part name first 
        send(socket, file_parts[0], string_a_len, 0);
        send(socket, file_parts[1], string_b_len, 0);

        //Sending actual file parts
        send(socket, P[0], file_part_len[0], 0);
        send(socket, P[1], file_part_len[1], 0);

        string_a_len = 0;
        string_b_len = 0;
    }

    //Part 2 and 3 
    if(part_a == 2 && part_b == 3)
    {
        printf("sending part 2 and 3\n");

        //Sending file size before sending file part
        send(socket, &file_part_len[1], sizeof(int), 0);
        send(socket, &file_part_len[2], sizeof(int), 0);

        string_a_len = strlen(file_parts[1])+1;
        string_b_len = strlen(file_parts[2])+1;

        //Sending file part name lengths before sending file part
        send(socket, &string_a_len, sizeof(int), 0);
        send(socket, &string_b_len, sizeof(int), 0);

        //Sending file part name first 
        send(socket, file_parts[1], string_a_len, 0);
        send(socket, file_parts[2], string_b_len, 0);

        //Sending actual file parts
        send(socket, P[1], file_part_len[1], 0);
        send(socket, P[2], file_part_len[2], 0);

        string_a_len = 0;
        string_b_len = 0;
    }

    //Part 3 and 4 
    if (part_a == 3 && part_b == 4)
    {
        printf("sending part 3 and 4\n");

        //Sending file size before sending file part
        send(socket, &file_part_len[2], sizeof(int), 0);
        send(socket, &file_part_len[3], sizeof(int), 0);

        string_a_len = strlen(file_parts[2])+1;
        string_b_len = strlen(file_parts[3])+1;

        //Sending file part name lengths before sending file part
        send(socket, &string_a_len, sizeof(int), 0);
        send(socket, &string_b_len, sizeof(int), 0);

        //Sending file part name first 
        send(socket, file_parts[2], string_a_len, 0);
        send(socket, file_parts[3], string_b_len, 0);

        //Sending actual file parts
        send(socket, P[2], file_part_len[2], 0);
        send(socket, P[3], file_part_len[3], 0);

        string_a_len = 0;
        string_b_len = 0;
    }

    //Part 4 and 1 
    if(part_a == 4 && part_b ==1)
    {

        printf("sending part 4 and 1\n");
        
        //Sending file size before sending file part
        send(socket, &file_part_len[3], sizeof(int), 0);
        send(socket, &file_part_len[0], sizeof(int), 0);

        string_a_len = strlen(file_parts[3])+1;
        string_b_len = strlen(file_parts[0])+1;

        //Sending file part name lengths before sending file part
        send(socket, &string_a_len, sizeof(int), 0);
        send(socket, &string_b_len, sizeof(int), 0);

        //Sending file part name first 
        send(socket, file_parts[3], string_a_len, 0);
        send(socket, file_parts[0], string_b_len, 0);

        //Sending actual file parts
        send(socket, P[3], file_part_len[3], 0);
        send(socket, P[0], file_part_len[0], 0);

        string_a_len = 0;
        string_b_len = 0;
    }
}



/*
* @brief Sends file to server
*
* Given a file name, this function sends the file to the server 
*
* @param *filename     File name pointer
*
* @return void 
*/
void put_file(char *data_file_name, char* subfolder)
{
    split_files(data_file_name);    //Splits file in 4 parts for 4 servers
    unsigned int x;
    int i;
    int len;
    FILE *fp = NULL;
   
    printf("File name is:%s\n", data_file_name);
    memset(command, 0, sizeof(command));

    //*Opening file to calculate its MD5SUM
    fp = fopen(data_file_name, "r");
    if(fp == NULL)
    {
       errsv = errno;
       printf("ERRNO is %d\n", errsv);
       perror("could not open file");
       exit(1);
    }   
    // printf("Return value is %d\n", MD5HASH(fp));
    printf("Getting mod value of MD5SUM....\n");
    x = MD5HASH(fp) % 4;
    printf("x is %d\n",x);
    fclose(fp);
    
    if(x == 0)
    {
        printf("0 case\n");
        printf("DFS1:(1,2), DFS2:(2,3), DFS3:(3,4), DFS4:(4,1)\n");
        
        send_part(clientSocket[0], 1, 2);
        send_part(clientSocket[1], 2, 3);
        send_part(clientSocket[2], 3, 4);
        send_part(clientSocket[3], 4, 1);

    }

    else if (x == 1)
    {
        printf("1 case\n");
        printf("DFS1:(4,1), DFS2:(1,2), DFS3:(2,3), DFS4:(3,4)\n");
        send_part(clientSocket[0], 4, 1);
        send_part(clientSocket[1], 1, 2);
        send_part(clientSocket[2], 2, 3);
        send_part(clientSocket[3], 3, 4);
        
    }
    else if (x == 2)
    {
        printf("2 case\n");
        printf("DFS1:(3,4), DFS2:(4,1), DFS3:(1,2), DFS4:(2,3)\n");
        send_part(clientSocket[0], 3, 4);
        send_part(clientSocket[1], 4, 1);
        send_part(clientSocket[2], 1, 2);
        send_part(clientSocket[3], 2, 3);
    }
    else if(x == 3)
    {
        printf("3 case\n");
        printf("DFS1:(2,3), DFS2:(3,4), DFS3:(4,1), DFS4:(1,2)");
        send_part(clientSocket[0], 2, 3);
        send_part(clientSocket[1], 3, 4);
        send_part(clientSocket[2], 4, 1);
        send_part(clientSocket[3], 1, 2);
    }

    printf("Freeing everything\n");
    for(int i = 0; i < 4; i++)
    {
       printf("data freeing\n");
       free(P[i]);
       printf("data name freeing\n");
       free(file_parts[i]);
    }
}



void user_credentials(void)
{
    memset(username, 0, 100);
    memset(password, 0, 100);
    printf("\nSending user credentials....\n");
    strcpy(username, config_file("Username"));
    strcpy(password, config_file("Password"));
    strcat(username, ",");
    strcat(username, password);

    /*---- Send user credentials from client to all servers ----*/
    send(clientSocket[0],username,strlen(username)+1,0);
    send(clientSocket[1],username,strlen(username)+1,0);
    send(clientSocket[2],username,strlen(username)+1,0);
    send(clientSocket[3],username,strlen(username)+1,0);

    /*---- Receive confirmation of user credentials from all servers ----*/
    printf("Waiting to receive confirmation..,...\n");
    recv(clientSocket[0], &reply[0], 1, 0);
    recv(clientSocket[1], &reply[1], 1, 0);
    recv(clientSocket[2], &reply[2], 1, 0);
    recv(clientSocket[3], &reply[3], 1, 0);
    
    if(reply[0] == '0' && reply[1] == '0' && reply[2] == '0'&& reply[3] == '0')
    {
        printf("User credentials wrong\n");
        exit(1);
    }
    else if(reply[0] == '1' && reply[1] == '1'&& reply[2] == '1' && reply[3] == '1')
        printf("User credentials correct\n");
    else
        printf("No reply from server\n");

}

int main(int argc, char const *argv[])
{
    int len;
    int i;
    char *data_file_name = NULL;

    // FILE *fp = NULL;

     if(argc < 2)
    {
        printf("Enter config file name.\n");
        exit(1);
    }
    strcpy(filename, argv[1]);

    /*** Parsing from dfc.conf file***/   
    /* Allocating memory for parsing user credentials*/ 
    username = malloc(100);
    password = malloc(100);
    reply = malloc(4);
    subfolder = malloc(50);
    data_file_name = malloc(50);
    
    for (i = 0; i < 4; ++i)
    {
        PORTS[i] = (char*)malloc(10);
    }

    strcpy(PORTS[0], config_file("DFS1"));
    strcpy(PORTS[1], config_file("DFS2"));
    strcpy(PORTS[2], config_file("DFS3"));
    strcpy(PORTS[3], config_file("DFS4"));
    
    printf("Port1:%s Port2:%s Port3:%s Port4:%s\n", PORTS[0], PORTS[1], PORTS[2], PORTS[3]);
    for (i = 0; i < 4; ++i)
    {
        s_port[i] = atoi(PORTS[i]);
    }
   
    addr_size = sizeof(serverAddr[0]);

    /*
    -> Create 4 sockets for 4 servers
    -> Configure settings of the server address struct
    -> Set IP address to localhost for all servers
    -> Set all bits of the padding field to 0
    -> Connect the socket to all servers using the address struct
    */

    for (i = 0; i < 4; i++)
    {
        clientSocket[i] = socket(PF_INET, SOCK_STREAM, 0);
        serverAddr[i].sin_family = AF_INET;
        serverAddr[i].sin_port = htons(s_port[i]);
        serverAddr[i].sin_addr.s_addr = inet_addr("127.0.0.1");
        // memset(serverAddr[i].sin_zero, '\0', sizeof(serverAddr[i].sin_zero));  
        connect(clientSocket[i], (struct sockaddr *)&serverAddr[i], addr_size);
    }

    //Cleanly exit the program. Close all sockets
    signal(SIGINT, signal_handler);

    
    while(1)
    {
        memset(command, 0, sizeof(command));
        memset(command_2, 0, sizeof(command_2));
        memset(data_file_name, 0, 50);
        memset(reply, 0, 4);
        if(subfolder != NULL)
            memset(subfolder, 0, 50);

        printf("\r\n     *** Main Menu ***\n\r");
        printf("Enter the following commands for file transfers / handling\n");
        printf("get <file_name>\n");
        printf("put <file_name>\n");
        printf("list\n");
        printf("mkdir <subfolder>\n");
        printf("Type in the command followed by the <file_name>, if the command requires\n");

        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;
        printf("Sending entered command to server\n");
        sending = send(clientSocket[0],command,1024,0);
        sending = send(clientSocket[1],command,1024,0);
        sending = send(clientSocket[2],command,1024,0);
        sending = send(clientSocket[3],command,1024,0);

        if(sending < 0)
        {
            perror("Send Error");
        }
        else
            printf("No. of bytes sent:%d\n", sending);
        
        receiving = recv(clientSocket[0], command_2, 1024, 0);
        receiving = recv(clientSocket[1], command_2, 1024, 0);
        receiving = recv(clientSocket[2], command_2, 1024, 0);
        receiving = recv(clientSocket[3], command_2, 1024, 0);


        if(receiving < 0)
        {
            perror("Receive error");
        }
        else
            printf("No. of bytes received:%d\n", receiving);

        printf("Command received is %s\n", command_2);

        if(strstr(command_2, "Send file"))
        {
            strcpy(data_file_name, (command_2 + 10));
            printf("Before strtok is:%s\n", data_file_name);

            //extracting subfolder name, if any 
            subfolder = strtok(data_file_name, " /");
            subfolder = strtok(NULL, " /");
            printf("subfolder is:%s\n", subfolder);

            //extracting data file from string
            data_file_name = strtok(data_file_name, " ");
            printf("Data file name is:%s\n", data_file_name);
            user_credentials();
            put_file(data_file_name, subfolder);
        }

        if(strstr(command_2, "Sending file"))
        {
            printf("Get command found\n");
        }
    }
    
    
    return 0;
}