#include <stdio.h>

int main() 
{
    int number;

    printf("enter a number: ");
    fflush(stdout);
    scanf("%d", &number);
    printf("you entered the number: %d\n", number);

    return 0;
}