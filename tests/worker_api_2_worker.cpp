#include <assert.h>
#include <emscripten.h>

struct Info {
  int i;
  float f;
  char c;
  double d;
};

int calls = 0; // global state that is not in all workers

extern "C" {

void one(char *data, int size) {
  calls++;
  emscripten_worker_respond(data, size);
}

void two(char *data, int size) {
  calls++;
  Info *x = (Info*)data;
  x[0].i++;
  x[0].f--;
  x[0].c++;
  x[0].d--;
  emscripten_worker_respond(data, size);
}

void three(char *data, int size) {
  assert(data == 0);
  assert(size == 0);
  calls++;
  // no response
}

void four(char *data, int size) {
  assert(data == 0);
  assert(size == 0);
  emscripten_worker_respond((char*)&calls, sizeof(calls));
}

}

