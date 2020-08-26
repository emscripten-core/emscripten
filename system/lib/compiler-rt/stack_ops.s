.globl stackSave
.globl stackRestore
.globl stackAlloc
.globl emscripten_stack_init
.globl emscripten_stack_get_current
.globl emscripten_stack_get_free

.globaltype __stack_pointer, i32

stackSave:
  .functype stackSave() -> (i32)
  global.get __stack_pointer
  end_function

stackRestore:
  .functype stackRestore(i32) -> ()
  local.get 0
  global.set __stack_pointer
  end_function

stackAlloc:
  .functype stackAlloc(i32) -> (i32)
  .local i32, i32
  global.get __stack_pointer
  # Get arg 0 -> number of bytes to allocate
  local.get 0
  # Stack grows down.  Subtract arg0 from __stack_pointer
  i32.sub
  # Align result by anding with ~15
  i32.const 0xfffffff0
  i32.and
  local.tee 1
  global.set __stack_pointer
  local.get 1
  end_function

emscripten_stack_get_current:
  .functype emscripten_stack_get_current () -> (i32)
  global.get __stack_pointer
  end_function

.functype emscripten_stack_get_end () -> (i32)
.functype emscripten_stack_get_base () -> (i32)
.globaltype __stack_end, i32
__stack_end:

emscripten_stack_init:
  # initialize __stack_end such that future calls to emscripten_stack_get_free
  # use the correct value.
  .functype emscripten_stack_init () -> ()
  call emscripten_stack_get_end
  global.set __stack_end
  end_function

emscripten_stack_get_free:
  .functype emscripten_stack_get_free () -> (i32)
  global.get __stack_pointer
  global.get __stack_end
  i32.sub
  end_function
