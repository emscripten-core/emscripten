var Library = {
  // stdio.h

  printf: function() {
    __print__(Pointer_stringify(__formatString.apply(null, arguments)));
  },

  fprintf: function() {
    var file = arguments[0]; // TODO: something clever with this
    var args = Array.prototype.slice.call(arguments, 1);
    __print__(Pointer_stringify(__formatString.apply(null, args)));
  },

  sprintf: function() {
    var str = arguments[0];
    var args = Array.prototype.slice.call(arguments, 1);
    _strcpy(str, __formatString.apply(null, args)); // not terribly efficient
  },

  fflush: function(file) {
    __print__(null);
  },

  puts: function(p) {
    __print__(Pointer_stringify(p) + '\n');
  },

  fputs: function(p, stream) {
    __print__(Pointer_stringify(p) + '\n');
  },

  putchar: function(p) {
    __print__(String.fromCharCode(p));
  },
  _ZNSo3putEc: 'putchar',

  fopen: function(filename, mode) {
    return 1; // XXX
  },

  _IO_getc: function(file) {
    return -1; // EOF
  },

  ungetc: function(chr, stream) {
    return chr;
  },

  feof: function(stream) {
    return 1;
  },

  ferror: function(stream) {
    return 0;
  },

  fclose: function(stream) {
    return 0;
  },

  _ZNSo5flushEv: function() {
    __print__('\n');
  },

  vsnprintf: function(dst, num, src, ptr) {
    var text = __formatString(-src, ptr); // |-|src tells formatstring to use C-style params (typically they are from varargs)
    for (var i = 0; i < num; i++) {
      IHEAP[dst+i] = IHEAP[text+i];
      if (IHEAP[dst+i] == 0) break;
    }
  },

  fileno: function(file) {
    return 1; // XXX
  },

  isatty: function(file) {
    return 0; // XXX
  },

  // stdlib.h

  abs: 'Math.abs',

  atexit: function(func) {
    __ATEXIT__.push(func);
  },
  __cxa_atexit: 'atexit',

  abort: function(code) {
    ABORT = true;
    throw 'ABORT: ' + code + ', at ' + (new Error().stack);
  },

  realloc: function(ptr, size) {
    // Very simple, inefficient implementation - if you use a real malloc, best to use
    // a real realloc with it
    if (!size) {
      if (ptr) _free(ptr);
      return 0;
    }
    var ret = _malloc(size);
    if (ptr) {
      _memcpy(ret, ptr, size); // might be some invalid reads
      _free(ptr);
    }
    return ret;
  },

  getenv: function(name_) {
    return 0; // TODO
  },

  strtod: function(str, endptr) {
    // XXX handles only whitespace + |[0-9]+(.[0.9]+)?|, no e+
    while (_isspace(str)) str++;
    var chr;
    var ret = 0;
    while(1) {
      chr = IHEAP[str];
      if (!_isdigit(chr)) break;
      ret = ret*10 + chr - '0'.charCodeAt(0);
      str++;
    }
    if (IHEAP[str] == '.'.charCodeAt(0)) {
      str++;
      var mul=1/10;
      while(1) {
        chr = IHEAP[str];
        if (!_isdigit(chr)) break;
        ret += mul*(chr - '0'.charCodeAt(0));
        mul /= 10;
        str++;
      }
    }
    if (endptr) {
      IHEAP[endptr] = str;
#if SAFE_HEAP
      SAFE_HEAP_ACCESS(endptr, null, true);
#endif
    }
    return ret;
  },

  qsort: function(base, num, size, comparator) {
    // forward calls to the JavaScript sort method
    // first, sort the items logically
    comparator = FUNCTION_TABLE[comparator];
    var keys = [];
    for (var i = 0; i < num; i++) keys.push(i);
    keys.sort(function(a, b) {
      return comparator(base+a*size, base+b*size);
    });
    print("KEYS:" + keys)
    // apply the sort
    var temp = _malloc(num*size);
    _memcpy(temp, base, num*size);
    for (var i = 0; i < num; i++) {
      if (keys[i] == i) continue; // already in place
      _memcpy(base+i*size, temp+keys[i]*size, size);
    }
    _free(temp);
  },

  // string.h

  strspn: function(pstr, pset) {
    var str = String_copy(pstr, true);
    var set = String_copy(pset);
    var i = 0;
    while (set.indexOf(str[i]) != -1) i++; // Must halt, as 0 is in str but not set
    return i;
  },

  strcspn: function(pstr, pset) {
    var str = String_copy(pstr, true);
    var set = String_copy(pset, true);
    var i = 0;
    while (set.indexOf(str[i]) == -1) i++; // Must halt, as 0 is in both
    return i;
  },

  strcpy: function(pdest, psrc) {
    var i = 0;
    do {
#if SAFE_HEAP
      SAFE_HEAP_STORE(pdest+i, IHEAP[psrc+i], null);
#else
      IHEAP[pdest+i] = IHEAP[psrc+i];
#endif
      i ++;
    } while (IHEAP[psrc+i-1] != 0);
  },

  strncpy: function(pdest, psrc, num) {
    var padding = false;
    for (var i = 0; i < num; i++) {
#if SAFE_HEAP
      SAFE_HEAP_STORE(pdest+i, padding ? 0 : IHEAP[psrc+i], null);
#else
      IHEAP[pdest+i] = padding ? 0 : IHEAP[psrc+i];
#endif
      padding = padding || IHEAP[psrc+i] == 0;
    }
  },

  strcat: function(pdest, psrc) {
    var len = Pointer_stringify(pdest).length; // TODO: use strlen, but need dependencies system
    var i = 0;
    do {
      IHEAP[pdest+len+i] = IHEAP[psrc+i];
      i ++;
    } while (IHEAP[psrc+i-1] != 0);
  },

  strtol: function(ptr) {
    // XXX: We ignore the other two params!
    return parseInt(Pointer_stringify(ptr));
  },

  strcmp: function(px, py) {
    var i = 0;
    while (true) {
      var x = IHEAP[px+i];
      var y = IHEAP[py+i];
      if (x == y && x == 0) return 0;
      if (x == 0) return -1;
      if (y == 0) return 1;
      if (x == y) {
        i ++;
        continue;
      } else {
        return x > y ? 1 : -1;
      }
    }
  },

  memcmp: function(p1, p2, num) {
    for (var i = 0; i < num; i++) {
      var v1 = IHEAP[p1+i];
      var v2 = IHEAP[p2+i];
      if (v1 != v2) return v1 > v2 ? 1 : -1;
    }
    return 0;
  },

  strchr: function(ptr, chr) {
    ptr--;
    do {
      ptr++;
      var val = IHEAP[ptr];
      if (val == chr) return ptr;
    } while (val);
    return 0;
  },

  // ctype.h

  isdigit: function(chr) {
    return chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0);
  },

  isxdigit: function(chr) {
    return (chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0)) ||
           (chr >= 'a'.charCodeAt(0) && chr <= 'f'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'F'.charCodeAt(0));
  },

  isalpha: function(chr) {
    return (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0));
  },

  isalnum: function(chr) {
    return (chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0)) ||
           (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0));
  },

  isspace: function(chr) {
    return chr in { 32: 0, 9: 0, 10: 0, 11: 0, 12: 0, 13: 0 };
  },

  iscntrl: function(chr) {
    return (chr >= 0 && chr <= 0x1f) || chr === 0x7f;
  },

  toupper: function(chr) {
    if (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) {
      return chr - 'a'.charCodeAt(0) + 'A'.charCodeAt(0);
    }
    return chr;
  },

  // LLVM specifics

  __assert_fail: function(condition, file, line) {
    ABORT = true;
    throw 'Assertion failed: ' + Pointer_stringify(condition);//JSON.stringify(arguments)//condition;
  },

  __cxa_guard_acquire: function() {
    return 1;
  },
  __cxa_guard_release: function() {
    return 1;
  },

  // Exceptions - minimal support, only (...) for now (no actual exception objects can be caught)
  __cxa_allocate_exception: function(size) {
    return _malloc(size); // warning: leaked
  },
  __cxa_throw: function(ptr, data, dunno) {
#if EXCEPTION_DEBUG
    print('Compiled code throwing an exception, ' + [ptr,data,dunno] + ', at ' + new Error().stack);
#endif
    throw ptr;
  },
  llvm_eh_exception: function() {
    return 'code-generated exception: ' + (new Error().stack);
  },
  llvm_eh_selector: function(exception, personality, num) {
    return 0;
  },
  __cxa_begin_catch: function(ptr) {
  },
  __cxa_end_catch: function(ptr) {
  },

  __cxa_call_unexpected: function(exception) {
    ABORT = true;
    throw exception;
  },

  __gxx_personality_v0: function() {
  },

  llvm_umul_with_overflow_i32: function(x, y) {
    return {
      f0: x*y,
      f1: 0 // We never overflow... for now
    };
  },

  llvm_stacksave: function() {
    var self = _llvm_stacksave;
    if (!self.LLVM_SAVEDSTACKS) {
      self.LLVM_SAVEDSTACKS = [];
    }
    self.LLVM_SAVEDSTACKS.push(STACKTOP);
    return self.LLVM_SAVEDSTACKS.length-1;
  },
  llvm_stackrestore: function(p) {
    var self = _llvm_stacksave;
    var ret = self.LLVM_SAVEDSTACKS[p];
    self.LLVM_SAVEDSTACKS.splice(p, 1);
    return ret;
  },

  __cxa_pure_virtual: function() {
    ABORT = true;
    throw 'Pure virtual function called!';
  },

  // iostream

  _ZNSt8ios_base4InitC1Ev: function() {
    // need valid 'file descriptors'
    __ZSt4cout = 1;
    __ZSt4cerr = 2;
  },
  _ZNSt8ios_base4InitD1Ev: '_ZNSt8ios_base4InitC1Ev',
  _ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_: 0, // endl
  _ZNSolsEi: function(stream, data) {
    __print__(data);
  },
  _ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc: function(stream, data) {
    __print__(Pointer_stringify(data));
  },
  _ZNSolsEd: function(stream, data) {
    __print__('\n');
  },
  _ZNSolsEPFRSoS_E: function(stream, data) {
    __print__('\n');
  },
  _ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_i: function(stream, data, call_) {
    __print__(Pointer_stringify(data));
  },

  // math.h

  cos: 'Math.cos',
  cosf: 'Math.cos',
  sin: 'Math.sin',
  sinf: 'Math.sin',
  tan: 'Math.tan',
  tanf: 'Math.tan',
  acos: 'Math.acos',
  acosf: 'Math.acos',
  asin: 'Math.asin',
  asinf: 'Math.asin',
  atan: 'Math.atan',
  atanf: 'Math.atan',
  atan2: 'Math.atan2',
  atan2f: 'Math.atan2',
  sqrt: 'Math.sqrt',
  sqrtf: 'Math.sqrt',
  fabs: 'Math.abs',
  fabsf: 'Math.abs',
  llvm_sqrt_f64: 'Math.sqrt',
  llvm_pow_f32: 'Math.pow',

  // unistd.h

  sysconf: function(name_) {
    switch(name_) {
      case 30: return PAGE_SIZE; // _SC_PAGE_SIZE
      default: throw 'unknown sysconf param: ' + name_;
    }
  },

  sbrk: function(bytes) {
    // Implement a Linux-like 'memory area' for our 'process'.
    // Changes the size of the memory area by |bytes|; returns the
    // address of the previous top ('break') of the memory area

    // We need to make sure no one else allocates unfreeable memory!
    // We must control this entirely. So we don't even need to do
    // unfreeable allocations - the HEAP is ours, from STATICTOP up.
    // TODO: We could in theory slice off the top of the HEAP when
    // sbrk gets a negative increment in |bytes|...
    var self = arguments.callee;
    if (!self.STATICTOP) {
      STATICTOP = alignMemoryPage(STATICTOP);
      self.STATICTOP = STATICTOP;
      self.DATASIZE = 0;
    } else {
      assert(self.STATICTOP == STATICTOP, "Noone should touch the heap!");
    }
    var ret = STATICTOP + self.DATASIZE;
    self.DATASIZE += alignMemoryPage(bytes);
    return ret; // previous break location
  },

  // time.h

  time: function(ptr) {
    var ret = Math.floor(Date.now()/1000);
    if (ptr) {
      IHEAP[ptr] = ret;
    }
    return ret;
  },

  gettimeofday: function(ptr) {
    // %struct.timeval = type { i32, i32 }
    var indexes = Runtime.calculateStructAlignment({ fields: ['i32', 'i32'] });
    var now = Date.now();
    IHEAP[ptr + indexes[0]] = Math.floor(now/1000); // seconds
    IHEAP[ptr + indexes[1]] = Math.floor((now-1000*Math.floor(now/1000))*1000); // microseconds
#if SAFE_HEAP
    SAFE_HEAP_ACCESS(ptr + indexes[0], 'i32', true);
    SAFE_HEAP_ACCESS(ptr + indexes[1], 'i32', true);
#endif
    return 0;
  },

  // setjmp.h

  _setjmp: function(env) {
    // not really working...
    assert(!arguments.callee.called);
    arguments.callee.called = true;
    return 0;
  },

  _longjmp: function(env, val) {
    // not really working...
    assert(0);
  },

  // signal.h

  signal: function(sig, func) {
    // TODO
    return 0;
  },
};

load('library_sdl.js');

