#!/usr/bin/env node
// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

import * as fs from 'node:fs';
import * as path from 'node:path';

function print(x) {
  process.stdout.write(x + '\n');
}

function printErr(x) {
  process.stderr.write(x + '\n');
}

globalThis.assert = (x, message) => {
  if (!x) throw new Error(message);
};

// Redirect console.log message from MiniLZ4 to stderr since stdout is
// where we return the decompressed data.
console.log = printErr;

const MiniLZ4 = await import('../third_party/mini-lz4.js');

function readBinary(filename) {
  filename = path.normalize(filename);
  return fs.readFileSync(filename);
}

const arguments_ = process.argv.slice(2);
const input = arguments_[0];
const output = arguments_[1];

const data = new Uint8Array(readBinary(input)).buffer;
const start = Date.now();
const compressedData = MiniLZ4.compressPackage(data);
fs.writeFileSync(output, Buffer.from(compressedData['data']));
compressedData['data'] = null;
printErr('compressed in ' + (Date.now() - start) + ' ms');
print(JSON.stringify(compressedData));
