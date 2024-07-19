#include <emscripten/webaudio.h>

// This code shows a simple example of how to create a Web Audio context from C/C++ code using the webaudio.h API,
// and how to add a pure sine wave tone generator to it.

int main()
{
	EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(0 /* use default constructor options */);

	// Illustrate how this handle can be passed to JS code (e.g. to a JS library function, EM_ASM or a EM_JS block)
	EM_ASM({
		var audioContext = emscriptenGetAudioObject($0);

		var oscillator = audioContext.createOscillator();
		oscillator.connect(audioContext.destination);
		oscillator.start();

		// Add a button on the page to toggle playback as a response to user click.
		var startButton = document.createElement('button');
		startButton.innerHTML = 'Toggle playback';
		document.body.appendChild(startButton);

		startButton.onclick = () => {
			if (audioContext.state != 'running') {
				audioContext.resume();
#ifdef REPORT_RESULT
				__ReportResult(0);
#endif
			} else {
				audioContext.suspend();
			}
		};
	}, context);
}
