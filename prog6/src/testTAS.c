#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "spin.h"

#define NUM_PROC 64
#define MYPROCS 4
#define NUMITR 1000000

void throwError(char *message, char *file)
{
    if (file)
        fprintf(stderr, "%s [%s]: Error code %i: %s\n", message, file, errno, strerror(errno));
    else
        fprintf(stderr, "%s\n", message);
    exit(-1);
}

int main(int argc, char const *argv[])
{
    int pid[MYPROCS], myPID = 0;

    unsigned long long idealCt = (MYPROCS * NUMITR);
    unsigned long long *counter = (unsigned long long *)mmap(NULL, sizeof(unsigned long long), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    unsigned long long *counterTAS = (unsigned long long *)mmap(NULL, sizeof(unsigned long long), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    char *lock = (char *)mmap(NULL, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (int i = 0; i < MYPROCS; i++)
    {
        if ((pid[i] = fork()) < 0)
        {
            throwError("Error: Failed to fork process.", NULL);
        }
        else if (pid[i] == 0)
        {
            myPID = 0;

            for (i = 0; i < NUMITR; i++)
            {
                *counter += 1;
            }

            for (i = 0; i < NUMITR; i++)
            {
                spin_lock(lock);
                *counterTAS += 1;
                spin_unlock(lock);
            }
            break;
        }
        else
            myPID = 1;
    }

    if (myPID)
    {
        for (int i = 0; i < MYPROCS; i++)
        {
            if (waitpid(pid[i], NULL, 0) < 0)
            {
                throwError("Error: Unable to wait for child process to complete", NULL);
            }
        }
        // PRINT OUT RESULTS
        fprintf(stderr, "IDEAL COUNT: %llu | NON-TAS COUNT: %llu | TAS COUNT: %llu\n", idealCt, *counter, *counterTAS);

        if ((munmap(counter, sizeof(unsigned long long)) < 0) || (munmap(counterTAS, sizeof(unsigned long long)) < 0))
            throwError("Error: Unable to munmap counter(s)", 0);

        if ((munmap(lock, sizeof(char)) < 0))
            throwError("Error: Unable to munmap lock", 0);
    }

    return 0;
}
