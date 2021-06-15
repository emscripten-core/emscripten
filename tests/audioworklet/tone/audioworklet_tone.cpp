// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <emscripten/emscripten.h>

// This is a simple example of using the emscripten audio worklet integration
// to run any native audio code.
// There are three components:
// 1) generateAudio is the native function that generates the audio samples
// 2) init is the JS-side audio worklet initialization that:
//    - a) creates an AudioContext
//    - b) initializes a PThread context in that AudioContext
//    - c) creates an AudioWorkletNode running the 'native-passthrough-processor' 
//         that calls generateAudio from 1)
// 3) the NativePassthroughProcessor in audioworklet_tone_post.js that is the
//    audio thread side implementation of 'native-passthrough-processor'
//
// Compile with
// emcc -s USE_PTHREADS=1 -s MODULARIZE=1 -s EXPORT_NAME=MyModule -s ENVIRONMENT=web,worker,audioworklet --extern-post-js=audioworklet_tone_post.js --shell-file ../../shell_that_launches_modularize.html -o audioworklet_tone.html audioworklet_tone.cpp

static pthread_t workletThreadId = 0;
static uint32_t sampleRate = 48000; 
static float invSampleRate = 1.0f / sampleRate;

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
EM_JS(uint32_t, init, (pthread_t* pthreadPtr), {
  // Create the context
  Module.audioCtx = new AudioContext();

  // Initialize the pthread shared by all AudioWorkletNodes in this context
  PThread.initAudioWorkletPThread(Module.audioCtx, pthreadPtr).then(function() {
    out("Audio worklet PThread context initialized!")
  }, function(err) {
    out("Audio worklet PThread context initialization failed: " + [err, err.stack]);
  });

  // Creates an AudioWorkletNode and connects it to the output once it's created
  PThread.createAudioWorkletNode(
    Module.audioCtx,
    'native-passthrough-processor', 
    {
      numberOfInputs: 0,
      numberOfOutputs : 1,
      outputChannelCount : [2]
    }
  ).then(function(workletNode) {
    // Connect the worklet to the audio context output
    out("Audio worklet node created! Tap/click on the window if you don't hear audio!");
    workletNode.connect(Module.audioCtx.destination);
  }, function(err) {
    out("Audio worklet node creation failed: " + [err, err.stack]);
  });

  // To make this example usable we setup a resume on user interaction as browsers
  // all require the user to interact with the page before letting audio play
  if (window && window.addEventListener) {
    var opts = { capture: true, passive : true };    
    window.addEventListener("touchstart", function() { Module.audioCtx.resume() }, opts);
    window.addEventListener("mousedown", function() { Module.audioCtx.resume() }, opts);
    window.addEventListener("keydown", function() { Module.audioCtx.resume() }, opts);
  }

  return Module.audioCtx.sampleRate;
});

int main()
{
  sampleRate = init(&workletThreadId);
  invSampleRate = 1.0f / sampleRate;
  printf("Initialized audio context. Sample rate: %d. PThread init pending.\n", sampleRate);
  return 0;
}
