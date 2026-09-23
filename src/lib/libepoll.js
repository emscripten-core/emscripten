/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// epoll(7) for the JS filesystem. The epoll syscalls build on the per-inode
// readiness wait-queue (FSNode.addListener/notifyListeners) and the synchronous
// readiness derivation ($pollOne) defined in libsyscall.js.

var EpollLibrary = {
  // An epoll instance's state lives on the stream's `shared` object - the open
  // file description (Linux's struct file / eventpoll) that dup'd fds share. It
  // carries an interest map `epoll` (fd -> reg) and a ready list
  // (rdlHead/rdlTail). Each registration arms a persistent listener on the
  // watched node's wait-queue at EPOLL_CTL_ADD (not per-wait), feeding the ready
  // list on each edge so readiness can be tracked across waits and up a nesting
  // chain. dup(2) yields another fd to the SAME instance (registrations and
  // ready list shared); close(2) drops one reference and only the last close
  // reclaims it (tearing every registration down). An epoll fd can itself be
  // added to another epoll.

  // Would a wait on this epoll block - i.e. does no listed registration have a
  // genuine ready event? Walks the ready list (O(ready)), masking out the
  // reporting-time flags (edge/oneshot/exclusive), and evicts a closed/reused fd
  // as it goes (so a set only ever probed, never drained, does not accumulate
  // dead registrations). This is the readiness derivation behind the epoll fd's
  // own poll handler (nesting): a stale ready-list entry (a spurious edge, or
  // one left after its fd was drained then closed) is not a ready event, so it
  // never reports one.
  $epollWouldBlock__internal: true,
  $epollWouldBlock__deps: ['$FS', '$pollOne', '$epollEvict'],
  $epollWouldBlock: (ep) => {
    for (var reg = ep.rdlHead, next; reg; reg = next) {
      next = reg.rdlNext;
      if (FS.getStream(reg.fd)?.shared !== reg.shared) {
        epollEvict(ep, reg);
        continue;
      }
      if (pollOne(reg.fd, reg.events & ~{{{ cDefs.EPOLLET | cDefs.EPOLLONESHOT | cDefs.EPOLLEXCLUSIVE }}})) {
        return false;
      }
    }
    return true;
  },

  $epollNewInstance__internal: true,
  $epollNewInstance__deps: ['$FS', '$epollWouldBlock', '$epollClearListener'],
  $epollNewInstance: () => {
    // Its own (detached) node, so the epoll fd can be watched by a parent epoll
    // (nesting) and carry the readiness wait-queue methods. Shared across dups.
    var node = new FS.FSNode(0, '', 0, 0);
    var stream = FS.createStream({
      node,
      stream_ops: {
        // Readable when any listed registration is currently ready: this is what
        // lets an epoll fd be polled/nested.
        poll(stream) {
          return epollWouldBlock(stream.shared) ? 0 : {{{ cDefs.POLLIN }}};
        },
        // dup(2): another fd to the same epoll instance (Linux: another reference
        // to the eventpoll). The instance state lives on the shared open file
        // description, already propagated by reference to the dup'd stream, so
        // there is nothing to copy - just count the new reference.
        dup(stream) {
          stream.shared.refcount++;
        },
        // close(2): drop one reference. Only the last close reclaims the
        // instance: drop every registration's listener (a fired EPOLLONESHOT has
        // already dropped its own) from its watched node. A surviving dup keeps
        // it all live.
        close(stream) {
          var ep = stream.shared;
          // FS.close already fired POLLNVAL on the (shared) node, waking any
          // parent epoll watching this fd so it re-derives and drops the
          // now-stale registration (via doEpollWait's shared check).
          if (--ep.refcount) return;
          for (var it of ep.interests.values()) epollClearListener(ep, it);
          for (var reg of ep.epoll.values()) {
            reg.listener?.listeners.delete(reg.listener.entry);
          }
          ep.epoll.clear();
        },
      },
    });
    // Hoist the instance state onto `shared` so every dup observes one instance.
    Object.assign(stream.shared, {
      node,
      epoll: new Map(),
      interests: new Map(), // emscripten_epoll_add_listener listeners
      // Open references (fds) to this instance; the last close reclaims it.
      refcount: 1,
    });
    return stream;
  },

  // The ready list (Linux's rdllist): registrations whose readiness edge has
  // fired but not yet been consumed by a wait, linked intrusively through
  // reg.rdlPrev/reg.rdlNext with head/tail on the epoll stream. Membership
  // (reg.onList) is the edge state - a reg is listed on an edge (or when seeded
  // ready at ctl), removed when a wait consumes it, and re-listed at the tail if
  // a level trigger is still ready. O(1) add/remove, O(delivered) to drain.
  $readyListAdd__internal: true,
  $readyListAdd: (ep, reg) => {
    if (reg.onList) return;
    reg.onList = true;
    reg.rdlPrev = ep.rdlTail;
    reg.rdlNext = null;
    if (ep.rdlTail) ep.rdlTail.rdlNext = reg;
    else ep.rdlHead = reg;
    ep.rdlTail = reg;
  },
  $readyListRemove__internal: true,
  $readyListRemove: (ep, reg) => {
    if (!reg.onList) return;
    reg.onList = false;
    if (reg.rdlPrev) reg.rdlPrev.rdlNext = reg.rdlNext;
    else ep.rdlHead = reg.rdlNext;
    if (reg.rdlNext) reg.rdlNext.rdlPrev = reg.rdlPrev;
    else ep.rdlTail = reg.rdlPrev;
    reg.rdlPrev = reg.rdlNext = null;
  },

  // Remove a registration from its epoll: off the ready list, unlink its
  // watched-node listener (a fired EPOLLONESHOT has none), and drop it from the
  // interest map. The single eviction primitive, used by EPOLL_CTL_DEL, a stale
  // entry at ctl time, and a closed/reused fd seen at derive time (doEpollWait
  // or the nesting poll).
  $epollEvict__internal: true,
  $epollEvict__deps: ['$readyListRemove'],
  $epollEvict: (ep, reg) => {
    readyListRemove(ep, reg);
    reg.listener?.listeners.delete(reg.listener.entry);
    reg.listener = null;
    ep.epoll.delete(reg.fd);
  },

  // The heavy lifting behind the epoll syscalls. The `__syscall_epoll_*` entry
  // points stay in libsyscall.js (like every other syscall) and resolve the
  // epoll stream before calling in here, so `ep` is a known-valid epoll stream.
  $epollCtl__internal: true,
  $epollCtl__deps: ['$FS', '$pollOne', '$readyListAdd', '$epollEvict'],
  $epollCtl: (ep, op, fd, ev) => {
    var target = FS.getStream(fd);
    if (!target) return -{{{ cDefs.EBADF }}};
    if (op != {{{ cDefs.EPOLL_CTL_ADD }}} && op != {{{ cDefs.EPOLL_CTL_MOD }}} && op != {{{ cDefs.EPOLL_CTL_DEL }}}) {
      return -{{{ cDefs.EINVAL }}};
    }
    // An epoll cannot watch itself (via any fd referring to the same instance).
    if (target.shared === ep) return -{{{ cDefs.EINVAL }}};

    // A registration keys on the open file description (stream.shared) - the
    // struct-file analog that dup'd fds share. If this fd's number now resolves
    // to a different open (closed and the slot reused), the old registration is
    // stale: evict it so ctl sees the fd as fresh, matching Linux's eviction of
    // the epitem when the watched file is released.
    var cur = ep.epoll.get(fd);
    if (cur && target.shared !== cur.shared) {
      epollEvict(ep, cur); // stale: this fd number is now a different open
      cur = undefined;
    }
    var has = !!cur;
    if (op == {{{ cDefs.EPOLL_CTL_DEL }}}) {
      if (!has) return -{{{ cDefs.ENOENT }}};
      epollEvict(ep, cur);
      return 0;
    }

    var events = {{{ makeGetValue('ev', C_STRUCTS.epoll_event.events, 'u32') }}};
    if (op == {{{ cDefs.EPOLL_CTL_ADD }}}) {
      if (has) return -{{{ cDefs.EEXIST }}};
      // Only descriptors with a readiness derivation can be epoll-watched
      // (sockets/pipes/epoll itself). Regular files have no poll handler and so
      // are not epoll-capable, matching Linux (-EPERM).
      if (!target.stream_ops?.poll) return -{{{ cDefs.EPERM }}};
      // Nesting another epoll: reject cycles, and chains deeper than 5 levels of
      // epoll (ELOOP) - the Linux cap is EP_MAX_NESTS (4) plus the leaf level.
      if (target.shared.epoll) {
        // Walk streams but key the graph on instances (stream.shared), so dup'd
        // fds of one epoll count as a single node.
        var reaches = (from, goal, seen) => {
          var inst = from?.shared;
          if (inst === goal) return true;
          if (!inst?.epoll || seen.has(inst)) return false;
          seen.add(inst);
          for (var f of inst.epoll.keys()) {
            if (reaches(FS.getStream(f), goal, seen)) return true;
          }
          return false;
        };
        var depth = (from, seen) => {
          var inst = from?.shared;
          if (!inst?.epoll || seen.has(inst)) return 0;
          seen.add(inst);
          var max = 0;
          for (var f of inst.epoll.keys()) max = Math.max(max, depth(FS.getStream(f), seen));
          seen.delete(inst);
          return 1 + max;
        };
        if (reaches(target, ep, new Set()) || 1 + depth(target, new Set()) > 5) {
          return -{{{ cDefs.ELOOP }}};
        }
      }
    } else { // EPOLL_CTL_MOD
      if (!has) return -{{{ cDefs.ENOENT }}};
      // An EPOLLEXCLUSIVE registration cannot be modified, and EPOLLEXCLUSIVE
      // may only be set at ADD time.
      if ((events | cur.events) & {{{ cDefs.EPOLLEXCLUSIVE }}}) return -{{{ cDefs.EINVAL }}};
    }

    // `data` is opaque user data echoed back by epoll_wait; keep its 8 bytes as
    // an i32 pair so this also works without WASM_BIGINT (e.g. wasm2js).
    var reg = cur ?? {};
    reg.fd = fd;
    reg.shared = target.shared; // open file description: the dup-shared identity
    reg.events = events;
    reg.dataLo = {{{ makeGetValue('ev', C_STRUCTS.epoll_event.data, 'i32') }}};
    reg.dataHi = {{{ makeGetValue('ev', C_STRUCTS.epoll_event.data + 4, 'i32') }}};
    if (op == {{{ cDefs.EPOLL_CTL_ADD }}}) ep.epoll.set(fd, reg);
    // The registration's listener is its edge in the interest graph - present
    // only while armed, so a watched node fires nothing for a dead edge. ADD
    // installs it; a fired EPOLLONESHOT dropped it, so a MOD re-arm reinstalls it.
    // (ep_poll_callback: on an edge, list the reg and wake any waiter on this
    // epoll - and through ep.node any parent epoll nesting it.)
    if (!reg.listener) {
      reg.listener = target.node.addListener((flags) => {
        readyListAdd(ep, reg);
        // A closing fd (POLLNVAL) wakes the epoll as a teardown, not readiness.
        ep.node.notifyListeners(flags & {{{ cDefs.POLLNVAL }}} ? {{{ cDefs.POLLNVAL }}} : {{{ cDefs.POLLIN }}});
      // EPOLLEXCLUSIVE: when one fd is watched by several epolls, the watched
      // node wakes only one of them per edge (round-robin), not all.
      }, !!(events & {{{ cDefs.EPOLLEXCLUSIVE }}}));
    }
    // Arming is itself an event source (ep_insert/ep_modify): a source-based
    // model only learns readiness from edges, so sample the level now - the
    // (re-)armed fd may already be ready with no producer notify to follow.
    if (pollOne(fd, reg.events & ~{{{ cDefs.EPOLLET | cDefs.EPOLLONESHOT | cDefs.EPOLLEXCLUSIVE }}})) {
      readyListAdd(ep, reg);
      ep.node.notifyListeners({{{ cDefs.POLLIN }}});
    }
    return 0;
  },

  // Consume the ready list (Linux's ep_send_events), writing up to `maxevents`
  // epoll_events into `ev` and returning the count. Each listed registration is
  // re-derived against its current mask: level-triggered ones still ready are
  // re-listed at the tail; edge-triggered ones leave the list until the next
  // edge; EPOLLONESHOT ones drop their watched-node listener until re-armed by
  // EPOLL_CTL_MOD; a no-longer-ready (spurious) edge is dropped; a closed/reused
  // fd is evicted.
  $doEpollWait__internal: true,
  $doEpollWait__deps: ['$FS', '$pollOne', '$readyListAdd', '$epollEvict'],
  $doEpollWait: (ep, ev, maxevents) => {
    // Detach the list and drain from the head: re-armed level triggers and the
    // unprocessed remainder go back onto ep's now-empty list, so a single pass
    // never revisits an entry. O(delivered), not O(registered).
    var node = ep.rdlHead, tail = ep.rdlTail;
    ep.rdlHead = ep.rdlTail = null;
    var n = 0;
    while (node && n < maxevents) {
      var next = node.rdlNext;
      node.onList = false;
      node.rdlPrev = node.rdlNext = null;
      var fd = node.fd;
      if (FS.getStream(fd)?.shared !== node.shared) {
        // The fd closed, or its number was reused for a different open: evict the
        // now-stale registration (a surviving dup keeps the open file alive).
        // Already detached from the list above, so epollEvict just unlinks the
        // listener and drops it from the map.
        epollEvict(ep, node);
      } else {
        var revents = pollOne(fd, node.events & ~{{{ cDefs.EPOLLET | cDefs.EPOLLONESHOT | cDefs.EPOLLEXCLUSIVE }}});
        if (revents) {
          var out = ev + {{{ C_STRUCTS.epoll_event.__size__ }}} * n;
          {{{ makeSetValue('out', C_STRUCTS.epoll_event.events, 'revents', 'u32') }}};
          {{{ makeSetValue('out', C_STRUCTS.epoll_event.data, 'node.dataLo', 'i32') }}};
          {{{ makeSetValue('out', C_STRUCTS.epoll_event.data + 4, 'node.dataHi', 'i32') }}};
          n++;
          if (node.events & {{{ cDefs.EPOLLONESHOT }}}) {
            // Fired: a dead edge until EPOLL_CTL_MOD re-arms it, so drop its
            // listener - the watched node stops poking it (no re-arm needed).
            node.listener.listeners.delete(node.listener.entry);
            node.listener = null;
          } else if (!(node.events & {{{ cDefs.EPOLLET }}})) {
            readyListAdd(ep, node); // level: re-list at tail
          }
        }
        // else: a spurious edge (no longer ready) - drop it from the list.
      }
      node = next;
    }
    // Stopped at maxevents with entries left: splice the unprocessed remainder
    // (node..tail) back to the FRONT, ahead of any re-armed items, so the next
    // wait services them first (round-robin fairness).
    if (node) {
      node.rdlPrev = null;
      tail.rdlNext = ep.rdlHead;
      if (ep.rdlHead) ep.rdlHead.rdlPrev = tail;
      else ep.rdlTail = tail;
      ep.rdlHead = node;
    }
    return n;
  },

  // The blocking wait behind __syscall_epoll_pwait; `ep` is a known-valid epoll
  // stream and `maxevents` already validated by the entry point.
  $epollPwait__internal: true,
  $epollPwait__deps: ['$doEpollWait'],
  $epollPwait: (ep, ev, maxevents, timeout) => {
#if PTHREADS || ASYNCIFY
#if PTHREADS
    const isAsyncContext = PThread.currentProxiedOperationCallerThread;
#else
    const isAsyncContext = true;
#endif
    // Always resolve through a Promise here: when proxied from a worker the
    // result is delivered by promise resolution, so a bare value would break
    // the proxy (it has no `.then`). Block on the epoll's own readiness - each
    // registration's persistent listener wakes ep.node on a leaf edge - and
    // re-derive on wake, resolving the count or 0 after `timeout`.
    if (isAsyncContext) {
      return new Promise((resolve) => {
        var count = doEpollWait(ep, ev, maxevents);
        if (count || !timeout) {
          resolve(count);
          return;
        }
        var done = false;
        var reg = ep.node.addListener(() => {
          if (done) return;
          var c = doEpollWait(ep, ev, maxevents);
          if (c) finish(c);
        });
        var timer = timeout > 0 ? setTimeout(() => finish(0), timeout) : undefined;
        function finish(c) {
          if (done) return;
          done = true;
          reg.listeners.delete(reg.entry);
          if (timer) clearTimeout(timer);
          resolve(c);
        }
      });
    }
#endif
    var count = doEpollWait(ep, ev, maxevents);
#if ASSERTIONS
    if (!count && timeout != 0) warnOnce('non-zero epoll_wait() timeout not supported: ' + timeout)
#endif
    return count;
  },

  $epollClearListener__internal: true,
  $epollClearListener: (ep, it) => {
    ep.interests.delete(it.key);
    it.cleared = true;
    it.listener.listeners.delete(it.listener.entry);
  },

  // See <emscripten/epoll.h>. A listener is keyed by (registering thread,
  // callback), signals the callback while the set has uncollected ready events,
  // and holds nothing itself: the only keepalive taken is for a scheduled
  // delivery, which is pending work like a safeSetTimeout callback.
  emscripten_epoll_add_listener__deps: ['$FS', '$epollWouldBlock', '$epollClearListener', '$callUserCallback', '$emSetImmediate',
#if !MINIMAL_RUNTIME
    '$maybeExit',
#endif
#if PTHREADS
    '$epollDeliveries', '_emscripten_epoll_run_callback_on_thread',
#endif
  ],
  emscripten_epoll_add_listener__proxy: 'sync',
  emscripten_epoll_add_listener: (epfd, callback, userdata) => {
    var stream = FS.getStream(epfd);
    // A public API, not a syscall: positive errno.
    if (!stream?.shared.epoll) return {{{ cDefs.EBADF }}};
    var ep = stream.shared;
#if PTHREADS
    // Runs on the main thread; deliveries are back-proxied to the registering
    // thread (0 = the main thread itself).
    var callerThread = PThread.currentProxiedOperationCallerThread;
    var key = callerThread + ':' + callback;
#else
    var key = callback;
#endif
    var prev = ep.interests.get(key);
    if (prev) epollClearListener(ep, prev);
    var it = {key};
    ep.interests.set(key, it);

    // Hold the runtime for the delivery scheduled on the next turn. Not once
    // FS.quit has begun: no delivery can follow and the hold would outlive the
    // exit.
    function hold() {
      if (FS.initialized && !it.held) {
        it.held = true;
        {{{ runtimeKeepalivePush() }}}
      }
    }
    // Runs the callback; resolves once it has completed. Takes the hold for
    // the next turn if the set is still ready (undrained, or a re-listed level
    // fd), on the local path before callUserCallback's maybeExit.
    function deliver() {
#if PTHREADS
      if (callerThread) {
        return new Promise((resolve) => {
          var token = epollDeliveries.nextToken++;
          epollDeliveries[token] = resolve;
          // The owner thread is gone: the listener dies with it.
          if (!__emscripten_epoll_run_callback_on_thread(callerThread, callback, userdata, token)) {
            epollClearListener(ep, it);
            delete epollDeliveries[token];
            resolve();
          }
        }).then(() => {
          if (!it.cleared && !epollWouldBlock(ep)) hold();
        });
      }
#endif
      callUserCallback(() => {
        {{{ makeDynCall('vp', 'callback') }}}(userdata);
        if (!it.cleared && !epollWouldBlock(ep)) hold();
      });
    }
    // Each delivery is a macrotask (a microtask could run re-entrantly: hosts
    // drain microtasks inside other calls, e.g. Node's module loader on a first
    // builtin load; and a still-ready level fd would never yield to I/O), so
    // synchronous producer notifies coalesce into the one pending turn. A
    // readiness wake holds the runtime for that turn; a teardown wake (POLLNVAL)
    // does not. One delivery is in flight at a time: a cross-thread one is
    // paced by its completion, else a still-ready level fd would be
    // re-signalled in a tight spin while the owner drains it.
    async function wake(held) {
      if (held) hold();
      if (it.running) return;
      it.running = true;
      do {
        await new Promise(emSetImmediate);
        if (it.held) {
          it.held = false;
          {{{ runtimeKeepalivePop() }}}
        }
        if (it.cleared || epollWouldBlock(ep)) {
          it.running = false;
          // Not delivering: callUserCallback's maybeExit will not run, and
          // the hold just released may have been what deferred main's exit.
#if !MINIMAL_RUNTIME
          maybeExit();
#endif
          return;
        }
        await deliver();
        // A local delivery ran maybeExit; a cross-thread one takes another turn
        // for it.
      } while (it.held
#if PTHREADS
        || callerThread
#endif
      );
      it.running = false;
    }
    it.listener = ep.node.addListener((flags) => wake(!(flags & {{{ cDefs.POLLNVAL }}})));
    wake(!epollWouldBlock(ep));
    return 0;
  },

  emscripten_epoll_remove_listener__deps: ['$FS', '$epollClearListener'],
  emscripten_epoll_remove_listener__proxy: 'sync',
  emscripten_epoll_remove_listener: (epfd, callback) => {
    var stream = FS.getStream(epfd);
    if (!stream?.shared.epoll) return {{{ cDefs.EBADF }}};
    var ep = stream.shared;
#if PTHREADS
    var key = PThread.currentProxiedOperationCallerThread + ':' + callback;
#else
    var key = callback;
#endif
    var it = ep.interests.get(key);
    if (!it) return {{{ cDefs.ENOENT }}};
    epollClearListener(ep, it);
    return 0;
  },

#if PTHREADS
  // Token -> completion of an in-flight cross-thread delivery.
  $epollDeliveries: {nextToken: 1},

  _emscripten_epoll_delivery_done__deps: ['$epollDeliveries'],
  _emscripten_epoll_delivery_done: (token) => {
    var resolve = epollDeliveries[token];
    delete epollDeliveries[token];
    resolve?.();
  },
#endif
};

addToLibrary(EpollLibrary);
