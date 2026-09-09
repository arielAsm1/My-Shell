#ifndef BUILTIN_H
#define BUILTIN_H

#include "rc.h"

#define MAX_COMMAND_NAME (64)

struct builtin_command_s
{
    char name[MAX_COMMAND_NAME];

    rc_t (*execute_command)(void *);
};

#endif