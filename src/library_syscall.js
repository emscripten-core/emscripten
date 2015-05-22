mergeInto(LibraryManager.library, {
  __syscall__deps: ['sbrk'],
  __syscall: function(which, varargs) {
    Module.print('syscall! ' + which);
    function get() { // gets a 32-bit vararg
      var ret = {{{ makeGetValue('varargs', '0', 'i32') }}};
      varargs += 4;
      return ret;
    }
    switch (which) {
      case 45: { // brk
        return _sbrk(get());
      }
      default: abort('bad syscall ' + which);
    }
  }
});

