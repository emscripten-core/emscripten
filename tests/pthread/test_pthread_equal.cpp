// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <pthread.h>

int main() {
  int x = pthread_equal(pthread_self(), pthread_self());
  printf("is this thread the same as this thread? %d\n", x);
}

