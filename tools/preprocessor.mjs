#!/usr/bin/env node
// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
//
// Preprocessor tool.  This is a wrapper for the 'preprocess' function which
// allows it to be called as a standalone tool.
//
// Parameters:
//    setting file.  Can specify 'settings.js' here, alternatively create a temp
//                   file with modified settings and supply the filename here.
//    input file     This is the file that will be processed by the preprocessor

'use strict';

import * as fs from 'fs';
import * as path from 'path';
import * as vm from 'vm';
import assert from 'assert';
import * as url from 'url';

const args = process.argv.slice(2);
const debug = false;

// Anything needed by the script that we load below must be added to the
// global object.  These, for example, are all needed by parseTools.js.
global.vm = vm;
global.assert = assert;
global.print = (x) => {
  process.stdout.write(x + '\n');
};
global.printErr = (x) => {
  process.stderr.write(x + '\n');
};

function find(filename) {
  const dirname = url.fileURLToPath(new URL('.', import.meta.url));
  const prefixes = [process.cwd(), path.join(dirname, '..', 'src')];
  for (let i = 0; i < prefixes.length; ++i) {
    const combined = path.join(prefixes[i], filename);
    if (fs.existsSync(combined)) {
      return combined;
    }
  }
  return filename;
}

global.read = (filename) => {
  const absolute = find(filename);
  return fs.readFileSync(absolute).toString();
};

global.load = (f) => {
  vm.runInThisContext(read(f), {filename: find(f)});
};

assert(args.length >= 2);
const settingsFile = args[0];
const inputFile = args[1];
const expandMacros = args.includes('--expandMacros');

load(settingsFile);
load('utility.js');
load('modules.js');
load('parseTools.js');

let output = preprocess(inputFile);
if (expandMacros) {
  output = processMacros(output, inputFile);
}
process.stdout.write(output);
