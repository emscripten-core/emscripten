/*
 * Used in wasm2js mode to grow the wasm memory.
 */
#include <sys/types.h>

ssize_t wasm2js_memory_grow(ssize_t ammount) {
  return __builtin_wasm_memory_grow(0, ammount);
}
