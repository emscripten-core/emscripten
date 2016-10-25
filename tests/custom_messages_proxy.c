#include <emscripten.h>

int main()
{
    EM_ASM({
        customMessageData += '[main]';
        postCustomMessage({ op: 'fromMain' });
    });

    emscripten_exit_with_live_runtime();
    return 0;
}
