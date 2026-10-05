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
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char** argv = NULL;

    int stdin_pipe[2] = { 0, 0 };
    int stdout_pipe[2] = { 0, 0 };

    pid_t child_pid = 0;
    char buffer[READ_BUFFER_SIZE] = { 0 };

    fd_set readfds;
    int max_fd = 0;
    int select_ret = 0;
    ssize_t bytes_read = 0;
    size_t j = 0;

    CLEANUP_IF_TRUE((NULL == tokens) || (tokens_size == 0), RETURN_CODE__NULL_PARAM);

    argv = malloc((tokens_size + 1) * sizeof(char*));
    CLEANUP_IF_TRUE((NULL == argv), RETURN_CODE__MALLOC_FAILED);

    for (j = 0; j < tokens_size; j++)
    {
        argv[j] = tokens[j];
    }
    argv[tokens_size] = NULL;

    CLEANUP_IF_TRUE((pipe(stdin_pipe) == -1), RETURN_CODE__PIPE_FAILED);
    CLEANUP_IF_TRUE((pipe(stdout_pipe) == -1), RETURN_CODE__PIPE_FAILED);

    child_pid = fork();
    CLEANUP_IF_TRUE((child_pid < 0), RETURN_CODE__FORK_FAILED);

    if (child_pid == 0)
    {
        close(stdin_pipe[1]); 
        close(stdout_pipe[0]);

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);

        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        execvp(argv[0], argv);
        
        exit(RETURN_CODE__EXEC_FAILED);
    }
    else
    {
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        while (1)
        {
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(stdout_pipe[0], &readfds);

            max_fd = STDIN_FILENO;
            if (stdout_pipe[0] > max_fd)
            {
                max_fd = stdout_pipe[0];
            }

            select_ret = select(max_fd + 1, &readfds, NULL, NULL, NULL);
            if (select_ret < 0)
            {
                break;
            }

            if (FD_ISSET(STDIN_FILENO, &readfds))
            {
                bytes_read = read(STDIN_FILENO, buffer, READ_BUFFER_SIZE);
                if (bytes_read < 0)
                {
                    break;
                }
                else if (bytes_read == 0)
                {
                    close(stdin_pipe[1]);
                    stdin_pipe[1] = 0;
                    break;
                }
                else
                {
                    write(stdin_pipe[1], buffer, bytes_read);
                }
            }

            if (FD_ISSET(stdout_pipe[0], &readfds))
            {
                bytes_read = read(stdout_pipe[0], buffer, READ_BUFFER_SIZE);
                if (bytes_read > 0)
                {
                    write(STDOUT_FILENO, buffer, bytes_read);
                }
                else
                {
                    break;
                }
            }
        }

        if (stdin_pipe[1] != 0) 
        {
            close(stdin_pipe[1]);
            stdin_pipe[1] = 0;
        }

        close(stdout_pipe[0]);
        stdout_pipe[0] = 0;

        CLEANUP_IF_TRUE((waitpid(child_pid, NULL, 0) == -1), RETURN_CODE__WAITPID_FAILED);
    }

    rc = RETURN_CODE__SUCCESS;

cleanup:
    if (NULL != argv)
    {
        free(argv);
    }

    if (stdin_pipe[0] > 0)
    {
        close(stdin_pipe[0]);
    }
    if (stdin_pipe[1] > 0)
    {
        close(stdin_pipe[1]);
    }
    if (stdout_pipe[0] > 0)
    {
        close(stdout_pipe[0]);
    }
    if (stdout_pipe[1] > 0)
    {
        close(stdout_pipe[1]);
    }

    return rc;
}