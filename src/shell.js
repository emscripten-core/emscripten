/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if STRICT_JS
"use strict";

#endif
// The Module object: Our interface to the outside world. We import
// and export values on it. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(Module) { ..generated code.. }
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
#if USE_CLOSURE_COMPILER
// if (!Module)` is crucial for Closure Compiler here as it will otherwise replace every `Module` occurrence with a string
var /** @type {{
  noImageDecoding: boolean,
  noAudioDecoding: boolean,
  canvas: HTMLCanvasElement,
  dataFileDownloads: Object,
  preloadResults: Object
}}
 */ Module;
if (!Module) /** @suppress{checkTypes}*/Module = {"__EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__":1};
#else
var Module = typeof {{{ EXPORT_NAME }}} !== 'undefined' ? {{{ EXPORT_NAME }}} : {};
#endif // USE_CLOSURE_COMPILER

#if ((MAYBE_WASM2JS && WASM != 2) || MODULARIZE) && (MIN_CHROME_VERSION < 33 || MIN_EDGE_VERSION < 12 || MIN_FIREFOX_VERSION < 29 || MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION < 80000) // https://caniuse.com/#feat=promises
// Include a Promise polyfill for legacy browsers. This is needed either for
// wasm2js, where we polyfill the wasm API which needs Promises, or when using
// modularize which creates a Promise for when the module is ready.
#include "promise_polyfill.js"
#endif

#if MODULARIZE
// Set up the promise that indicates the Module is initialized
var readyPromiseResolve, readyPromiseReject;
Module['ready'] = new Promise(function(resolve, reject) {
  readyPromiseResolve = resolve;
  readyPromiseReject = reject;
});
#if ASSERTIONS
{{{ addReadyPromiseAssertions("Module['ready']") }}}
#endif
#endif

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
// {{PRE_JSES}}

// Sometimes an existing Module object exists with properties
// meant to overwrite the default module functionality. Here
// we collect those properties and reapply _after_ we configure
// the current environment's defaults to avoid having to be so
// defensive during initialization.
var moduleOverrides = {};
var key;
for (key in Module) {
  if (Module.hasOwnProperty(key)) {
    moduleOverrides[key] = Module[key];
  }
}

var arguments_ = [];
var thisProgram = './this.program';
var quit_ = function(status, toThrow) {
  throw toThrow;
};

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

#if ENVIRONMENT && ENVIRONMENT.indexOf(',') < 0
var ENVIRONMENT_IS_WEB = {{{ ENVIRONMENT === 'web' }}};
var ENVIRONMENT_IS_WORKER = {{{ ENVIRONMENT === 'worker' }}};
var ENVIRONMENT_IS_NODE = {{{ ENVIRONMENT === 'node' }}};
var ENVIRONMENT_IS_SHELL = {{{ ENVIRONMENT === 'shell' }}};
#else // ENVIRONMENT
var ENVIRONMENT_IS_WEB = false;
var ENVIRONMENT_IS_WORKER = false;
var ENVIRONMENT_IS_NODE = false;
var ENVIRONMENT_IS_SHELL = false;
ENVIRONMENT_IS_WEB = typeof window === 'object';
ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
ENVIRONMENT_IS_NODE = typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string';
ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;
#endif // ENVIRONMENT

#if ASSERTIONS
if (Module['ENVIRONMENT']) {
  throw new Error('Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -s ENVIRONMENT=web or -s ENVIRONMENT=node)');
}
#endif

#if USE_PTHREADS
#include "shell_pthreads.js"
#endif

#if USE_PTHREADS && !MODULARIZE
// In MODULARIZE mode _scriptDir needs to be captured already at the very top of the page immediately when the page is parsed, so it is generated there
// before the page load. In non-MODULARIZE modes generate it here.
#if EXPORT_ES6
var _scriptDir = import.meta.url;
#else
var _scriptDir = (typeof document !== 'undefined' && document.currentScript) ? document.currentScript.src : undefined;

if (ENVIRONMENT_IS_WORKER) {
  _scriptDir = self.location.href;
}
#if ENVIRONMENT_MAY_BE_NODE
else if (ENVIRONMENT_IS_NODE) {
  _scriptDir = __filename;
}
#endif // ENVIRONMENT_MAY_BE_NODE
#endif
#endif

// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = '';
function locateFile(path) {
#if expectToReceiveOnModule('locateFile')
  if (Module['locateFile']) {
    return Module['locateFile'](path, scriptDirectory);
  }
#endif
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var read_,
    readAsync,
    readBinary,
    setWindowTitle;

#if ENVIRONMENT_MAY_BE_NODE
var nodeFS;
var nodePath;

if (ENVIRONMENT_IS_NODE) {
#if ENVIRONMENT
#if ASSERTIONS
  if (!(typeof process === 'object' && typeof require === 'function')) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
#endif
#endif
  if (ENVIRONMENT_IS_WORKER) {
    scriptDirectory = require('path').dirname(scriptDirectory) + '/';
  } else {
    scriptDirectory = __dirname + '/';
  }

#include "node_shell_read.js"

  if (process['argv'].length > 1) {
    thisProgram = process['argv'][1].replace(/\\/g, '/');
  }

  arguments_ = process['argv'].slice(2);

#if MODULARIZE
  // MODULARIZE will export the module in the proper place outside, we don't need to export here
#else
  if (typeof module !== 'undefined') {
    module['exports'] = Module;
  }
#endif

#if NODEJS_CATCH_EXIT
  process['on']('uncaughtException', function(ex) {
    // suppress ExitStatus exceptions from showing an error
    if (!(ex instanceof ExitStatus)) {
      throw ex;
    }
  });
#endif

#if NODEJS_CATCH_REJECTION
  process['on']('unhandledRejection', abort);
#endif

  quit_ = function(status) {
    process['exit'](status);
  };

  Module['inspect'] = function () { return '[Emscripten Module object]'; };

#if USE_PTHREADS
  var nodeWorkerThreads;
  try {
    nodeWorkerThreads = require('worker_threads');
  } catch (e) {
    console.error('The "worker_threads" module is not supported in this node.js build - perhaps a newer version is needed?');
    throw e;
  }
  global.Worker = nodeWorkerThreads.Worker;
#endif

#if WASM == 2
  // If target shell does not support Wasm, load the JS version of the code.
  if (typeof WebAssembly === 'undefined') {
    var fs = require('fs');
    eval(fs.readFileSync(locateFile('{{{ TARGET_BASENAME }}}.wasm.js'))+'');
  }
#endif

} else
#endif // ENVIRONMENT_MAY_BE_NODE
#if ENVIRONMENT_MAY_BE_SHELL
if (ENVIRONMENT_IS_SHELL) {

#if ENVIRONMENT
#if ASSERTIONS
  if ((typeof process === 'object' && typeof require === 'function') || typeof window === 'object' || typeof importScripts === 'function') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
#endif
#endif

  if (typeof read != 'undefined') {
    read_ = function shell_read(f) {
#if SUPPORT_BASE64_EMBEDDING
      var data = tryParseAsDataURI(f);
      if (data) {
        return intArrayToString(data);
      }
#endif
      return read(f);
    };
  }

  readBinary = function readBinary(f) {
    var data;
#if SUPPORT_BASE64_EMBEDDING
    data = tryParseAsDataURI(f);
    if (data) {
      return data;
    }
#endif
    if (typeof readbuffer === 'function') {
      return new Uint8Array(readbuffer(f));
    }
    data = read(f, 'binary');
    assert(typeof data === 'object');
    return data;
  };

  if (typeof scriptArgs != 'undefined') {
    arguments_ = scriptArgs;
  } else if (typeof arguments != 'undefined') {
    arguments_ = arguments;
  }

  if (typeof quit === 'function') {
    quit_ = function(status) {
      quit(status);
    };
  }

  if (typeof print !== 'undefined') {
    // Prefer to use print/printErr where they exist, as they usually work better.
    if (typeof console === 'undefined') console = /** @type{!Console} */({});
    console.log = /** @type{!function(this:Console, ...*): undefined} */ (print);
    console.warn = console.error = /** @type{!function(this:Console, ...*): undefined} */ (typeof printErr !== 'undefined' ? printErr : print);
  }

#if WASM == 2
  // If target shell does not support Wasm, load the JS version of the code.
  if (typeof WebAssembly === 'undefined') {
    eval(read(locateFile('{{{ TARGET_BASENAME }}}.wasm.js'))+'');
  }
#endif

} else
#endif // ENVIRONMENT_MAY_BE_SHELL

// Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
#if ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  if (ENVIRONMENT_IS_WORKER) { // Check worker, not web, since window could be polyfilled
    scriptDirectory = self.location.href;
  } else if (document.currentScript) { // web
    scriptDirectory = document.currentScript.src;
  }
#if MODULARIZE
  // When MODULARIZE, this JS may be executed later, after document.currentScript
  // is gone, so we saved it, and we use it here instead of any other info.
  if (_scriptDir) {
    scriptDirectory = _scriptDir;
  }
#endif
  // blob urls look like blob:http://site.com/etc/etc and we cannot infer anything from them.
  // otherwise, slice off the final part of the url to find the script directory.
  // if scriptDirectory does not contain a slash, lastIndexOf will return -1,
  // and scriptDirectory will correctly be replaced with an empty string.
  if (scriptDirectory.indexOf('blob:') !== 0) {
    scriptDirectory = scriptDirectory.substr(0, scriptDirectory.lastIndexOf('/')+1);
  } else {
    scriptDirectory = '';
  }

#if ENVIRONMENT
#if ASSERTIONS
  if (!(typeof window === 'object' || typeof importScripts === 'function')) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
#endif
#endif

  // Differentiate the Web Worker from the Node Worker case, as reading must
  // be done differently.
#if USE_PTHREADS && ENVIRONMENT_MAY_BE_NODE
  if (ENVIRONMENT_IS_NODE) {

#include "node_shell_read.js"

  } else
#endif
  {

#include "web_or_worker_shell_read.js"

  }

  setWindowTitle = function(title) { document.title = title };
} else
#endif // ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
{
#if ASSERTIONS
  throw new Error('environment detection error');
#endif // ASSERTIONS
}

#if ENVIRONMENT_MAY_BE_NODE && USE_PTHREADS
if (ENVIRONMENT_IS_NODE) {
  // Polyfill the performance object, which emscripten pthreads support
  // depends on for good timing.
  if (typeof performance === 'undefined') {
    global.performance = require('perf_hooks').performance;
  }
}
#endif

// Set up the out() and err() hooks, which are how we can print to stdout or
// stderr, respectively.
{{{ makeModuleReceiveWithVar('out', 'print',    'console.log.bind(console)',  true) }}}
{{{ makeModuleReceiveWithVar('err', 'printErr', 'console.warn.bind(console)', true) }}}

// Merge back in the overrides
for (key in moduleOverrides) {
  if (moduleOverrides.hasOwnProperty(key)) {
    Module[key] = moduleOverrides[key];
  }
}
// Free the object hierarchy contained in the overrides, this lets the GC
// reclaim data used e.g. in memoryInitializerRequest, which is a large typed array.
moduleOverrides = null;

// Emit code to handle expected values on the Module object. This applies Module.x
// to the proper local x. This has two benefits: first, we only emit it if it is
// expected to arrive, and second, by using a local everywhere else that can be
// minified.
{{{ makeModuleReceive('arguments_', 'arguments') }}}
{{{ makeModuleReceive('thisProgram') }}}
{{{ makeModuleReceive('quit_', 'quit') }}}

// perform assertions in shell.js after we set up out() and err(), as otherwise if an assertion fails it cannot print the message
#if ASSERTIONS
// Assertions on removed incoming Module JS APIs.
assert(typeof Module['memoryInitializerPrefixURL'] === 'undefined', 'Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['pthreadMainPrefixURL'] === 'undefined', 'Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['cdInitializerPrefixURL'] === 'undefined', 'Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['filePackagePrefixURL'] === 'undefined', 'Module.filePackagePrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['read'] === 'undefined', 'Module.read option was removed (modify read_ in JS)');
assert(typeof Module['readAsync'] === 'undefined', 'Module.readAsync option was removed (modify readAsync in JS)');
assert(typeof Module['readBinary'] === 'undefined', 'Module.readBinary option was removed (modify readBinary in JS)');
assert(typeof Module['setWindowTitle'] === 'undefined', 'Module.setWindowTitle option was removed (modify setWindowTitle in JS)');
assert(typeof Module['TOTAL_MEMORY'] === 'undefined', 'Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY');
{{{ makeRemovedModuleAPIAssert('read', 'read_') }}}
{{{ makeRemovedModuleAPIAssert('readAsync') }}}
{{{ makeRemovedModuleAPIAssert('readBinary') }}}
{{{ makeRemovedModuleAPIAssert('setWindowTitle') }}}
{{{ makeRemovedFSAssert('IDBFS') }}}
{{{ makeRemovedFSAssert('PROXYFS') }}}
{{{ makeRemovedFSAssert('WORKERFS') }}}
{{{ makeRemovedFSAssert('NODEFS') }}}

#if USE_PTHREADS
assert(ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER || ENVIRONMENT_IS_NODE, 'Pthreads do not work in this environment yet (need Web Workers, or an alternative to them)');
#endif // USE_PTHREADS
#endif // ASSERTIONS

{{BODY}}

// {{MODULE_ADDITIONS}}
