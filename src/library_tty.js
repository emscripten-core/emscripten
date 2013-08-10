mergeInto(LibraryManager.library, {
  $TTY__deps: ['$FS'],
  $TTY: {
    ttys: [],
    register: function(dev, ops) {
      TTY.ttys[dev] = { input: [], output: [], ops: ops };
      FS.registerDevice(dev, TTY.stream_ops);
    },
    stream_ops: {
      open: function(stream) {
        // this wouldn't be required if the library wasn't eval'd at first...
        if (!TTY.utf8) {
          TTY.utf8 = new Runtime.UTF8Processor();
        }
        var tty = TTY.ttys[stream.node.rdev];
        if (!tty) {
          throw new FS.ErrnoError(ERRNO_CODES.ENODEV);
        }
        stream.tty = tty;
        stream.seekable = false;
      },
      close: function(stream) {
        // flush any pending line data
        if (stream.tty.output.length) {
          stream.tty.ops.put_char(stream.tty, {{{ charCode('\n') }}});
        }
      },
      read: function(stream, buffer, offset, length, pos /* ignored */) {
        if (!stream.tty || !stream.tty.ops.get_char) {
          throw new FS.ErrnoError(ERRNO_CODES.ENXIO);
        }
        var bytesRead = 0;
        for (var i = 0; i < length; i++) {
          var result;
          try {
            result = stream.tty.ops.get_char(stream.tty);
          } catch (e) {
            throw new FS.ErrnoError(ERRNO_CODES.EIO);
          }
          if (result === undefined && bytesRead === 0) {
            throw new FS.ErrnoError(ERRNO_CODES.EAGAIN);
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
      write: function(stream, buffer, offset, length, pos) {
        if (!stream.tty || !stream.tty.ops.put_char) {
          throw new FS.ErrnoError(ERRNO_CODES.ENXIO);
        }
        for (var i = 0; i < length; i++) {
          try {
            stream.tty.ops.put_char(stream.tty, buffer[offset+i]);
          } catch (e) {
            throw new FS.ErrnoError(ERRNO_CODES.EIO);
          }
        }
        if (length) {
          stream.node.timestamp = Date.now();
        }
        return i;
      }
    },
    // NOTE: This is weird to support stdout and stderr
    // overrides in addition to print and printErr overrides.
    default_tty_ops: {
      get_char: function(tty) {
        if (!tty.input.length) {
          var result = null;
          if (ENVIRONMENT_IS_NODE) {
            if (process.stdin.destroyed) {
              return undefined;
            }
            result = process.stdin.read();
          } else if (typeof window != 'undefined' &&
            typeof window.prompt == 'function') {
            // Browser.
            result = window.prompt('Input: ');  // returns null on cancel
            if (result !== null) {
              result += '\n';
            }
          } else if (typeof readline == 'function') {
            // Command line.
            result = readline();
            if (result !== null) {
              result += '\n';
            }
          }
          if (!result) {
            return null;
          }
          tty.input = intArrayFromString(result, true);
        }
        return tty.input.shift();
      },
      put_char: function(tty, val) {
        if (val === null || val === {{{ charCode('\n') }}}) {
          Module['print'](tty.output.join(''));
          tty.output = [];
        } else {
          tty.output.push(TTY.utf8.processCChar(val));
        }
      }
    },
    default_tty1_ops: {
      put_char: function(tty, val) {
        if (val === null || val === {{{ charCode('\n') }}}) {
          Module['printErr'](tty.output.join(''));
          tty.output = [];
        } else {
          tty.output.push(TTY.utf8.processCChar(val));
        }
      }
    }
  }
});