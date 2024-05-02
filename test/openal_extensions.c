/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// So far just does some sanity checks for the available extensions.
//
// In the future, could also try to enable different extensions and test for
// output correctness.

#include <stdio.h>
#include <stdlib.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

static int result = EXIT_SUCCESS;

static void end_test() {
#ifdef __EMSCRIPTEN__
  REPORT_RESULT(result);
#endif
  exit(result);
}

#define NUM_ALC_EXTENSIONS 2
static const ALCchar *alc_extensions[NUM_ALC_EXTENSIONS] = {
  "ALC_SOFT_pause_device",
  "ALC_SOFT_HRTF",
};

#define NUM_AL_EXTENSIONS 5
static const ALCchar *al_extensions[NUM_AL_EXTENSIONS] = {
  "AL_EXT_float32",
  "AL_SOFT_loop_points",
  "AL_SOFT_source_length",
  "AL_EXT_source_distance_model",
  "AL_SOFT_source_spatialize",
};

static void check_alc_extension(const ALCchar *extension) {
  ALCdevice *device = alcOpenDevice(NULL);
  if (!device) {
    fprintf(stderr, "Failed to open default device.");
    result = EXIT_FAILURE;
    return;
  }

  if (alcIsExtensionPresent(device, extension) != ALC_TRUE) {
    fprintf(stderr, "Extension %s was not present.", extension);
    result = EXIT_FAILURE;
  }
}

static void check_al_extension(const ALchar *extension) {
  if (alIsExtensionPresent(extension) != ALC_TRUE) {
    fprintf(stderr, "Extension %s was not present.", extension);
    result = EXIT_FAILURE;
  }
}

int main() {

  for (int i = 0; i < NUM_ALC_EXTENSIONS; i++) {
    check_alc_extension(alc_extensions[i]);
  }

  for (int i = 0; i < NUM_AL_EXTENSIONS; i++) {
    check_al_extension(al_extensions[i]);
  }

  end_test();
}
