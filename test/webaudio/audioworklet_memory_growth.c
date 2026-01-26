#include <assert.h>
#include <string.h>

#include <emscripten/em_js.h>
#include <emscripten/heap.h>
#include <emscripten/webaudio.h>

// Tests processing two stereo audio inputs being mixed to a single stereo audio
// output in process() (by adding the inputs together).

// This needs to be big enough for the stereo output, 2x inputs and the worker stack
#define AUDIO_STACK_SIZE 4096

// Shared file playback and bootstrap
#include "audioworklet_test_shared.inc"

// Callback to process and mix the audio tracks
bool process(int numInputs, const AudioSampleFrame* inputs, int numOutputs, AudioSampleFrame* outputs, int numParams, const AudioParamFrame* params, void* data) {
#ifdef TEST_AND_EXIT
  audioProcessedCount++;
#endif

  // Single stereo output
  assert(numOutputs == 1 && outputs[0].numberOfChannels == 2);
  for (int n = 0; n < numInputs; n++) {
    // And all inputs are also stereo
    assert(inputs[n].numberOfChannels == 2 || inputs[n].numberOfChannels == 0);
    // This should always be the case
    assert(inputs[n].samplesPerChannel == outputs[0].samplesPerChannel);
  }
  // We can now do a quick mix since we know the layouts
  if (numInputs > 0) {
    int totalSamples = outputs[0].samplesPerChannel * outputs[0].numberOfChannels;
    // Simple copy of the first input's audio data, checking that we have
    // channels (since a muted input has zero channels).
    float* outputData = outputs[0].data;
    if (inputs[0].numberOfChannels > 0) {
      memcpy(outputData, inputs[0].data, totalSamples * sizeof(float));
    } else {
      // And for muted we need to fill the buffer with zeroes otherwise it repeats the last frame
      memset(outputData, 0, totalSamples * sizeof(float));
    }
    // Now add another inputs
    for (int n = 1; n < numInputs; n++) {
      if (inputs[n].numberOfChannels > 0) {
        float* inputData = inputs[n].data;
        for (int i = totalSamples - 1; i >= 0; i--) {
          outputData[i] += inputData[i];
        }
      }
    }
  }
  return true;
}

void doGrow() {
  size_t max = emscripten_get_heap_max();
  size_t preGrow = emscripten_get_heap_size();
  // Note we're leaking this on purpose
  if (!malloc((preGrow / 3) * 2)) {
    emscripten_out("Failed to malloc()");
  }
  emscripten_outf("Heap was %zu, now %zu (of %zu)", preGrow, emscripten_get_heap_size(), max);
}

// Registered keypress event to grow the heap by 2/3
bool onPress(int type, const EmscriptenKeyboardEvent* e, void* data) {
  if (!e->repeat) {
    size_t size = emscripten_get_heap_size();
    if (emscripten_get_heap_max() == size) {
      emscripten_outf("Cannot grow heap, rebuild with ALLOW_MEMORY_GROWTH? Heap is already %zu", size);
    } else {
      if (e->charCode == 32) {
        emscripten_out("Growing from audio worklet (see Console)");
        emscripten_audio_worklet_post_function_v(VOIDP_2_WA(data), &doGrow);
      } else {
        doGrow();
      }
    }
  }
  return false;
}

// Audio processor created, now register the audio callback
void processorCreated(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
  assert(success && "Audio worklet failed in processorCreated()");
  emscripten_out("Audio worklet processor created");
  emscripten_out("Click to toggle audio playback");
  emscripten_out("Keypress to grow the heap, [space] to grow from the audio worklet's thread");

  // Stereo output, two inputs
  int outputChannelCounts[1] = { 2 };
  EmscriptenAudioWorkletNodeCreateOptions opts = {
    .numberOfInputs  = 2,
    .numberOfOutputs = 1,
    .outputChannelCounts = outputChannelCounts
  };
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T worklet = emscripten_create_wasm_audio_worklet_node(context, "mixer", &opts, &process, NULL);
  emscripten_audio_node_connect(worklet, context, 0, 0);

  // Create the two stereo source nodes and connect them to the two inputs
  // Note: we can connect the sources to the same input and it'll get mixed for us, but that's not the point
  beatID = createTrack(context, "audio_files/emscripten-beat.mp3", true);
  if (beatID) {
    emscripten_audio_node_connect(beatID, worklet, 0, 0);
  }
  bassID = createTrack(context, "audio_files/emscripten-bass.mp3", true);
  if (bassID) {
    emscripten_audio_node_connect(bassID, worklet, 0, 1);
  }

  // Register a click to start playback
  emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, WA_2_VOIDP(context), false, &onClick);
  // And a keypress to alloc (and leak) to grow the heap
  emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, WA_2_VOIDP(context), false, &onPress);

#ifdef TEST_AND_EXIT
  // Register the counter that exits the test after one second of mixing
  emscripten_set_timeout_loop(&playedAndMixed, 16, NULL);
#endif
}

// This implementation has no custom start-up requirements
EmscriptenStartWebAudioWorkletCallback getStartCallback(void) {
  return &initialised;
}
