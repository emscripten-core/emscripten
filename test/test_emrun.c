/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

#ifdef __EMSCRIPTEN_WASM_WORKERS__
#include <emscripten/console.h>
#include <emscripten/wasm_worker.h>

void finish(void) {
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(100);
}

void nested_worker_finished(void) {
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT,
                                         finish);
}

void nested_worker_main(void) {
  emscripten_out("Hello from nested Wasm worker!");
  emscripten_err("Error from nested Wasm worker!");
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT,
                                         nested_worker_finished);
}

void worker_main(void) {
  emscripten_out("Hello from Wasm worker!");
  emscripten_err("Error from Wasm worker!");
  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(4096);
  assert(worker);
  emscripten_wasm_worker_post_function_v(worker, nested_worker_main);
}
#endif

int main(int argc, char **argv) {
  printf("argc: %d\n", argc);
  for (int i = 0; i < argc; ++i) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }
  assert(argc > 1);

  // Dump a file to local filesystem with emrun.
  EM_ASM(emrun_file_dump("test.dat", HEAPU8.subarray(0, 128)););
  EM_ASM(emrun_file_dump("heap.dat", HEAPU8));
  EM_ASM(emrun_file_dump("nested/with space.dat", HEAPU8.subarray(128, 256)););

  printf("Hello, world!\n");
  fprintf(stderr, "hello, error stream!\n");

  printf("Testing ASCII characters: !\"$%%&'()*+,-./:;<=>?@[\\]^_`{|}~\n");
  printf("Testing char sequences: %%20%%21 &auml;\n");

#ifdef __EMSCRIPTEN_WASM_WORKERS__
  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(4096);
  assert(worker);
  emscripten_wasm_worker_post_function_v(worker, worker_main);
  emscripten_exit_with_live_runtime();
#else
  exit(100);
#endif
}
