
// See settings.js, CORRUPTION_CHECK

var CorruptionChecker = {
  BUFFER_FACTOR: Math.round({{{ CORRUPTION_CHECK }}}),

  ptrs: {},
  checks: 0,

  init: function() {
    this.realMalloc = _malloc;
    _malloc = Module['_malloc'] = this.malloc;

    this.realFree = _free;
    _free = Module['_free'] = this.free;

    __ATEXIT__.push({ func: function() {
      Module.printErr('No corruption detected, ran ' + CorruptionChecker.checks + ' checks.');
    } });
  },
  malloc: function(size) {
    if (size <= 0) size = 1; // malloc(0) sometimes happens - just allocate a larger area, no harm
    CorruptionChecker.checkAll();
    size = (size+7)&(~7);
    var allocation = CorruptionChecker.realMalloc(size*(1+2*CorruptionChecker.BUFFER_FACTOR));
    var ptr = allocation + size*CorruptionChecker.BUFFER_FACTOR;
    assert(!CorruptionChecker.ptrs[ptr]);
    CorruptionChecker.ptrs[ptr] = size;
    CorruptionChecker.fillBuffer(allocation, size*CorruptionChecker.BUFFER_FACTOR);
    CorruptionChecker.fillBuffer(allocation + size*(1+CorruptionChecker.BUFFER_FACTOR), size*CorruptionChecker.BUFFER_FACTOR);
    //Module.print('malloc ' + size + ' ==> ' + [ptr, allocation]);
    return ptr;
  },
  free: function(ptr) {
    if (!ptr) return; // ok to free(NULL), does nothing
    CorruptionChecker.checkAll();
    var size = CorruptionChecker.ptrs[ptr];
    //Module.print('free ' + ptr + ' of size ' + size);
    assert(size);
    var allocation = ptr - size*CorruptionChecker.BUFFER_FACTOR;
    //Module.print('free ' + ptr + ' of size ' + size + ' and allocation ' + allocation);
    delete CorruptionChecker.ptrs[ptr];
    CorruptionChecker.realFree(allocation);
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
    CorruptionChecker.checks++;
  },
  checkPtr: function(ptr) {
    var size = CorruptionChecker.ptrs[ptr];
    assert(size);
    var allocation = ptr - size*CorruptionChecker.BUFFER_FACTOR;
    CorruptionChecker.checkBuffer(allocation, size*CorruptionChecker.BUFFER_FACTOR);
    CorruptionChecker.checkBuffer(allocation + size*(1+CorruptionChecker.BUFFER_FACTOR), size*CorruptionChecker.BUFFER_FACTOR);
  },
  checkAll: function() {
    for (var ptr in CorruptionChecker.ptrs) {
      CorruptionChecker.checkPtr(ptr, false);
    }
  },
};

CorruptionChecker.init();

