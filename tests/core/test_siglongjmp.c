#include <setjmp.h>
#include <stdio.h>

int main() {
    sigjmp_buf env;
    volatile int flag = 0;
    if (sigsetjmp(env, 1) == 0) {
        // Cannot print anything here, because siglongjmp will
        // print a warning in between but only with ASSERTIONS enabled
        flag = 1;
        siglongjmp(env, 1);
    } else {
        if (flag) {
            puts("Success");
        }
    }
    return 0;
}

