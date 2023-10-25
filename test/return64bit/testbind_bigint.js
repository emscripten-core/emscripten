// This code represents a simple native JavaScript binding to a test C function
// that returns a 64 bit long. Notice that the least significant 32 bits are
// returned in the normal return value, but the most significant 32 bits are
// returned via the accessor method getTempRet0()

Module['runtest'] = function() {
  // Use eval to create BigInt, as no support for Xn notation yet in JS
  // optimizer.
  var bigint = _test_return64(eval('0xaabbccdd11223344n'));
  var low = Number(bigint & 0xffffffffn);
  var high = Number(bigint >> 32n);
  console.log("low = " + low);
  console.log("high = " + high);

  var ptr = _get_func_ptr();
  bigint = dynCall('jj', ptr, [eval('0xabcdef1912345678n')]);
  low = Number(bigint & 0xffffffffn);
  high = Number(bigint >> 32n);
  console.log("low = " + low);
  console.log("high = " + high);
};
