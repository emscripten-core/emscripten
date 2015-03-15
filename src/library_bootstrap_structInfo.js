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
  sbrk: function(bytes) {
    // Implement a Linux-like 'memory area' for our 'process'.
    // Changes the size of the memory area by |bytes|; returns the
    // address of the previous top ('break') of the memory area
    // We control the "dynamic" memory - DYNAMIC_BASE to DYNAMICTOP
    var self = _sbrk;
    if (!self.called) {
      DYNAMICTOP = alignMemoryPage(DYNAMICTOP); // make sure we start out aligned
      self.called = true;
      assert(Runtime.dynamicAlloc);
      self.alloc = Runtime.dynamicAlloc;
      Runtime.dynamicAlloc = function() { abort('cannot dynamically allocate, sbrk now has control') };
    }
    var ret = DYNAMICTOP;
    if (bytes != 0) self.alloc(bytes);
    return ret;  // Previous break location.
  },
  printf__deps: ['_formatString'],
  printf: function(format, varargs) {
    // int printf(const char *restrict format, ...);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/printf.html
    // extra effort to support printf, even without a filesystem. very partial, very hackish
    var result = __formatString(format, varargs);
    var string = intArrayToString(result);
    if (string[string.length-1] === '\n') string = string.substr(0, string.length-1); // remove a final \n, as Module.print will do that
    Module.print(string);
    return result.length;
  },
  puts: function(s) {
    // extra effort to support puts, even without a filesystem. very partial, very hackish
    var result = Pointer_stringify(s);
    var string = result.substr(0);
    if (string[string.length-1] === '\n') string = string.substr(0, string.length-1); // remove a final \n, as Module.print will do that
    Module.print(string);
    return result.length;
  },
};

