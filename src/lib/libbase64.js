/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  // Decodes a _known valid_ base64 string (without validation) and returns it as a new Uint8Array.
  // Benchmarked to be around 5x faster compared to a simple
  // "Uint8Array.from(atob(b64), c => c.charCodeAt(0))" (TODO: perhaps use this form in -Oz builds?)
  $base64Decode__postset: `
  // Precreate a reverse lookup table from chars
  // "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" back to
  // bytes to make decoding fast.
  for (var base64ReverseLookup = new Uint8Array(123/*'z'+1*/), i = 25; i >= 0; --i) {
    base64ReverseLookup[48+i] = 52+i; // '0-9'
    base64ReverseLookup[65+i] = i; // 'A-Z'
    base64ReverseLookup[97+i] = 26+i; // 'a-z'
  }
  base64ReverseLookup[43] = 62; // '+'
  base64ReverseLookup[47] = 63; // '/'
`,
  $base64Decode__docs: '/** @noinline */',
  $base64Decode: (b64) => {
#if ENVIRONMENT_MAY_BE_NODE
    if (ENVIRONMENT_IS_NODE) {
      var buf = Buffer.from(b64, 'base64');
      return new Uint8Array(buf.buffer, buf.byteOffset, buf.length);
    }
#endif

#if ASSERTIONS
    assert(b64.length % 4 == 0);
#endif
    var b1, b2, i = 0, j = 0, bLength = b64.length;
    var output = new Uint8Array((bLength*3>>2) - (b64[bLength-2] == '=') - (b64[bLength-1] == '='));
    for (; i < bLength; i += 4, j += 3) {
      b1 = base64ReverseLookup[b64.charCodeAt(i+1)];
      b2 = base64ReverseLookup[b64.charCodeAt(i+2)];
      output[j] = base64ReverseLookup[b64.charCodeAt(i)] << 2 | b1 >> 4;
      output[j+1] = b1 << 4 | b2 >> 2;
      output[j+2] = b2 << 6 | base64ReverseLookup[b64.charCodeAt(i+3)];
    }
    return output;
  },

  // Prefix of data URIs emitted by SINGLE_FILE and related options.
  // Double quotes here needed, othersise jsifier will not include any in the
  // output.
  $dataURIPrefix: "'data:application/octet-stream;base64,'",

  /**
   * Indicates whether filename is a base64 data URI.
   */
  $isDataURI: (filename) => filename.startsWith(dataURIPrefix),

  // If filename is a base64 data URI, parses and returns data (Buffer on node,
  // Uint8Array otherwise). If filename is not a base64 data URI, returns
  // undefined.
  $tryParseAsDataURI__deps: ['$base64Decode', '$isDataURI', '$dataURIPrefix'],
  $tryParseAsDataURI: (filename) => {
    if (isDataURI(filename)) {
      return base64Decode(filename.slice(dataURIPrefix.length));
    }
  },
});
