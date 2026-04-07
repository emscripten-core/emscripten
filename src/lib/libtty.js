/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $TTY__deps: [
    '$DEV',
    '$FS',
    '$UTF8ArrayToString',
    '$FS_stdin_getChar',
  ],
  $TTY: {
    ttys: {},
    register(dev, ops) {
      const tty = { input: [], output: [], ops };
      TTY.ttys[dev] = tty;
      const devops = {
        tty,
        write(devops, buffer) {
          if (!ops.put_char) {
            throw new FS.ErrnoError({{{ cDefs.ENXIO }}});
          }
          for (var i = 0; i < buffer.length; i++) {
            ops.put_char(tty, buffer[i]);
          }
          return i;
        },
        read(devops, buffer) {
          if (!ops.get_char) {
            throw new FS.ErrnoError({{{ cDefs.ENXIO }}});
          }
          var bytesRead = 0;
          for (var i = 0; i < buffer.length; i++) {
            var result = ops.get_char(tty);
            if (result === undefined && bytesRead === 0) {
              throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
            }
            if (result === null || result === undefined) break;
            bytesRead++;
            buffer[i] = result;
          }
          return bytesRead;
        },
      };
      if (ops.fsync) {
        devops.fsync = (devops) => ops.fsync(tty)
      }
      DEV.register(dev, devops);
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
        if (tty.output?.length > 0) {
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
        if (tty.output?.length > 0) {
          err(UTF8ArrayToString(tty.output));
          tty.output = [];
        }
      }
    }
  }
});
