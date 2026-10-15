#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/select.h>

#include "executor.h"
#include "rc.h"

#define READ_BUFFER_SIZE (1024)

rc_t EXECUTOR__execute_command(char** tokens, size_t tokens_size)
{

}