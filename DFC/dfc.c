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
* @date November 18 2017
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
int no_sub;
int errsv = 0;
char filename[FILENAME_LEN];
int sending, receiving;
int string_a_len, string_b_len;

char command[100];
char command_2[100];
FILE *fp_part[4];        /* Array of file pointers */
char *file_parts[4];     /* Array of string pointers for split file names*/
char *PORTS[4];          /* Array of string pointers for port numbers*/
int s_port[4];
int file_part_len[4];
int clientSocket[4];     /* Array of socket descriptors for 4 connections*/
struct sockaddr_in serverAddr[4];

char buffer[1024], buffer_1[1024], buffer_2[1024], buffer_3[1024], buffer_4[1024], get_buffer[1024];
socklen_t addr_size;
char* username = NULL;
char* password = NULL;
char* reply = NULL;
char* subfolder = NULL;
char *P[4];              /* Array of pointers to split file contents*/
int file_num_1, file_num_2, file_num_3, file_num_4;
int server_1_file_len, server_2_file_len, server_3_file_len, server_4_file_len;

char* server_1_buf = NULL;
char* server_2_buf = NULL;
char* server_3_buf = NULL;
char* server_4_buf = NULL;

char* temp1 = NULL;
char* temp2 = NULL;
char* temp3 = NULL;
char* temp4 = NULL;
char* cpy_temp = NULL;
char* final_buffer = NULL;

char* list_filename = NULL;
char* list_filename_temp = NULL;
char* old_list_filename = NULL;
char* tk1 = NULL;
char* a = NULL;
char* part1 = NULL;
char* part2 = NULL;


char* saveptr = NULL;
char* saveptr_1 = NULL;

char *recv_file_names[4];   /* Array of string pointers for get file names*/
char recv1_file_names[1024], recv2_file_names[1024], recv3_file_names[1024], recv4_file_names[1024];

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
    // printf("Mask is %d\n",mask);
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
    int len, sub_len;
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
    // printf("Getting mod value of MD5SUM....\n");
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


    if(subfolder == NULL)
    {
        printf("subfolder is NULLLLLLLLL\n");
        no_sub = 2;

        //Sending size of NACK packet
        send(clientSocket[0],&no_sub,4,0);
        send(clientSocket[1],&no_sub,4,0);
        send(clientSocket[2],&no_sub,4,0);
        send(clientSocket[3],&no_sub,4,0);
        
        //Sending NACK of subfolder
        send(clientSocket[0],"0",no_sub,0);
        send(clientSocket[1],"0",no_sub,0);
        send(clientSocket[2],"0",no_sub,0);
        send(clientSocket[3],"0",no_sub,0);

    }
    else
    {
        printf("subfolder is %s\n", subfolder);
        no_sub = 2;
        //Sending size of ACK packet
        send(clientSocket[0],&no_sub,4,0);
        send(clientSocket[1],&no_sub,4,0);
        send(clientSocket[2],&no_sub,4,0);
        send(clientSocket[3],&no_sub,4,0);
        
        //Sending ACK of subfolder
        send(clientSocket[0],"1",no_sub,0);
        send(clientSocket[1],"1",no_sub,0);
        send(clientSocket[2],"1",no_sub,0);
        send(clientSocket[3],"1",no_sub,0);

        //Sending subfolder length
        sub_len = strlen(subfolder) + 1;
        send(clientSocket[0], &sub_len, 4, 0);
        send(clientSocket[1], &sub_len, 4, 0);
        send(clientSocket[2], &sub_len, 4, 0);
        send(clientSocket[3], &sub_len, 4, 0);

        //Sending subfolder
        send(clientSocket[0], subfolder, sub_len, 0);
        send(clientSocket[1], subfolder, sub_len, 0);
        send(clientSocket[2], subfolder, sub_len, 0);
        send(clientSocket[3], subfolder, sub_len, 0);
    }

    // printf("Freeing everything\n");
    for(int i = 0; i < 4; i++)
    {
       // printf("data freeing\n");
       free(P[i]);
       // printf("data name freeing\n");
       free(file_parts[i]);
    }
}


void list(char* subfolder)
{
    int i, sub_len;
    int flag[4] = {0};
    // printf("Waiting for file part name lengths...\n");
    printf("In list function\n");
    
    memset(server_1_buf, 0, 1024);
    memset(server_2_buf, 0, 1024);
    memset(server_3_buf, 0, 1024);
    memset(server_4_buf, 0, 1024);

    memset(buffer_1, 0, 1024);
    memset(buffer_2, 0, 1024);
    memset(buffer_3, 0, 1024);
    memset(buffer_4, 0, 1024);

    //Listing subfolder
    if(subfolder == NULL)
    {
        printf("subfolder is NULLLLLLLLL\n");
        no_sub = 2;

        //Sending size of NACK packet
        send(clientSocket[0],&no_sub,4,0);
        send(clientSocket[1],&no_sub,4,0);
        send(clientSocket[2],&no_sub,4,0);
        send(clientSocket[3],&no_sub,4,0);
        
        //Sending NACK of subfolder
        send(clientSocket[0],"0",no_sub,0);
        send(clientSocket[1],"0",no_sub,0);
        send(clientSocket[2],"0",no_sub,0);
        send(clientSocket[3],"0",no_sub,0);
    }
    else
    {
        printf("subfolder is %s\n", subfolder);
        no_sub = 2;
        //Sending size of ACK packet
        send(clientSocket[0],&no_sub,4,0);
        send(clientSocket[1],&no_sub,4,0);
        send(clientSocket[2],&no_sub,4,0);
        send(clientSocket[3],&no_sub,4,0);
        
        //Sending ACK of subfolder
        send(clientSocket[0],"1",no_sub,0);
        send(clientSocket[1],"1",no_sub,0);
        send(clientSocket[2],"1",no_sub,0);
        send(clientSocket[3],"1",no_sub,0);

        //Sending subfolder length
        sub_len = strlen(subfolder) + 1;
        send(clientSocket[0], &sub_len, 4, 0);
        send(clientSocket[1], &sub_len, 4, 0);
        send(clientSocket[2], &sub_len, 4, 0);
        send(clientSocket[3], &sub_len, 4, 0);

        //Sending subfolder name string
        send(clientSocket[0], subfolder, sub_len, 0);
        send(clientSocket[1], subfolder, sub_len, 0);
        send(clientSocket[2], subfolder, sub_len, 0);
        send(clientSocket[3], subfolder, sub_len, 0);
    }

    //Sync bytes
    send(clientSocket[0],"0",no_sub,0);
    send(clientSocket[1],"0",no_sub,0);
    send(clientSocket[2],"0",no_sub,0);
    send(clientSocket[3],"0",no_sub,0);

    printf("Waiting before recv\n");
    
    //Receiving the number of files in each server
    recv(clientSocket[0], &file_num_1, sizeof(int), 0);
    recv(clientSocket[1], &file_num_2, sizeof(int), 0);
    recv(clientSocket[2], &file_num_3, sizeof(int), 0);
    recv(clientSocket[3], &file_num_4, sizeof(int), 0);

    //Receiving the file name lengths and the file name from each server
    while(1)
    {
        if(file_num_1 > 0)
        {
            // printf("Server 1:%d\n", file_num_1);
            memset(buffer_1, 0, 1024);
            recv(clientSocket[0], &server_1_file_len, sizeof(int), 0);
            recv(clientSocket[0], buffer_1, server_1_file_len, 0);
            strcat(server_1_buf, buffer_1);
            // printf("Server 1 file:%s", server_1_buf);
            file_num_1--;
        }
        if(file_num_2 > 0)
        {
            // printf("Server 2:%d\n", file_num_2);
            memset(buffer_2, 0, 1024);
            recv(clientSocket[1], &server_2_file_len, sizeof(int), 0);
            recv(clientSocket[1], buffer_2, server_2_file_len, 0);
            // printf("Server 2 file:%s", server_2_buf);
            strcat(server_2_buf, buffer_2);
            file_num_2--;
        }

        if(file_num_3 > 0)
        {
            // printf("Server 3:%d\n", file_num_3);
            memset(buffer_3, 0, 1024);
            recv(clientSocket[2], &server_3_file_len, sizeof(int), 0);
            recv(clientSocket[2], buffer_3, server_3_file_len, 0);
            // printf("Server 3 file:%s", server_3_buf);
            strcat(server_3_buf, buffer_3);
            file_num_3--;
        }

        if(file_num_4 > 0)
        {
            // printf("Server 4:%d\n", file_num_4);
            memset(buffer_4, 0, 1024);
            recv(clientSocket[3], &server_4_file_len, sizeof(int), 0);
            recv(clientSocket[3], buffer_4, server_4_file_len, 0);
            // printf("Server 4 file:%s", server_4_buf);
            strcat(server_4_buf, buffer_4);
            file_num_4--;
        }
        else 
        {
            printf("Break away from tyranny\n");
            break;
        }
    }
    printf("file part names received!!!\n");
    // printf("Server 1 file:%s", server_1_buf);
    // printf("Server 2 file:%s", server_2_buf);
    // printf("Server 3 file:%s", server_3_buf);
    // printf("Server 4 file:%s", server_4_buf);
    printf("\n");

    
    tk1 = malloc(strlen(server_1_buf) + 1);
    strcpy(temp1, server_1_buf);
    strcpy(temp2, server_2_buf);
    strcpy(temp3, server_3_buf);
    strcpy(temp4, server_4_buf);
    
    
    //Replacing all the '\n' chars with '!' for strstr()
    i = 0;
    while(temp1[i] != '\0')
    {
        if(temp1[i] == '\n')
        {
            // printf("Remove1\n");
            temp1[i] = '!';
        }
        i++;
    }

    i = 0;
    while(temp2[i] != '\0')
    {
        if(temp2[i] == '\n')
        {
            // printf("Remove\n");
            temp2[i] = '!';
        }
        i++;
    }

    i = 0;
    while(temp3[i] != '\0')
    {
        if(temp3[i] == '\n')
        {
            // printf("Remove\n");
            temp3[i] = '!';
        }
        i++;
    }

    i = 0;
    while(temp4[i] != '\0')
    {
        if(temp4[i] == '\n')
        {
            // printf("Remove\n");
            temp4[i] = '!';
        }
        i++;
    }

    tk1 = strtok_r(server_1_buf, "\n", &saveptr);
    memset(old_list_filename, 0, 1024);

    while(tk1 != NULL)
    {   
        // memset(filename, 0, 1024);
        // printf("\nCurrent token is:%s", tk1);
        part1 = strtok_r(tk1, ".", &saveptr_1);
        part2 = strtok_r(NULL, ".", &saveptr_1);

        sprintf(list_filename, "%s.%s", part1, part2);
        
        // printf("\nparts:%s",list_filename);
        // printf("\nValue of strcmp is:%d\n", strcmp(old_list_filename, list_filename));

        if(strcmp(old_list_filename, list_filename) == 0)
        {   
            printf("Prsed file same as old\n");
            // memset(list_filename, 0, 1024);
            // memset(old_list_filename, 0, 1024);
            tk1 = strtok_r(NULL, "\n", &saveptr);
            continue;
        }
        else
        {
            printf("New file\n");
            memset(old_list_filename, 0, 200);
        }

        for(i = 0; i < 4; i++)
        {
            // printf("\n\n");
            memset(cpy_temp, 0, 1024);
            // strcpy(temp3, ".");
            memset(a, 0, 2);

            memset(list_filename_temp, 0, 200);
            strcpy(list_filename_temp, list_filename);
            sprintf(a,".%d", i+1);
            strcat(list_filename_temp, a);
            strcat(cpy_temp, list_filename_temp);
            // printf("\nTemp file is:%s", cpy_temp);
            
            if(strstr(temp1, cpy_temp) || strstr(temp2, cpy_temp) || strstr(temp3, cpy_temp) || strstr(temp4, cpy_temp))
            {
                // printf("File found\n");
                flag[i] = 1;
            }
            else
            {
                // printf("File not found\n");
                flag[i] = 0;
            }
        }

        if(flag[0] == 0 || flag[1] == 0 || flag[2] == 0 || flag[3] == 0)
        {
            printf("File '%s' is Incomplete\n", list_filename);
            strcat(final_buffer, list_filename);
            strcat(final_buffer, "\t\tIncomplete");
            // sprintf(final_buffer, "\n%s\t%s", filename, "\tIncomplete");
        }
        else
        {
            strcat(final_buffer, list_filename);
            strcat(final_buffer, "\n");
        }

        strcpy(old_list_filename, list_filename);
        printf("\nOld file is:%s\n", old_list_filename);
        tk1 = strtok_r(NULL, "\n", &saveptr);
    }
    printf("Final buffer:\n%s", final_buffer);

    free(tk1);
}

void get_file(char* data_file_name, char* subfolder)
{
    int sub_len, i;
    int part1_name_len, part2_name_len;
    char temp_buf[1024];
    char a[2];
    char get_ACK[5];
    int get_ACK_len;
    FILE *fp;

    //Get subfolder: sending info to server
    if(subfolder == NULL)
    {
        printf("subfolder is NULLLLLLLLL\n");
        no_sub = 2;

        //Sending size of NACK packet
        send(clientSocket[0],&no_sub,4,0);
        send(clientSocket[1],&no_sub,4,0);
        send(clientSocket[2],&no_sub,4,0);
        send(clientSocket[3],&no_sub,4,0);
        
        //Sending NACK of subfolder
        send(clientSocket[0],"0",no_sub,0);
        send(clientSocket[1],"0",no_sub,0);
        send(clientSocket[2],"0",no_sub,0);
        send(clientSocket[3],"0",no_sub,0);

    }
    else
    {
        printf("subfolder is %s\n", subfolder);
        no_sub = 2;
        //Sending size of ACK packet
        send(clientSocket[0],&no_sub,4,0);
        send(clientSocket[1],&no_sub,4,0);
        send(clientSocket[2],&no_sub,4,0);
        send(clientSocket[3],&no_sub,4,0);
        
        //Sending ACK of subfolder
        send(clientSocket[0],"1",no_sub,0);
        send(clientSocket[1],"1",no_sub,0);
        send(clientSocket[2],"1",no_sub,0);
        send(clientSocket[3],"1",no_sub,0);

        //Sending subfolder length
        sub_len = strlen(subfolder) + 1;
        send(clientSocket[0], &sub_len, 4, 0);
        send(clientSocket[1], &sub_len, 4, 0);
        send(clientSocket[2], &sub_len, 4, 0);
        send(clientSocket[3], &sub_len, 4, 0);

        //Sending subfolder name string
        send(clientSocket[0], subfolder, sub_len, 0);
        send(clientSocket[1], subfolder, sub_len, 0);
        send(clientSocket[2], subfolder, sub_len, 0);
        send(clientSocket[3], subfolder, sub_len, 0);
    }
    memset(recv1_file_names, 0, 1024);
    memset(recv2_file_names, 0, 1024);
    memset(recv3_file_names, 0, 1024);
    memset(recv4_file_names, 0, 1024);

    memset(buffer_1, 0, 1024);
    memset(buffer_2, 0, 1024);
    memset(buffer_3, 0, 1024);
    memset(buffer_4, 0, 1024);
    printf("Waiting to receive file part name lengths...\n");
    
    char* old_get_filename1 = malloc(100);
    char* old_get_filename2 = malloc(100);

    memset(a, 0, 2);
    memset(temp_buf, 0, 1024);
    // sprintf(a, ".%d", i);
    // strcpy(temp_buf, data_file_name);
    // strcat(temp_buf, a);

    //Server 1
    recv(clientSocket[0], &part1_name_len, 4, 0);
    recv(clientSocket[0], buffer_1, part1_name_len, 0);
    strcat(buffer_1, "!");
    strcat(recv1_file_names, buffer_1);
    memset(buffer_1, 0, 1024);

    recv(clientSocket[0], &part2_name_len, 4, 0);
    recv(clientSocket[0], buffer_1, part2_name_len, 0);
    strcat(buffer_1, "!");
    strcat(recv1_file_names, buffer_1);
    memset(buffer_1, 0, 1024);
    printf("Server 1:%s\n", recv1_file_names);

    //Server 2
    recv(clientSocket[1], &part1_name_len, 4, 0);
    recv(clientSocket[1], buffer_2, part1_name_len, 0);
    strcat(buffer_2, "!");
    strcat(recv2_file_names, buffer_2);
    memset(buffer_2, 0, 1024);

    recv(clientSocket[1], &part2_name_len, 4, 0);
    recv(clientSocket[1], buffer_2, part2_name_len, 0);
    strcat(buffer_2, "!");
    strcat(recv2_file_names, buffer_2);
    memset(buffer_2, 0, 1024);
    printf("Server 2:%s\n", recv2_file_names);

    //Server 3
    recv(clientSocket[2], &part1_name_len, 4, 0);
    recv(clientSocket[2], buffer_3, part1_name_len, 0);
    strcat(buffer_3, "!");
    strcat(recv3_file_names, buffer_3);
    memset(buffer_3, 0, 1024);

    recv(clientSocket[2], &part2_name_len, 4, 0);
    recv(clientSocket[2], buffer_3, part2_name_len, 0);
    strcat(buffer_3, "!");
    strcat(recv3_file_names, buffer_3);
    memset(buffer_3, 0, 1024);
    printf("Server 3:%s\n", recv3_file_names);

    //Server 4
    recv(clientSocket[3], &part1_name_len, 4, 0);
    recv(clientSocket[3], buffer_4, part1_name_len, 0);
    strcat(buffer_4, "!");
    strcat(recv4_file_names, buffer_4);
    memset(buffer_4, 0, 1024);

    recv(clientSocket[3], &part2_name_len, 4, 0);
    recv(clientSocket[3], buffer_4, part2_name_len, 0);
    strcat(buffer_4, "!");
    strcat(recv4_file_names, buffer_4);
    memset(buffer_4, 0, 1024);
    printf("Server 4:%s\n", recv4_file_names);

    memset(file_part_len, 0, 16);
    
    for(i = 1; i < 5; i++)
    {
        memset(a, 0, 2);
        memset(temp_buf, 0, 1024);
        sprintf(a, ".%d", i);
        strcpy(temp_buf, data_file_name);
        strcat(temp_buf, a);
        printf("searching file:%s\n",temp_buf);
        memset(get_ACK, 0, 5);
        if(strstr(recv1_file_names, temp_buf))
        {
            printf("Part %d found in DFS1\n", i);

            strcpy(get_ACK, "1");
            get_ACK_len = strlen(get_ACK) + 1;

            send(clientSocket[0], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[0], get_ACK, get_ACK_len, 0);            
          
            send(clientSocket[0], &i, sizeof(int), 0);
            recv(clientSocket[0], &file_part_len[i-1], sizeof(int), 0);
            printf("File len received:%d\n", file_part_len[i-1]);

            P[i-1] = malloc(file_part_len[i-1]);
            recv(clientSocket[0], P[i-1], file_part_len[i-1], 0);
            
            //Sending NACK to rest of the servers
            memset(get_ACK, 0, 5);
            strcpy(get_ACK, "0");
            get_ACK_len = strlen(get_ACK) + 1;

            send(clientSocket[1], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[1],get_ACK, get_ACK_len, 0); 

            send(clientSocket[2], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[2],get_ACK, get_ACK_len, 0); 

            send(clientSocket[3], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[3],get_ACK, get_ACK_len, 0); 

            
        }
        else if(strstr(recv2_file_names, temp_buf))
        {
            printf("Part %d found in DFS2\n", i);
            strcpy(get_ACK, "1");
            get_ACK_len = strlen(get_ACK) + 1;

            send(clientSocket[1], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[1], get_ACK, get_ACK_len, 0);            

            send(clientSocket[1], &i, sizeof(int), 0);
            recv(clientSocket[1], &file_part_len[i-1], sizeof(int), 0);

            printf("File len received:%d\n", file_part_len[i-1]);

            P[i-1] = malloc(file_part_len[i-1]);
            recv(clientSocket[1], P[i-1], file_part_len[i-1], 0);
            
            //Sending NACK to rest of the servers

             //Sending NACK to rest of the servers
            memset(get_ACK, 0, 5);
            strcpy(get_ACK, "0");
            get_ACK_len = strlen(get_ACK) + 1;

            send(clientSocket[0], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[0],get_ACK, get_ACK_len, 0); 

            send(clientSocket[2], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[2],get_ACK, get_ACK_len, 0); 

            send(clientSocket[3], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[3],get_ACK, get_ACK_len, 0); 

        }

        else if(strstr(recv3_file_names, temp_buf))
        {
            printf("Part %d found in DFS3\n", i);
            strcpy(get_ACK, "1");
            get_ACK_len = strlen(get_ACK) + 1;

            send(clientSocket[2], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[2], get_ACK, get_ACK_len, 0);            

            send(clientSocket[2], &i, sizeof(int), 0);
            recv(clientSocket[2], &file_part_len[i-1], sizeof(int), 0);

            printf("File len received:%d\n", file_part_len[i-1]);

            P[i-1] = malloc(file_part_len[i-1]);
            recv(clientSocket[2], P[i-1], file_part_len[i-1], 0);
            
            //Sending NACK to rest of the servers

            memset(get_ACK, 0, 5);
            strcpy(get_ACK, "0");
            get_ACK_len = strlen(get_ACK) + 1;

            send(clientSocket[0], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[0],get_ACK, get_ACK_len, 0); 

            send(clientSocket[1], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[1],get_ACK, get_ACK_len, 0); 

            send(clientSocket[3], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[3],get_ACK, get_ACK_len, 0); 
        }

        else if(strstr(recv4_file_names, temp_buf))
        {
            printf("Part %d found in DFS4\n", i);
            
            strcpy(get_ACK, "1");
            get_ACK_len = strlen(get_ACK) + 1;

            send(clientSocket[3], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[3], get_ACK, get_ACK_len, 0);          

            send(clientSocket[3], &i, sizeof(int), 0);
            recv(clientSocket[3], &file_part_len[i-1], sizeof(int), 0);

            printf("File len received:%d\n", file_part_len[i-1]);

            P[i-1] = malloc(file_part_len[i-1]);
            recv(clientSocket[0], P[i-1], file_part_len[i-1], 0);
            
            //Sending NACK to rest of the servers

            memset(get_ACK, 0, 5);
            strcpy(get_ACK, "0");
            get_ACK_len = strlen(get_ACK) + 1;

            send(clientSocket[0], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[0],get_ACK, get_ACK_len, 0); 

            send(clientSocket[1], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[1],get_ACK, get_ACK_len, 0); 

            send(clientSocket[2], &get_ACK_len, sizeof(get_ACK_len), 0);  
            send(clientSocket[2],get_ACK, get_ACK_len, 0);
        }
        // send()
    }

    fp = fopen(data_file_name, "w");
    for(i = 0; i < 4; i++)
    {
        fwrite(P[i], 1, file_part_len[i], fp);
        free(P[i]);

    }
    fclose(fp);
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
    
    server_1_buf = malloc(1024);
    server_2_buf = malloc(1024);
    server_3_buf = malloc(1024);
    server_4_buf = malloc(1024);

    temp1 = malloc(1024);
    temp2 = malloc(1024);
    temp3 = malloc(1024);
    temp4 = malloc(1024);
    cpy_temp = malloc(1024);

    list_filename = malloc(200);
    list_filename_temp = malloc(200);
    old_list_filename = malloc(200);
    a = malloc(2);
    
    saveptr = malloc(100);
    saveptr_1 = malloc(100);

    part1 = malloc(100);
    part2 = malloc(500);

    final_buffer = malloc(1024);


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
        printf("get <file_name> </subfolder>\n");
        printf("put <file_name> </subfolder>\n");
        printf("list </subfolder>\n");
        printf("mkdir </subfolder>\n");
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

        if(strstr(command_2, "Sending list"))
        {
            printf("List command found\n");
            strcpy(subfolder, (command_2 + 14));
            if(strcmp(subfolder, "\0") == 0)
            {
                printf("subfolder is string ending\n");
                subfolder = NULL;
            }
            printf("Subfolder in main:%s\n", subfolder);
            user_credentials();
            list(subfolder);
        }

        if(strstr(command_2, "Sending files"))
        {
            strcpy(data_file_name, (command_2 + 14));
            printf("Before strtok is:%s\n", data_file_name);

            //extracting subfolder name, if any 
            subfolder = strtok(data_file_name, " /");
            subfolder = strtok(NULL, " /");
            printf("subfolder is:%s\n", subfolder);

            //extracting data file from string
            data_file_name = strtok(data_file_name, " ");
            printf("Data file name is:%s\n", data_file_name);
            user_credentials();
            get_file(data_file_name, subfolder);
        }

        if(strstr(command_2, "Send subfolder"))
        {
            printf("MKDIR command found\n");
            strcpy(subfolder, (command_2 + 14));

            if(strcmp(subfolder, "\0") == 0)
            {
                printf("subfolder is string ending\n");
                subfolder = NULL;
            }
            printf("Subfolder in main:%s\n", subfolder);
            user_credentials();
            // mkdir_folder(subfolder);
        }

    }
    
    
    return 0;
}