#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <assert.h>
#include <emscripten.h>

Mix_Chunk *sound;

void play() {
  int channel = Mix_PlayChannel(-1, sound, 1);
  assert(channel >= 0);
}

int main() {
  SDL_Init(SDL_INIT_AUDIO);

  int ret = Mix_OpenAudio(0, 0, 0, 0); // we ignore all these..
  assert(ret == 0);

  sound = Mix_LoadWAV("sound.ogg");
  assert(sound);

  play();

  emscripten_run_script("element = document.createElement('input');"
                        "element.setAttribute('type', 'button');"
                        "element.setAttribute('value', 'replay!');"
                        "element.setAttribute('onclick', '_play()');"
                        "document.body.appendChild(element);");

  printf("you should hear a victory sound. press the button to replay!\n");

  int result = 1;
  REPORT_RESULT();

  return 0;
}

