// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifndef EMSCRIPTEN_KEEPALIVE
#define EMSCRIPTEN_KEEPALIVE
#endif

void EMSCRIPTEN_KEEPALIVE test_finished() {
#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
}

#if defined(TEST_ALC_SOFT_PAUSE_DEVICE)
  typedef void (*ALC_DEVICE_PAUSE_SOFT)(ALCdevice *);
  typedef void (*ALC_DEVICE_RESUME_SOFT)(ALCdevice *);

  ALC_DEVICE_PAUSE_SOFT alcDevicePauseSOFT;
  ALC_DEVICE_RESUME_SOFT alcDeviceResumeSOFT;
#endif

void playSource(void* arg) {
  ALuint source = (ALuint)((intptr_t)(arg));
  ALint state;

  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PLAYING);

  alSourceRewind(source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_INITIAL);
  alSourcePlay(source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PLAYING);
  alSourcePause(source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PAUSED);
#ifndef TEST_LOOPED_PLAYBACK
  alSourceStop(source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_STOPPED);
#endif

  alSourceRewindv(1, &source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_INITIAL);
  alSourcePlayv(1, &source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PLAYING);
  alSourcePausev(1, &source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PAUSED);
#ifndef TEST_LOOPED_PLAYBACK
  alSourceStopv(1, &source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_STOPPED);
  test_finished();
#endif
}

void main_tick(void *arg) {
  ALuint source = (ALuint)((intptr_t)(arg));
  double t = emscripten_get_now() * 0.001;

#if defined(TEST_LOOPED_SEEK_PLAYBACK)
  int offset = 0;
  alGetSourcei(source, AL_SAMPLE_OFFSET, &offset);
  if (offset < 44100 * 3 / 2) {
    alSourcei(source, AL_SAMPLE_OFFSET, 44100 * 3 / 2);
  }
#elif defined(TEST_ANIMATED_LOOPED_PITCHED_PLAYBACK)
  double pitch = sin(t) * 0.5 + 1.0;
  alSourcef(source, AL_PITCH, pitch);
#elif defined(TEST_ANIMATED_LOOPED_DISTANCE_PLAYBACK)
  double pos = (sin(t) - 1.0) * 100.0;
  ALfloat listenerPos[] = {0.0, 0.0, pos};
  alListenerfv(AL_POSITION, listenerPos);
#elif defined(TEST_ANIMATED_LOOPED_DOPPLER_PLAYBACK)
  double vel = sin(t) * (343.3 / 2.0);
  ALfloat listenerVel[] = {0.0, 0.0, vel};
  alListenerfv(AL_VELOCITY, listenerVel);
#elif defined(TEST_ANIMATED_LOOPED_PANNED_PLAYBACK) || defined(TEST_ANIMATED_LOOPED_RELATIVE_PLAYBACK) || defined(TEST_AL_SOFT_SOURCE_SPATIALIZE)
  ALfloat listenerPos[] = {cosf(t), 0.0, sinf(t)};
  alListenerfv(AL_POSITION, listenerPos);
#elif defined(TEST_ALC_SOFT_PAUSE_DEVICE)
  ALCcontext *ctx = alcGetCurrentContext();
  ALCdevice *dev = alcGetContextsDevice(ctx);
  if (fmod(t, 2.0) < 1.0) {
    alcDeviceResumeSOFT(dev);
  } else {
    alcDevicePauseSOFT(dev);
  }
#endif
}

int main() {
  int major, minor;
  alcGetIntegerv(NULL, ALC_MAJOR_VERSION, 1, &major);
  alcGetIntegerv(NULL, ALC_MINOR_VERSION, 1, &minor);

  assert(major == 1);

  printf("ALC version: %i.%i\n", major, minor);
  printf("Default device: %s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));

  ALCdevice* device = alcOpenDevice(NULL);
  assert(alcIsExtensionPresent(device, "ALC_SOFT_HRTF") == AL_TRUE);
#if defined(TEST_ANIMATED_LOOPED_PANNED_PLAYBACK)
  ALCint attrs[] = {ALC_HRTF_SOFT, ALC_TRUE, ALC_HRTF_ID_SOFT, 0, 0};
  ALCcontext* context = alcCreateContext(device, attrs);
#else
  ALCcontext* context = alcCreateContext(device, NULL);
#endif
  alcMakeContextCurrent(context);

  assert(alGetString(AL_VERSION));

  printf("OpenAL version: %s\n", alGetString(AL_VERSION));
  printf("OpenAL vendor: %s\n", alGetString(AL_VENDOR));
  printf("OpenAL renderer: %s\n", alGetString(AL_RENDERER));

  ALfloat listenerPos[] = {0.0, 0.0, 1.0};
  ALfloat listenerVel[] = {0.0, 0.0, 0.0};
  ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

  alListenerfv(AL_POSITION, listenerPos);
  alListenerfv(AL_VELOCITY, listenerVel);
  alListenerfv(AL_ORIENTATION, listenerOri);

  // check getting and setting global gain
  ALfloat volume;
  alGetListenerf(AL_GAIN, &volume);
  assert(volume == 1.0);
  alListenerf(AL_GAIN, 0.0);
  alGetListenerf(AL_GAIN, &volume);
  assert(volume == 0.0);

  alListenerf(AL_GAIN, 1.0); // reset gain to default

  ALuint buffers[1];

  alGenBuffers(1, buffers);

#ifdef __EMSCRIPTEN__
  FILE* source = fopen("audio.wav", "rb");
#else
  FILE* source = fopen("sounds/audio.wav", "rb");
#endif
  fseek(source, 0, SEEK_END);
  int size = ftell(source);
  fseek(source, 0, SEEK_SET);

  unsigned char* buffer = (unsigned char*) malloc(size);
  fread(buffer, size, 1, source);

  unsigned offset = 12; // ignore the RIFF header
  offset += 8; // ignore the fmt header
  offset += 2; // ignore the format type

  unsigned channels = buffer[offset + 1] << 8;
  channels |= buffer[offset];
  offset += 2;
  printf("Channels: %u\n", channels);

  unsigned frequency = buffer[offset + 3] << 24;
  frequency |= buffer[offset + 2] << 16;
  frequency |= buffer[offset + 1] << 8;
  frequency |= buffer[offset];
  offset += 4;
  printf("Frequency: %u\n", frequency);

  offset += 6; // ignore block size and bps

  unsigned bits = buffer[offset + 1] << 8;
  bits |= buffer[offset];
  offset += 2;
  printf("Bits: %u\n", bits);

  ALenum format = 0;
  if(bits == 8)
  {
    if(channels == 1)
      format = AL_FORMAT_MONO8;
    else if(channels == 2)
      format = AL_FORMAT_STEREO8;
  }
  else if(bits == 16)
  {
    if(channels == 1)
      format = AL_FORMAT_MONO16;
    else if(channels == 2)
      format = AL_FORMAT_STEREO16;
  }

  offset += 8; // ignore the data chunk

  printf("Start offset: %d\n", offset);

  alBufferData(buffers[0], format, &buffer[offset], size - offset, frequency);

#if defined(TEST_AL_SOFT_LOOP_POINTS)
  ALint loopPoints[] = {44100, 44100 * 2};
  ALint alLoopPointsSoft = alGetEnumValue("AL_LOOP_POINTS_SOFT");
  alBufferiv(buffers[0], alLoopPointsSoft, loopPoints);
#endif

  ALint val;
  alGetBufferi(buffers[0], AL_FREQUENCY, &val);
  assert(val == frequency);
  alGetBufferi(buffers[0], AL_SIZE, &val);
  assert(val == size - offset);
  alGetBufferi(buffers[0], AL_BITS, &val);
  assert(val == bits);
  alGetBufferi(buffers[0], AL_CHANNELS, &val);
  assert(val == channels);

  ALuint sources[1];
  alGenSources(1, sources);

  assert(alIsSource(sources[0]));

  alSourcei(sources[0], AL_BUFFER, buffers[0]);

  ALint state;
  alGetSourcei(sources[0], AL_SOURCE_STATE, &state);
  assert(state == AL_INITIAL);

  alSourcePlay(sources[0]);

  alGetSourcei(sources[0], AL_SOURCE_STATE, &state);
  assert(state == AL_PLAYING);

#ifdef TEST_LOOPED_PLAYBACK
  alSourcei(sources[0], AL_LOOPING, AL_TRUE);
#if defined(TEST_LOOPED_SEEK_PLAYBACK)
  printf("You should hear a continuously looping ~1.5 second half of a clip of the 1902 piano song \"The Entertainer\". If you hear a full 3 second clip, the test has failed. Press OK when confirmed.\n");
#elif defined(TEST_ANIMATED_LOOPED_PITCHED_PLAYBACK)
  printf("You should hear a continuously looping clip of the 1902 piano song \"The Entertainer\" played back at a dynamic playback rate that smoothly varies its pitch according to a sine wave. Press OK when confirmed.\n");
#elif defined(TEST_ANIMATED_LOOPED_DISTANCE_PLAYBACK)
  alSourcef(sources[0], AL_REFERENCE_DISTANCE, 25.0);
  printf("You should hear a continuously looping clip of the 1902 piano song \"The Entertainer\" fade in and out. Press OK when confirmed.\n");
#elif defined(TEST_ANIMATED_LOOPED_DOPPLER_PLAYBACK)
  printf("You should hear a continuously looping clip of the 1902 piano song \"The Entertainer\" played back at a dynamic playback rate that smoothly varies its pitch according to a sine wave doppler shift. Press OK when confirmed.\n");
#elif defined(TEST_ANIMATED_LOOPED_PANNED_PLAYBACK)
  assert(!alIsEnabled(AL_SOURCE_DISTANCE_MODEL));
  alDisable(AL_SOURCE_DISTANCE_MODEL);
  assert(!alIsEnabled(AL_SOURCE_DISTANCE_MODEL));
  alEnable(AL_SOURCE_DISTANCE_MODEL);
  assert(alIsEnabled(AL_SOURCE_DISTANCE_MODEL));
  printf("You should hear a continuously looping clip of the 1902 piano song \"The Entertainer\" smoothly panning around the listener. Press OK when confirmed.\n");
#elif defined(TEST_ANIMATED_LOOPED_RELATIVE_PLAYBACK)
  alSourcei(sources[0], AL_SOURCE_RELATIVE, AL_TRUE);
  printf("You should hear a continuously looping clip of the 1902 piano song \"The Entertainer\" centered at the listener. If it is panning, then the test failed. Press OK when confirmed.\n");
#elif defined(TEST_ALC_SOFT_PAUSE_DEVICE)
  alcDevicePauseSOFT = (ALC_DEVICE_PAUSE_SOFT)(alcGetProcAddress(device, "alcDevicePauseSOFT"));
  alcDeviceResumeSOFT = (ALC_DEVICE_RESUME_SOFT)(alcGetProcAddress(device, "alcDeviceResumeSOFT"));
  assert(alcDevicePauseSOFT && alcDeviceResumeSOFT);
  printf("You should hear a looping clip of the 1902 piano song \"The Entertainer\" That pauses for 1 second every second. Press OK when confirmed.\n");
#elif defined(TEST_AL_SOFT_LOOP_POINTS)
  printf("You should hear a clip of the 1902 piano song \"The Entertainer\" start normally, then begin looping the same 3 notes repeatedly. If you hear the entire clip, then the test failed. Press OK when confirmed.\n");
#elif defined(TEST_AL_SOFT_SOURCE_SPATIALIZE)
  alSourcei(sources[0], 0x1214 /* AL_SOURCE_SPATIALIZE_SOFT */, AL_FALSE);
  printf("You should hear a continuously looping clip of the 1902 piano song \"The Entertainer\" centered at the listener. If it is panning, then the test failed. Press OK when confirmed.\n");
#else
  alSourcef(sources[0], AL_PITCH, 1.5f);
  printf("You should hear a continuously looping clip of the 1902 piano song \"The Entertainer\" played back at a high playback rate (high pitch). Press OK when confirmed.\n");
#endif
  EM_ASM(
    var btn = document.createElement('input');
    btn.type = 'button';
    btn.name = btn.value = 'OK';
    btn.onclick = function() {
      _test_finished();
    };
    document.body.appendChild(btn);
  );
#else
  printf("You should hear a short audio clip playing back.\n");
#endif

#ifdef __EMSCRIPTEN__

#if defined(TEST_LOOPED_PLAYBACK)
  emscripten_set_main_loop_arg(main_tick, (void*)sources[0], 0, 0);
#else
  emscripten_async_call(playSource, (void*)(sources[0]), 700);
#endif
#else
  usleep(700000);
  playSource((void*)(sources[0]));
#endif

  return 0;
}
