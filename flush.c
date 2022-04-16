#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <assert.h>
#define MAX_SIZE 300

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int readIndex = 0;
int writeIndex = 0;
int output = 1;
int input = 0;
char pathBuffer[1024];
// not necessary
char *readToBuffer(const char *filename)
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
}

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
}

int checkInput(char *inputString) // Kan gjøre dette på en annen måte?
{
    char *stringBuffer;
    stringBuffer = fgets(inputString, MAXLIST, stdin);
    if (stringBuffer == NULL) // Checks if null which is the case for ctrl-d, then exits
    {
        exit(0);
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

    if (getcwd(pathBuffer, sizeof(pathBuffer)) == NULL)
    {
        perror("Error when calling getcwd()");
        return 0;
    } // Gets the path name of the working directory
    printf(" %s:", pathBuffer);
    return 1;
}

int parseString(char *inputString, char **inputBuffer)
{
    output = dup(1);
    input = dup(0);

    int i;

    for (i = 0; i < MAXLIST; i++)
    {
        char sep[2] = {' ', '\t'};

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
    }

    if (strcmp(inputBuffer[0], "cd") == 0)
    {
        chdir(inputBuffer[1]); // Sjekk om child skal gjøre dette.
    }
    return 1;
}

int executeProcess(char **inputBuffer)
{
    if (writeIndex)
    {
        int newfd = open(inputBuffer[writeIndex], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        output = dup(1);
        dup2(newfd, STDOUT_FILENO);
        writeIndex = 0;
    }
    if (readIndex)
    {
        int newfd = open(inputBuffer[readIndex], O_RDONLY);
        input = dup(0);
        dup2(newfd, STDIN_FILENO);
        readIndex = 0;
    }
    // Forking a child
    int status;
    pid_t pid = fork();
    char finalString[512] = "";
    char *finalStr = finalString;

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
        // if (execvp(inputBuffer[0], strncpy(execBuffer, &inputBuffer, sizeof(inputBuffer)-MIN(readIndex, writeIndex)-1)) < 0)
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
        // stdout and stdin back to console
        dup2(output, 1);
        close(output);
        dup2(input, 1);
        close(input);
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

int main()
{

    char inputString[MAXCOM], *inputBuffer[MAXLIST];
    char *parsedArgsPiped[MAXLIST];
    int flag = 0;
    // init_shell();
    // char *currentline[MAXLIST];

    // char *readfrom = readToBuffer("textfile.txt");
    // writeToFile(( char ** )readfrom, "writefile.txt");

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