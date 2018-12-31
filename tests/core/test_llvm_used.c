#include <stdio.h>
#include <emscripten.h>

extern "C" {
        __attribute__((annotate("hello attribute world")))
        EMSCRIPTEN_KEEPALIVE void foobar(int x) {
                printf("Worked! %d\n", x);
        }
}

int main() {
        emscripten_run_script("Module['_foobar'](10)");
        return 0;
}
