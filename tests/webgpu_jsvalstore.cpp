// Copyright 2020 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Make sure this header exists and compiles
// (webgpu_cpp.h includes webgpu.h so that's tested too).
#include <webgpu/webgpu_cpp.h>

#include <emscripten.h>
#include <emscripten/html5_webgpu.h>

class EmJsHandle {
public:
  EmJsHandle() : mHandle(0) {}
  EmJsHandle(int handle) : mHandle(handle) {}
  ~EmJsHandle() {
    if (mHandle != 0) {
      emscripten_webgpu_release_js_handle(mHandle);
    }
  }

  EmJsHandle(const EmJsHandle&) = delete;
  EmJsHandle& operator=(const EmJsHandle&) = delete;

  EmJsHandle(EmJsHandle&& rhs) : mHandle(rhs.mHandle) { rhs.mHandle = 0; }

  EmJsHandle& operator=(EmJsHandle&& rhs) {
    int tmp = rhs.mHandle;
    rhs.mHandle = this->mHandle;
    this->mHandle = tmp;
    return *this;
  }

  int Get() { return mHandle; }

private:
  int mHandle;
};

EM_ASYNC_JS(int, init_js_device, (), {
  const adapter = await navigator.gpu.requestAdapter();
  const device = await adapter.requestDevice();
  return JsValStore.add(device);
});

wgpu::Device init_device() {
  EmJsHandle deviceHandle = EmJsHandle(init_js_device());
  wgpu::Device device = wgpu::Device::Acquire(emscripten_webgpu_import_device(deviceHandle.Get()));
  return device;
}

int main() {
  wgpu::Device device = init_device();

  wgpu::BufferDescriptor desc = {};
  desc.size = 4;
  desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
  wgpu::Buffer buffer = device.CreateBuffer(&desc);

  EmJsHandle bufferHandle = EmJsHandle(emscripten_webgpu_export_buffer(buffer.Get()));
  EM_ASM(
    {
      const b = JsValStore.get($0);
      b.mapAsync(GPUMapMode.WRITE).then(() => {
        console.log('Mapping length', b.getMappedRange().byteLength);
        b.unmap();
      });
    }, bufferHandle.Get());

  EmJsHandle deviceHandle = EmJsHandle(emscripten_webgpu_export_device(device.Get()));
  EmJsHandle textureHandle = EmJsHandle(EM_ASM_INT(
    {
      const device = JsValStore.get($0);
      const t = device.createTexture({
        size : [ 16, 16 ],
        usage : GPUTextureUsage.COPY_DST,
        format : 'rgba8unorm',
      });
      return JsValStore.add(t);
    },
    deviceHandle.Get()));

  EmJsHandle canvasHandle =
    EmJsHandle(EM_ASM_INT({ return JsValStore.add(document.createElement('canvas')); }));
  EM_ASM(
    {
      const device = JsValStore.get($0);
      const canvas = JsValStore.get($1);
      const texture = JsValStore.get($2);
      console.log('Copy', canvas, 'to', texture, 'with', device);
    },
    deviceHandle.Get(), canvasHandle.Get(), textureHandle.Get());

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
