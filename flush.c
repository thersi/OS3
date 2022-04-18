#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <assert.h>
#include "linkedlist.h"
#define MAX_SIZE 300

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int readIndex = 0;
int writeIndex = 0;
int output = 1;
int input = 0;
int bg = 0;

/* Takes in user input and moves to buffer */
int checkInput(char *inputString)
{
    char *stringBuffer;
    stringBuffer = fgets(inputString, MAXLIST, stdin);
    if (stringBuffer == NULL) // Checks if null which is the case for ctrl-d, then exits
    {
        exit(EXIT_FAILURE);
    }

    stringBuffer[strlen(stringBuffer) - 1] = '\0'; // Removes whitespace

    if (strlen(stringBuffer) > 0)

    {
        strcpy(inputString, stringBuffer);
        return 0;
    }
    return 1;
}

int activeDirectory() // Find current active directory
{
    char pathBuffer[1024];
    if (getcwd(pathBuffer, sizeof(pathBuffer)) == NULL)
    {
        perror("Error when calling getcwd()");
        return 0;
    }                            // Gets the path name of the working directory
    printf(" %s: ", pathBuffer); // Prints active directory before user input in terminal
    return 1;
}
// Finds pipe from user input
int pipeParser(char *stringInput, char **pipedString)
{
    int i;
    for (i = 0; i < 3; i++)
    {
        pipedString[i] = strsep(&stringInput, "|");
        if (pipedString[i] == NULL)
            break;
    }

    if (pipedString[1] == NULL)
        return 0; // zero is returned in case no pipe
    else if (pipedString[1] != NULL && pipedString[2] == NULL)
        return 1; // Checks how many pipes are found 1 or 2
    else
    {
        return 2;
    }
}
// Parses command via space
void parseCmd(char *inputString, char **commandBuffer)
{
    int i;

    for (i = 0; i < MAXLIST; i++)
    {
        commandBuffer[i] = strsep(&inputString, " "); // Seperated via space

        if (commandBuffer[i] == NULL)
            break;
        if (strlen(commandBuffer[i]) == 0)
            i--;
    }
}

int parseString(char *inputString, char **inputBuffer, char **inputpipe)
{
    output = dup(1);
    input = dup(0);

    int i;

    char *pipedString[2];
    int pipe = 0;

    pipe = pipeParser(inputString, pipedString); // Calls pipeParser function to find if there are any pipes

    if (pipe)
    {
        parseCmd(pipedString[0], inputBuffer);
        parseCmd(pipedString[1], inputpipe);
        if (pipe == 2) // In case of 2 pipes:
        {
            parseCmd(pipedString[2], inputpipe);
        }
        return 2;
    }

    for (i = 0; i < MAXLIST; i++)
    {
        char sep[3] = {' ', '\t', '\n'};

        inputBuffer[i] = strsep(&inputString, sep);

        if (inputBuffer[i] == NULL)
            break;
        if (strlen(inputBuffer[i]) == 0)
            i--;

        // checks for redirections
        if (!strcmp(inputBuffer[i], "<"))
        {
            // something to read from file
            readIndex = i + 1;

            // i--;
        }
        if (!strcmp(inputBuffer[i], ">"))
        {
            // something to write to file
            writeIndex = i + 1;
            // i--;
        }
        if (!strcmp(inputBuffer[i], "&"))
        {
            // task should be executed as background process
            bg = 1;
        }
    }
    return 1;
}

// Creates a commandbuffer from inputbuffer, this ensures certaincharacters are NULL, signaling end of buffer
void findCommand(char **inputBuffer, char **commandBuffer)
{
    int i;

    for (i = 0; i < MAXLIST; i++)
    {
        if (inputBuffer[i] == NULL)
        {
            commandBuffer[i] = NULL;
            break;
        }
        if (strlen(inputBuffer[i]) == 0)
        {
            i--;
        }
        if (!strcmp(inputBuffer[i], "<"))
        {
            commandBuffer[i] = NULL;
            break;
        }
        if (!strcmp(inputBuffer[i], ">"))
        {
            commandBuffer[i] = NULL;
            break;
        }
        if (!strcmp(inputBuffer[i], "&"))
        {
            commandBuffer[i] = NULL;
            break;
        }

        commandBuffer[i] = inputBuffer[i];
    }
}
int executeProcess(char **inputBuffer, char **commandBuffer)
{
    int status;
    pid_t pid = fork();
    char finalString[512] = "";
    char *finalStr = finalString;
    if (!strcmp(inputBuffer[0], "cd")) // Compares user input to 'cd'
    {
        chdir(inputBuffer[1]); // cd has own command, not covered by execvp. Not required to be placed in child.
    }

    if (pid == -1)
    {
        printf("\n Unable to fork");
        return 0;
    }
    else if (pid == 0) // child pid
    {

        if (writeIndex) // For writitng to file
        {
            int newfd = open(inputBuffer[writeIndex], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            output = dup(1);
            dup2(newfd, STDOUT_FILENO);
            writeIndex = 0;
        }
        if (readIndex) // For reading to file
        {

            int newfd = open(inputBuffer[readIndex], O_RDONLY);
            input = dup(0);
            dup2(newfd, STDIN_FILENO);
            readIndex = 0;
        }
        if (!strcmp(inputBuffer[0], "jobs")) // Compares user input to jobs command
        {
            printRunning(); // Calls on print function in linkedlist
        }
        else
        {
            execvp(commandBuffer[0], commandBuffer); // Cals execvp function, first parameter is the function called
        }

        dup2(output, 1);
        close(output);
        close(*inputBuffer[writeIndex]);
        dup2(input, 1);
        close(input);
        close(*inputBuffer[readIndex]);
        exit(EXIT_FAILURE);
    }
    else
    {
        if (bg) // Checks if background task
        {
            insertNode(*inputBuffer, pid); // Calls on insert node from linkedlist
            return 0;
        }

        // Waits for specific PID, not all (wait and waitpid(-1)) does this
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) // Checks if exited
        {
            int exitStatus = WEXITSTATUS(status); // Checks exit status

            for (int i = 0; i < MAXLIST; i++)
            {
                if (!inputBuffer[i])
                {
                    break;
                }
                strcat(finalStr, inputBuffer[i]);
                strcat(finalStr, " ");
            }
            printf("\n %s", finalStr);
            printf("\n Exit status [ %s] = %d \n", finalStr, exitStatus); // Prints exit status
        }
        else if (WIFSIGNALED(status))
            psignal(WTERMSIG(status), "Exit signal");

        return 1;
    }
}

// This function executes all piped system commands
void executePipes(char **commandBuffer, char **pipeBuffer)
{
    // 0 is read end, 1 is write end
    int pfd[2];
    pid_t pid1, pid2;

    if (pipe(pfd) < 0) // Checks if pipe initialization is true
    {
        printf("\n Initialization of pipe failed");
        return;
    }
    pid1 = fork();
    if (pid1 < 0)
    {
        printf("\nUnable to fork");
        return;
    }

    if (pid1 == 0)
    {

        // Child 1 executes
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[1]);

        if (execvp(commandBuffer[0], commandBuffer) < 0)
        {
            printf("\nUnable to execute first command");
            exit(0);
        }
    }
    else
    {
        // Parent executes
        pid2 = fork();

        if (pid2 < 0)
        {
            printf("\nUnable to fork");
            return;
        }

        if (pid2 == 0)
        {

            close(pfd[1]);
            dup2(pfd[0], STDIN_FILENO);
            close(pfd[0]);
            if (execvp(pipeBuffer[0], pipeBuffer) < 0)
            {
                printf("\nUnable to execute first command");
                exit(0);
            }
        }

        else
        {
            // parent executes and wait for children
            wait(NULL);
            wait(NULL);
        }
    }
}

int main()
{
    char inputString[MAXCOM], *inputBuffer[MAXLIST], *commandBuffer[MAXLIST];
    char *pipeBuffer[MAXLIST];
    int flag = 0;

    while (1)
    {
        bg = 0;

        //  print shell line
        activeDirectory();
        // take input
        if (checkInput(inputString))
            continue;

        // process
        flag = parseString(inputString, inputBuffer, pipeBuffer);
        findCommand(inputBuffer, commandBuffer);
        if (flag == 1) // If flag == 1, no pipes
            executeProcess(inputBuffer, commandBuffer);
        ;
        if (flag == 2) // flag == 2 indicates pipes
            executePipes(commandBuffer, pipeBuffer);
        removeZombies(); // removes zombies, function from linkedlist
    }
    return 0;
}
