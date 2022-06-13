#include <emscripten/threading.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    // https://github.com/WebAssembly/binaryen/issues/2170
    unsigned char obj = 10;
    emscripten_atomic_cas_u8(&obj, 1, 2);
    printf("%d\n", (int)obj);
    emscripten_atomic_cas_u8(&obj, 10, 2);
    printf("%d\n", (int)obj);
}
