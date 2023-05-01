#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <emscripten/fetch.h>
#include <emscripten/eventloop.h>

void readyStateChange(emscripten_fetch_t *fetch) {
  printf("readyStateChange: %d\n", fetch->readyState);
  if (fetch->readyState == 2) { // HEADERS_RECEIVED
    printf("HEADERS_RECEIVED: numBytes=%llu\n", fetch->numBytes);
  }
}

void onerror(emscripten_fetch_t *fetch) {
  printf("onerror: %d\n", fetch->status);
  assert(fetch->status == 408);
  emscripten_fetch_close(fetch);
}

void success(emscripten_fetch_t *fetch) {
  printf("unexpected success: numBytes=%llu\n", fetch->numBytes);
  assert(false);
  emscripten_fetch_close(fetch);
}

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_REPLACE;
  attr.onsuccess = success;
  attr.onerror = onerror;
  attr.onreadystatechange = readyStateChange;
  attr.timeoutMSecs = 100;
  emscripten_fetch(&attr, "timeout.txt");
  return 0;
}
