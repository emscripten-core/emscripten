#ifdef __wasm64__
#define PTR i64
#else
#define PTR i32
#endif

.globl emscripten_memset_big
emscripten_memset_big:
  .functype emscripten_memset_big (PTR, i32, PTR) -> ()
  local.get 0
  local.get 1
  local.get 2
  memory.fill 0
  end_function
