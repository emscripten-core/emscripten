/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <assert.h>
#include <sys/stat.h>

#include <emscripten/emscripten.h>
#include <emscripten/eventloop.h>

Mix_Chunk *sound1, *sound2, *sound3;
Mix_Music * music;
void play2(void*);

EMSCRIPTEN_KEEPALIVE int play() {
  int channel = Mix_PlayChannel(-1, sound1, 0);
  printf("playing sound1 -> %d\n", channel);
  assert(channel >= 0);

  emscripten_set_timeout(play2, 500, NULL);
  return channel;
}

void done(int channel) {
  printf("done channel: %d\n", channel);
}

EMSCRIPTEN_KEEPALIVE void success(int channel) {
  emscripten_force_exit(0);
}

EMSCRIPTEN_KEEPALIVE void play2(void* arg) {
  int channel2 = Mix_PlayChannel(-1, sound2, 0);
  printf("playing sound2 -> %d\n", channel2);
  assert(channel2 >= 0);

  int channel3 = Mix_PlayChannel(-1, sound3, 0);
  printf("playing sound3 -> %d\n", channel3);
  assert(channel2 >= 0);
}

EMSCRIPTEN_KEEPALIVE void play_music() {
  int channel = Mix_PlayMusic(music, 1);
  printf("playing music -> %d\n", channel);
  assert(channel >= 0);
}

int main(int argc, char **argv) {
  printf("in main\n");
  SDL_Init(SDL_INIT_AUDIO);

  int ret = Mix_OpenAudio(0, 0, 0, 0); // we ignore all these..

  assert(ret == 0);

  Mix_ChannelFinished(done);

  {
      printf("loading sound.ogg (sound1)\n");
      SDL_RWops * ops = SDL_RWFromFile("sound.ogg", "r");
      sound1 = Mix_LoadWAV_RW(ops, 0);
      SDL_FreeRW(ops);
      assert(sound1);
  }

  {
    printf("loading sound2.wav (sound2)\n");
    sound2 = Mix_LoadWAV("sound2.wav");
    assert(sound2);
  }

  {
      printf("loading noise.ogg (sound3)\n");
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

  int channel = play();
  printf("Pausing Channel %d\n", channel);
  Mix_Pause(channel);
  int paused = Mix_Paused(channel);
  printf("Channel %d %s\n", channel, paused ? "is paused" : "is NOT paused");
  assert(paused);
  Mix_Resume(channel);
  paused = Mix_Paused(channel);
  printf("Channel %d %s\n", channel, paused ? "is paused" : "is NOT paused");
  assert(paused == 0);

  EM_ASM(
      var element = document.createElement('input');
      element.setAttribute('type', 'button');
      element.setAttribute('value', 'replay!');
      element.setAttribute('onclick', '_play()');
      document.body.appendChild(element);

      var element = document.createElement('input');
      element.setAttribute('type', 'button');
      element.setAttribute('value', 'Play music');
      element.setAttribute('onclick', '_play_music()');
      document.body.appendChild(element);

      var element = document.createElement('input');
      element.setAttribute('type', 'button');
      element.setAttribute('value', 'Success');
      element.setAttribute('onclick', '_success()');
      document.body.appendChild(element);
  );

  printf("you should hear two sounds. press the button to replay!\n");
  emscripten_runtime_keepalive_push();

  return 0;
}
