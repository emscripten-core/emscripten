function asm(x, y) {
  x = +x;
  y = y | 0;
  var a = 0;
  a = cheez(y + ~~x | 0) | 0;
  fleefl(a * a | 0, a | 0);
}
function __Z11printResultPiS_j($needle, $haystack, $len) {
  $needle = $needle | 0;
  $haystack = $haystack | 0;
  $len = $len | 0;
  var $3 = 0, __stackBase__ = 0;
  __stackBase__ = STACKTOP;
  $3 = _bsearch($needle, $haystack, $len, 4, 2);
  if (($3 | 0) == 0) {
    _puts(_str | 0);
    STACKTOP = __stackBase__;
    return;
  } else {
    _printf(__str1 | 0, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[(tempInt & 16777215) >> 2] = HEAP32[($3 & 16777215) >> 2] | 0, tempInt));
    STACKTOP = __stackBase__;
    return;
  }
}
function _segment_holding($addr) {
  $addr = $addr | 0;
  var $sp_0 = 0, $3 = 0, $12 = 0, $_0 = 0, label = 0;
  $sp_0 = __gm_ + 444 | 0;
  while (1) {
    $3 = HEAP32[(($sp_0 | 0) & 16777215) >> 2] | 0;
    if (!($3 >>> 0 > $addr >>> 0)) {
      if (($3 + (HEAP32[(($sp_0 + 4 | 0) & 16777215) >> 2] | 0) | 0) >>> 0 > $addr >>> 0) {
        $_0 = $sp_0;
        label = 1658;
        break;
      }
    }
    $12 = HEAP32[(($sp_0 + 8 | 0) & 16777215) >> 2] | 0;
    if (($12 | 0) == 0) {
      $_0 = 0;
      label = 1659;
      break;
    } else {
      $sp_0 = $12;
    }
  }
  if (label == 1659) {
    return $_0;
  } else if (label == 1658) {
    return $_0;
  }
}
function __ZN5identC2EiPKcPci($this, $n, $a) {
  $this = $this | 0;
  $n = $n | 0;
  $a = $a | 0;
  HEAP32[($this & 16777215) >> 2] = __ZTV5ident + 8 | 0;
  HEAP32[($this + 4 & 16777215) >> 2] = 5;
  HEAP32[($this + 8 & 16777215) >> 2] = $n;
  HEAP32[($this + 20 & 16777215) >> 2] = 2147483647;
  HEAP32[($this + 24 & 16777215) >> 2] = 0;
  HEAP32[($this + 28 & 16777215) >> 2] = $a;
  HEAP32[($this + 32 & 16777215) >> 2] = 0;
  HEAP32[($this + 40 & 16777215) >> 2] = 1;
  return;
}
function _vec2Length($this) {
  $this = $this | 0;
  var $__first_addr_i = 0, $__last_addr_i = 0, __stackBase__ = 0;
  __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 8 | 0;
  $__first_addr_i = __stackBase__;
  $__last_addr_i = __stackBase__ + 4;
  STACKTOP = __stackBase__;
  return 0;
}
function exc($this) {
  $this = $this | 0;
  var $1 = 0, $5 = 0;
  $1 = (function() {
    try {
      __THREW__ = false;
      return __ZNSt3__16locale8__globalEv();
    } catch (e) {
      if (typeof e != "number") throw e;
      if (ABORT) throw e;
      __THREW__ = true;
      Module.print("Exception: " + e + ", currently at: " + (new Error).stack);
      return null;
    }
  })();
  if (!__THREW__) {
    $5 = HEAP32[(($1 | 0) & 16777215) >> 2] | 0;
    HEAP32[(($this | 0) & 16777215) >> 2] = $5;
    __ZNSt3__114__shared_count12__add_sharedEv($5 | 0);
    return;
  } else {
    $8$0 = ___cxa_find_matching_catch(HEAP32[(_llvm_eh_exception.buf & 16777215) >> 2] | 0, HEAP32[(_llvm_eh_exception.buf + 4 & 16777215) >> 2] | 0, []);
    $8$1 = tempRet0;
    ___cxa_call_unexpected($8$0);
  }
}
function label() {
  if (f()) {
    g();
  }
  L100 : if (h()) {
    i();
  }
}

