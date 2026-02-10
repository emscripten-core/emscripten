#ifdef __wasm64__
#define PTR i64
#else
#define PTR i32
#endif

.globl _emscripten_memset_bulkmem
_emscripten_memset_bulkmem:
  .functype _emscripten_memset_bulkmem (PTR, i32, PTR) -> (PTR)
  local.get 2
#ifdef __wasm64__
  i32.wrap_i64
#endif
  // memory.fill traps on OOB zero-length sets, but memset must not.
  if
   local.get 0
   local.get 1
   local.get 2
   memory.fill 0
  end_if
  local.get 0
  end_function

.section .custom_section.target_features,"",@
.int8 1
.int8 43
.int8 11
.ascii "bulk-memory"
