#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#define MAX_SIZE 300

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100

int checkInput(char *inputString) // Kan gjøre dette på en annen måte?
{
    char *stringBuffer;

    stringBuffer = readline(": ");
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
    printf(" %s", pathBuffer);
    return 1;
}

int parseString(char *inputString, char **inputBuffer)
{

    int i;

    for (i = 0; i < MAXLIST; i++)
    {
        inputBuffer[i] = strsep(&inputString, " ");

        if (inputBuffer[i] == NULL)
            break;
        if (strlen(inputBuffer[i]) == 0)
            i--;
    }
    if (strcmp(inputBuffer[0], "cd") == 0)
    {
        chdir(inputBuffer[1]); // Sjekk om child skal gjøre dette.
    }
    return 1;
}

int executeProcess(char **inputBuffer)
{
    // Forking a child
    int status;
    pid_t pid = fork();
    char finalString[512] = "";
    if (pid == -1)
    {
        printf("\n Unable to fork");
        return 0;
    }
    else if (pid == 0)
    {
        if (strcmp(inputBuffer[0], "cd") == 0)
        {
            return 0;
        }
        if (execvp(inputBuffer[0], inputBuffer) < 0)
        {
            printf("\nUnable to execute :");
        }
        exit(0);
    }
    else
    {
        // Dette funker ikke for cd, se nærmere på det
        wait(&status);
        if (WIFEXITED(status))
        {
            int exitStatus = WEXITSTATUS(status);
            for (int i = 0; i < MAXLIST; i++)
            {
                if (!inputBuffer[i])
                {
                    break;
                }
                strcat(finalString, inputBuffer[i]);
                strcat(finalString, " ");
            }
            printf("\n %s", finalString);
            printf("\n Exit status [%s] = %d \n", finalString, exitStatus);
        }
        else if (WIFSIGNALED(status))
            psignal(WTERMSIG(status), "Exit signal");

        // wait(status); // Equivalent to waitPid(-1, &status, 0)
        return 1;
    }
}

int main()
{
    char inputString[MAXCOM], *inputBuffer[MAXLIST];
    char *parsedArgsPiped[MAXLIST];
    int flag = 0;
    // init_shell();

    while (1)
    {
        // print shell line
        activeDirectory();
        // take input
        if (checkInput(inputString))
            continue;
        // process
        flag = parseString(inputString,
                           inputBuffer);
        executeProcess(inputBuffer);
    }
    return 0;
}