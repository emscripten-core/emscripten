mergeInto(LibraryManager.library, {
  test_dyncalls_vijdf: function(funcPtr) {
    // 1. Directly access a function pointer via a static signature (32-bit ABI)
    //    (this is the fastest way to call a function pointer when the signature is statically known in WASM_BIGINT==0 builds)
    dynCall_vijdf(funcPtr, 1, /*lo=*/2, /*hi=*/3, 4, 5); // Available only in WASM_BIGINT != 2 builds

    // 2. Access a function pointer using the convenience/legacy 'dynCall' function (32-bit ABI)
    //    (this form should never be used, it is suboptimal for performance, but provided for legacy compatibility)
    dynCall('vijdf', funcPtr, [2, /*lo=*/3, /*hi=*/4, 5, 6]); // Available only in WASM_BIGINT != 2 builds
  },

  test_dyncalls_iii: function(funcPtr) {
    // 1. Directly access a function pointer via a static signature (32-bit ABI)
    //    (this is the fastest way to call a function pointer when the signature is statically known in WASM_BIGINT==0 builds)
    var ret = dynCall_iii(funcPtr, 1, 2); // Available only in WASM_BIGINT != 2 builds
    console.log('iii returned ' + ret);

    // 2. Access a function pointer using the convenience/legacy 'dynCall' function (32-bit ABI)
    //    (this form should never be used, it is suboptimal for performance, but provided for legacy compatibility)
    var ret = dynCall('iii', funcPtr, [2, 3]); // Available only in WASM_BIGINT != 2 builds
    console.log('iii returned ' + ret);
  }
});
