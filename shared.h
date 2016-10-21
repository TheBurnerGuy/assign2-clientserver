typedef struct node_{
	char* name;
	struct node_* next;
}node;

//Adds name to list
void addName(node* nodeHead, char* name){
	node* currentNode = nodeHead;
	while(currentNode->next != NULL){
		currentNode = currentNode->next;
	}
	currentNode->next = (node*)malloc(sizeof(node));
	currentNode->next->name = name;
	currentNode->next->next = NULL;
}

//Finds and removes name from list
//Preassumption: name is inside of list of nodes
void removeName(node* nodeHead, char* name){
	//Special case: node is head
	if(nodeHead->name == name){
		node* nodeTemp = nodeHead;
		nodeHead = nodeHead->next;
		free(nodeTemp);
		return;
	}
	node* currentNode = nodeHead;
	while((currentNode->next != NULL) && (currentNode->next->name != name)){
		currentNode = currentNode->next;
	}
	node* tempNode = currentNode->next;
	currentNode->next = currentNode->next->next;
	free(tempNode);
}

//Prints all names in list of nodes
void printNames(node* nodeHead){
	node* currentNode = nodeHead;
	fprintf(stderr, "Users:\n");
	while(currentNode!=NULL){
		fprintf(stderr, "%s", currentNode->name);
		currentNode = currentNode->next;
	}
}
