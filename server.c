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

int main(int argc, char* argv[])
{	
	//Daemon stuff should go here?
	
	//Initializing stuff
	int	sock, fromlength, snew;
	unsigned short tempNum;
	int PORT = atoi(argv[1]);
	struct	sockaddr_in	master, from;
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror ("Server: cannot open master socket");
		exit (1);
	}
	master.sin_family = AF_INET;
	master.sin_addr.s_addr = INADDR_ANY;
	master.sin_port = htons (PORT);
	if (bind (sock, (struct sockaddr*) &master, sizeof (master))) {
		perror ("Server: cannot bind master socket");
		exit (1);
	}
	if (listen(sock, 10) == -1) {
        perror("listen");
        exit(-1);
    }
	fromlength = sizeof (from);
	snew = accept (sock, (struct sockaddr*) & from, & fromlength);
	if (snew < 0) {
		perror ("Server: accept failed");
		exit (1);
	}
	//Done Initialization
	
	printf("I'm here!\n");
	
	//Set up signal handlers
	void signal_handler(int sig){
		perror("Terminating...");
		close(snew);
		exit(0);
	}
	struct sigaction segv;
	segv.sa_handler = signal_handler;
	sigemptyset(&segv.sa_mask);
	segv.sa_flags = 0;
	sigaction(SIGINT,&segv,0);
	
	//Set up example username list
	node* headNode = (node*)malloc(sizeof(node));
	headNode->next = NULL;
	addName(headNode,"Jerry");
	addName(headNode,"Gary");
	addName(headNode,"Barry");
	
	char* nameBuffer = (char*)malloc(256);
    if(nameBuffer == NULL){
		perror ("Client: failed to allocate enough memory");
		exit (1);
	}
	
	printf("I'm here!\n");
	
	//Begin handshake
	tempNum = 0xCFA7;
	send (snew, &tempNum, 2, 0);
	tempNum = htons(nameCount(headNode));
	send (snew, &tempNum, 2, 0);
	node* currentNode = headNode;
	char tempChar;
	while(currentNode->next!=NULL){
		tempChar = strlen(currentNode->name);
		send(snew, &tempChar, 1, 0);
		send(snew, currentNode->name, tempChar, 0);
		currentNode = currentNode->next;
	}
	recv(snew, &tempChar, 1, 0);
	tempNum = tempChar;
	recv(snew, nameBuffer, tempNum, 0);
	addName(headNode,nameBuffer);
	printNames(headNode); //just a test ;)
	//End handshake
	
	while(1){//Testing purposes... wait for signalC
		printf("Your orders Master...\n");
		sleep(1);
	}
	
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
	//~ 
    //~ // get us a socket and bind it
    //~ memset(&sa, 0, sizeof sa);
    //~ sa.sin_family = AF_INET;
    //~ sa.sin_addr.s_addr = inet_addr("127.0.0.1");
    //~ sa.sin_port = htons(MY_PORT);
           //~ 
    //~ if (bind(listener, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        //~ perror("bind");
    	//~ exit(-1);
    //~ }
//~ 
    //~ // listen
    //~ if (listen(listener, 10) == -1) {
        //~ perror("listen");
        //~ exit(-1);
    //~ }
//~ 
    //~ // add the listener to the master set
    //~ FD_SET(listener, &master);
//~ 
    //~ // keep track of the biggest file descriptor
    //~ fdmax = listener; // so far, it's this one
//~ 
    //~ // main loop
    //~ for(;;) {
        //~ read_fds = master; // copy it
        //~ if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            //~ perror("select");
            //~ exit(-1);
        //~ }
//~ 
        //~ // run through the existing connections looking for data to read
        //~ for(i = 0; i <= fdmax; i++) {
            //~ if (FD_ISSET(i, &read_fds)) { // we got one!!
                //~ if (i == listener) {
                    //~ // handle new connections
                    //~ addrlen = sizeof remoteaddr;
                    //~ newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
//~ 
                    //~ if (newfd == -1) {
                        //~ perror("accept");
                    //~ } else {
                        //~ FD_SET(newfd, &master); // add to master set
                        //~ if (newfd > fdmax) {    // keep track of the max
                            //~ fdmax = newfd;
                        //~ }
                        //~ printf("selectserver: new connection from %s:%d on socket %d\n",
                            //~ inet_ntoa(remoteaddr.sin_addr), ntohs(remoteaddr.sin_port), newfd);
                    //~ }
                //~ } else {
                    //~ // handle data from a client
                    //~ if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        //~ // got error or connection closed by client
                        //~ if (nbytes == 0) {
                            //~ // connection closed
                            //~ printf("selectserver: socket %d hung up\n", i);
                        //~ } else {
                            //~ perror("recv");
                        //~ }
                        //~ close(i); // bye!
                        //~ FD_CLR(i, &master); // remove from master set
                    //~ } else {
                        //~ // we got some data from a client
                        //~ for(j = 0; j <= fdmax; j++) {
                            //~ // send to everyone!
                            //~ if (FD_ISSET(j, &master)) {
                                //~ // except the listener and ourselves
                                //~ if (j != listener && j != i) {
                                    //~ if (send(j, buf, nbytes, 0) == -1) {
                                        //~ perror("send");
                                    //~ }
                                //~ }
                            //~ }
                        //~ }
                    //~ }
                //~ } // END handle data from client
            //~ } // END got new incoming connection
        //~ } // END looping through file descriptors
    //~ } // END for(;;)--and you thought it would never end!
	//~ 
	return 0;
}
