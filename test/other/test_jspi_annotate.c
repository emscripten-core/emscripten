#include <emscripten.h>

extern int async_import();

EMSCRIPTEN_KEEPALIVE EM_JSPI int async_export() {
  return async_import();
}

EM_JSPI int async_internal() {
  return async_import();
}

EMSCRIPTEN_KEEPALIVE void* get_func_ptr() {
  return &async_internal;
}
