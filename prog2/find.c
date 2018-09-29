#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// FUNCTION TO THROW ERRORS => INFLUENCED BY GRAPHQL QUERY ERROR REPORTING
void throwError(char *message, char *file)
{
    if (file)
        fprintf(stderr, "%s [%s]: Error code %i: %s\n", message, file, errno, strerror(errno));
    else
        fprintf(stderr, "%s. Proper usage of the arguments is: [-b ###] [-o outfile]\n", message);
    exit(-1);
}

int main(int argc, char *argv[])
{
}
