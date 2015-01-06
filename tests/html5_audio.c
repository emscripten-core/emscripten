#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten.h>
#include <html5.h>
#include <sys/stat.h>
#include <math.h>

EMSCRIPTEN_AUDIO_INSTANCE sound3, channel, noisey;

void done() {
  printf("test is done\n");
  int result = 1;
  REPORT_RESULT();
}

int state = 0;

void did_play(void* arg) {
  if (state == 0) {
    assert(123 == (int)arg);
    printf("did play of a nice sound, now here is some artificial nonsense\n");
    float samples[44000];
    for (int i = 0; i < 44000; i++) {
      samples[i] = sqrtf(((float)((i*i) % 4000))/4000);
    }
    noisey = emscripten_audio_load_pcm(1, 44000, 22000, samples);
    printf("noisey instance: %d\n", noisey);
    EMSCRIPTEN_RESULT status = emscripten_audio_get_load_state(noisey);
    assert(status == EMSCRIPTEN_RESULT_SUCCESS);
    emscripten_audio_play(noisey, channel);
    state = 1;
  } else if (state == 1) {
    done();
  }
}

void loop() {
  EMSCRIPTEN_RESULT status = emscripten_audio_get_load_state(sound3);
  if (status == EMSCRIPTEN_RESULT_DEFERRED) return;
  assert(status == EMSCRIPTEN_RESULT_SUCCESS);

  emscripten_audio_play(sound3, channel);
  printf("you should now hear a sound.\n");

  emscripten_cancel_main_loop();
}

int main(int argc, char **argv) {
  EMSCRIPTEN_RESULT ret = emscripten_audio_init();
  assert(ret == EMSCRIPTEN_RESULT_SUCCESS);

  {
    // load some data and create a buffer
    struct stat info;
    int result = stat("sound.ogg", &info);
    char * bytes = malloc( info.st_size );
    FILE * f = fopen("sound.ogg", "rb");
    fread(bytes, 1, info.st_size, f);
    fclose(f);
    sound3 = emscripten_audio_load(bytes, info.st_size);
    free(bytes);
  }

  EMSCRIPTEN_RESULT status = emscripten_audio_get_load_state(sound3);
  assert(status == EMSCRIPTEN_RESULT_DEFERRED);

  channel = emscripten_audio_create_channel(did_play, (void*)123);
  emscripten_set_main_loop(loop, 0, 0);

  return 0;
}

