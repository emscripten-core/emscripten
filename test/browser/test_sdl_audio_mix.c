/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <assert.h>
#include <emscripten.h>

static Mix_Chunk *sound = NULL;
static Mix_Chunk *noiseLoop = NULL;
static Mix_Music *music = NULL;

static int soundChannel = 0;
static int noiseLoopChannel = 0;

void one_iter() {
  static int frames = 0;
  frames++;

  switch( frames ) {
    case 1:
      soundChannel = Mix_PlayChannel(-1, sound, 0);
      printf("channel = %d", soundChannel);
      assert(soundChannel != -1 && soundChannel != 0);

      noiseLoopChannel = Mix_PlayChannel(-1, noiseLoop, -1);
      printf("noiseLoopChannel = %d", noiseLoopChannel);
      assert(noiseLoopChannel != -1 && noiseLoopChannel != 0);
      // set noiseLoopChannel to half volume
      Mix_Volume(noiseLoopChannel,MIX_MAX_VOLUME/10);
      break;
    case 2:
      printf("channel %d is playing = %d", soundChannel, Mix_Playing(soundChannel));
      assert(Mix_Playing(soundChannel));
      break;
    case 30:
      Mix_Pause(soundChannel);
      Mix_PlayMusic(music, 1);
      break;
    case 31:
      assert(Mix_Paused(soundChannel));
      assert(Mix_PlayingMusic());
      break;
    case 60:
      Mix_Resume(soundChannel);
      Mix_PauseMusic();
      break;
    case 61:
      assert(Mix_Playing(soundChannel));
      assert(Mix_PausedMusic());
      break;
    case 90:
      Mix_ResumeMusic();
      break;
    case 91:
      assert(Mix_PlayingMusic());
      break;
    case 120:
      Mix_HaltChannel(soundChannel);
      Mix_HaltMusic();
      emscripten_force_exit(0);
      break;
  };
}


int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_AUDIO);
  Mix_Init(MIX_INIT_OGG);

  // This reserves channel 0 for other purposes.
  // We are just going to verify that we are not
  // allocated channel 0 when we call Mix_PlayChannel(-1, ...)
  Mix_ReserveChannels(1);

  int ret = Mix_OpenAudio(0, 0, 0, 0); // we ignore all these..
  assert(ret == 0);

  sound = Mix_LoadWAV("sound.ogg");
  assert(sound);
  noiseLoop = Mix_LoadWAV("noise.ogg");
  assert(noiseLoop);

  music = Mix_LoadMUS("music.ogg");
  assert(music);
  emscripten_set_main_loop(one_iter, 30, 0);

  // force a quit
  while(Mix_Init(0))
    Mix_Quit();
  Mix_CloseAudio();

  return 99;
}

