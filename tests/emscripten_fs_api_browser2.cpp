#include<stdio.h>
#include<emscripten.h>
#include<assert.h>
#include<string.h>
#include<SDL/SDL.h>
#include"SDL/SDL_image.h"
 
extern "C" {

int result = 1;
int get_count = 0;

void onLoaded(const char* file) {
  if (strcmp(file, "/tmp/test.html")) {
    printf("what?\n");
    result = 0;
  }

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
  printf("onLoaded %s\n", file);

  if (get_count == 2) {
    emscripten_cancel_main_loop();
    REPORT_RESULT();
  }
}

void onError(const char* file) {
  printf("error...\n");
  result = 0;
}

int main() {
  emscripten_async_wget(
    "http://localhost:8888/test.html", 
    "/tmp/test.html",
    onLoaded,
    onError);

  // get another file to the same place
  emscripten_async_wget(
    "http://localhost:8888/test.js", 
    "/tmp/test.html",
    onLoaded,
    onError);

  return 0;
}

}

