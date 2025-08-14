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
#if MODULARIZE == 'instance'
var Module = {};
#else
var Module = moduleArg;
#endif
#elif USE_CLOSURE_COMPILER
/** @type{Object} */
var Module;
// if (!Module)` is crucial for Closure Compiler here as it will otherwise replace every `Module` occurrence with a string
if (!Module) /** @suppress{checkTypes}*/Module = {"__EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__":1};
#elif AUDIO_WORKLET
var Module = globalThis.Module || (typeof {{{ EXPORT_NAME }}} != 'undefined' ? {{{ EXPORT_NAME }}} : {});
#else
var Module = typeof {{{ EXPORT_NAME }}} != 'undefined' ? {{{ EXPORT_NAME }}} : {};
#endif // USE_CLOSURE_COMPILER

#if POLYFILL
#if WASM_BIGINT && MIN_SAFARI_VERSION < 150000
// See https://caniuse.com/mdn-javascript_builtins_bigint64array
#include "polyfill/bigint64array.js"
#endif
#endif // POLYFILL

#if WASM_WORKERS
// The way we signal to a worker that it is hosting a pthread is to construct
// it with a specific name.
var ENVIRONMENT_IS_WASM_WORKER = globalThis.name == 'em-ww';
#endif

#if AUDIO_WORKLET
var ENVIRONMENT_IS_AUDIO_WORKLET = typeof AudioWorkletGlobalScope !== 'undefined';
// Audio worklets behave as wasm workers.
if (ENVIRONMENT_IS_AUDIO_WORKLET) ENVIRONMENT_IS_WASM_WORKER = true;
#endif

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

#if ENVIRONMENT.length == 1
var ENVIRONMENT_IS_WEB = {{{ ENVIRONMENT[0] === 'web' }}};
#if PTHREADS && ENVIRONMENT_MAY_BE_NODE
// node+pthreads always supports workers; detect which we are at runtime
var ENVIRONMENT_IS_WORKER = typeof WorkerGlobalScope != 'undefined';
#else
var ENVIRONMENT_IS_WORKER = {{{ ENVIRONMENT[0] === 'worker' }}};
#endif
var ENVIRONMENT_IS_NODE = {{{ ENVIRONMENT[0] === 'node' }}};
var ENVIRONMENT_IS_SHELL = {{{ ENVIRONMENT[0] === 'shell' }}};
#else // ENVIRONMENT
// Attempt to auto-detect the environment
var ENVIRONMENT_IS_WEB = typeof window == 'object';
var ENVIRONMENT_IS_WORKER = typeof WorkerGlobalScope != 'undefined';
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
var ENVIRONMENT_IS_NODE = {{{ nodeDetectionCode() }}};
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

#if ENVIRONMENT_MAY_BE_NODE && (EXPORT_ES6 || PTHREADS || WASM_WORKERS)
if (ENVIRONMENT_IS_NODE) {
#if EXPORT_ES6
  // When building an ES module `require` is not normally available.
  // We need to use `createRequire()` to construct the require()` function.
  const { createRequire } = await import('module');
  /** @suppress{duplicate} */
  var require = createRequire(import.meta.url);
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
#if WASM_WORKERS
  ENVIRONMENT_IS_WASM_WORKER = ENVIRONMENT_IS_WORKER && worker_threads['workerData'] == 'em-ww'
#endif
#endif // PTHREADS || WASM_WORKERS
}
#endif // ENVIRONMENT_MAY_BE_NODE

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
{{{ preJS() }}}

var arguments_ = [];
var thisProgram = './this.program';
var quit_ = (status, toThrow) => {
  throw toThrow;
};

#if EXPORT_ES6
var _scriptName = import.meta.url;
#else
#if ENVIRONMENT_MAY_BE_WEB
#if !MODULARIZE
// In MODULARIZE mode _scriptName needs to be captured already at the very top of the page immediately when the page is parsed, so it is generated there
// before the page load. In non-MODULARIZE modes generate it here.
#if SINGLE_FILE && OUTPUT_FORMAT == 'HTML'
var _scriptName = typeof document != 'undefined' ? URL.createObjectURL(new Blob([document.getElementById('mainScript').textContent], { "type" : "text/javascript" })) : undefined;
#else
var _scriptName = typeof document != 'undefined' ? document.currentScript?.src : undefined;
#endif
#endif // !MODULARIZE
#elif ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_WORKER
var _scriptName;
#endif // ENVIRONMENT_MAY_BE_WEB

#if ENVIRONMENT_MAY_BE_NODE
if (typeof __filename != 'undefined') { // Node
  _scriptName = __filename;
} else
#endif // ENVIRONMENT_MAY_BE_NODE
#if ENVIRONMENT_MAY_BE_WORKER
if (ENVIRONMENT_IS_WORKER) {
  _scriptName = self.location.href;
}
#elif ENVIRONMENT_MAY_BE_NODE
  /*no-op*/{}
#endif // ENVIRONMENT_MAY_BE_WORKER
#endif // EXPORT_ES6

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
#if ENVIRONMENT.length && ASSERTIONS
  const isNode = {{{ nodeDetectionCode() }}};
  if (!isNode) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
#endif

#if ASSERTIONS
  var nodeVersion = process.versions.node;
  var numericVersion = nodeVersion.split('.').slice(0, 3);
  numericVersion = (numericVersion[0] * 10000) + (numericVersion[1] * 100) + (numericVersion[2].split('-')[0] * 1);
  if (numericVersion < {{{ MIN_NODE_VERSION }}}) {
    throw new Error('This emscripten-generated code requires node {{{ formattedMinNodeVersion() }}} (detected v' + nodeVersion + ')');
  }
#endif

  // These modules will usually be used on Node.js. Load them eagerly to avoid
  // the complexity of lazy-loading.
  var fs = require('fs');

#if EXPORT_ES6
  if (_scriptName.startsWith('file:')) {
    scriptDirectory = require('path').dirname(require('url').fileURLToPath(_scriptName)) + '/';
  }
#else
  scriptDirectory = __dirname + '/';
#endif

#include "node_shell_read.js"

  if (process.argv.length > 1) {
    thisProgram = process.argv[1].replace(/\\/g, '/');
  }

  arguments_ = process.argv.slice(2);

#if !MODULARIZE
  // MODULARIZE will export the module in the proper place outside, we don't need to export here
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

#if ENVIRONMENT.length && ASSERTIONS
  const isNode = {{{ nodeDetectionCode() }}};
  if (isNode || typeof window == 'object' || typeof WorkerGlobalScope != 'undefined') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
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

  readAsync = async (f) => readBinary(f);

  globalThis.clearTimeout ??= (id) => {};

  // spidermonkey lacks setTimeout but we use it above in readAsync.
  globalThis.setTimeout ??= (f) => f();

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
  try {
    scriptDirectory = new URL('.', _scriptName).href; // includes trailing slash
  } catch {
    // Must be a `blob:` or `data:` URL (e.g. `blob:http://site.com/etc/etc`), we cannot
    // infer anything from them.
  }

#if ENVIRONMENT.length && ASSERTIONS
  if (!(typeof window == 'object' || typeof WorkerGlobalScope != 'undefined')) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
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

#if ENVIRONMENT_MAY_BE_NODE && (PTHREADS || WASM_WORKERS)
// Set up the out() and err() hooks, which are how we can print to stdout or
// stderr, respectively.
// Normally just binding console.log/console.error here works fine, but
// under node (with workers) we see missing/out-of-order messages so route
// directly to stdout and stderr.
// See https://github.com/emscripten-core/emscripten/issues/14804
var defaultPrint = console.log.bind(console);
var defaultPrintErr = console.error.bind(console);
if (ENVIRONMENT_IS_NODE) {
  var utils = require('util');
  var stringify = (a) => typeof a == 'object' ? utils.inspect(a) : a;
  defaultPrint = (...args) => fs.writeSync(1, args.map(stringify).join(' ') + '\n');
  defaultPrintErr = (...args) => fs.writeSync(2, args.map(stringify).join(' ') + '\n');
}
{{{ makeModuleReceiveWithVar('out', 'print',    'defaultPrint') }}}
{{{ makeModuleReceiveWithVar('err', 'printErr', 'defaultPrintErr') }}}
#else
{{{ makeModuleReceiveWithVar('out', 'print',    'console.log.bind(console)') }}}
{{{ makeModuleReceiveWithVar('err', 'printErr', 'console.error.bind(console)') }}}
#endif

#if ASSERTIONS

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

// perform assertions in shell.js after we set up out() and err(), as otherwise
// if an assertion fails it cannot print the message
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
