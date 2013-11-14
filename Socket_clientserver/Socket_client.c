/*
 * Socket_client.c
 * Developer: Birkir Oskarsson
 *
 * Usage:
 * compile using gcc:
 * $ gcc Socket_client.c -o client
 * $ ./client 192.168.7.2
 *
	CLIENT                     SERVER

				+------------+
				|  socket()  |
				+------------+
				      v
				+------------+
				|   bind()   |
				+------------+
				      v
				+------------+
				|  listen()  |
				+------------+
				      v
	+-----------+           +------------+
	|  socket() |           |  accept()  |
	+-----------+           +------------+
	     v                        |
	+-----------+                 |
	| connect() |                 |
	+-----------+                 |
	     v                        v
	+-----------+           +-------------+
+-->|  write()  |+--------> |    read()   |<---+
|	+-----------+           +-------------+    |
|	     +                        +            |
|	     |                        |            |
|	     |                 process request     |
|	     |                        |            |
|	     v                        v            |
|	+------------+          +-------------+    |
+--+|   read()   |<--------+|   write()   |+---+
	+------------+          +-------------+
	     v                        v
	+------------+          +-------------+
	|  close()   |          |   close()   |
	+------------+          +-------------+
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>		// gethostbyname()
#include <sys/types.h>
#include <netinet/in.h> // htons()
#include <sys/socket.h> // socket(), connect(), recv()

#define PORT 3490 // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    char inputWord;
    struct hostent *host_entry;
    struct sockaddr_in connectors_addr; // connector's address information

    if (argc != 2)
    { // Check if user passes an argument
        fprintf(stderr,"Usage Example: ./client 192.168.7.2\n");
        exit(1);
    }

    // Extract the host info from the passed argument
    if ((host_entry = gethostbyname( argv[1]) ) == NULL) { perror("gethostbyname"); exit(1); }

    // socket()
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    connectors_addr.sin_family = AF_INET;    // host byte order
    connectors_addr.sin_port = htons(PORT);  // short, network byte order
    connectors_addr.sin_addr = *((struct in_addr *)host_entry->h_addr);
    memset(&(connectors_addr.sin_zero), '\0', 8);  // zero the rest of the struct

	// connect()
	if (connect(sockfd, (struct sockaddr *)&connectors_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("Error Connecting to Server");
		exit(1);
	}

	while(1)
	{
		// Socket write() function
		printf("Send a message to server: ");
		bzero(buf, MAXDATASIZE);					// Fill buffer with zeros
		fgets(buf, MAXDATASIZE, stdin);				// Read from stream
		numbytes = write(sockfd, buf, strlen(buf)); // send buffer content through socket
		if(numbytes < 0) { perror("Error in write()"); exit(1); }

		// The socket read() function
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) { perror("recv"); exit(1); }
		buf[numbytes] = '\0';  // NULL
		printf("Received: %s",buf);
	}
	close(sockfd);
    return 0;
}
