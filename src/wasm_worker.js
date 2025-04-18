// N.B. The contents of this file are duplicated in src/library_wasm_worker.js
// in variable "_wasmWorkerBlobUrl" (where the contents are pre-minified) If
// doing any changes to this file, be sure to update the contents there too.

'use strict';

#if ENVIRONMENT_MAY_BE_NODE
// Node.js support
var ENVIRONMENT_IS_NODE = typeof process == 'object' && typeof process.versions == 'object' && typeof process.versions.node == 'string' && process.type != 'renderer';
if (ENVIRONMENT_IS_NODE) {
  // Create as web-worker-like an environment as we can.

  var nodeWorkerThreads = require('worker_threads');

  var parentPort = nodeWorkerThreads.parentPort;

  parentPort.on('message', (msg) => global.onmessage?.({ data: msg }));

  // Weak map of handle functions to their wrapper. Used to implement
  // addEventListener/removeEventListener.
  var wrappedHandlers = new WeakMap();
  function wrapMsgHandler(h) {
    var f = wrappedHandlers.get(h)
    if (!f) {
      f = (msg) => h({data: msg});
      wrappedHandlers.set(h, f);
    }
    return f;
  }

  var fs = require('fs');
  var vm = require('vm');

  Object.assign(global, {
    self: global,
    require,
#if !EXPORT_ES6
    // `vm.runInThisContext` global scope lacks `__filename` and `__dirname`
    __filename,
    __dirname,
#endif
    Worker: nodeWorkerThreads.Worker,
    importScripts: (f) => vm.runInThisContext(fs.readFileSync(f, 'utf8'), {filename: f}),
    postMessage: (msg) => parentPort.postMessage(msg),
    performance: global.performance || { now: Date.now },
    addEventListener: (name, handler) => parentPort.on(name, wrapMsgHandler(handler)),
    removeEventListener: (name, handler) => parentPort.off(name, wrapMsgHandler(handler)),
  });
}
#endif // ENVIRONMENT_MAY_BE_NODE

{{{ implicitSelf() }}}onmessage = function(d) {
  // The first message sent to the Worker is always the bootstrap message.
  // Drop this message listener, it served its purpose of bootstrapping
  // the Wasm Module load, and is no longer needed. Let user code register
  // any desired message handlers from now on.
  {{{ implicitSelf() }}}onmessage = null;
  d = d.data;
#if !MODULARIZE
  self.{{{ EXPORT_NAME }}} = d;
#endif
#if !MINIMAL_RUNTIME
  d['instantiateWasm'] = (info, receiveInstance) => { var instance = new WebAssembly.Instance(d['wasm'], info); return receiveInstance(instance, d['wasm']); }
#endif
#if TRUSTED_TYPES
  // Use Trusted Types compatible wrappers.
  if (typeof trustedTypes != 'undefined' && trustedTypes.createPolicy) {
    var p = trustedTypes.createPolicy('emscripten#scriptPolicy1', { createScriptURL: (ignored) => d.js });
    importScripts(p.createScriptURL('ignored'));
  } else
#endif
  importScripts(d.js);
#if MODULARIZE
  {{{ EXPORT_NAME }}}(d);
#endif
  // Drop now unneeded references to from the Module object in this Worker,
  // these are not needed anymore.
  d.wasm = d.mem = d.js = 0;
}
