#include <stdio.h>

int main()
{
    printf("hello!");
    fflush(stdout);
    fprintf(stderr, "hello from stderr too!");
    fflush(stderr);
}
