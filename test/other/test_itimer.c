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
  printf("test_oneoff\n");
  memset(got_alarm, 0, sizeof(got_alarm));

  int rtn;
  struct itimerval val;
  memset(&val, 0, sizeof(val));

  // Set a long non-repeating timer (5 seconds) so we can safely inspect
  // getitimer before expiration even on slow CI runners.
  val.it_value.tv_sec = 5;
  val.it_value.tv_usec = 0;
  rtn = setitimer(which, &val, NULL);
  assert(rtn == 0);

  rtn = getitimer(which, &val);
  assert(rtn == 0);
  printf("remaining: %llds %dms\n", val.it_value.tv_sec, val.it_value.tv_usec / 1000);
  assert(val.it_value.tv_usec || val.it_value.tv_sec);

  // Wait 10ms
  usleep(10 * 1000);

  // Verify less time remains and the timer has not yet fired
  rtn = getitimer(which, &val);
  assert(rtn == 0);
  printf("remaining: %llds %dms\n", val.it_value.tv_sec, val.it_value.tv_usec / 1000);
  assert(val.it_value.tv_sec < 5);
  assert(val.it_value.tv_sec > 0 || val.it_value.tv_usec > 0);
  assert(!got_alarm[which]);

  // Now set a short non-repeating timer (50ms) and wait for it to fire
  val.it_value.tv_sec = 0;
  val.it_value.tv_usec = 50 * 1000;
  rtn = setitimer(which, &val, NULL);
  assert(rtn == 0);

  while (!got_alarm[which]) {
    usleep(10 * 1000);
  }

  // Verify that the timer fired once and is no longer active
  assert(got_alarm[which] == 1);
  rtn = getitimer(which, &val);
  assert(rtn == 0);
  assert(val.it_value.tv_sec == 0);
  assert(val.it_value.tv_usec == 0);
}

#define NUM_TIMERS 5

void test_sequence(int which) {
  printf("test_sequence (waiting for %d alarms)\n", NUM_TIMERS);
  memset(got_alarm, 0, sizeof(got_alarm));
  // Set a timer to fire every 50ms
  struct itimerval val;
  val.it_value.tv_sec = 0;
  val.it_value.tv_usec = 50 * 1000;
  val.it_interval.tv_sec = 0;
  val.it_interval.tv_usec = 50 * 1000;
  int rtn = setitimer(which, &val, NULL);
  assert(rtn == 0);

  while (got_alarm[which] < NUM_TIMERS) {
    usleep(10 * 1000);
  }
  printf("got %d alarms\n", got_alarm[which]);

  // Disable the timer and verify no further alarms arrive
  struct itimerval old;
  memset(&val, 0, sizeof(val));
  rtn = setitimer(which, &val, &old);
  assert(rtn == 0);
  int count_after_stop = got_alarm[which];
  usleep(100 * 1000);
  assert(got_alarm[which] == count_after_stop);
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

void test_getitimer() {
  printf("test_getitimer\n");
  struct itimerval val = {
    .it_value = { .tv_sec = 5, .tv_usec = 500 * 1000 },
    .it_interval = { .tv_sec = 2, .tv_usec = 250 * 1000 },
  };
  int rtn = setitimer(ITIMER_REAL, &val, NULL);
  assert(rtn == 0);

  struct itimerval curr;
  rtn = getitimer(ITIMER_REAL, &curr);
  assert(rtn == 0);
  assert(curr.it_value.tv_sec >= 0 && curr.it_value.tv_sec <= 5);
  assert(curr.it_value.tv_usec >= 0 && curr.it_value.tv_usec < 1000000);
  assert(curr.it_interval.tv_sec == 2);
  assert(curr.it_interval.tv_usec == 250 * 1000);

  // Disarm and check old value returned by setitimer
  struct itimerval old;
  memset(&val, 0, sizeof(val));
  rtn = setitimer(ITIMER_REAL, &val, &old);
  assert(rtn == 0);
  assert(old.it_value.tv_sec >= 0 && old.it_value.tv_sec <= 5);
  assert(old.it_value.tv_usec >= 0 && old.it_value.tv_usec < 1000000);
  assert(old.it_interval.tv_sec == 2);
  assert(old.it_interval.tv_usec == 250 * 1000);
}

int main() {
  set_handlers();

  test_getitimer();

  test_oneoff(ITIMER_REAL);
  test_oneoff(ITIMER_VIRTUAL);
  test_oneoff(ITIMER_PROF);

  test_sequence(ITIMER_REAL);

  printf("done\n");
  return 0;
}
