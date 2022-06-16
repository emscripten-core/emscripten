.. _wasm_audio_worklets:

================
Wasm Audio Worklets API
================

The AudioWorklet extension to the `Web Audio API specification 
<https://webaudio.github.io/web-audio-api/#AudioWorklet>`_ enables web sites
to implement custom AudioWorkletProcessor Web Audio graph node types.

These custom processor nodes process audio data in real-time as part of the
audio graph processing flow, and enable developers to write low latency
sensitive audio processing code in JavaScript.

The Emscripten Wasm Audio Worklets API is an Emscripten-specific integration
of these AudioWorklet nodes to WebAssembly. Wasm Audio Worklets enables
developers to implement AudioWorklet processing nodes in C/C++ code that
compile down to WebAssembly, rather than implementing audio processing in
JavaScript.

Developing AudioWorkletProcessors in WebAssembly provides the benefit of
greatly improved performance compared to JavaScript, and the Emscripten
Wasm Audio Worklets system runtime has been carefully developed to guarantee
that no temporary JavaScript level VM garbage will be generated, eliminating
the possibility of GC pauses from impacting audio performance.

Development Overview
====================

Authoring Wasm Audio Worklets is very similar to developing Audio Worklets
API based applications in general (see `MDN: Using AudioWorklets <https://developer.mozilla.org/en-US/docs/Web/API/Web_Audio_API/Using_AudioWorklet>`_), with the exception that a
small amount of JS <-> Wasm (C/C++) interop needs to be taken into account.

Audio Worklets operate on a two layer "class type & its instance" design:
first one defines one or more node types (or classes) called AudioWorkletProcessors,
and then, these processors are instantiated one or more times in the audio
processing graph as AudioWorkletNodes.

Once a class type is instantiated on the Web Audio graph and the graph is
running, a C/C++ function pointer callback will be invoked for each 128
samples of the processed audio stream that flows through the node.

This callback will be executed on a dedicated separate audio processing
thread with real-time processing priority. Each Web Audio context will
utilize only a single audio processing thread. That is, even if there are
multiple audio node instances (maybe from multiple different audio processors),
these will all share the same dedicated audio thread on the AudioContext,
and will not run in a separate thread of their own each.

Programming Example
===================

To get hands-on experience with programming Wasm Audio Worklets, let's create a
simple application which outputs random noise through the output channels.

1. First, we will create a Web Audio context in C/C++ code. This is achieved
via the ``emscripten_create_audio_context()`` function. In a larger application
that integrates existing Web Audio libraries, you may already have an
``AudioContext`` created via some other library, in which case you would instead
register the context to be visible to WebAssembly by calling the function
``emscriptenRegisterAudioObject()``.

Then, we will instruct the Emscripten runtime to establish a Wasm Audio Worklet
thread scope on this context. The code for this looks like:

.. code-block:: cpp

  #include <emscripten/webaudio.h>

  uint8_t audioThreadStack[4096];

  int main()
  {
    EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(0 /* use default constructor options */);

    emscripten_start_wasm_audio_worklet_thread_async(context, audioThreadStack, sizeof(audioThreadStack), AudioThreadInitialized, 0);
  }

2. When the worklet thread context has been initialized, we are ready to define our
own noise generator AudioWorkletProcessor node type:

.. code-block:: cpp

  void AudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData)
  {
    if (!success) return; // Check browser console in a debug build for detailed errors
    WebAudioWorkletProcessorCreateOptions opts = {
      .name = "noise-generator",
    };
    emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, AudioWorkletProcessorCreated, 0);
  }

3. After the processor has initialized, we can now instantiate and connect it as a node on the graph. Since on
web pages audio playback can only be initiated as a response to user input, we will also register an event handler
which resumes the audio context when the user clicks on the DOM Canvas element that exists on the page.

.. code-block:: cpp

void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData)
{
    if (!success) return; // Check browser console in a debug build for detailed errors

  int outputChannelCounts[1] = { 1 };
  EmscriptenAudioWorkletNodeCreateOptions options = {
    .numberOfInputs = 0,
    .numberOfOutputs = 1,
    .outputChannelCounts = outputChannelCounts
  };

  // Create node
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext, "noise-generator", &options, &GenerateNoise, 0);

  // Connect it to audio context destination
  EM_ASM({emscriptenGetAudioObject($0).connect(emscriptenGetAudioObject($1).destination)},
    wasmAudioWorklet, audioContext);

  // Resume context on mouse click
  emscripten_set_click_callback("canvas", (void*)audioContext, 0, OnCanvasClick);
}

4. The code to resume the audio context on click looks like this:

.. code-block:: cpp

EM_BOOL OnCanvasClick(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
  EMSCRIPTEN_WEBAUDIO_T audioContext = (EMSCRIPTEN_WEBAUDIO_T)userData;
  if (emscripten_audio_context_state(audioContext) != AUDIO_CONTEXT_STATE_RUNNING) {
    emscripten_resume_audio_context_sync(audioContext);
  }
  return EM_FALSE;
}

5. Finally we can implement the audio callback that is to generate the noise:

.. code-block:: cpp

  #include <emscripten/em_math.h>

  EM_BOOL GenerateNoise(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData)
  {
    for(int i = 0; i < numOutputs; ++i)
      for(int j = 0; j < 128*outputs[i].numberOfChannels; ++j)
        outputs[i].data[j] = emscripten_random() * 0.2 - 0.1; // Warning: scale down audio volume, raw noise can be really loud otherwise

    return EM_TRUE; // Keep the graph output going
  }

And that's it! Compile the code with the linker flags ``-sAUDIO_WORKLET=1 -sWASM_WORKERS=1`` to enable targeting AudioWorklets.

More Examples
=============

See the directory tests/webaudio/ for more code examples on Web Audio API and Wasm AudioWorklets.
