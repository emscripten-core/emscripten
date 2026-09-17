/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// epoll(7) for the JS filesystem. The epoll syscalls and the
// emscripten_epoll_add_listener extension build on the per-inode readiness
// wait-queue (FSNode.addListener/notifyListeners) and the synchronous readiness
// derivation ($pollOne) defined in libsyscall.js.

var EpollLibrary = {
  // An epoll instance's state lives on the stream's `shared` object - the open
  // file description (Linux's struct file / eventpoll) that dup'd fds share. It
  // carries an interest map `epoll` (fd -> reg) and a ready list
  // (rdlHead/rdlTail). Each registration arms a persistent listener on the
  // watched node's wait-queue at EPOLL_CTL_ADD (not per-wait), feeding the ready
  // list on each edge so readiness can be tracked across waits and up a nesting
  // chain. dup(2) yields another fd to the SAME instance (registrations, ready
  // list, and listeners all shared); close(2) drops one reference and only the
  // last close reclaims it (tearing every registration down). An epoll fd can
  // itself be added to another epoll.

  // Would a wait on this epoll block - i.e. does no listed registration have a
  // genuine ready event? Walks the ready list (O(ready)), masking out the
  // reporting-time flags (edge/oneshot/exclusive), and evicts a closed/reused fd
  // as it goes (so a set only ever probed, never drained, does not accumulate
  // dead registrations). This is the shared readiness derivation behind the
  // epoll fd's own poll handler (nesting) and the listeners' fire gate: a stale
  // ready-list entry (a spurious edge, or one left after its fd was drained then
  // closed) is not a ready event, so neither fires on it.
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
        // instance: remove any readiness listeners, then drop every
        // registration's listener (a fired EPOLLONESHOT has already dropped its
        // own) from its watched node. A surviving dup keeps it all live.
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
      // Readiness listeners (emscripten_epoll_add_listener), keyed by
      // (registering thread, callback).
      interests: new Map(),
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
        // Readiness wakes the epoll as POLLIN; a closing fd (POLLNVAL) wakes
        // it as a teardown, so listeners can tell an eviction from an event.
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

  // Drop one readiness listener: remove its wait-queue entry on the epoll node
  // and retire its cross-thread delivery token, so a completion that lands
  // after this finds nothing.
  $epollClearListener__internal: true,
  $epollClearListener__deps: [
#if PTHREADS
    '$epollDeliveries',
#endif
  ],
  $epollClearListener: (ep, it) => {
    ep.interests.delete(it.key);
    it.cleared = true;
    it.listener.listeners.delete(it.listener.entry);
#if PTHREADS
    if (it.token) delete epollDeliveries[it.token];
#endif
  },

  // Register a persistent readiness listener on an existing epoll fd: instead of
  // blocking in epoll_wait, the runtime invokes `callback` on the event loop
  // whenever the epoll set has ready events waiting to be collected. The callback
  // receives only `userdata` and does NOT drain the set - to collect the events
  // it calls epoll_wait(epfd, ..., 0) (a non-blocking, zero-timeout wait) itself.
  //
  // Any number of listeners may be added, keyed by (registering thread,
  // callback). Every listener is signalled while uncollected ready events remain
  // (broadcast); collectors race, so per-fd EPOLLET/EPOLLONESHOT items are
  // collected by exactly one of them - the same load balancing as multiple
  // blocking epoll_wait callers on one epoll. A level fd left undrained
  // re-signals every tick, an edge fd once per edge.
  //
  // A listener is an unref'd handle (Node's handle.unref()): it never keeps the
  // runtime, or with pthreads its registering thread, alive. A program that
  // only stays alive for its listeners holds the runtime itself with
  // emscripten_runtime_keepalive_push/pop. The one hold taken here is for a
  // scheduled delivery, which is pending work like a safeSetTimeout callback.
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
    // This is a direct public API (not a syscall), so it returns a positive
    // errno rather than the -errno syscall convention.
    if (!stream?.shared.epoll) return {{{ cDefs.EBADF }}};
    // Operate on the shared instance so a listener added on one fd sees
    // registrations made through any dup of it.
    var ep = stream.shared;

#if PTHREADS
    // __proxy: 'sync' runs this (and every derivation) on the main thread; each
    // delivery is back-proxied to the registering thread (0 = the main thread
    // itself, delivered inline).
    var callerThread = PThread.currentProxiedOperationCallerThread;
    var key = callerThread + ':' + callback;
#else
    var key = callback;
#endif
    // Re-adding the same (thread, callback) identity replaces the registration,
    // just updating userdata.
    var prev = ep.interests.get(key);
    if (prev) epollClearListener(ep, prev);

    var it = {key};
    ep.interests.set(key, it);

    function deliver() {
      if (it.cleared) return;
#if PTHREADS
      // One cross-thread delivery in flight at a time: the registering thread
      // collects (drains) inside the callback via a proxied epoll_wait, so firing
      // again before it completes would just re-see the same still-ready level fd
      // in a tight spin. The delivery's completion (do_epoll_done ->
      // epoll_delivery_done) clears this and re-wakes.
      if (it.inflight) return;
#endif
      if (epollWouldBlock(ep)) return; // no genuine uncollected ready event
#if PTHREADS
      if (callerThread) {
        it.inflight = true;
        // The owner thread is gone (exited under an exit() elsewhere): the
        // listener dies with it.
        if (!__emscripten_epoll_run_callback_on_thread(callerThread, callback, userdata, it.token)) {
          epollClearListener(ep, it);
        }
        return;
      }
#endif
      callUserCallback(() => {
        {{{ makeDynCall('vp', 'callback') }}}(userdata);
        // Still readable (this callback didn't drain, or a still-ready level fd
        // re-listed): fire again on the next turn. Note this is NOT a blocking
        // epoll_wait loop - a level-triggered fd that is structurally always
        // ready (e.g. EPOLLOUT on a writable socket) will re-schedule every
        // turn and so starve the event loop; use EPOLLET or remove the
        // listener for such fds. Inside the wrapper so the re-wake's hold is
        // taken before callUserCallback's maybeExit.
        if (!it.cleared && !epollWouldBlock(ep)) wake(true);
      });
    }
    // Producer notifies arrive synchronously (SOCKFS.emit, pipe writes); coalesce
    // them into one delivery per listener on a macrotask - not a microtask,
    // since hosts drain microtasks synchronously inside other calls (Node's
    // module loader does so on a first builtin load, e.g. from connect()),
    // which would run the callback re-entrantly under the caller's frames.
    //
    // A scheduled delivery is pending work and holds the runtime until it runs
    // (like safeSetTimeout): a pipe write from a callback's last act must still
    // deliver. A teardown wake holds nothing: a watched fd closing (POLLNVAL)
    // only evicts, and once FS.quit has begun (exitRuntime: FS.initialized
    // cleared, every open fd closed, pipe peers reporting POLLHUP on the way)
    // no delivery can run, while a hold taken there would outlive the exit,
    // leaving keepRuntimeAlive() set at _proc_exit and onExit skipped.
    function wake(held) {
      if (held && FS.initialized && !it.held) {
        it.held = true;
        {{{ runtimeKeepalivePush() }}}
      }
      if (it.scheduled) return;
      it.scheduled = true;
      emSetImmediate(() => {
        it.scheduled = false;
        // Release first, so callUserCallback's maybeExit sees the true state
        // (the callback's own re-wake takes a fresh hold inside).
        if (it.held) {
          it.held = false;
          {{{ runtimeKeepalivePop() }}}
        }
        // Nothing to deliver (cleared, or drained synchronously meanwhile), or
        // a cross-thread dispatch that runs elsewhere: callUserCallback's
        // maybeExit will not run here, and the hold just released may have
        // been what deferred main's exit.
        if (it.cleared || epollWouldBlock(ep)) {
#if !MINIMAL_RUNTIME
          maybeExit();
#endif
          return;
        }
#if PTHREADS
        if (callerThread) {
          deliver();
#if !MINIMAL_RUNTIME
          maybeExit();
#endif
          return;
        }
#endif
        deliver();
      });
    }
#if PTHREADS
    // Resume point for a completed cross-thread delivery, keyed by token so the
    // C completion can find this listener again.
    if (callerThread) {
      it.ep = ep;
      it.wake = wake;
      it.token = epollDeliveries.nextToken++;
      epollDeliveries[it.token] = it;
    }
#endif
    it.listener = ep.node.addListener((flags) => wake(!(flags & {{{ cDefs.POLLNVAL }}})));
    wake(!epollWouldBlock(ep)); // deliver initial readiness if the set is already ready
    return 0;
  },

  // Remove the calling thread's listener for `callback`. All listeners are also
  // removed when the last fd to the instance closes.
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
  // Token -> listener for cross-thread deliveries (numeric keys), plus nextToken:
  // the next token to hand out. A monotonic token means a stale completion
  // (listener removed mid-flight) never resolves to a different listener - it
  // simply finds nothing.
  $epollDeliveries: {nextToken: 1},

  // Called (on the main thread) by the C helper once a cross-thread delivery
  // finishes on the registering thread: clear the in-flight gate and re-derive,
  // so a still-ready set delivers its next batch.
  _emscripten_epoll_delivery_done__deps: ['$epollDeliveries', '$epollWouldBlock'],
  _emscripten_epoll_delivery_done: (token) => {
    var it = epollDeliveries[token];
    if (!it) return; // listener was removed while the delivery was in flight
    it.inflight = false;
    it.wake(!epollWouldBlock(it.ep));
  },
#endif
};

addToLibrary(EpollLibrary);
