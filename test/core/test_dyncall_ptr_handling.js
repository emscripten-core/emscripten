addToLibrary({
  test_ptr_handling: function(fnptr, arg) {
    // Both 'p' as return type and 'p' as argument type should be handled.
    var ptr = {{{ makeDynCall('pp', 'fnptr') }}}(arg);
    console.log('ptr type: ' + typeof ptr);
    console.log('ptr value: ' + UTF8ToString(ptr));
  }
});
