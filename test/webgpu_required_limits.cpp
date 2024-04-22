#include <cassert>
#include <emscripten.h>
#include <iostream>
#include <webgpu/webgpu.h>

int adapter_limit_maxColorAttachmentBytesPerSample = -1;

EM_ASYNC_JS(int, get_limit, (), {
  if (!navigator.gpu) {
    throw Error("WebGPU not supported.");
  }

  const adapter = await navigator.gpu.requestAdapter();
  if (!adapter) {
    throw Error("Couldn't request WebGPU adapter.");
  }

  console.log("adapter limits", adapter.limits);

  return adapter.limits.maxColorAttachmentBytesPerSample;
});

void on_device_request_ended(WGPURequestDeviceStatus status,
                             WGPUDevice device,
                             char const* message,
                             void* userdata) {
  assert(status == WGPURequestDeviceStatus::WGPURequestDeviceStatus_Success);

  WGPUSupportedLimits device_supported_limits{};
  wgpuDeviceGetLimits(device, &device_supported_limits);

  std::cout << "required maxColorAttachmentBytesPerSample="
            << adapter_limit_maxColorAttachmentBytesPerSample << std::endl;
  std::cout << "supported maxColorAttachmentBytesPerSample="
            << device_supported_limits.limits.maxColorAttachmentBytesPerSample
            << std::endl;

  // device supported limit MUST be equal or larger than what was requested,
  // otherwise device acquisition should have failed
  assert(device_supported_limits.limits.maxColorAttachmentBytesPerSample >=
         adapter_limit_maxColorAttachmentBytesPerSample);
}

void on_adapter_request_ended(WGPURequestAdapterStatus status,
                              WGPUAdapter adapter,
                              char const* message,
                              void* userdata) {
  assert(status == WGPURequestAdapterStatus::WGPURequestAdapterStatus_Success);

  // retrieving limits supported by adapter - currently unsupported!
  // WGPUSupportedLimits adapter_supported_limits {};
  // wgpuAdapterGetLimits(adapter, &adapter_supported_limits);
  // adapter_limit_maxColorAttachmentBytesPerSample =
  // adapter_supported_limits.limits.maxColorAttachmentBytesPerSample;

  // use js callout instead
  adapter_limit_maxColorAttachmentBytesPerSample = get_limit();
  std::cout << "adapter supports maxColorAttachmentBytesPerSample="
            << adapter_limit_maxColorAttachmentBytesPerSample << std::endl;

  // if max supported limit is the default, we cant check if requesting more works
  if (adapter_limit_maxColorAttachmentBytesPerSample == 32) {
    exit(0);
  }

  WGPURequiredLimits device_required_limits{};
  device_required_limits.limits.minStorageBufferOffsetAlignment =
    256; // irrelevant but needs to be set
  device_required_limits.limits.minUniformBufferOffsetAlignment =
    256; // irrelevant but needs to be set

  // requesting adapter supported limit
  device_required_limits.limits.maxColorAttachmentBytesPerSample =
    adapter_limit_maxColorAttachmentBytesPerSample;

  WGPUDeviceDescriptor device_desc{};
  device_desc.requiredFeatureCount = 0;
  device_desc.requiredLimits = &device_required_limits;

  wgpuAdapterRequestDevice(
    adapter, &device_desc, on_device_request_ended, nullptr);
}

int main() {
  const WGPUInstance instance = wgpuCreateInstance(nullptr);

  WGPURequestAdapterOptions adapter_options{};
  wgpuInstanceRequestAdapter(
    instance, &adapter_options, on_adapter_request_ended, nullptr);

  return 0;
}
