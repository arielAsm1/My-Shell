#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define SIZE 1024

int main()
{
    int asmfdnigger[2] = {0};
    pid_t child_pid = 0;
    char * buffer = "AMIR SHAMEN";
    char buffer_read[SIZE] = {0};
    int i = 0;

    if(pipe(asmfdnigger) == -1)
    {
        printf("failed to pipe\n");

        return 1;
    }

    child_pid = fork();

    if(child_pid == 0)
    {
        close(asmfdnigger[0]);
        write(asmfdnigger[1], buffer, strlen(buffer));
        printf("child: BYE SHOOMAN TRANS\n");
        close(asmfdnigger[1]);
        sleep(10);
    }
    else
    {
        close(asmfdnigger[1]);
        while(1)
        {
            ssize_t size = read(asmfdnigger[0], buffer_read + i, 1);
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