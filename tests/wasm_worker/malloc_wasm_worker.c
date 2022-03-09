#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Test emscripten_malloc_wasm_worker() and emscripten_current_thread_is_wasm_worker() functions

EM_JS(void, console_log, (char* str), {
  console.log(UTF8ToString(str));
});

void worker_main()
{
  console_log("Hello from wasm worker!");
  assert(emscripten_current_thread_is_wasm_worker());
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

int main()
{
	assert(!emscripten_current_thread_is_wasm_worker());
	emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stack size: */1024);
	emscripten_wasm_worker_post_function_v(worker, worker_main);
}
