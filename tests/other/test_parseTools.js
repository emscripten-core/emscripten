// Tests for JS library compile time helpers defined in parseTools.js

mergeInto(LibraryManager.library, {
  test_makeGetValue: function(ptr) {
    let val;

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
  }
});
