#include <stdio.h>

// insert node as head
void insertNode(char *command, int pid);

// display the zombies
void printCommands();

// display the active processes
void printRunning();

// find a link with given key
struct Node *find(int pid);

void setExitStatus(int pid, int exitStatus);

int getZombie(int pid);

int removeZombies();