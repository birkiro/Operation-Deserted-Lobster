/*
 * Socket_server.c
 * Developer: Birkir Oskarsson
 *
 * Usage:
 * compile using gcc:
 * $ gcc Socket_server.c -o server
 * $ ./server
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MYPORT 		3490    // the port users will be connecting to
#define MAXDATASIZE 100 	// max number of bytes we can get at once
#define BACKLOG 	10     	// how many pending connections queue will hold

void sigchld_handler(int s) { while(wait(NULL) > 0); }

int main(void)
{
    int sockfd, my_socket;  // listen on sock_fd, new connection on my_socket
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    int sin_size;
    struct sigaction sa;
    int yes=1;
    char buf[MAXDATASIZE];
    int numbytes;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { perror("socket"); exit(1); }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) { perror("listen"); exit(1); }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) { perror("sigaction"); exit(1); }

	// main accept() loop
	sin_size = sizeof(struct sockaddr_in);
	if ((my_socket = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
	{
		perror("accept");
		//continue;
	}
	printf("Server: Connection Established With %s\n", inet_ntoa(their_addr.sin_addr));

    while(1)
    {
    	// The socket read() function
    	if ((numbytes = recv(my_socket, buf, MAXDATASIZE-1, 0)) == -1)
    	{
    		perror("recv");
    		exit(1);
    	}
    	buf[numbytes] = '\0';  // NULL
    	printf("Message received: %s",buf);

        if (!fork())
        { // this is the child process
            close(sockfd); // child doesn't need the listener
            if (send(my_socket, "Hi from BEAGLE\n", 32, 0) == -1) { perror("send"); }
            close(my_socket);
            exit(0);
        }
    }
    close(my_socket);
    return 0;
}
