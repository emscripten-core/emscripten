
// See settings.js, CORRUPTION_CHECK

var CorruptionChecker = {
  BUFFER_FACTOR: {{{ CORRUPTION_CHECK }}},

  ptrs: {},

  init: function() {
    this.realMalloc = _malloc;
    _malloc = Module['_malloc'] = this.malloc;

    this.realFree = _free;
    _free = Module['_free'] = this.free;
  },
  malloc: function(size) {
    assert(size > 0); // some mallocs accept zero - fix your code if you want to use this tool
    var allocation = CorruptionChecker.realMalloc(size*(1+2*CorruptionChecker.BUFFER_FACTOR));
    var ptr = allocation + size*CorruptionChecker.BUFFER_FACTOR;
    assert(!CorruptionChecker.ptrs[ptr]);
    CorruptionChecker.ptrs[ptr] = size;
    CorruptionChecker.fillBuffer(allocation, size*CorruptionChecker.BUFFER_FACTOR);
    CorruptionChecker.fillBuffer(allocation + size*(1+CorruptionChecker.BUFFER_FACTOR), size*CorruptionChecker.BUFFER_FACTOR);
    return ptr;
  },
  free: function(ptr) {
    CorruptionChecker.checkPtr(ptr, true);
  },
  canary: function(x) {
    return (x + (x << 3) + (x&75) - (x&47))&255;
  },
  fillBuffer: function(allocation, size) {
    for (var x = allocation; x < allocation + size; x++) {
      {{{ makeSetValue('x', 0, 'CorruptionChecker.canary(x)', 'i8') }}};
    }
  },
  checkBuffer: function(allocation, size) {
    for (var x = allocation; x < allocation + size; x++) {
      assert(({{{ makeGetValue('x', 0, 'i8') }}}&255) == CorruptionChecker.canary(x), 'Heap corruption detected!');
    }
  },
  checkPtr: function(ptr, free) {
    var size = CorruptionChecker.ptrs[ptr];
    assert(size);
    var allocation = ptr - size*CorruptionChecker.BUFFER_FACTOR;
    CorruptionChecker.checkBuffer(allocation, size*CorruptionChecker.BUFFER_FACTOR);
    CorruptionChecker.checkBuffer(allocation + size*(1+CorruptionChecker.BUFFER_FACTOR), size*CorruptionChecker.BUFFER_FACTOR);
    if (free) {
      delete CorruptionChecker.ptrs[ptr];
      CorruptionChecker.realFree(allocation);
    }
  },
  checkAll: function() {
    for (var ptr in CorruptionChecker.ptrs) {
      CorruptionChecker.checkPtr(ptr, false);
    }
  },
};

CorruptionChecker.init();

