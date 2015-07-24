#include<stdio.h>
#include<emscripten.h>
#include<assert.h>

void print(const char *c) {
  EM_ASM_({ Module.print($0) }, c);
}

void sleeper() {
  emscripten_sleep(100);
}

void sibling() {
  print("hi\n");
}

void middle();

void recurser() {
  middle();
}

void middle() {
  sleeper();
  sibling();
  recurser();
}

void pre() {
  print("bi\n");
}

void post() {
  print("tri\n");
}

int main() {
  post();
  middle();
  pre();
}

