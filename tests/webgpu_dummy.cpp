// Copyright 2020 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Make sure this header exists and compiles
// (webgpu_cpp.h includes webgpu.h so that's tested too).
#include <webgpu/webgpu_cpp.h>

#include <emscripten.h>
#include <emscripten/html5_webgpu.h>

EM_JS(struct EmJsHandle*, init_js_device, (), {
  return Asyncify.handleAsync(async () => {
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter.requestDevice();
    return JsValStore.add(device);
  });
});

wgpu::Device init_device() {
  struct EmJsHandle* deviceHandle = init_js_device();
  wgpu::Device device = wgpu::Device::Acquire(emscripten_webgpu_import_device(deviceHandle));
  emscripten_unwrap_js_handle(deviceHandle);
  return device;
}

int main() {
  wgpu::Device device = init_device();

  wgpu::BufferDescriptor desc = {};
  desc.size = 4;
  desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
  wgpu::Buffer buffer = device.CreateBuffer(&desc);

  struct EmJsHandle* bufferHandle = emscripten_webgpu_export_buffer(buffer.Get());
  EM_ASM(
    {
      const b = $0;
      b.mapAsync(GPUMapMode.WRITE).then(() => {
        console.log('Mapping length', b.getMappedRange().byteLength);
        b.unmap();
      });
    }, bufferHandle);

  struct EmJsHandle* deviceHandle = emscripten_webgpu_export_device(device.Get());
  struct EmJsHandle* textureHandle = EM_ASM_JS_VAL(
    {
      const device = $0;
      const t = device.createTexture({
        size : [ 16, 16 ],
        usage : GPUTextureUsage.COPY_DST,
        format : 'rgba8unorm',
      });
      return t;
    },
    deviceHandle);

  struct EmJsHandle* canvasHandle = EM_ASM_JS_VAL({ return document.createElement('canvas'); });
  EM_ASM(
    {
      const device = $0;
      const canvas = $1;
      const texture = $2;
      console.log('Copy', canvas, 'to', texture, 'with', device);
    }, deviceHandle, canvasHandle, textureHandle);

  // TODO: Make managed wrappers for these so C++ code doesn't need to manually free.
  emscripten_unwrap_js_handle(bufferHandle);
  emscripten_unwrap_js_handle(deviceHandle);
  emscripten_unwrap_js_handle(canvasHandle);
  emscripten_unwrap_js_handle(textureHandle);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
