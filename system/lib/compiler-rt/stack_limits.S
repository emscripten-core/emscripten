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
.globl __stack_pointer

.section .globals,"",@

# TODO(sbc): It would be nice if these we initialized directly
# using PTR.const rather than using the `emscripten_stack_init`
.globaltype __stack_end, PTR
__stack_end:
.globl __stack_end
.globaltype __stack_base, PTR
.globl __stack_base
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

# Add emscripten_stack_init to static ctors
.section .init_array.1,"",@
.p2align ALIGN
PTRSTORE emscripten_stack_init
