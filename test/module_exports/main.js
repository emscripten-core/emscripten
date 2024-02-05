// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

var Module = require("./test.js");

console.log("\nTesting main.js");

var length = 20;
var ptr = Module._malloc(length); // Get buffer from emscripten.
var buffer= new Uint8Array(Module.HEAPU8.buffer, ptr, length); // Get a bytes view on the newly allocated buffer.

// Populate the buffer in JavaScript land.
console.log("buffer length = " + length + "\n");
for (var i = 0; i < length; i++) {
    buffer[i] = i + 20; // Add 20 just for a bit of interest.
    console.log("setting buffer[" + i + "] = " + buffer[i]);
}

// Export bufferTest function.
var bufferTest = Module.cwrap('bufferTest', 'number', ['number', 'number']);
console.log("\ncalling bufferTest\n");

bufferTest(ptr, length); // Call our exported C function to prove the buffer was passed correctly.

console.log("\nbufferTest finished\n");

// free the heap buffer
Module._free(ptr);
