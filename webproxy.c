#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <error.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>  //for gethostbyname()
#include <fcntl.h>
#include <arpa/inet.h>  //inet_addr
#include <openssl/md5.h>

#define NUM_CONNECTIONS		(1000)
#define BUFSIZE				(1024)		//1kB
#define MAX_BUFF_SIZE		(10000)
#define MAX_FILESIZE        (1000000)

int sockfd;
int multi_clients[NUM_CONNECTIONS] = {1};
int process_index = 0;
char port_str[10];

static inline int file_len(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return len;
}

int blocked_hosts(char *path)
{
    char *blocked_list = "list_of_blocked_host.txt";
    char *URL_with_slash = NULL;
    char *find_str = NULL;
    char *URL = NULL;
    char blocked_website[MAX_BUFF_SIZE];
    char buff[10000];
    int ret;

    FILE *fp = fopen(blocked_list, "r");
    if (fp == NULL)
    {
        perror("fopen - blocked hosts()");
        return 0;
    }
    URL_with_slash = strstr(path, "//");
    URL_with_slash += 2;
    for (int i = 0; i < strlen(URL_with_slash); i++)
    {
        if(URL_with_slash[i] == '/')
            break;
        blocked_website[i] = URL_with_slash[i];
    }
    printf("blocked website is:%s\n", blocked_website);

    int file_size = file_len(fp);
    fread(buff, 1, file_size, fp);
    find_str = strstr(buff, blocked_website);
    if(find_str != NULL)
    {
        ret = 1;
    }
    fclose(fp);
    return ret;

}

//Find out the creation time of the file
void file_timestamp(char *pwd, char *time_of_file)
{
    struct stat st;
    char date[25];
    stat(pwd, &st);
    strcpy(time_of_file, ctime(&st.st_mtime));
    // sprintf(time_of_file, "%s". )
}

char *md5_compute(char *str, int len)
{
    int i;
    MD5_CTX c;
    unsigned char digest[16];
    char *output = (char*)malloc(50);
    MD5_Init(&c);
    while (len > 0) 
    {
        if (len > 512) 
        {
            MD5_Update(&c, str, 512);
        } 
        else 
        {
            MD5_Update(&c, str, len);
        }
        len -= 512;
        str += 512;
    }
    MD5_Final(digest, &c);
    for (i = 0; i < 16; ++i) 
    {
        snprintf(&(output[i*2]), 16*2, "%02x", (unsigned int)digest[i]);
    }
    return output;
}

void prefetch_website(char *pwd, int sock)
{
    char send_buffer[MAX_FILESIZE];
    char link[MAX_BUFF_SIZE];
    char buffer[MAX_BUFF_SIZE];
    char temp_buffer[MAX_BUFF_SIZE];
    char filepath[MAX_BUFF_SIZE];
    char get_prefechted_data_msg[MAX_BUFF_SIZE];
    char path[MAX_BUFF_SIZE];
    char *search_ptr;
    char *search_ptr_val;
    char *md5_str;
    char *new_ptr;
    int i;
    int file_size;
    int readbytes, sendbytes;
    FILE *fp_open;
    FILE *fp_write;

    fp_open = fopen(pwd, "r");
    if(fp_open == NULL)
    {
        perror("fopen()");
    }

    file_size = file_len(fp_open);
    // printf("Size of file is:%d\n", file_size);

    readbytes = fread(send_buffer, 1, file_size, fp_open);
    if(readbytes != file_size)
    {
        perror("fread");
        exit(1);
    }
    //start of actual prefetch operation
    if((search_ptr = strstr(send_buffer, "href=\"http://")) != NULL)
    {
        i = 0;
        while((search_ptr = strstr(send_buffer, "href=\"http://")) != NULL)
        {
            i = 0;
            search_ptr += 13;
            //get URL following the href command
            while(*search_ptr != '"')
            {
                link[i] = *search_ptr;
                // printf("%c\n", *search_ptr);
                search_ptr++;
                i++;
            }
            new_ptr = search_ptr;
            link[i] = '\0'; //String Termination
            md5_str = md5_compute(buffer, (int)MAX_BUFF_SIZE);
            // printf("md5sum of the link prefetch:%s\n", md5_str);

            strcpy(filepath, "./cachedir/");
            strcat(filepath, md5_str);
            strcat(filepath, ".html");
            printf("filepath:%s\n",filepath);
            if((search_ptr_val = strstr(link, "/")) != NULL)
            {
                strcpy(path,search_ptr_val);            
            }
            else if(search_ptr_val == NULL)
            {
                continue;
            }

            *search_ptr_val = '\0';
            search_ptr = new_ptr + 1;

            printf("\nGET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",path,link);
            sprintf(get_prefechted_data_msg,"GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",path,link);

            // for(int j=0; j<100; j++);

            // sending request to server
            sendbytes = send(sock,get_prefechted_data_msg,strlen(get_prefechted_data_msg),0);
            // printf("\n sending request");

            fp_write = fopen(filepath,"w");
            do{
                // printf("\n getting written into file"); 
                // receiving the file from server
                readbytes = recv(sock,temp_buffer,500,0);
                // writing the received contents into a file
                // printf("\nbuffer is: %s\n",buffer);
                fwrite(temp_buffer, 1, readbytes,fp_write);
            }while(readbytes > 0);
           
        }
    }
    fclose(fp_open);
}

void webserver_init(void)
{
    struct addrinfo webServer_hints, *res, *pt;
    memset (&webServer_hints, 0, sizeof(webServer_hints));      // Clearing the struct
    webServer_hints.ai_family = AF_INET;                        // IPv4
    webServer_hints.ai_socktype = SOCK_STREAM;                  // TCP stream sockets
    webServer_hints.ai_flags = AI_PASSIVE;
    int s = 1;
  
    if(getaddrinfo( NULL, port_str, &webServer_hints, &res) != 0)
    {
        perror ("getaddrinfo");
        exit(1);
    }

    // Bind the socket address
    for(pt = res; pt!=NULL; pt=pt->ai_next) 
    {
        sockfd = socket (pt->ai_family, pt->ai_socktype, 0);
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &s, sizeof(int)) < 0)  
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, pt->ai_addr, pt->ai_addrlen) == 0) 
            break;
    }
    freeaddrinfo(res);  
    
    if(pt==NULL) 
    {
        perror ("socket() or bind() error");
        exit(1);
    }

    if (listen(sockfd, NUM_CONNECTIONS) != 0 ) 
    {
        perror("listen");
        exit(1);
    }
}

int cache_find(char *req_timeout, char *pwd, int sock)
{
    char time_of_file[100]; 
    char current_time[100];
    char send_buffer[MAX_BUFF_SIZE];
    int read_bytes;
    char del_file_buffer[MAX_BUFF_SIZE];

    file_timestamp(pwd, time_of_file);
    printf("Time of file creation is:%s\n", time_of_file);
    
    int hr_int, sec_int, min_int;
    char *hr, *min, *sec;
    int timeout_sec = atoi(req_timeout);
    FILE *cache_fp;
    time_t curr_time;

    //Checking if file is available
    if(access(pwd, F_OK) < 0)
    {
        return 0;
    }
    else
    {
        //Get the individual memebers of the file time
        hr = strtok(time_of_file,":") ;
        min = strtok(NULL,":") ;
        sec = strtok(NULL,":") ;
        sec = strtok(sec, " ");
        hr = strtok(hr," ") ;
        hr = strtok(NULL," ");
        hr = strtok(NULL," ");
        hr = strtok(NULL," ");
        int fileTime = atoi(hr)*3600 + atoi(min)*60 + atoi(sec); 
        time(&curr_time);   //getting current time
        // memset()
        sprintf(current_time, "%s", ctime(&curr_time));
        printf("Current time:%s\n",current_time);
        hr = strtok(current_time,":") ;
        min = strtok(NULL,":") ;
        sec = strtok(NULL,":") ;
        sec = strtok(sec, " ");
        hr = strtok(hr," ") ;
        hr = strtok(NULL," ");
        hr = strtok(NULL," ");
        hr = strtok(NULL," ");
        int presentTime = atoi(hr)*3600 + atoi(min)*60 + atoi(sec); 

        printf("Present time %d\n", presentTime);
        printf("File time %d \n", fileTime);
        if (presentTime - fileTime > timeout_sec) 
        {
            printf("timeout period expired\n");
            printf("cached pwd is:%s\n", pwd);
            memset(del_file_buffer, 0, MAX_BUFF_SIZE);
            sprintf(del_file_buffer, "rm -f %s", pwd);
            return 0;
        }
        else 
        {
            printf("Send cached file to client\n");
            memset(send_buffer, 0, MAX_BUFF_SIZE);
            cache_fp = fopen(pwd,"r");
            do
            {
                read_bytes = fread(send_buffer, 1, MAX_BUFF_SIZE, cache_fp);
                send(sock,send_buffer,read_bytes,0);
            }
            while(read_bytes > 0);
            fclose(cache_fp);
            close(sock);
            return 1;
        }

    }   
}

//Route data from remote server to client through proxy server 
void proxy_response(int sock, char* timeout, char *pwd)
{
    int i, len, host_sock;
    int find_cache;
    int k;
    int sent_bytes, recv_bytes;

    struct hostent *he;
    struct sockaddr_in rem_addr;

    char buff_from_client[MAX_BUFF_SIZE];
    char buff_from_server[MAX_BUFF_SIZE];
    char method[MAX_BUFF_SIZE];
    char path[MAX_BUFF_SIZE];
    char http_version[MAX_BUFF_SIZE];
    char md5sum[100];
    char buff[MAX_BUFF_SIZE];
    char remote_host_rqt[MAX_BUFF_SIZE];

    char website[MAX_BUFF_SIZE];
    char *URL_with_slash = NULL;
    char *URL_with_slash1;
    char *URL = NULL;
    char file_name[MAX_BUFF_SIZE];
    FILE* fp;


    char unsupported_method[MAX_BUFF_SIZE] = "<html><body><H1>Error 400 Bad Request: Invalid Method </H1></body></html>";
    char unsupported_http_version[MAX_BUFF_SIZE] =  "<html><body><H1>Error 400 Bad Request: Invalid HTTP Version</H1></body></html>";
    char blocked_request[MAX_BUFF_SIZE] =  "<html><body><H1>Error 403 ERROR Forbidden: Invalid request</H1></body></html>";

    if(read(sock, buff_from_client, MAX_BUFF_SIZE) < 0)
    {
        perror("recv error");

    }
    //Parse the client response packet to get method, URL & HTTP version
    else    
    {
        sscanf(buff_from_client, "%s %s %s", method, path, http_version);   //separate out different parts of the received message
        printf("buff_from_client: %s %s %s\n", method, path, http_version);


        //Check if requested remote host is blocked
        if (blocked_hosts(path))
        {
            write(sock,blocked_request,strlen(blocked_request));
            printf("ERROR 403 Forbidden");
            exit(1);
        }
        
        //checking supported of HTTP version
        else if ((strncmp(http_version,"HTTP/1.0",strlen("HTTP/1.0")) == 0)  && (strncmp(http_version,"HTTP/1.1",strlen("HTTP/1.1")) == 0)) 
        {
            write(sock,unsupported_http_version,strlen(unsupported_http_version));
            printf("Unsupported HTTP version");
            exit(1);
        }
        
        else if(strstr(method, "GET") != NULL)
        {
            printf("Found GET method\n");
            URL_with_slash = strstr(path, "//");
            URL_with_slash = URL_with_slash + 2;
            for (int i = 0; i < strlen(URL_with_slash); i++)
            {
                if(URL_with_slash[i] == '/')
                    break;

                //Copying byte-by-byte after removing the extra '/'
                website[i] = URL_with_slash[i];  //Extracting the website
            }

            URL = strstr(URL_with_slash, "/");
            printf("\nWebsite is:%s\n", website);
            printf(" \nURL is:%s\n", URL);

            MD5_CTX mdContext;
            MD5_Init(&mdContext);
            MD5_Update (&mdContext, path, strlen(path));
            MD5_Final (md5sum, &mdContext);

            for (i = 0; i< MD5_DIGEST_LENGTH; i++) 
            {
                sprintf(&buff[2*i],"%02x", md5sum[i]);
            }

            sprintf(file_name,"%s.html",buff);
            printf("md5sum %s\n", buff);
            sprintf(pwd,"%s%s", pwd, file_name);
            printf("pwd with md5hash name:%s\n", pwd);

            //Check if the file exists in the cache before fetching them
            find_cache = cache_find(timeout, pwd, sock);
            if (find_cache == 1)
                printf("cached file found and sent to client\n");

            //Cache of requested file not found
            if(find_cache == 0)
            {
                printf("NO Cache found\n");
                printf("Getting file from remote server\n");
                fp = fopen(pwd, "ab");
                if(fp == NULL)
                {
                    perror("fopen");
                }
                he = gethostbyname(website);
                if(!he)
                {
                    perror("host address error");
                    exit(1);
                }
                //Setting remote host connection
                len = sizeof(struct sockaddr_in);
                memset(&rem_addr, 0, len);
                rem_addr.sin_family = AF_INET;
                rem_addr.sin_port = htons(80);
                memcpy(&rem_addr.sin_addr, he->h_addr, he->h_length);
                //Creating remote host socket
                host_sock = socket(AF_INET, SOCK_STREAM, 0);
                if(host_sock < 0)
                {
                    perror("Socket creation error");
                    exit(1);
                }
                k = 1;
                //re-use created socket on use
                setsockopt(host_sock, SOL_SOCKET, SO_REUSEADDR, &k, 4);
                int conn = connect(host_sock, (struct sockaddr *)&rem_addr, len);
                if(conn < 0)
                {
                    perror("connect");
                    close(host_sock);
                }

                if (URL == 0)
                    sprintf(remote_host_rqt,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n", http_version, website);
                else
                    sprintf(remote_host_rqt,"GET %s %s\r\nHost: %s\r\nConnection: close\r\n\r\n", URL, http_version, website);
        
                printf("remote_host_rqt:%s\n", remote_host_rqt);

                //Send request message packet to remote host
                sent_bytes = send(host_sock, remote_host_rqt, sizeof(remote_host_rqt), 0);
                
                if (sent_bytes < 0)
                {
                    perror("send failure");
                }
                
                else
                {
                    printf("\n\rSending file to client from remote host\n");
                    do
                    {
                        memset(buff_from_server, 0, sizeof(buff_from_server));
                        recv_bytes = recv(host_sock, buff_from_server, sizeof(buff_from_server), 0);
                        if(recv_bytes > 0)
                        {
                            send(sock, buff_from_server, recv_bytes, 0);
                            fwrite(buff_from_server, 1, recv_bytes, fp);
                        }
                    }while(recv_bytes > 0);

                    printf("\n\nStarting the Prefeting operation... \n\n");
                    prefetch_website(pwd, host_sock);
                    printf("Prefetch_the_link Done\n" );
                }
                fclose(fp);
            }
            else
            {
                //Do nothing
            }
        }
        

        else
        {
            write(sock, unsupported_method, strlen(unsupported_method));
            printf("Unsupported method\n");
            exit(1);
        }
    }
    memset(unsupported_method, 0, sizeof(unsupported_method));
    close(sock);
    close(host_sock);
    printf("Closing all sockets ....\n");

}

int main(int argc, char const *argv[])
{
	char c;
	int connection_number;
    int connection_count = 0;;
	char cwd[MAX_BUFF_SIZE];
    int timeout;
    char timeout_str[10];

	struct sockaddr_in server, client, proxy;
    // socklen_t addrlen;
    int addr_size;
    char current_folder[MAX_BUFF_SIZE];
    char cache[MAX_BUFF_SIZE];
	int i, proxy_port;

	if(argc < 3)
    {
    	printf("Enter in the following format: ./webproxy <port> <timeout>\n");
    	exit(1);
    }
    strcpy(port_str, argv[1]);
    proxy_port = atoi(argv[1]);
    printf("Proxy Port:%d\n",proxy_port);
    strcpy(timeout_str, argv[2]);
	timeout = atoi(argv[2]);
    printf("timeout in seconds:%d\n",timeout);

    //Create cache folder in current working directory
    if (getcwd(current_folder, sizeof(current_folder)) != NULL) 
    {
        printf("Current working dir: %s\n", current_folder);
        sprintf(cwd,"%s/cache/",current_folder);
        sprintf(cache,"mkdir %s",cwd);
        system(cache);
    }

    webserver_init();   //create the webserver
    
    //Accept incoming connection
    printf("Waiting for incoming connection\n");
    addr_size = sizeof(struct sockaddr_in);

    while(1)
    {
        multi_clients[process_index] = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&addr_size);\
        if(multi_clients[process_index] > 0)
        {
            if(fork() == 0)
            {
                proxy_response(multi_clients[process_index], timeout_str, cwd);
                close(sockfd);
                exit(0);
            }
        }
    }

	return 0;
}