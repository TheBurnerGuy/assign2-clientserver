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

#include "shared.h"

#define STDIN 0

//Sends username in the form of length string
void send_message(int s, void* address, int byte_length){
	if(send(s, address, byte_length, 0) <= 0){
		perror("Client: failed to send message");
		raise(SIGINT);
	}
}

void receive_message(int s, void* address, int byte_length){
	if(recv(s, address, byte_length, 0) <= 0){
		perror("Client: failed to receive message");
		raise(SIGINT);
	}
}

int main(int argc, char* argv[])
{
	int	s;
	unsigned short number; //Used as a temporary variable, then the current users count
	int PORT = atoi(argv[2]);
	struct	sockaddr_in	server;
	unsigned long host = inet_addr (argv[1]); //0.0.0.0
	
	//Initializing variables not related to socket initialization
	int i, stringLength;
	node* nodeHead = (node*)malloc(sizeof(node));
	nodeHead->next = NULL;
	char temp;
	char* messageBuffer = (char*)malloc(sizeof(char)*65536);
    if(messageBuffer == NULL){
		perror ("Client: failed to allocate enough memory");
		exit (1);
	}
	char* nameBuffer = (char*)malloc(sizeof(char)*256);
    if(nameBuffer == NULL){
		perror ("Client: failed to allocate enough memory");
		exit (1);
	}
    unsigned short charCount = 0;
    node* deleteNode;
	
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
	if (connect (s, (struct sockaddr*) & server, sizeof (server))==-1) {
		perror ("Client: cannot connect to server");
		exit (1);
	}
	//Finished Connecting
	
	//Handshake begin
	receive_message(s, &number, sizeof (number));
	if (number != htons(0xCFA7)){
		perror ("Client: failed to connect to server");
		exit (1);
	}
	//fprintf (stderr, "Process %d gets number %d\n", getpid (),
	//ntohl (number));
	
	receive_message(s, &number, sizeof (number));
	number = ntohs(number);
	//Check if there are other users in server
	if(number != 0){
		for(i = 0; i<number; ++i){
			receive_message(s,&stringLength,1);
			receive_message(s,nameBuffer,stringLength);
			addName(nodeHead,nameBuffer);
		}
	}
	//Finally, send own username
	number = strlen(argv[3]);
	temp = number;
	send_message(s,&temp,1);
	send_message(s,argv[3],number);
	//Handshake end
	printNames(nodeHead);
	
	//Initialize some functions/signal handlers before entering select()
	
	//User log - used to report user-update messages (otherwise pointless)
	FILE* userlog;
	userlog = fopen("userlog.txt","w");
	
	int alarmBool = 0;
	//Alarm signal handler function
	void alarm_handler(int sig){ alarmBool = 1;}
	alarm(9); //Initialize alarm
	
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
	
	while(1){
		tv.tv_sec = 2;
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);
		FD_SET(STDIN, &readfds);
		
		if(select(s+1,&readfds, NULL, NULL, &tv) == -1){
			perror("Client: Error while waiting for input/receiving output");
			raise(SIGINT);
		}
		//Was a message received?
		if(FD_ISSET(s, &readfds)){
			receive_message(s,&temp,1);
			if(temp==0){
				//User message received -- might have error since unsigned short is supposed to be 2 bytes long?
				receive_message(s, &temp, 1);
				receive_message(s, nameBuffer, temp);
				printf("%s: ",nameBuffer);
				receive_message(s, &number, 2);
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
				else{ //number!= 0
					receive_message(s, &messageBuffer,number);
					printf("%s",messageBuffer);
					printf("\n");
				}
			}else if(temp==1){
				//New user signal received
				receive_message(s, &number, 1);
				receive_message(s, nameBuffer, number);
				addName(nodeHead,nameBuffer);
			}else if(temp==2){
				//User left signal received
				receive_message(s, &number, 1);
				receive_message(s, nameBuffer, number);
				deleteNode = removeName(nodeHead,nameBuffer); //value returned to deleteNode is the node with the username found with removeName()
				if (nodeHead->name == deleteNode->name){
					nodeHead = nodeHead->next;
				}
				free(deleteNode);
			}else{
				perror("Received unknown message.\n");
				exit(1);
			}
			continue; //Prioritise reading over sending message. Don't want send() to block while messages are waiting to be read.
		}//End FD_ISSET readfds
		//If there's a message entered into STDIN, read it into messageBuffer
		if(FD_ISSET(STDIN, &readfds)){
			charCount = 0;
			while(1){//waiting for temp == 10
				receive_message(STDIN,&temp,1);
				if(temp == 10){
					break;
				}
				messageBuffer[charCount] = temp;
				++charCount;
			}
			//Now write it.
			//Special case: It's a command, beginning with a /
			if(messageBuffer[0]=='/' && charCount == 6){
				//Slightly complex code to find if messageBuffer is equal to "/users"
				for(i = 0; i< charCount; ++i){
					if(messageBuffer[i]!="/users"[i]) break;
				}
				if(i==6){
					printNames(nodeHead);
				}
				else{
					printf("Command not found.");
				}
			}else if(messageBuffer[0]=='/'){
				printf("Command not found.");
			//Normal Case:
			}else{
				charCount = htons(charCount);
				send_message(s,&charCount,2);
				send_message(s, messageBuffer, ntohs(charCount));
				alarmBool = 0;
				alarm(9);
			}
		}//End FD_ISSET STDIN
		//If no message has been sent in the past 9 seconds, send an idle message
		if(alarmBool){
			fprintf(stderr, "Sending idle message\n"); //Testing
			number = htons(0);
			send_message(s,&number,2);
			alarmBool = 0;
			alarm(9);
		}
	}//END while loop
}//END main()
