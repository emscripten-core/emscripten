#include <emscripten/em_asm.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu.h>
#include <emscripten/em_math.h>

WGPUInstance instance;
WGPUDevice device;
WGPUQueue queue;
WGPUSwapChain swapChain;

void preferred_texture_callback(WGPUTextureFormat format, void* userdata)
{
  *(WGPUTextureFormat*)userdata = format;
}

double hue2color(double hue)
{
  hue = emscripten_math_fmod(hue, 1.0);
  if (hue < 1.0 / 6.0) return 6.0 * hue;
  if (hue < 1.0 / 2.0) return 1;
  if (hue < 2.0 / 3.0) return 4.0 - 6.0 * hue;
  return 0;
}

EM_BOOL raf(double time, void *userData)
{
  WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, 0);

  double hue = time * 0.00005;

  WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder,
    &(WGPURenderPassDescriptor) {
        .colorAttachments = &(WGPURenderPassColorAttachment) {
            .view = wgpuSwapChainGetCurrentTextureView(swapChain),
            .loadOp = WGPULoadOp_Clear,
            .storeOp = WGPUStoreOp_Store,
            .clearColor = (WGPUColor) {
                .r = hue2color(hue + 1.0 / 3.0),
                .g = hue2color(hue),
                .b = hue2color(hue - 1.0 / 3.0),
                .a = 1.0,
            },
        },
        .colorAttachmentCount = 1,
    });

  wgpuRenderPassEncoderEndPass(pass);

  WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(encoder, &(WGPUCommandBufferDescriptor){});
  wgpuQueueSubmit(queue, 1, &cmdBuffer);
  wgpuSwapChainPresent(swapChain);

  return EM_TRUE;
}

void run()
{
  device = emscripten_webgpu_get_device();
  queue = wgpuDeviceGetQueue(device);

  WGPUSurfaceDescriptor surfaceDesc = {};
  surfaceDesc.label = "#canvas";
  WGPUSurface surface = wgpuInstanceCreateSurface(instance, &surfaceDesc);

  WGPUTextureFormat swapChainFormat;
  //wgpuSurfaceGetPreferredFormat(surface, 0/*adapter*/, preferred_texture_callback, &swapChainFormat);
  // wgpuSurfaceGetPreferredFormat() above does not yet exist, so hardcode preferred format.
  swapChainFormat = WGPUTextureFormat_RGBA8Unorm;

  int canvasWidth, canvasHeight;
  emscripten_get_canvas_element_size(surfaceDesc.label, &canvasWidth, &canvasHeight);

  swapChain = wgpuDeviceCreateSwapChain(device, surface, &(WGPUSwapChainDescriptor){
    .usage = WGPUTextureUsage_RenderAttachment,
    .format = swapChainFormat,
    .width = canvasWidth,
    .height = canvasHeight,
    .presentMode = WGPUPresentMode_Fifo,
  });

  emscripten_request_animation_frame_loop(raf, 0);
}

int main(int argc, char **argv)
{
  // wgpuCreateInstance() does not yet exist, so just use instance == 0 ptr. 
  // instance = wgpuCreateInstance();

  // wgpuInstanceRequestAdapter() and wgpuAdapterRequestDevice() do not yet
  // exist, but we need to manually initialize a WebGPU device into
  // Module['preinitializedWebGPUDevice'].
  EM_ASM({
    navigator['gpu']['requestAdapter']().then(adapter => {
      adapter['requestDevice']().then(device => {
        Module['preinitializedWebGPUDevice'] = device;
        wasmTable.get($0)();
      });
    });
  }, &run);
}
