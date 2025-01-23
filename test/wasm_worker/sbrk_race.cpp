/* Build with e.g.

  em++ -O3 -g3 -sWASM_WORKERS=1 -sASSERTIONS=1 -sINITIAL_MEMORY=64KB -sALLOW_MEMORY_GROWTH=1 -sSAFE_HEAP=2
    -sDISABLE_EXCEPTION_CATCHING=0 -sNO_FILESYSTEM -sEXIT_RUNTIME=0 -sMALLOC=emmalloc-verbose
    -sSTACK_SIZE=16KB -sMEMORY_GROWTH_LINEAR_STEP=64KB
    test/wasm_worker/sbrk_race.cpp -o a.html
*/

#include <emscripten/emscripten.h>
#include <emscripten/wasm_worker.h>

#include <stdlib.h>

void done()
{
  EM_ASM(document.body.innerHTML = 'Test finished without crashing');
}

void worker() {
  for(int i = 0; i < 60000; ++i)
    EM_ASM({}, malloc(i)); // Allocate 60k*(60k-1)/2 == 1.8 GB of memory
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, done);
}

int main() {
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024), &worker);
  emscripten_set_main_loop([](){}, 0, 1);
}