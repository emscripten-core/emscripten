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
// next few releases.
//
// Register a persistent readiness listener on an existing epoll fd (built with
// epoll_create1/epoll_ctl): instead of blocking in epoll_wait, the runtime
// invokes `callback` on the event loop whenever the epoll set has ready events
// waiting to be collected. The callback receives only `userdata`; it does not
// receive the events. To collect them it calls epoll_wait(epfd, ..., 0) itself
// - a non-blocking, zero-timeout wait - from within the callback (or later).
// Unlike epoll_wait it never blocks the calling stack, so it works without
// ASYNCIFY/JSPI. The callback is delivered on the registering thread's event
// loop: with pthreads the epoll readiness is tracked on the main thread (the
// syscalls are proxied there), but each delivery is dispatched back to the
// thread that added the listener.
//
// Any number of listeners may be added, from any threads, identified by the
// (callback, registering thread) pair; re-adding the same identity just updates
// `userdata`. Every listener is signalled while uncollected ready events remain
// (broadcast), and listeners race to collect: per-fd trigger modes distribute
// events across collectors exactly as between multiple blocking epoll_wait
// callers on one epoll, so an EPOLLET edge or an EPOLLONESHOT firing is
// collected by exactly one listener (load balancing), while a level fd keeps
// signalling every listener until drained.
//
// A listener fires on the next event-loop tick while the set has ready events
// that have not yet been collected, and keeps firing while any remain - it only
// signals that events are pending, so a callback that does not drain them (via
// epoll_wait) leaves them pending and re-fires. Whether a given fd is
// re-reported follows its per-fd trigger mode (set via epoll_ctl) exactly as
// epoll_wait does, so one epoll can mix modes:
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
// Listeners keep the runtime alive as long as the set can still fire - i.e.
// while the epoll has at least one open watched fd. This follows the Node.js
// model, where registered I/O interest holds the event loop open. Once every
// watched fd is closed the set is terminal (it can never become ready again)
// and its listeners stop holding the runtime, so no explicit disposal is
// required in that case.
//
// Listeners are shared instance state: they see registrations made through any
// dup'd fd, and closing the last fd to the instance removes them all. Returns
// 0, or a positive errno (EBADF if `epfd` is not an epoll fd).
typedef void (*em_epoll_callback)(void *userdata);
int emscripten_epoll_add_listener(int epfd, em_epoll_callback callback, void *userdata);

// Remove the calling thread's listener for `callback`. Returns 0, EBADF if
// `epfd` is not an epoll fd, or ENOENT if no such listener is registered.
int emscripten_epoll_remove_listener(int epfd, em_epoll_callback callback);

#ifdef __cplusplus
}
#endif
