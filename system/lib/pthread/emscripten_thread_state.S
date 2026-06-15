#ifdef __wasm64__
#define PTR i64
#else
#define PTR i32
#endif

.section .globals,"",@

#ifndef __EMSCRIPTEN_WASM_WORKERS__
.globaltype thread_ptr, PTR
thread_ptr:
#endif

.globaltype is_main_thread, i32
is_main_thread:

.globaltype is_runtime_thread, i32
is_runtime_thread:

.globaltype supports_wait, i32
supports_wait:

#if __EMSCRIPTEN_WASM_WORKERS__
.globaltype done_init, i32
done_init:
#endif

.section .text,"",@

.globl __set_thread_state
__set_thread_state:
  .functype __set_thread_state (PTR, i32, i32, i32) -> ()
#ifndef __EMSCRIPTEN_WASM_WORKERS__
  local.get 0
  global.set thread_ptr
#endif
  local.get 1
  global.set is_main_thread
  local.get 2
  global.set is_runtime_thread
  local.get 3
  global.set supports_wait
  end_function

#if __EMSCRIPTEN_WASM_WORKERS__
// With Wasm Workers we do lazy initializtion of the thread
// state so that only workers that call these APIs actually
// initializes their state.
.functype __do_set_thread_state () -> ()

lazy_init_thread_state:
  .functype lazy_init_thread_state () -> ()
  block
    global.get done_init
    br_if 0
    call __do_set_thread_state
    i32.const 1
    global.set done_init
  end_block
  end_function
#endif

#ifndef __EMSCRIPTEN_WASM_WORKERS__
.globl __get_tp
__get_tp:
  .functype __get_tp () -> (PTR)
  global.get thread_ptr
  end_function
#endif

# Semantically the same as testing "!ENVIRONMENT_IS_PTHREAD" in JS
.globl emscripten_is_main_runtime_thread
emscripten_is_main_runtime_thread:
  .functype emscripten_is_main_runtime_thread () -> (i32)
#if __EMSCRIPTEN_WASM_WORKERS__
  call lazy_init_thread_state
#endif
  global.get is_runtime_thread
  end_function

# Semantically the same as testing "!ENVIRONMENT_IS_WORKER" in JS
.globl emscripten_is_main_browser_thread
emscripten_is_main_browser_thread:
  .functype emscripten_is_main_browser_thread () -> (i32)
#if __EMSCRIPTEN_WASM_WORKERS__
  call lazy_init_thread_state
#endif
  global.get is_main_thread
  end_function

# Semantically the same as testing "!ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_AUDIO_WORKLET" in JS
.globl _emscripten_thread_supports_atomics_wait
_emscripten_thread_supports_atomics_wait:
  .functype _emscripten_thread_supports_atomics_wait () -> (i32)
#if __EMSCRIPTEN_WASM_WORKERS__
  call lazy_init_thread_state
#endif
  global.get supports_wait
  end_function
