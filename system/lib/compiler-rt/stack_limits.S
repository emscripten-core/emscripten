.globl emscripten_stack_init
.globl emscripten_stack_set_limits
.globl emscripten_stack_get_free
.globl emscripten_stack_get_base
.globl emscripten_stack_get_end

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

.section .globals,"",@

# TODO(sbc): It would be nice if these we initialized directly
# using PTR.const rather than using the `emscripten_stack_init`
.globaltype __stack_end, PTR
__stack_end:
.globaltype __stack_base, PTR
__stack_base:

.section .text,"",@

emscripten_stack_get_base:
  .functype emscripten_stack_get_base () -> (PTR)
  global.get __stack_base
  end_function

emscripten_stack_get_end:
  .functype emscripten_stack_get_end () -> (PTR)
  global.get __stack_end
  end_function

emscripten_stack_init:
  # Initialize __stack_end and __stack_base.
  # This must be called before emscripten_stack_get_end,
  # emscripten_stack_get_base, or emscripten_stack_get_free are called
  .functype emscripten_stack_init () -> ()

  # What llvm calls __stack_high is the high address from where is grows
  # downwards.  We call this the stack base here in emscripten.
#ifdef __PIC__
  global.get __stack_high@GOT
#else
  PTR.const __stack_high
#endif
  global.set __stack_base

  # What llvm calls __stack_low is that end of the stack
#ifdef __PIC__
  global.get __stack_low@GOT
#else
  PTR.const __stack_low
#endif
  # Align up to 16 bytes
  PTR.const 0xf
  PTR.add
  PTR.const -0x10
  PTR.and
  global.set __stack_end

  end_function

emscripten_stack_set_limits:
  .functype emscripten_stack_set_limits (PTR, PTR) -> ()
  local.get 0
  global.set __stack_base
  local.get 1
  global.set __stack_end
  end_function

emscripten_stack_get_free:
  .functype emscripten_stack_get_free () -> (PTR)
  global.get __stack_pointer
  global.get __stack_end
  PTR.sub
  end_function

#ifdef __EMSCRIPTEN_WASM_WORKERS__
# TODO: Relocate the following to its own file wasm_worker.S, but need to figure out how to reference
# __stack_base and __stack_end globals from a separate file as externs in order for that to work.
.globl emscripten_wasm_worker_initialize
emscripten_wasm_worker_initialize:
  .functype emscripten_wasm_worker_initialize (PTR /*stackLowestAddress*/, i32 /*stackSize*/) -> ()

  // __stack_end = stackLowestAddress + (__builtin_wasm_tls_size() + 15) & -16;
  local.get 0
  .globaltype __tls_size, PTR, immutable
  global.get __tls_size
  PTR.add
  PTR.const 0xf
  PTR.add
  PTR.const -0x10
  PTR.and
  global.set __stack_end

  // __stack_base = stackLowestAddress + stackSize;
  local.get 0
  local.get 1
#ifdef __wasm64__
  i64.extend_i32_u
#endif
  PTR.add
  global.set __stack_base

// TODO: We'd like to do this here to avoid JS side calls to __set_stack_limits.
//       (or even better, we'd like to avoid duplicate versions of the stack variables)
// See https://github.com/emscripten-core/emscripten/issues/16496
//  global.get __stack_base
//  global.get __stack_end
//  .functype __set_stack_limits (PTR, PTR) -> ()
//  call __set_stack_limits

  // __wasm_init_tls(stackLowestAddress);
  local.get 0
  .functype __wasm_init_tls (PTR) -> ()
  call __wasm_init_tls

  // N.b. The function __wasm_init_tls above does not need
  // __stack_pointer initialized, and it destroys the value it was set to.
  // So we must initialize __stack_pointer only *after* completing __wasm_init_tls:

  // __stack_pointer = __stack_base;
  global.get __stack_base
  global.set __stack_pointer

  end_function
#endif

# Add emscripten_stack_init to static ctors
.section .init_array.1,"",@
.p2align ALIGN
PTRSTORE emscripten_stack_init
