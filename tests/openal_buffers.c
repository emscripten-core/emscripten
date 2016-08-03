#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <AL/al.h>
#include <AL/alc.h>
#else
#include "../system/include/AL/al.h"
#include "../system/include/AL/alc.h"
#endif

#define NUM_BUFFERS 4
#define BUFFER_SIZE 1470*10

ALCdevice* device = NULL;
ALCcontext* context = NULL;

// Audio source state.
unsigned char* data = NULL;
unsigned int size = 0;
unsigned int offset = 0;
unsigned int channels = 0;
unsigned int frequency = 0;
unsigned int bits = 0;
ALenum format = 0;
ALuint source = 0;
#ifdef TEST_ANIMATED_PITCH
// When testing animated pitch, start testing at a high pitch, and scale smaller.
float pitch = 1.3f;
#else
// When not testing animating the pitch value, play back at a slow rate.
float pitch = 0.5f;
#endif

void iter() {
  ALuint buffer = 0;
  ALint buffersProcessed = 0;
  ALint buffersWereQueued = 0;
  ALint buffersQueued = 0;
  ALint state;
  float testPitch;

  alGetSourcei(source, AL_BUFFERS_PROCESSED, &buffersProcessed);

  while (offset < size && buffersProcessed--) {
    // unqueue the old buffer and validate the queue length
    alGetSourcei(source, AL_BUFFERS_QUEUED, &buffersWereQueued);
    alSourceUnqueueBuffers(source, 1, &buffer);

    assert(alGetError() == AL_NO_ERROR);
    int len = size - offset;
    if (len > BUFFER_SIZE) {
      len = BUFFER_SIZE;
    }

    alGetSourcei(source, AL_BUFFERS_QUEUED, &buffersQueued);
    assert(buffersQueued == buffersWereQueued - 1);

    // queue the new buffer and validate the queue length
    buffersWereQueued = buffersQueued;
    alBufferData(buffer, format, &data[offset], len, frequency);

    alSourceQueueBuffers(source, 1, &buffer);
    assert(alGetError() == AL_NO_ERROR);

    alGetSourcei(source, AL_BUFFERS_QUEUED, &buffersQueued);
    assert(buffersQueued == buffersWereQueued + 1);

    // make sure it's still playing
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    assert(state == AL_PLAYING);

    offset += len;
  }

#ifdef TEST_ANIMATED_PITCH
  pitch *= .999;
  if (pitch < 0.5f)
    pitch = 0.5f;
  alSourcef(source, AL_PITCH, pitch);
  alGetSourcef(source, AL_PITCH, &testPitch);
  assert(pitch == testPitch);
#endif

  // Exit once we've processed the entire clip.
  if (offset >= size) {
#ifdef __EMSCRIPTEN__
    int result = 0;
    REPORT_RESULT();
#endif
    exit(0);
  }
}

int main(int argc, char* argv[]) {
  float testPitch;
  //
  // Setup the AL context.
  //
  device = alcOpenDevice(NULL);
  context = alcCreateContext(device, NULL);
  alcMakeContextCurrent(context);

  //
  // Read in the audio sample.
  //
#ifdef __EMSCRIPTEN__
  FILE* fp = fopen("the_entertainer.wav", "rb");
#else
  FILE* fp = fopen("sounds/the_entertainer.wav", "rb");
#endif
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  data = (unsigned char*)malloc(size);
  fread(data, size, 1, fp);
  fclose(fp);

  offset = 12; // ignore the RIFF header
  offset += 8; // ignore the fmt header
  offset += 2; // ignore the format type

  channels = data[offset + 1] << 8;
  channels |= data[offset];
  offset += 2;
  printf("Channels: %u\n", channels);

  frequency = data[offset + 3] << 24;
  frequency |= data[offset + 2] << 16;
  frequency |= data[offset + 1] << 8;
  frequency |= data[offset];
  offset += 4;
  printf("Frequency: %u\n", frequency);

  offset += 6; // ignore block size and bps

  bits = data[offset + 1] << 8;
  bits |= data[offset];
  offset += 2;
  printf("Bits: %u\n", bits);

  format = 0;
  if (bits == 8) {
    if (channels == 1) {
      format = AL_FORMAT_MONO8;
    } else if (channels == 2) {
      format = AL_FORMAT_STEREO8;
    }
  } else if (bits == 16) {
    if (channels == 1) {
      format = AL_FORMAT_MONO16;
    } else if (channels == 2) {
      format = AL_FORMAT_STEREO16;
    }
  }
  offset += 8; // ignore the data chunk

  //
  // Seed the buffers with some initial data.
  //
  ALuint buffers[NUM_BUFFERS];
  alGenBuffers(NUM_BUFFERS, buffers);
  alGenSources(1, &source);

  alSourcef(source, AL_PITCH, pitch);
  alGetSourcef(source, AL_PITCH, &testPitch);
  assert(pitch == testPitch);

  ALint numBuffers = 0;
  while (numBuffers < NUM_BUFFERS && offset < size) {
    int len = size - offset;
    if (len > BUFFER_SIZE) {
      len = BUFFER_SIZE;
    }

    alBufferData(buffers[numBuffers], format, &data[offset], len, frequency);
    alSourceQueueBuffers(source, 1, &buffers[numBuffers]);
    assert(alGetError() == AL_NO_ERROR);

    offset += len;
    numBuffers++;
  }

#ifdef TEST_ANIMATED_PITCH
  printf("You should hear a clip of the 1902 piano song \"The Entertainer\" played back at a high pitch rate, and animated to slow down to half playback speed.\n");
#else
  printf("You should hear a clip of the 1902 piano song \"The Entertainer\" played back at half speed.\n");
#endif

  //
  // Start playing the source.
  //
  alSourcePlay(source);

  ALint state;
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  assert(state == AL_PLAYING);

  alGetSourcei(source, AL_BUFFERS_QUEUED, &numBuffers);
  assert(numBuffers == NUM_BUFFERS);

  //
  // Cycle and refill the buffers until we're done.
  //
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(iter, 0, 0);
#else
  while (1) {
    iter();
    usleep(16);
  }
#endif
}
