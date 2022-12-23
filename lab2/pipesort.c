// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING

// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Kevin Ding

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>
#include <math.h>

#define OUTPUT_FORMAT "%-10d%s\n"

void reset(char *array, int size)
{
    memset(array, 0, size);
}

int size(char *buffer, int total) // count the length of the string coming out of fgets
{
    int count = 0;
    for (int i = 0; i < total; i++)
    {
        if (!isalpha(buffer[i])) // means either new line or string terminator
            return count;
        count++;
    }
    return count;
}
int main(int argc, char *argv[])
{
    int option;
    int nflag = 0;
    int sflag = 0;
    int lflag = 0;
    int n = 1;
    int s;
    int l;
    int pipe1[2]; // from parsing to sorting
    int pipe2[2]; // from sorting to output results

    while ((option = getopt(argc, argv, "n:s:l:")) != -1)
    {
        switch (option)
        {
        case 'n':
        {
            nflag++;
            n = strtol(optarg, &optarg, 10);
            break;
        }
        case 's':
        {
            sflag++;
            s = strtol(optarg, &optarg, 10);
            break;
        }
        case 'l':
        {
            lflag++;
            l = strtol(optarg, &optarg, 10);
            break;
        }
        }
    }

    if (!(nflag == 1 && sflag == 1 && lflag == 1))
    {
        fprintf(stderr, "Usage error, refer to documentation.\n");
        exit(1);
    }

    if (n != 1)
    {
        fprintf(stderr, "This program only deals with 1 sorters.\n");
        exit(1);
    }

    if (l < s)
    {
        fprintf(stderr, "Input error, refer to documentation.\n");
        exit(1);
    }

    // create pipes
    int id = pipe(pipe1);
    if (id == -1)
    {
        fprintf(stderr, "Failed to create pipe.\n");
        exit(1);
    }
    id = pipe(pipe2);
    if (id == -1)
    {
        fprintf(stderr, "Failed to create pipe.\n");
        exit(1);
    }

    int pid = fork();
    // sort process first child
    if (pid == 0)
    {
        close(pipe1[1]);
        close(pipe2[0]);
        dup2(pipe1[0], 0); // first pipe read end to standard input
        close(pipe1[0]);
        dup2(pipe2[1], 1); // second pipe write end to standard output
        close(pipe2[1]);
        execlp("sort", "sort", NULL);
    }
    pid = fork();
    // merge process second child
    if (pid == 0)
    {
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]);
        dup2(pipe2[0], 0);
        close(pipe2[0]);

        char current[l + 1];
        reset(current, l + 1);
        char template[l + 1];
        reset(template, l + 1);
        int occurs = 1;
        while (fgets(current, l + 1, stdin) != NULL)
        {
            // process strings
            for (int i = 0; i < l + 1; i++)
            {
                if (!isalpha(current[i]))
                {
                    current[i] = '\0';
                    break;
                }
            }
            // only need >s length string
            if (size(current, l + 1) > s)
            {
                if (template[0] == '\0')
                {
                    strcpy(template, current);
                }
                else
                {
                    int diff = strcmp(current, template);
                    if (diff == 0) // strings are the same
                    {
                        occurs++;
                    }
                    else
                    {
                        // different strrings, so output one and update
                        printf(OUTPUT_FORMAT, occurs, template);
                        occurs = 1;
                        reset(template, l + 1);
                        strcpy(template, current);
                    }
                }
            }
        }
        // this part takes care of the last template string
        if (template[0] != '\0')
            printf(OUTPUT_FORMAT, occurs, template);

        exit(0);
    }

    // parent process for parsing strings
    close(pipe2[0]);
    close(pipe2[1]);
    close(pipe1[0]);
    FILE *text = fdopen(pipe1[1], "w");
    char c = fgetc(stdin);
    int count = 0;
    while (c != EOF)
    {
        if (isalpha(c) && count < l)
        {
            c = tolower(c);
            fputc(c, text);
            count++;
        }
        if (!isalpha(c))
        {
            if (count > 0)
                fputc('\n', text);
            count = 0;
        }
        c = fgetc(stdin);
    }
    fclose(text); // close writing end
    close(pipe1[1]);
    wait(NULL); // wait for each child
    wait(NULL);
}
