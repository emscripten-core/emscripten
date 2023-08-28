#ifdef __wasm64__
#define PTR i64
#else
#define PTR i32
#endif

.globl emscripten_memcpy_bulkmem
emscripten_memcpy_bulkmem:
  .functype emscripten_memcpy_bulkmem (PTR, PTR, PTR) -> (PTR)
  local.get 0
  local.get 1
  local.get 2
  memory.copy 0, 0
  local.get 0
  end_function
