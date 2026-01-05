/**
 * @license
 * Copyright 2025 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// This file is used as the initial script loaded into pthread workers when
// running in WASM_ESM_INTEGRATION mode.
// The point of this file is to delay the loading of the main program module
// until the wasm memory has been received via postMessage.

#if RUNTIME_DEBUG
console.log("Running pthread_esm_startup");
#endif

#if ENVIRONMENT_MAY_BE_NODE
if ({{{ nodeDetectionCode() }}}) {
  // Create as web-worker-like an environment as we can.
  globalThis.self = globalThis;
  var worker_threads = await import('worker_threads');
  globalThis.Worker = worker_threads.Worker;
  var parentPort = worker_threads['parentPort'];
  // Deno and Bun already have `postMessage` defined on the global scope and
  // deliver messages to `globalThis.onmessage`, so we must not duplicate that
  // behavior here if `postMessage` is already present.
  if (!globalThis.postMessage) {
    parentPort.on('message', (msg) => globalThis.onmessage?.({ data: msg }));
    globalThis.postMessage = (msg) => parentPort['postMessage'](msg);
  }
}
#endif

self.onmessage = async (msg) => {
#if RUNTIME_DEBUG
  console.log('pthread_esm_startup', msg.data.cmd);
#endif
  if (msg.data.cmd == 'load') {
    // Until we initialize the runtime, queue up any further incoming messages
    // that can arrive while the async import (await import below) is happening.
    // For examples the `run` message often arrives right away before the import
    // is complete.
    let messageQueue = [msg];
    self.onmessage = (e) => messageQueue.push(e);

    // Now that we have the wasmMemory we can import the main program
    globalThis.wasmMemory = msg.data.wasmMemory;
    const prog = await import('./{{{ TARGET_JS_NAME }}}');

    // Now that the import is completed the main program will have installed
    // its own `onmessage` handler and replaced our handler.
    // Now we can dispatch any queued messages to this new handler.
    for (let msg of messageQueue) {
      await self.onmessage(msg);
    }

    await prog.default()
  }
};
