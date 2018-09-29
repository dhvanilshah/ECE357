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
        printf("%s. Proper usage of the arguments is: [-b ###] [-o outfile]\n", message);
    exit(-1);
}

int main(int argc, char *argv[])
{
    int buffer = 4096, flag, fdo, fdi, amtRead, amtWritten = 0;
    char *outfile;
    // FIND THE [-b ###] AND [-o OUTFILE] OPTIONS USING GETOPT
    while ((flag = getopt(argc, argv, "b:o:")) != -1)
        switch (flag)
        {
        case 'b':
            buffer = atoi(optarg);
            break;
        case 'o':
            outfile = optarg;
            break;
        case '?':
            throwError("Error: Unknown argument supplied", NULL);
        default:
            throwError("Error: The arguments '-b' and '-o' require arguments", NULL);
        }
    char *buff = malloc((sizeof(char)) * buffer);
    //OPEN THE OUTFILE IF ONE IS SPECIFIED
    if (outfile)
    {
        fdo = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fdo < 0)
            throwError("Error: Unable to open the output file", outfile);
    }
    else
        fdo = STDOUT_FILENO;

    // CHECK IF ZERO INPUT => IF TRUE, TREAT IT LIKE A "-" BY APPENDING "-" TO ARGV
    if (optind == argc)
    {
        argc = 0, optind = 0;
        argv[argc++] = "-";
    }
    //ITERATE THROUGH ARGUMENTS STARTING AT CURRENT OPTIND+1 ENDING AT LAST ARGUMENT
    for (; optind < argc; ++optind)
    {
        if (!strcmp("-", argv[optind]))
        {
            fdi = STDIN_FILENO;
            argv[optind] = "stdin";
        }
        // OPEN INPUT FILE FOR READ
        else if ((fdi = open(argv[optind], O_RDONLY, 0666)) < 0)
        {
            throwError("Error: Unable to open the input file", argv[optind]);
        }
        // THE READ AND WRITE OPERATIONS WITH CORRECTION FOR PARTIAL WRITES
        while ((amtRead = read(fdi, buff, (sizeof(char)) * buffer)) != 0)
        {
            if (amtRead < 0)
            {
                throwError("Error: Could not read from the input file", argv[optind]);
            }
            else
            {

                while (amtWritten < amtRead)
                {
                    if ((amtWritten = write(fdo, buff, amtRead)) < 0)
                    {
                        throwError("Error: Could not write to the output file", outfile);
                    }

                    amtRead = amtRead - amtWritten;
                    buff = buff + amtRead;
                    amtWritten = 0;
                }
            }
        }
    }
    // CLOSE THE OUTPUT FILE IF IT ISNT STD OUT
    if (fdo != STDOUT_FILENO && close(fdo) < 0)
        throwError("Error: Could not close the output file", outfile);
    return 0;
}
