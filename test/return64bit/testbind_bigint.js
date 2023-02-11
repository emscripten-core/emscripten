// This code represents a simple native JavaScript binding to a test C function
// that returns a 64 bit long. Notice that the least significant 32 bits are
// returned in the normal return value, but the most significant 32 bits are
// returned via the accessor method Runtime.getTempRet0()

var Module = {
    'noExitRuntime' : true
};

Module['runtest'] = function() {
  var bigint = Module._test_return64(0xaabbccdd11223344n);
  var low = Number(bigint & 0xffffffffn);
  var high = Number(bigint >> 32n);
  console.log("low = " + low);
  console.log("high = " + high);

  var ptr = Module._get_func_ptr();
  bigint = Module.dynCall_jj(ptr, 0xabcdef1912345678n);
  low = Number(bigint & 0xffffffffn);
  high = Number(bigint >> 32n);
  console.log("low = " + low);
  console.log("high = " + high);
};
