#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <emscripten.h>

int main()
{
    const char * file = "/test.txt";
    emscripten_wget(file , file);
    FILE * f = fopen(file, "r");
    int result = 0;
    if(f) {
#define BUFSIZE 1024
        char buf[BUFSIZE];
        fgets(buf, BUFSIZE, f);
        buf[BUFSIZE-1] = 0;
        for(int i = 0; i < BUFSIZE; ++i)
            buf[i] = tolower(buf[i]);
        if(strstr(buf, "emscripten")) 
            result = 1;
        fclose(f);
    }
    REPORT_RESULT();
    return 0;
}
