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
// epoll_create1/epoll_ctl): instead of blocking in epoll_wait, the runtime
// invokes `callback` on the event loop whenever the epoll set has ready events
// waiting to be collected. The callback receives only `userdata`; it does not
// receive the events. To collect them it calls epoll_wait(epfd, ..., 0) itself
// - a non-blocking, zero-timeout wait - from within the callback (or later).
// Unlike epoll_wait it never blocks the calling stack, so it works without
// ASYNCIFY/JSPI. The callback is delivered on the calling thread's event loop:
// with pthreads the epoll readiness is tracked on the thread that owns the
// filesystem (the syscalls are proxied there), but each delivery is dispatched
// back to the thread that registered the callback.
//
// The callback fires on the next event-loop tick while the set has ready events
// that have not yet been collected, and keeps firing while any remain - it only
// signals that events are pending, so a callback that does not drain them (via
// epoll_wait) leaves them pending and re-fires. Whether a given fd is re-reported
// follows its per-fd trigger mode (set via epoll_ctl) exactly as epoll_wait does,
// so one callback can mix modes:
//   - Level-triggered (the default): the fd is reported on the next tick whenever
//     it is ready, and keeps re-firing while it stays ready. The runtime - not
//     the application - drives the loop, so an fd that is structurally always
//     ready (notably EPOLLOUT on a writable socket) will spin the event loop.
//     Use one of the modes below for such fds.
//   - EPOLLET (edge-triggered): reported once per readiness edge and not again
//     until a fresh edge; usually preferable in this model.
//   - EPOLLONESHOT: reported once, then the registration is disabled until you
//     re-arm it with epoll_ctl(EPOLL_CTL_MOD).
//
// While armed it keeps the runtime alive only as long as it can still fire - i.e.
// while the epoll has at least one open watched fd. Once every watched fd is
// closed the set is terminal (it can never become ready again) and the callback
// stops holding the runtime, so no explicit disposal is required in that case.
// To dispose while open fds remain, either pass a NULL `callback` to unregister,
// or close the epoll fd. There is at most one callback per epoll: calling again
// replaces it (it does not stack). Returns 0, or a positive errno (EBADF if
// `epfd` is not an epoll fd).
typedef void (*em_epoll_callback)(void *userdata);
int emscripten_epoll_set_callback(int epfd, em_epoll_callback callback, void *userdata);

#ifdef __cplusplus
}
#endif
