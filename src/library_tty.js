mergeInto(LibraryManager.library, {
  $TTY__deps: ['$FS'],
  $TTY__postset: '__ATINIT__.unshift(function() { TTY.init() });' +
                 '__ATEXIT__.push(function() { TTY.shutdown() });',
  $TTY: {
    ttys: [],
    init: function () {
      // https://github.com/kripken/emscripten/pull/1555
      // if (ENVIRONMENT_IS_NODE) {
      //   // currently, FS.init does not distinguish if process.stdin is a file or TTY
      //   // device, it always assumes it's a TTY device. because of this, we're forcing
      //   // process.stdin to UTF8 encoding to at least make stdin reading compatible
      //   // with text files until FS.init can be refactored.
      //   process['stdin']['setEncoding']('utf8');
      // }
    },
    shutdown: function() {
      // https://github.com/kripken/emscripten/pull/1555
      // if (ENVIRONMENT_IS_NODE) {
      //   // inolen: any idea as to why node -e 'process.stdin.read()' wouldn't exit immediately (with process.stdin being a tty)?
      //   // isaacs: because now it's reading from the stream, you've expressed interest in it, so that read() kicks off a _read() which creates a ReadReq operation
      //   // inolen: I thought read() in that case was a synchronous operation that just grabbed some amount of buffered data if it exists?
      //   // isaacs: it is. but it also triggers a _read() call, which calls readStart() on the handle
      //   // isaacs: do process.stdin.pause() and i'd think it'd probably close the pending call
      //   process['stdin']['pause']();
      // }
    },
    register: function(dev, ops) {
      TTY.ttys[dev] = { input: [], output: [], ops: ops };
      FS.registerDevice(dev, TTY.stream_ops);
    },
    stream_ops: {
      open: function(stream) {
        var tty = TTY.ttys[stream.node.rdev];
        if (!tty) {
          throw new FS.ErrnoError(ERRNO_CODES.ENODEV);
        }
        stream.tty = tty;
        stream.seekable = false;
      },
      close: function(stream) {
        // flush any pending line data
        stream.tty.ops.flush(stream.tty);
      },
      flush: function(stream) {
        stream.tty.ops.flush(stream.tty);
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
    default_tty_ops: {
      // get_char has 3 particular return values:
      // a.) the next character represented as an integer
      // b.) undefined to signal that no data is currently available
      // c.) null to signal an EOF
      get_char: function(tty) {
        if (!tty.input.length) {
          var result = null;
          if (ENVIRONMENT_IS_NODE) {
            // we will read data by chunks of BUFSIZE
            var BUFSIZE = 256;
            var buf = new Buffer(BUFSIZE);
            var bytesRead = 0;

            var isPosixPlatform = (process.platform != 'win32'); // Node doesn't offer a direct check, so test by exclusion

            var fd = process.stdin.fd;
            if (isPosixPlatform) {
              // Linux and Mac cannot use process.stdin.fd (which isn't set up as sync)
              var usingDevice = false;
              try {
                fd = fs.openSync('/dev/stdin', 'r');
                usingDevice = true;
              } catch (e) {}
            }

            try {
              bytesRead = fs.readSync(fd, buf, 0, BUFSIZE, null);
            } catch(e) {
              // Cross-platform differences: on Windows, reading EOF throws an exception, but on other OSes,
              // reading EOF returns 0. Uniformize behavior by treating the EOF exception to return 0.
              if (e.toString().indexOf('EOF') != -1) bytesRead = 0;
              else throw e;
            }

            if (usingDevice) { fs.closeSync(fd); }
            if (bytesRead > 0) {
              result = buf.slice(0, bytesRead).toString('utf-8');
            } else {
              result = null;
            }

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
          Module['print'](UTF8ArrayToString(tty.output, 0));
          tty.output = [];
        } else {
          if (val != 0) tty.output.push(val); // val == 0 would cut text output off in the middle.
        }
      },
      flush: function(tty) {
        if (tty.output && tty.output.length > 0) {
          Module['print'](UTF8ArrayToString(tty.output, 0));
          tty.output = [];
        }
      }
    },
    default_tty1_ops: {
      put_char: function(tty, val) {
        if (val === null || val === {{{ charCode('\n') }}}) {
          Module['printErr'](UTF8ArrayToString(tty.output, 0));
          tty.output = [];
        } else {
          if (val != 0) tty.output.push(val);
        }
      },
      flush: function(tty) {
        if (tty.output && tty.output.length > 0) {
          Module['printErr'](UTF8ArrayToString(tty.output, 0));
          tty.output = [];
        }
      }
    }
  }
});
