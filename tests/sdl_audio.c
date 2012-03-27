#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <assert.h>
#include <emscripten.h>

Mix_Chunk *sound, *sound2;

void play2();

void play() {
  int channel = Mix_PlayChannel(-1, sound, 1);
  assert(channel >= 0);

  emscripten_run_script("setTimeout(_play2, 1000)");
}

void play2() {
  int channel2 = Mix_PlayChannel(-1, sound2, 1);
  assert(channel2 >= 0);
}

int main() {
  SDL_Init(SDL_INIT_AUDIO);

  int ret = Mix_OpenAudio(0, 0, 0, 0); // we ignore all these..
  assert(ret == 0);

  sound = Mix_LoadWAV("sound.ogg");
  assert(sound);
  sound2 = Mix_LoadWAV("sound2.wav");
  assert(sound);

  play();
  if (ret == 12121) play2(); // keep it alive

  emscripten_run_script("element = document.createElement('input');"
                        "element.setAttribute('type', 'button');"
                        "element.setAttribute('value', 'replay!');"
                        "element.setAttribute('onclick', '_play()');"
                        "document.body.appendChild(element);");

  printf("you should hear two sounds. press the button to replay!\n");

  int result = 1;
  REPORT_RESULT();

  return 0;
}

