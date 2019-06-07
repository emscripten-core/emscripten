function WasmOffsetConverter(wasmBytes) {
  var offset = 8;
  var count;
  var idx = 0;

  this.map = {};

  function unsignedLEB128() {
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
        count = unsignedLEB128();

        while (count --> 0) {
          // skip module
          offset = unsignedLEB128() + offset;
          // skip name
          offset = unsignedLEB128() + offset;

          switch (wasmBytes[offset++]) {
            case 0: // function import
              ++idx;
              unsignedLEB128(); // skip funcidx
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
        count = unsignedLEB128();
        while (count --> 0) {
          var size = unsignedLEB128();
          this.map[idx++] = offset;
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
