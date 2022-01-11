var fs = require('fs');

function readFile(filename) {
  return fs.readFileSync(filename).toString();
}

function include(filename) {
  eval.call(null, readFile(`${__dirname}/${filename}`));
}

include('utility.js');
include('settings.js');
include('settings_internal.js');
include('parseTools.js');

// Read all input JS library files into LibraryManager.
var LibraryManager = { library: {} };
for(let f of process.argv.slice(2)) {
  eval(processMacros(preprocess(readFile(f), f)));
}

// Grab all JS -> JS deps and JS -> C deps from the input files
const DEPS_SUFFIX = '__deps';
const WASM_DEPS_SUFFIX = '__wasm_deps';

let jsToJsDeps = {};
let jsToWasmDeps = {};

for(let s in LibraryManager.library) {
  if (s.endsWith(DEPS_SUFFIX)) jsToJsDeps[s.substr(0, s.length - DEPS_SUFFIX.length)] = LibraryManager.library[s];
  else if (s.endsWith(WASM_DEPS_SUFFIX)) jsToWasmDeps[s.substr(0, s.length - WASM_DEPS_SUFFIX.length)] = LibraryManager.library[s];
}

// Key jsToJsDeps backwards: given a JS function as key, lists all functions that depend on this function.
let jsToJsBackDeps = {};

for(let depender in jsToJsDeps) {
  for(let dependee of jsToJsDeps[depender]) {
    if (!jsToJsBackDeps[dependee]) jsToJsBackDeps[dependee] = [];
    if (!jsToJsBackDeps[dependee].includes(depender)) jsToJsBackDeps[dependee].push(depender);
  }
}

// Appends those elements from array src to array dst that did not yet exist in dst.
// Operates in-place, returns true if any modifications to array dst were done.
function appendToArrayIfNotExists(dst, src) {
  let modified = false;
  for(let element of src) {
    if (!dst.includes(element)) {
      dst.push(element);
      modified = true;
    }
  }
  return modified;
}

// Transitively propagate all jsToWasm deps backwards, i.e. if jsFunc1 depends on jsFunc2,
// and jsFunc2 depends on wasmFunc1, also record jsFunc1 to depend on wasmFunc1.
// Perform the propagation to a new dictionary to not disturb iteration over jsToWasmDeps.
let transitiveJsToWasmDeps = {};
for(let dep in jsToWasmDeps) {
  const wasmDeps = jsToWasmDeps[dep];
  let stack = [dep];
  while(stack.length > 0) {
    let f = stack.pop();
    if (!transitiveJsToWasmDeps[f]) transitiveJsToWasmDeps[f] = [];
    if (appendToArrayIfNotExists(transitiveJsToWasmDeps[f], wasmDeps)) {
      // Keep going if this append produced some modifications (this check makes sure we don't infinite loop on cycles)
      if (jsToJsBackDeps[f]) stack = stack.concat(jsToJsBackDeps[f]);
    }
  }
}
jsToWasmDeps = transitiveJsToWasmDeps;

// Print final output
console.log(JSON.stringify(transitiveJsToWasmDeps));
