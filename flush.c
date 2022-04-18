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
/*
// not necessary
char *
readToBuffer(const char *filename)
{
    FILE *file = fopen(filename, "r");
    char *readBuffer[1000];
    char *currentline[1000];

    assert(file != NULL);

    while (fgets((char *)currentline, sizeof(currentline), file) != NULL)
    {
        strcat((char *)readBuffer, (char *)currentline);
        // fprintf(stderr, "got line: %s\n", currentline);
    }
    printf("read: %s\n", (char *)readBuffer);

    fclose(file);

    char *str = (char *)readBuffer;

    return str;
} */
/*
// not necessary in finished shell
void readFromFileToFile(char *readfile, char *writefile)
{
    FILE *file = fopen(readfile, "r");

    char currentline[1000];

    assert(file != NULL);

    FILE *write = fopen(writefile, "w");
    rewind(write);

    while (fgets(currentline, sizeof(currentline), file) != NULL)
    {
        fprintf(write, "%s\n", (char *)&currentline);
    }

    fclose(file);
    fclose(write);
}

// not necessary
void *writeToFile(char **readfrom, char *writefile)
{
    assert(readfrom != NULL);
    printf("Nå skrives det greier");

    FILE *write = fopen(writefile, "w");
    rewind(write);
    fprintf(write, "%s\n", (char *)readfrom);
    fclose(write);
} */

int checkInput(char *inputString) // Kan gjøre dette på en annen måte?
{
    char *stringBuffer;
    stringBuffer = fgets(inputString, MAXLIST, stdin);
    if (stringBuffer == NULL) // Checks if null which is the case for ctrl-d, then exits
    {
        exit(EXIT_FAILURE); // SKal dette være exit kode 1 eller 0?
    }

    stringBuffer[strlen(stringBuffer) - 1] = '\0';

    // fflush(stdin);

    if (strlen(stringBuffer) > 0)

    {
        strcpy(inputString, stringBuffer);
        return 0;
    }
    return 1;
}

int activeDirectory()
{
    char pathBuffer[1024];
    if (getcwd(pathBuffer, sizeof(pathBuffer)) == NULL)
    {
        perror("Error when calling getcwd()");
        return 0;
    } // Gets the path name of the working directory
    printf(" %s: ", pathBuffer);
    return 1;
}
// function for finding pipe
int parsePipe(char *str, char **strpiped)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }

    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else
    {
        return 1;
    }
}
// function for parsing command words
void parseSpace(char *str, char **parsed)
{
    int i;

    for (i = 0; i < MAXLIST; i++)
    {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

int parseString(char *inputString, char **inputBuffer, char **inputpipe)
{
    output = dup(1);
    input = dup(0);

    int i;

    char *strpiped[2];
    int piped = 0;

    piped = parsePipe(inputString, strpiped);

    if (piped)
    {
        parseSpace(strpiped[0], inputBuffer);
        parseSpace(strpiped[1], inputpipe);
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

        // sjekker etter redirections
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

    /*for (int i = 0; i < MAXLIST; i++)
    {
        if (inputBuffer[i] == NULL)
            break;
        printf("commandbuffr: %s\n", commandBuffer[i]);
        printf("inputbuffr: %s\n", inputBuffer[i]);
    }*/

    // Forking a child
    /*     int background = 0;
        if (!strcmp(inputBuffer[strlen(*inputBuffer) - 1], "&"))
        {
            background = 1;
        } */
    int status;
    pid_t pid = fork();
    char finalString[512] = "";
    char *finalStr = finalString;
    if (!strcmp(inputBuffer[0], "cd")) // Fant ut av at denne ikke trenger være child process Ctrl d
    {
        chdir(inputBuffer[1]);
    }

    if (pid == -1)
    {
        printf("\n Unable to fork");
        return 0;
    }
    else if (pid == 0)
    {

        /*     if (writeIndex)
            {
                int newfd = open(inputBuffer[writeIndex], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                output = dup(1);
                dup2(newfd, STDOUT_FILENO);
                writeIndex = 0;
            } */
        if (readIndex)
        {

            int newfd = open(inputBuffer[readIndex], O_RDONLY);
            input = dup(0);
            dup2(newfd, STDIN_FILENO);
            readIndex = 0;
        }
        if (!strcmp(inputBuffer[0], "jobs"))
        {
            printRunning();
        }
        else
        {
            execvp(commandBuffer[0], commandBuffer);
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
        if (bg)
        {
            insertNode(*inputBuffer, pid);
            return 0;
        }

        // Dette funker ikke for cd, se nærmere på det
        // legge inn først sjekk etter & og terminere etter den funksjonaliteten dersom det finnes
        waitpid(pid, &status, 0);

        /*/stdout and stdin back to console
        dup2(output, 1);
        close(output);
        close(inputBuffer[writeIndex]);
        dup2(input, 1);
        close(input);
        close(inputBuffer[readIndex]);*/
        if (WIFEXITED(status))
        {
            int exitStatus = WEXITSTATUS(status);

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
            printf("\n Exit status [ %s] = %d \n", finalStr, exitStatus);
        }
        else if (WIFSIGNALED(status))
            psignal(WTERMSIG(status), "Exit signal");

        // wait(status); // Equivalent to waitPid(-1, &status, 0)
        return 1;
    }
}

// Function where the piped system commands is executed
void execArgsPiped(char **parsed, char **parsedpipe)
{
    // 0 is read end, 1 is write end
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0)
    {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0)
    {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0)
    {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0)
        {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    }
    else
    {
        // Parent executing
        p2 = fork();

        if (p2 < 0)
        {
            printf("\nCould not fork");
            return;
        }

        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0)
        {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);

            /*  if (readIndex)
             {

                 int newfd = open(parsedpipe[readIndex], O_RDONLY);
                 input = dup(0);
                 dup2(newfd, STDIN_FILENO);
                 readIndex = 0;
             } */
            if (execvp(parsedpipe[0], parsedpipe) < 0)
            {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        }
        else
        {
            // parent executing, waiting for two children
            wait(NULL);
            wait(NULL);
        }
    }
}

int main()
{
    char inputString[MAXCOM], *inputBuffer[MAXLIST], *commandBuffer[MAXLIST];
    char *inputPipes[MAXLIST];
    int flag = 0;
    // init_shell();
    // char *currentline[MAXLIST];

    // char *readfrom = readToBuffer("textfile.txt");
    // writeToFile(( char ** )readfrom, "writefile.txt");

    while (1)
    {
        bg = 0;
        // printRunning();
        //  print shell line
        activeDirectory();
        // take input
        if (checkInput(inputString))
            continue;

        // process
        flag = parseString(inputString, inputBuffer, inputPipes);
        findCommand(inputBuffer, commandBuffer);

        printf("%d \n", flag); // kommer hit
        if (flag == 1)
            executeProcess(inputBuffer, commandBuffer);
        ;
        if (flag == 2)
            execArgsPiped(commandBuffer, inputPipes); // noe feiler her
        removeZombies();
        //    bg = 0;

        /*/stdout and stdin back to console
        dup2(output, 1);
        close(output);
        close(inputBuffer[writeIndex]);
        dup2(input, 1);
        close(input);
        close(inputBuffer[readIndex]);*/
    }
    return 0;
}
