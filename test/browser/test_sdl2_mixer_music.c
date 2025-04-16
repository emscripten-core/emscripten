#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

Mix_Music* music;

void main2()
{
	if (Mix_PlayingMusic())
		return;

	printf("Done playing sound\n");
	printf("Shutting down\n");
	emscripten_cancel_main_loop();
	Mix_FreeMusic(music);
	Mix_CloseAudio();
	emscripten_force_exit(0);
}

int main(int argc, char* argv[])
{
	int frequency, active_flags;

	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	frequency = EM_ASM_INT({
		var context;
		try {
		  context = new AudioContext();
		} catch (e) {
		  context = new webkitAudioContext(); // safari only
		}
		return context.sampleRate;
	});

	if(Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
		printf("Failed to Mix_OpenAudio(): %s\n", Mix_GetError());
		return -1;
	}

#ifndef FLAGS
#error "must define FLAGS"
#endif
	active_flags = Mix_Init(FLAGS);
	if ((FLAGS & active_flags) != FLAGS) {
		printf("Failed to Mix_Init(): %s\n", Mix_GetError());
		return -1;
	}

#ifndef SOUND_PATH
#error "must define SOUND_PATH"
#endif
	music = Mix_LoadMUS(SOUND_PATH);
	if (music == NULL) {
		printf("Failed to Mix_LoadMUS(): %s\n", Mix_GetError());
		return -1;
	}

	if (Mix_PlayMusic(music, 1)) {
		printf("Failed to Mix_PlayMusic(): %s\n", Mix_GetError());
		return -1;
	}

	printf("Playing sound\n");
	emscripten_set_main_loop(main2, 0, 1);
	return 0;
}
