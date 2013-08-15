#include <stdio.h>
#include <stdlib.h>

__attribute__((noinline)) void* test() {
    int x;
    scanf("%d", &x);
    return malloc(x);
}

int main(int argc, char** argv) {
    printf("wat%s!", argv[0]);
    test();
    return 0;
}
