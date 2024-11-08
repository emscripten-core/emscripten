#include <stdio.h>

#include <emscripten/em_js.h>
#include <emscripten/webaudio.h>

#define AUDIO_STACK_SIZE 4096

#ifdef REPORT_RESULT
void playedAndMixed(void* data) {
	REPORT_RESULT(0);
}
#endif

// Adds a button to play and stop an audio file
EM_JS(bool, addAudio, (EMSCRIPTEN_WEBAUDIO_T ctxHnd, EMSCRIPTEN_AUDIO_WORKLET_NODE_T nodeHnd, int index, const char* url, const char* label), {
	var context = emscriptenGetAudioObject(ctxHnd);
	if (context) {
		var audio = document.createElement('audio');
		audio.src = UTF8ToString(url);
		audio.loop = true;
		var track = context.createMediaElementSource(audio);

		var worklet = emscriptenGetAudioObject(nodeHnd);
		track.connect(worklet ? worklet : context.destination, 0, index);

		var button = document.createElement('button');
		button.innerHTML = UTF8ToString(label);
		button.onclick = () => {
			if (context.state == 'suspended') {
				context.resume();
			}
			if (audio.paused) {
				audio.currentTime = 0;
				audio.play();
			} else {
				audio.pause();
			}
			
		};
		document.body.appendChild(button);
		return true;
	}
	return false;
});

bool process(int numInputs, const AudioSampleFrame* inputs, int numOutputs, AudioSampleFrame* outputs, int numParams, const AudioParamFrame* params, void* data) {
	for (int o = 0; o < numOutputs; o++) {
		for (int n = outputs[o].samplesPerChannel * outputs[o].numberOfChannels - 1; n >= 0; n--) {
			outputs[o].data[n] = 0.0f;
			for (int i = 0; i < numInputs; i++) {
				outputs[o].data[n] += inputs[i].data[n] * 0.75f;
			}
		}
	}
	return true;
}

void processorCreated(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
	if (success) {
		printf("Audio worklet processor created\n");

		int outputChannelCounts[1] = { 2 };
		EmscriptenAudioWorkletNodeCreateOptions opts = {
			.numberOfInputs  = 2,
			.numberOfOutputs = 1,
			.outputChannelCounts = outputChannelCounts
		};
		EMSCRIPTEN_AUDIO_WORKLET_NODE_T worklet = emscripten_create_wasm_audio_worklet_node(context, "mixer", &opts, &process, NULL);
		emscripten_audio_node_connect(worklet, context, 0, 0);

		addAudio(context, worklet, 0, "audio_files/emscripten-beat.mp3", "Toggle Beat");
		addAudio(context, worklet, 1, "audio_files/emscripten-bass.mp3", "Toggle Bass");
	} else {
		printf("Audio worklet node creation failed\n");
	}
}

void initialised(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
	if (success) {
		printf("Audio worklet initialised\n");

		WebAudioWorkletProcessorCreateOptions opts = {
			.name = "mixer",
		};
		emscripten_create_wasm_audio_worklet_processor_async(context, &opts, &processorCreated, NULL);
	} else {
		printf("Audio worklet failed to initialise\n");
	}
}

int main() {
	static char workletStack[AUDIO_STACK_SIZE];
	EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(NULL);
	emscripten_start_wasm_audio_worklet_thread_async(context, workletStack, sizeof(workletStack), initialised, NULL);
	
	return 0;
}
