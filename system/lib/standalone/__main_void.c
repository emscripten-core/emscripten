/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <alloca.h>
#include <wasi/api.h>
#include <sysexits.h>

// Much of this code comes from:
// https://github.com/CraneStation/wasi-libc/blob/master/libc-bottom-half/crt/crt1.c
// Converted malloc() calls to alloca() to avoid including malloc in all programs.

int __main_argc_argv(int argc, char *argv[]);

// If the application's `main` does not uses argc/argv, then it will be defined
// (by llvm) with an __main_void alias, and therefore this function will not
// be included, and `_start` will call the application's `__main_void` directly.
//
// If the application's `main` does use argc/argv, then _start will call this
// function which which loads argv values and sends them to to the
// application's `main` which gets mangled to `__main_argc_argv` by llvm.
__attribute__((weak))
int __main_void(void) {
  /* Fill in the arguments from WASI syscalls. */
  size_t argc;
  char **argv = NULL;
  size_t argv_buf_size;
  __wasi_errno_t err;

  /* Get the sizes of the arrays we'll have to create to copy in the args. */
  err = __wasi_args_sizes_get(&argc, &argv_buf_size);
  if (err != __WASI_ERRNO_SUCCESS) {
    __wasi_proc_exit(EX_OSERR);
  }

  if (argc) {
    /* Allocate memory for the array of pointers, adding null terminator. */
    argv = alloca(sizeof(char *) * (argc + 1));
    /* Allocate memory for storing the argument chars. */
    uint8_t *argv_buf = alloca(sizeof(char) * argv_buf_size);
    /* Make sure the last pointer in the array is NULL. */
    argv[argc] = NULL;
    /* Fill the argument chars, and the argv array with pointers into those chars. */
    err = __wasi_args_get((uint8_t**)argv, argv_buf);
    if (err != __WASI_ERRNO_SUCCESS) {
      __wasi_proc_exit(EX_OSERR);
    }
  }

  return __main_argc_argv(argc, argv);
}
