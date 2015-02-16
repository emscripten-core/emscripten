#include<stdio.h>
#include<emscripten.h>
#include<assert.h>

typedef void (*func_v)();
typedef void (*func_f)(float);
typedef void (*func_i)(int);
typedef void (*func_ii)(int, int);

void sleeper() {
  emscripten_sleep(100);
}

void sibling(int x, int y) {
  printf("hi %d, %d\n", x, y);
}

void middle();

void recurser() {
  middle();
}

void middle() {
  sleeper();
  sibling(1, 2);
  recurser();
}

void pre(float f) {
  printf("bi, %f\n", f);
  sibling(3, 4);
  volatile func_ii ii = sibling;
  ii(5, 6);
}

void post(int y) {
  printf("tri %d\n", y);
  volatile func_i i = post;
  i(4);
  volatile func_v v = recurser;
  v();
}

void post2(int y) {
  printf("trip %d\n", y);
  volatile func_i i = post;
  i(4);
  recurser();
}


int main() {
  volatile func_f f = pre;
  f(1.5);
  middle();
  volatile func_i i = post;
  i(3);
  volatile func_i i2 = post2;
  i2(3);
}

