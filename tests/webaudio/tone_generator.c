#include <emscripten/webaudio.h>
#include <emscripten/em_math.h>

// This program tests that sharing the WebAssembly Memory works between the audio generator thread and the main browser UI thread.
// Two sliders, frequency and volume, can be adjusted on the HTML page, and the audio thread generates a sine wave tone based on
// these parameters.

// Implement smooth transition between the UI values and the values that the audio callback are actually processing, to avoid crackling when user adjusts the sliders.
float targetToneFrequency = 440.0f; // [shared variable between main thread and audio thread]
float targetVolume = 0.3f; // [shared variable between main thread and audio thread]

#define SAMPLE_RATE 48000
#define PI 3.14159265359

float phase = 0.f; // [local variable to the audio thread]
float phaseIncrement = 440 * 2.f * PI / SAMPLE_RATE; // [local variable to the audio thread]
float currentVolume = 0.3; // [local variable to the audio thread]

#ifdef REPORT_RESULT // This is defined when running in Emscripten test harness. You can strip these out in your own project.
volatile int audioProcessedCount = 0;
#endif

// This function will be called for every fixed 128 samples of audio to be processed.
EM_BOOL ProcessAudio(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData)
{
#ifdef REPORT_RESULT
	++audioProcessedCount;
#endif

	// Interpolate towards the target frequency and volume values.
	float targetPhaseIncrement = targetToneFrequency * 2.f * PI / SAMPLE_RATE;
	phaseIncrement = phaseIncrement * 0.95f + 0.05f * targetPhaseIncrement;
	currentVolume = currentVolume * 0.95f + 0.05f * targetVolume;

	// Produce a sine wave tone of desired frequency to all output channels.
	for(int o = 0; o < numOutputs; ++o)
		for(int i = 0; i < 128; ++i)
		{
			float s = emscripten_math_sin(phase);
			phase += phaseIncrement;
			for(int ch = 0; ch < outputs[o].numberOfChannels; ++ch)
				outputs[o].data[ch*128 + i] = s * currentVolume;
		}

	// Range reduce to keep precision around zero.
	phase = emscripten_math_fmod(phase, 2.f * PI);

	// We generated audio and want to keep this processor going. Return EM_FALSE here to shut down.
	return EM_TRUE;
}

#ifdef REPORT_RESULT
EM_BOOL observe_test_end(double time, void *userData)
{
  if (audioProcessedCount >= 100)
  {
    REPORT_RESULT(0);
    return EM_FALSE;
  }
  return EM_TRUE;
}
#endif

// This callback will fire after the Audio Worklet Processor has finished being added to the Worklet global scope.
void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData)
{
	if (!success) return;

	// Specify the input and output node configurations for the Wasm Audio Worklet. A simple setup with single mono output channel here, and no inputs.
	int outputChannelCounts[1] = { 1 };

	EmscriptenAudioWorkletNodeCreateOptions options = {
		.numberOfInputs = 0,
		.numberOfOutputs = 1,
		.outputChannelCounts = outputChannelCounts
	};

	// Instantiate the noise-generator Audio Worklet Processor.
	EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext, "tone-generator", &options, &ProcessAudio, 0);

	EM_ASM({
		let audioContext = emscriptenGetAudioObject($0);

		// Add a button on the page to toggle playback as a response to user click.
		let startButton = document.createElement('button');
		startButton.innerHTML = 'Toggle playback';
		document.body.appendChild(startButton);

		startButton.onclick = () => {
			if (audioContext.state != 'running') {
				audioContext.resume();
				let audioWorkletNode = emscriptenGetAudioObject($1);

				// Connect the audio worklet node to the graph.
				audioWorkletNode.connect(audioContext.destination);
			} else {
				audioContext.suspend();
			}
		};
	}, audioContext, wasmAudioWorklet);

#ifdef REPORT_RESULT
	emscripten_set_timeout_loop(observe_test_end, 10, 0);
#endif
}

// This callback will fire when the Wasm Module has been shared to the AudioWorklet global scope, and is now ready to begin adding Audio Worklet Processors.
void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData)
{
	if (!success) return;

	WebAudioWorkletProcessorCreateOptions opts = {
		.name = "tone-generator",
	};
	emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, AudioWorkletProcessorCreated, 0);
}

// Define a global stack space for the AudioWorkletGlobalScope. Note that all AudioWorkletProcessors and/or AudioWorkletNodes on the given Audio Context all share the same AudioWorkerGlobalScope,
// i.e. they all run on the same one audio thread (multiple nodes/processors do not each get their own thread). Hence one stack is enough.
uint8_t wasmAudioWorkletStack[4096];

int main()
{
	// Add a UI slider to the page to adjust the pitch of the tone.
	EM_ASM({
		let div = document.createElement('div');
		div.innerHTML = 'Choose frequency: <input style="width: 800px;" type="range" min="20" max="10000" value="440" class="slider" id="pitch"> <span id="pitchValue">440</span><br>' +
		                'Choose volume: <input style="width: 300px;" type="range" min="0" max="100" value="30" class="slider" id="volume"> <span id="volumeValue">30%</span><br>';
		document.body.appendChild(div);
		document.querySelector('#pitch').oninput = (e) => {
			document.querySelector('#pitchValue').innerHTML = HEAPF32[$0>>2] = parseInt(e.target.value);
		};
		document.querySelector('#volume').oninput = (e) => {
			HEAPF32[$1>>2] = parseInt(e.target.value) / 100;
			document.querySelector('#volumeValue').innerHTML = parseInt(e.target.value) + '%';
		};
	}, &targetToneFrequency, &targetVolume);

	// Create an audio context
	EmscriptenWebAudioCreateAttributes attrs = {
		.latencyHint = "interactive",
		.sampleRate = SAMPLE_RATE
	};

	EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(&attrs);

	// and kick off Audio Worklet scope initialization, which shares the Wasm Module and Memory to the AudioWorklet scope and initializes its stack.
	emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, 0);
}
