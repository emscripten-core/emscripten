#include <assert.h>
#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define WAV_PATH "/sound.wav"

Mix_Chunk *wave = NULL;

void sound_loop_then_quit() {
    if (Mix_Playing(-1))
        return;
    printf("Done audio\n");
    Mix_FreeChunk(wave);
    Mix_CloseAudio();

    emscripten_cancel_main_loop();
    printf("Shutting down\n");
    exit(0);
}

int main(int argc, char* argv[]){
    SDL_version version;
    SDL_GetVersion(&version);
    printf("Linked against SDL %d.%d.%d.\n",
           version.major, version.minor, version.patch);
    assert(version.major == 2);

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        puts("Failed to init audio");
        return 100;
    }
    int const frequency = EM_ASM_INT({
        var context;
        try {
            context = new AudioContext();
        } catch (e) {
            context = new webkitAudioContext(); // safari only
        }
        return context.sampleRate;
    });
    if(Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        puts("Failed to open audio");
        return 101;
    }
    wave = Mix_LoadWAV(WAV_PATH);
    if (wave == NULL) {
        puts("Failed to load audio");
        return 102;
    }
    if (Mix_PlayChannel(-1, wave, 0) == -1) {
        puts("Failed to play audio");
        return 103;
    }
    printf("Starting sound play loop\n");
    emscripten_set_main_loop(sound_loop_then_quit, 0, 1);
    return 0;
}
