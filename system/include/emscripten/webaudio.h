/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
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

typedef void (*EmscriptenResumeAudioContextCallback)(EMSCRIPTEN_WEBAUDIO_T audioContext, AUDIO_CONTEXT_STATE state, void *userData);

// Resumes the given AudioContext. The specified callback will fire when the AudioContext has completed resuming. Call this function
// inside a user event handler (mousedown, button click, etc.)
void emscripten_resume_audio_context_async(EMSCRIPTEN_WEBAUDIO_T audioContext, EmscriptenResumeAudioContextCallback callback, void *userData);

typedef void (*EmscriptenStartWebAudioWorkletCallback)(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData);

// Create Wasm AudioWorklet thread. Call this function once at application startup to establish an AudioWorkletGlobalScope for your app.
// After the scope has been initialized, the given callback will fire.
void emscripten_start_wasm_audio_worklet_thread_async(EMSCRIPTEN_WEBAUDIO_T audioContext, void *stackLowestAddress, uint32_t stackSize, EmscriptenStartWebAudioWorkletCallback callback, void *userData);

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

typedef void (*EmscriptenWorkletProcessorCreatedCallback)(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData);

// Creates a new AudioWorkletProcessor with the given name and specified set of control parameters.
void emscripten_create_wasm_audio_worklet_processor_async(EMSCRIPTEN_WEBAUDIO_T audioContext, const WebAudioWorkletProcessorCreateOptions *options, EmscriptenWorkletProcessorCreatedCallback callback, void *userData);

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

typedef EM_BOOL (*EmscriptenWorkletNodeProcessCallback)(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData);

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
EMSCRIPTEN_AUDIO_WORKLET_NODE_T emscripten_create_wasm_audio_worklet_node(EMSCRIPTEN_WEBAUDIO_T audioContext, const char *name, const EmscriptenAudioWorkletNodeCreateOptions *options, EmscriptenWorkletNodeProcessCallback processCallback, void *userData);

// Returns EM_TRUE if the current thread is executing a Wasm AudioWorklet, EM_FALSE otherwise.
// Note that calling this function can be relatively slow as it incurs a Wasm->JS transition,
// so avoid calling it in hot paths.
EM_BOOL emscripten_current_thread_is_audio_worklet(void);

#ifdef __cplusplus
} // ~extern "C"
#endif
