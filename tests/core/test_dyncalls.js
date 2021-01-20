mergeInto(LibraryManager.library, {
  test_dyncalls_vijdf__deps: [
#if WASM_DYNCALLS
    '$getDynCaller', '$bindDynCall',
#endif
    '$wbind', '$wbindArray'],
  test_dyncalls_vijdf: function(funcPtr) {
#if WASM_DYNCALLS
    // 1. Directly access a function pointer via a static signature (32-bit ABI)
    //    (this is the fastest way to call a function pointer when the signature is statically known in WASM_BIGINT==0 builds)
    dynCall_vijdf(funcPtr, 1, /*lo=*/2, /*hi=*/3, 4, 5); // Available only in WASM_BIGINT != 2 builds

    // 2. Access a function pointer using the convenience/legacy 'dynCall' function (32-bit ABI)
    //    (this form should never be used, it is suboptimal for performance, but provided for legacy compatibility)
    dynCall('vijdf', funcPtr, [2, /*lo=*/3, /*hi=*/4, 5, 6]); // Available only in WASM_BIGINT != 2 builds

    // 3. Obtain a dynamic function caller to a given signature and call it with .apply() (32-bit ABI)
    //    (this form should be used when dealing with a dynamic input signature problem with varying length of function args, and funcPtr + args are fused together in one array)
    getDynCaller('vijdf').apply(null, [funcPtr, 3, /*lo=*/4, /*hi=*/5, 6, 7]); // Available only in WASM_BIGINT != 2 builds

    // 4. Obtain a function wrapper to given function pointer and call it by submitting args in an array (32-bit ABI)
    //    (this form should be used when dealing with a dynamic input signature problem with varying length of function args, but funcPtr and args params are dealt with separately)
    bindDynCall('vijdf', funcPtr)([4, /*lo=*/5, /*hi=*/6, 7, 8]); // Available only in WASM_BIGINT != 2 builds
#else
    // Appease test runner and output the same text if not building with WASM_DYNCALLS enabled.
    wbind(funcPtr)(1, BigInt(2) | (BigInt(3) << BigInt(32)), 4, 5);
    wbind(funcPtr)(2, BigInt(3) | (BigInt(4) << BigInt(32)), 5, 6);
    wbind(funcPtr)(3, BigInt(4) | (BigInt(5) << BigInt(32)), 6, 7);
    wbind(funcPtr)(4, BigInt(5) | (BigInt(6) << BigInt(32)), 7, 8);
#endif

#if WASM_BIGINT
    // 5. Directly access a function pointer via a static signature (64-bit ABI)
    //    (this is the fastest way to call a function pointer when the signature is statically known in WASM_BIGINT>0 builds)
    wbind(funcPtr)(5, BigInt(6) | (BigInt(7) << BigInt(32)), 8, 9); // Available in all builds, but in WASM_BIGINT==0 builds cannot be used to call int64 signatures

    // 6. Obtain an array form access to the specified signature. (64-bit ABI)
    //    (this form should be used when dealing with a dynamic input signature problem with varying length of function args)
    wbindArray(funcPtr)([6, BigInt(7) | (BigInt(8) << BigInt(32)), 9, 10]); // Available in all builds, but in WASM_BIGINT==0 builds cannot be used to call int64 signatures
#else
    // Appease test runner and output the same text if not building with WASM_BIGINT enabled.
    dynCall_vijdf(funcPtr, 5, /*lo=*/6, /*hi=*/7, 8, 9);
    dynCall_vijdf(funcPtr, 6, /*lo=*/7, /*hi=*/8, 9, 10);
#endif
  },

  test_dyncalls_iii: function(funcPtr) {
#if WASM_DYNCALLS
    // 1. Directly access a function pointer via a static signature (32-bit ABI)
    //    (this is the fastest way to call a function pointer when the signature is statically known in WASM_BIGINT==0 builds)
    var ret = dynCall_iii(funcPtr, 1, 2); // Available only in WASM_BIGINT != 2 builds
    console.log('iii returned ' + ret);

    // 2. Access a function pointer using the convenience/legacy 'dynCall' function (32-bit ABI)
    //    (this form should never be used, it is suboptimal for performance, but provided for legacy compatibility)
    var ret = dynCall('iii', funcPtr, [2, 3]); // Available only in WASM_BIGINT != 2 builds
    console.log('iii returned ' + ret);

    // 3. Obtain a dynamic function caller to a given signature and call it with .apply() (32-bit ABI)
    //    (this form should be used when dealing with a dynamic input signature problem with varying length of function args, and funcPtr + args are fused together in one array)
    var ret = getDynCaller('iii').apply(null, [funcPtr, 3, 4]); // Available only in WASM_BIGINT != 2 builds
    console.log('iii returned ' + ret);

    // 4. Obtain a function wrapper to given function pointer and call it by submitting args in an array (32-bit ABI)
    //    (this form should be used when dealing with a dynamic input signature problem with varying length of function args, but funcPtr and args params are dealt with separately)
    var ret = bindDynCall('iii', funcPtr)([4, 5]); // Available only in WASM_BIGINT != 2 builds
    console.log('iii returned ' + ret);
#else
    // Appease test runner and output the same text if not building with WASM_DYNCALLS enabled.
    console.log('iii returned ' + wbind(funcPtr)(1, 2));
    console.log('iii returned ' + wbind(funcPtr)(2, 3));
    console.log('iii returned ' + wbind(funcPtr)(3, 4));
    console.log('iii returned ' + wbind(funcPtr)(4, 5));
#endif

    // 5. Directly access a function pointer via a static signature (64-bit ABI)
    //    (this is the fastest way to call a function pointer when the signature is statically known in WASM_BIGINT>0 builds)
    var ret = wbind(funcPtr)(5, 6); // Available in all builds, but in WASM_BIGINT==0 builds cannot be used to call int64 signatures
    console.log('iii returned ' + ret);

    // 6. Obtain an array form access to the specified signature. (64-bit ABI)
    //    (this form should be used when dealing with a dynamic input signature problem with varying length of function args)
    var ret = wbindArray(funcPtr)([6, 7]); // Available in all builds, but in WASM_BIGINT==0 builds cannot be used to call int64 signatures
    console.log('iii returned ' + ret);
  }
});
