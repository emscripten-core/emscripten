#include <emscripten.h>

int main() {
    EM_ASM({
        Module['canvas'].style['cursor'] = 'pointer';
    });
}
