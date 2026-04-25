/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/webaudio.h>

#include "emscripten_internal.h"
#include "threading_internal.h"

#ifdef __EMSCRIPTEN_PTHREADS__
void* init_pthread_struct(void *stackPlusTLSAddress, pid_t tid, size_t* stackPlusTLSSize);
#endif

// Simple wrapper function around the JS _emscripten_create_audio_worklet
// function that adds the _emscripten_get_next_tid() as arg0
void emscripten_start_wasm_audio_worklet_thread_async(EMSCRIPTEN_WEBAUDIO_T audioContext, void *stackLowestAddress, uint32_t stackSize, EmscriptenStartWebAudioWorkletCallback callback, void *userData2) {
  emscripten_wasm_worker_t wwID = _emscripten_get_next_tid();
  void* pthreadPtr = stackLowestAddress;
#ifdef __EMSCRIPTEN_PTHREADS__
  size_t stackSize_ = stackSize;
  stackLowestAddress = init_pthread_struct(stackLowestAddress, wwID, &stackSize_);
  stackSize = stackSize_;
#endif
  _emscripten_create_audio_worklet(wwID, audioContext, stackLowestAddress, stackSize, pthreadPtr, callback, userData2);
}
