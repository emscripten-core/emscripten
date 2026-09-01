/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Threads blocking in epoll_wait() on one shared epoll fd:
 *   - EPOLLET: each pipe-write edge wakes exactly one waiter.
 *   - level-triggered: one write wakes every waiter (still-ready re-list).
 *   - EPOLLONESHOT: one write wakes exactly one waiter, a further write wakes
 *     none (disarmed), and EPOLL_CTL_MOD re-arms off the buffered level.
 *   - EPOLLONESHOT fairness: rounds of events on several oneshot
 *     registrations, harvested by racing waiters, each deliver exactly once
 *     with none lost or duplicated, and re-arm cleanly for the next round.
 */

#include <sys/epoll.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

static int ep;
static atomic_int woken;

static void* waiter(void* arg) {
  struct epoll_event out;
  int n = epoll_wait(ep, &out, 1, -1);
  assert(n == 1);
  assert(out.events & EPOLLIN);
  assert(out.data.u32 == 0xabcd);
  atomic_fetch_add(&woken, 1);
  return NULL;
}

// Wait for `expect` wakeups, then verify no spurious extra one follows.
static void settle(int expect) {
  while (atomic_load(&woken) < expect) usleep(1000);
  usleep(10000);
  assert(atomic_load(&woken) == expect);
}

#define NPIPES 4
#define QUIT 0xffff
static atomic_int counts[NPIPES];
static atomic_int total;

static void* harvester(void* arg) {
  for (;;) {
    struct epoll_event out;
    int n = epoll_wait(ep, &out, 1, -1);
    assert(n == 1);
    if (out.data.u32 == QUIT) return NULL;
    assert(out.data.u32 < NPIPES);
    atomic_fetch_add(&counts[out.data.u32], 1);
    atomic_fetch_add(&total, 1);
  }
}

static void drain(int fd, int count) {
  char buf[8];
  assert(read(fd, buf, count) == count);
}

int main(void) {
  ep = epoll_create1(0);
  assert(ep >= 0);
  int p[2];
  assert(pipe(p) == 0);
  struct epoll_event ev = { .events = EPOLLIN | EPOLLET };
  ev.data.u32 = 0xabcd;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &ev) == 0);

  pthread_t t1, t2;

  // EPOLLET: each write edge wakes exactly one of the two blocked waiters.
  assert(pthread_create(&t1, NULL, waiter, NULL) == 0);
  assert(pthread_create(&t2, NULL, waiter, NULL) == 0);
  assert(write(p[1], "a", 1) == 1);
  settle(1);
  assert(write(p[1], "b", 1) == 1);
  settle(2);
  assert(pthread_join(t1, NULL) == 0);
  assert(pthread_join(t2, NULL) == 0);

  // Level-triggered: a single write wakes both waiters.
  drain(p[0], 2);
  atomic_store(&woken, 0);
  ev.events = EPOLLIN;
  assert(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &ev) == 0);
  assert(pthread_create(&t1, NULL, waiter, NULL) == 0);
  assert(pthread_create(&t2, NULL, waiter, NULL) == 0);
  assert(write(p[1], "c", 1) == 1);
  settle(2);
  assert(pthread_join(t1, NULL) == 0);
  assert(pthread_join(t2, NULL) == 0);

  // EPOLLONESHOT: first write wakes exactly one waiter, the second wakes none
  // (disarmed), and re-arming via MOD delivers the buffered level to the other.
  drain(p[0], 1);
  atomic_store(&woken, 0);
  ev.events = EPOLLIN | EPOLLONESHOT;
  assert(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &ev) == 0);
  assert(pthread_create(&t1, NULL, waiter, NULL) == 0);
  assert(pthread_create(&t2, NULL, waiter, NULL) == 0);
  assert(write(p[1], "d", 1) == 1);
  settle(1);
  assert(write(p[1], "e", 1) == 1);
  settle(1);
  assert(epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &ev) == 0);
  settle(2);
  assert(pthread_join(t1, NULL) == 0);
  assert(pthread_join(t2, NULL) == 0);

  // Oneshot fairness: rounds of NPIPES oneshot events against two racing
  // harvesters; every round delivers each registration exactly once.
  int dp[NPIPES][2], qp[2];
  for (int i = 0; i < NPIPES; i++) {
    assert(pipe(dp[i]) == 0);
    ev.events = EPOLLIN | EPOLLONESHOT;
    ev.data.u32 = i;
    assert(epoll_ctl(ep, EPOLL_CTL_ADD, dp[i][0], &ev) == 0);
  }
  assert(pipe(qp) == 0);
  ev.events = EPOLLIN;
  ev.data.u32 = QUIT;
  assert(epoll_ctl(ep, EPOLL_CTL_ADD, qp[0], &ev) == 0);
  assert(pthread_create(&t1, NULL, harvester, NULL) == 0);
  assert(pthread_create(&t2, NULL, harvester, NULL) == 0);
  for (int round = 1; round <= 3; round++) {
    for (int i = 0; i < NPIPES; i++) {
      assert(write(dp[i][1], "x", 1) == 1);
    }
    while (atomic_load(&total) < round * NPIPES) usleep(1000);
    usleep(10000);
    assert(atomic_load(&total) == round * NPIPES); // none duplicated
    for (int i = 0; i < NPIPES; i++) {
      assert(atomic_load(&counts[i]) == round); // each exactly once
      drain(dp[i][0], 1);
      ev.events = EPOLLIN | EPOLLONESHOT;
      ev.data.u32 = i;
      assert(epoll_ctl(ep, EPOLL_CTL_MOD, dp[i][0], &ev) == 0); // re-arm
    }
  }
  assert(write(qp[1], "q", 1) == 1); // level: releases both harvesters
  assert(pthread_join(t1, NULL) == 0);
  assert(pthread_join(t2, NULL) == 0);

  printf("done\n");
  return 0;
}
