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
	unsigned short tempNum;
	unsigned char tempChar;
	struct	sockaddr_in	master, from;
	
	//Set up example username list
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
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror ("Server: cannot open master socket");
		exit (1);
	}
	memset(&master, 0, sizeof master);
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
	
	//Initialize select and variables/structures it needs
	fd_set master_fds;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax = sock;        // maximum file descriptor number
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);
	FD_SET(sock, &master_fds);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof optval);
	
	// main loop for original server.
    while(1){
        read_fds = master_fds;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(-1);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    fromlength = sizeof (from);
					snew = accept (sock, (struct sockaddr*) & from, & fromlength);
					if (snew < 0) {
						perror ("Server: accept failed");
						//exit(1);
					}else{
                        //printf("selectserver: new connection from %s:%d on socket %d\n", inet_ntoa(remoteaddr.sin_addr), ntohs(remoteaddr.sin_port), newfd);
                        //Initial handshake
						tempNum = 0xCFA7;
						bool = send_message (snew, &tempNum, 2); //Assumes send() will return -1 if socket is disconnected
						tempNum = htons(nameCount(headNode));
						bool = send_message (snew, &tempNum, 2);
						currentNode = headNode;
						while(currentNode->next!=NULL){
							tempChar = strlen(currentNode->name);
							bool = send_message(snew, &tempChar, 1);
							bool = send_message(snew, currentNode->name, tempChar);
							currentNode = currentNode->next;
						}
						//If no error received from sending in socket...
						if(bool != 0){
							receive_message(snew, &tempChar, 1);
							tempNum = tempChar;
							receive_message(snew, nameBuffer, tempNum);
							//End handshake
							//Now add it to the server's data structures
							addNameServer(headNode,nameBuffer,snew);
							printNames(headNode); //just a test ;)
							FD_SET(snew, &master_fds); // add to master set
							if (snew > fdmax) {    // keep track of the max
								fdmax = snew;
							}
							//Send user connected message
						}
                    }
                } else {
                    // handle data from a client
					//Implementing this part without using fork for now, until can figure out a good fork implementation
					//Personal notes:
					//Advantages for using fork(): Doesn't have to search for socket's name using socket fd
					//Disadvantages: Have to keep each child process separate... (definitely possible)
                    if (receive_message(i, &tempNum, 2) <= 0) {
                        // got error or connection closed by client
                        if (tempNum == 0) {
                            // connection closed -- should write to a log instead of STDOUT here
                            printf("Server: socket %d hung up\n", i);
                        } else {
                            perror("Server: Error reading from socket");
                        }
                        //close socket and remove from master file descriptor
                        close(i);
                        FD_CLR(i, &master_fds);
						//if i was fdmax, use fdmax-1
						if (fdmax == i) --fdmax;
                        //remove node from list of nodes
                        currentNode = removeFd(headNode,i);
                        if (nodeHead == currentNode){
							nodeHead = nodeHead->next;
						}
						free(currentNode);
						//Send termination message to all clients
                    } else {
                        // we got data from a client
						receive_message(i, messageBuffer, tempNum);
						currentNode = findFd(headNode, i);
						nameBuffer = currentNode->name;
						tempChar = strlen(nameBuffer);
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {//is this really needed?
                                // except the listener and ourselves
                                if (j != listener && j != i) {
									//Send username length
									//Can probably reduce these 4 repetitions to a function--> will do later on
									if (send_message(j, tempChar, 1) == 0) {
                                        perror("send");
										continue;
										//might try to terminate connection here? Will test later
                                    }
									//Send username
									if (send_message(j, nameBuffer, tempChar) == 0) {
                                        perror("send");
										continue;
										//might try to terminate connection here? Will test later
                                    }
									//Send message length
									if (send_message(j, tempNum, 2) == 0) {
                                        perror("send");
										continue;
										//might try to terminate connection here? Will test later
                                    }
									//Send message
                                    if (send_message(j, messageBuffer, tempNum) == 0) {
                                        perror("send");
										continue;
										//might try to terminate connection here? Will test later
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
			if(){//need to implement time here
				
			}//END checking if process is idle
        } // END looping through file descriptors
    } // END while()
	
	return 0;
	
	
	
	
	
	//NOTE!!!!!!!!!
	//This is a copy paste from demo2... will revise!
	//Code below is not functional, do not use!
	//NOTE!!!!!

	//~ fd_set master;    // master file descriptor list
    //~ fd_set read_fds;  // temp file descriptor list for select()
    //~ int fdmax;        // maximum file descriptor number
//~ 
    //~ int listener;     // listening socket descriptor
    //~ int newfd;        // newly accept()ed socket descriptor
    //~ struct sockaddr_in sa; 
    //~ struct sockaddr_in remoteaddr; // client address
    //~ socklen_t addrlen;
//~ 
    //~ char buf[256];    // buffer for client data
    //~ int nbytes;
    //~ 
    //~ int yes=1;        // for setsockopt() SO_REUSEADDR, below
    //~ int i, j, rv;
//~ 
    //~ FD_ZERO(&master);    // clear the master and temp sets
    //~ FD_ZERO(&read_fds);
//~ 
	//~ listener = socket(AF_INET, SOCK_STREAM, 0);
}
