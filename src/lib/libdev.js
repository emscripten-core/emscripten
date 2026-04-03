/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $DEV__deps: [
    '$FS',
    '$ERRNO_CODES',
#if ENVIRONMENT_MAY_BE_NODE
    '$nodeTTY',
    '$nodeFsync',
#endif
  ],
  $DEV: {
    readWriteHelper: (stream, cb, method) => {
      try {
        var nbytes = cb();
      } catch (e) {
        // Convert Node errors into ErrnoError
        if (e && e.code && ERRNO_CODES[e.code]) {
          throw new FS.ErrnoError(ERRNO_CODES[e.code]);
        }
        if (e?.errno) {
          // propagate errno
          throw e;
        }
        // Other errors converted to EIO.
#if ASSERTIONS
        console.error(`Error thrown in ${method}:`);
        console.error(e);
#endif
        throw new FS.ErrnoError({{{ cDefs.EIO }}});
      }
      if (nbytes === undefined) {
        // Prevent an infinite loop caused by incorrect code that doesn't return a
        // value
        // Maybe we should set nbytes = buffer.length here instead?
#if ASSERTIONS
        console.warn(
          `${method} returned undefined; a correct implementation must return a number`,
        );
#endif
        throw new FS.ErrnoError({{{ cDefs.EIO }}});
      }
      if (nbytes !== 0) {
        stream.node.timestamp = Date.now();
      }
      return nbytes;
    },
    devs: [],
    register(dev, ops) {
      DEV.devs[dev] = ops;
      FS.registerDevice(dev, DEV.stream_ops);
    },
    TTY_OPS: {
      ioctl_tiocgwinsz(tty) {
        const { rows = 24, columns = 80 } = tty.devops.getTerminalSize?.() ?? {};
        return [rows, columns];
      },
    },
    stream_ops: {
      open(stream) {
        var devops = DEV.devs[stream.node.rdev];
        if (!devops) {
          throw new FS.ErrnoError({{{ cDefs.ENODEV }}});
        }
        stream.devops = devops;
        stream.seekable = false;
        stream.tty =
          stream.devops.tty ??
          (stream.devops.isatty
            ? {
                ops: DEV.TTY_OPS,
                devops,
              }
            : undefined);
        devops.open?.(stream);
      },
      close(stream) {
        // flush any pending line data
        stream.stream_ops.fsync(stream);
      },
      fsync(stream) {
        stream.devops.fsync?.(stream.devops);
      },
      read: function (stream, buffer, offset, length, pos /* ignored */) {
        buffer = buffer.subarray(offset, offset + length);
        return DEV.readWriteHelper(stream, () => stream.devops.read(stream.devops, buffer), "read");
      },
      write: function (stream, buffer, offset, length, pos /* ignored */) {
        buffer = buffer.subarray(offset, offset + length);
        return DEV.readWriteHelper(stream, () => stream.devops.write(stream.devops, buffer), "write");
      },
    },
#if ENVIRONMENT_MAY_BE_NODE
    nodeInputDevice: (nodeStream) => ({
      isatty: nodeTTY.isatty(nodeStream.fd),
      fsync() {
        nodeFsync(nodeStream.fd);
      },
      read(ops, buffer) {
        return fs.readSync(nodeStream.fd, buffer, 0, buffer.length);
      },
    }),
    nodeOutputDevice: (nodeStream) => ({
      isatty: nodeTTY.isatty(nodeStream.fd),
      fsync() {
        nodeFsync(nodeStream.fd);
      },
      write(ops, buffer) {
        return fs.writeSync(nodeStream.fd, buffer, 0, buffer.length);
      },
      getTerminalSize() {
        return nodeStream;
      }
    }),
#endif
  },
#if ENVIRONMENT_MAY_BE_NODE
  $nodeTTY: "require('node:tty');",
  $nodeFsync: (fd) => {
    try {
      fs.fsyncSync(fd);
    } catch (e) {
      if (e?.code === "EINVAL") {
        return;
      }
      // On Mac, calling fsync when not isatty returns ENOTSUP
      // On Windows, stdin/stdout/stderr may be closed, returning EBADF or EPERM
      const isStdStream = fd === 0 || fd === 1 || fd === 2;
      if (
        isStdStream &&
        (e?.code === "ENOTSUP" || e?.code === "EBADF" || e?.code === "EPERM")
      ) {
        return;
      }

      throw e;
    }
  }
#endif
});
