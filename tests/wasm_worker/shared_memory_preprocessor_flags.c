// This file should be compiled with -sSHARED_MEMORY=1

#ifndef __EMSCRIPTEN_SHARED_MEMORY__
#error __EMSCRIPTEN_SHARED_MEMORY__ should be defined when building with -sSHARED_MEMORY=1!
#endif

#ifdef __EMSCRIPTEN_WASM_WORKERS__
#error __EMSCRIPTEN_WASM_WORKERS__ should not defined when building with -sSHARED_MEMORY=1!
#endif

#ifdef __EMSCRIPTEN_PTHREADS__
#error __EMSCRIPTEN_PTHREADS__ should not be defined when building with -sSHARED_MEMORY=1!
#endif
