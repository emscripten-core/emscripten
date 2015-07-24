#include<stdio.h>
#include<emscripten.h>
#include<assert.h>

void print1(const char *c, int x) {
  EM_ASM_({ Module.print([$0, $1]) }, c, x);
}

void print2(const char *c, int x, int y) {
  EM_ASM_({ Module.print([$0, $1, $2]) }, c, x, y);
}

typedef void (*func_v)();
typedef void (*func_f)(float);
typedef void (*func_i)(int);
typedef void (*func_ii)(int, int);

void sleeper() {
  emscripten_sleep(100);
}

void sibling(int x, int y) {
  print2("hi %d, %d\n", x, y);
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
  print1("bi, %f\n", f);
  sibling(3, 4);
  volatile func_ii ii = sibling;
  ii(5, 6);
}

void post(int y) {
  print1("tri %d\n", y);
  volatile func_i i = post;
  i(4);
  volatile func_v v = recurser;
  v();
}

void post2(int y) {
  print1("trip %d\n", y);
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

