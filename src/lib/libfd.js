/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// The file descriptor core: the descriptor table, open file descriptions and
// the per-node readiness wait-queue, with no filesystem namespace. Sockets,
// pipes and epoll need only this; FS builds on it for everything with a path.

var LibraryFDS = {
  $FDS__deps: [
#if ASSERTIONS
    '$strError', '$ERRNO_CODES',
#endif
#if !SYSCALLS_REQUIRE_FILESYSTEM
    '$printChar', '$printCharBuffers',
#endif
  ],
#if !SYSCALLS_REQUIRE_FILESYSTEM
  $FDS__postset: () => {
    addAtInit('FDS.createStandardStreams();');
    addAtExit('FDS.quit();');
  },
#endif
  $FDS: {
    streams: [],
    nextInode: 1,
    MAX_OPEN_FDS: 4096,
#if ASSERTIONS
    ErrnoError: class extends Error {
#else
    ErrnoError: class {
#endif
      name = 'ErrnoError';
      // We set the `name` property to be able to identify `FS.ErrnoError`
      // - the `name` is a standard ECMA-262 property of error objects. Kind of good to have it anyway.
      // - when using PROXYFS, an error can come from an underlying FS
      // as different FS objects have their own FS.ErrnoError each,
      // the test `err instanceof FS.ErrnoError` won't detect an error coming from another filesystem, causing bugs.
      // we'll use the reliable test `err.name == "ErrnoError"` instead
      constructor(errno) {
#if ASSERTIONS
        super(runtimeInitialized ? strError(errno) : '');
#endif
        this.errno = errno;
#if ASSERTIONS
        for (var key in ERRNO_CODES) {
          if (ERRNO_CODES[key] === errno) {
            this.code = key;
            break;
          }
        }
#endif
      }
    },
    Stream: class {
      shared = {};
#if USE_CLOSURE_COMPILER
      // Closure compiler requires us to declare all properties ahead of time
      node = null;
#endif
      get object() {
        return this.node;
      }
      set object(val) {
        this.node = val;
      }
      get isRead() {
        return (this.flags & {{{ cDefs.O_ACCMODE }}}) !== {{{ cDefs.O_WRONLY }}};
      }
      get isWrite() {
        return (this.flags & {{{ cDefs.O_ACCMODE }}}) !== {{{ cDefs.O_RDONLY }}};
      }
      get isAppend() {
        return (this.flags & {{{ cDefs.O_APPEND }}});
      }
      get flags() {
        return this.shared.flags;
      }
      set flags(val) {
        this.shared.flags = val;
      }
      get position() {
        return this.shared.position;
      }
      set position(val) {
        this.shared.position = val;
      }
    },
    // The inode-level identity behind a stream: a type mode, an inode number
    // and the readiness wait-queue. Sockets, pipes and epoll instances use it
    // directly; FS.FSNode extends it with the namespace (parent, name, ops).
    Node: class {
#if USE_CLOSURE_COMPILER
      // Closure (@struct) requires these declared ahead of time. The readiness
      // wait-queue is populated lazily, and only on nodes that derive real
      // readiness (sockets, pipes, an epoll's own node).
      /** @type {Set<?>|null} */
      listeners = null;
      /** @type {number} */
      exclTurn = 0;
#endif
      constructor(mode) {
        this.id = FDS.nextInode++;
        this.mode = mode;
      }
      // The per-inode readiness wait-queue. The node carries a Set of listener
      // entries {cb}; producers (SOCKFS, PIPEFS) call notifyListeners on a
      // readiness transition, and poll()/epoll consume it. It lives on the node
      // (not the fd) so dup'd fds share one queue. Only nodes that derive real
      // readiness (sockets, pipes, and an epoll's own node) ever use this -
      // always-ready types (regular files, ttys) never register or notify.
      addListener(cb, exclusive = false) {
        var entry = {cb, exclusive};
        var listeners = (this.listeners ??= new Set());
        listeners.add(entry);
        return {listeners, entry};
      }
      notifyListeners(flags) {
        // Iterates the set without copying, which is safe ONLY under a
        // load-bearing contract that every internal listener must honour:
        //   1. A listener must not run user code synchronously (a poll waiter only
        //      resolves a Promise; an epoll registration only re-lists +
        //      re-notifies; the epoll callback only schedules a tick). User code
        //      runs on a later tick, never inside this loop.
        //   2. A listener may delete entries only from ITS OWN waiter, never from
        //      a sibling node's set that may be mid-iteration. (Deleting an entry
        //      of the set being iterated here is fine - a Set tolerates removal of
        //      a not-yet-visited entry mid-iteration; mutating a *different* node's
        //      set is fine because that set is not being iterated.)
        // Violating either gives silently skipped wakeups that are near-impossible
        // to reproduce. Any new producer/listener must preserve it.
        if (!this.listeners) return;
        // Fire every non-exclusive listener. Among EPOLLEXCLUSIVE registrations
        // (one fd watched by several epolls) wake only one, rotating round-robin
        // per node, to avoid a thundering herd. (Only epoll registrations are ever
        // exclusive; poll waiters and a node's own consumers are not.)
        var excl;
        for (var entry of this.listeners) {
          if (entry.exclusive) (excl ||= []).push(entry);
          else entry.cb(flags);
        }
        if (excl) {
          var i = (this.exclTurn || 0) % excl.length;
          this.exclTurn = i + 1;
          excl[i].cb(flags);
        }
      }
    },

    isFile(mode) {
      return (mode & {{{ cDefs.S_IFMT }}}) === {{{ cDefs.S_IFREG }}};
    },
    isDir(mode) {
      return (mode & {{{ cDefs.S_IFMT }}}) === {{{ cDefs.S_IFDIR }}};
    },
    isLink(mode) {
      return (mode & {{{ cDefs.S_IFMT }}}) === {{{ cDefs.S_IFLNK }}};
    },
    isChrdev(mode) {
      return (mode & {{{ cDefs.S_IFMT }}}) === {{{ cDefs.S_IFCHR }}};
    },
    isBlkdev(mode) {
      return (mode & {{{ cDefs.S_IFMT }}}) === {{{ cDefs.S_IFBLK }}};
    },
    isFIFO(mode) {
      return (mode & {{{ cDefs.S_IFMT }}}) === {{{ cDefs.S_IFIFO }}};
    },
    isSocket(mode) {
      return (mode & {{{ cDefs.S_IFSOCK }}}) === {{{ cDefs.S_IFSOCK }}};
    },

    //
    // streams
    //
    nextfd() {
      for (var fd = 0; fd <= FDS.MAX_OPEN_FDS; fd++) {
        if (!FDS.streams[fd]) {
          return fd;
        }
      }
      throw new FDS.ErrnoError({{{ cDefs.EMFILE }}});
    },
    getStreamChecked(fd) {
      var stream = FDS.getStream(fd);
      if (!stream) {
        throw new FDS.ErrnoError({{{ cDefs.EBADF }}});
      }
      return stream;
    },
    getStream: (fd) => FDS.streams[fd],
    // TODO parameterize this function such that a stream
    // object isn't directly passed in. not possible until
    // SOCKFS is completed.
    createStream(stream, fd = -1) {
#if ASSERTIONS
      assert(fd >= -1);
#endif

      // clone it, so we can return an instance of Stream
      stream = Object.assign(new FDS.Stream(), stream);
      if (fd == -1) {
        fd = FDS.nextfd();
      }
      stream.fd = fd;
      FDS.streams[fd] = stream;
      return stream;
    },
    closeStream(fd) {
      FDS.streams[fd] = null;
    },
    dupStream(origStream, fd = -1) {
      var stream = FDS.createStream(origStream, fd);
      stream.stream_ops?.dup?.(stream);
      return stream;
    },
    close(stream) {
      if (FDS.isClosed(stream)) {
        throw new FDS.ErrnoError({{{ cDefs.EBADF }}});
      }
      // The fd is going away: wake anything waiting on it (poll/epoll) with
      // POLLNVAL so a blocking wait unblocks and an epoll registration is evicted
      // on its next derive. Only sockets/pipes/epoll ever carry a wait-queue, so
      // for every other stream (incl. nodeless noderawfs stdio) this is a no-op.
      stream.node?.notifyListeners({{{ cDefs.POLLNVAL }}});
      try {
        if (stream.stream_ops.close) {
          stream.stream_ops.close(stream);
        }
      } catch (e) {
        throw e;
      } finally {
        FDS.closeStream(stream.fd);
      }
      stream.fd = null;
    },
    isClosed(stream) {
      return stream.fd === null;
    },
    llseek(stream, offset, whence) {
      if (FDS.isClosed(stream)) {
        throw new FDS.ErrnoError({{{ cDefs.EBADF }}});
      }
      if (!stream.seekable || !stream.stream_ops.llseek) {
        throw new FDS.ErrnoError({{{ cDefs.ESPIPE }}});
      }
      if (whence != {{{ cDefs.SEEK_SET }}} && whence != {{{ cDefs.SEEK_CUR }}} && whence != {{{ cDefs.SEEK_END }}}) {
        throw new FDS.ErrnoError({{{ cDefs.EINVAL }}});
      }
      stream.position = stream.stream_ops.llseek(stream, offset, whence);
      stream.ungotten = [];
      return stream.position;
    },
    read(stream, buffer, offset, length, position = undefined) {
#if ASSERTIONS
      assert(offset >= 0);
#endif
      if (length < 0 || position < 0) {
        throw new FDS.ErrnoError({{{ cDefs.EINVAL }}});
      }
      if (FDS.isClosed(stream)) {
        throw new FDS.ErrnoError({{{ cDefs.EBADF }}});
      }
      if ((stream.flags & {{{ cDefs.O_ACCMODE }}}) === {{{ cDefs.O_WRONLY}}}) {
        throw new FDS.ErrnoError({{{ cDefs.EBADF }}});
      }
      if (FDS.isDir(stream.node.mode)) {
        throw new FDS.ErrnoError({{{ cDefs.EISDIR }}});
      }
      if (!stream.stream_ops.read) {
        throw new FDS.ErrnoError({{{ cDefs.EINVAL }}});
      }
      var seeking = typeof position != 'undefined';
      if (!seeking) {
        position = stream.position;
      } else if (!stream.seekable) {
        throw new FDS.ErrnoError({{{ cDefs.ESPIPE }}});
      }
      var bytesRead = stream.stream_ops.read(stream, buffer, offset, length, position);
      if (!seeking) stream.position += bytesRead;
      return bytesRead;
    },
    /**
     * @param {TypedArray} buffer
     */
    write(stream, buffer, offset, length, position = undefined, canOwn = undefined) {
#if ASSERTIONS
      assert(offset >= 0);
      assert(buffer.subarray, 'FS.write expects a TypedArray');
#endif
      if (length < 0 || position < 0) {
        throw new FDS.ErrnoError({{{ cDefs.EINVAL }}});
      }
      if (FDS.isClosed(stream)) {
        throw new FDS.ErrnoError({{{ cDefs.EBADF }}});
      }
      if ((stream.flags & {{{ cDefs.O_ACCMODE }}}) === {{{ cDefs.O_RDONLY}}}) {
        throw new FDS.ErrnoError({{{ cDefs.EBADF }}});
      }
      if (FDS.isDir(stream.node.mode)) {
        throw new FDS.ErrnoError({{{ cDefs.EISDIR }}});
      }
      if (!stream.stream_ops.write) {
        throw new FDS.ErrnoError({{{ cDefs.EINVAL }}});
      }
      if (stream.seekable && stream.flags & {{{ cDefs.O_APPEND }}}) {
        // seek to the end before writing in append mode
        FDS.llseek(stream, 0, {{{ cDefs.SEEK_END }}});
      }
      var seeking = typeof position != 'undefined';
      if (!seeking) {
        position = stream.position;
      } else if (!stream.seekable) {
        throw new FDS.ErrnoError({{{ cDefs.ESPIPE }}});
      }
      var bytesWritten = stream.stream_ops.write(stream, buffer, offset, length, position, canOwn);
      if (!seeking) stream.position += bytesWritten;
      return bytesWritten;
    },

#if !SYSCALLS_REQUIRE_FILESYSTEM
    // Without a filesystem there are no tty devices: stdin is at EOF and
    // stdout/stderr are line-buffered onto out()/err().
    createStandardStreams() {
      var stream_ops = {
        read: () => 0,
        write(stream, buffer, offset, length) {
          for (var i = 0; i < length; i++) {
            printChar(stream.fd, buffer[offset + i]);
          }
          return length;
        },
      };
      for (var fd = 0; fd < 3; fd++) {
        FDS.createStream({
          node: new FDS.Node({{{ cDefs.S_IFCHR }}}),
          tty: true,
          flags: fd ? {{{ cDefs.O_WRONLY }}} : {{{ cDefs.O_RDONLY }}},
          stream_ops,
        }, fd);
      }
    },
    quit() {
#if hasExportedSymbol('fflush')
      _fflush(0);
#endif
      for (var stream of FDS.streams) {
        if (stream) FDS.close(stream);
      }
      if (printCharBuffers[1].length) printChar(1, {{{ charCode('\n') }}});
      if (printCharBuffers[2].length) printChar(2, {{{ charCode('\n') }}});
    },
#endif
  },
};

// Node subclasses (FS.FSNode, SOCKFS.Node, PIPEFS.Node) are class expressions
// evaluated when their library files load, so FDS must be bound then (library
// files share one context); in the output, FDS is the emitted library object.
var FDS = LibraryFDS.$FDS;

addToLibrary(LibraryFDS);
