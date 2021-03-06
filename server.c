#include <sys/types.h>
#include <sys/select.h>
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

//node for server -- bigger than client
typedef struct node_{
	char* name;
	struct node_* next;
	int fd;
	time_t idleTime;
}node;

//Adds name, file descriptor, and idling time to list -- server only
void addNameServer(node* nodeHead, char* name, int nameSize, int fd, time_t idleTime){
	node* currentNode = nodeHead;
	while(currentNode->next != NULL){
		currentNode = currentNode->next;
	}
	currentNode->next = (node*)malloc(sizeof(node));
	 if (currentNode->next == NULL) raise(SIGINT);
	//~ currentNode->name = strndup(name,nameSize);
	currentNode->name = (char*)calloc(1,sizeof(char)*nameSize+1);
	strncpy(currentNode->name,name,nameSize);
	currentNode->fd = fd;
	currentNode->idleTime = idleTime;
	currentNode->next->next = NULL;
}

//Finds and removes fd from list
//Preassumption: fd is inside of list of nodes, each fd is unique
node* removeFd(node* nodeHead, int fd){
	//Special case: node is head
	if(nodeHead->fd == fd){
		return nodeHead;
	}
	node* currentNode = nodeHead;
	while((currentNode->next != NULL) && (currentNode->next->fd != fd)){
		currentNode = currentNode->next;
	}
	//Actually returns node before found node for deletion purposes
	node* tempNode = currentNode->next;
	currentNode->next = currentNode->next->next;
	return tempNode;
}

//Finds fd
//Preassumption: fd is inside of list of nodes, each fd is unique
node* findFd(node* nodeHead, int fd){
	node* currentNode = nodeHead;
	while(currentNode->fd != fd){
		currentNode = currentNode->next;
	}
	return currentNode;
}

//finds if name is inside list of nodes
int findName(node* nodeHead, char* name){
	node* currentNode = nodeHead;
	int length = strlen(name);
	while(currentNode->next != NULL && strncmp(currentNode->name,name,length)!=0){
		currentNode = currentNode->next;
	}
	return currentNode->next != NULL;
}

//Counts nodes with names in them
unsigned short nameCount(node* nodeHead){
	node* currentNode = nodeHead;
	unsigned short num = 0;
	while(currentNode->next!=NULL){
		 ++num;
		 currentNode = currentNode->next;
	 }
	return num;
}

//Deletes all nodes
//Postcondition: nodeHead points to an unknown address
void deleteNodes(node* nodeHead){
	node* currentNode = nodeHead;
	while(nodeHead!=NULL){
		currentNode = nodeHead;
		nodeHead = nodeHead->next;
		free(currentNode);
	}
}


//send_message()
//sends message to specified socket while checking for errors
//returns 0 if socket closed or error occurred, returns 1 if send was successful
int send_message(int s, void* address, int byte_length){
	if(send(s, address, byte_length, 0) <= 0){
		return 0;
	}
	return 1;
}

//like send_message, but for receive
int receive_message(int s, void* address, int byte_length){
	if(recv(s, address, byte_length, 0) <= 0){
		return 0;
	}
	return 1;
}

int main(int argc, char* argv[])
{	
	char* fileName = (char*)malloc(256);
    if(fileName == NULL){
		exit (1);
	}
	
	//Daemon initialization
	pid_t pid = 0;
    pid_t sid = 0;
    pid = fork();
    if (pid < 0){
        //perror("fork failed!\n");
        exit(1);
    }

    if (pid > 0){
		//parent process waits until receives SIGINT, then kills child
		void psignal_handler(int sig){
			kill(pid,SIGINT);
			wait(NULL);
			exit(0);
		}
		
		struct sigaction segv;
		segv.sa_handler = psignal_handler;
		sigemptyset(&segv.sa_mask);
		segv.sa_flags = 0;
		sigaction(SIGINT,&segv,0);
		
		while(1){
			sleep(0.5);//Busy wait
		}
		exit(0); 
    }
    umask(0);
    // open a log file
             
	sprintf (fileName, "server379%d.log", getpid());

    
    FILE* fp;
    fp = fopen (fileName, "w+");
    if(!fp){
    	printf("cannot open log file");
    }
    // create new process group
    sid = setsid();
    if(sid < 0)
    {
    	fprintf(fp, "cannot create new process group");
        exit(1);
    }
    /* Change the current working directory */ 
    if ((chdir("/")) < 0) {
      fprintf(fp,"Could not change working directory to /\n");
      exit(1);
    }
	// close standard fds
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
	
	//Initializing stuff
	int	sock, fromlength, snew, i, j, nbytes, bool;
	unsigned short tempNum, tempNum2;
	unsigned char tempChar, tempChar2;
	time_t currentTime = time(NULL);
	
	//Set up example username list
	node* nodeHead = (node*)malloc(sizeof(node));
	nodeHead->next = NULL;
	node* currentNode;
	
	char* fileBuffer = (char*)malloc(256);
    if(fileBuffer == NULL){
		exit (1);
	}
	char* nameBuffer = (char*)malloc(256);
    if(nameBuffer == NULL){
		//~ sprintf(fileBuffer,"Server: failed to allocate enough memory");
		//~ fputs(fileBuffer, fp)
		//~ exit(1);
		fputs("Server: failed to allocate enough memory\n",fp);
		exit (1);
	}
	char* messageBuffer = (char*)malloc(sizeof(char)*65536);
    if(messageBuffer == NULL){
		fputs("Client: failed to allocate enough memory\n",fp);
		exit (1);
	}
	
	struct	sockaddr_in	master, from;
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		fputs("Server: cannot open master socket\n",fp);
		exit (1);
	}
	memset(&master, 0, sizeof (master));
	master.sin_family = AF_INET;
	master.sin_addr.s_addr = inet_addr("127.0.0.1");//replaced INADDR_ANY
	master.sin_port = htons (atoi(argv[1]));
	if (bind (sock, (struct sockaddr*) &master, sizeof (master))) {
		fputs("Server: cannot bind master socket\n",fp);
		exit (1);
	}
	if (listen(sock, 10) == -1) {
        fputs("Server: listen failed\n",fp);
        exit(-1);
    }
	
	//Done Initialization
	
	//Set up signal handlers
	void signal_handler(int sig){
		fputs("Terminating...\n",fp);
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
	
	// main loop for original server.
    while(1){
		tv.tv_sec = 2;
        read_fds = master_fds;
        time(&currentTime);//reset currentTime every loop
        nameBuffer = calloc(1,256);
        
        if (select(fdmax+1, &read_fds, NULL, NULL, &tv) == -1) {
            fputs("select\n",fp);
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
						fputs("Server: accept failed\n",fp);
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
							sprintf(fileBuffer,"Server: socket %d connected\n", snew);
							fputs(fileBuffer, fp);
							receive_message(snew, &tempChar, 1);
							//~ memset(nameBuffer, 0, tempChar+1);
							receive_message(snew, nameBuffer, tempChar);
							//Special Case: What if username is already logged in?
							if(findName(nodeHead, nameBuffer)==1){
								sprintf(fileBuffer,"Server: %s is already in server.\n",nameBuffer);
								fputs(fileBuffer, fp);
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
							fputs("Server: error sending usernames\n",fp);
							close(snew);
						}
                    }
                } else {
                    // handle data from a client
                    if (receive_message(i, &tempNum, 2) <= 0) {
                        // got error or connection closed by client
                        // connection closed -- should write to a log instead of STDOUT here
                        sprintf(fileBuffer,"Server: socket %d hung up\n", i);
						fputs(fileBuffer, fp);
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
							sprintf(fileBuffer,"Server: received idle message from %d\n",i);
							fputs(fileBuffer, fp);
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
					sprintf(fileBuffer,"Server: socket %d failed to send idle message\n", currentNode->fd);
					fputs(fileBuffer, fp);
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
