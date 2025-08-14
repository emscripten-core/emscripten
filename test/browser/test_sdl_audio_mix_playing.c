#include <SDL/SDL_mixer.h>
#include <emscripten.h>

Mix_Chunk* chunk = 0;
uint64_t started = 0;

void loop() {
  uint64_t now = emscripten_get_now();
  if (now - started > 2000) {
    // length of sound is 1sec
    printf("isPlaying? %s\n", Mix_Playing(0) ? "yes" : "no");
    emscripten_force_exit(Mix_Playing(0) ? 1 : 0);
  }
}

int main(int argc, char** argv) {
  if ((Mix_Init(MIX_INIT_OGG) & MIX_INIT_OGG) != MIX_INIT_OGG) {
    printf("ERR! Mix_Init: Failed to init with required flags support!\n");
    printf("ERR! Mix_Init: %s\n", Mix_GetError());
    return 1;
  }

  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 1, 1024) == -1) {
    printf("ERR! Mix_OpenAudio: %s\n", Mix_GetError());
    return 1;
  }

  Mix_AllocateChannels(1);
  chunk = Mix_LoadWAV("/noise.ogg");
  if (!chunk) {
    printf("ERR! Mix_LoadWAV: %s\n", Mix_GetError());
    return 1;
  }
  Mix_PlayChannel(0, chunk, 0);
  if (!Mix_Playing(0)) {
    printf("ERR! Channel is not playing\n");
    return 1;
  }
  started = emscripten_get_now();
  emscripten_set_main_loop(loop, 0, 0);
  return 0;
}
