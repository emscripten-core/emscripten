#if AUDIO_WORKLET
if (ENVIRONMENT_IS_WASM_WORKER && !ENVIRONMENT_IS_AUDIO_WORKLET) {
#else
if (ENVIRONMENT_IS_WASM_WORKER) {
#endif
#if RUNTIME_DEBUG
  dbg('wasm worker starting ...');
#endif

#if ENVIRONMENT_MAY_BE_NODE
// Node.js support
if (ENVIRONMENT_IS_NODE) {
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

  Object.assign(globalThis, {
    addEventListener: (name, handler) => parentPort['on'](name, wrapMsgHandler(handler)),
    removeEventListener: (name, handler) => parentPort['off'](name, wrapMsgHandler(handler)),
  });
}
#endif // ENVIRONMENT_MAY_BE_NODE

onmessage = (d) => {
  // The first message sent to the Worker is always the bootstrap message.
  // Drop this message listener, it served its purpose of bootstrapping
  // the Wasm Module load, and is no longer needed. Let user code register
  // any desired message handlers from now on.
  /** @suppress {checkTypes} */
  onmessage = null;
#if RUNTIME_DEBUG
  dbg('wasm worker initial onmessage');
#endif
  d = d.data;
#if MINIMAL_RUNTIME
  Module ||= {};
#endif
  /** @suppress {checkTypes} */
  Object.assign(Module, d);
  wasmMemory = d['mem'];
  updateMemoryViews();
#if MINIMAL_RUNTIME
  loadModule()
#else
  wasmModuleReceived(d['wasm']);
#endif
  // Drop now unneeded references to from the Module object in this Worker,
  // these are not needed anymore.
  d['wasm'] = d['mem'] = 0;
}

}
