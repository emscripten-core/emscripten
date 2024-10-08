/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $TTY__deps: [
    '$FS',
    '$UTF8ArrayToString',
    '$FS_stdin_getChar'
  ],
#if !MINIMAL_RUNTIME
  $TTY__postset: () => {
    addAtInit('TTY.init();');
    addAtExit('TTY.shutdown();');
  },
#endif
  $TTY: {
    ttys: [],
    init() {
      // https://github.com/emscripten-core/emscripten/pull/1555
      // if (ENVIRONMENT_IS_NODE) {
      //   // currently, FS.init does not distinguish if process.stdin is a file or TTY
      //   // device, it always assumes it's a TTY device. because of this, we're forcing
      //   // process.stdin to UTF8 encoding to at least make stdin reading compatible
      //   // with text files until FS.init can be refactored.
      //   process.stdin.setEncoding('utf8');
      // }
    },
    shutdown() {
      // https://github.com/emscripten-core/emscripten/pull/1555
      // if (ENVIRONMENT_IS_NODE) {
      //   // inolen: any idea as to why node -e 'process.stdin.read()' wouldn't exit immediately (with process.stdin being a tty)?
      //   // isaacs: because now it's reading from the stream, you've expressed interest in it, so that read() kicks off a _read() which creates a ReadReq operation
      //   // inolen: I thought read() in that case was a synchronous operation that just grabbed some amount of buffered data if it exists?
      //   // isaacs: it is. but it also triggers a _read() call, which calls readStart() on the handle
      //   // isaacs: do process.stdin.pause() and i'd think it'd probably close the pending call
      //   process.stdin.pause();
      // }
    },
    register(dev, ops) {
      TTY.ttys[dev] = { input: [], output: [], ops: ops };
      FS.registerDevice(dev, TTY.stream_ops);
    },
    stream_ops: {
      open(stream) {
        var tty = TTY.ttys[stream.node.rdev];
        if (!tty) {
          throw new FS.ErrnoError({{{ cDefs.ENODEV }}});
        }
        stream.tty = tty;
        stream.seekable = false;
      },
      close(stream) {
        // flush any pending line data
        stream.tty.ops.fsync(stream.tty);
      },
      fsync(stream) {
        stream.tty.ops.fsync(stream.tty);
      },
      read(stream, buffer, offset, length, pos /* ignored */) {
        if (!stream.tty || !stream.tty.ops.get_char) {
          throw new FS.ErrnoError({{{ cDefs.ENXIO }}});
        }
        var bytesRead = 0;
        for (var i = 0; i < length; i++) {
          var result;
          try {
            result = stream.tty.ops.get_char(stream.tty);
          } catch (e) {
            throw new FS.ErrnoError({{{ cDefs.EIO }}});
          }
          if (result === undefined && bytesRead === 0) {
            throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
          }
          if (result === null || result === undefined) break;
          bytesRead++;
          buffer[offset+i] = result;
        }
        if (bytesRead) {
          stream.node.timestamp = Date.now();
        }
        return bytesRead;
      },
      write(stream, buffer, offset, length, pos) {
        if (!stream.tty || !stream.tty.ops.put_char) {
          throw new FS.ErrnoError({{{ cDefs.ENXIO }}});
        }
        try {
          for (var i = 0; i < length; i++) {
            stream.tty.ops.put_char(stream.tty, buffer[offset+i]);
          }
        } catch (e) {
          throw new FS.ErrnoError({{{ cDefs.EIO }}});
        }
        if (length) {
          stream.node.timestamp = Date.now();
        }
        return i;
      }
    },
    default_tty_ops: {
      get_char(tty) {
        return FS_stdin_getChar();
      },
      put_char(tty, val) {
        if (val === null || val === {{{ charCode('\n') }}}) {
          out(UTF8ArrayToString(tty.output));
          tty.output = [];
        } else {
          if (val != 0) tty.output.push(val); // val == 0 would cut text output off in the middle.
        }
      },
      fsync(tty) {
        if (tty.output && tty.output.length > 0) {
          out(UTF8ArrayToString(tty.output));
          tty.output = [];
        }
      },
      ioctl_tcgets(tty) {
        // typical setting
        return {
          c_iflag: {{{ cDefs.ICRNL | cDefs.IXON | cDefs.IMAXBEL | cDefs.IUTF8 }}},
          c_oflag: {{{ cDefs.OPOST | cDefs.ONLCR }}},
          c_cflag: {{{ cDefs.B38400 | cDefs.CSIZE | cDefs.CREAD }}},
          c_lflag: {{{ cDefs.ISIG | cDefs.ICANON | cDefs.ECHO | cDefs.ECHOE | cDefs.ECHOK | cDefs.ECHOCTL | cDefs.ECHOKE | cDefs.IEXTEN }}},
          c_cc: [
            0x03, 0x1c, 0x7f, 0x15, 0x04, 0x00, 0x01, 0x00, 0x11, 0x13, 0x1a, 0x00,
            0x12, 0x0f, 0x17, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          ]
        };
      },
      ioctl_tcsets(tty, optional_actions, data) {
        // currently just ignore
        return 0;
      },
      ioctl_tiocgwinsz(tty) {
        return [24, 80];
      }
    },
    default_tty1_ops: {
      put_char(tty, val) {
        if (val === null || val === {{{ charCode('\n') }}}) {
          err(UTF8ArrayToString(tty.output));
          tty.output = [];
        } else {
          if (val != 0) tty.output.push(val);
        }
      },
      fsync(tty) {
        if (tty.output && tty.output.length > 0) {
          err(UTF8ArrayToString(tty.output));
          tty.output = [];
        }
      }
    }
  }
});
