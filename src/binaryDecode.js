// Prevent Closure from minifying the binaryDecode() function, or otherwise
// Closure may analyze through the WASM_BINARY_DATA placeholder string into this
// function, leading into incorrect results.
/** @noinline */
function binaryDecode(bin, expectedLength) {
#if ASSERTIONS
  assert(bin.length === expectedLength, `The embedded Wasm binary is ${bin.length} characters long, but should be ${expectedLength}: this file was not read as UTF-8. See the documentation on SINGLE_FILE_BINARY_ENCODE for ways to address this issue`);
#endif
  for (var i = 0, l = bin.length, o = new Uint8Array(l), c; i < l; ++i) {
    c = bin.charCodeAt(i);
    o[i] = ~c >> 8 & c; // Recover the null byte in a manner that is compatible with https://crbug.com/453961758
  }
  return o;
}
