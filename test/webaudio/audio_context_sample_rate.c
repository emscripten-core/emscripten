#include <emscripten/webaudio.h>
#include <assert.h>
#include <stdio.h>

// This test verifies setting audio context sample rate, and that emscripten_audio_context_sample_rate() works.

void test(int sampleRate) {
  EmscriptenWebAudioCreateAttributes attr = {};
  attr.sampleRate = sampleRate;
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(&attr);
  assert(context);
  printf("emscripten_audio_context_sample_rate(): %d (expected %d)\n", emscripten_audio_context_sample_rate(context), sampleRate);
  assert(emscripten_audio_context_sample_rate(context) == sampleRate);
}

int main() {
  test(44100);
  test(48000);
  test(8000);
  test(96000);
  test(192000);
}
