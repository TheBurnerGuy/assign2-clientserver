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
#include <signal.h>

#include "shared.h"

#define STDIN 0

//Sends username in the form of length string
void send_name(int s, char* user, int user_length){
	char temp = user_length;
	send(s,&temp,1,0);
	send(s,user,user_length,0);
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
	
	//Handshake begin
	recv (s, &number, sizeof (number), 0);
	if (number != 0xCFA7){
		perror ("Client: failed to connect to server");
		exit (1);
	}
	//fprintf (stderr, "Process %d gets number %d\n", getpid (),
	//ntohl (number));
	
	recv (s, &number, sizeof (number), 0);
	number = ntohs(number);
	node* nodeHead = (node*)malloc(sizeof(node));
	nodeHead->next = NULL;
	//Check if there are other users in server
	if(number != 0){
		int i, stringLength;
		char* string;
		for(i = 0; i<number; ++i){
			recv(s,&stringLength,1,0);
			string = (char*)malloc(stringLength+1);
			recv(s,string,stringLength,0);
			addName(nodeHead,string);
		}
	}
	//Finally, send own username
	send_name(s, argv[3], strlen(argv[3]));
	//Handshake end
	printNames(nodeHead);
	
	//Initialize some functions/signal handlers before entering select()
	
	//Set up signal handler function
	void signal_handler(int sig){
		close(s);
		fprintf(stderr, "Terminating...\n");
		//Send termination message to server
		//Just realized we might not need this
		//char tempNum = 0x02;
		//send(s,&tempNum,1,0);
		//tempNum = strlen(argv[3]);
		//send(s,&tempNum,1,0);
		//send_name(s, argv[3], tempNum);
		deleteNodes(nodeHead);
		exit(0);
	}
	
	//User log - used to report user-update messages (otherwise pointless)
	FILE* userlog;
	userlog = fopen("userlog.txt","w");
	
	//Set up signal handlers
	struct sigaction segv;
	segv.sa_handler = signal_handler;
	sigemptyset(&segv.sa_mask);
	segv.sa_flags = 0;
	sigaction(SIGINT,&segv,0);
	
	int alarmBool = 0;
	//Alarm signal handler function
	void alarm_handler(int sig){ alarmBool = 1;}
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
    tv.tv_usec = 500000;
    
    //Variables used in the while loop, most of them temporarily/reused
    
    char* messageBuffer = (char*)malloc(sizeof(char)*65535);
    if(messageBuffer == NULL){
		perror ("Client: failed to allocate enough memory");
		exit (1);
	}
	char* nameBuffer = (char*)malloc(sizeof(char)*255);
    if(nameBuffer == NULL){
		perror ("Client: failed to allocate enough memory");
		exit (1);
	}
    unsigned short charCount = 0;
    char temp;
    int i;
    node* deleteNode;
	
	while(1){
		tv.tv_sec = 2;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_SET(s, &readfds);
		FD_SET(STDIN, &readfds);
		FD_SET(s, &writefds);
		
		select(s+1,&readfds, &writefds, NULL, &tv); //Should test if alarm goes through block
		//Was a message received?
		if(FD_ISSET(s, &readfds)){
			recv(s,&temp,1,0);
			if(temp==0){
				//User message received
				recv(s, &number, 1, 0);
				recv(s, nameBuffer, number, 0);
				printf("%s: ",nameBuffer);
				recv(s, &number, 2, 0);
				number = ntohs(number);
				//Special case: Message of length 0, write to a userlog
				if(number == 0){
					number = strlen(nameBuffer);
					for(i = 0; i<number; ++i){
						fputc(nameBuffer[i],userlog);
					}
					number = 0;
				}
				//Normal case: Normal message
				for(i = 0; i<number; ++i){
					recv(s, &temp, 1, 0);
					printf("%c",temp);
				}
				if (number!= 0) printf("\n");
			}else if(temp==1){
				//New user signal received
				recv(s, &number, 1, 0);
				recv(s, nameBuffer, number, 0);
				addName(nodeHead,nameBuffer);
			}else if(temp==2){
				//User left signal received
				recv(s, &number, 1, 0);
				recv(s, nameBuffer, number, 0);
				deleteNode = removeName(nodeHead,nameBuffer);
				if (nodeHead->name == deleteNode->name){
					nodeHead = nodeHead->next;
				}
				free(deleteNode);
			}else{
				perror("Received unknown message.\n");
				exit(1);
			}
		}
		//If there's a message entered into STDIN, read it into messageBuffer
		if(FD_ISSET(STDIN, &readfds)){
			while(temp != 10){
				recv(STDIN,&temp,1,0);
				if(temp == 10){
					break;
				}
				messageBuffer[charCount] = temp;
				++charCount;
			}
			
		}
		//If socket is writeable... 
		if(FD_ISSET(s, &writefds)){
			if(charCount>0){
				charCount = htons(charCount);
				send(s,&charCount,2,0);
				send(s, messageBuffer, ntohs(charCount), 0);
				charCount = 0;
				alarmBool = 0;
				alarm(9);
			}
		}
		//If no message has been sent in the past 9 seconds, send an idle message
		if(alarmBool){
			fprintf(stderr, "Sending idle message\n"); //Testing
			number = htons(0);
			send(s,&number,2,0);
			alarmBool = 0;
			alarm(9);
		}
	}
	close (s);
}
