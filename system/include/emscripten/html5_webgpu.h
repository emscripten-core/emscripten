/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <webgpu/webgpu.h>

#ifdef __cplusplus
extern "C" {
#endif

WGPUDevice emscripten_webgpu_get_device(void);

void emscripten_webgpu_release_js_handle(int js_handle);

#define WEBGPU_MAKE_IMPORT_EXPORT(snake_case, CamelCase)      \
  WGPU##CamelCase emscripten_webgpu_import_##snake_case(int); \
  int emscripten_webgpu_export_##snake_case(WGPU##CamelCase);

WEBGPU_MAKE_IMPORT_EXPORT(surface, Surface)
WEBGPU_MAKE_IMPORT_EXPORT(swap_chain, SwapChain)

WEBGPU_MAKE_IMPORT_EXPORT(device, Device)
WEBGPU_MAKE_IMPORT_EXPORT(queue, Queue)

WEBGPU_MAKE_IMPORT_EXPORT(command_buffer, CommandBuffer)
WEBGPU_MAKE_IMPORT_EXPORT(command_encoder, CommandEncoder)
WEBGPU_MAKE_IMPORT_EXPORT(render_pass_encoder, RenderPassEncoder)
WEBGPU_MAKE_IMPORT_EXPORT(compute_pass_encoder, ComputePassEncoder)

WEBGPU_MAKE_IMPORT_EXPORT(bind_group, BindGroup)
WEBGPU_MAKE_IMPORT_EXPORT(buffer, Buffer)
WEBGPU_MAKE_IMPORT_EXPORT(sampler, Sampler)
WEBGPU_MAKE_IMPORT_EXPORT(texture, Texture)
WEBGPU_MAKE_IMPORT_EXPORT(texture_view, TextureView)
WEBGPU_MAKE_IMPORT_EXPORT(query_set, QuerySet)

WEBGPU_MAKE_IMPORT_EXPORT(bind_group_layout, BindGroupLayout)
WEBGPU_MAKE_IMPORT_EXPORT(pipeline_layout, PipelineLayout)
WEBGPU_MAKE_IMPORT_EXPORT(render_pipeline, RenderPipeline)
WEBGPU_MAKE_IMPORT_EXPORT(compute_pipeline, ComputePipeline)
WEBGPU_MAKE_IMPORT_EXPORT(shader_module, ShaderModule)

WEBGPU_MAKE_IMPORT_EXPORT(render_bundle_encoder, RenderBundleEncoder)
WEBGPU_MAKE_IMPORT_EXPORT(render_bundle, RenderBundle)

#undef WEBGPU_MAKE_IMPORT_EXPORT

#ifdef __cplusplus
} // ~extern "C"
#endif
