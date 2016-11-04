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
		fprintf(userlog,"Client: failed to send message/disconnected by server\n");
		raise(SIGINT);
	}
}

void receive_message(int s, void* address, int byte_length){
	if(recv(s, address, byte_length, 0) <= 0){
		fprintf(userlog,"Client: failed to receive message/disconnected by server\n");
		raise(SIGINT);
	}
}

int main(int argc, char* argv[])
{
	int	s;
	unsigned short number; //Used as a temporary variable, then the current users count
	int PORT = atoi(argv[2]);
	struct	sockaddr_in	server;
	//~ unsigned long host = inet_addr (argv[1]); //0.0.0.0
	struct	hostent	*host;
	host = gethostbyname (argv[1]);
	if (host == NULL) {
		perror ("Client: cannot get host description");
		exit (1);
	}
	
	//Initializing variables not related to socket initialization
	int i, stringLength;
	node* nodeHead = (node*)malloc(sizeof(node));
	nodeHead->next = NULL;
	unsigned char temp;
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
	bcopy (host->h_addr, & (server.sin_addr), host->h_length);
	//~ server.sin_addr.s_addr = host;
	server.sin_family = AF_INET;
	server.sin_port = htons (PORT);
	if (connect (s, (struct sockaddr*) & server, sizeof (server))==-1) {
		perror ("Client: cannot connect to server");
		exit (1);
	}
	//Finished Connecting
	
	//Handshake begin
	receive_message(s, &number, 2);
	if (ntohs(number) != 0xCFA7){
		perror ("Client: failed to connect to server");
		exit (1);
	}
	
	receive_message(s, &number, sizeof (number));
	number = ntohs(number);
	printf("Number of other users: %d\n",number);
	//Check if there are other users in server
	if(number != 0){
		for(i = 0; i<number; ++i){
			receive_message(s,&temp,1);
			receive_message(s,nameBuffer,temp);
			addName(nodeHead,nameBuffer,temp);
		}
	}
	//Finally, send own username
	number = strlen(argv[3]);
	addName(nodeHead,argv[3],number);
	temp = number;
	send_message(s,&temp,1);
	send_message(s,argv[3],number);
	//Handshake end
	printNames(nodeHead);
	
	//Initialize some functions/signal handlers before entering select()
	
	//User log - used to report user-update messages (otherwise pointless)
	
	userlog = fopen("client379.log","w");
	
	int alarmBool = 0; //Variable was to be used for alarm, but used instead to tick off the idle messages
	
	//Set up select
	struct timeval tv;
    fd_set readfds;
    fd_set masterfds;
    tv.tv_usec = 500000;
    int timeout = 25; //seconds until send idle message
    tv.tv_sec = timeout;
	
	FD_ZERO(&readfds);
	FD_SET(s, &masterfds);
	FD_SET(STDIN, &masterfds);
	
	while(1){
		readfds = masterfds;
		if(select(s+1,&readfds, NULL, NULL, &tv) <= 0){
			//perror("Client: Error while waiting for input/receiving output");
			tv.tv_sec = timeout;
			alarmBool = 1;
		}
		
		//Was a message received?
		if(FD_ISSET(s, &readfds)){
			receive_message(s,&temp,1);
			if(temp==0){
				//User message received -- might have error since unsigned short is supposed to be 2 bytes long?
				receive_message(s, &temp, 1);
				receive_message(s, nameBuffer, temp);
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
					receive_message(s, messageBuffer,number);
					printf("%s: %s\n",nameBuffer, messageBuffer);
					memset(messageBuffer, 0, number+1); //clear messageBuffer for future messages
				}
				//clear nameBuffer 
				memset(nameBuffer, 0, temp+1);
				
			}else if(temp==1){
				//New user signal received
				receive_message(s, &number, 1);
				receive_message(s, nameBuffer, number);
				printf("%s has entered.\n", nameBuffer);
				addName(nodeHead,nameBuffer,number);
			}else if(temp==2){
				//User left signal received
				receive_message(s, &number, 1);
				receive_message(s, nameBuffer, number);
				printf("%s has left.\n", nameBuffer);
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
			while(1){//waiting for temp == 10 or 14
				read(STDIN,&temp,1);
				if(temp == 10 || temp == 14){
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
					printf("Command not found.\n");
				}
			}else if(messageBuffer[0]=='/'){
				printf("Command not found.\n");
			//Normal Case:
			}else{
				charCount = htons(charCount);
				send_message(s,&charCount,2);
				charCount = ntohs(charCount);
				if(charCount != 0) send_message(s, messageBuffer, charCount);
				tv.tv_sec = timeout; //Remember to reset idle timer
			}
			//clear messageBuffer for future messages
			memset(messageBuffer, 0, charCount); 
		}//End FD_ISSET STDIN
		//If no message has been sent in the past 9 seconds, send an idle message
		if(alarmBool){
			//perror("Sending idle message\n"); //Testing
			number = 0; //htons(0)
			send_message(s,&number,2);
			alarmBool = 0;
		}
	}//END while loop
}//END main()
