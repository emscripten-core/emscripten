// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include <atomic>
#include <mutex>

#include <emscripten/emscripten.h>
#include <emscripten/em_js.h>
#include <emscripten/threading.h>

static pthread_t workletThreadId = 0; // This is set on the main thread when the pthread is created
static uint32_t sampleRate = 48000; 
static float invSampleRate = 1.0f / sampleRate;

///////////////////////////////////////////////////////////////////////////////
// Section 1 - Test-only code
//
// This is just the testing code used to verify the pthread setup in the audio
// worklet. This can be ignored if you're just wanting to see how to implement
// native code audio worklets with Emscripten.
///////////////////////////////////////////////////////////////////////////////

std::atomic<pthread_t> pthreadIdFromWorklet(0); // This is set from the audio thread when the AudioWorkletNode is constructed
std::mutex testMutex; // A test mutex to test the worklet futex implementation

// Sends the current pthread id from the audio worklet node, signalling test success
// This happens immediately in the worklet node constructor since 'process' is never
// invoked without user interaction
EMSCRIPTEN_KEEPALIVE extern "C" void signalTestSuccess() {
  pthread_t threadId = pthread_self();
  pthreadIdFromWorklet.store(threadId);

  printf("Audio thread time: %f\n", emscripten_get_now());

  // Put the worklet to sleep until the main thread sees the above store.
  // NOTE: This is *really bad* and should never be done in an audio worklet, but
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

EMSCRIPTEN_KEEPALIVE extern "C" void beforeUnload() {
  // Always release the mutex on unload, otherwise the worklet can get stuck if we navigate away
  testMutex.unlock();
}

EM_JS(void, setupUnloadHandler, (), {
  window.addEventListener("beforeunload", function() {
      _beforeUnload();
  });
});

///////////////////////////////////////////////////////////////////////////////
// Section 2 - Audio worklet example
//
// This is the relevant example code showing how to use native code to render 
// audio in an audio worklet almost the same way as if it was a regular pthread
///////////////////////////////////////////////////////////////////////////////

// This is the native code audio generator - it outputs an interleaved stereo buffer
// containing a simple, continuous sine wave. 
EMSCRIPTEN_KEEPALIVE extern "C" float* generateAudio(unsigned int numSamples) {
  assert(numSamples == 128); // Audio worklet quantum size is always 128
  static float outputBuffer[128*2]; // This is where we generate our data into
  static float wavePos = 0; // This is the generator wave position [0, 2*PI) 
  const float PI2 = 3.14159f * 2.0f; // Very approximate :)
  const float FREQ = 440.0f; // Sine frequency
  const float MAXAMP = 0.2f; // 20% so it's not too loud

  float* out = outputBuffer;
  while(numSamples > 0) {
    // Amplitude at current position
    float a = sinf(wavePos) * MAXAMP; 
  
    // Advance position, keep it in [0, 2*PI) range to avoid running out of float precision
    wavePos += invSampleRate * FREQ * PI2;
    if(wavePos > PI2) {
      wavePos -= PI2;
    }
    
    // Set both left and right samples to the same value
    out[0] = a;
    out[1] = a;
    out += 2;

    numSamples -= 1;
  }

  return outputBuffer;
}

// Initializes the audio context and the pthread it it's AudioWorkletGlobalScope
EM_JS(uint32_t, initAudioContext, (pthread_t* pthreadPtr), {
  // Create the context
  Module.audioCtx = new AudioContext();

  // To make this example usable we setup a resume on user interaction as browsers
  // all require the user to interact with the page before letting audio play
  if (window && window.addEventListener) {
    var opts = { capture: true, passive : true };
    
    var resume = function () {
      Module.audioCtx.resume();
      window.removeEventListener("touchstart", resume, opts);
      window.removeEventListener("mousedown", resume, opts);
      window.removeEventListener("keydown", resume, opts);
    };
    
    window.addEventListener("touchstart", resume, opts);
    window.addEventListener("mousedown", resume, opts);
    window.addEventListener("keydown", resume, opts);
  }

  // Initialize the pthread shared by all AudioWorkletNodes in this context
  PThread.initAudioWorkletPThread(Module.audioCtx, pthreadPtr).then(function() {
    out("PThread context initialized!")
  }, function(err) {
    out("PThread context initialization failed: " + [err, err.stack]);
  });

  return Module.audioCtx.sampleRate;
});

// Creates an AudioWorkletNode and connects it to the output once it's created
EM_JS(void, createAudioWorklet, (), {
  PThread.createAudioWorklet(
    Module.audioCtx,
    'native-passthrough-processor', 
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
  // Section 1 - test-only code, ignore 
  setupUnloadHandler();
  emscripten_request_animation_frame_loop(mainLoop, nullptr);
  testMutex.lock(); // Lock the mutex so that the worklet thread can wait on it (see comment in 'signalTestSuccess')

  // Section 2 - audio worklet example
  sampleRate = initAudioContext(&workletThreadId);
  invSampleRate = 1.0f / sampleRate;
  printf("Initialized audio context. Sample rate: %d. PThread init pending.\n", sampleRate);
  createAudioWorklet();
  printf("Creating audio worklet.\n");
}
