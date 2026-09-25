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
// ASYNCIFY/JSPI.
//
// Listeners must be added from, and are delivered on, the main thread; with
// pthreads, a call from any other thread fails with ENOTSUP.
//
// Any number of listeners may be added, identified by the (callback, userdata)
// pair; adding a pair that is already registered fails with EEXIST. Every
// listener is signalled while uncollected ready events remain (broadcast), and
// listeners race to collect: per-fd trigger modes distribute events across
// collectors exactly as between multiple blocking epoll_wait callers on one
// epoll, so an EPOLLET edge or an EPOLLONESHOT firing is collected by exactly
// one listener (load balancing), while a level fd keeps signalling every
// listener until drained.
//
// A listener fires once the call that made the set ready has returned (as a
// microtask: never from within a running wasm call, and before the host's next
// turn) while the set has ready events that have not yet been collected, and keeps firing while any remain: it only signals that
// events are pending, so a callback that does not drain them (via epoll_wait)
// leaves them pending and re-fires. Whether a given fd is re-reported follows
// its per-fd trigger mode (set via epoll_ctl) exactly as epoll_wait does. Note
// that for a level-triggered fd the runtime, not the application, drives the
// loop, so an fd that is structurally always ready (notably EPOLLOUT on a
// writable socket) will spin the event loop; use EPOLLET or EPOLLONESHOT for
// such fds.
//
// A listener is an unref'd handle (like Node's handle.unref()): while the
// runtime is alive, readiness is delivered to it, but it never keeps the
// runtime alive by itself. A program whose only reason to stay alive is a
// listener holds the runtime itself:
//
//   emscripten_runtime_keepalive_push();  // e.g. before main() returns
//   ...
//   emscripten_runtime_keepalive_pop();   // e.g. from the callback, when done
//
// Likewise emscripten_epoll_remove_listener and the last close of the epoll fd
// release nothing, since nothing was held.
//
// Listeners are shared instance state: they see registrations made through any
// dup'd fd, and closing the last fd to the instance removes them all. Returns
// 0, or a positive errno (EBADF if `epfd` is not an epoll fd).
typedef void (*em_epoll_callback)(void *userdata);
int emscripten_epoll_add_listener(int epfd, em_epoll_callback callback, void *userdata);

// Remove the listener for the (callback, userdata) pair. Returns 0, EBADF if
// `epfd` is not an epoll fd, or ENOENT if no such listener is registered.
int emscripten_epoll_remove_listener(int epfd, em_epoll_callback callback, void *userdata);

#ifdef __cplusplus
}
#endif
