/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdint.h>
#include <memory.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int EMSCRIPTEN_WEBAUDIO_T;

typedef struct EmscriptenWebAudioCreateAttributes
{
	const char *latencyHint; // Specify one of "balanced", "interactive" or "playback"
	uint32_t sampleRate; // E.g. 44100 or 48000
} EmscriptenWebAudioCreateAttributes;

// Creates a new Web Audio AudioContext, and returns a handle to it.
EMSCRIPTEN_WEBAUDIO_T emscripten_create_audio_context(const EmscriptenWebAudioCreateAttributes *options);

typedef int AUDIO_CONTEXT_STATE;
#define AUDIO_CONTEXT_STATE_SUSPENDED 0
#define AUDIO_CONTEXT_STATE_RUNNING 1
#define AUDIO_CONTEXT_STATE_CLOSED 2
#define AUDIO_CONTEXT_STATE_INTERRUPTED	3

typedef void (*EmscriptenResumeAudioContextCallback)(EMSCRIPTEN_WEBAUDIO_T audioContext, AUDIO_CONTEXT_STATE state, void *userData1);

// Resumes the given AudioContext. The specified callback will fire when the AudioContext has completed resuming. Call this function
// inside a user event handler (mousedown, button click, etc.)
// userData1: A custom userdata pointer to pass to the callback function. This value will be passed on to the call to the given EmscriptenResumeAudioContextCallback callback function.
void emscripten_resume_audio_context_async(EMSCRIPTEN_WEBAUDIO_T audioContext, EmscriptenResumeAudioContextCallback callback, void *userData1);

// Synchronously attempts to resume the given AudioContext.
void emscripten_resume_audio_context_sync(EMSCRIPTEN_WEBAUDIO_T audioContext);

// Returns the current AudioContext state.
AUDIO_CONTEXT_STATE emscripten_audio_context_state(EMSCRIPTEN_WEBAUDIO_T audioContext);

typedef void (*EmscriptenStartWebAudioWorkletCallback)(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData2);

// Calls .suspend() on the given AudioContext and releases the JS object table
// reference to the given audio context. The specified handle is invalid
// after calling this function.
void emscripten_destroy_audio_context(EMSCRIPTEN_WEBAUDIO_T audioContext);

// Disconnects the given audio node from its audio graph, and then releases
// the JS object table reference to the given audio node. The specified handle
// is invalid after calling this function.
void emscripten_destroy_web_audio_node(EMSCRIPTEN_WEBAUDIO_T objectHandle);

// Create Wasm AudioWorklet thread. Call this function once at application startup to establish an AudioWorkletGlobalScope for your app.
// After the scope has been initialized, the given callback will fire.
// audioContext: The Web Audio context object to initialize the Wasm AudioWorklet thread on. Each AudioContext can have only one AudioWorklet
//               thread running, so do not call this function a multiple times on the same AudioContext.
// stackLowestAddress: The base address for the thread's stack. Must be aligned to 16 bytes. Use e.g. memalign(16, 1024) to allocate a 1KB stack for the thread.
// stackSize: The size of the thread's stack. Must be a multiple of 16 bytes.
// callback: The callback function that will be run when thread creation either succeeds or fails.
// userData2: A custom userdata pointer to pass to the callback function. This value will be passed on to the call to the given EmscriptenStartWebAudioWorkletCallback callback function.
void emscripten_start_wasm_audio_worklet_thread_async(EMSCRIPTEN_WEBAUDIO_T audioContext, void *stackLowestAddress, uint32_t stackSize, EmscriptenStartWebAudioWorkletCallback callback, void *userData2);

typedef int WEBAUDIO_PARAM_AUTOMATION_RATE;
#define WEBAUDIO_PARAM_A_RATE 0
#define WEBAUDIO_PARAM_K_RATE 1

typedef struct WebAudioParamDescriptor
{
	float defaultValue; // Default == 0.0
	float minValue; // Default = -3.4028235e38;
	float maxValue; // Default = 3.4028235e38;
	WEBAUDIO_PARAM_AUTOMATION_RATE automationRate; // Either WEBAUDIO_PARAM_A_RATE or WEBAUDIO_PARAM_K_RATE. Default = WEBAUDIO_PARAM_A_RATE
} WebAudioParamDescriptor;

typedef struct WebAudioWorkletProcessorCreateOptions
{
	const char *name; // The name of the AudioWorkletProcessor that is being created.

	int numAudioParams;
	const WebAudioParamDescriptor *audioParamDescriptors;
} WebAudioWorkletProcessorCreateOptions;

typedef void (*EmscriptenWorkletProcessorCreatedCallback)(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData3);

// Creates a new AudioWorkletProcessor with the given name and specified set of control parameters.
// userData3: A custom userdata pointer to pass to the callback function. This value will be passed on to the call to the given EmscriptenWorkletProcessorCreatedCallback callback function.
void emscripten_create_wasm_audio_worklet_processor_async(EMSCRIPTEN_WEBAUDIO_T audioContext, const WebAudioWorkletProcessorCreateOptions *options, EmscriptenWorkletProcessorCreatedCallback callback, void *userData3);

typedef int EMSCRIPTEN_AUDIO_WORKLET_NODE_T;

typedef struct AudioSampleFrame
{
	const int numberOfChannels;
	// An array of length numberOfChannels*128 elements, where data[channelIndex*128+i] locates the data of the i'th sample of channel channelIndex.
	float *data;
} AudioSampleFrame;

typedef struct AudioParamFrame
{
	// Specifies the length of the input array data (in float elements). This will be guaranteed to either have
	// a value of 1 or 128, depending on whether the audio parameter changed during this frame.
	int length;
	// An array of length specified in 'length'.
	float *data;
} AudioParamFrame;

typedef EM_BOOL (*EmscriptenWorkletNodeProcessCallback)(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData4);

typedef struct EmscriptenAudioWorkletNodeCreateOptions
{
	// How many audio nodes does this node take inputs from? Default=1
	int numberOfInputs;
	// How many audio nodes does this node output to? Default=1
	int numberOfOutputs;
	// For each output, specifies the number of audio channels (1=mono/2=stereo/etc.) for that output. Default=an array of ones for each output channel.
	int *outputChannelCounts;
} EmscriptenAudioWorkletNodeCreateOptions;

// Instantiates the given AudioWorkletProcessor as an AudioWorkletNode, which continuously calls the specified processCallback() function on the browser's audio thread to perform audio processing.
// userData4: A custom userdata pointer to pass to the callback function. This value will be passed on to the call to the given EmscriptenWorkletNodeProcessCallback callback function.
EMSCRIPTEN_AUDIO_WORKLET_NODE_T emscripten_create_wasm_audio_worklet_node(EMSCRIPTEN_WEBAUDIO_T audioContext, const char *name, const EmscriptenAudioWorkletNodeCreateOptions *options, EmscriptenWorkletNodeProcessCallback processCallback, void *userData4);

// Returns EM_TRUE if the current thread is executing a Wasm AudioWorklet, EM_FALSE otherwise.
// Note that calling this function can be relatively slow as it incurs a Wasm->JS transition,
// so avoid calling it in hot paths.
EM_BOOL emscripten_current_thread_is_audio_worklet(void);

#define EMSCRIPTEN_AUDIO_MAIN_THREAD 0

/* emscripten_audio_worklet_function_*: Post a pointer to a C/C++ function to be executed either
  on the Audio Worklet thread of the given Web Audio context. Notes:
 - If running inside an Audio Worklet thread, specify ID EMSCRIPTEN_AUDIO_MAIN_THREAD (== 0) to pass a message
   from the audio worklet to the main thread.
 - When specifying non-zero ID, the Audio Context denoted by the ID must have been created by the calling thread.
 - Passing messages between audio thread and main thread with this family of functions is relatively slow and has
   a really high latency cost compared to direct coordination using atomics and synchronization primitives like
   mutexes and synchronization primitives. Additionally these functions will generate garbage on the JS heap.
   Therefore avoid using these functions where performance is critical. */
void emscripten_audio_worklet_post_function_v(EMSCRIPTEN_WEBAUDIO_T id, void (*funcPtr)(void));
void emscripten_audio_worklet_post_function_vi(EMSCRIPTEN_WEBAUDIO_T id, void (*funcPtr)(int), int arg0);
void emscripten_audio_worklet_post_function_vii(EMSCRIPTEN_WEBAUDIO_T id, void (*funcPtr)(int, int), int arg0, int arg1);
void emscripten_audio_worklet_post_function_viii(EMSCRIPTEN_WEBAUDIO_T id, void (*funcPtr)(int, int, int), int arg0, int arg1, int arg2);
void emscripten_audio_worklet_post_function_vd(EMSCRIPTEN_WEBAUDIO_T id, void (*funcPtr)(double), double arg0);
void emscripten_audio_worklet_post_function_vdd(EMSCRIPTEN_WEBAUDIO_T id, void (*funcPtr)(double, double), double arg0, double arg1);
void emscripten_audio_worklet_post_function_vddd(EMSCRIPTEN_WEBAUDIO_T id, void (*funcPtr)(double, double, double), double arg0, double arg1, double arg2);
void emscripten_audio_worklet_post_function_sig(EMSCRIPTEN_WEBAUDIO_T id, void *funcPtr, const char *sig, ...);

#ifdef __cplusplus
} // ~extern "C"
#endif
