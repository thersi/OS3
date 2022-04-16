#include <stdio.h>
#include <stdlib.h>

struct Node {
    char *command;
    int exitStatus;
    int pid;
    struct Node *next;
    int active;
};

struct Node *head = NULL;
struct Node *current = NULL;

//insert node as head
void insertNode(char *command, int pid) {
   //create a link
   struct Node *node = (struct Node*) malloc(sizeof(struct Node));
	
   node->pid = pid;
   node->command = command;
   node->exitStatus = 0;
   node->active = 1;
	
   //point it to old first node
   node->next = head;
	
   //point first to new first node
   head = node;
}

int isActive(struct Node* node) {
    if (node->active) {
        return 1;
    }
    return 0;
}

//display the zombies
void printCommands() {
   struct Node *node = head;
   struct Node *previous;
   //start from the beginning
   while(node != NULL) {
        if (!isActive(node)) {
            printf("\nExit status [%s] = %d \n",node->command,node->exitStatus);
            if(node == head) {
            //change first to point to next link
                head = head->next;
            } else {
            //bypass the current link
                previous->next = node->next;
            }
        }
        else {
            previous = node;
        }
        node = node->next;
   }
}

//display the active processes
void printRunning() {
   struct Node *node = head;
   //start from the beginning
   while(node != NULL) {
        if (isActive(node)) {
            printf("pid: %d, command: %s\n", node->pid, node->command);
        }
        node = node->next;
   }
}

//find a link with given key
struct Node* find(int pid) {

   //start from the first link
   struct Node* node = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(node->pid != pid) {
	
      //if it is last node
      if(node->next == NULL) {
         return NULL;
      } else {
         //go to next link
         node = node->next;
      }
   }      
	
   //if data found, return the current Link
   return node;
}

void setExitStatus(int pid, int exitStatus) {
    struct Node* node = find(pid);

    node->exitStatus = exitStatus;
    node->active = 0;
}

//trenger en som fjerner zombes også så de ikke kommer hver gang & kalles


void main() {
    insertNode("ls",1);
    insertNode("ls -l",2);
    insertNode("cd",3);
	
    printf("printing commands\n");
    printCommands();
    printf("\nprinting running processes\n");
    printRunning();

    setExitStatus(2, 3);

    printf("printing commands\n");
    printCommands();
    printf("\nprinting running processes\n");
    printRunning();

   
}