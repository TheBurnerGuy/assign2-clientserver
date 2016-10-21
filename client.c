#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/time.h>

#include "shared.h"

//Sends username in the form of length string
void send_name(int s, char* user, int user_length){
	char temp = user_length;
	send(s,&temp,1,0)
	int i;
	for(i = 0; i<user_length; ++i){
		send(s,user[i],1,0);
	}
}

int main(int argc, char* argv[])
{
	int	s;
	unsigned short number; //Used as a temporary variable, then the current users count
	int PORT = atoi(argv[2]);
	struct	sockaddr_in	server;
	unsigned long host = inet_addr (argv[1]); //0.0.0.0

	//Start connecting
	s = socket (AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror ("Client: cannot open socket");
		exit (1);
	}
	bzero (&server, sizeof (server));
	server.sin_addr.s_addr = host;
	server.sin_family = AF_INET;
	server.sin_port = htons (PORT);
	if (connect (s, (struct sockaddr*) & server, sizeof (server))) {
		perror ("Client: cannot connect to server");
		exit (1);
	}
	//Finished Connecting
	
	//Set up signal handler function
	void signal_handler(int sig){
		fprintf(stderr, "Terminating...\n");
		//Send termination message to server
		char tempNum = 0x02;
		send(s,&tempNum,1,0);
		//tempNum = strlen(argv[3]);
		//send(s,&tempNum,1,0);
		send_name(s, argv[3], tempNum);
		close(s);
		exit(0);
	}
	
	//Handshake begin
	recv (s, &number, sizeof (number), 0);
	if (number != 0xCFA7){
		perror ("Client: failed to connect to server");
		exit (1);
	}
	//fprintf (stderr, "Process %d gets number %d\n", getpid (),
	//ntohl (number));
	
	recv (s, &number, sizeof (number), 0);
	number = ntohl(number);
	node* userStart = (node*)malloc(sizeof(node));
	userStart->next = NULL;
	//Check if there are other users in server
	if(number != 0){
		int i, stringLength;
		char* string;
		for(i = 0; i<number; ++i){
			recv(s,&stringLength,1,0);
			string = (char*)malloc(stringLength+1);
			recv(s,string,stringLength,0);
			addName(userStart,string);
		}
	}
	//Finally, send own username
	send_name(s, argv[3], strlen(argv[3]));
	//Handshake end
	printNames(nodeHead);
	
	//Set up signal handlers
	struct sigaction segv;
	segv.sa_handler = signal_handler;
	sigemptyset(&segv.sa_mask);
	segv.sa_flags = 0;
	sigaction(SIGINT,&segv,0);
	
	int alarm = 0;
	//Alarm signal handler function
	void alarm_handler(int sig){ alarm = 1;}
	alarm(9);
	
	//Set up alarm signal handler
	struct sigaction sega;
	sega.sa_handler = alarm_handler;
	sigemptyset(&sega.sa_mask);
	sega.sa_flags = 0;
	sigaction(SIGALRM,&sega,0);
	
	//Set up select
	struct timeval tv;
    fd_set readfds;
    fd_set writefds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(s, &readfds);
    FD_SET(s, &writefds);
    
    tv.tv_sec = 2;
    tv.tv_usec = 500000;
	
	while(1){
		select(s+1,&readfds, &writefds, NULL, &tv);
		if(FD_ISSET(s, &readfds){
			
		}
		if(FD_ISSET(s, &writefds)){
			
			alarm = 0;
			alarm(9);
		}
		if(alarm){
			//send idle message
		}
	}
	close (s);
}
