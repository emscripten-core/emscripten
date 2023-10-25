// This code represents a simple native JavaScript binding to a test C function
// that returns a 64 bit long. Notice that the least significant 32 bits are
// returned in the normal return value, but the most significant 32 bits are
// returned via the accessor method getTempRet0()

Module['runtest'] = function() {
  var low = _test_return64(0x11223344, 0xaabbccdd);
  var high = getTempRet0();
  console.log("low = " + low);
  console.log("high = " + high);

  var ptr = _get_func_ptr();
  low = dynCall_jj(ptr, 0x12345678, 0xabcdef19);
  high = getTempRet0();
  console.log("low = " + low);
  console.log("high = " + high);
};
