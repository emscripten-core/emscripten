/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <AL/al.h>
#include <AL/alc.h>

ALCdevice* device = NULL;
ALCcontext* context = NULL;

int main(int argc, char* argv[]) {
  ALCboolean ret;

  //
  // Setup the AL context.
  //
  device = alcOpenDevice(NULL);

  // Request an invalid enum to generate an ALC error
  int value = 0;
  alcGetIntegerv(device, 0, 1, &value);
  assert(alcGetError(device) == ALC_INVALID_ENUM);
  // Check that the error is reset after reading it.
  assert(alcGetError(device) == ALC_NO_ERROR);

  context = alcCreateContext(device, NULL);
  ret = alcMakeContextCurrent(context);
  assert(ret == ALC_TRUE);

  // Request an invalid enum to generate an AL error
  alGetFloat(0);
  assert(alGetError() == AL_INVALID_ENUM);
  // Check that the error is reset after reading it.
  assert(alGetError() == AL_NO_ERROR);

  ret = alcMakeContextCurrent(NULL);
  assert(ret == ALC_TRUE);

  alcDestroyContext(context);
  alcCloseDevice(device);
  return 0;
}

