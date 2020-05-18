.globl stackSave
.globl stackRestore
.globl stackAlloc

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
