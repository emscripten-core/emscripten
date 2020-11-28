.globaltype thread_id, i32
thread_id:

.globaltype is_main_thread, i32
is_main_thread:

.globaltype is_runtime_thread, i32
is_runtime_thread:

.globl __pthread_self
__pthread_self:
  .functype __pthread_self () -> (i32)
  global.get thread_id
  end_function

.globl _emscripten_thread_init
_emscripten_thread_init:
  .functype _emscripten_thread_init (i32, i32, i32) -> ()
  local.get 0
  global.set thread_id
  local.get 1
  global.set is_main_thread
  local.get 2
  global.set is_runtime_thread
  end_function

# Semantically the same as testing "!ENVIRONMENT_IS_PTHREAD" in JS
.globl emscripten_is_main_runtime_thread
emscripten_is_main_runtime_thread:
  .functype emscripten_is_main_runtime_thread () -> (i32)
  global.get is_runtime_thread
  end_function

# Semantically the same as testing "!ENVIRONMENT_IS_WORKER" in JS
.globl emscripten_is_main_browser_thread
emscripten_is_main_browser_thread:
  .functype emscripten_is_main_browser_thread () -> (i32)
  global.get is_main_thread
  end_function
