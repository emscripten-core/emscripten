// no merging, this is the entire library. it is literally just enough to run the bootstrap program that prints out C constants for us,
// we obviously need to run without any such constants ourselves...
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
  malloc: function(x) {
    return Runtime.stackAlloc(x);
  },
};

