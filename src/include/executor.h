#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stddef.h>
#include "rc.h"

rc_t EXECUTOR__execute_command(char** tokens, size_t tokens_size);

#endif