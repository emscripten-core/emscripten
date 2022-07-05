/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $TERMIOS: {},
  $TERMIOS__postset: function() {
    return `
// c_iflag
var ISTRIP = {{{ cDefine('ISTRIP') }}};
var INLCR = {{{ cDefine('INLCR') }}};
var IGNCR = {{{ cDefine('IGNCR') }}};
var ICRNL = {{{ cDefine('ICRNL') }}};
var IUCLC = {{{ cDefine('IUCLC') }}};
var IXON = {{{ cDefine('IXON') }}};
var IXANY = {{{ cDefine('IXANY') }}};
var IMAXBEL = {{{ cDefine('IMAXBEL') }}};
var IUTF8 = {{{ cDefine('IUTF8') }}};

// c_oflag
var OPOST = {{{ cDefine('OPOST') }}};
var OLCUC = {{{ cDefine('OLCUC') }}};
var ONLCR = {{{ cDefine('ONLCR') }}};
var OCRNL = {{{ cDefine('OCRNL') }}};
var ONOCR = {{{ cDefine('ONOCR') }}};
var ONLRET = {{{ cDefine('ONLRET') }}};
var TABDLY = {{{ cDefine('TABDLY') }}};
var XTABS = {{{ cDefine('XTABS') }}};

// c_lflag
var ISIG = {{{ cDefine('ISIG') }}};
var ICANON = {{{ cDefine('ICANON') }}};
var ECHO = {{{ cDefine('ECHO') }}};
var ECHOE = {{{ cDefine('ECHOE') }}};
var ECHOK = {{{ cDefine('ECHOK') }}};
var ECHONL = {{{ cDefine('ECHONL') }}};
var NOFLSH = {{{ cDefine('NOFLSH') }}};
var ECHOCTL = {{{ cDefine('ECHOCTL') }}};
var ECHOPRT = {{{ cDefine('ECHOPRT') }}};
var ECHOKE = {{{ cDefine('ECHOKE') }}};
var IEXTEN = {{{ cDefine('IEXTEN') }}};

// c_cc
var VINTR = {{{ cDefine('VINTR') }}};
var VQUIT = {{{ cDefine('VQUIT') }}};
var VERASE = {{{ cDefine('VERASE') }}};
var VKILL = {{{ cDefine('VKILL') }}};
var VEOF = {{{ cDefine('VEOF') }}};
var VTIME = {{{ cDefine('VTIME') }}};
var VMIN = {{{ cDefine('VMIN') }}};
var VSWTC = {{{ cDefine('VSWTC') }}};
var VSTART = {{{ cDefine('VSTART') }}};
var VSTOP = {{{ cDefine('VSTOP') }}};
var VSUSP = {{{ cDefine('VSUSP') }}};
var VEOL = {{{ cDefine('VEOL') }}};
var VREPRINT = {{{ cDefine('VREPRINT') }}};
var VDISCARD = {{{ cDefine('VDISCARD') }}};
var VWERASE = {{{ cDefine('VWERASE') }}};
var VLNEXT = {{{ cDefine('VLNEXT') }}};
var VEOL2 = {{{ cDefine('VEOL2') }}};

var Termios = /** @constructor */ function(iflag, oflag, cflag, lflag, cc) {
  this.iflag = iflag;
  this.oflag = oflag;
  this.cflag = cflag;
  this.lflag = lflag;
  this.cc = cc;
};

Object.defineProperties(Termios.prototype, {
  ISTRIP_P: {
    get: /** @this{Termios} */function() {
      return (this.iflag & ISTRIP) != 0;
    }
  },
  INLCR_P: {
    get: /** @this{Termios} */function() {
      return (this.iflag & INLCR) != 0;
    }
  },
  IGNCR_P: {
    get: /** @this{Termios} */function() {
      return (this.iflag & IGNCR) != 0;
    }
  },
  ICRNL_P: {
    get: /** @this{Termios} */function() {
      return (this.iflag & ICRNL) != 0;
    }
  },
  IUCLC_P: {
    get: /** @this{Termios} */function() {
      return (this.iflag & IUCLC) != 0;
    }
  },
  IXON_P: {
    get: /** @this{Termios} */function() {
      return (this.iflag & IXON) != 0;
    }
  },
  IXANY_P: {
    get: /** @this{Termios} */function() {
      return (this.iflag & IXANY) != 0;
    }
  },
  IUTF8_P: {
    get: /** @this{Termios} */function() {
      return (this.iflag & IUTF8) != 0;
    }
  },
  OPOST_P: {
    get: /** @this{Termios} */function() {
      return (this.oflag & OPOST) != 0;
    }
  },
  OLCUC_P: {
    get: /** @this{Termios} */function() {
      return (this.oflag & OLCUC) != 0;
    }
  },
  ONLCR_P: {
    get: /** @this{Termios} */function() {
      return (this.oflag & ONLCR) != 0;
    }
  },
  OCRNL_P: {
    get: /** @this{Termios} */function() {
      return (this.oflag & OCRNL) != 0;
    }
  },
  ONOCR_P: {
    get: /** @this{Termios} */function() {
      return (this.oflag & ONOCR) != 0;
    }
  },
  ONLRET_P: {
    get: /** @this{Termios} */function() {
      return (this.oflag & ONLRET) != 0;
    }
  },
  TABDLY_XTABS_P: {
    get: /** @this{Termios} */function() {
      return (this.oflag & TABDLY) == XTABS;
    }
  },
  ISIG_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & ISIG) != 0;
    }
  },
  ICANON_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & ICANON) != 0;
    }
  },
  ECHO_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & ECHO) != 0;
    }
  },
  ECHOE_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & ECHOE) != 0;
    }
  },
  ECHOK_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & ECHOK) != 0;
    }
  },
  ECHONL_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & ECHONL) != 0;
    }
  },
  NOFLSH_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & NOFLSH) != 0;
    }
  },
  ECHOCTL_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & ECHOCTL) != 0;
    }
  },
  ECHOPRT_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & ECHOPRT) != 0;
    }
  },
  ECHOKE_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & ECHOKE) != 0;
    }
  },
  IEXTEN_P: {
    get: /** @this{Termios} */function() {
      return (this.lflag & IEXTEN) != 0;
    }
  },

  INTR_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VINTR];
    }
  },
  QUIT_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VQUIT];
    }
  },
  ERASE_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VERASE];
    }
  },
  KILL_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VKILL];
    }
  },
  EOF_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VEOF];
    }
  },
  TIME_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VTIME]; // not supported
    }
  },
  MIN_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VMIN]; // not supported
    }
  },
  SWTCH_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VSWTC]; // not supported
    }
  },
  START_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VSTART];
    }
  },
  STOP_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VSTOP];
    }
  },
  SUSP_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VSUSP];
    }
  },
  EOL_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VEOL];
    }
  },
  REPRINT_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VREPRINT];
    }
  },
  DISCARD_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VDISCARD]; // not supported
    }
  },
  WERASE_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VWERASE];
    }
  },
  LNEXT_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VLNEXT];
    }
  },
  EOL2_V: {
    get: /** @this{Termios} */function() {
      return this.cc[VEOL2];
    }
  }
});
TERMIOS.Termios = Termios;
TERMIOS.toData = (termios) => {
  var data = [termios.iflag, termios.oflag, termios.cflag, termios.lflag];
  let word = 0;
  let offset = 8;
  for (let i = 0; i < termios.cc.length; i++) {
    word |= termios.cc[i] << offset;
    offset += 8;
    if (offset == 32) {
      data.push(word);
      word = 0;
      offset = 0;
    }
  }
  data.push(word);
  return data;
};

TERMIOS.fromData = (data) => {
  let cc = [];
  let ptr = 4;
  let word = data[ptr++];
  let offset = 8;
  for (let i = 0; i < 32; i++) {
    cc.push((word >> offset) & 0xff);
    offset += 8;
    if (offset >= 32) {
      word = data[ptr++];
      offset = 0;
    }
  }
  return new Termios(data[0], data[1], data[2], data[3], cc);
}
`;
  } /* $TERMIOS__postset */
});

