function _png_create_write_struct_2($user_png_ver, $error_ptr, $error_fn, $warn_fn, $mem_ptr, $malloc_fn, $free_fn) {
  var $png_ptr$s2;
  var __label__;
  __label__ = 2;
  var setjmpTable = {
    "2": (function(value) {
      __label__ = 5;
      $call1 = value;
    }),
    dummy: 0
  };
  while (1) try {
    switch (__label__) {
     case 2:
      var $png_ptr;
      var $call = _png_create_struct(1);
      $png_ptr = $call;
      var $call1 = (HEAP32[$png_ptr >> 2] = __label__, 0);
      __label__ = 5;
      break;
     case 5:
      var $2 = $png_ptr;
      if (($call1 | 0) == 0) {
        __label__ = 4;
        break;
      } else {
        __label__ = 3;
        break;
      }
     case 3:
      var $4 = HEAP32[($png_ptr >> 2) + (148 >> 2)];
      _png_free($2, $4);
      HEAP32[($png_ptr >> 2) + (148 >> 2)] = 0;
      _png_destroy_struct($png_ptr);
      var $retval_0 = 0;
      __label__ = 4;
      break;
     case 4:
      var $retval_0;
      return $retval_0;
     default:
      assert(0, "bad label: " + __label__);
    }
  } catch (e) {
    if (!e.longjmp) throw e;
    setjmpTable[e.label](e.value);
  }
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["_png_create_write_struct_2"]
