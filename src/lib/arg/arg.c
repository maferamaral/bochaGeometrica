#include "arg.h"
#include <string.h>

const char *getArgValue(int argc, char *argv[], char *arg)
{
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], arg) == 0)
        {
            if (argv[i + 1] == NULL)
            {
                return NULL;
            }
            return argv[i + 1];
        }
    }
    return NULL;
}
