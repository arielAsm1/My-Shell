#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "executor.h"
#include "rc.h"

#define READ_BUFFER_SIZE (1024)

rc_t EXECUTOR__execute_command(char** tokens, size_t tokens_size)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char** argv = NULL;
    int pipefd[2] = { 0, 0};
    pid_t child_pid = 0;
    char buffer_read[READ_BUFFER_SIZE] = { 0 };
    ssize_t read_size = 0;
    size_t i = 0;
    size_t j = 0;

    CLEANUP_IF_TRUE((NULL == tokens), RETURN_CODE__NULL_PARAM);

    argv = malloc((tokens_size + 1) * sizeof(char*));
    CLEANUP_IF_TRUE((NULL == argv), RETURN_CODE__MALLOC_FAILED);

    for (j = 0; j < tokens_size; j++)
    {
        argv[j] = tokens[j];
    }
    
    argv[tokens_size] = NULL;


    CLEANUP_IF_TRUE((pipe(pipefd) == -1), RETURN_CODE__PIPE_FAILED);

    child_pid = fork();
    CLEANUP_IF_TRUE((child_pid < 0), RETURN_CODE__FORK_FAILED);

    if (child_pid == 0)
    {
        close(pipefd[0]); 

        dup2(pipefd[1], 1); 
        close(pipefd[1]);

        execvp(argv[0], argv);
        
        exit(RETURN_CODE__EXEC_FAILED);
    }
    else
    {
        close(pipefd[1]); 

        while (1)
        {
            read_size = read(pipefd[0], buffer_read + i, 1);
            CLEANUP_IF_TRUE((read_size < 0), RETURN_CODE__READ_FAILED);

            if (read_size == 0 || i >= READ_BUFFER_SIZE - 1)
            {
                break;
            }
            i++;
        }
        buffer_read[i] = '\0';

        if (i > 0)
        {
            printf("\n%s\n", buffer_read);
        }

        CLEANUP_IF_TRUE((waitpid(child_pid, NULL, 0) == -1), RETURN_CODE__WAITPID_FAILED);
    }

    rc = RETURN_CODE__SUCCESS;

cleanup:
    if (NULL != argv)
    {
        free(argv);
    }

    if (pipefd[0] != -1)
    {
        close(pipefd[0]);
    }
    
    if (pipefd[1] != -1)
    {
        close(pipefd[1]);
    }

    return rc;
}