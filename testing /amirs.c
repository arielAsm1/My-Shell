#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

int main()
{
    int i = 0;
    char buffer_read[BUFFER_SIZE] = {0};

    printf("HELLO 1\n");

    int pipefd[2];

    if(pipe(pipefd) == -1)
    {
        printf("failed to pipe\n");

        return 1;
    }

    pid_t child_pid = fork();

    if(child_pid == 0)
    {
        close(pipefd[0]);

        dup2(pipefd[1], 1);

        char *argsv[] = {"ls", "-la", NULL};
        execv("/usr/bin/ls", argsv);
    }
    else
    {
        close(pipefd[1]);

        while(1)
        {
            ssize_t size = read(pipefd[0], buffer_read + i, 1);
            if(size == 0)
            {
                break;
            }
            i++;
        }

        printf("father: %s\n", buffer_read);

        waitpid(child_pid, NULL, 0);

        printf("father: my child died\n");
    }

    return 0;
}