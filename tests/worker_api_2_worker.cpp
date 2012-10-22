#include <assert.h>
#include <emscripten.h>

struct Info {
  int i;
  float f;
  char c;
  double d;
};

extern "C" {

void one(char *data, int size) {
  emscripten_worker_respond(data, size);
}

void two(char *data, int size) {
  Info *x = (Info*)data;
  x[0].i++;
  x[0].f--;
  x[0].c++;
  x[0].d--;
  emscripten_worker_respond(data, size);
}

}

