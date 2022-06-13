// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

struct Info {
  int i;
  float f;
  char c;
  double d;
};

int w1;

Info x[3] = { {    22,      3.159,  97, 2.1828 },
              { 55123, 987612.563, static_cast<char>(190), 0.0009 },
              {  -102,    -12.532, -21, -51252 } };

int stage = -1;

int c3_7 = 0, c3_8 = 0;

void c3(char *data, int size, void *arg) { // tests calls different in different workers.
  int calls = *((int*)data);
  printf("%ld: %d\n", (long)arg, calls);
  if ((long)arg == 7) {
    assert(c3_7 == 0);
    c3_7++;
    assert(calls == 5);
  } else {
    assert((long)arg == 8);
    assert(c3_8 == 0);
    c3_8++;
    assert(calls == 1);
  }
  if (c3_7 && c3_8) { // note: racey, responses from 2 workers here
    emscripten_destroy_worker(w1);
    REPORT_RESULT(11);
    emscripten_force_exit(0);
  }
}

void c2(char *data, int size, void *arg) { // tests queuing up several messages, each with different data
  assert((long)arg == stage);
  Info *x2 = (Info*)data;

  int i = stage - 3;
  printf("c2-%d\n", i);
  printf("%d, %.2f, %d, %.2f\n", x2[0].i, x2[0].f, x2[0].c, x2[0].d);
  printf("%d, %.2f, %d, %.2f\n", x[i].i, x[i].f, x[i].c, x[i].d);
  assert(x2[0].i == x[i].i+1);
  assert(x2[0].f == x[i].f-1);
  assert(x2[0].c == x[i].c+1);
  assert(x2[0].d == x[i].d-1);

  if (stage == 5) {
    int w2 = emscripten_create_worker("worker.js");
    emscripten_call_worker(w2, "three", NULL, 0, NULL, (void*)6); // bump calls in new worker, once

    emscripten_call_worker(w1, "four", NULL, 0, c3, (void*)7);
    emscripten_call_worker(w2, "four", NULL, 0, c3, (void*)8);
  }
  stage++;
}

void c1(char *data, int size, void *arg) { // tests copying + buffer enlargement
  assert((long)arg == stage);
  if (stage == 1) {
    printf("wait 0? %d\n", emscripten_get_worker_queue_size(w1));
    assert(emscripten_get_worker_queue_size(w1) == 0);
  }
  Info *x2 = (Info*)data;
  assert(x2 != x && x2 != x+1 && x2 != x+2);
  for (int i = 0; i < size/sizeof(Info); i++) {
    printf("c1-%d\n", i);
    printf("  %d, %.2f, %d, %.2f\n", x2[i].i, x2[i].f, x2[i].c, x2[i].d);
    printf("  %d, %.2f, %d, %.2f\n", x[i].i, x[i].f, x[i].c, x[i].d);
    assert(x2[i].i == x[i].i);
    assert(x2[i].f == x[i].f);
    assert(x2[i].c == x[i].c);
    assert(x2[i].d == x[i].d);
  }
  if (stage < 2) {
    emscripten_call_worker(w1, "one", (char*)x, sizeof(Info)*3, c1, (void*)2);
  } else {
    printf("wait 1? %d\n", emscripten_get_worker_queue_size(w1));
    assert(emscripten_get_worker_queue_size(w1) == 0);
    emscripten_call_worker(w1, "two", (char*)&x[0], sizeof(Info), c2, (void*)3);
    emscripten_call_worker(w1, "two", (char*)&x[1], sizeof(Info), c2, (void*)4);
    emscripten_call_worker(w1, "two", (char*)&x[2], sizeof(Info), c2, (void*)5);
    printf("wait 2? %d\n", emscripten_get_worker_queue_size(w1));
    assert(emscripten_get_worker_queue_size(w1) == 3);
  }
  stage++;
}

int main() {
  w1 = emscripten_create_worker("worker.js");

  printf("wait -1? %d\n", emscripten_get_worker_queue_size(w1));
  assert(emscripten_get_worker_queue_size(w1) == 0);
  emscripten_call_worker(w1, "one", (char*)x, sizeof(Info)*2, c1, (void*)1);
  printf("wait -1? %d\n", emscripten_get_worker_queue_size(w1));
  assert(emscripten_get_worker_queue_size(w1) == 1);
  stage = 1; // make sure we get here

  return 0;
}

