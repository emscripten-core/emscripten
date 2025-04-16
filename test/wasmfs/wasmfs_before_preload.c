/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/wasmfs.h>
#include <stdio.h>

static backend_t my_js_file_backend;

void wasmfs_before_preload(void) {
  emscripten_console_log("before_preload");

  // Make a JS file backend during startup, by implementing this hook function,
  // so that the preloaded files end up stored there (instead of in the default
  // backend).
  my_js_file_backend = wasmfs_create_js_file_backend();
  assert(my_js_file_backend);
  int result =
    wasmfs_create_directory("/js_backend_files", 0777, my_js_file_backend);
  assert(result == 0);
}

int main() {
  emscripten_console_log("main");

  // The root uses the default backend.
  backend_t default_backend = wasmfs_get_backend_by_path("/");
  assert(default_backend != my_js_file_backend);

  // The preloaded file uses the js backend we created during startup.
  backend_t file_backend =
    wasmfs_get_backend_by_path("/js_backend_files/file.dat");
  assert(file_backend == my_js_file_backend);

  emscripten_console_log("success");
}
