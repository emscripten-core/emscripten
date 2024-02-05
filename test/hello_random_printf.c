#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emscripten.h>

int main() {
  srand(time(NULL));

  printf("hello: a random string: %s, an integer: %d, a float: %f. Time now: %f\n",
    emscripten_random() > 0.5 ? "test" : "test2",
    rand(),
    emscripten_random(),
    emscripten_get_now());
}
