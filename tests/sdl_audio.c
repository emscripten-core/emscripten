#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <assert.h>
#include <emscripten.h>
#include <sys/stat.h>

Mix_Chunk *sound, *sound2, *sound3;
Mix_Music * music;
int play2();

int play() {
  int channel = Mix_PlayChannel(-1, sound, 0);
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

  int channel2 = Mix_PlayChannel(-1, sound2, 0);
  assert(channel2 == 1);
  int channel3 = Mix_PlayChannel(-1, sound3, 0);
  assert(channel3 == 2);
  assert(Mix_PlayMusic(music, 1) == 0);
  return channel2;
}

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_AUDIO);

  int ret = Mix_OpenAudio(0, 0, 0, 0); // we ignore all these..
  assert(ret == 0);

  sound = Mix_LoadWAV("sound.ogg");
  assert(sound);

  {
      struct stat info;
      int result = stat("noise.ogg", &info);
      char * bytes = malloc( info.st_size );
      FILE * f = fopen( "noise.ogg", "rb" );
      fread( bytes, 1, info.st_size, f  );
      fclose(f);

      SDL_RWops * ops = SDL_RWFromConstMem(bytes, info.st_size);
      sound3 = Mix_LoadWAV_RW(ops, 0);
      SDL_FreeRW(ops);
      free(bytes);
  }

  {
      music = Mix_LoadMUS("the_entertainer.ogg");
  }
  
  
  sound2 = Mix_LoadWAV("sound2.wav");
  assert(sound2);

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
