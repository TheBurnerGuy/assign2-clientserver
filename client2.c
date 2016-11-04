#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include "shared.h"


#define MAXMESSAGESIZE 400
int main(int argc, char* argv[]) 
{
	if (argc != 4)
	{
		perror("Invalid paramaters. Should be ./chat379 HOSTNAME PORT# USERNAME\n");
		exit(1);
	}

	int s, numbytes;
	int PORT = atoi(argv[2]);
	char message[MAXMESSAGESIZE];	
	char messagebuffer[MAXMESSAGESIZE];
	struct sockaddr_in serveraddress; /* server address info */
	unsigned long host = inet_addr (argv[1]);
	node* nodeHead = (node*)malloc(sizeof(node));
	nodeHead->next = NULL;

	

	//Set up signal handler function - used to terminate if handshake/messaging fails
	void signal_handler(int sig){
		close(s);
		fprintf(stderr, "Terminating...\n");
		deleteNodes(nodeHead);
		exit(0);
	}
	//Set up signal handlers
	struct sigaction segv;
	segv.sa_handler = signal_handler;
	sigemptyset(&segv.sa_mask);
	segv.sa_flags = 0;
	sigaction(SIGINT,&segv,0);


	if((s = socket( AF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("Client: cannot open socket");
		exit (1);
	}

	memset( &serveraddress, 0, sizeof( serveraddress));   
	serveraddress.sin_family = AF_INET;    
	serveraddress.sin_addr.s_addr = host;            
	serveraddress.sin_port = htons( PORT);   

	if( connect( s, (struct sockaddr *)&serveraddress,sizeof(serveraddress)) == -1) {
		perror( "Client connect");
		exit( 1);
	}
	while(1) {
		printf( "Message (or /users): ");
		scanf( "%s", message);
		if( (numbytes = send( s, message, strlen( message), 0)) <=0) {
			perror( "Client send");
			raise(SIGINT);
		}
		if( (numbytes = recv( s, messagebuffer, MAXMESSAGESIZE - 1, 0)) == -1) {
			perror( "Client recv");
			raise(SIGINT);
		}
		messagebuffer[ numbytes] = '\0';   // end of string        

		printf( "Received: %s\n", messagebuffer);
	} 
	close( s);
	return 0;
}	