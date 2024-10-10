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
// 2. A function parameter, function(moduleArg) => Promise<Module>
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
#if MODULARIZE
var Module = moduleArg;
#elif USE_CLOSURE_COMPILER
// if (!Module)` is crucial for Closure Compiler here as it will otherwise replace every `Module` occurrence with a string
var /** @type {{
  canvas: HTMLCanvasElement,
  ctx: Object,
}}
 */ Module;
if (!Module) /** @suppress{checkTypes}*/Module = {"__EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__":1};
#elif AUDIO_WORKLET
var Module = globalThis.Module || (typeof {{{ EXPORT_NAME }}} != 'undefined' ? {{{ EXPORT_NAME }}} : {});
#else
var Module = typeof {{{ EXPORT_NAME }}} != 'undefined' ? {{{ EXPORT_NAME }}} : {};
#endif // USE_CLOSURE_COMPILER

#if POLYFILL
#if ((MAYBE_WASM2JS && WASM != 2) || MODULARIZE) && (MIN_CHROME_VERSION < 33 || MIN_FIREFOX_VERSION < 29 || MIN_SAFARI_VERSION < 80000)
// Include a Promise polyfill for legacy browsers. This is needed either for
// wasm2js, where we polyfill the wasm API which needs Promises, or when using
// modularize which creates a Promise for when the module is ready.
// See https://caniuse.com/#feat=promises
#include "polyfill/promise.js"
#endif

#if MIN_CHROME_VERSION < 45 || MIN_FIREFOX_VERSION < 34 || MIN_SAFARI_VERSION < 90000
// See https://caniuse.com/mdn-javascript_builtins_object_assign
#include "polyfill/objassign.js"
#endif

#if WASM_BIGINT && MIN_SAFARI_VERSION < 150000
// See https://caniuse.com/mdn-javascript_builtins_bigint64array
#include "polyfill/bigint64array.js"
#endif

#if MIN_CHROME_VERSION < 40 || MIN_FIREFOX_VERSION < 39 || MIN_SAFARI_VERSION < 103000
// See https://caniuse.com/fetch
#include "polyfill/fetch.js"
#endif
#endif // POLYFILL

#if MODULARIZE
// Set up the promise that indicates the Module is initialized
var readyPromiseResolve, readyPromiseReject;
var readyPromise = new Promise((resolve, reject) => {
  readyPromiseResolve = resolve;
  readyPromiseReject = reject;
});
#if ASSERTIONS
{{{ addReadyPromiseAssertions() }}}
#endif
#endif

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

#if AUDIO_WORKLET
var ENVIRONMENT_IS_AUDIO_WORKLET = typeof AudioWorkletGlobalScope !== 'undefined';
#endif

#if ENVIRONMENT && !ENVIRONMENT.includes(',')
var ENVIRONMENT_IS_WEB = {{{ ENVIRONMENT === 'web' }}};
#if PTHREADS && ENVIRONMENT_MAY_BE_NODE
// node+pthreads always supports workers; detect which we are at runtime
var ENVIRONMENT_IS_WORKER = typeof importScripts == 'function';
#else
var ENVIRONMENT_IS_WORKER = {{{ ENVIRONMENT === 'worker' }}};
#endif
var ENVIRONMENT_IS_NODE = {{{ ENVIRONMENT === 'node' }}};
var ENVIRONMENT_IS_SHELL = {{{ ENVIRONMENT === 'shell' }}};
#else // ENVIRONMENT
// Attempt to auto-detect the environment
var ENVIRONMENT_IS_WEB = typeof window == 'object';
var ENVIRONMENT_IS_WORKER = typeof importScripts == 'function';
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
var ENVIRONMENT_IS_NODE = typeof process == 'object' && typeof process.versions == 'object' && typeof process.versions.node == 'string' && process.type != 'renderer';
#if AUDIO_WORKLET
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER && !ENVIRONMENT_IS_AUDIO_WORKLET;
#else
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;
#endif
#endif // ENVIRONMENT

#if PTHREADS
// Three configurations we can be running in:
// 1) We could be the application main() thread running in the main JS UI thread. (ENVIRONMENT_IS_WORKER == false and ENVIRONMENT_IS_PTHREAD == false)
// 2) We could be the application main() thread proxied to worker. (with Emscripten -sPROXY_TO_WORKER) (ENVIRONMENT_IS_WORKER == true, ENVIRONMENT_IS_PTHREAD == false)
// 3) We could be an application pthread running in a worker. (ENVIRONMENT_IS_WORKER == true and ENVIRONMENT_IS_PTHREAD == true)

// The way we signal to a worker that it is hosting a pthread is to construct
// it with a specific name.
var ENVIRONMENT_IS_PTHREAD = ENVIRONMENT_IS_WORKER && self.name?.startsWith('em-pthread');

#if MODULARIZE && ASSERTIONS
if (ENVIRONMENT_IS_PTHREAD) {
  assert(!globalThis.moduleLoaded, 'module should only be loaded once on each pthread worker');
  globalThis.moduleLoaded = true;
}
#endif
#endif

#if ENVIRONMENT_MAY_BE_NODE
if (ENVIRONMENT_IS_NODE) {
  // `require()` is no-op in an ESM module, use `createRequire()` to construct
  // the require()` function.  This is only necessary for multi-environment
  // builds, `-sENVIRONMENT=node` emits a static import declaration instead.
  // TODO: Swap all `require()`'s with `import()`'s?
#if EXPORT_ES6 && ENVIRONMENT_MAY_BE_WEB
  const { createRequire } = await import('module');
  let dirname = import.meta.url;
  if (dirname.startsWith("data:")) {
    dirname = '/';
  }
  /** @suppress{duplicate} */
  var require = createRequire(dirname);
#endif

#if PTHREADS || WASM_WORKERS
  var worker_threads = require('worker_threads');
  global.Worker = worker_threads.Worker;
  ENVIRONMENT_IS_WORKER = !worker_threads.isMainThread;
#if PTHREADS
  // Under node we set `workerData` to `em-pthread` to signal that the worker
  // is hosting a pthread.
  ENVIRONMENT_IS_PTHREAD = ENVIRONMENT_IS_WORKER && worker_threads['workerData'] == 'em-pthread'
#endif // PTHREADS
#endif // PTHREADS || WASM_WORKERS
}
#endif // ENVIRONMENT_MAY_BE_NODE

#if WASM_WORKERS
var ENVIRONMENT_IS_WASM_WORKER = Module['$ww'];
#endif

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
{{{ preJS() }}}

// Sometimes an existing Module object exists with properties
// meant to overwrite the default module functionality. Here
// we collect those properties and reapply _after_ we configure
// the current environment's defaults to avoid having to be so
// defensive during initialization.
var moduleOverrides = Object.assign({}, Module);

var arguments_ = [];
var thisProgram = './this.program';
var quit_ = (status, toThrow) => {
  throw toThrow;
};

#if SHARED_MEMORY && !MODULARIZE
// In MODULARIZE mode _scriptName needs to be captured already at the very top of the page immediately when the page is parsed, so it is generated there
// before the page load. In non-MODULARIZE modes generate it here.
var _scriptName = (typeof document != 'undefined') ? document.currentScript?.src : undefined;

#if ENVIRONMENT_MAY_BE_NODE
if (ENVIRONMENT_IS_NODE) {
#if EXPORT_ES6
  _scriptName = typeof __filename != 'undefined' ? __filename : import.meta.url
#else
  _scriptName = __filename;
#endif
} else
#endif // ENVIRONMENT_MAY_BE_NODE
if (ENVIRONMENT_IS_WORKER) {
  _scriptName = self.location.href;
}
#endif // SHARED_MEMORY && !MODULARIZE

// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = '';
function locateFile(path) {
#if RUNTIME_DEBUG
  dbg('locateFile:', path, 'scriptDirectory:', scriptDirectory);
#endif
#if expectToReceiveOnModule('locateFile')
  if (Module['locateFile']) {
    return Module['locateFile'](path, scriptDirectory);
  }
#endif
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var readAsync, readBinary;

#if ENVIRONMENT_MAY_BE_NODE
if (ENVIRONMENT_IS_NODE) {
#if ENVIRONMENT && ASSERTIONS
  if (typeof process == 'undefined' || !process.release || process.release.name !== 'node') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
#endif

#if ASSERTIONS
  var nodeVersion = process.versions.node;
  var numericVersion = nodeVersion.split('.').slice(0, 3);
  numericVersion = (numericVersion[0] * 10000) + (numericVersion[1] * 100) + (numericVersion[2].split('-')[0] * 1);
  var minVersion = {{{ MIN_NODE_VERSION }}};
  if (numericVersion < {{{ MIN_NODE_VERSION }}}) {
    throw new Error('This emscripten-generated code requires node {{{ formattedMinNodeVersion() }}} (detected v' + nodeVersion + ')');
  }
#endif

  // These modules will usually be used on Node.js. Load them eagerly to avoid
  // the complexity of lazy-loading.
  var fs = require('fs');
  var nodePath = require('path');

#if EXPORT_ES6
  // EXPORT_ES6 + ENVIRONMENT_IS_NODE always requires use of import.meta.url,
  // since there's no way getting the current absolute path of the module when
  // support for that is not available.
  if (!import.meta.url.startsWith('data:')) {
    scriptDirectory = nodePath.dirname(require('url').fileURLToPath(import.meta.url)) + '/';
  }
#else
  scriptDirectory = __dirname + '/';
#endif

#include "node_shell_read.js"

  if (!Module['thisProgram'] && process.argv.length > 1) {
    thisProgram = process.argv[1].replace(/\\/g, '/');
  }

  arguments_ = process.argv.slice(2);

#if MODULARIZE
  // MODULARIZE will export the module in the proper place outside, we don't need to export here
#else
  if (typeof module != 'undefined') {
    module['exports'] = Module;
  }
#endif

#if NODEJS_CATCH_EXIT
  process.on('uncaughtException', (ex) => {
    // suppress ExitStatus exceptions from showing an error
#if RUNTIME_DEBUG
    dbg(`node: uncaughtException: ${ex}`)
#endif
    if (ex !== 'unwind' && !(ex instanceof ExitStatus) && !(ex.context instanceof ExitStatus)) {
      throw ex;
    }
  });
#endif

#if NODEJS_CATCH_REJECTION
  // Without this older versions of node (< v15) will log unhandled rejections
  // but return 0, which is not normally the desired behaviour.  This is
  // not be needed with node v15 and about because it is now the default
  // behaviour:
  // See https://nodejs.org/api/cli.html#cli_unhandled_rejections_mode
  var nodeMajor = process.versions.node.split(".")[0];
  if (nodeMajor < 15) {
    process.on('unhandledRejection', (reason) => { throw reason; });
  }
#endif

  quit_ = (status, toThrow) => {
    process.exitCode = status;
    throw toThrow;
  };

#if WASM == 2
  // If target shell does not support Wasm, load the JS version of the code.
  if (typeof WebAssembly == 'undefined') {
    eval(fs.readFileSync(locateFile('{{{ TARGET_BASENAME }}}.wasm.js'))+'');
  }
#endif

} else
#endif // ENVIRONMENT_MAY_BE_NODE
#if ENVIRONMENT_MAY_BE_SHELL || ASSERTIONS
if (ENVIRONMENT_IS_SHELL) {

#if ENVIRONMENT && ASSERTIONS
  if ((typeof process == 'object' && typeof require === 'function') || typeof window == 'object' || typeof importScripts == 'function') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
#endif

#if ENVIRONMENT_MAY_BE_SHELL
  readBinary = (f) => {
    if (typeof readbuffer == 'function') {
      return new Uint8Array(readbuffer(f));
    }
    let data = read(f, 'binary');
    assert(typeof data == 'object');
    return data;
  };

  readAsync = (f) => {
    return new Promise((resolve, reject) => {
      setTimeout(() => resolve(readBinary(f)));
    });
  };

  globalThis.clearTimeout ??= (id) => {};

  // spidermonkey lacks setTimeout but we use it above in readAsync.
  globalThis.setTimeout ??= (f) => (typeof f == 'function') ? f() : abort();

  // v8 uses `arguments_` whereas spidermonkey uses `scriptArgs`
  arguments_ = globalThis.arguments || globalThis.scriptArgs;

  if (typeof quit == 'function') {
    quit_ = (status, toThrow) => {
      // Unlike node which has process.exitCode, d8 has no such mechanism. So we
      // have no way to set the exit code and then let the program exit with
      // that code when it naturally stops running (say, when all setTimeouts
      // have completed). For that reason, we must call `quit` - the only way to
      // set the exit code - but quit also halts immediately.  To increase
      // consistency with node (and the web) we schedule the actual quit call
      // using a setTimeout to give the current stack and any exception handlers
      // a chance to run.  This enables features such as addOnPostRun (which
      // expected to be able to run code after main returns).
      setTimeout(() => {
        if (!(toThrow instanceof ExitStatus)) {
          let toLog = toThrow;
          if (toThrow && typeof toThrow == 'object' && toThrow.stack) {
            toLog = [toThrow, toThrow.stack];
          }
          err(`exiting due to exception: ${toLog}`);
        }
        quit(status);
      });
      throw toThrow;
    };
  }

  if (typeof print != 'undefined') {
    // Prefer to use print/printErr where they exist, as they usually work better.
    globalThis.console ??= /** @type{!Console} */({});
    console.log = /** @type{!function(this:Console, ...*): undefined} */ (print);
    console.warn = console.error = /** @type{!function(this:Console, ...*): undefined} */ (globalThis.printErr ?? print);
  }

#if WASM == 2
  // If target shell does not support Wasm, load the JS version of the code.
  if (typeof WebAssembly == 'undefined') {
    eval(read(locateFile('{{{ TARGET_BASENAME }}}.wasm.js'))+'');
  }
#endif
#endif // ENVIRONMENT_MAY_BE_SHELL

} else
#endif // ENVIRONMENT_MAY_BE_SHELL || ASSERTIONS

// Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
#if ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  if (ENVIRONMENT_IS_WORKER) { // Check worker, not web, since window could be polyfilled
    scriptDirectory = self.location.href;
  } else if (typeof document != 'undefined' && document.currentScript) { // web
    scriptDirectory = document.currentScript.src;
  }
#if MODULARIZE
  // When MODULARIZE, this JS may be executed later, after document.currentScript
  // is gone, so we saved it, and we use it here instead of any other info.
  if (_scriptName) {
    scriptDirectory = _scriptName;
  }
#endif
  // blob urls look like blob:http://site.com/etc/etc and we cannot infer anything from them.
  // otherwise, slice off the final part of the url to find the script directory.
  // if scriptDirectory does not contain a slash, lastIndexOf will return -1,
  // and scriptDirectory will correctly be replaced with an empty string.
  // If scriptDirectory contains a query (starting with ?) or a fragment (starting with #),
  // they are removed because they could contain a slash.
  if (scriptDirectory.startsWith('blob:')) {
    scriptDirectory = '';
  } else {
    scriptDirectory = scriptDirectory.substr(0, scriptDirectory.replace(/[?#].*/, '').lastIndexOf('/')+1);
  }

#if ENVIRONMENT && ASSERTIONS
  if (!(typeof window == 'object' || typeof importScripts == 'function')) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
#endif

#if PTHREADS && ENVIRONMENT_MAY_BE_NODE
  // Differentiate the Web Worker from the Node Worker case, as reading must
  // be done differently.
  if (!ENVIRONMENT_IS_NODE)
#endif
  {
#include "web_or_worker_shell_read.js"
  }
} else
#endif // ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
#if AUDIO_WORKLET && ASSERTIONS
if (!ENVIRONMENT_IS_AUDIO_WORKLET)
#endif
{
#if ASSERTIONS
  throw new Error('environment detection error');
#endif // ASSERTIONS
}

#if ENVIRONMENT_MAY_BE_NODE && PTHREADS
// Set up the out() and err() hooks, which are how we can print to stdout or
// stderr, respectively.
// Normally just binding console.log/console.error here works fine, but
// under node (with workers) we see missing/out-of-order messages so route
// directly to stdout and stderr.
// See https://github.com/emscripten-core/emscripten/issues/14804
var defaultPrint = console.log.bind(console);
var defaultPrintErr = console.error.bind(console);
if (ENVIRONMENT_IS_NODE) {
  defaultPrint = (...args) => fs.writeSync(1, args.join(' ') + '\n');
  defaultPrintErr = (...args) => fs.writeSync(2, args.join(' ') + '\n');
}
{{{ makeModuleReceiveWithVar('out', 'print',    'defaultPrint',    true) }}}
{{{ makeModuleReceiveWithVar('err', 'printErr', 'defaultPrintErr', true) }}}
#else
{{{ makeModuleReceiveWithVar('out', 'print',    'console.log.bind(console)',  true) }}}
{{{ makeModuleReceiveWithVar('err', 'printErr', 'console.error.bind(console)', true) }}}
#endif

// Merge back in the overrides
Object.assign(Module, moduleOverrides);
// Free the object hierarchy contained in the overrides, this lets the GC
// reclaim data used.
moduleOverrides = null;
#if ASSERTIONS
checkIncomingModuleAPI();
#endif

// Emit code to handle expected values on the Module object. This applies Module.x
// to the proper local x. This has two benefits: first, we only emit it if it is
// expected to arrive, and second, by using a local everywhere else that can be
// minified.
{{{ makeModuleReceive('arguments_', 'arguments') }}}
{{{ makeModuleReceive('thisProgram') }}}

// perform assertions in shell.js after we set up out() and err(), as otherwise if an assertion fails it cannot print the message
#if ASSERTIONS
// Assertions on removed incoming Module JS APIs.
assert(typeof Module['memoryInitializerPrefixURL'] == 'undefined', 'Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['pthreadMainPrefixURL'] == 'undefined', 'Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['cdInitializerPrefixURL'] == 'undefined', 'Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['filePackagePrefixURL'] == 'undefined', 'Module.filePackagePrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['read'] == 'undefined', 'Module.read option was removed');
assert(typeof Module['readAsync'] == 'undefined', 'Module.readAsync option was removed (modify readAsync in JS)');
assert(typeof Module['readBinary'] == 'undefined', 'Module.readBinary option was removed (modify readBinary in JS)');
assert(typeof Module['setWindowTitle'] == 'undefined', 'Module.setWindowTitle option was removed (modify emscripten_set_window_title in JS)');
assert(typeof Module['TOTAL_MEMORY'] == 'undefined', 'Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY');
{{{ makeRemovedModuleAPIAssert('asm', 'wasmExports', false) }}}
{{{ makeRemovedModuleAPIAssert('readAsync') }}}
{{{ makeRemovedModuleAPIAssert('readBinary') }}}
{{{ makeRemovedModuleAPIAssert('setWindowTitle') }}}
{{{ makeRemovedFSAssert('IDBFS') }}}
{{{ makeRemovedFSAssert('PROXYFS') }}}
{{{ makeRemovedFSAssert('WORKERFS') }}}
{{{ makeRemovedFSAssert('FETCHFS') }}}
{{{ makeRemovedFSAssert('ICASEFS') }}}
{{{ makeRemovedFSAssert('JSFILEFS') }}}
{{{ makeRemovedFSAssert('OPFS') }}}

#if !NODERAWFS
{{{ makeRemovedFSAssert('NODEFS') }}}
#endif

#if PTHREADS
assert(
#if AUDIO_WORKLET
  ENVIRONMENT_IS_AUDIO_WORKLET ||
#endif
  ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER || ENVIRONMENT_IS_NODE, 'Pthreads do not work in this environment yet (need Web Workers, or an alternative to them)');
#else
#endif // PTHREADS

#if !ENVIRONMENT_MAY_BE_WEB
assert(!ENVIRONMENT_IS_WEB, 'web environment detected but not enabled at build time.  Add `web` to `-sENVIRONMENT` to enable.');
#endif

#if !ENVIRONMENT_MAY_BE_WORKER
assert(!ENVIRONMENT_IS_WORKER, 'worker environment detected but not enabled at build time.  Add `worker` to `-sENVIRONMENT` to enable.');
#endif

#if !ENVIRONMENT_MAY_BE_NODE
assert(!ENVIRONMENT_IS_NODE, 'node environment detected but not enabled at build time.  Add `node` to `-sENVIRONMENT` to enable.');
#endif

#if !ENVIRONMENT_MAY_BE_SHELL
assert(!ENVIRONMENT_IS_SHELL, 'shell environment detected but not enabled at build time.  Add `shell` to `-sENVIRONMENT` to enable.');
#endif

#endif // ASSERTIONS
