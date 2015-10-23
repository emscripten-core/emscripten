#include<stdio.h>
#include<emscripten.h>
#include<assert.h>
#include<string.h>
#include<SDL/SDL.h>
#include"SDL/SDL_image.h"
 
extern "C" {

int result = 1;
int get_count = 0;
int data_ok = 0;
int data_bad = 0;

void onLoadedData(void *arg, void *buffer, int size) {
  printf("onLoadedData %d\n", (int)arg);
  get_count++;
  assert(size == 329895);
  assert((int)arg == 135);
  unsigned char *b = (unsigned char*)buffer;
  assert(b[0] == 137);
  assert(b[1122] == 128);
  assert(b[1123] == 201);
  assert(b[202125] == 218);
  data_ok = 1;
}

void onErrorData(void *arg) {
  printf("onErrorData %d\n", (int)arg);
  get_count++;
  assert((int)arg == 246);
  data_bad = 1;
}

int counter = 0;
void wait_wgets() {
  if (counter++ == 60) {
    printf("%d\n", get_count);
    counter = 0;
  }

  if (get_count == 3) {
    static bool fired = false;
    if (!fired) {
      fired = true;
      emscripten_async_wget_data(
        "http://localhost:8888/screenshot.png",
        (void*)135,
        onLoadedData,
        onErrorData);
      emscripten_async_wget_data(
        "http://localhost:8888/fail_me",
        (void*)246,
        onLoadedData,
        onErrorData);
    }
  } else if (get_count == 5) {
    assert(IMG_Load("/tmp/screen_shot.png"));
    assert(data_ok == 1 && data_bad == 1);
    emscripten_cancel_main_loop();
    REPORT_RESULT();
  }
  assert(get_count <= 5);
}

void onLoaded(const char* file) {
  if (strcmp(file, "/tmp/test.html") && strcmp(file, "/tmp/screen_shot.png")) {
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
}

void onError(const char* file) {
  if (strcmp(file, "/tmp/null")) {
    result = 0;
  }

  get_count++;
  printf("onError %s\n", file);
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

  char name[40];
  strcpy(name, "/tmp/screen_shot.png"); // test for issue #2349, name being free'd
  emscripten_async_wget(
    "http://localhost:8888/screenshot.png", 
    name,
    onLoaded,
    onError);
  memset(name, 0, 30);

  emscripten_set_main_loop(wait_wgets, 0, 0);

  return 0;
}

}

