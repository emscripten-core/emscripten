/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
 * This file and library_webgpu.js together implement <webgpu/webgpu.h>.
 */

#include <webgpu/webgpu.h>

//
// Declarations for JS emwgpu functions (defined in library_webgpu.js)
//

extern "C" {

void emwgpuAddRef(void* id);
void emwgpuRelease(void* id);
void emwgpuSetLabel(void* id, const char* label);
void emwgpuDestroy(void* id);

} // extern "C"

//
// Implementation helpers
//

#define DEFINE_ADDREF_RELEASE(Name) \
  void wgpu##Name##Reference(WGPU##Name o) { emwgpuAddRef(o); } \
  void wgpu##Name##Release(WGPU##Name o) { emwgpuRelease(o); }
#define DEFINE_SETLABEL(Name) \
  void wgpu##Name##SetLabel(WGPU##Name o, const char* label) { emwgpuSetLabel(o, label); }
#define DEFINE_DESTROY(Name) \
  void wgpu##Name##Destroy(WGPU##Name o) { emwgpuDestroy(o); }

//
// WebGPU function definitions, with methods organized by "class". Note these
// don't need to be extern "C" because they are already declared in webgpu.h.
//

// Surface

DEFINE_ADDREF_RELEASE(Surface)

// SwapChain

DEFINE_ADDREF_RELEASE(SwapChain)

// Adapter

DEFINE_ADDREF_RELEASE(Adapter)

// Device

DEFINE_ADDREF_RELEASE(Device)
DEFINE_SETLABEL(Device)
DEFINE_DESTROY(Device)

// Queue

DEFINE_ADDREF_RELEASE(Queue)
DEFINE_SETLABEL(Queue)

// CommandBuffer

DEFINE_ADDREF_RELEASE(CommandBuffer)
DEFINE_SETLABEL(CommandBuffer)

// CommandEncoder

DEFINE_ADDREF_RELEASE(CommandEncoder)
DEFINE_SETLABEL(CommandEncoder)

// RenderPassEncoder

DEFINE_ADDREF_RELEASE(RenderPassEncoder)
DEFINE_SETLABEL(RenderPassEncoder)

// ComputePassEncoder

DEFINE_ADDREF_RELEASE(ComputePassEncoder)
DEFINE_SETLABEL(ComputePassEncoder)

// BindGroup

DEFINE_ADDREF_RELEASE(BindGroup)
DEFINE_SETLABEL(BindGroup)

// Buffer

DEFINE_ADDREF_RELEASE(Buffer)
DEFINE_SETLABEL(Buffer)
// wgpuBufferDestroy is implemented in library_webgpu.js.

// Sampler

DEFINE_ADDREF_RELEASE(Sampler)
DEFINE_SETLABEL(Sampler)

// Texture

DEFINE_ADDREF_RELEASE(Texture)
DEFINE_SETLABEL(Texture)
DEFINE_DESTROY(Texture)

// TextureView

DEFINE_ADDREF_RELEASE(TextureView)
DEFINE_SETLABEL(TextureView)

// QuerySet

DEFINE_ADDREF_RELEASE(QuerySet)
DEFINE_SETLABEL(QuerySet)
DEFINE_DESTROY(QuerySet)

// BindGroupLayout

DEFINE_ADDREF_RELEASE(BindGroupLayout)
DEFINE_SETLABEL(BindGroupLayout)

// PipelineLayout

DEFINE_ADDREF_RELEASE(PipelineLayout)
DEFINE_SETLABEL(PipelineLayout)

// RenderPipeline

DEFINE_ADDREF_RELEASE(RenderPipeline)
DEFINE_SETLABEL(RenderPipeline)

// ComputePipeline

DEFINE_ADDREF_RELEASE(ComputePipeline)
DEFINE_SETLABEL(ComputePipeline)

// ShaderModule

DEFINE_ADDREF_RELEASE(ShaderModule)
DEFINE_SETLABEL(ShaderModule)

// RenderBundleEncoder

DEFINE_ADDREF_RELEASE(RenderBundleEncoder)
DEFINE_SETLABEL(RenderBundleEncoder)

// RenderBundle

DEFINE_ADDREF_RELEASE(RenderBundle)
DEFINE_SETLABEL(RenderBundle)
