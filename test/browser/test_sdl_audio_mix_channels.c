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

static const int kNumChannels = 40;

static int loadAndPlay() {
  return Mix_PlayChannel(-1, sound, -1);
}

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_AUDIO);
  Mix_Init(MIX_INIT_OGG);

  int ret = Mix_OpenAudio(0, 0, 0, 0); // we ignore all these..
  assert(ret == 0);

  Mix_AllocateChannels(kNumChannels);

  sound = Mix_LoadWAV("sound.ogg");

  // allocate all the channels
  for (int i = 0; i < kNumChannels; i++) {
    assert(loadAndPlay() != -1);
  }

  // This point, we should have exhausted our channels
  int lastChannel = loadAndPlay();
  assert(lastChannel == -1);

  // force a quit
  while(Mix_Init(0))
    Mix_Quit();
  Mix_CloseAudio();

  return 0;
}

