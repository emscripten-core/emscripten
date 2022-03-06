// N.B. The contents of this file are duplicated in src/library_wasm_worker.js
// in variable "_wasmWorkerBlobUrl" (where the contents are pre-minified) If
// doing any changes to this file, be sure to update the contents there too.
onmessage = function(d) {
	// The first message sent to the Worker is always the bootstrap message.
	// Drop this message listener, it served its purpose of bootstrapping
	// the Wasm Module load, and is no longer needed. Let user code register
	// any desired message handlers from now on.
	onmessage = null;
	d = d.data;
#if !MODULARIZE
	self.{{{ EXPORT_NAME }}} = d;
#endif
#if !MINIMAL_RUNTIME
	d['instantiateWasm'] = (info, receiveInstance) => { var instance = new WebAssembly.Instance(d['wasm'], info); receiveInstance(instance, d['wasm']); return instance.exports; }
#endif
	importScripts(d.js);
#if MODULARIZE
	{{{ EXPORT_NAME }}}(d);
#endif
	// Drop now unneeded references to from the Module object in this Worker,
	// these are not needed anymore.
	d.wasm = d.mem = d.js = 0;
}
