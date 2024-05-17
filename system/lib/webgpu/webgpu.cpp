// Copyright 2024 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

//
// This file and library_webgpu.js together implement <webgpu/webgpu.h>.
//

#include <webgpu/webgpu.h>

#include <array>
#include <cassert>

static constexpr std::array<WGPUTextureFormat, 3>
    kBGRA8UnormPreferredContextFormats = {WGPUTextureFormat_BGRA8Unorm,
                                          WGPUTextureFormat_RGBA8Unorm,
                                          WGPUTextureFormat_RGBA16Float};

static constexpr std::array<WGPUTextureFormat, 3>
    kRGBA8UnormPreferredContextFormats = {WGPUTextureFormat_RGBA8Unorm,
                                          WGPUTextureFormat_BGRA8Unorm,
                                          WGPUTextureFormat_RGBA16Float};

//
// WebGPU function definitions, with methods organized by "class". Note these
// don't need to be extern "C" because they are already declared in webgpu.h.
//

// Standalone (non-method) functions

WGPUInstance wgpuCreateInstance(const WGPUInstanceDescriptor* descriptor) {
  assert(descriptor == nullptr); // descriptor not implemented yet
  return reinterpret_cast<WGPUInstance>(1);
}

// Instance

void wgpuInstanceReference(WGPUInstance) { /* no-op for now */ }
void wgpuInstanceRelease(WGPUInstance) { /* no-op for now */ }

// WGPUSurface

void wgpuSurfaceGetCapabilities(WGPUSurface surface,
                                WGPUAdapter adapter,
                                WGPUSurfaceCapabilities* capabilities) {
  assert(capabilities->nextInChain == nullptr); // TODO: Return WGPUStatus_Error
  WGPUTextureFormat preferredFormat =
      wgpuSurfaceGetPreferredFormat(surface, adapter);
  assert(preferredFormat == WGPUTextureFormat_BGRA8Unorm ||
         preferredFormat == WGPUTextureFormat_RGBA8Unorm);
  capabilities->formatCount = 3;
  if (preferredFormat == WGPUTextureFormat_RGBA8Unorm) {
    capabilities->formats = reinterpret_cast<const WGPUTextureFormat*>(
        kBGRA8UnormPreferredContextFormats.data());
  } else {
    capabilities->formats = reinterpret_cast<const WGPUTextureFormat*>(
        kRGBA8UnormPreferredContextFormats.data());
  }

  // TODO: What do we return for presentModes and alphaModes?
};
