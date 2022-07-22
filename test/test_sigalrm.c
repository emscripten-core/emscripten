// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <emscripten/emscripten.h>

void alarm_handler(int dummy) {
  printf("Received alarm!\n");
  emscripten_force_exit(0);
}

int main() {
  if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
    printf("Error in signal()!\n");
    return 1;
  }
  alarm(1);
  // Make sure that we keep the runtime alive long enough
  // to receive the alarm.
  emscripten_exit_with_live_runtime();
  __builtin_unreachable();
}
