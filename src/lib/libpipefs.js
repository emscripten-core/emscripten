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
    // Queued bytes above which a PIPE_BUF write no longer fits.
    THRESHOLD: 1024 * 64 - {{{ cDefs.PIPE_BUF }}},

    mount(mount) {
      // Do not pollute the real root directory or its child nodes with pipes
      // Looks like it is OK to create another pseudo-root node not linked to the FS.root hierarchy this way
      return FS.createNode(null, '/', {{{ cDefs.S_IFDIR }}} | 0o777, 0);
    },
    createPipe() {
      var pipe = {
        buffer: new Uint8Array(PIPEFS.CAPACITY),
        offset: 0,
        roffset: 0,
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
      return pipe.offset - pipe.roffset;
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
        buffer.set(pipe.buffer.subarray(pipe.roffset, pipe.roffset + toRead), offset);
        pipe.roffset += toRead;
        // Reset cursors when fully drained so subsequent writes start at 0
        // without needing to shift data.
        if (pipe.roffset == pipe.offset) {
          pipe.roffset = pipe.offset = 0;
        }

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
        // Lazily compact unread bytes to the start of the buffer only when a
        // write would run past the end of the 64 KiB array.
        if (pipe.offset + dataLen > PIPEFS.CAPACITY) {
          pipe.buffer.copyWithin(0, pipe.roffset, pipe.offset);
          pipe.offset -= pipe.roffset;
          pipe.roffset = 0;
        }
        pipe.buffer.set(data, pipe.offset);
        pipe.offset += dataLen;

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
