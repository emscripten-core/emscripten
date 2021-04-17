mergeInto(LibraryManager.library, {
  // TODO(kainino0x): make it possible to actually create devices through webgpu.h
  emscripten_webgpu_get_device__deps: ['$WebGPU'],
  emscripten_webgpu_get_device: function() {
#if ASSERTIONS
    assert(Module['preinitializedWebGPUDevice']);
#endif
    return WebGPU["mgrDevice"].create(Module['preinitializedWebGPUDevice']);
  },

  emscripten_webgpu_import_device__sig: 'ii',
  emscripten_webgpu_import_device__deps: ['$WebGPU', '$JsValStore'],
  emscripten_webgpu_import_device: function (handle) {
    return WebGPU["mgrDevice"].create(JsValStore.get(handle));
  },

  emscripten_webgpu_export_device__sig: 'ii',
  emscripten_webgpu_export_device__deps: ['$WebGPU', '$JsValStore'],
  emscripten_webgpu_export_device: function (handle) {
    return JsValStore.add(WebGPU["mgrDevice"].get(handle));
  },

  emscripten_webgpu_import_buffer__sig: 'ii',
  emscripten_webgpu_import_buffer__deps: ['$WebGPU', '$JsValStore'],
  emscripten_webgpu_import_buffer: function (handle) {
    return WebGPU["mgrBuffer"].create(JsValStore.get(handle));
  },

  emscripten_webgpu_export_buffer__sig: 'ii',
  emscripten_webgpu_export_buffer__deps: ['$WebGPU', '$JsValStore'],
  emscripten_webgpu_export_buffer: function (handle) {
    return JsValStore.add(WebGPU["mgrBuffer"].get(handle));
  },
});
