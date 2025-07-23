#if __EMSCRIPTEN__
#include <setjmp.h>
#include <signal.h>

int sigsetjmp(jmp_buf sigjmp_buf, int savesigs) {
    return setjmp(sigjmp_buf);
}
#endif
