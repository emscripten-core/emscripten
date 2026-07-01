.extern __stack_pointer

#ifdef __wasm64__
#define PTR i64
#define ALIGN 3
#define PTRSTORE .int64
#else
#define PTR i32
#define ALIGN 2
#define PTRSTORE .int32
#endif

.globaltype __stack_pointer, PTR
.globaltype __tls_size, PTR, immutable
.functype emscripten_stack_set_limits (PTR /*base*/, PTR /*end*/) -> ()

.section .__self_id,"",@
.globaltype __self_id, i32
__self_id:

.section .text,"",@

.globl emscripten_wasm_worker_self_id
emscripten_wasm_worker_self_id:
  .functype emscripten_wasm_worker_self_id () -> (i32)
  global.get __self_id
  end_function

.globl _emscripten_wasm_worker_initialize
_emscripten_wasm_worker_initialize:
  .functype _emscripten_wasm_worker_initialize (i32 /*selfID*/, PTR /*stackLowestAddress*/, i32 /*stackSize*/) -> ()
  .local PTR, PTR

  local.get 0
  global.set __self_id

  // stack_end = stackLowestAddress + (__builtin_wasm_tls_size() + 15) & -16;
  local.get 1
  global.get __tls_size
  PTR.add
  PTR.const 0xf
  PTR.add
  PTR.const -0x10
  PTR.and
  local.set 3

  // stack_base = stackLowestAddress + stackSize;
  local.get 1
  local.get 2
#ifdef __wasm64__
  i64.extend_i32_u
#endif
  PTR.add
  local.set 4

  local.get 4
  local.get 3
  // emscripten_stack_set_limits(stack_base, stack_end);
  call emscripten_stack_set_limits

// TODO: We'd like to do this here to avoid JS side calls to __set_stack_limits.
//       (or even better, we'd like to avoid duplicate versions of the stack variables)
// See https://github.com/emscripten-core/emscripten/issues/16496
//  global.get __stack_base
//  global.get __stack_end
//  .functype __set_stack_limits (PTR, PTR) -> ()
//  call __set_stack_limits


  // __wasm_init_tls(stackLowestAddress);
  local.get 1
  .functype __wasm_init_tls (PTR) -> ()
  call __wasm_init_tls

  // N.b. The function __wasm_init_tls above does not need
  // __stack_pointer initialized, and it destroys the value it was set to.
  // So we must initialize __stack_pointer only *after* completing __wasm_init_tls:

  // __stack_pointer = stack_base;
  local.get 4
  global.set __stack_pointer

  end_function
