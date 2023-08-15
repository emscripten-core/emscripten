// Tests for JS library compile time helpers defined in parseTools.js

addToLibrary({
  test_receiveI64ParamAsI53__deps: i53ConversionDeps,
  test_receiveI64ParamAsI53: function({{{ defineI64Param('arg1') }}},
                                      {{{ defineI64Param('arg2') }}}) {
    out('\ntest_receiveI64ParamAsI53:');
    {{{ receiveI64ParamAsI53('arg1', '-1') }}};
    out('arg1: ' + arg1);
    {{{ receiveI64ParamAsI53('arg2', '-2') }}};
    out('arg2: ' + arg2);
    return 0;
  },

  // receiveI64ParamAsDoulbe is a legacy function that is no longer
  // used within emscripten, but we continue to test it in case
  // there are external users.
  test_receiveI64ParamAsDouble: function({{{ defineI64Param('arg1') }}},
                                         {{{ defineI64Param('arg2') }}}) {
    out('\ntest_receiveI64ParamAsDouble:');
    {{{ receiveI64ParamAsDouble('arg1') }}};
    out('arg1: ' + arg1);
    {{{ receiveI64ParamAsDouble('arg2') }}};
    out('arg2: ' + arg2);
    return 0;
  },

  test_makeGetValue__deps: ['$readI53FromI64', '$readI53FromU64'],
  test_makeGetValue: function(ptr) {
    out('\ntest_makeGetValue:');
    let val;

    // i53
    val = {{{ makeGetValue('ptr', '0', 'i53') }}};
    out('i53: ' + val.toString(16))
    assert(val == -0xaabb12345678);

    // u53
    // When interpreted as an unsigned value that bytes stored at in the linear
    // memory represent a number outside of the i53 range so we get some
    // rounding here.
    // readI53FromU64 doesn't currently have any kind of range checkes, even in
    // debug mode.
    val = {{{ makeGetValue('ptr', '0', 'u53') }}};
    out('u53: ' + val.toString(16))
    assert(val == 0xffff5544edcba800);

    // i32
    val = {{{ makeGetValue('ptr', '0', 'i32') }}};
    out('i32: ' + val.toString(16))
    assert(val == -0x12345678);

    // u32
    val = {{{ makeGetValue('ptr', '0', 'u32') }}};
    out('u32: ' + val.toString(16))
    assert(val == 0xedcba988);

    // unsigned i32 (legacy)
    val = {{{ makeGetValue('ptr', '0', 'i32', undefined, /*unsigned=*/true) }}};
    out('u32 legacy: ' + val.toString(16))
    assert(val == 0xedcba988);

    // i16
    val = {{{ makeGetValue('ptr', '0', 'i16') }}};
    out('i16: ' + val.toString(16))
    assert(val == -0x5678);

    // u16
    val = {{{ makeGetValue('ptr', '0', 'u16') }}};
    out('u16: ' + val.toString(16))
    assert(val == 43400);

    // unsigned i16 (legacy)
    val = {{{ makeGetValue('ptr', '0', 'i16', undefined, /*unsigned=*/true) }}};
    out('u16 legacy: ' + val.toString(16))
    assert(val == 43400);

    // i8
    val = {{{ makeGetValue('ptr', '0', 'i8') }}};
    out('i8: ' + val.toString(16))
    assert(val == -0x78);

    // u8
    val = {{{ makeGetValue('ptr', '0', 'u8') }}};
    out('u8: ' + val.toString(16))
    assert(val == 0x88);

    // unsigned i8 (legacy)
    val = {{{ makeGetValue('ptr', '0', 'i8', undefined, /*unsigned=*/true) }}};
    out('u8 legacy: ' + val.toString(16))
    assert(val == 0x88);

    // pointer
    val = {{{ makeGetValue('ptr', '0', 'void*') }}};
    out('ptr: ' + val.toString(16))
    assert(val == 0xedcba988);
    val = {{{ makeGetValue('ptr', '0', 'i32*') }}};
    out('ptr: ' + val.toString(16))
    assert(val == 0xedcba988);
  },

  test_makeSetValue__deps: ['$writeI53ToI64'],
  test_makeSetValue: function(ptr) {
    out('\ntest_makeSetValue:');
    _clearI64(ptr);
    {{{ makeSetValue('ptr', '0', 0x12345678AB, 'i64') }}};
    _printI64(ptr);

    // This value doesn't fit into i64.  The current behaviour truncate (i.e.
    // ignore the upper bits), in the same way that `BigInt64Array[X] = Y` does.
    // (see splitI16 in parseTools.js)
    _clearI64(ptr);
    {{{ makeSetValue('ptr', '0', 0x1122334455667788AA, 'i64') }}};
    _printI64(ptr);

    _clearI64(ptr);
    {{{ makeSetValue('ptr', '0', -0x1122334455667788AA, 'i64') }}};
    _printI64(ptr);

    _clearI64(ptr);
    {{{ makeSetValue('ptr', '0', 0x12345678AB, 'i53') }}};
    _printI64(ptr);

    _clearI64(ptr);
    {{{ makeSetValue('ptr', '0', -1, 'i53') }}};
    _printI64(ptr);

    _clearI64(ptr);
    {{{ makeSetValue('ptr', '0', 0xff, 'i32') }}};
    _printI64(ptr);

    _clearI64(ptr);
    {{{ makeSetValue('ptr', '0', 0x12345678ab, 'i32') }}};
    _printI64(ptr);
  },
});
