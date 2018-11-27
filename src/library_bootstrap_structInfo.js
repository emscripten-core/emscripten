// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
//
// no merging, this is the entire library. it is literally just enough to run
// the bootstrap program that prints out C constants for us, we obviously need
// to run without any such constants ourselves...

assert(!LibraryManager.library);
LibraryManager.library = {
  sysconf: function(name) {
    assert(name == 30); 
    return PAGE_SIZE;
  },
  time: function(ptr) {
    var ret = (Date.now()/1000)|0;
    if (ptr) {
      {{{ makeSetValue('ptr', 0, 'ret', 'i32') }}};
    }
    return ret;
  },
  sbrk: function(bytes) {
    // Implement a Linux-like 'memory area' for our 'process'.
    // Changes the size of the memory area by |bytes|; returns the
    // address of the previous top ('break') of the memory area
    // We control the "dynamic" memory - DYNAMIC_BASE to DYNAMICTOP
    var self = _sbrk;
    if (!self.called) {
      HEAP32[DYNAMICTOP_PTR>>2] = alignUp(HEAP32[DYNAMICTOP_PTR>>2], 16777216); // make sure we start out aligned
      self.called = true;
      assert(dynamicAlloc);
      self.alloc = dynamicAlloc;
      dynamicAlloc = function() { abort('cannot dynamically allocate, sbrk now has control') };
    }
    var ret = HEAP32[DYNAMICTOP_PTR>>2];
    if (bytes != 0) self.alloc(bytes);
    return ret;  // Previous break location.
  },
  malloc: function(x) {
    return dynamicAlloc(x);
  },
};

