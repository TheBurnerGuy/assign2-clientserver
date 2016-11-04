typedef struct node_{
	char* name;
	struct node_* next;
	int fd;
	time_t idleTime;
}node;

//Adds name to list
void addName(node* nodeHead, char* name){
	node* currentNode = nodeHead;
	while(currentNode->next != NULL){
		currentNode = currentNode->next;
	}
	currentNode->next = (node*)malloc(sizeof(node));
	currentNode->name = name;
	currentNode->next->next = NULL;
}

void addNameServer(node* nodeHead, char* name, int fd, time_t idleTime){
	node* currentNode = nodeHead;
	while(currentNode->next != NULL){
		currentNode = currentNode->next;
	}
	currentNode->next = (node*)malloc(sizeof(node));
	if (currentNode->next == NULL) raise(SIGINT);
	currentNode->name = name;
	currentNode->fd = fd;
	currentNode->idleTime = idleTime;
	currentNode->next->next = NULL;
}

//Finds and removes name from list
//Preassumption: name is inside of list of nodes, each username is unique
//Example:if (nodeHead->name == deleteNode->name){
			//nodeHead = nodeHead->next;
		//}
		//free(deleteNode);
node* removeName(node* nodeHead, char* name){
	//Special case: node is head
	if(nodeHead->name == name){
		node* tempNode = nodeHead;
		return tempNode;
	}
	node* currentNode = nodeHead;
	while((currentNode->next != NULL) && (currentNode->next->name != name)){
		currentNode = currentNode->next;
	}
	//Actually returns node before found node for deletion purposes
	node* tempNode = currentNode->next;
	currentNode->next = currentNode->next->next;
	return tempNode;
}

//Finds and removes fd from list
//Preassumption: fd is inside of list of nodes, each fd is unique
node* removeFd(node* nodeHead, int fd){
	//Special case: node is head
	if(nodeHead->fd == fd){
		node* tempNode = nodeHead;
		return tempNode;
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

//Prints all names in list of nodes
void printNames(node* nodeHead){
	node* currentNode = nodeHead;
	printf("Users:");
	while(currentNode->next!=NULL){
		printf(" %s,", currentNode->name);
		currentNode = currentNode->next;
	}
	printf("\n");
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
