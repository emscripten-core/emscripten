#include <stdio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <assert.h>
#include <emscripten.h>

void playSource(void* arg)
{
  ALuint source = reinterpret_cast<ALuint>(arg);
  ALint state;
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PLAYING);
  alSourcePause(source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PAUSED);
  alSourcePlay(source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PLAYING);
  alSourceStop(source);
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_STOPPED);

  int result = 1;
  REPORT_RESULT();
}

int main() {
  ALCdevice* device = alcOpenDevice(NULL);
  ALCcontext* context = alcCreateContext(device, NULL);
  alcMakeContextCurrent(context);

  ALfloat listenerPos[] = {0.0, 0.0, 0.0};
  ALfloat listenerVel[] = {0.0, 0.0, 0.0};
  ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

  alListenerfv(AL_POSITION, listenerPos);
  alListenerfv(AL_VELOCITY, listenerVel);
  alListenerfv(AL_ORIENTATION, listenerOri);

  ALuint buffers[1];

  alGenBuffers(1, buffers);

  FILE* source = fopen("audio.wav", "rb");
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

  ALuint sources[1];
  alGenSources(1, sources);

  alSourcei(sources[0], AL_BUFFER, buffers[0]);

  ALint state;
  alGetSourcei(sources[0], AL_SOURCE_STATE, &state);
  assert(state == AL_INITIAL);

  alSourcePlay(sources[0]);

  alGetSourcei(sources[0], AL_SOURCE_STATE, &state);
  assert(state == AL_PLAYING);

  emscripten_async_call(playSource, reinterpret_cast<void*>(sources[0]), 700);

  return 0;
}
