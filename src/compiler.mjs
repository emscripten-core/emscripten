#!/usr/bin/env node
/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// LLVM => JavaScript compiler, main entry point

import * as fs from 'node:fs';
import * as path from 'node:path';
import * as url from 'node:url';

import {Benchmarker, applySettings, assert, loadSettingsFile, printErr, read} from './utility.mjs';

function find(filename) {
  assert(filename);
  const dirname = url.fileURLToPath(new URL('.', import.meta.url));
  const prefixes = [dirname, process.cwd()];
  for (let i = 0; i < prefixes.length; ++i) {
    const combined = path.join(prefixes[i], filename);
    if (fs.existsSync(combined)) {
      return combined;
    }
  }
  return filename;
}

// Load default settings
loadSettingsFile(find('settings.js'));
loadSettingsFile(find('settings_internal.js'));

const argv = process.argv.slice(2);
const symbolsOnlyArg = argv.indexOf('--symbols-only');
if (symbolsOnlyArg != -1) {
  argv.splice(symbolsOnlyArg, 1);
}

// Load settings from JSON passed on the command line
const settingsFile = argv[0];
assert(settingsFile);
const user_settings = JSON.parse(read(settingsFile));
applySettings(user_settings);

export const symbolsOnly = symbolsOnlyArg != -1;

// In case compiler.mjs is run directly (as in gen_sig_info)
// ALL_INCOMING_MODULE_JS_API might not be populated yet.
if (!ALL_INCOMING_MODULE_JS_API.length) {
  ALL_INCOMING_MODULE_JS_API = INCOMING_MODULE_JS_API;
}

EXPORTED_FUNCTIONS = new Set(EXPORTED_FUNCTIONS);
WASM_EXPORTS = new Set(WASM_EXPORTS);
SIDE_MODULE_EXPORTS = new Set(SIDE_MODULE_EXPORTS);
INCOMING_MODULE_JS_API = new Set(INCOMING_MODULE_JS_API);
ALL_INCOMING_MODULE_JS_API = new Set(ALL_INCOMING_MODULE_JS_API);
EXPORTED_RUNTIME_METHODS = new Set(EXPORTED_RUNTIME_METHODS);
WEAK_IMPORTS = new Set(WEAK_IMPORTS);
if (symbolsOnly) {
  INCLUDE_FULL_LIBRARY = 1;
}

// Side modules are pure wasm and have no JS
assert(
  !SIDE_MODULE || (ASYNCIFY && symbolsOnly),
  'JS compiler should only run on side modules if asyncify is used.',
);

// Load compiler code

// We can't use static import statements here because several of these
// file depend on having the settings defined in the global scope (which
// we do dynamically above.
await import('./modules.mjs');
await import('./parseTools.mjs');
if (!STRICT) {
  await import('./parseTools_legacy.mjs');
}
const jsifier = await import('./jsifier.mjs');

// ===============================
// Main
// ===============================

const B = new Benchmarker();

try {
  jsifier.runJSify(symbolsOnly);

  B.print('glue');
} catch (err) {
  if (err.toString().includes('Aborting compilation due to previous errors')) {
    // Compiler failed on user error, don't print the stacktrace in this case.
    printErr(err);
  } else {
    // Compiler failed on internal compiler error!
    printErr('Internal compiler error in src/compiler.mjs!');
    printErr('Please create a bug report at https://github.com/emscripten-core/emscripten/issues/');
    printErr(
      'with a log of the build and the input files used to run. Exception message: "' +
        (err.stack || err),
    );
  }

  // Work around a node.js bug where stdout buffer is not flushed at process exit:
  // Instead of process.exit() directly, wait for stdout flush event.
  // See https://github.com/joyent/node/issues/1669 and https://github.com/emscripten-core/emscripten/issues/2582
  // Workaround is based on https://github.com/RReverser/acorn/commit/50ab143cecc9ed71a2d66f78b4aec3bb2e9844f6
  process.stdout.once('drain', () => process.exit(1));
  // Make sure to print something to force the drain event to occur, in case the
  // stdout buffer was empty.
  console.log(' ');
  // Work around another node bug where sometimes 'drain' is never fired - make
  // another effort to emit the exit status, after a significant delay (if node
  // hasn't fired drain by then, give up)
  setTimeout(() => process.exit(1), 500);
}
