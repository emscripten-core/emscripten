/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// If the user uses argc/argv, then no __original_main is emitted, and then
// this file gets linked in, which loads those values and sends them to main.
// If the user does not use argc/argv, then the compiler emits __original_main
// and this file is not necessary, which avoids some wasi calls.

#include <alloca.h>
#include <stdlib.h>
#include <sysexits.h>
#include <wasi/wasi.h>
#include <stdio.h>

// TODO(sbc): We shouldn't even link this file if there is no main:
// https://github.com/emscripten-core/emscripten/issues/9640
extern int main(int argc, char** argv) __attribute__((weak));

// Avoid using stack allocation for argument-passed values here, as the
// stack allocation for them can't be eliminated by Binaryen later, so if we
// have no main we end up with a stack push and pop for no reason. Also, the
// stack allocation here would last for the entire program anyhow, so it's
// effectively static.
static size_t argc;
static size_t argv_buf_size;

int __original_main(void) {
  /* Fill in the arguments from WASI syscalls. */
  char **argv;
  __wasi_errno_t err;

  /* Get the sizes of the arrays we'll have to create to copy in the args. */
  err = __wasi_args_sizes_get(&argc, &argv_buf_size);
  if (err != __WASI_ESUCCESS) {
    __wasi_proc_exit(EX_OSERR);
  }

  if (argc) {
    /* Allocate memory for the array of pointers, adding null terminator. */
    argv = alloca(sizeof(char *) * (argc + 1));
    /* Allocate memory for storing the argument chars. */
    char *argv_buf = alloca(sizeof(char) * argv_buf_size);
    /* Make sure the last pointer in the array is NULL. */
    argv[argc] = NULL;
    /* Fill the argument chars, and the argv array with pointers into those chars. */
    err = __wasi_args_get(argv, argv_buf);
    if (err != __WASI_ESUCCESS) {
      __wasi_proc_exit(EX_OSERR);
    }
  }

  return main(argc, argv);
}
