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
    printf("\n %s", pathBuffer);
    return 1;
}

void stripString(char *inputStr, char **inputBuffer)
{

    for (int i = 0; i < sizeof(MAX_SIZE); i++)
    {
        inputBuffer[i] = strsep(&inputStr, "\n");
        inputBuffer[i] = strsep(&inputStr, "\t");
    }
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

    printf("yo\n");
    return 1;
}

int executeProcess(char **inputBuffer)
{
    // Forking a child
    pid_t pid = fork();
    if (pid == -1)
    {
        printf("\n Unable to fork");
        return 0;
    }
    else if (pid == 0)
    {
        if (execvp(inputBuffer[0], inputBuffer) < 0)
        {
            printf("\nUnable to execute :");
        }
        exit(0);
    }
    else
    {
        wait(NULL);
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
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command
        // 2 if it is including a pipe.

        // execute
        /*if (execFlag == 1)
            execArgs(parsedArgs);

        if (execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);*/
        printf("%s\n", inputBuffer[0]);
        printf("%s\n", inputBuffer[1]);
        printf("%s\n", inputBuffer[2]);

        printf("made it here :) \n");
    }
    return 0;
}