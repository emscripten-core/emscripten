/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <sys/epoll.h>

#ifdef __cplusplus
extern "C" {
#endif

// EXPERIMENTAL. This API is new and may change (signature or semantics) over the
// next few releases; it is not yet covered by Emscripten's stability guarantees.
//
// Register a persistent readiness callback on an existing epoll fd (built with
// epoll_create1/epoll_ctl): instead of blocking in epoll_wait, the runtime calls
// `callback` every time the set makes progress, delivering up to `maxevents`
// ready events. An epoll is a long-lived readiness aggregator, so the interest is
// armed once and reused across every delivery - no per-spin re-arming. Unlike
// epoll_wait it never blocks the calling stack, so it works without ASYNCIFY/JSPI.
// The callback is delivered on the calling thread's event loop: with pthreads the
// epoll readiness is tracked on the thread that owns the filesystem (the syscalls
// are proxied there), but each delivery is dispatched back to the thread that
// registered the callback.
//
// While armed it keeps the runtime alive only as long as it can still fire - i.e.
// while the epoll has at least one open watched fd. Once every watched fd is
// closed the set is terminal (it can never become ready again) and the callback
// stops holding the runtime, so no explicit disposal is required in that case.
// To dispose while open fds remain, either pass a NULL `callback` (any
// `maxevents`) to unregister, or close the epoll fd. There is at most one
// callback per epoll: calling again replaces it (it does not stack). `events` is
// a runtime-owned buffer valid only for the duration of each callback. Returns 0,
// or a positive errno (EBADF if `epfd` is not an epoll fd, EINVAL).
//
// Each registration's trigger mode (set per-fd via epoll_ctl) controls how often
// the callback fires for it - identically to epoll_wait, so one callback can mix
// modes:
//   - Level-triggered (the default): the callback fires on the next tick whenever
//     the fd is ready, and keeps firing while it stays ready. The runtime - not
//     the application - drives the loop, so an fd that is structurally always
//     ready and never drained (notably EPOLLOUT on a writable socket) will spin
//     the event loop. Use one of the modes below for such fds.
//   - EPOLLET (edge-triggered): the callback fires once per readiness edge and
//     not again until a fresh edge. Drain the fd fully on each delivery; this is
//     the way to watch an always-writable fd without spinning.
//   - EPOLLONESHOT: the callback fires once for that fd, then the registration is
//     disabled until you re-arm it with epoll_ctl(EPOLL_CTL_MOD). Use it to
//     handle an fd exactly once (e.g. before handing it elsewhere).
typedef void (*em_epoll_callback)(int epfd, struct epoll_event *events, int nready, void *userdata);
int emscripten_epoll_set_callback(int epfd, int maxevents, em_epoll_callback callback, void *userdata);

#ifdef __cplusplus
}
#endif
