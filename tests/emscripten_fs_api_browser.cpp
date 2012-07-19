#include<stdio.h>
#include<emscripten.h>
#include<assert.h>
#include<string.h>
#include<SDL/SDL.h>
#include"SDL/SDL_image.h"
 
extern "C" {

int result = 1;
int get_count = 0;

void wait_wgets() {
  if (get_count == 3) {
    assert(IMG_Load("/tmp/screen_shot.png"));
    emscripten_cancel_main_loop();
    REPORT_RESULT();
  }
}

void onLoaded(const char* file) {
  if (strcmp(file, "/tmp/test.html") && strcmp(file, "/tmp/screen_shot.png")) {
    result = 0;
  }

  printf("loaded: %s\n", file);

  if (FILE * f = fopen(file, "r")) {
      printf("exists: %s\n", file);
      int c = fgetc (f);
      if (c == EOF) {
        printf("file empty: %s\n", file);
        result = 0;
      }
      fclose(f);
  } else {
    result = 0;
    printf("!exists: %s\n", file);
  }
  
  get_count++;
}

void onError(const char* file) {
  if (strcmp(file, "/tmp/null")) {
    result = 0;
  }

  printf("error: %s\n", file);
  get_count++;
}

int main() {
  emscripten_async_wget(
    "http://localhost:8888/this_is_not_a_file", 
    "/tmp/null",
    onLoaded,
    onError);

  emscripten_async_wget(
    "http://localhost:8888/test.html", 
    "/tmp/test.html",
    onLoaded,
    onError);

  emscripten_async_wget(
    "http://localhost:8888/screenshot.png", 
    "/tmp/screen_shot.png",
    onLoaded,
    onError);

  emscripten_set_main_loop(wait_wgets, 0);

  return 0;
}

}

