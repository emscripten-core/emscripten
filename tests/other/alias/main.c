/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <dlfcn.h>
#include <emscripten.h>

int bar(void) {
    return 42;
}

int foo(void) __attribute__((alias("bar")));

int main()
{
    EM_ASM({
        FS.mkdir('/working');
        FS.mount(NODEFS, { root: '.' }, '/working');
    });
    void *handle = dlopen("/working/side.wasm", RTLD_NOW);
    if (!handle) {
        printf("dlopen failed: %s", dlerror());
        return 1;
    }

    typedef int (*func_type)(void);
    func_type exportedfn = (func_type)dlsym(handle, "callAlias");
    if (!exportedfn) {
        const char *err = dlerror();
        printf("ERROR: dlsym failed: for callAlias: %s", err);
        return 1;
    }
    if (exportedfn() != 42)
      return 1;
    dlclose(handle);
    printf("success\n");
    return 0;
}
