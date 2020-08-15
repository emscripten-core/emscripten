.globl emscripten_stack_init
.globl emscripten_stack_set_limits
.globl emscripten_stack_get_free
.globl emscripten_stack_get_base
.globl emscripten_stack_get_end

.globaltype __stack_pointer, i32

# TODO(sbc): It would be nice if these we initialized directly
# using i32.const rather than using the `emscripten_stack_init`
.globaltype __stack_end, i32
__stack_end:
.globaltype __stack_base, i32
__stack_base:

emscripten_stack_get_base:
  .functype emscripten_stack_get_base () -> (i32)
  global.get __stack_base
  end_function

emscripten_stack_get_end:
  .functype emscripten_stack_get_end () -> (i32)
  global.get __stack_end
  end_function

emscripten_stack_init:
  # Initialize __stack_end and __stack_base.
  # This must be called before emscripten_stack_get_end,
  # emscripten_stack_get_base, or emscripten_stack_get_free are called
  .functype emscripten_stack_init () -> ()

  # The heap base is where the stack grown down from.
#ifdef __PIC__
  global.get __heap_base@GOT
#else
  i32.const __heap_base
#endif
  global.set __stack_base

  # The end of stack data is the limit of the stack growth
#ifdef __PIC__
  global.get __data_end@GOT
#else
  i32.const __data_end
#endif
  # Align up to 16 bytes
  i32.const 0xf
  i32.add
  i32.const -0x10
  i32.and
  global.set __stack_end

  end_function

emscripten_stack_set_limits:
  .functype emscripten_stack_set_limits (i32, i32) -> ()
  local.get 0
  global.set __stack_base
  local.get 1
  global.set __stack_end
  end_function

emscripten_stack_get_free:
  .functype emscripten_stack_get_free () -> (i32)
  global.get __stack_pointer
  global.get __stack_end
  i32.sub
  end_function

# Add emscripten_stack_init to static ctors
.section .init_array.1,"",@
.p2align 2
.int32 emscripten_stack_init
