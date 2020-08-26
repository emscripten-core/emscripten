/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <alloca.h>
#include <wasi/api.h>
#include <sysexits.h>

int __main_argc_argv(int argc, char *argv[]);

// If main() uses argc/argv, then no __original_main is emitted, and then
// this definition is used, which loads those values and sends them to main.
// If main() does not use argc/argv, then the compiler emits __original_main
// and this definition is not necessary, which avoids the wasi calls for
// getting the args.
// If there is no main() at all, we don't need this definition, but it will get
// linked in. However, _start checks for main()'s existence and only calls
// __original_main() if it does, so this will not be called, which allows
// LLVM LTO or the Binaryen optimizer to remove it.
__attribute__((weak))
int __main_void(void) {
  /* Fill in the arguments from WASI syscalls. */
  size_t argc;
  char **argv;
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
