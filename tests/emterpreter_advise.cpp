// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>
#include <assert.h>

void print(const char *c) {
  EM_ASM_({ out($0) }, c);
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

