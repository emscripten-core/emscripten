{{{
  // Helper functions for code generation
  globalThis.html5_gpu = {
    makeImportExport: (snake_case, CamelCase) => {
      return `
LibraryHTML5WebGPU.emscripten_webgpu_import_${snake_case}__deps = ['$WebGPU', '$JsValStore'];
LibraryHTML5WebGPU.emscripten_webgpu_import_${snake_case} = (handle) =>
  WebGPU.mgr${CamelCase}.create(JsValStore.get(handle));

LibraryHTML5WebGPU.emscripten_webgpu_export_${snake_case}__deps = ['$WebGPU', '$JsValStore'];
LibraryHTML5WebGPU.emscripten_webgpu_export_${snake_case} = (handle) =>
  JsValStore.add(WebGPU.mgr${CamelCase}.get(handle));`
    },
  };
  null;
}}}


var LibraryHTML5WebGPU = {
  $JsValStore: {
    values: {},
    next_id: 1,

    add(js_val) {
      var id;
      do {
        id = JsValStore.next_id++;
        if (JsValStore.next_id > 2147483647) JsValStore.next_id = 1; // Wraparound signed int32.
      } while (id in JsValStore.values);

      JsValStore.values[id] = js_val;
      return id;
    },
    remove(id) {
#if ASSERTIONS
      assert(id in JsValStore.values);
#endif
      delete JsValStore.values[id];
    },
    get(id) {
#if ASSERTIONS
      assert(id === 0 || id in JsValStore.values);
#endif
      return JsValStore.values[id];
    },
  },

  emscripten_webgpu_release_js_handle__deps: ['$JsValStore'],
  emscripten_webgpu_release_js_handle: (id) => JsValStore.remove(id),

  emscripten_webgpu_get_device__deps: ['$WebGPU'],
  emscripten_webgpu_get_device: () => {
#if ASSERTIONS
    assert(Module['preinitializedWebGPUDevice']);
#endif
    if (WebGPU.preinitializedDeviceId === undefined) {
      var device = Module['preinitializedWebGPUDevice'];
      var deviceWrapper = { queueId: WebGPU.mgrQueue.create(device["queue"]) };
      WebGPU.preinitializedDeviceId = WebGPU.mgrDevice.create(device, deviceWrapper);
    }
    WebGPU.mgrDevice.reference(WebGPU.preinitializedDeviceId);
    return WebGPU.preinitializedDeviceId;
  },
};

{{{ html5_gpu.makeImportExport('surface', 'Surface') }}}
{{{ html5_gpu.makeImportExport('swap_chain', 'SwapChain') }}}

{{{ html5_gpu.makeImportExport('device', 'Device') }}}
{{{ html5_gpu.makeImportExport('queue', 'Queue') }}}

{{{ html5_gpu.makeImportExport('command_buffer', 'CommandBuffer') }}}
{{{ html5_gpu.makeImportExport('command_encoder', 'CommandEncoder') }}}
{{{ html5_gpu.makeImportExport('render_pass_encoder', 'RenderPassEncoder') }}}
{{{ html5_gpu.makeImportExport('compute_pass_encoder', 'ComputePassEncoder') }}}

{{{ html5_gpu.makeImportExport('bind_group', 'BindGroup') }}}
{{{ html5_gpu.makeImportExport('buffer', 'Buffer') }}}
{{{ html5_gpu.makeImportExport('sampler', 'Sampler') }}}
{{{ html5_gpu.makeImportExport('texture', 'Texture') }}}
{{{ html5_gpu.makeImportExport('texture_view', 'TextureView') }}}
{{{ html5_gpu.makeImportExport('query_set', 'QuerySet') }}}

{{{ html5_gpu.makeImportExport('bind_group_layout', 'BindGroupLayout') }}}
{{{ html5_gpu.makeImportExport('pipeline_layout', 'PipelineLayout') }}}
{{{ html5_gpu.makeImportExport('render_pipeline', 'RenderPipeline') }}}
{{{ html5_gpu.makeImportExport('compute_pipeline', 'ComputePipeline') }}}
{{{ html5_gpu.makeImportExport('shader_module', 'ShaderModule') }}}

{{{ html5_gpu.makeImportExport('render_bundle_encoder', 'RenderBundleEncoder') }}}
{{{ html5_gpu.makeImportExport('render_bundle', 'RenderBundle') }}}

addToLibrary(LibraryHTML5WebGPU);
