// This code represents a simple native JavaScript binding to a test C function
// that returns a 64 bit long. Notice that the least significant 32 bits are
// returned in the normal return value, but the most significant 32 bits are
// returned via the accessor method Runtime.getTempRet0()

var Module = {
    'noExitRuntime' : true
};

Module['runtest'] = function() {
    var low = _test();
    var high = Runtime.getTempRet0();

    console.log("low = " + low);
    console.log("high = " + high);
};


