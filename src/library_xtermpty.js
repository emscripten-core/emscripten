/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $XTERMPTY__deps: ['$ENV', '$TERMIOS'],
#if !MINIMAL_RUNTIME
  $XTERMPTY__postset: function() {
    addAtInit('XTERMPTY.init();');
  },
#endif
  $XTERMPTY: {
    init: function () {
      if (Module['tty']) {
        ENV["TERM"] = "xterm-256color";
      }
    },
    tty_ops: {
      get_char: function(tty) {
        if (!tty.input.length) {
          tty.input.push(...Module['tty'].onRead());
        }
        var c = tty.input.shift();
        return c >= 0 ? (c < 128 ? c : c - 256) : null;
      },
      put_char: function(tty, val) {
        Module['tty'].onWrite([(val + 256) % 256]);
      },
      flush: function (tty) { /* nop */ },
      ioctl: function(tty, op, argp) {
        switch (op) {
          case {{{ cDefine('TCGETS') }}}: {
            if (!Module['tty']) return 0;
            var tios = Module['tty'].onIoctlTcgets();
            HEAP32.set(TERMIOS.toData(tios), argp >> 2);
            return 0;
          }
          case {{{ cDefine('TCSETS') }}}:
          case {{{ cDefine('TCSETSW') }}}:
          case {{{ cDefine('TCSETSF') }}}: {
            if (!Module['tty']) return 0;
            var tios = Array.from(HEAP32.slice(argp >> 2, (argp + {{{ C_STRUCTS.termios.__size__ }}}) >> 2));
            Module['tty'].onIoctlTcsets(TERMIOS.fromData(tios));
            return 0;
          }
          case {{{ cDefine('TCGETA') }}}:
          case {{{ cDefine('TCSETA') }}}:
          case {{{ cDefine('TCSETAW') }}}:
          case {{{ cDefine('TCSETAF') }}}: {
            if (!Module['tty']) return 0;
            return {{{ cDefine('EINVAL') }}};
          }
          case {{{ cDefine('TIOCGPGRP') }}}: {
            {{{ makeSetValue('argp', 0, 0, 'i32') }}};
            return 0;
          }
          case {{{ cDefine('TIOCSPGRP') }}}: {
            return -{{{ cDefine('EINVAL') }}}; // not supported
          }
          case {{{ cDefine('TIOCGWINSZ') }}}: {
            if (!Module['tty']) return 0;
            var sz = Module['tty'].onIoctlTiocgwinsz();
            {{{ makeSetValue('argp', 0, 'sz[1] + (sz[0] << 16)', 'i32') }}}
            return 0;
          }
          case {{{ cDefine('TIOCSWINSZ') }}}: {
            if (!Module['tty']) return 0;
            return -{{{ cDefine('EINVAL') }}}; // not supported
          }
          case {{{ cDefine('FIONREAD') }}}: {
            var avail = tty.input.length;
            if (avail > 0 && tty.input[avail-1] < 0) {
              avail--;
            }
            {{{ makeSetValue('argp', 0, 'avail', 'i32') }}}
            return 0;
          }
          default: {
            return TTY.default_tty_op_ioctl(tty, op, argp);
          }
        }
      }
    }
  }
});

if (XTERMPTY) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$XTERMPTY');
}
