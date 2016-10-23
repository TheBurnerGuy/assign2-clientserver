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
	currentNode->name = name;
	currentNode->next->next = NULL;
	
}

//Finds and removes name from list
//Preassumption: name is inside of list of nodes, each username is unique
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

//Prints all names in list of nodes
void printNames(node* nodeHead){
	node* currentNode = nodeHead;
	printf("Users:\n");
	while(currentNode->name!=NULL){
		printf("%s\n", currentNode->name);
		currentNode = currentNode->next;
	}
}

//Deletes all nodes
//Postcondition: nodeHead points to an unknown address
//Sample code:
/*
deleteNode = removeName(headNode,"Gary");
if (headNode->name == deleteNode->name){
	headNode = headNode->next;
}
free(deleteNode);
*/
void deleteNodes(node* nodeHead){
	node* currentNode = nodeHead;
	while(nodeHead!=NULL){
		currentNode = nodeHead;
		nodeHead = nodeHead->next;
		free(currentNode);
	}
}
