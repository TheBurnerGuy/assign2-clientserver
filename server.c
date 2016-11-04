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

#include "shared.h"

//send_message()
//sends message to specified socket while checking for errors
//returns 0 if socket closed or error occurred, returns 1 if send was successful
//Should be writing to log instead of STDERR
int send_message(int s, void* address, int byte_length){
	if(send(s, address, byte_length, 0) <= 0){
		perror("Server: failed to send message");
		return 0;
	}
	return 1;
}

int receive_message(int s, void* address, int byte_length){
	if(recv(s, address, byte_length, 0) <= 0){
		perror("Server: failed to receive message");
		return 0;
	}
	return 1;
}

int main(int argc, char* argv[])
{	
	
	//Daemon stuff should go here? ctrl+e reminder
	//~ pid_t pid = 0;
    //~ pid_t sid = 0;
    //~ pid = fork();
    //~ if (pid < 0){
        //~ perror("fork failed!\n");
        //~ exit(1);
    //~ }
    //~ if (pid > 0){
       //~ printf("pid of child process %d \n", pid);
       //~ exit(0); 
    //~ }
    //~ umask(0);
	//~ // open a log file
    //~ fp = fopen ("logfile.log", "w+");
    //~ if(!fp){
    	//~ printf("cannot open log file");
    //~ }
    //~ // create new process group
    //~ sid = setsid();
    //~ if(sid < 0)
    //~ {
    	//~ fprintf(fp, "cannot create new process group");
        //~ exit(1);
    //~ }
    //~ /* Change the current working directory */ 
    //~ if ((chdir("/")) < 0) {
      //~ printf("Could not change working directory to /\n");
      //~ exit(1);
    //~ }
	//~ // close standard fds
    //~ close(STDIN_FILENO);
    //~ close(STDOUT_FILENO);
    //~ close(STDERR_FILENO);

	//Initializing stuff
	int	sock, fromlength, snew, i, j, nbytes, bool;
	unsigned short tempNum, tempNum2;
	unsigned char tempChar, tempChar2;
	time_t currentTime = time(NULL);
	
	//Set up example username list
	node* nodeHead = (node*)malloc(sizeof(node));
	nodeHead->next = NULL;
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
	
	struct	sockaddr_in	master, from;
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror ("Server: cannot open master socket");
		exit (1);
	}
	memset(&master, 0, sizeof (master));
	master.sin_family = AF_INET;
	master.sin_addr.s_addr = inet_addr("127.0.0.1");//replaced INADDR_ANY
	master.sin_port = htons (atoi(argv[1]));
	if (bind (sock, (struct sockaddr*) &master, sizeof (master))) {
		perror ("Server: cannot bind master socket");
		exit (1);
	}
	if (listen(sock, 10) == -1) {
        perror("Server: listen failed");
        exit(-1);
    }
	
	//Done Initialization
	
	//Set up signal handlers
	void signal_handler(int sig){
		perror("Terminating...");
		//Close all sockets connected to the server
		close(sock);
		currentNode = nodeHead;
		while(currentNode->next!= NULL){
			close(currentNode->fd);
			currentNode = currentNode->next;
		}
		deleteNodes(nodeHead);
		exit(0);
	}
	struct sigaction segv;
	segv.sa_handler = signal_handler;
	sigemptyset(&segv.sa_mask);
	segv.sa_flags = 0;
	sigaction(SIGINT,&segv,0);
	
	//Initialize select and variables/structures it needs
	fd_set master_fds;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax = sock;        // maximum file descriptor number
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);
	FD_SET(sock, &master_fds);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	struct timeval tv;
	tv.tv_usec = 500000;
	
	printf("Start\n");
	
	// main loop for original server.
    while(1){
		tv.tv_sec = 2;
        read_fds = master_fds;
        time(&currentTime);//reset currentTime every loop
        nameBuffer = calloc(1,256);
        
        if (select(fdmax+1, &read_fds, NULL, NULL, &tv) == -1) {
            perror("select");
            exit(-1);
        }
		
        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == sock) {
                    // handle new connections
                    fromlength = sizeof (from);
					snew = accept (sock, (struct sockaddr*) & from, & fromlength);
					if (snew < 0) {
						perror ("Server: accept failed");
						exit(-1);
					}else{
                        //printf("selectserver: new connection from %s:%d on socket %d\n", inet_ntoa(remoteaddr.sin_addr), ntohs(remoteaddr.sin_port), newfd);
                        //Initial handshake
						tempChar = 0xCF;
						tempChar2 = 0xA7;
						bool = send_message (snew, &tempChar, 1); //Assumes send() will return -1 if socket is disconnected
						bool = send_message (snew, &tempChar2, 1);
						tempNum = htons(nameCount(nodeHead));
						bool = send_message (snew, &tempNum, 2);
						currentNode = nodeHead;
						while(currentNode->next!=NULL){
							tempChar = strlen(currentNode->name);
							bool = send_message(snew, &tempChar, 1);
							bool = send_message(snew, currentNode->name, tempChar);
							currentNode = currentNode->next;
						}
						//If no error received from sending in socket... (should probably check if receive is successful anyway)
						if(bool != 0){
							printf("Server: socket %d connected\n", snew);
							receive_message(snew, &tempChar, 1);
							//~ memset(nameBuffer, 0, tempChar+1);
							receive_message(snew, nameBuffer, tempChar);
							//Special Case: What if username is already logged in?
							if(findName(nodeHead, nameBuffer)==1){
								printf("Server: %s is already in server.\n",nameBuffer);
								close(snew);
							}else{
								//End handshake
								//Now add it to the server's data structures
								addNameServer(nodeHead,nameBuffer,tempChar,snew,currentTime);
								//~ //printNames(nodeHead); //just a test ;)
								FD_SET(snew, &master_fds); // add to master set
								if (snew > fdmax) {    // keep track of the max
									fdmax = snew;
								}
								//Send user connected message
								tempChar2 = 1;
								for(j = 0; j <= fdmax; j++) {
									if (FD_ISSET(j, &master_fds)) {
										// except the listener and ourselves
										if (j != sock && j != snew) {
											//Send 0x01
											if(send_message(j, &tempChar2, 1) == 0) continue;
											//Send username length
											if(send_message(j, &tempChar, 1) == 0) continue;
											//Send username
											if(send_message(j, nameBuffer, tempChar) == 0) continue;
											//Might want to test if send_message is successful or not here
										}
									}
								}
							}
						}else{
							//Error has occurred, close socket
							perror("Server: error sending usernames\n");
							close(snew);
						}
                    }
                } else {
                    // handle data from a client
                    if (receive_message(i, &tempNum, 2) <= 0) {
                        // got error or connection closed by client
                        // connection closed -- should write to a log instead of STDOUT here
                        printf("Server: socket %d hung up\n", i);
                        //close socket and remove from master file descriptor
                        close(i);
                        FD_CLR(i, &master_fds);
						//if i was fdmax, use fdmax-1
						if (fdmax == i) --fdmax;
                        //remove node from list of nodes
                        currentNode = removeFd(nodeHead,i);
                        if (nodeHead == currentNode){
							nodeHead = nodeHead->next;
						}
						nameBuffer = currentNode->name;
						tempChar = strlen(nameBuffer);
						free(currentNode);
						//Send termination message to all sockets except self
						tempChar2 = 2;
						for(j = 0; j <= fdmax; j++) {
                            if (FD_ISSET(j, &master_fds)) {//is this really needed?
                                // except the listener and ourselves
                                if (j != sock && j != i) {
									//Send 0x02
									if (send_message(j, &tempChar2, 1) == 0) continue;
									//Send username length
									if (send_message(j, &tempChar, 1) == 0) continue;
									//Send username
									if (send_message(j, nameBuffer, tempChar) == 0) continue;
                                }
                            }
                        }
                    } else {
                        // received message from a client
                        tempNum2 = ntohs(tempNum);
                        currentNode = findFd(nodeHead, i);
						nameBuffer = currentNode->name;
						currentNode->idleTime = currentTime; //update user's last sent msg time
						tempChar = strlen(nameBuffer);
                        //Special case: Idle message
                        if(tempNum == 0){
							// send to everyone!
							printf("Server: received idle message from %d\n",i);
							for(j = 0; j <= fdmax; j++) {
								if (FD_ISSET(j, &master_fds)) {//is this really needed?
									// except the listener and ourselves
									if (j != sock && j != i) {
										//Send 0x00
										if (send_message(j, &tempNum, 1) == 0) continue;
										//Send username length
										if (send_message(j, &tempChar, 1) == 0) continue;
										//might try to terminate connection here? Will test later
										//Send username
										if (send_message(j, nameBuffer, tempChar) == 0) continue;
										//might try to terminate connection here? Will test later
										//Send message length = 0
										if (send_message(j, &tempNum, 2) == 0) continue;
									}
								}
							}
						}//Normal case: Non-idle message
                        else{
							receive_message(i, messageBuffer, tempNum2);
							for(j = 0; j <= fdmax; j++) {
								// send to everyone!
								if (FD_ISSET(j, &master_fds)) {//is this really needed?
									// except the listener and ourselves
									if (j != sock && j != i) {
										//Send 0x00
										if (send_message(j, &tempNum, 1) == 0) continue;
										//Send username length
										if (send_message(j, &tempChar, 1) == 0) continue;
										//might try to terminate connection here? Will test later
										//Send username
										if (send_message(j, nameBuffer, tempChar) == 0) continue;
										//might try to terminate connection here? Will test later
										//Send message length
										if (send_message(j, &tempNum, 2) == 0) continue;
										//might try to terminate connection here? Will test later
										//Send message
										if (send_message(j, messageBuffer, tempNum2) == 0) continue;
										//might try to terminate connection here? Will test later
									}
								}
							}
						}
                    } //END received data from client
                } // END handle data from client
            } // END got new incoming connection
            //Check if any connections have been idled for more than 30
			currentNode = nodeHead;
			while(currentNode->next!=NULL){
				if(currentTime - currentNode->idleTime >= 30){
					printf("Server: socket %d failed to send idle message\n", currentNode->fd);
					//Close socket, and send termination message to all sockets
                    FD_CLR(currentNode->fd, &master_fds);
					//if i was fdmax, use fdmax-1
					if (fdmax == currentNode->fd) --fdmax;
					nameBuffer = currentNode->name;
					tempChar = strlen(nameBuffer);
					//Send termination message to all sockets except self
					tempChar2 = 2;
					for(j = 0; j <= fdmax; j++) {
                        if (FD_ISSET(j, &master_fds)) {//is this really needed?
                            // except the listener and ourselves
                            if (j != sock && j != i) {
							//Send 0x02
							if (send_message(j, &tempChar2, 1) == 0) continue;
							//Send username length
							if (send_message(j, &tempChar, 1) == 0) continue;
							//might try to terminate connection here? Will test later
							//Send username
							if (send_message(j, nameBuffer, tempChar) == 0) continue;
							//might try to terminate connection here? Will test later
                            }
                        }
                    }
                    //remove node from list of nodes
                    currentNode = removeFd(nodeHead,currentNode->fd);
                    if (nodeHead == currentNode){
						nodeHead = nodeHead->next;
					}
                    close(currentNode->fd);
                    free(currentNode);
				}
				currentNode = currentNode->next;
			}//END checking if process is idle
        } // END looping through file descriptors
    } // END while()
	
	return 0;
}
