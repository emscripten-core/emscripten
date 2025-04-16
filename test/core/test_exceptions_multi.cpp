// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>

static int current_exception_id = 0;

typedef struct {
  int jmp;
} jmp_state;

void setjmp_func(jmp_state* s, int level) {
  int prev_jmp = s->jmp;
  int c_jmp;

  if (level == 2) {
    printf("level is 2, perform longjmp!\n");
    throw 1;
  }

  c_jmp = current_exception_id++;
  try {
    printf("setjmp normal execution path, level: %d, prev_jmp: %d\n", level,
           prev_jmp);
    s->jmp = c_jmp;
    setjmp_func(s, level + 1);
  }
  catch (int caught_eid) {
    printf("caught %d\n", caught_eid);
    if (caught_eid == c_jmp) {
      printf("setjmp exception execution path, level: %d, prev_jmp: %d\n",
             level, prev_jmp);
      if (prev_jmp != -1) {
        printf("prev_jmp is not empty, continue with longjmp!\n");
        s->jmp = prev_jmp;
        throw s->jmp;
      }
    } else {
      throw;
    }
  }

  printf("Exiting setjmp function, level: %d, prev_jmp: %d\n", level, prev_jmp);
}

int main(int argc, char* argv[]) {
  jmp_state s;
  s.jmp = -1;

  setjmp_func(&s, 0);

  return 0;
}
