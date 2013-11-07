
// See settings.js, CORRUPTION_CHECK

var CorruptionChecker = {
  BUFFER_FACTOR: Math.round({{{ CORRUPTION_CHECK }}}),

  ptrs: {},
  checks: 0,
  checkFrequency: 1,

  init: function() {
    this.realMalloc = _malloc;
    _malloc = Module['_malloc'] = this.malloc;

    this.realFree = _free;
    _free = Module['_free'] = this.free;

    if (typeof _realloc != 'undefined') {
      this.realRealloc = _realloc;
      _realloc = Module['_realloc'] = this.realloc;
    }

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
    //Module.printErr('malloc ' + size + ' ==> ' + [ptr, allocation]);
    return ptr;
  },
  free: function(ptr) {
    if (!ptr) return; // ok to free(NULL), does nothing
    CorruptionChecker.checkAll();
    var size = CorruptionChecker.ptrs[ptr];
    //Module.printErr('free ' + ptr + ' of size ' + size);
    assert(size, ptr);
    var allocation = ptr - size*CorruptionChecker.BUFFER_FACTOR;
    //Module.printErr('free ' + ptr + ' of size ' + size + ' and allocation ' + allocation);
    delete CorruptionChecker.ptrs[ptr];
    CorruptionChecker.realFree(allocation);
  },
  realloc: function(ptr, newSize) {
    //Module.printErr('realloc ' + ptr + ' to size ' + newSize);
    if (newSize <= 0) newSize = 1; // like in malloc
    if (!ptr) return CorruptionChecker.malloc(newSize); // realloc(NULL, size) forwards to malloc according to the spec
    var size = CorruptionChecker.ptrs[ptr];
    assert(size);
    var allocation = ptr - size*CorruptionChecker.BUFFER_FACTOR;
    var newPtr = CorruptionChecker.malloc(newSize);
    //Module.printErr('realloc ' + ptr + ' to size ' + newSize + ' is now ' + newPtr);
    var newAllocation = newPtr + newSize*CorruptionChecker.BUFFER_FACTOR;
    HEAPU8.set(HEAPU8.subarray(ptr, ptr + Math.min(size, newSize)), newPtr);
    CorruptionChecker.free(ptr);
    return newPtr;
  },
  canary: function(x) {
    return (x&127) + 10;
  },
  fillBuffer: function(buffer, size) {
    for (var x = buffer; x < buffer + size; x++) {
      {{{ makeSetValue('x', 0, 'CorruptionChecker.canary(x)', 'i8', null, null, null, 1) }}};
    }
  },
  checkBuffer: function(buffer, size) {
    for (var x = buffer; x < buffer + size; x++) {
      if (({{{ makeGetValue('x', 0, 'i8', null, null, null, null, 1) }}}&255) != CorruptionChecker.canary(x)) {
        assert(0, 'Heap corruption detected!' + [x, buffer, size, {{{ makeGetValue('x', 0, 'i8') }}}&255, CorruptionChecker.canary(x)]);
      }
    }
  },
  checkPtr: function(ptr) {
    var size = CorruptionChecker.ptrs[ptr];
    assert(size);
    var allocation = ptr - size*CorruptionChecker.BUFFER_FACTOR;
    CorruptionChecker.checkBuffer(allocation, size*CorruptionChecker.BUFFER_FACTOR);
    CorruptionChecker.checkBuffer(allocation + size*(1+CorruptionChecker.BUFFER_FACTOR), size*CorruptionChecker.BUFFER_FACTOR);
  },
  checkAll: function(force) {
    CorruptionChecker.checks++;
    if (!force && CorruptionChecker.checks % CorruptionChecker.checkFrequency != 0) return;
    //Module.printErr('checking for corruption ' + (CorruptionChecker.checks/CorruptionChecker.checkFrequency));
    for (var ptr in CorruptionChecker.ptrs) {
      CorruptionChecker.checkPtr(ptr, false);
    }
  },
};

CorruptionChecker.init();

