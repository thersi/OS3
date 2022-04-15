#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct Node {
    char *command;
    int exitStatus;
    int pid;
    struct Node *next;
};

struct Node *head = NULL;
struct Node *current = NULL;

//insert node as head
void insertNode(char *command, int pid) {
   //create a link
   struct Node *link = (struct Node*) malloc(sizeof(struct Node));
	
   link->pid = pid;
   link->command = command;
   link->exitStatus = NULL;
	
   //point it to old first node
   link->next = head;
	
   //point first to new first node
   head = link;
}

//display the zombies
void printCommands() {
   struct Node *node = head;
   struct Node *previous;
   //start from the beginning
   while(node != NULL) {
        if (&node->exitStatus != NULL) {
            printf("\n Exit status [%s] = %d \n",node->command,node->exitStatus);
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
        if (&node->exitStatus == NULL) {
            printf("\n pid: %d, command: %s", node->pid, node->command);
        }
        node = node->next;
   }
}


void main() {
    //void insertNode(char *command, int exitStatus, int pid) {
    insertNode("ls",1);
    insertNode("ls -l",2);
    insertNode("cd",3);
	
    //print list
    printf("printing commands");
    printCommands();
    printf("printing running processes\n");
    printRunning();

   
}