#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

#include "shared.h"


int main(int argc, char* argv[]) 
{
	int listeningsocketfd, connectionsocketfd;                     
	struct sockaddr_in master; 
	struct sockaddr_in clientaddress; 
	int sin_size = sizeof(master);              /* size of address structure */
	int yes=1;        // for setsockopt() SO_REUSEADDR, below
	
	if (argc != 2)
    {
      perror("Invalid paramaters. Should be ./server379 PORT#\n");
      exit(1);
	}

	if( (listeningsocketfd = socket( AF_INET, SOCK_STREAM, 0)) < 0) {
	 	perror ("Client: cannot open socket");
		exit (1);
	}

	//allow reuse of sockets if a process terminates
	if( setsockopt( listeningsocketfd, SOL_SOCKET, SO_REUSEADDR,&yes, sizeof(yes)) < 0) {
		perror( "Error setsockopt");
		exit( 1);
	}

	//Set up example username listsend_message
	node* headNode = (node*)malloc(sizeof(node));
	headNode->next = NULL;
	node* currentNode;
	char* nameBuffer = (char*)malloc(256);
    if(nameBuffer == NULL){
		perror ("Server: failed to allocate enough memory");
		exit (1);
	}
	char* messageBuffer = (char*)malloc(sizeof(char)*65536);
    if(messageBuffer == NULL){
		perror ("Client: failed to allocate enough memory");
		exit (1);
	}

	memset( &master, 0, sizeof master);            
	master.sin_family = AF_INET;          
	master.sin_port = htons (atoi(argv[1]));; //convert port into network byte order
	master.sin_addr.s_addr = INADDR_ANY; 

	if( bind( listeningsocketfd, (struct sockaddr *) &master, sizeof(master)) < 0) {
		perror ("Server: cannot bind master socket");
		exit( 1);
	}
	if (listen(listeningsocketfd, 10) == -1) {
        perror("Server: listen failed");
        exit(-1);
    }

    //Set up signal handlers
	void signal_handler(int sig){
		perror("Terminating...");
		//Close all sockets connected to the server
		close(listeningsocketfd);
		currentNode = headNode;
		while(currentNode->next!= NULL){
			close(currentNode->fd);
			currentNode = currentNode->next;
		}
		exit(0);
	}
	struct sigaction segv;
	segv.sa_handler = signal_handler;
	sigemptyset(&segv.sa_mask);
	segv.sa_flags = 0;
	sigaction(SIGINT,&segv,0);
	
	// main loop for original server.
    while(1)
    {
		sin_size = sizeof( struct sockaddr_in);
	
		if( (connectionsocketfd = accept( listeningsocketfd, (struct sockaddr *)&clientaddress, &sin_size)) < 0) {
			perror ("Server: accept failed");
			continue;
		}

		if( !fork()) 
		{        
	


		}
		close(connectionsocketfd); 

	} //while

return 0;
}
