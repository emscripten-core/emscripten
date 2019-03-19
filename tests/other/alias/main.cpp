/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <dlfcn.h>
#include <iostream>
#include <stdexcept>
#include <emscripten.h>

using namespace std;

int main()
{   
    EM_ASM({
        FS.mkdir('/working');
        FS.mount(NODEFS, { root: '.' }, '/working');
    });
    void *handle = dlopen("/working/side.wasm", RTLD_NOW);
        if (!handle) {
        printf("dlopen failed:");
    }

    typedef void (*sideModule_fn)(void);
    sideModule_fn exportedfn = (sideModule_fn)dlsym(handle, "_Z19destructorWithAliasv");
    const char *err = dlerror();
    if (err) {
        printf("ERROR: dlsym failed: for sideModule_fn");
        abort();
    }
    exportedfn();
    dlclose(handle);
    return 0;
}