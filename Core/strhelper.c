#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void parse_args(char *input, char ***argv, int *argc)
{
    *argc = 0;
    int i = 0;
    *argv = malloc(0);
    
    while (1)
    {
        if (input[i] == ' ' || input[i] == '\0')
        {
            char tmp = input[i];
            input[i] = '\0';

            *argv = realloc(*argv, (*argc + 1) * sizeof(char *));
            (*argv)[*argc] = malloc(strlen(input) + 1);
            strcpy((*argv)[*argc], input);

            input[i] = tmp;

            while (input[++i] == ' ')
            {
            }

            input = &(input[i]);
            i = 0;
            *argc = *argc + 1;
        }

        if (input[i] == '\0')
        {
            return;
        }

        i++;
    }
}
