// Copyright 2024 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

//
// This file and library_webgpu.js together implement <webgpu/webgpu.h>.
//

#include <webgpu/webgpu.h>

#include <array>
#include <cstdlib>
#include <cassert>

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

  static constexpr std::array<WGPUTextureFormat, 3> kSurfaceFormatsRGBAFirst = {
    WGPUTextureFormat_RGBA8Unorm,
    WGPUTextureFormat_BGRA8Unorm,
    WGPUTextureFormat_RGBA16Float,
  };
  static constexpr std::array<WGPUTextureFormat, 3> kSurfaceFormatsBGRAFirst = {
    WGPUTextureFormat_BGRA8Unorm,
    WGPUTextureFormat_RGBA8Unorm,
    WGPUTextureFormat_RGBA16Float,
  };
  WGPUTextureFormat preferredFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
  switch (preferredFormat) {
    case WGPUTextureFormat_RGBA8Unorm:
      capabilities->formatCount = kSurfaceFormatsRGBAFirst.size();
      capabilities->formats = kSurfaceFormatsRGBAFirst.data();
      break;
    case WGPUTextureFormat_BGRA8Unorm:
      capabilities->formatCount = kSurfaceFormatsBGRAFirst.size();
      capabilities->formats = kSurfaceFormatsBGRAFirst.data();
      break;
    default:
      assert(false);
  }

  {
    static constexpr WGPUPresentMode kPresentMode = WGPUPresentMode_Fifo;
    capabilities->presentModeCount = 1;
    capabilities->presentModes = &kPresentMode;
  }

  {
    static constexpr std::array<WGPUCompositeAlphaMode, 2> kAlphaModes = {
      WGPUCompositeAlphaMode_Opaque,
      WGPUCompositeAlphaMode_Premultiplied,
    };
    capabilities->alphaModeCount = kAlphaModes.size();
    capabilities->alphaModes = kAlphaModes.data();
  }
};

// WGPUSurfaceCapabilities

void wgpuSurfaceCapabilitiesFreeMembers(WGPUSurfaceCapabilities value) {
  // wgpuSurfaceCapabilities doesn't currently allocate anything.
}

// WGPUAdapterInfo

void wgpuAdapterInfoFreeMembers(WGPUAdapterInfo value) {
  free(const_cast<char *>(value.vendor));
  free(const_cast<char *>(value.architecture));
  free(const_cast<char *>(value.device));
  free(const_cast<char *>(value.description));
}
