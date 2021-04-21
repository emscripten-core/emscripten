{{{ (function() {
  // Helper functions for code generation
  global.html5_gpu = {
    makeImportExport: function(snake_case, CamelCase) {
      var s = '';
      s += 'LibraryHTML5WebGPU.emscripten_webgpu_import_' + snake_case + '__sig = "ii";';
      s += 'LibraryHTML5WebGPU.emscripten_webgpu_import_' + snake_case + '__deps = ["$WebGPU", "$JsValStore"];';
      s += 'LibraryHTML5WebGPU.emscripten_webgpu_import_' + snake_case + ' = function(handle) { '
      s += 'return WebGPU["mgr' + CamelCase + '"].create(JsValStore.get(handle));'
      s += '};';

      s += 'LibraryHTML5WebGPU.emscripten_webgpu_export_' + snake_case + '__sig = "ii";';
      s += 'LibraryHTML5WebGPU.emscripten_webgpu_export_' + snake_case + '__deps = ["$WebGPU", "$JsValStore"];';
      s += 'LibraryHTML5WebGPU.emscripten_webgpu_export_' + snake_case + ' = function(handle) { '
      s += 'return JsValStore.add(WebGPU["mgr' + CamelCase + '"].get(handle));'
      s += '};';
      return s;
    },
  };
  return null;
})(); }}}


var LibraryHTML5WebGPU = {
  // TODO(kainino0x): make it possible to actually create devices through webgpu.h
  emscripten_webgpu_get_device__deps: ['$WebGPU'],
  emscripten_webgpu_get_device: function() {
#if ASSERTIONS
    assert(Module['preinitializedWebGPUDevice']);
#endif
    return WebGPU["mgrDevice"].create(Module['preinitializedWebGPUDevice']);
  },
};

{{{ html5_gpu.makeImportExport('surface', 'Surface') }}}
{{{ html5_gpu.makeImportExport('swap_chain', 'SwapChain') }}}

{{{ html5_gpu.makeImportExport('device', 'Device') }}}
{{{ html5_gpu.makeImportExport('queue', 'Queue') }}}
{{{ html5_gpu.makeImportExport('fence', 'Fence') }}}

{{{ html5_gpu.makeImportExport('command_buffer', 'CommandBuffer') }}}
{{{ html5_gpu.makeImportExport('command_encoder', 'CommandEncoder') }}}
{{{ html5_gpu.makeImportExport('render_passEncoder', 'RenderPassEncoder') }}}
{{{ html5_gpu.makeImportExport('compute_passEncoder', 'ComputePassEncoder') }}}

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

mergeInto(LibraryManager.library, LibraryHTML5WebGPU);
