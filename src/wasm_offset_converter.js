function WasmOffsetConverter(wasmBytes) {
  // This class parses a WASM binary file, and constructs a mapping from
  // function indices to the start of their code in the binary file.
  //
  // The main purpose of this module is to enable the conversion of function
  // index and offset from start of function to an offset into the WASM binary.
  // This is needed to look up the WASM source map as well as generate
  // consistent program counter representations given v8's non-standard
  // WASM stack trace format.
  //
  // v8 bug: https://crbug.com/v8/9172
  // We may be able to remove this code once the fix makes its way into all
  // commonly used versions of node.

  // current byte offset into the WASM binary, as we parse it
  // the first section starts at offset 8
  var offset = 8;

  // the index of the next function we see in the binary
  var funcidx = 0;

  // map from function index to byte offset in WASM binary
  this.map = {};

  function unsignedLEB128() {
    // consumes an unsigned LEB128 integer starting at `offset`.
    // changes `offset` to immediately after the integer
    var result = 0;
    var shift = 0;
    do {
      var byte = wasmBytes[offset++];
      result += (byte & 0x7F) << shift;
      shift += 7;
    } while (byte & 0x80);
    return result;
  }

  function skipLimits() {
    // skip `offset` over a WASM limits object
    switch (wasmBytes[offset++]) {
      case 1: unsignedLEB128(); // has both initial and maximum, fall through
      case 0: unsignedLEB128(); // just initial
    }
  }

  while (offset < wasmBytes.length) {
    var start = offset;
    var type = wasmBytes[offset++];
    var end = unsignedLEB128() + offset;
    switch (type) {
      case 2: // import section
        // we need to find all function imports and increment funcidx for each one
        // since functions defined in the module are numbered after all imports
        var count = unsignedLEB128();

        while (count-- > 0) {
          // skip module
          offset = unsignedLEB128() + offset;
          // skip name
          offset = unsignedLEB128() + offset;

          switch (wasmBytes[offset++]) {
            case 0: // function import
              ++funcidx;
              unsignedLEB128(); // skip function type
              break;
            case 1: // table import
              ++offset;
              skipLimits();
              break;
            case 2: // memory import
              skipLimits();
              break;
            case 3: // global import
              offset += 2; // skip type id byte and mutability byte
              break;
          }
        }
        break;
      case 10: // code section
        var count = unsignedLEB128();
        while (count-- > 0) {
          var size = unsignedLEB128();
          this.map[funcidx++] = offset;
          offset += size;
        }
        return;
    }
    offset = end;
  }
}

WasmOffsetConverter.prototype.convert = function (funcidx, offset) {
  return this.map[funcidx] + offset;
}
