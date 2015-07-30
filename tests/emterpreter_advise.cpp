#include<stdio.h>
#include<emscripten.h>
#include<assert.h>

void sleeper() {
  emscripten_sleep(100);
}

void sibling() {
  printf("hi\n");
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
  printf("bi\n");
}

void post() {
  printf("tri\n");
}

int main() {
  post();
  middle();
  pre();
}

