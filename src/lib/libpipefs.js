/**
 * @license
 * Copyright 2017 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $PIPEFS__postset: () => addAtInit('PIPEFS.root = FS.mount(PIPEFS, {}, null);'),
  $PIPEFS__deps: ['$FS'],
  $PIPEFS: {
    CAPACITY: 1024 * 64, // as on Linux
    // A new pipe starts with a small buffer that doubles as data arrives, so
    // the many mostly-empty pipes some programs create stay cheap.
    INITIAL_SIZE: 1024 * 8,
    // Queued bytes above which a PIPE_BUF write no longer fits.
    THRESHOLD: 1024 * 64 - {{{ cDefs.PIPE_BUF }}},

    mount(mount) {
      // Do not pollute the real root directory or its child nodes with pipes
      // Looks like it is OK to create another pseudo-root node not linked to the FS.root hierarchy this way
      return FS.createNode(null, '/', {{{ cDefs.S_IFDIR }}} | 0o777, 0);
    },
    createPipe() {
      var pipe = {
        // Ring buffer: `count` unread bytes starting at `head`, wrapping
        // around the end of `buffer`.
        buffer: new Uint8Array(PIPEFS.INITIAL_SIZE),
        head: 0,
        count: 0,
        // Open write ends. When it drops to 0 the reader sees EOF and poll must
        // report POLLHUP (Linux semantics). The buffer is freed once both counts
        // reach 0.
        writerCount: 1,
        writeClosed: false,
        // Open read ends. When it drops to 0 the writer sees POLLERR (a further
        // write would get EPIPE).
        readerCount: 1,
        readClosed: false,
        timestamp: new Date(),
      };

      var rName = PIPEFS.nextname();
      var wName = PIPEFS.nextname();
      var rNode = FS.createNode(PIPEFS.root, rName, {{{ cDefs.S_IFIFO }}}, 0);
      var wNode = FS.createNode(PIPEFS.root, wName, {{{ cDefs.S_IFIFO }}}, 0);

      rNode.pipe = pipe;
      wNode.pipe = pipe;
      // The read end's node carries the reader poll wait-queue (writes wake it);
      // the write end's node carries the writer wait-queue (read-end close wakes it).
      pipe.readNode = rNode;
      pipe.writeNode = wNode;

      var readableStream = FS.createStream({
        path: rName,
        node: rNode,
        flags: {{{ cDefs.O_RDONLY }}},
        seekable: false,
        stream_ops: PIPEFS.stream_ops
      });
      rNode.stream = readableStream;

      var writableStream = FS.createStream({
        path: wName,
        node: wNode,
        flags: {{{ cDefs.O_WRONLY }}},
        seekable: false,
        stream_ops: PIPEFS.stream_ops
      });
      wNode.stream = writableStream;

      return {
        readable_fd: readableStream.fd,
        writable_fd: writableStream.fd
      };
    },
    queuedBytes(pipe) {
      return pipe.count;
    },
    // Grows the buffer (doubling, up to CAPACITY) until `needed` bytes fit,
    // unwrapping the queued bytes to the start of the new buffer.
    grow(pipe, needed) {
#if ASSERTIONS
      assert(needed <= PIPEFS.CAPACITY);
#endif
      var size = pipe.buffer.length;
      while (size < needed) size *= 2;
      var buffer = new Uint8Array(size);
      PIPEFS.copyOut(pipe, buffer, 0, pipe.count);
      pipe.buffer = buffer;
      pipe.head = 0;
    },
    // Copies `length` queued bytes starting at `head` into `dest` at `offset`,
    // in at most two pieces when the data wraps.
    copyOut(pipe, dest, offset, length) {
      var first = Math.min(length, pipe.buffer.length - pipe.head);
      dest.set(pipe.buffer.subarray(pipe.head, pipe.head + first), offset);
      if (first < length) {
        dest.set(pipe.buffer.subarray(0, length - first), offset + first);
      }
    },
    stream_ops: {
      getattr(stream) {
        var node = stream.node;
        var timestamp = node.pipe.timestamp;
        return {
          dev: 14,
          ino: node.id,
          mode: 0o10600,
          nlink: 1,
          uid: 0,
          gid: 0,
          rdev: 0,
          size: 0,
          atime: timestamp,
          mtime: timestamp,
          ctime: timestamp,
          blksize: 4096,
          blocks: 0,
        };
      },
      poll(stream) {
        var pipe = stream.node.pipe;

        if ((stream.flags & {{{ cDefs.O_ACCMODE }}}) === {{{ cDefs.O_WRONLY }}}) {
          // Writable while a PIPE_BUF write fits. Linux keeps the write end
          // writable (the write itself fails with EPIPE) while also signalling
          // POLLERR once every read end is closed.
          var mask = 0;
          if (pipe.readClosed || PIPEFS.queuedBytes(pipe) <= PIPEFS.THRESHOLD) {
            mask = {{{ cDefs.POLLWRNORM }}} | {{{ cDefs.POLLOUT }}};
          }
          if (pipe.readClosed) {
            mask |= {{{ cDefs.POLLERR }}};
          }
          return mask;
        }
        var mask = 0;
        if (PIPEFS.queuedBytes(pipe)) {
          mask = {{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}};
        }
        // With every write end closed the read end is at EOF: readable (read
        // returns 0) and hung up.
        if (pipe.writeClosed) {
          mask |= {{{ cDefs.POLLHUP }}} | {{{ cDefs.POLLIN }}};
        }
        return mask;
      },
      dup(stream) {
        var pipe = stream.node.pipe;
        if ((stream.flags & {{{ cDefs.O_ACCMODE }}}) === {{{ cDefs.O_WRONLY }}}) {
          pipe.writerCount++;
        } else {
          pipe.readerCount++;
        }
      },
      ioctl(stream, request, argp) {
        if (request == {{{ cDefs.FIONREAD }}}) {
          {{{ makeSetValue('argp', 0, 'PIPEFS.queuedBytes(stream.node.pipe)', 'i32') }}};
          return 0;
        }
        return {{{ cDefs.EINVAL }}};
      },
      fsync(stream) {
        return {{{ cDefs.EINVAL }}};
      },
      read(stream, buffer, offset, length, position /* ignored */) {
        var pipe = stream.node.pipe;
        var currentLength = PIPEFS.queuedBytes(pipe);

#if ASSERTIONS && !(MEMORY64 && MAXIMUM_MEMORY > FOUR_GB)
#if PTHREADS
        assert(buffer instanceof ArrayBuffer || buffer instanceof SharedArrayBuffer || ArrayBuffer.isView(buffer));
#else
        assert(buffer instanceof ArrayBuffer || ArrayBuffer.isView(buffer));
#endif
#endif
        if (length <= 0) {
          return 0;
        }
        if (currentLength == 0) {
          if (pipe.writeClosed) return 0;
          // Behave as if the read end is always non-blocking
          throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
        }
        var toRead = Math.min(currentLength, length);
        PIPEFS.copyOut(pipe, buffer, offset, toRead);
        pipe.count -= toRead;
        // Restart at 0 once drained so the next write does not wrap.
        pipe.head = pipe.count ? (pipe.head + toRead) % pipe.buffer.length : 0;

        // Wake POLLOUT waiters on the transition to writable: listeners only
        // notify on readiness changes, and a waiter only exists while the
        // pipe is above the threshold.
        if (currentLength > PIPEFS.THRESHOLD && currentLength - toRead <= PIPEFS.THRESHOLD) {
          pipe.writeNode.notifyListeners({{{ cDefs.POLLWRNORM }}} | {{{ cDefs.POLLOUT }}});
        }
        return toRead;
      },
      write(stream, buffer, offset, length, position /* ignored */) {
        var pipe = stream.node.pipe;

#if ASSERTIONS && !(MEMORY64 && MAXIMUM_MEMORY > FOUR_GB)
#if PTHREADS
        assert(buffer instanceof ArrayBuffer || buffer instanceof SharedArrayBuffer || ArrayBuffer.isView(buffer));
#else
        assert(buffer instanceof ArrayBuffer || ArrayBuffer.isView(buffer));
#endif
#endif
        var data = buffer.subarray(offset, offset + length);

        var dataLen = data.byteLength;
        if (dataLen <= 0) {
          return 0;
        }
        if (pipe.readClosed) {
          throw new FS.ErrnoError({{{ cDefs.EPIPE }}});
        }

        // Behave as if the write end is always non-blocking.
        // PIPE_BUF rule: a small write is all-or-nothing, a larger one takes
        // what fits.
        var room = PIPEFS.CAPACITY - PIPEFS.queuedBytes(pipe);
        if (room <= 0 || (dataLen <= {{{ cDefs.PIPE_BUF }}} && dataLen > room)) {
          throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
        }
        if (dataLen > room) {
          data = data.subarray(0, room);
          dataLen = room;
        }
        if (pipe.count + dataLen > pipe.buffer.length) {
          PIPEFS.grow(pipe, pipe.count + dataLen);
        }
        // Write after the queued bytes, wrapping around the end of the buffer.
        var size = pipe.buffer.length;
        var tail = (pipe.head + pipe.count) % size;
        var first = Math.min(dataLen, size - tail);
        pipe.buffer.set(data.subarray(0, first), tail);
        if (first < dataLen) {
          pipe.buffer.set(data.subarray(first), 0);
        }
        pipe.count += dataLen;

        pipe.readNode.notifyListeners({{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}});
        return dataLen;
      },
      close(stream) {
        var pipe = stream.node.pipe;
        // When the last write end closes, wake any poll/epoll waiter on the read
        // end with POLLHUP so a reader blocked on the writer dropping unblocks.
        if ((stream.flags & {{{ cDefs.O_ACCMODE }}}) === {{{ cDefs.O_WRONLY }}}) {
          if (!--pipe.writerCount) {
            pipe.writeClosed = true;
            pipe.readNode.notifyListeners({{{ cDefs.POLLHUP }}} | {{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}});
          }
        } else if (!--pipe.readerCount) {
          // Mirror: when the last read end closes, wake any poll/epoll waiter on
          // the write end with POLLERR (a further write would get EPIPE).
          pipe.readClosed = true;
          pipe.writeNode.notifyListeners({{{ cDefs.POLLERR }}} | {{{ cDefs.POLLWRNORM }}} | {{{ cDefs.POLLOUT }}});
        }
        if (!pipe.readerCount && !pipe.writerCount) {
          pipe.buffer = null;
        }
      }
    },
    nextname() {
      if (!PIPEFS.nextname.current) {
        PIPEFS.nextname.current = 0;
      }
      return 'pipe[' + (PIPEFS.nextname.current++) + ']';
    },
  },
});
