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
#include <AL/alext.h>

ALCdevice* device = NULL;
ALCcontext* context = NULL;

void test_offsets_with_zero_buffer(void) {
  // Test AL_SAMPLE_OFFSET and AL_BYTE_OFFSET when buffer 0 is queued before a real buffer
  ALuint buf = 0;
  alGenBuffers(1, &buf);
  char dummy_data[2000] = {0};
  alBufferData(buf, AL_FORMAT_MONO16, dummy_data, sizeof(dummy_data), 44100);

  ALuint src = 0;
  alGenSources(1, &src);

  ALuint bufs[2] = {0, buf};
  alSourceQueueBuffers(src, 2, bufs);
  assert(alGetError() == AL_NO_ERROR);

  alSourcei(src, AL_SAMPLE_OFFSET, 100);
  assert(alGetError() == AL_NO_ERROR);

  alSourcei(src, AL_BYTE_OFFSET, 200);
  assert(alGetError() == AL_NO_ERROR);

  alDeleteSources(1, &src);
  alDeleteBuffers(1, &buf);
}

void test_stopped_seek(void) {
  ALuint buf = 0;
  alGenBuffers(1, &buf);
  short dummy_data[2000] = {0};
  alBufferData(buf, AL_FORMAT_MONO16, dummy_data, sizeof(dummy_data), 44100);
  assert(alGetError() == AL_NO_ERROR);

  ALuint src = 0;
  alGenSources(1, &src);
  assert(alGetError() == AL_NO_ERROR);

  alSourcei(src, AL_BUFFER, buf);
  assert(alGetError() == AL_NO_ERROR);

  alSourcePlay(src);
  assert(alGetError() == AL_NO_ERROR);

  alSourceStop(src);
  assert(alGetError() == AL_NO_ERROR);

  // See of 0.01 seconds while the stream in stopped.
  alSourcef(src, AL_SEC_OFFSET, 0.01f);
  assert(alGetError() == AL_NO_ERROR);

  // Verify the the seek was successful
  float offset = 0.0f;
  alGetSourcef(src, AL_SEC_OFFSET, &offset);
  assert(offset == 0.01f);
  offset = 0.0f;
  alGetSourcefv(src, AL_SEC_OFFSET, &offset);
  assert(offset == 0.01f);

  alDeleteSources(1, &src);
  alDeleteBuffers(1, &buf);
}

void test_buffer_and_source_validation(void) {
  ALuint buf = 0;
  alGenBuffers(1, &buf);
  assert(alIsBuffer(buf) == AL_TRUE);
  assert(alIsBuffer(9999) == AL_FALSE);

  short dummy_data[1000] = {0};
  alBufferData(buf, AL_FORMAT_MONO16, dummy_data, sizeof(dummy_data), 44100);
  assert(alGetError() == AL_NO_ERROR);

  // Default AL_LOOP_POINTS_SOFT should be [0, sample_length]
  ALint loop_pts[2] = {-1, -1};
  alGetBufferiv(buf, AL_LOOP_POINTS_SOFT, loop_pts);
  assert(alGetError() == AL_NO_ERROR);
  assert(loop_pts[0] == 0);
  assert(loop_pts[1] == 1000);

  // Out-of-bounds loop end point should fail with AL_INVALID_VALUE
  ALint bad_pts[2] = {0, 1001};
  alBufferiv(buf, AL_LOOP_POINTS_SOFT, bad_pts);
  assert(alGetError() == AL_INVALID_VALUE);

  ALint valid_pts[2] = {100, 500};
  alBufferiv(buf, AL_LOOP_POINTS_SOFT, valid_pts);
  assert(alGetError() == AL_NO_ERROR);
  alGetBufferiv(buf, AL_LOOP_POINTS_SOFT, loop_pts);
  assert(loop_pts[0] == 100);
  assert(loop_pts[1] == 500);

  // Queueing a buffer with a mismatched format should fail with
  // AL_INVALID_OPERATION and not queue the incompatible buffer.
  ALuint stereo_buf = 0;
  alGenBuffers(1, &stereo_buf);
  alBufferData(stereo_buf, AL_FORMAT_STEREO16, dummy_data, sizeof(dummy_data), 44100);
  assert(alGetError() == AL_NO_ERROR);

  ALuint src = 0;
  alGenSources(1, &src);
  alSourceQueueBuffers(src, 1, &buf);
  assert(alGetError() == AL_NO_ERROR);
  alSourceQueueBuffers(src, 1, &stereo_buf);
  assert(alGetError() == AL_INVALID_OPERATION);
  ALint queued = 0;
  alGetSourcei(src, AL_BUFFERS_QUEUED, &queued);
  assert(queued == 1);

  alDeleteSources(1, &src);
  alDeleteBuffers(1, &stereo_buf);
  alDeleteBuffers(1, &buf);
}

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

  // Destroying the current or an invalid context should fail with ALC_INVALID_CONTEXT
  alcDestroyContext(context);
  assert(alcGetError(device) == ALC_INVALID_CONTEXT);
  alcDestroyContext(NULL);
  assert(alcGetError(device) == ALC_INVALID_CONTEXT);
  alcDestroyContext((ALCcontext*)9999);
  assert(alcGetError(device) == ALC_INVALID_CONTEXT);

  // ALC_MAX_AUXILIARY_SENDS should succeed without falling through
  value = 0;
  alcGetIntegerv(device, 0x20003 /* ALC_MAX_AUXILIARY_SENDS */, 1, &value);
  assert(alcGetError(device) == ALC_NO_ERROR);
  assert(value == 1);

  // Request an invalid enum to generate an AL error
  alGetFloat(0);
  assert(alGetError() == AL_INVALID_ENUM);
  // Check that the error is reset after reading it.
  assert(alGetError() == AL_NO_ERROR);

  assert(alIsEnabled(AL_SOURCE_DISTANCE_MODEL) == AL_FALSE);
  alEnable(AL_SOURCE_DISTANCE_MODEL);
  assert(alIsEnabled(AL_SOURCE_DISTANCE_MODEL) == AL_TRUE);
  alDisable(AL_SOURCE_DISTANCE_MODEL);
  assert(alIsEnabled(AL_SOURCE_DISTANCE_MODEL) == AL_FALSE);

  test_offsets_with_zero_buffer();
  test_stopped_seek();
  test_buffer_and_source_validation();

  ret = alcMakeContextCurrent(NULL);
  assert(ret == ALC_TRUE);

  alcDestroyContext(context);
  alcCloseDevice(device);
  return 0;
}
