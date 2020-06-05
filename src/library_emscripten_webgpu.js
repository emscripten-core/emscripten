mergeInto(LibraryManager.library, {
  // TODO(kainino0x): make it possible to actually create devices through webgpu.h
  emscripten_webgpu_get_device__deps: ['$WebGPU'],
  emscripten_webgpu_get_device__postset: 'WebGPU.initManagers();',
  emscripten_webgpu_get_device: function() {
#if ASSERTIONS
    assert(Module['preinitializedWebGPUDevice']);
#endif
    return WebGPU["mgrDevice"].create(Module['preinitializedWebGPUDevice']);
  },
});
