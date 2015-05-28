#include <stdio.h>
#include <sys/mman.h>
#include <emscripten.h>
#include <string.h>
#include <assert.h>

int main() {
    printf("*\n");
    FILE *f = fopen("data.dat", "r");
    char *m;
    m = (char*)mmap(NULL, 9000, PROT_READ, MAP_PRIVATE, fileno(f), 0);
    for (int i = 0; i < 20; i++) putchar(m[i]);
    munmap(m, 9000);
    printf("\n");
    m = (char*)mmap(NULL, 9000, PROT_READ, MAP_PRIVATE, fileno(f), 4096);
    for (int i = 0; i < 20; i++) putchar(m[i]);
    munmap(m, 9000);
    printf("\n*\n");

#ifdef REPORT_RESULT
    int result = 1;
    REPORT_RESULT();
#endif
    return 0;
}
