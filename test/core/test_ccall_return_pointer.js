// This test code fills an array using 'begin < end' comparisons into an array range.
addToLibrary({
  fill_array_up_to__deps: ['$ccall'],
  fill_array_up_to__sig: 'vpi',
  fill_array_up_to: (end_ptr, val) => {
    for(var i = 0;;++i) {
      var ptr = ccall('get_pointer_to_array_index', 'pointer', ['number'], [i]);
      if (ptr >= end_ptr) break;
      {{{ makeSetValue('ptr', '0', 'val', 'i32') }}}
    }
  }
});
