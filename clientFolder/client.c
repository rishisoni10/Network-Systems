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
#include <errno.h>

#define MAXBUFSIZE 100

/* You will have to modify the program below */

int main (int argc, char * argv[])
{
	int errsv;
	int sbytes = -1;                             // number of bytes send by sendto()
	int rbytes = -1;                             // number of bytes send by recvfrom()
	int sockfd;                               //this will be our socket
	char buffer[MAXBUFSIZE];

	struct sockaddr_in remote;              //"Internet socket address structure"

	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/
	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address


	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create client socket");
	}

	/******************
	  sendto() sends immediately.  
	  it will report an error if the message fails to leave the computer
	  however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
	 ******************/
	char command[] = "apple is the shit";	
	sbytes = sendto(sockfd, command, (sizeof(command) - 1), 0, (struct sockaddr *)&remote, sizeof(struct sockaddr));

	if(sbytes < 0)
		printf("unable to send over UDP\n");

	// Blocks till bytes are received
	struct sockaddr_in from_addr;
	socklen_t addr_length = sizeof(struct sockaddr);
	bzero(buffer,sizeof(buffer));

	rbytes = recvfrom(sockfd, buffer, (int)MAXBUFSIZE, 0, (struct sockaddr *)&from_addr, &addr_length);  

	if(rbytes < 0)
	{
		errsv = errno;
		printf("error in receiving data over UDP\n");
		printf("The error number is %d\n", errsv);
	}
	else if (rbytes == 0)
		printf("no data received on the socket\n");

	printf("client: number of bytes received: %d\n", rbytes);

	printf("Server says %s\n", buffer);
	
	close(sockfd);

}

