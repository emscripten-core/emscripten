// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <atomic>
#include <mutex>
#include <emscripten/emscripten.h>

// This test tests the futex implementation for audio worklets which is 
// very similar to the main thread futex spin loop implementation since
// audio worklets don't support Atomics.Wait/Notify to keep them as 
// non-blocking as possible

pthread_t workletThreadId = 0; // This is set on the main thread when the pthread is created
std::atomic<pthread_t> pthreadIdFromWorklet(0); // This is set from the audio thread when the AudioWorkletNode is constructed
std::mutex testMutex; // A test mutex to test the worklet futex implementation

// Sends the current pthread id from the audio worklet node, signalling test success
// This happens immediately in the worklet node constructor
EMSCRIPTEN_KEEPALIVE extern "C" void signalTestSuccess() {
  pthread_t threadId = pthread_self();
  pthreadIdFromWorklet.store(threadId);

  printf("Audio thread time: %f\n", emscripten_get_now());

  // Put the worklet to sleep until the main thread sees the above store.
  // Blocking like this is *really bad* and should never be done in an audio worklet, but
  // is here purely to test the worklet futex implementation which is sometimes needed.
  testMutex.lock(); 
  testMutex.unlock();
}

// The main loop keeps going until 'pthreadIdFromWorklet' is sent from the AudioWorkletNode
EM_BOOL mainLoop(double time, void* userdata) {
  pthread_t threadId = pthreadIdFromWorklet.load();
  if(threadId != 0) {
    printf("Main thread time: %f\n", emscripten_get_now());

    if(threadId == workletThreadId) {
      printf("Sucesss! Got pthread id: %lu, expected %lu\n", threadId, workletThreadId);
      #ifdef REPORT_RESULT
        REPORT_RESULT(1);
      #endif
    } else {
      printf("Failed! Got wrong pthread id: %lu, expected %lu\n", threadId, workletThreadId);
      #ifdef REPORT_RESULT
        REPORT_RESULT(-100);
      #endif
    }

    testMutex.unlock();
    return EM_FALSE;
  }

  return EM_TRUE;
}

// Initializes the audio context and the pthread it it's AudioWorkletGlobalScope
EM_JS(uint32_t, init, (pthread_t* pthreadPtr), {
  // Create the context
  Module.audioCtx = new AudioContext();

  // Initialize the pthread shared by all AudioWorkletNodes in this context
  PThread.initAudioWorkletPThread(Module.audioCtx, pthreadPtr).then(function() {
    out("PThread context initialized!")
  }, function(err) {
    out("PThread context initialization failed: " + [err, err.stack]);
  });

  // Creates an AudioWorkletNode and connects it to the output once it's created
  PThread.createAudioWorkletNode(
    Module.audioCtx,
    'test-processor', 
    {
      numberOfInputs: 0,
      numberOfOutputs : 1,
      outputChannelCount : [2]
    }
  ).then(function(workletNode) {
    // Connect the worklet to the audio context output
    out("Audio worklet created!");
    workletNode.connect(Module.audioCtx.destination);
  }, function(err) {
    out("Audio worklet creation failed: " + [err, err.stack]);
  });
});

int main()
{
  emscripten_request_animation_frame_loop(mainLoop, nullptr);
  testMutex.lock(); // Lock the mutex so that the worklet thread can wait on it (see comment in 'signalTestSuccess')
  init(&workletThreadId);
  return 0;
}
