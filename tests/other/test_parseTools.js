// Tests for JS library compile time helpers defined in parseTools.js

mergeInto(LibraryManager.library, {
  test_makeGetValue: function(ptr) {
    let val;

    // i32
    val = {{{ makeGetValue('ptr', '0', 'i32') }}};
    out('i32: ' + val.toString(16))
    assert(val == -0x12345678);

    // unsigned i32
    val = {{{ makeGetValue('ptr', '0', 'i32', undefined, /*unsigned=*/true) }}};
    out('u32: ' + val.toString(16))
    assert(val == 0xedcba988);

    // i16
    val = {{{ makeGetValue('ptr', '0', 'i16') }}};
    out('i16: ' + val.toString(16))
    assert(val == -0x5678);

    // unsigned i16
    val = {{{ makeGetValue('ptr', '0', 'i16', undefined, /*unsigned=*/true) }}};
    out('u16: ' + val.toString(16))
    assert(val == 43400);

    // i8
    val = {{{ makeGetValue('ptr', '0', 'i8') }}};
    out('i8: ' + val.toString(16))
    assert(val == -0x78);

    // unsigned i8
    val = {{{ makeGetValue('ptr', '0', 'i8', undefined, /*unsigned=*/true) }}};
    out('u8: ' + val.toString(16))
    assert(val == 0x88);

    // pointer
    val = {{{ makeGetValue('ptr', '0', 'void*') }}};
    out('ptr: ' + val.toString(16))
    assert(val == -0x12345678);
    val = {{{ makeGetValue('ptr', '0', 'i32*') }}};
    out('ptr: ' + val.toString(16))
    assert(val == -0x12345678);
  }
});
