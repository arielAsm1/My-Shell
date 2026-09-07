#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    int newfd = open("nigger2.txt", O_WRONLY | O_CREAT);

    printf("%d\n", newfd);
     
    if(dup2(newfd, 1) == -1)
    {
        printf("failed to dup2\n");

        return 1;
    }

    printf("AMIR SHAMEN MEOED\n");
    
    close(newfd);

    return 0;
}