#include <emscripten/em_js.h>
#include <emscripten/webaudio.h>

#define AUDIO_STACK_SIZE 2048

EM_JS(bool, addAudio, (EMSCRIPTEN_WEBAUDIO_T ctxHnd, const char* url, const char* label), {
	var context = emscriptenGetAudioObject(ctxHnd);
	if (context) {
		var audio = document.createElement('audio');
		audio.src = UTF8ToString(url);
		audio.loop = true;
		var track = context.createMediaElementSource(audio);
		track.connect(context.destination);
		
		var button = document.createElement('button');
		button.innerHTML = UTF8ToString(label);
		button.onclick = () => {
			if (context.state == 'suspended') {
				context.resume();
			}
			audio.play();
		};
		
		document.body.appendChild(button);
		return true;
	}
	return false;
});

int main() {
	EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(NULL);
	
	addAudio(context, "audio_files/emscripten-beat.mp3", "Play Beat");
	addAudio(context, "audio_files/emscripten-bass.mp3", "Play Bass");
}
