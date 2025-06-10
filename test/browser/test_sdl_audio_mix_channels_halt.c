#include <SDL/SDL_mixer.h>
#include <emscripten.h>

Mix_Chunk* chunk = 0;
uint64_t started = 0;
uint64_t lastRun = 0;

void loop() {
  uint64_t now = emscripten_get_now();
  if (now - lastRun > 300) {
    Mix_PlayChannel(0, chunk, -1);
    lastRun = now;
  }
  if (now - started > 3000) {
    emscripten_force_exit(0);
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
  chunk = Mix_LoadWAV("/the_entertainer.ogg");
  if (!chunk) {
    printf("ERR! Mix_LoadWAV: %s\n", Mix_GetError());
    return 1;
  }

  printf("The sound should reset to start every 300ms, if you hear overapplied music then the test has failed.\n");
  started =  emscripten_get_now();
  emscripten_set_main_loop(loop, 0, 0);
  return 0;
}
