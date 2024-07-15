// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#define _GNU_SOURCE
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include <emscripten/emscripten.h>

_Atomic int got_alarm[3];

void alarm_handler(int dummy) {
  printf("Received SIGALRM!\n");
  got_alarm[ITIMER_REAL]++;
}

void vtalarm_handler(int dummy) {
  printf("Received SIGVTALRM!\n");
  got_alarm[ITIMER_VIRTUAL]++;
}

void prof_handler(int dummy) {
  printf("Received SIGPROF!\n");
  got_alarm[ITIMER_PROF]++;
}

void test_oneoff(int which) {
  memset(got_alarm, 0, sizeof(got_alarm));

  int rtn;
  struct itimerval val;
  memset(&val, 0, sizeof(val));

  // Set a timer for 1 second
  val.it_value.tv_sec = 1;
  rtn = setitimer(which, &val, NULL);
  assert(rtn == 0);

  rtn = getitimer(which, &val);
  assert(rtn == 0);
  printf("ms remaining: %d\n", val.it_value.tv_usec / 1000);
  assert(val.it_value.tv_usec || val.it_value.tv_sec);

  // Wait 100ms
  usleep(100 * 1000);

  // Verify less time remains
  rtn = getitimer(which, &val);
  assert(rtn == 0);
  printf("ms remaining: %d\n", val.it_value.tv_usec / 1000);
  assert(val.it_value.tv_sec == 0);
  assert(val.it_value.tv_usec > 0);

  // Wait 1.5s
  assert(!got_alarm[which]);
  usleep(1500 * 1000);

  // Verify that the time fired and is no longer active
  assert(got_alarm[which]);
  rtn = getitimer(which, &val);
  assert(val.it_value.tv_sec == 0);
  assert(val.it_value.tv_usec == 0);
}

#define NUM_TIMERS 10
#define ERROR_MARGIN 3

void test_sequence(int which) {
  memset(got_alarm, 0, sizeof(got_alarm));
  // Set a timer to fire every 100ms
  struct itimerval val;
  val.it_value.tv_sec = 0;
  val.it_value.tv_usec = 100 * 1000;
  val.it_interval.tv_sec = 0;
  val.it_interval.tv_usec = 100 * 1000;
  int rtn = setitimer(which, &val, NULL);
  // Sleep for a little over NUM_TIMERS * 100ms
  usleep((NUM_TIMERS * 100 + 50) * 1000);
  printf("got %d alarms\n", got_alarm[which]);
  // Normally we would expect NUM_TIMERS to fire in this time
  // but leave some wiggle room for scheduling anomalies.
  assert(got_alarm[which] > NUM_TIMERS - ERROR_MARGIN);
  assert(got_alarm[which] < NUM_TIMERS + ERROR_MARGIN);
  struct itimerval old;
  val.it_value.tv_sec = 0;
  val.it_value.tv_usec = 0;
  rtn = setitimer(ITIMER_REAL, &val, &old);
}

void set_handlers() {
  sighandler_t rtn;
  rtn = signal(SIGALRM, alarm_handler);
  assert(rtn != SIG_ERR);
  rtn = signal(SIGVTALRM, vtalarm_handler);
  assert(rtn != SIG_ERR);
  rtn = signal(SIGPROF, prof_handler);
  assert(rtn != SIG_ERR);
}

int main() {
  set_handlers();

  test_oneoff(ITIMER_REAL);
  test_oneoff(ITIMER_VIRTUAL);
  test_oneoff(ITIMER_PROF);

  test_sequence(ITIMER_REAL);

  printf("done\n");
  return 0;
}
