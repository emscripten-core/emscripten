var Library = {
  // stdio.h

  printf: function() {
    __print__(Pointer_stringify(__formatString.apply(null, arguments)));
  },

  puts: function(p) {
    __print__(Pointer_stringify(p) + '\n');
  },

  putchar: function(p) {
    __print__(String.fromCharCode(p));
  },
  _ZNSo3putEc: 'putchar',

  _ZNSo5flushEv: function() {
    __print__('\n');
  },

  vsnprintf: function(dst, num, src, ptr) {
    var args = Array_copy(ptr+1, IHEAP[ptr]); // # of args in in first place
    var text = __formatString.apply(null, [src].concat(args));
    for (var i = 0; i < num; i++) {
      IHEAP[dst+i] = IHEAP[text+i];
      if (IHEAP[dst+i] == 0) break;
    }
  },

  // stdlib.h

  atexit: function(func) {
    __ATEXIT__.push(func);
  },
  __cxa_atexit: 'atexit',

  abort: function(code) {
    ABORT = true;
    throw 'ABORT: ' + code + ', at ' + (new Error().stack);
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
      IHEAP[pdest+i] = IHEAP[psrc+i];
      i ++;
    } while (IHEAP[psrc+i-1] != 0);
  },

  strncpy: function(pdest, psrc, num) {
    var padding = false;
    for (var i = 0; i < num; i++) {
      IHEAP[pdest+i] = padding ? 0 : IHEAP[psrc+i];
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

  isdigit: function(chr) {
    return chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0);
  },

  memcmp: function(p1, p2, num) {
    for (var i = 0; i < num; i++) {
      var v1 = IHEAP[p1+i];
      var v2 = IHEAP[p2+i];
      if (v1 != v2) return v1 > v2 ? 1 : -1;
    }
    return 0;
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

  llvm_eh_exception: function() {
    return 'code-generated exception: ' + (new Error().stack);
  },

  llvm_eh_selector: function(exception, personality, num) {
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
    return 0;
  },
};

load('library_sdl.js');

