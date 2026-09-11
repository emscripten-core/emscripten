/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * The JSPI hook exports. Each passes the current __stack_pointer and its event
 * to the C implementation in jspi.c and installs the stack pointer it returns,
 * which a C function could not do itself since its epilogue restores the stack
 * pointer it entered with. Under REENTRANT_JSPI the C side also leaves the stack limits
 * to install in the __jspi_pending_* globals; they are applied here, after the
 * C code has fully returned, since with STACK_OVERFLOW_CHECK=2 any C epilogue
 * running between the limits switch and the stack pointer switch would be
 * checked against the wrong limits. The token to return is left in the
 * __jspi_token global.
 */

#ifdef __wasm64__
#define PTR i64
#else
#define PTR i32
#endif

.globaltype __stack_pointer, PTR

.functype __jspi_hook_impl (PTR, i32, i64, i32) -> (PTR)

.section .globals,"",@

.globl __jspi_token
.globaltype __jspi_token, i64
__jspi_token:

#ifdef REENTRANT_JSPI
.functype emscripten_stack_set_limits (PTR, PTR) -> ()
.functype __jspi_set_stack_limits (PTR, PTR) -> ()

.globl __jspi_pending_base
.globaltype __jspi_pending_base, PTR
__jspi_pending_base:
.globl __jspi_pending_end
.globaltype __jspi_pending_end, PTR
__jspi_pending_end:
# Nonzero when STACK_OVERFLOW_CHECK=2 instrumented the module, whose bounds
# globals are only reachable through its export, via JS.
.globl __jspi_stack_check
.globaltype __jspi_stack_check, i32
__jspi_stack_check:
#endif

.section .text,"",@

# Common tail: takes the stack pointer the impl returned, installs it (and the
# pending limits) and returns the token. Called from frameless shims, so the
# switch is in effect when they return.
__jspi_finish:
  .functype __jspi_finish (PTR) -> (i64)
#ifdef REENTRANT_JSPI
  # Install the pending stack limits, if any.
  block
    global.get __jspi_pending_base
    PTR.eqz
    br_if 0
    global.get __jspi_pending_base
    global.get __jspi_pending_end
    call emscripten_stack_set_limits
    block
      global.get __jspi_stack_check
      i32.eqz
      br_if 0
      global.get __jspi_pending_base
      global.get __jspi_pending_end
      call __jspi_set_stack_limits
    end_block
    PTR.const 0
    global.set __jspi_pending_base
  end_block
#endif
  local.get 0
  global.set __stack_pointer
  global.get __jspi_token
  end_function

.globl __jspi_enter
__jspi_enter:
  .functype __jspi_enter () -> (i64)
  global.get __stack_pointer
  i32.const 0
  i64.const 0
  i32.const 0
  call __jspi_hook_impl
  call __jspi_finish
  end_function

.globl __jspi_exit
__jspi_exit:
  .functype __jspi_exit (i64, i32) -> ()
  global.get __stack_pointer
  i32.const 1
  local.get 0
  local.get 1
  call __jspi_hook_impl
  call __jspi_finish
  drop
  end_function

.globl __jspi_suspend
__jspi_suspend:
  .functype __jspi_suspend () -> (i64)
  global.get __stack_pointer
  i32.const 2
  i64.const 0
  i32.const 0
  call __jspi_hook_impl
  call __jspi_finish
  end_function

.globl __jspi_resume
__jspi_resume:
  .functype __jspi_resume (i64, i32) -> ()
  global.get __stack_pointer
  i32.const 3
  local.get 0
  local.get 1
  call __jspi_hook_impl
  call __jspi_finish
  drop
  end_function
