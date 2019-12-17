#include "libc.h"

#ifdef __EMSCRIPTEN__
#include <stdlib.h>
#include <wasi/api.h>
#endif

char **__environ = 0;
weak_alias(__environ, ___environ);
weak_alias(__environ, _environ);
weak_alias(__environ, environ);

#ifdef __EMSCRIPTEN__
__attribute__((constructor(100))) // construct this before user code
void __emscripten_environ_constructor(void) {
    size_t environ_count;
    size_t environ_buf_size;
    __wasi_errno_t err = __wasi_environ_sizes_get(&environ_count,
                                                  &environ_buf_size);
    if (err != __WASI_ERRNO_SUCCESS) {
        return;
    }

    __environ = malloc(sizeof(char *) * (environ_count + 1));
    if (__environ == 0) {
        return;
    }
    char *environ_buf = malloc(sizeof(char) * environ_buf_size);
    if (__environ == 0 || environ_buf == 0) {
        __environ = 0;
        return;
    }

    // Ensure null termination.
    __environ[environ_count] = 0;

    err = __wasi_environ_get((uint8_t**)__environ, environ_buf);
    if (err != __WASI_ERRNO_SUCCESS) {
        __environ = 0;
    }
}
#endif
