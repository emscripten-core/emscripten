/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// epoll(7) for the JS filesystem. The epoll syscalls and the
// emscripten_epoll_set_callback extension build on the per-inode readiness
// wait-queue (FSNode.addListener/notifyListeners) and the synchronous readiness
// derivation ($pollOne) defined in libsyscall.js.

var EpollLibrary = {
  // An epoll instance is a real FS fd whose stream carries an interest map
  // `epoll` (fd -> reg) and a ready list (rdlHead/rdlTail). Each registration
  // arms a persistent listener on the watched node's wait-queue at EPOLL_CTL_ADD
  // (not per-wait), feeding the ready list on each edge so readiness can be
  // tracked across waits and up a nesting chain. Being an fd, close(2) reclaims
  // it (tearing every registration down) and it can itself be added to another
  // epoll.
  $newEpollInstance__internal: true,
  $newEpollInstance__deps: ['$FS', '$pollOne', '$clearEpollInterest', '$reconcileEpollKeepalive', '$epollEvict'],
  $newEpollInstance: () => FS.createStream({
    // Its own (detached) node, so the epoll fd can be watched by a parent epoll
    // (nesting) and carry the readiness wait-queue methods.
    node: new FS.FSNode(0, 'epoll', 0, 0),
    epoll: new Map(),
    // Count of armed registrations: those with a live watched-node listener. A
    // fired EPOLLONESHOT drops its listener (so it stops counting until a MOD
    // re-arm), a plain registration counts until evicted. This - not the raw
    // interest-map size - is what the callback keepalive keys on: a set with no
    // armed registration can never fire again on its own, so it is terminal.
    armed: 0,
    stream_ops: {
      // Readable when any listed registration is currently ready: this is what
      // lets an epoll fd be polled/nested. Walks only the ready list (O(ready));
      // edge/oneshot/exclusive are reporting-time concerns, masked out here. A
      // closed/reused fd is evicted here too, so a nested epoll that is only ever
      // polled (never directly waited) does not accumulate dead registrations.
      poll(stream) {
        for (var reg = stream.rdlHead, next; reg; reg = next) {
          next = reg.rdlNext;
          if (FS.getStream(reg.fd)?.shared !== reg.shared) { epollEvict(stream, reg); continue; }
          if (pollOne(reg.fd, reg.events & ~{{{ cDefs.EPOLLET | cDefs.EPOLLONESHOT | cDefs.EPOLLEXCLUSIVE }}})) {
            return {{{ cDefs.POLLIN }}};
          }
        }
        return 0;
      },
      // close(2): drop the readiness callback interest (if any), then every
      // registration's listener (a fired EPOLLONESHOT has already dropped its
      // own) from its watched node.
      close(stream) {
        // FS.close already fires POLLNVAL on this node, waking any parent epoll
        // watching this epoll fd so it re-derives and drops the now-stale
        // registration (via doEpollWait's shared check).
        clearEpollInterest(stream);
        reconcileEpollKeepalive(stream); // drop the keepalive if it was held
        for (var reg of stream.epoll.values()) {
          reg.listener?.listeners.delete(reg.listener.entry);
        }
        stream.epoll.clear();
      },
    },
  }),

  // Drop an epoll's persistent readiness callback interest: remove its listener
  // on the epoll node and free the output buffer. Keepalive is managed by the
  // caller (popped on clear/close, kept on replace).
  $clearEpollInterest__internal: true,
  $clearEpollInterest__deps: ['free'],
  $clearEpollInterest: (ep) => {
    var it = ep.interest;
    if (!it) return;
    ep.interest = null;
    it.listener.listeners.delete(it.listener.entry);
    _free(it.buf);
#if PTHREADS
    // Retire its delivery token; a still-in-flight cross-thread delivery whose
    // completion arrives after this finds nothing and is dropped.
    if (it.token) delete epollDeliveries[it.token];
#endif
  },

  // A registered callback keeps the runtime alive only while it can still fire -
  // i.e. while the epoll has at least one armed registration. Once every watched
  // fd is closed (or every one-shot has fired) the set is terminal, so the
  // keepalive is dropped and the runtime may exit. Reconciled after any change to
  // the callback or the armed count.
  //
  // Two threads must be held while armed under pthread proxy: this (the FS/main
  // thread) runs the readiness derivation and delivery scheduling, and the owner
  // thread (ep.ownerThread) both runs the callback and must survive to receive
  // it. So push/pop the local keepalive AND, for a distinct owner worker, proxy
  // the same push/pop onto it. keepaliveThread records where the owner push
  // landed so a later handover (a replace from another thread) pops the right one.
  $reconcileEpollKeepalive__internal: true,
  $reconcileEpollKeepalive__deps: [
#if PTHREADS
    '_emscripten_epoll_keepalive_on_thread',
#endif
  ],
  $reconcileEpollKeepalive: (ep) => {
    var want = !!ep.interest && ep.armed > 0;
    if (want == !!ep.keepalive) return;
    ep.keepalive = want;
#if PTHREADS
    // ownerThread is 0 when the FS thread itself registered - then the local
    // push/pop below already covers it.
    if (ep.ownerThread) {
      if (want) __emscripten_epoll_keepalive_on_thread(ep.keepaliveThread = ep.ownerThread, 1);
      else { __emscripten_epoll_keepalive_on_thread(ep.keepaliveThread, -1); ep.keepaliveThread = 0; }
    }
#endif
#if !MINIMAL_RUNTIME && (EXIT_RUNTIME || PTHREADS)
    if (want) { {{{ runtimeKeepalivePush() }}} } else { {{{ runtimeKeepalivePop() }}} }
#endif
  },

  // The ready list (Linux's rdllist): registrations whose readiness edge has
  // fired but not yet been consumed by a wait, linked intrusively through
  // reg.rdlPrev/reg.rdlNext with head/tail on the epoll stream. Membership
  // (reg.onList) is the edge state - a reg is listed on an edge (or when seeded
  // ready at ctl), removed when a wait consumes it, and re-listed at the tail if
  // a level trigger is still ready. O(1) add/remove, O(delivered) to drain.
  $rdllistAdd__internal: true,
  $rdllistAdd: (ep, reg) => {
    if (reg.onList) return;
    reg.onList = true;
    reg.rdlPrev = ep.rdlTail;
    reg.rdlNext = null;
    if (ep.rdlTail) ep.rdlTail.rdlNext = reg;
    else ep.rdlHead = reg;
    ep.rdlTail = reg;
  },
  $rdllistRemove__internal: true,
  $rdllistRemove: (ep, reg) => {
    if (!reg.onList) return;
    reg.onList = false;
    if (reg.rdlPrev) reg.rdlPrev.rdlNext = reg.rdlNext;
    else ep.rdlHead = reg.rdlNext;
    if (reg.rdlNext) reg.rdlNext.rdlPrev = reg.rdlPrev;
    else ep.rdlTail = reg.rdlPrev;
    reg.rdlPrev = reg.rdlNext = null;
  },

  // Remove a registration from its epoll: off the ready list, unlink its
  // watched-node listener (a fired EPOLLONESHOT has none), drop from the interest
  // map, and reconcile the callback keepalive. The single eviction primitive,
  // used by EPOLL_CTL_DEL, a stale entry at ctl time, and a closed/reused fd seen
  // at derive time (doEpollWait or the nesting poll).
  $epollEvict__internal: true,
  $epollEvict__deps: ['$rdllistRemove', '$reconcileEpollKeepalive'],
  $epollEvict: (ep, reg) => {
    rdllistRemove(ep, reg);
    // A fired EPOLLONESHOT already dropped its listener (and its armed count), so
    // only decrement for a still-armed registration.
    if (reg.listener) { reg.listener.listeners.delete(reg.listener.entry); ep.armed--; }
    ep.epoll.delete(reg.fd);
    reconcileEpollKeepalive(ep);
  },

  // The heavy lifting behind the epoll syscalls. The `__syscall_epoll_*` entry
  // points stay in libsyscall.js (like every other syscall) and resolve the
  // epoll stream before calling in here, so `ep` is a known-valid epoll stream.
  $epollCtl__internal: true,
  $epollCtl__deps: ['$FS', '$pollOne', '$rdllistAdd', '$epollEvict', '$reconcileEpollKeepalive'],
  $epollCtl: (ep, op, fd, ev) => {
    var target = FS.getStream(fd);
    if (!target) return -{{{ cDefs.EBADF }}};
    if (op != {{{ cDefs.EPOLL_CTL_ADD }}} && op != {{{ cDefs.EPOLL_CTL_MOD }}} && op != {{{ cDefs.EPOLL_CTL_DEL }}}) {
      return -{{{ cDefs.EINVAL }}};
    }
    // An epoll cannot watch itself.
    if (fd == ep.fd) return -{{{ cDefs.EINVAL }}};

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
      if (target.epoll) {
        var reaches = (from, goal, seen) => {
          if (from === goal) return true;
          if (!from?.epoll || seen.has(from)) return false;
          seen.add(from);
          for (var f of from.epoll.keys()) {
            if (reaches(FS.getStream(f), goal, seen)) return true;
          }
          return false;
        };
        var depth = (s, seen) => {
          if (!s?.epoll || seen.has(s)) return 0;
          seen.add(s);
          var max = 0;
          for (var f of s.epoll.keys()) max = Math.max(max, depth(FS.getStream(f), seen));
          seen.delete(s);
          return 1 + max;
        };
        if (reaches(target, ep, new Set()) || 1 + depth(target, new Set()) > 5) {
          return -{{{ cDefs.ELOOP }}};
        }
      }
    } else { // EPOLL_CTL_MOD
      if (!has) return -{{{ cDefs.ENOENT }}};
      // EPOLLEXCLUSIVE may only be set at ADD time.
      if (events & {{{ cDefs.EPOLLEXCLUSIVE }}}) return -{{{ cDefs.EINVAL }}};
    }

    // `data` is opaque user data echoed back by epoll_wait; keep its 8 bytes.
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
    // Arming (re-)counts it toward the keepalive.
    // (ep_poll_callback: on an edge, list the reg and wake any waiter on this
    // epoll - and through ep.node any parent epoll nesting it.)
    if (!reg.listener) {
      reg.listener = target.node.addListener(() => {
        rdllistAdd(ep, reg);
        ep.node.notifyListeners({{{ cDefs.POLLIN }}});
      // EPOLLEXCLUSIVE: when one fd is watched by several epolls, the watched
      // node wakes only one of them per edge (round-robin), not all.
      }, !!(events & {{{ cDefs.EPOLLEXCLUSIVE }}}));
      ep.armed++;
    }
    // Arming is itself an event source (ep_insert/ep_modify): a source-based
    // model only learns readiness from edges, so sample the level now - the
    // (re-)armed fd may already be ready with no producer notify to follow.
    if (pollOne(fd, reg.events & ~{{{ cDefs.EPOLLET | cDefs.EPOLLONESHOT | cDefs.EPOLLEXCLUSIVE }}})) {
      rdllistAdd(ep, reg);
      ep.node.notifyListeners({{{ cDefs.POLLIN }}});
    }
    // After arming (ADD or a MOD re-arm): the armed count may have changed.
    reconcileEpollKeepalive(ep);
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
  $doEpollWait__deps: ['$FS', '$pollOne', '$rdllistAdd', '$epollEvict', '$reconcileEpollKeepalive'],
  $doEpollWait: (ep, ev, maxevents) => {
    var disarmed = false; // a fired EPOLLONESHOT dropped its armed count
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
        // listener, drops it from the map, and reconciles the keepalive.
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
            // listener - the watched node stops poking it (no re-arm needed) - and
            // its armed count, so a set of only-fired one-shots reads as terminal.
            node.listener.listeners.delete(node.listener.entry);
            node.listener = null;
            ep.armed--;
            disarmed = true;
          } else if (!(node.events & {{{ cDefs.EPOLLET }}})) {
            rdllistAdd(ep, node); // level: re-list at tail
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
    // A fired one-shot may have made the set terminal (no armed registration
    // left); evictions above already reconciled themselves.
    if (disarmed) reconcileEpollKeepalive(ep);
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

  // Register a persistent readiness callback on an existing epoll fd: instead of
  // blocking in epoll_wait, the runtime delivers the ready set to `callback`
  // every time the epoll set makes progress. An epoll is a long-lived readiness
  // aggregator, so the interest (a single listener on the epoll's own node plus a
  // runtime-owned output buffer) is armed once and reused across every delivery -
  // no per-spin register/deregister. Per-fd EPOLLET/EPOLLONESHOT apply exactly as
  // in epoll_wait (one-shot is a property of the registration, not of this call),
  // so a long-lived callback can mix level/edge/oneshot fds in one set.
  //
  // The interest persists until replaced (call again), cleared (callback == NULL),
  // or the epoll fd is closed. It never suspends the stack, so it works without
  // ASYNCIFY/JSPI, and it keeps the runtime alive while armed. Returns 0 or -errno.
  emscripten_epoll_set_callback__deps: ['$FS', '$doEpollWait', '$clearEpollInterest', '$reconcileEpollKeepalive', '$callUserCallback', 'malloc', 'free',
#if PTHREADS
    '$epollDeliveries', '_emscripten_epoll_run_callback_on_thread',
#endif
  ],
  emscripten_epoll_set_callback__proxy: 'sync',
  emscripten_epoll_set_callback: (epfd, maxevents, callback, userdata) => {
    var ep = FS.getStream(epfd);
    if (!ep?.epoll) return -{{{ cDefs.EBADF }}};
    // maxevents only matters when (re-)arming; validate before any mutation so a
    // bad register call has no side effects (an unregister ignores it).
    if (callback && maxevents <= 0) return -{{{ cDefs.EINVAL }}};

#if PTHREADS
    // __proxy: 'sync' runs this (and every delivery) on the thread that owns the
    // filesystem. When a worker registered the callback it must fire on that
    // worker, not here, so capture the caller and back-proxy each delivery to it.
    // Zero when the caller is the FS-owning thread itself - then deliver inline.
    var callerThread = PThread.currentProxiedOperationCallerThread;
#endif

    // Tear down any existing interest first - a second call replaces the
    // callback, it does not stack. Reconcile immediately: if a different thread
    // owned the previous callback, this drops that thread's keepalive before we
    // take ownership on the caller's thread below (a handover, not a no-op).
    clearEpollInterest(ep);
    reconcileEpollKeepalive(ep);
    if (!callback) return 0;

    // Runtime-owned output buffer reused across every delivery; freed at clear.
    var buf = _malloc(maxevents * {{{ C_STRUCTS.epoll_event.__size__ }}});
    var it = ep.interest = {buf};
#if PTHREADS
    // The registering thread owns the callback and, once armed, its runtime
    // keepalive. 0 is the FS thread itself (delivered inline, keepalive local).
    ep.ownerThread = callerThread;
#endif
    // Producer notifies arrive synchronously (SOCKFS.emit, pipe writes); coalesce
    // them into one delivery on a microtask (the callback must not run in the
    // producer's/caller's stack), re-deriving the ready set at that tick. A
    // microtask keeps delivery latency minimal (no setTimeout clamp). Edges are
    // still disjoint from any concurrent blocking epoll_wait on the same epoll -
    // that waiter drains synchronously in the producer's stack, ahead of this
    // tick - but the tick may now run before vs after the waiter's async
    // resumption; that relative ordering is not guaranteed.
    function deliver() {
      if (ep.interest !== it) return; // cleared before the tick fired
#if PTHREADS
      // One cross-thread delivery in flight at a time: the registering thread
      // drains inside the callback, so re-deriving before it completes would just
      // re-see (and re-deliver) the same still-ready level fd in a tight spin.
      // The delivery's completion (do_epoll_done -> epoll_delivery_done) clears
      // this and re-wakes.
      if (it.inflight) return;
#endif
      var c = doEpollWait(ep, buf, maxevents);
      if (!c) return;
#if PTHREADS
      if (callerThread) {
        // The helper copies the events out synchronously, so buf is free to be
        // reused; it also reports back on completion to pace the next delivery.
        it.inflight = true;
        __emscripten_epoll_run_callback_on_thread(callerThread, callback, epfd, buf, c, userdata, it.token);
        return;
      }
#endif
      callUserCallback(() => {{{ makeDynCall('vipip', 'callback') }}}(epfd, buf, c, userdata));
      // Still ready (overflow past maxevents, or a still-ready level fd
      // re-listed): keep draining on the next tick. Note this is NOT a
      // blocking epoll_wait loop - there the app owns the loop and may block
      // elsewhere. A level-triggered fd that is structurally always ready and
      // never drained (e.g. EPOLLOUT on a writable socket) will re-schedule a
      // microtask each tick and so starve the event loop; use EPOLLET or
      // unregister for such fds.
      if (ep.interest === it && ep.rdlHead) wake();
    }
    function wake() {
      if (it.scheduled) return;
      it.scheduled = true;
      queueMicrotask(() => {
        it.scheduled = false;
        deliver();
      });
    }
#if PTHREADS
    // Resume point for a completed cross-thread delivery, keyed by token so the
    // C completion can find this interest again.
    if (callerThread) {
      it.wake = wake;
      it.token = epollDeliveries.nextToken++;
      epollDeliveries[it.token] = it;
    }
#endif
    it.listener = ep.node.addListener(wake);
    reconcileEpollKeepalive(ep); // hold the runtime only while there are live fds
    wake(); // deliver initial readiness if the set is already ready
    return 0;
  },

#if PTHREADS
  // Token -> interest for cross-thread deliveries (numeric keys), plus nextToken:
  // the next token to hand out. A monotonic token means a stale completion
  // (interest cleared mid-flight) never resolves to a different interest - it
  // simply finds nothing.
  $epollDeliveries: {nextToken: 1},

  // Called (on the FS-owning thread) by the C helper once a cross-thread delivery
  // finishes on the registering thread: clear the in-flight gate and re-derive,
  // so a still-ready set delivers its next batch.
  _emscripten_epoll_delivery_done__deps: ['$epollDeliveries'],
  _emscripten_epoll_delivery_done: (token) => {
    var it = epollDeliveries[token];
    if (!it) return; // interest was cleared while the delivery was in flight
    it.inflight = false;
    it.wake();
  },
#endif
};

addToLibrary(EpollLibrary);
