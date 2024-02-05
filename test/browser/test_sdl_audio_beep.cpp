// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <queue>
#include <cmath>
#include <stdio.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifdef __EMSCRIPTEN__
#include "emscripten/emscripten.h"
#endif

#ifdef main
#undef main
#endif

const int tone_duration = 1000;

struct BeepObject {
  double toneFrequency;
  int samplesLeft;
};

class Beeper {
private:
  double phase;
  int frequency;
  int numChannels;
  int mutedChannel;
public:
  Beeper(int frequency, int numChannels, int sdlAudioFormat);
  ~Beeper();
  void beep(double toneFrequency, int durationMSecs);
  template<typename T>
  void generateSamples(T *stream, int length);
  void wait();

  std::queue<BeepObject> beeps;
  int sdlAudioFormat;
};

void audio_callback(void*, Uint8*, int);

Beeper::Beeper(int frequency_, int numChannels_, int sdlAudioFormat_) {
  phase = 0.0;
  mutedChannel = 1;

  SDL_AudioSpec desiredSpec;

  desiredSpec.freq = frequency_;
  desiredSpec.format = sdlAudioFormat_;
  desiredSpec.channels = numChannels_;
  desiredSpec.samples = 1024; // This is samples per channel.
  desiredSpec.callback = audio_callback;
  desiredSpec.userdata = this;

  SDL_AudioSpec obtainedSpec;

  // you might want to look for errors here
  SDL_OpenAudio(&desiredSpec, &obtainedSpec);

  // In this test, we require *exactly* the identical SDL result that we provide, since we test
  // all various configurations individually.
  if (obtainedSpec.freq != desiredSpec.freq || obtainedSpec.format != desiredSpec.format
    || obtainedSpec.channels != desiredSpec.channels || obtainedSpec.samples != desiredSpec.samples) {
    SDL_CloseAudio();
    throw std::runtime_error("Failed to initialize desired SDL_OpenAudio!");
  }

  frequency = obtainedSpec.freq;
  numChannels = obtainedSpec.channels;
  sdlAudioFormat = obtainedSpec.format;

  // Immediately start producing audio.
  SDL_PauseAudio(0);
}

Beeper::~Beeper() {
  SDL_CloseAudio();
}

template<typename T>
void Beeper::generateSamples(T *stream, int length) {
  T AMPLITUDE;
  if (sdlAudioFormat == AUDIO_F32) {
    AMPLITUDE = (T) 0.8f;
  }
  else {
   AMPLITUDE = (sizeof(T) == 2) ? 28000 : 120;
  }
  const T offset = (sdlAudioFormat == AUDIO_U8) ? 120 : 0;

  int i = 0;
  length /= numChannels;
  while (i < length) {
    if (beeps.empty()) {
      memset(stream + numChannels*i, 0, sizeof(T)*numChannels*(length-i));
      return;
    }
    BeepObject& bo = beeps.front();

    // In Stereo tests, mute one of the channels to be able to distinguish that Stereo output works.
    if (bo.samplesLeft > tone_duration * frequency / 2 / 1000) {
      mutedChannel = 1;
    } else {
      mutedChannel = 0;
    }

    int samplesToDo = std::min(i + bo.samplesLeft, length);
    bo.samplesLeft -= samplesToDo - i;

    while (i < samplesToDo) {
      for(int j = 0; j < numChannels; ++j) {
        stream[numChannels*i+j] = (T)(offset + (AMPLITUDE * std::sin(phase * 2 * M_PI / frequency)));
        if (numChannels > 1 && j == mutedChannel) {
          stream[numChannels*i+j] = 0;
        }
      }
      phase += bo.toneFrequency;
      i++;
    }

    if (bo.samplesLeft == 0) {
      beeps.pop();
    }
  }
}

void Beeper::beep(double toneFrequency, int durationMSecs) {
  BeepObject bo;
  bo.toneFrequency = toneFrequency;
  bo.samplesLeft = durationMSecs * frequency / 1000;

  SDL_LockAudio();
  beeps.push(bo);
  SDL_UnlockAudio();
}

Beeper *beep = 0;

// Test all kinds of various possible formats. Not all are supported, but running this
// test will report you which work.
const int freqs[] = { 8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000 };
const int channels[] = { 1, 2 };
const int sdlAudioFormats[] = { AUDIO_U8, AUDIO_S16LSB, AUDIO_F32 /*, AUDIO_S8, AUDIO_U16LSB, AUDIO_U16MSB, AUDIO_S16MSB */ };

const char *SdlAudioFormatToString(int sdlAudioType) {
  switch(sdlAudioType) {
  case AUDIO_U8: return "AUDIO_U8";
  case AUDIO_S8: return "AUDIO_S8";
  case AUDIO_U16LSB: return "AUDIO_U16LSB";
  case AUDIO_U16MSB: return "AUDIO_U16MSB";
  case AUDIO_S16LSB: return "AUDIO_S16LSB";
  case AUDIO_S16MSB: return "AUDIO_S16MSB";
  case AUDIO_F32: return "AUDIO_F32";
  default: return "(unknown)";
  }
}

#define NUM_ELEMS(x) (sizeof(x)/sizeof((x)[0]))

// Indices to the currently running test.
int f = -1;
int c = 0;
int s = 0;

void nextTest(void *unused = 0) {
  ++f;
  if (f >= NUM_ELEMS(freqs)) {
    f = 0;
    ++c;
    if (c >= NUM_ELEMS(channels)) {
      c = 0;
      ++s;
      if (s >= NUM_ELEMS(sdlAudioFormats)) {
        printf("All tests done. Quit.\n");
#ifdef __EMSCRIPTEN__
        emscripten_force_exit(0);
#endif
        return;
      }
    }
  }

  double Hz = 440;
  try {
    beep = new Beeper(freqs[f], channels[c], sdlAudioFormats[s]);
  } catch(...) {
    printf("FAILED to play beep for %d msecs at %d Hz tone with audio format %s, %d channels, and %d samples/sec.\n",
        tone_duration, (int)Hz, SdlAudioFormatToString(sdlAudioFormats[s]), channels[c], freqs[f]);
    nextTest();
    return;
  }

  printf("Playing back a beep for %d msecs at %d Hz tone with audio format %s, %d channels, and %d samples/sec.\n",
      tone_duration, (int)Hz, SdlAudioFormatToString(sdlAudioFormats[s]), channels[c], freqs[f]);
  beep->beep(Hz, tone_duration);
}

void update() {
  SDL_LockAudio();
  int size = beep->beeps.size();
  SDL_UnlockAudio();
  if (size == 0 && beep) {
    delete beep;
    beep = 0;
#ifdef __EMSCRIPTEN__
    emscripten_async_call(nextTest, 0, 1500);
#else
    SDL_Delay(1500);
    nextTest();
#endif
  }
}

void audio_callback(void *_beeper, Uint8 *_stream, int _length) {
  Beeper* beeper = (Beeper*) _beeper;

  if (beeper->sdlAudioFormat == AUDIO_U8) {
    Uint8 *stream = (Uint8*) _stream;
    beeper->generateSamples(stream, _length);
  } else if (beeper->sdlAudioFormat == AUDIO_S16LSB) {
    Sint16 *stream = (Sint16*) _stream;
    int length = _length / 2;
    beeper->generateSamples(stream, length);
  } else if (beeper->sdlAudioFormat == AUDIO_F32) {
    float* stream = (float*) _stream;
    int length =_length / 4;
    beeper->generateSamples(stream, length);
  } else {
    assert(false && "Audio sample generation not implemented for current format!\n");
  }
}

int main(int argc, char** argv) {
  SDL_Init(SDL_INIT_AUDIO);

  nextTest();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(update, 60, 0);
#else
  while(beep) {
    SDL_Delay(20);
    update();
  }
#endif

  return 0;
}
