#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <assert.h>
#include <limits.h>
#include <emscripten.h>

Mix_Chunk *sound;

void play() {
  int channel = Mix_PlayChannel(-1, sound, 1);
  assert(channel == 0);

  int result = 1;
  REPORT_RESULT();
}

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_AUDIO);

  int ret = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
  assert(ret == 0);

  Uint16* buffer = (Uint16*)malloc(10*44100*sizeof(Uint16));
  for (Uint32 i = 0; i < 10*44100; ++i) {
    buffer[i] = (i * 5) % UINT32_MAX;
  }
  sound = Mix_QuickLoad_RAW((Uint8*) buffer, 10*44100*sizeof(Uint16));
  assert(sound);

  play();

  emscripten_run_script("element = document.createElement('input');"
                        "element.setAttribute('type', 'button');"
                        "element.setAttribute('value', 'replay!');"
                        "element.setAttribute('onclick', 'Module[\"_play\"]()');"
                        "document.body.appendChild(element);");

  printf("you should one sounds. press the button to replay!\n");

  return 0;
}

