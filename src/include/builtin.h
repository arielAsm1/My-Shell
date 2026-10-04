#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdlib.h>
#include <stddef.h>
#include "rc.h"

#define MAX_COMMAND_NAME (64)

struct builtin_command_s
{
    char name[MAX_COMMAND_NAME];

    rc_t (*execute_command)(void *);
    rc_t (*prepare_arguments)(char **, size_t, void **);
};

rc_t BUILTIN__execute_command(char ** tokens, size_t tokens_size);

#endif