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

import assert from 'node:assert';
import {parseArgs} from 'node:util';

import {loadSettingsFile} from '../src/utility.mjs';

const options = {
  'expand-macros': {type: 'boolean'},
  help: {type: 'boolean', short: 'h'},
};
const {values, positionals} = parseArgs({options, allowPositionals: true});

if (values.help) {
  console.log(`\
Run JS preprocessor / macro processor on an input file

Usage: preprocessor.mjs <settings.json> <input-file> [--expand-macros]`);
  process.exit(0);
}

assert(positionals.length == 2, 'Script requires 2 arguments');
const settingsFile = positionals[0];
const inputFile = positionals[1];

loadSettingsFile(settingsFile);

// We can't use static import statements here because several of these
// file depend on having the settings defined in the global scope (which
// we do dynamically above.
const parseTools = await import('../src/parseTools.mjs');
await import('../src/modules.mjs');

let output = parseTools.preprocess(inputFile);
if (values['expand-macros']) {
  output = parseTools.processMacros(output, inputFile);
}
process.stdout.write(output);
