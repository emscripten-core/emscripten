/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// This tests captures a fixed amount of audio data,
// then plays it back.
//
// Wishlist:
// - Try multiple devices simultaneously;
// - Have several recording passes over the same fixed buffer_size.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/eventloop.h>
#define ASSUME_AL_FLOAT32
#endif
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

const char* alformat_string(ALenum format) {
  switch (format) {
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

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void end_test(int result) {
  printf("end_test: %d\n", result);
  exit(result);
}

#ifndef TEST_DURATION
#define TEST_DURATION 8000000 // 8s -> 8000000us
#endif
#ifndef TEST_SAMPLERATE
#define TEST_SAMPLERATE 44100
#endif
#ifndef TEST_FORMAT
#define TEST_FORMAT AL_FORMAT_MONO16
#endif
#ifndef TEST_BUFFERSIZE
#define TEST_BUFFERSIZE TEST_SAMPLERATE * 50 / 1000 // 50ms buffer
#endif

// The "arg" pointer passed to iter().
// It's also a state machine with only two states (see is_playing_back):
// either capturing audio or playing back the captured samples.
typedef struct {
  bool is_playing_back;

  // When capturing
  const char *capture_device_name;
  ALCuint sample_rate;
  ALenum format;
  ALCsizei buffer_size;
  ALCdevice *capture_device;
  size_t sample_size;
  unsigned nchannels;

  // Playback
  ALuint source, buffer;
  ALCcontext *context;
  ALCdevice *playback_device;
  size_t captured;
  long long duration;
  int recorded_size;
  char * recorded;
} App;

App app = {
  .is_playing_back = false,
  .sample_rate = TEST_SAMPLERATE,
  .format = TEST_FORMAT,
  .buffer_size = TEST_BUFFERSIZE,
  .captured = 0,
  .duration = TEST_DURATION
};

// frames -> bytes
int bytesForFrames(int frames) {
  return frames * app.nchannels * app.sample_size;
}

// usec -> frames
int framesForDuration(long long usec) {
  return (usec * app.sample_rate) / 1000000LL;
}

// usec -> bytes
int bytesForDuration(long long usec) {
  return bytesForFrames(framesForDuration(usec));
}

// frames -> usec
long long durationForFrames(int frames) {
  return frames * 1000000LL / app.sample_rate;
}

void iter() {
  if (app.is_playing_back) {
    ALint state;
    alGetSourcei(app.source, AL_SOURCE_STATE, &state);

#ifdef __EMSCRIPTEN__
    return;
#else
    if (state != AL_STOPPED)
      return;
#endif

    alDeleteSources(1, &app.source);
    alDeleteBuffers(1, &app.buffer);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(app.context);
    alcCloseDevice(app.playback_device);
    end_test(EXIT_SUCCESS);
  }

  ALCint ncaptured = 0;
  alcGetIntegerv(app.capture_device, ALC_CAPTURE_SAMPLES, 1, &ncaptured);

  // take samples when the buffer is at least filled for 1/3 of its size
  if (ncaptured < app.buffer_size / 3)
    return;

  const int target = framesForDuration(app.duration);
  ALCint readSize = ncaptured;

  // check if there are more frames than what's needed
  if (app.captured + readSize > target)
    readSize = target - app.captured;

  alcCaptureSamples(app.capture_device, app.recorded + bytesForFrames(app.captured), readSize);
  ALCenum err = alcGetError(app.capture_device);
  if (err != ALC_NO_ERROR) {
    fprintf(stderr, "alcCaptureSamples() yielded an error, but wasn't supposed to! (%x, %s)\n", err, alcGetString(NULL, err));
    end_test(EXIT_FAILURE);
  }

  app.captured += readSize;

  if (app.captured < target)
    return;
  else if (app.captured > target) {
    fprintf(stderr, "Captured frames exeedes expectations!\n");
    end_test(EXIT_FAILURE);
  }


  // This was here to see if alcCaptureSamples() would reset the number of
  // available captured samples as a side-effect.
  // Turns out, it does (on Linux with OpenAL-Soft).
  // That's important to know because this behaviour, while reasonably
  // expected, isn't documented anywhere.
  /*
  {
    ALCint ncaptured_now = 0;
    alcGetIntegerv(app.capture_device, ALC_CAPTURE_SAMPLES, 1, &ncaptured_now);

    printf(
      "For information, number of captured sample frames :\n"
      "- Before alcCaptureSamples(): %u;\n"
      "- After  alcCaptureSamples(): %u.\n"
      , (unsigned)ncaptured, (unsigned)ncaptured_now
    );
  }
  */

  alcCaptureStop(app.capture_device);

#ifdef __EMSCRIPTEN__
  // Restarting capture must zero the reported number of captured samples.
  // Works in our case because no processing takes place until the current
  // iteration yields to the javascript main loop.
  alcCaptureStart(app.capture_device);
  alcCaptureStop(app.capture_device);
  ALCint zeroed_ncaptured = 0xdead;
  alcGetIntegerv(app.capture_device, ALC_CAPTURE_SAMPLES, 1, &zeroed_ncaptured);
  if (zeroed_ncaptured) {
    fprintf(stderr, "Restarting capture didn't zero the reported number of available sample frames!\n");
  }
#endif

  ALCboolean could_close = alcCaptureCloseDevice(app.capture_device);
  if (!could_close) {
    fprintf(stderr, "Could not close device \"%s\"!\n", app.capture_device_name);
    end_test(EXIT_FAILURE);
  }

  // We're not as careful with playback - this is already tested
  // elsewhere.
  app.playback_device = alcOpenDevice(NULL);
  assert(app.playback_device);
  app.context = alcCreateContext(app.playback_device, NULL);
  assert(app.context);
  alcMakeContextCurrent(app.context);
  alGenBuffers(1, &app.buffer);
  alGenSources(1, &app.source);
  alBufferData(app.buffer, app.format, app.recorded, app.recorded_size, app.sample_rate);
  alSourcei(app.source, AL_BUFFER, app.buffer);

  free(app.recorded);

#ifdef __EMSCRIPTEN__
  EM_ASM(
    var succeed_btn = document.createElement('input');
    var fail_btn  = document.createElement('input');
    succeed_btn.type = fail_btn.type = 'button';
    succeed_btn.name = succeed_btn.value = 'Succeed';
    fail_btn.name = fail_btn.value = 'Fail';
    succeed_btn.onclick = function() {
      _end_test(0);
    };
    fail_btn.onclick = function() {
      _end_test(1);
    };
    document.body.appendChild(succeed_btn);
    document.body.appendChild(fail_btn);
  );
#endif

  app.is_playing_back = true;
  alSourcePlay(app.source);
  printf(
    "You should now hear the captured audio data.\n"
#ifdef __EMSCRIPTEN__
    "Press the [Succeed] button to end the test successfully, or the [Fail] button otherwise.\n"
#endif
  );
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void ignite() {
#ifdef __EMSCRIPTEN__
  emscripten_runtime_keepalive_pop();
#endif
  app.capture_device_name = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);

  app.capture_device = alcCaptureOpenDevice(
    app.capture_device_name, app.sample_rate, app.format, app.buffer_size
  );

  if (!app.capture_device) {
    ALCenum err = alcGetError(app.capture_device);
    fprintf(stderr,
      "alcCaptureOpenDevice(\"%s\", sample_rate=%u, format=%s, "
      "buffer_size=%u) failed with ALC error %x (%s)\n",
      app.capture_device_name,
      (unsigned) app.sample_rate, alformat_string(app.format),
      (unsigned) app.buffer_size,
      (unsigned) err, alcGetString(NULL, err)
    );
    end_test(EXIT_FAILURE);
  }

  switch (app.format) {
  case AL_FORMAT_MONO8:      app.sample_size=1; app.nchannels=1; break;
  case AL_FORMAT_MONO16:     app.sample_size=2; app.nchannels=1; break;
  case AL_FORMAT_STEREO8:    app.sample_size=1; app.nchannels=2; break;
  case AL_FORMAT_STEREO16:     app.sample_size=2; app.nchannels=2; break;
#ifdef ASSUME_AL_FLOAT32
  case AL_FORMAT_MONO_FLOAT32:   app.sample_size=4; app.nchannels=1; break;
  case AL_FORMAT_STEREO_FLOAT32: app.sample_size=4; app.nchannels=2; break;
#endif
  }

  //allocate memory to store captured audio
  app.recorded_size = bytesForDuration(app.duration);
  app.recorded = malloc(app.recorded_size);
  if (!app.recorded) {
    fprintf(stderr, "Out of memory!\n");
    end_test(EXIT_FAILURE);
  }

  alcCaptureStart(app.capture_device);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(iter, NULL, 0, 0);
#else
  for (;;) {
    iter(&app);
    usleep(durationForFrames(app.buffer_size) / 3);
  }
#endif
}

int main() {
  printf(
    "This test will attempt to capture %f seconds "
    "worth of audio data from your default audio "
    "input device, and then play it back.\n"
    , app.duration / 1000000.F
  );
#ifdef __EMSCRIPTEN__
  printf(
    "Press the [Start Recording] button below when you're ready, then "
    "allow audio capture when asked by the browser.\n"
    "No sample should be captured until that moment.\n"
  );
  EM_ASM(
    var btn = document.createElement('input');
    btn.type = 'button';
    btn.name = btn.value = 'Start recording';
    btn.onclick = function() {
      _ignite();
      document.body.removeChild(btn);
    };
    document.body.appendChild(btn);
  );
  emscripten_runtime_keepalive_push();
#else
  printf("Press [Enter] when you're ready.\n");
  getchar();
  ignite();
#endif

  return EXIT_SUCCESS;
}
