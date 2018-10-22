#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

void throwError(char *message, char *file)
{
    if (file)
        fprintf(stderr, "%s [%s]. Error code %i: %s.\n", message, file, errno, strerror(errno));
    else
        fprintf(stderr, "%s\n", message);
}

int mycd(char *path)
{
    char *location;
    if (path == NULL)
    {
        location = getenv("HOME");
    }
    else
    {
        location = path;
    }

    if (chdir(location) < 0)
    {
        throwError("Error: Could not change to directory", path);
    }
    return 0;
}

void myexit(char *code)
{
    if (code != NULL)
        exit(atoi(code));
    exit(0);
}

int mypwd()
{
    char cwd[4096];

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);
    }
    else
    {
        throwError("Error: Could not print current directory due to an error with getcwd().", NULL);
    }
    return 0;
}

int run(char **argVec, char *redirFile, int redirFD, int redirMode)
{
    int fd, status;
    pid_t process, waiting;
    struct rusage rusage;
    struct timeval start, end;

    if ((gettimeofday(&start, NULL)) < 0)
    {
        throwError("Error: Unable to start timing command execution.", NULL);
        return 1;
    }
    switch ((process = fork()))
    {
    case -1:
        throwError("Error: Unable to execute command due to an error in the fork process", NULL);
        exit(1);
        break;
    case 0:
        if (redirFD > -1)
        {
            if ((fd = open(redirFile, redirMode, 0666)) < 0)
            {
                throwError("Error: Unable to open file for redirection", redirFile);
                return 1;
            }
            if (dup2(fd, redirFD) < 0)
            {
                throwError("Error: Unable to redirect to appropriate file.", NULL);
                return 1;
            }
            if (close(fd) < 0)
            {

                throwError("Error: Unable to close redirected file descriptor.", NULL);
                return 1;
            }
        }
        if (execvp(argVec[0], argVec) == -1)
        {
            throwError("Error: Unable to execute the command", argVec[0]);
            return 1;
        }
        //we should never get here
        break;
    default:
        if ((waiting = wait3(&status, WUNTRACED, &rusage)) < 0)
        {
            throwError("Error: Unable to wait for completion of child process", argVec[0]);
            return 1;
        };
        if ((gettimeofday(&end, NULL)) < 0)
        {
            throwError("Error: Unable to start timing command execution.", NULL);
            return 1;
        }
        fprintf(stderr, "Command returned with the return code: %i,\n", WEXITSTATUS(status));
        fprintf(stderr, "consuming %ld.%06u real seconds, %ld.%06u user, %ld.%06u system\n", (end.tv_sec - start.tv_sec), (end.tv_usec - start.tv_usec), rusage.ru_utime.tv_sec, rusage.ru_utime.tv_usec, rusage.ru_stime.tv_sec, rusage.ru_stime.tv_usec);
        break;
    }
    return 0;
}

void myshell(FILE *infile)
{
    char *line = NULL, *delims = " \r\n", *arg;
    int redirFD = -1, redirMode, i = 0;
    size_t len = 0;
    ssize_t nread;
    char *redirFile = malloc(BUFSIZ * (sizeof(char)));
    if (redirFile == NULL)
        throwError("Error: Failure to dynamically allocate memory.", NULL);
    char **argVec = malloc(BUFSIZ * (sizeof(char *)));
    if (argVec == NULL)
        throwError("Error: Failure to dynamically allocate memory.", NULL);

    while ((nread = getline(&line, &len, infile)) != -1)
    {
        if (line[0] == '#' || nread <= 1)
        {
            continue;
        }
        else
        {
            arg = strtok(line, delims);
            while (arg != NULL)
            {
                if (arg[0] == '<')
                {
                    redirFD = 0;
                    redirMode = O_RDONLY;
                    strcpy(redirFile, (arg + 1));
                }
                else if (arg[0] == '>')
                {
                    if (arg[1] == '>')
                    {
                        redirMode = O_WRONLY | O_APPEND | O_CREAT;
                        strcpy(redirFile, (arg + 2));
                    }
                    else
                    {
                        redirMode = O_WRONLY | O_TRUNC | O_CREAT;
                        strcpy(redirFile, (arg + 1));
                    }
                    redirFD = 1;
                }
                else if (arg[0] == '2' && arg[1] == '>')
                {
                    if (arg[2] == '>')
                    {
                        redirMode = O_WRONLY | O_APPEND | O_CREAT;
                        strcpy(redirFile, (arg + 3));
                    }
                    else
                    {
                        redirMode = O_WRONLY | O_TRUNC | O_CREAT;
                        strcpy(redirFile, (arg + 2));
                    }
                    redirFD = 2;
                }
                else
                {
                    argVec[i++] = arg;
                }
                arg = strtok(NULL, delims);
            }
            argVec[i] = NULL;
            if (strcmp(argVec[0], "pwd") == 0)
            {
                mypwd();
            }
            else if (strcmp(argVec[0], "cd") == 0)
            {
                mycd(argVec[1]);
            }
            else if (strcmp(argVec[0], "exit") == 0)
            {
                myexit(argVec[1]);
            }
            else
            {
                if ((run(argVec, redirFile, redirFD, redirMode)) > 0)
                {
                    exit(1);
                }
            }

            redirFD = -1;
            redirMode = 0;
            i = 0;
        }
    }

    free(redirFile);
    free(argVec);
    free(line);
    return;
}

int main(int argc, char *argv[])
{
    FILE *infile;

    if (argc > 1)
    {
        if ((infile = fopen(argv[1], "r")) == NULL)
        {
            throwError("Error: Unable to open input file", argv[1]);
            return -1;
        }
        myshell(infile);
    }
    else
    {

        myshell(stdin);
    }
    if (argc > 1 && fclose(infile) != 0)
    {
        throwError("Error: Unable to close input file", argv[1]);
        return -1;
    }
    fprintf(stderr, "EOF Characted Detected. Exiting myshell\n");
    return 0;
}
