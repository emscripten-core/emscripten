addToLibrary({
  $classLike: {
    fnptr: 0,
    call: function(val) {
      {{{ makeDynCall('vp', 'this.fnptr') }}}(val);
    }
  },

  test__deps: ['$classLike'],
  test: function(fnptr, val) {
    // Bug: Function pointers are currently treated as 64-bit, but should be 32-bit integers.
    classLike.fnptr = fnptr;
    classLike.call(val);
  }
});
