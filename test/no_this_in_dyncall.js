mergeInto(LibraryManager.library, {
	$classLike: {
		fnptr: 0,
    call: function(val) {
      {{{ makeDynCall('vp', 'this.fnptr') }}}(val);
    }
	},

  test__deps: ['$classLike'],
  test: function(fnptr, val) {
    classLike.fnptr = fnptr;
    classLike.call(val);
  }
});
