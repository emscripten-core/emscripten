// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <assert.h>
#include <string.h>
#include <SDL/SDL.h>
#include "SDL/SDL_image.h"
#include <sys/stat.h>
#include <unistd.h>

int get_count = 0;
int data_ok = 0;
int data_bad = 0;

void onLoadedData(void *arg, void *buffer, int size) {
  printf("onLoadedData %ld\n", (long)arg);
  get_count++;
  assert(size == 329895);
  assert((long)arg == 135);
  unsigned char *b = (unsigned char*)buffer;
  assert(b[0] == 137);
  assert(b[1122] == 128);
  assert(b[1123] == 201);
  assert(b[202125] == 218);
  data_ok = 1;
}

void onErrorData(void *arg) {
  printf("onErrorData %ld\n", (long)arg);
  get_count++;
  assert((long)arg == 246);
  data_bad = 1;
}

int counter = 0;
void wait_wgets() {
  if (counter++ == 60) {
    printf("%d\n", get_count);
    counter = 0;
  }

  if (get_count == 6) {
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
  } else if (get_count == 8) {
    assert(IMG_Load("/tmp/screen_shot.png"));
    assert(data_ok == 1 && data_bad == 1);
    emscripten_cancel_main_loop();
    exit(0);
  }
  assert(get_count <= 8);
}

void onLoaded(const char* file) {
  assert(strcmp(file, "/tmp/test.html") == 0 ||
         strcmp(file, "/tmp/screen_shot.png") == 0 ||
         strcmp(file, "/this_directory_does_not_exist_and_should_be_created_by_wget/test.html") == 0 ||
         strcmp(file, "/path/this_directory_is_relative_to_cwd/test.html") == 0);

  FILE * f = fopen(file, "r");
  assert(f);
  printf("exists: %s\n", file);
  int c = fgetc (f);
  assert(c != EOF && "file empty");
  fclose(f);

  get_count++;
  printf("onLoaded %s\n", file);
}

void onError(const char* file) {
  printf("onError %s\n", file);
  assert(strcmp(file, "/tmp/null") == 0);

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

  // Try downloading the same file a second time
  emscripten_async_wget(
    "http://localhost:8888/test.html",
    "/tmp/test.html",
    onLoaded,
    onError);

  // Try downloading a file to a destination directory that does not exist.
  emscripten_async_wget(
    "http://localhost:8888/test.html",
    "/this_directory_does_not_exist_and_should_be_created_by_wget/test.html",
    onLoaded,
    onError);

  mkdir("/path", 0777);
  chdir("/path");

  // Try downloading a file to a destination directory that is a nonexisting path relative to CWD.
  emscripten_async_wget(
    "http://localhost:8888/test.html",
    "this_directory_is_relative_to_cwd/test.html",
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

  return 99;
}
