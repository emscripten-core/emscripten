#include <stdio.h>
#include <emscripten.h>

int main(void) {
    printf("No yield:\n");
    emscripten_sleep(500);
    printf("With yield:\n");
    emscripten_sleep_with_yield(500);
    printf("Again no yield:\n");
    emscripten_sleep(500);
    printf("Done!\n");
    REPORT_RESULT(1);
    return 0;
}

