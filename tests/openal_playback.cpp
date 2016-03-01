#include <stdio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
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

extern "C"
{
void EMSCRIPTEN_KEEPALIVE test_finished()
{
#ifdef REPORT_RESULT
  int result = 1;
  REPORT_RESULT();
#endif
}
}

void playSource(void* arg)
{
  ALuint source = static_cast<ALuint>(reinterpret_cast<intptr_t>(arg));
  ALint state;

  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PLAYING);
  alSourcePause(source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PAUSED);
  alSourcePlay(source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PLAYING);
#ifndef TEST_LOOPED_PLAYBACK
  alSourceStop(source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_STOPPED);
  test_finished();
#endif
}

void main_tick(void *arg)
{
  ALuint source = static_cast<ALuint>(reinterpret_cast<intptr_t>(arg));

  double t = emscripten_get_now() * 0.001;
  double pitch = sin(t) * 0.5 + 1.0;
  alSourcef(source, AL_PITCH, pitch);
}

int main() {
  int major, minor;
  alcGetIntegerv(NULL, ALC_MAJOR_VERSION, 1, &major);
  alcGetIntegerv(NULL, ALC_MAJOR_VERSION, 1, &minor);

  assert(major == 1);

  printf("ALC version: %i.%i\n", major, minor);
  printf("Default device: %s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));

  ALCdevice* device = alcOpenDevice(NULL);
  ALCcontext* context = alcCreateContext(device, NULL);
  alcMakeContextCurrent(context);

  assert(alGetString(AL_VERSION));

  printf("OpenAL version: %s\n", alGetString(AL_VERSION));
  printf("OpenAL vendor: %s\n", alGetString(AL_VENDOR));
  printf("OpenAL renderer: %s\n", alGetString(AL_RENDERER));

  ALfloat listenerPos[] = {0.0, 0.0, 0.0};
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
  alSourcef(sources[0], AL_PITCH, 1.5f);
#ifdef TEST_ANIMATED_LOOPED_PITCHED_PLAYBACK
  printf("You should hear a continuously looping clip of the 1902 piano song \"The Entertainer\" played back at a dynamic playback rate that smoothly varies its pitch according to a sine wave. Press OK when confirmed.\n");
#else
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

#ifdef TEST_ANIMATED_LOOPED_PITCHED_PLAYBACK
  emscripten_set_main_loop_arg(main_tick, (void*)buffers[0], 0, 0);
#else
  emscripten_async_call(playSource, reinterpret_cast<void*>(sources[0]), 700);
#endif
#else
  usleep(700000);
  playSource(reinterpret_cast<void*>(sources[0]));
#endif

  return 0;
}
