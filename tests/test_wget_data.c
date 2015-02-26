#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include <emscripten.h>

int main()
{
    const char * file = "/test.txt";
    void* buffer;
    int num, error;

    printf("load %s\n", file);
    emscripten_wget_data(file, &buffer, &num, &error);
    assert(!error);
    assert(strstr(buffer, "emscripten") == buffer); 

    printf("load non-existing\n");
    emscripten_wget_data("doesnotexist", &buffer, &num, &error);
    assert(error);

    printf("ok!\n");
    int result = 1;
    REPORT_RESULT();
    return 0;
}
