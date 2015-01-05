#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten.h>
#include <html5.h>
#include <sys/stat.h>

EMSCRIPTEN_AUDIO_INSTANCE sound3, channel;

int did = 0;

void done(void* x) {
  assert(did);
  printf("test is done\n");
  int result = 1;
  REPORT_RESULT();
}

void did_play(void* arg) {
  printf("got a channel.\n");
  assert(123 == (int)arg);
  did = 1;
  printf("did play\n");
}

void loop() {
  EMSCRIPTEN_RESULT status = emscripten_audio_get_load_state(sound3);
  if (status == EMSCRIPTEN_RESULT_DEFERRED) return;
  assert(status == EMSCRIPTEN_RESULT_SUCCESS);

  emscripten_audio_play(sound3, channel);
  printf("you should now hear a sound.\n");

  emscripten_async_call(done, NULL, 6000);

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

