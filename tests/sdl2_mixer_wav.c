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
#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif
}

int main(int argc, char* argv[]){
    SDL_version version;
    SDL_GetVersion(&version);
    printf("Linked against SDL %d.%d.%d.\n",
           version.major, version.minor, version.patch);
    assert(version.major == 2);

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        puts("Failed to init audio");
#ifdef REPORT_RESULT
        REPORT_RESULT(100);
#endif
    }
    int const frequency = EM_ASM_INT_V({
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
#ifdef REPORT_RESULT
        REPORT_RESULT(101);
#endif
    }
    wave = Mix_LoadWAV(WAV_PATH);
    if (wave == NULL) {
        puts("Failed to load audio");
#ifdef REPORT_RESULT
        REPORT_RESULT(102);
#endif
    }
    if (Mix_PlayChannel(-1, wave, 0) == -1) {
        puts("Failed to play audio");
#ifdef REPORT_RESULT
        REPORT_RESULT(103);
#endif
    }
    printf("Starting sound play loop\n");
    emscripten_set_main_loop(sound_loop_then_quit, 0, 1);
    return 0;
}
