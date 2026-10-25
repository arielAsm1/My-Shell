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
    char ** argv = NULL;

    int stdin_pipe[2] = {0};
    int stdout_pipe[2] = {0};

    pid_t child_pid = 0;
    char buffer[READ_BUFFER_SIZE] = {0};
    
    fd_set readfds;
    int max_fd = 0;
    int select_ret = 0;
    ssize_t bytes_read = 0;
    ssize_t bytes_wrote = 0;
    size_t i = 0;
    int dup2_ret = 0;
    int close_ret = 0;

    CLEANUP_IF_TRUE(((NULL == tokens) || (0 == tokens_size)), RETURN_CODE__NULL_PARAM);
    
    argv = (char**)malloc((tokens_size + 1) * sizeof(char *));
    CLEANUP_IF_TRUE((NULL == argv), RETURN_CODE__MALLOC_FAILED);

    for(i = 0; i < tokens_size; i++)
    {
        argv[i] = tokens[i];
    }

    argv[tokens_size] = NULL;

    CLEANUP_IF_TRUE((-1 == pipe(stdin_pipe)), RETURN_CODE__PIPE_FAILED);
    CLEANUP_IF_TRUE((-1 == pipe(stdout_pipe)), RETURN_CODE__PIPE_FAILED);

    child_pid = fork();
    CLEANUP_IF_TRUE((0 > child_pid), RETURN_CODE__FORK_FAILED);

    if(0 == child_pid)
    {
        close_ret = close(stdin_pipe[1]);
        EXIT_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);

        close_ret = close(stdout_pipe[0]);
        EXIT_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);

        dup2_ret = dup2(stdin_pipe[0], STDIN_FILENO);
        EXIT_IF_TRUE((0 > dup2_ret), RETURN_CODE__DUP2_FAILED);

        dup2_ret = dup2(stdout_pipe[1], STDOUT_FILENO);
        EXIT_IF_TRUE((0 > dup2_ret), RETURN_CODE__DUP2_FAILED);

        close_ret = close(stdin_pipe[0]);
        EXIT_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);

        close_ret = close(stdout_pipe[1]);
        EXIT_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);

        (void)execvp(argv[0], argv);

        exit(RETURN_CODE__EXEC_FAILED);
    }
    else
    {
        close_ret = close(stdin_pipe[0]);
        CLEANUP_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);

        close_ret = close(stdout_pipe[1]);
        CLEANUP_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);

        while(1)
        {
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(stdout_pipe[0], &readfds);

            max_fd = 0;
            if(stdout_pipe[0] > max_fd)
            {
                max_fd = stdout_pipe[0];
            }
            
            select_ret = select(max_fd + 1, &readfds, NULL, NULL, NULL);
            CLEANUP_IF_TRUE((0 > select_ret), RETURN_CODE__SELECT_FAILED);

            if(FD_ISSET(STDIN_FILENO, &readfds))
            {
                bytes_read = read(STDIN_FILENO, buffer, READ_BUFFER_SIZE);
                CLEANUP_IF_TRUE((0 > bytes_read), RETURN_CODE__READ_FAILED);

                if(0 == bytes_read)
                {
                    close_ret = close(stdin_pipe[1]);
                    CLEANUP_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);
                    stdin_pipe[1] = 0;
                    break;
                }
                else
                {
                    bytes_wrote = write(stdin_pipe[1], buffer, (size_t)bytes_read);
                    CLEANUP_IF_TRUE((0 > bytes_wrote), RETURN_CODE__WRITE_FAILED);
                }
            }

            if(FD_ISSET(stdout_pipe[0], &readfds))
            {
                bytes_read = read(stdout_pipe[0], buffer, READ_BUFFER_SIZE);
                CLEANUP_IF_TRUE((0 > bytes_read), RETURN_CODE__READ_FAILED);

                if(0 < bytes_read)
                {
                    bytes_wrote = write(STDOUT_FILENO, buffer, (size_t)bytes_read);
                    CLEANUP_IF_TRUE((0 > bytes_wrote), RETURN_CODE__WRITE_FAILED);
                }
                else
                {
                    break;
                }
            }
        }

        if(0 != stdin_pipe[1])
        {
            close_ret = close(stdin_pipe[1]);
            CLEANUP_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);
            stdin_pipe[1] = 0;
        }

        close_ret = close(stdout_pipe[0]);
        CLEANUP_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);
        stdout_pipe[0] = 0;

        CLEANUP_IF_TRUE((-1 == waitpid(child_pid, NULL, 0)), RETURN_CODE__WAITPID_FAILED);
    }

    rc = RETURN_CODE__SUCCESS;

cleanup:
    if(NULL != argv)
    {
        free(argv);
    }

    if(0 < stdin_pipe[0])
    {
        close_ret = close(stdin_pipe[0]);
        if((0 > close_ret) && (RETURN_CODE__SUCCESS == rc))
        {
            rc = RETURN_CODE__CLOSE_FAILED;
        }
    }

    if(0 < stdin_pipe[1])
    {
        close_ret = close(stdin_pipe[1]);
        if((0 > close_ret) && (RETURN_CODE__SUCCESS == rc))
        {
            rc = RETURN_CODE__CLOSE_FAILED;
        }
    }

    if(0 < stdout_pipe[0])
    {
        close_ret = close(stdout_pipe[0]);
        if((0 > close_ret) && (RETURN_CODE__SUCCESS == rc))
        {
            rc = RETURN_CODE__CLOSE_FAILED;
        }
    }

    if(0 < stdout_pipe[1])
    {
        close_ret = close(stdout_pipe[1]);
        if((0 > close_ret) && (RETURN_CODE__SUCCESS == rc))
        {
            rc = RETURN_CODE__CLOSE_FAILED;
        }
    }

    return rc;
}