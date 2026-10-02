/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <emscripten.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
MIX_Audio *audio = NULL;
MIX_Track *track = NULL;
MIX_Mixer *mixer = NULL;

#define WIDTH 640
#define HEIGHT 480

#ifndef SOUND_PATH
#error "must define SOUND_PATH"
#endif

void sound_loop_then_quit() {
  if (MIX_TrackPlaying(track))
    return;

  MIX_DestroyAudio(audio);
  MIX_DestroyTrack(track);
  MIX_DestroyMixer(mixer);

  emscripten_cancel_main_loop();
  printf("Shutting down\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

  if (!MIX_Init()) {
    printf("MIX_Init failed: %s\n", SDL_GetError());
    return 1;
  }

  if (!SDL_CreateWindowAndRenderer("SDL3 MIXER", WIDTH, HEIGHT, 0, &window, &renderer)) {
    printf("SDL_CreateWindowAndRenderer: %s\n", SDL_GetError());
    return 1;
  }

  mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
  if (!mixer) {
    printf("Couldn't create mixer on default device: %s", SDL_GetError());
    return 1;
  }

  audio = MIX_LoadAudio(mixer, SOUND_PATH, false);
  if (!audio) {
    printf("MIX_LoadAudio: %s\n", SDL_GetError());
    return 1;
  }

  track = MIX_CreateTrack(mixer);
  if (!track) {
    printf("MIX_CreateTrack: %s\n", SDL_GetError());
    return 1;
  }

  MIX_SetTrackAudio(track, audio);
  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, 0);

  printf("Starting sound play loop\n");
  MIX_PlayTrack(track, props);

  emscripten_set_main_loop(sound_loop_then_quit, 0, 1);

  return 0;
}
