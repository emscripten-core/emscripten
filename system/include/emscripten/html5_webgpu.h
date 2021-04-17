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

typedef struct WGPUDeviceImpl* WGPUDevice;
typedef struct WGPUBufferImpl* WGPUBuffer;

WGPUDevice emscripten_webgpu_get_device(void);

WGPUDevice emscripten_webgpu_import_device(struct EmJsHandle*);
struct EmJsHandle* emscripten_webgpu_export_device(WGPUDevice);

WGPUBuffer emscripten_webgpu_import_buffer(struct EmJsHandle*);
struct EmJsHandle* emscripten_webgpu_export_buffer(WGPUBuffer);

#ifdef __cplusplus
} // ~extern "C"
#endif
