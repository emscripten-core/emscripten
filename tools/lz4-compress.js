#!/usr/bin/env node
// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

'use strict';

const fs = require('fs');
const path = require('path');

const arguments_ = process.argv.slice(2);
const debug = false;

function print(x) {
  process.stdout.write(x + '\n');
}

function printErr(x) {
  process.stderr.write(x + '\n');
}

function read(filename, binary) {
  filename = path.normalize(filename);
  let ret = fs.readFileSync(filename);
  if (ret && !binary) ret = ret.toString();
  return ret;
}

function readBinary(filename) {
  return read(filename, true);
}

function globalEval(x) {
  eval.call(null, x);
}

function load(f) {
  globalEval(read(f));
}

global.assert = (x, message) => {
  if (!x) throw new Error(message);
};

// Redirect console.log message from MiniLZ4 to stderr since stdout is
// where we return the decompressed data.
console.log = printErr;

const lz4 = arguments_[0];
const input = arguments_[1];
const output = arguments_[2];

load(lz4);

const data = new Uint8Array(readBinary(input)).buffer;
const start = Date.now();
const compressedData = MiniLZ4.compressPackage(data);
fs.writeFileSync(output, Buffer.from(compressedData['data']));
compressedData['data'] = null;
printErr('compressed in ' + (Date.now() - start) + ' ms');
print(JSON.stringify(compressedData));
