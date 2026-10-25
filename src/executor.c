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
#include "shell.h"

#define READ_BUFFER_SIZE (1024)

static rc_t resolve_path(char * cmd, char ** out_path)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char * path_env = NULL;
    char * path_copy = NULL;
    char * token = NULL;
    char * full_path = NULL;
    char * str = NULL;
    char * slash = NULL;
    size_t cmd_len = 0;
    size_t token_len = 0;
    size_t path_env_len = 0;
    int access_ret = -1;

    CLEANUP_IF_TRUE(((NULL == cmd) || (NULL == out_path)), RETURN_CODE__NULL_PARAM);
    
    *out_path = NULL;
    cmd_len = strlen(cmd);
    slash = strchr(cmd, '/');

    if (NULL != slash)
    {
        *out_path = (char *)malloc((cmd_len + 1) * sizeof(char));
        CLEANUP_IF_TRUE((NULL == *out_path), RETURN_CODE__MALLOC_FAILED);
        
        str = strncpy(*out_path, cmd, cmd_len + 1);
        CLEANUP_IF_TRUE((NULL == str), RETURN_CODE__STRNCPY_FAILED);
        
        rc = RETURN_CODE__SUCCESS;
        goto cleanup;
    }

    rc = SHELL__get_env("PATH", &path_env);
    CLEANUP_NON_SUCCESS(rc);

    if (NULL == path_env)
    {
        *out_path = (char *)malloc((cmd_len + 1) * sizeof(char));
        CLEANUP_IF_TRUE((NULL == *out_path), RETURN_CODE__MALLOC_FAILED);
        
        str = strncpy(*out_path, cmd, cmd_len + 1);
        CLEANUP_IF_TRUE((NULL == str), RETURN_CODE__STRNCPY_FAILED);
        
        rc = RETURN_CODE__SUCCESS;
        goto cleanup;
    }

    path_env_len = strlen(path_env);
    path_copy = (char *)malloc((path_env_len + 1) * sizeof(char));
    CLEANUP_IF_TRUE((NULL == path_copy), RETURN_CODE__MALLOC_FAILED);
    
    str = strncpy(path_copy, path_env, path_env_len + 1);
    CLEANUP_IF_TRUE((NULL == str), RETURN_CODE__STRNCPY_FAILED);

    token = strtok(path_copy, ":");

    while (NULL != token)
    {
        token_len = strlen(token);
        full_path = (char *)malloc((token_len + cmd_len + 2) * sizeof(char));
        CLEANUP_IF_TRUE((NULL == full_path), RETURN_CODE__MALLOC_FAILED);

        str = strncpy(full_path, token, token_len + 1);
        CLEANUP_IF_TRUE((NULL == str), RETURN_CODE__STRNCPY_FAILED);
        
        str = strncat(full_path, "/", 2);
        str = strncat(full_path, cmd, cmd_len + 1);

        access_ret = access(full_path, X_OK);
        if (0 == access_ret)
        {
            *out_path = full_path;
            full_path = NULL;
            rc = RETURN_CODE__SUCCESS;
            goto cleanup;
        }

        free(full_path);
        full_path = NULL;
        
        token = strtok(NULL, ":");
    }

    *out_path = (char *)malloc((cmd_len + 1) * sizeof(char));
    CLEANUP_IF_TRUE((NULL == *out_path), RETURN_CODE__MALLOC_FAILED);
    
    str = strncpy(*out_path, cmd, cmd_len + 1);
    CLEANUP_IF_TRUE((NULL == str), RETURN_CODE__STRNCPY_FAILED);

    rc = RETURN_CODE__SUCCESS;

cleanup:
    if (NULL != path_copy)
    {
        free(path_copy);
    }
    
    if (NULL != full_path)
    {
        free(full_path);
    }

    return rc;
}

rc_t EXECUTOR__execute_command(char ** tokens, size_t tokens_size)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char ** argv = NULL;
    char ** envp = NULL;
    char * cmd_path = NULL;

    int stdin_pipe[2] = { 0 };
    int stdout_pipe[2] = { 0 };
    int stderr_pipe[2] = { 0 };

    pid_t child_pid = 0;
    char buffer[READ_BUFFER_SIZE] = { 0 };
    
    fd_set readfds;
    int max_fd = 0;
    int select_ret = 0;
    ssize_t bytes_read = 0;
    ssize_t bytes_wrote = 0;
    size_t i = 0;
    int dup2_ret = 0;
    int close_ret = 0;
    int waitpid_ret = 0;

    CLEANUP_IF_TRUE(((NULL == tokens) || (0 == tokens_size)), RETURN_CODE__NULL_PARAM);
    
    argv = (char **)malloc((tokens_size + 1) * sizeof(char *));
    CLEANUP_IF_TRUE((NULL == argv), RETURN_CODE__MALLOC_FAILED);

    for (i = 0; i < tokens_size; i++)
    {
        argv[i] = tokens[i];
    }
    
    argv[tokens_size] = NULL;

    CLEANUP_IF_TRUE((-1 == pipe(stdin_pipe)), RETURN_CODE__PIPE_FAILED);
    CLEANUP_IF_TRUE((-1 == pipe(stdout_pipe)), RETURN_CODE__PIPE_FAILED);
    CLEANUP_IF_TRUE((-1 == pipe(stderr_pipe)), RETURN_CODE__PIPE_FAILED);

    child_pid = fork();
    CLEANUP_IF_TRUE((0 > child_pid), RETURN_CODE__FORK_FAILED);

    if (0 == child_pid)
    {
        close_ret = close(stdin_pipe[1]);
        EXIT_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);

        close_ret = close(stdout_pipe[0]);
        EXIT_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);
        
        close_ret = close(stderr_pipe[0]);
        EXIT_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);

        dup2_ret = dup2(stdin_pipe[0], STDIN_FILENO);
        EXIT_IF_TRUE((0 > dup2_ret), RETURN_CODE__DUP2_FAILED);

        dup2_ret = dup2(stdout_pipe[1], STDOUT_FILENO);
        EXIT_IF_TRUE((0 > dup2_ret), RETURN_CODE__DUP2_FAILED);
        
        dup2_ret = dup2(stderr_pipe[1], STDERR_FILENO);
        EXIT_IF_TRUE((0 > dup2_ret), RETURN_CODE__DUP2_FAILED);

        close_ret = close(stdin_pipe[0]);
        EXIT_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);

        close_ret = close(stdout_pipe[1]);
        EXIT_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);
        
        close_ret = close(stderr_pipe[1]);
        EXIT_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);

        rc = resolve_path(argv[0], &cmd_path);
        EXIT_IF_TRUE((RETURN_CODE__SUCCESS != rc), RETURN_CODE__EXEC_FAILED);
        
        rc = SHELL__build_envp(&envp);
        EXIT_IF_TRUE((RETURN_CODE__SUCCESS != rc), RETURN_CODE__EXEC_FAILED);

        (void)execve(cmd_path, argv, envp);

        exit(RETURN_CODE__EXEC_FAILED);
    }
    else
    {
        close_ret = close(stdin_pipe[0]);
        CLEANUP_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);
        stdin_pipe[0] = 0; 

        close_ret = close(stdout_pipe[1]);
        CLEANUP_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);
        stdout_pipe[1] = 0; 
        
        close_ret = close(stderr_pipe[1]);
        CLEANUP_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);
        stderr_pipe[1] = 0;

        while (1)
        {
            FD_ZERO(&readfds);
            
            if (0 != stdin_pipe[1])
            {
                FD_SET(STDIN_FILENO, &readfds);
            }
            
            if (0 != stdout_pipe[0])
            {
                FD_SET(stdout_pipe[0], &readfds);
            }
            
            if (0 != stderr_pipe[0])
            {
                FD_SET(stderr_pipe[0], &readfds);
            }

            max_fd = STDIN_FILENO;
            
            if (stdout_pipe[0] > max_fd)
            {
                max_fd = stdout_pipe[0];
            }
            
            if (stderr_pipe[0] > max_fd)
            {
                max_fd = stderr_pipe[0];
            }
            
            if ((0 == stdout_pipe[0]) && (0 == stderr_pipe[0]))
            {
                break;
            }

            select_ret = select(max_fd + 1, &readfds, NULL, NULL, NULL);
            CLEANUP_IF_TRUE((0 > select_ret), RETURN_CODE__SELECT_FAILED);

            if (FD_ISSET(STDIN_FILENO, &readfds) && (0 != stdin_pipe[1]))
            {
                bytes_read = read(STDIN_FILENO, buffer, READ_BUFFER_SIZE);
                CLEANUP_IF_TRUE((0 > bytes_read), RETURN_CODE__READ_FAILED);

                if (0 == bytes_read)
                {
                    close_ret = close(stdin_pipe[1]);
                    CLEANUP_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);
                    stdin_pipe[1] = 0;
                }
                else
                {
                    bytes_wrote = write(stdin_pipe[1], buffer, (size_t)bytes_read);
                    CLEANUP_IF_TRUE((0 > bytes_wrote), RETURN_CODE__WRITE_FAILED);
                }
            }

            if ((0 != stdout_pipe[0]) && FD_ISSET(stdout_pipe[0], &readfds))
            {
                bytes_read = read(stdout_pipe[0], buffer, READ_BUFFER_SIZE);
                CLEANUP_IF_TRUE((0 > bytes_read), RETURN_CODE__READ_FAILED);

                if (0 < bytes_read)
                {
                    bytes_wrote = write(STDOUT_FILENO, buffer, (size_t)bytes_read);
                    CLEANUP_IF_TRUE((0 > bytes_wrote), RETURN_CODE__WRITE_FAILED);
                }
                else
                {
                    close_ret = close(stdout_pipe[0]);
                    stdout_pipe[0] = 0;
                }
            }
            
            if ((0 != stderr_pipe[0]) && FD_ISSET(stderr_pipe[0], &readfds))
            {
                bytes_read = read(stderr_pipe[0], buffer, READ_BUFFER_SIZE);
                CLEANUP_IF_TRUE((0 > bytes_read), RETURN_CODE__READ_FAILED);

                if (0 < bytes_read)
                {
                    bytes_wrote = write(STDERR_FILENO, buffer, (size_t)bytes_read);
                    CLEANUP_IF_TRUE((0 > bytes_wrote), RETURN_CODE__WRITE_FAILED);
                }
                else
                {
                    close_ret = close(stderr_pipe[0]);
                    stderr_pipe[0] = 0;
                }
            }
        }

        if (0 != stdin_pipe[1])
        {
            close_ret = close(stdin_pipe[1]);
            CLEANUP_IF_TRUE((0 > close_ret), RETURN_CODE__CLOSE_FAILED);
            stdin_pipe[1] = 0;
        }

        waitpid_ret = waitpid(child_pid, NULL, 0);
        CLEANUP_IF_TRUE((-1 == waitpid_ret), RETURN_CODE__WAITPID_FAILED);
    }

    rc = RETURN_CODE__SUCCESS;

cleanup:
    if (NULL != argv)
    {
        free(argv);
    }
    
    if (NULL != cmd_path)
    {
        free(cmd_path);
    }
    
    if (NULL != envp) 
    {
        for (i = 0; NULL != envp[i]; i++)
        {
            free(envp[i]);
        }
        free(envp);
    }

    if (0 < stdin_pipe[0])
    {
        close_ret = close(stdin_pipe[0]);
        if ((0 > close_ret) && (RETURN_CODE__SUCCESS == rc))
        {
            rc = RETURN_CODE__CLOSE_FAILED;
        }
    }
    
    if (0 < stdin_pipe[1])
    {
        close_ret = close(stdin_pipe[1]);
        if ((0 > close_ret) && (RETURN_CODE__SUCCESS == rc))
        {
            rc = RETURN_CODE__CLOSE_FAILED;
        }
    }
    
    if (0 < stdout_pipe[0])
    {
        close_ret = close(stdout_pipe[0]);
        if ((0 > close_ret) && (RETURN_CODE__SUCCESS == rc))
        {
            rc = RETURN_CODE__CLOSE_FAILED;
        }
    }
    
    if (0 < stdout_pipe[1])
    {
        close_ret = close(stdout_pipe[1]);
        if ((0 > close_ret) && (RETURN_CODE__SUCCESS == rc))
        {
            rc = RETURN_CODE__CLOSE_FAILED;
        }
    }
    
    if (0 < stderr_pipe[0])
    {
        close_ret = close(stderr_pipe[0]);
        if ((0 > close_ret) && (RETURN_CODE__SUCCESS == rc))
        {
            rc = RETURN_CODE__CLOSE_FAILED;
        }
    }
    
    if (0 < stderr_pipe[1])
    {
        close_ret = close(stderr_pipe[1]);
        if ((0 > close_ret) && (RETURN_CODE__SUCCESS == rc))
        {
            rc = RETURN_CODE__CLOSE_FAILED;
        }
    }

    return rc;
}