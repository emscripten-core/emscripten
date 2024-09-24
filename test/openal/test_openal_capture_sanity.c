/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// This test attempts to open all possible capture devices, each one
// several times with a combination of "reasonable" parameters, and
// checks some basic conformance to expectations w.r.t the spec.
//
// Wishlist:
// - Any operation a closed device should fail; 
// - Trying to open multiple devices with the same name at the same time
//   and different settings should be fine;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define ASSUME_AL_FLOAT32
#endif
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#define countof(x) (sizeof(x)/sizeof((x)[0]))

// OfflineAudioContexts are required to support sample rates ranging
// from 22050 to 96000.
// Should we test for purposefully-ugly sample rates within that range, too ?
static const ALCuint SAMPLE_RATES[] = {
  22050, 32000, 37800, 44100, 48000, 88200, 96000
};

static const ALenum FORMATS[] = {
  AL_FORMAT_MONO8, 
  AL_FORMAT_MONO16,
  AL_FORMAT_STEREO8,
  AL_FORMAT_STEREO16,
#ifdef ASSUME_AL_FLOAT32
  AL_FORMAT_MONO_FLOAT32,
  AL_FORMAT_STEREO_FLOAT32,
#endif
};

static const char* alformat_string(ALenum format) {
  switch(format) {
#define CASE(X) case X: return #X;
  CASE(AL_FORMAT_MONO8)
  CASE(AL_FORMAT_MONO16)
  CASE(AL_FORMAT_STEREO8)
  CASE(AL_FORMAT_STEREO16)
#ifdef ASSUME_AL_FLOAT32
  CASE(AL_FORMAT_MONO_FLOAT32)
  CASE(AL_FORMAT_STEREO_FLOAT32)
#endif
#undef CASE
  }
  return "<no_string_available>";
}

static void check_device_sanity_with_params(const char* name,
                                            ALCuint sample_rate,
                                            ALenum format,
                                            ALCsizei buffer_size) {
  printf(
    "Testing \"%s\" @%uHz with %u sample-frames (format: %s)...\n", 
    name, (unsigned)sample_rate, (unsigned) buffer_size,
    alformat_string(format)
  );

  ALCdevice* dev = alcCaptureOpenDevice(name, sample_rate, format, buffer_size);

  if (dev) {
    const char *claimed = alcGetString(dev, ALC_CAPTURE_DEVICE_SPECIFIER);
    if (strcmp(name, claimed)) {
      fprintf(stderr, "The device \"%s\" claims to be actually named \"%s\", which is not correct behavior.\n", name, claimed);
      exit(1);
    }
    ALCboolean could_close = alcCaptureCloseDevice(dev);
    if (!could_close) {
      fprintf(stderr, "alcCaptureCloseDevice() with \"%s\" failed!\n", name);
      exit(1);
    }
    return;
  }

  ALCenum err = alcGetError(dev);
  fprintf(stderr,
    "alcCaptureOpenDevice(\"%s\", sample_rate=%u, format=%s, "
    "buffer_size=%u) failed with ALC error %x (%s)\n", 
    name, (unsigned)sample_rate, alformat_string(format), 
    (unsigned) buffer_size,
    (unsigned) err, alcGetString(NULL, err)
  );
  exit(1);
}

static void check_device_sanity(const char *name) {
  for (int si=0 ; si<countof(SAMPLE_RATES) ; ++si) {
    for (int fi=0 ; fi<countof(FORMATS) ; ++fi) {
      // 8 seconds of data
      check_device_sanity_with_params(
        name, SAMPLE_RATES[si], FORMATS[fi], 8*SAMPLE_RATES[si]
      );
    }
  }
}

static bool is_defaultname_in_names(const char *dft, const char *names) {
  for (const char *name = names; *name ; name += 1+strlen(name)) {
    if (!strcmp(dft, name)) {
      return true;
    }
  }
  return false;
}

int main() {
  const char *dft = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
  const char *names = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

  if (!is_defaultname_in_names(dft, names)) {
    printf(
      "The default device specifier (\"%s\") was not found amongst the device specifier list : ...\n"
      , dft
    );
    for (const char *name = names; *name ; name += 1+strlen(name)) {
      printf("- \"%s\";\n", name);
    }
    printf("... this is not an error, though.\n\n");
  }

  for (const char *name = names; *name ; name += 1+strlen(name)) {
    check_device_sanity(name);
  }

  return 0;
}
