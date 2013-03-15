#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <assert.h>
#include <emscripten.h>

Mix_Chunk *sound, *sound2;

int play2();

int play() {
  int channel = Mix_PlayChannel(-1, sound, 1);
  assert(channel == 0);

  emscripten_run_script("setTimeout(Module['_play2'], 500)");
  return channel;
}

void done(int channel) {
  assert(channel == 1);

  int result = 1;
  REPORT_RESULT();
}

int play2() {
  Mix_ChannelFinished(done);

  int channel2 = Mix_PlayChannel(-1, sound2, 1);
  assert(channel2 == 1);
  return channel2;
}

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_AUDIO);

  int ret = Mix_OpenAudio(0, 0, 0, 0); // we ignore all these..
  assert(ret == 0);

  sound = Mix_LoadWAV("sound.ogg");
  assert(sound);
  sound2 = Mix_LoadWAV("sound2.wav");
  assert(sound);

  int channel = play();
  printf( "Pausing Channel %d", channel );
  Mix_Pause(channel);
  int paused = Mix_Paused(channel);
  printf( "Channel %d %s", channel, paused ? "is paused" : "is NOT paused" );
  assert(paused);
  Mix_Resume(channel);
  paused = Mix_Paused(channel);
  printf( "Channel %d %s", channel, paused ? "is paused" : "is NOT paused" );
  assert(paused == 0);

  if (argc == 12121) play2(); // keep it alive

  emscripten_run_script("element = document.createElement('input');"
                        "element.setAttribute('type', 'button');"
                        "element.setAttribute('value', 'replay!');"
                        "element.setAttribute('onclick', 'Module[\"_play\"]()');"
                        "document.body.appendChild(element);");

  printf("you should hear two sounds. press the button to replay!\n");

  return 0;
}

