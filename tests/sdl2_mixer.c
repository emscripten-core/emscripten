#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define OGG_PATH "/sound.ogg"

Mix_Chunk *wave = NULL;

void sound_loop_then_quit() {
    if (Mix_Playing(-1))
        return;
    printf("Done audio\n");
    Mix_FreeChunk(wave);
    Mix_CloseAudio();

    emscripten_cancel_main_loop();
    printf("Shutting down\n");
    REPORT_RESULT(1);
}

int main(int argc, char* argv[]){
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
        return -1;
    int const frequency = EM_ASM_INT_V({
        var context;
        try {
            context = new AudioContext();
        } catch (e) {
            context = new webkitAudioContext(); // safari only
        }
        return context.sampleRate;
    });
    if(Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
        return -1;
    wave = Mix_LoadWAV(OGG_PATH);
    if (wave == NULL)
        return -1;
    if (Mix_PlayChannel(-1, wave, 0) == -1)
        return -1;
    printf("Starting sound play loop\n");
    emscripten_set_main_loop(sound_loop_then_quit, 0, 1);
    return 0;
}
