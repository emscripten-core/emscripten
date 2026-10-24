// Prevent Closure from minifying the binaryDecode() function, or otherwise
// Closure may analyze through the WASM_BINARY_DATA placeholder string into this
// function, leading into incorrect results.
/** @noinline */
#if ASSERTIONS
function binaryDecode(bin, expectedLength) {
  // Each byte of the binary is one character of the string, so if the lengths
  // differ, the file was not read as UTF-8 (for example a page without
  // <meta charset="utf-8"> loading it from file://), and the bytes are wrong.
  // Say so here, rather than let it fail as a confusing wasm CompileError.
  assert(bin.length === expectedLength, `The embedded Wasm binary is ${bin.length} characters long, but should be ${expectedLength}: this file was not read as UTF-8 (SINGLE_FILE_BINARY_ENCODE). Serve it with charset=utf-8, add <meta charset="utf-8"> to the page, or build with -sSINGLE_FILE_BINARY_ENCODE=0`);
#else
function binaryDecode(bin) {
#endif
  for (var i = 0, l = bin.length, o = new Uint8Array(l), c; i < l; ++i) {
    c = bin.charCodeAt(i);
    o[i] = ~c >> 8 & c; // Recover the null byte in a manner that is compatible with https://crbug.com/453961758
  }
  return o;
}
