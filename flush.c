#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <assert.h>
#define MAX_SIZE 300

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100

int readIndex = 0;
int writeIndex = 0;

char * readToBuffer(const char *filename) 
{
    FILE *file = fopen(filename, "r");
    char *readBuffer[1000];
    char *currentline[1000];

    assert(file != NULL);

    while (fgets(( char * )currentline, sizeof(currentline), file) != NULL) {
        strcat(( char * )readBuffer, ( char * )currentline);
        //fprintf(stderr, "got line: %s\n", currentline);

    }
    printf("read: %s\n", ( char * )readBuffer);

    fclose(file);

    char *str = ( char * )readBuffer;

    return str;
}

// Used to learn, possibly not necessary in finished shell
void readFromFileToFile(char *readfile, char *writefile) 
{
    FILE *file = fopen(readfile, "r");
    char currentline[1000];

    assert(file != NULL);
    
    FILE* write = fopen(writefile, "w");
    rewind(write);

    while (fgets(currentline, sizeof(currentline), file) != NULL) {
        fprintf(write, "%s\n", ( char * )&currentline);
    }

    fclose(file);
    fclose(write);
}

void * writeToFile(char **readfrom, char *writefile) 
{
    assert(readfrom != NULL);
    printf("Nå skrives det greier");
    
    FILE* write = fopen(writefile, "w");
    rewind(write);
    fprintf(write, "%s\n", ( char * )readfrom);
    fclose(write);
}


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
            // use dup2 to assign file as input descriptor
            int newfd = open(inputBuffer[i+1]);
            //int dup2(*stdin, newfd);
            readIndex = i + 1;
        }
        else { readIndex = 0; }
        if (!strcmp(inputBuffer[i], ">"))
        {
            // something to write to file
            //int dup2(*stdout, *inputBuffer[i+1]);
            writeIndex = i + 1;
        }
        else { writeIndex = 0; }
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
    char* finalStr = finalString;
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
        //if (execlp( "/bin/sh", "/bin/sh", "-c", inputBuffer, (char *)NULL ))
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
                strcat(finalStr, inputBuffer[i]);
                strcat(finalStr, " ");
            }
            // finalString er standard output
            // disse er ikke helt ferdige enda 
            /*if (readIndex) {
                finalStr = readToBuffer(inputBuffer[readIndex]);
            }
            if (writeIndex){
                writeToFile(inputBuffer[writeIndex-2], inputBuffer[writeIndex]);
            }
            printf("\nin %s", inputBuffer);*/
            //else { printf("\n %s", finalString); }
            printf("\n %s", finalStr);
            printf("\n Exit status [%s] = %d \n", finalStr, exitStatus);
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
    //char *currentline[MAXLIST];

    //char *readfrom = readToBuffer("textfile.txt");
    //writeToFile(( char ** )readfrom, "writefile.txt");



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