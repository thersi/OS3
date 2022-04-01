#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LETTERS 1000
#define MAX_LIST_SIZE 100

void init()
{
    printf("\033[H\033[J"); // Clears shell via escape sequences PRØV FINNE ALTERNATIV?
    char *user = getenv("ACTIVE_USER");
    printf("\n The ACTIVE_USER is: @%s \n", user);
    sleep(2);
    printf("\033[H\033[J");
}

int parseString(char *string)
{
    int error_signal;
    int status;
    char *buffer;
    buffer = readline(":");
    if (WIFEXITED(status))
    {
        int es = WEXITSTATUS(status);
        printf("Exit status was %d\n", es);
    }
    printf("Exit status [%s] = %d\n", string, error_signal);
    return 1;
}

void getDirectory()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nDir: %s", cwd);
}

void executeFork(char **parse) // Kall denne noe annet?
{
    pid_t pid = fork();

    if (pid == -1)
    {
        printf("\nFailed fork");
        return;
    }
    else if (pid == 0)
    {
        if (execvp(parse[0], parse) < 0)
        {
            printf("\nUnable to execute");
        }
        exit(0);
    }
    else
    {
        wait(NULL);
        return;
    }
}
int parse(char *str, char **pipes)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        pipes[i] = strsep(&str, ":");
        if (pipes[i] == NULL)
            break;
    }

    if (pipes[1] == NULL)
        return 0;
    else
    {
        return 1;
    }
}
void parseSpace(char *str, char **parsed) // KOmbiner i den metoden over
{
    int i;

    for (i = 0; i < MAX_LIST_SIZE; i++)
    {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}
int ownCmdHandler(char **parsed)
{
    int NoOfOwnCmds = 4, i, switchOwnArg = 0;
    char *ListOfOwnCmds[NoOfOwnCmds];
    char *username;

    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "hello";

    for (i = 0; i < NoOfOwnCmds; i++)
    {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0)
        {
            switchOwnArg = i + 1;
            break;
        }
    }

    switch (switchOwnArg)
    {
    case 1:
        printf("\nGoodbye\n");
        exit(0);
    case 2:
        chdir(parsed[1]);
        return 1;
    case 3:
        // openHelp();
        return 1;
    case 4:
        username = getenv("USER");
        printf("\nHello %s.\nMind that this is "
               "not a place to play around."
               "\nUse help to know more..\n",
               username);
        return 1;
    default:
        break;
    }

    return 0;
}
int stringPipe(char *str, char **parsedArg, char **pipes)
{

    char *strpiped[2];
    int piped = 0;

    piped = parse(str, strpiped);

    if (piped)
    {
        parseSpace(strpiped[0], parsedArg);
        parseSpace(strpiped[1], pipes);
    }
    else
    {

        parseSpace(str, parsedArg);
    }

    if (ownCmdHandler(parsedArg))
        return 0;
    else
        return 1 + piped;
}
void executePipes(char **parse, char **pipeParse)
{
    pid_t pipe0, pipe1;
    int pipes[2];
    if (pipe(pipes) < 0)
    {
        printf("\n An error occured in the initialization stage");
        return;
    }
    pipe0 = fork();
    if (pipe0 < 0)
    {
        printf("Unable to fork");
        return;
    }
    if (pipe0 == 0)
    {
        close(pipes[0]);
        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[1]);

        if (execvp(parse[0], pipeParse) < 0)
        {
            printf("\nUnable to execute the first command");
            exit(0);
        }
    }
    pipe1 = fork();

    if (pipe1 < 0)
    {
        printf("\nCould not fork");
        return;
    }
    if (pipe1 == 0)
    {
        close(pipes[1]);
        dup2(pipes[0], STDIN_FILENO);
        close(pipes[0]);
        if (execvp(pipeParse[0], pipeParse) < 0)
        {
            printf("\nUnable to execute the second command");
            exit(0);
        }
    }
    else
    {
        wait(NULL);
        wait(NULL);
    }
}

int main()
{
    char input[MAX_LETTERS], *parsedArgs[MAX_LIST_SIZE];
    char *parsedPipes[MAX_LIST_SIZE];
    int flag = 0;
    init();

    while (1)
    {
        getDirectory();
        if (parseString(input))
            continue;
        flag = stringPipe(input,
                          parsedArgs, parsedPipes);
        if (flag == 1)
            executeFork(parsedArgs);

        if (flag == 2)
            executePipes(parsedArgs, parsedPipes);
    }
    return 0;
}