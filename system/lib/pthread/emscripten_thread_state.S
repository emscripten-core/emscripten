#ifdef __wasm64__
#define PTR i64
#else
#define PTR i32
#endif

.section .globals,"",@

.globaltype thread_ptr, PTR
thread_ptr:

.globaltype is_main_thread, i32
is_main_thread:

.globaltype is_runtime_thread, i32
is_runtime_thread:

.globaltype supports_wait, i32
supports_wait:

#if WASM_WORKERS_ONLY
.globaltype done_init, i32
done_init:
#endif

.section .text,"",@

.globl __set_thread_state
__set_thread_state:
  .functype __set_thread_state (PTR, i32, i32, i32) -> ()
  local.get 0
  global.set thread_ptr
  local.get 1
  global.set is_main_thread
  local.get 2
  global.set is_runtime_thread
  local.get 3
  global.set supports_wait
  end_function

#if WASM_WORKERS_ONLY
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

.globl __get_tp
__get_tp:
  .functype __get_tp () -> (PTR)
#if WASM_WORKERS_ONLY
  call lazy_init_thread_state
#endif
  global.get thread_ptr
  end_function

# Semantically the same as testing "!ENVIRONMENT_IS_PTHREAD" in JS
.globl emscripten_is_main_runtime_thread
emscripten_is_main_runtime_thread:
  .functype emscripten_is_main_runtime_thread () -> (i32)
#if WASM_WORKERS_ONLY
  call lazy_init_thread_state
#endif
  global.get is_runtime_thread
  end_function

# Semantically the same as testing "!ENVIRONMENT_IS_WORKER" in JS
.globl emscripten_is_main_browser_thread
emscripten_is_main_browser_thread:
  .functype emscripten_is_main_browser_thread () -> (i32)
#if WASM_WORKERS_ONLY
  call lazy_init_thread_state
#endif
  global.get is_main_thread
  end_function

# Semantically the same as testing "!ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_AUDIO_WORKLET" in JS
.globl _emscripten_thread_supports_atomics_wait
_emscripten_thread_supports_atomics_wait:
  .functype _emscripten_thread_supports_atomics_wait () -> (i32)
#if WASM_WORKERS_ONLY
  call lazy_init_thread_state
#endif
  global.get supports_wait
  end_function
