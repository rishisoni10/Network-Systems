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
#include <openssl/md5.h>

#define FILENAME_LEN        (50)

char command[100];

unsigned int MD5HASH(FILE *fp)
{
    int i, bytes;
    MD5_CTX mdContext;
    unsigned long mask;
    unsigned char c[MD5_DIGEST_LENGTH];
    unsigned char data[1024];
    unsigned char* md5_mask = malloc(sizeof(char) * 4);

    MD5_Init(&mdContext);
    while((bytes = fread(data, 1, 1024, fp)) != 0)
        MD5_Update(&mdContext, data, bytes) ;
    MD5_Final(c, &mdContext);

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        printf("%x",c[i]);;
    }

    // fclose(fp);
    printf("\n");

    // printf("Md5_mask:\n");
    // for(i = 0; i < 4; i++)
    // {
    //     md5_mask[i] = c[i];
    //     printf("%d ", md5_mask[i]);
    // }
    // md5_mask[4] = '\0';
    // printf("\n");
    // i = 0;
    // while(md5_mask[i] !='\0')
    // {
    //     printf("%x", md5_mask[i]);
    //     i++;
    // }

    //Extracting first 4 bytes of md5sum string
    md5_mask = strncpy(md5_mask, c+12, 4);
    for (i = 0; i < 4; i++)
    {
        printf("%d ", md5_mask[i]);
    }
    mask = 0;
    for(i = 0; i < 4; i++)
    {
        if(md5_mask[i] < 100)
            mask = (mask * 100) + md5_mask[i];
        else //if (md5_mask[i] >=100)
            mask = (mask * 1000) + md5_mask[i];
        
        printf("Result of multiplication: %ld\n", mask);
    }
    
    printf("\nMask is %ld\n", mask);
    return mask;
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


int main(int argc, char const *argv[])
{
    int clientSocket, clientSocket_1, clientSocket_2, clientSocket_3, clientSocket_4;
    char buffer[1024], buffer1[1024], buffer2[1024], buffer3[1024], buffer4[1024];
    struct sockaddr_in serverAddr1, serverAddr2, serverAddr3, serverAddr4;
    socklen_t addr_size;
    int s1_port, s2_port, s3_port, s4_port;
    char* key = NULL;
    unsigned int x;
    char reply;
    char filename[FILENAME_LEN];
    int len;
   
    FILE* fp = NULL;

     if(argc < 2)
    {
        printf("Enter file to be sent.\n");
        exit(1);
    }
    strcpy(filename, argv[1]);

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


    /*---- Send user credentials from client to all servers ----*/
    send(clientSocket_1,username,strlen(username)+1,0);

    recv(clientSocket_1, &reply, 1, 0);
    
    if(reply == '0')
    {
        printf("User credentials wrong\n");
        exit(1);
    }
    else if(reply == '1')
        printf("User credentials correct\n");


    fp = fopen(filename, "r");
    if(fp == NULL)
    {
        printf("File cannot opened / file does not exist\n");
        return 1;
    }   

    // printf("Return value is %d\n", MD5HASH(fp));
    x = MD5HASH(fp) % 4;
    printf("x is %d\n",x);
    fclose(fp);

    while(1)
    {
        memset(command, 0, sizeof(command));
        printf("\r\n     *** Main Menu ***\n\r");
        printf("Enter the following commands for file transfers / handling\n");
        printf("get <file_name>\n");
        printf("put <file_name>\n");
        printf("list\n");
        printf("Type in the command followed by the <file_name>, if the command requires\n");

        char *file_name = malloc(10);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;
        printf("Sending entered command to server\n");
        send(clientSocket_1,command,strlen(command)+1,0);
        memset(command, 0, sizeof(command));
        recv(clientSocket_1, command, 1024, 0);



        if(x == 0)
        {
            printf("0 case\n");
            fp = fopen(filename, "r");
            len = file_size(fp);
            char* P1 = (char*)malloc(len/4);
            char* P2 = (char*)malloc(len/4);
            char* P3 = (char*)malloc(len/4);
            char* P4 = (char*)malloc(len/4);
            fread(P1, 1, (len/4), fp);
            fread(P2, 1, (len/4), fp);
            fread(P3, 1, (len/4), fp);
            fread(P4, 1, (len/4), fp);
        }

        if (x == 1)
        {
            printf("1 case\n");
        }
        if (x == 2)
        {
            printf("2 case\n");
        }
        if(x == 3)
        {
            printf("3 case\n");
        }
    }
    

    /*---- Print the received message ----*/
    // printf("Data received: %s",buffer1);   
    // printf("Data received: %s",buffer2);   
    // printf("Data received: %s",buffer3);   
    // printf("Data received: %s",buffer4);   


    return 0;
}