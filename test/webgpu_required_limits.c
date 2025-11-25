#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <webgpu/webgpu.h>

static WGPULimits adapter_supported_limits = {0};

static void assertLimitsCompatible(WGPULimits required_limits,
                                   WGPULimits supported_limits) {
#define ASSERT_LIMITS_COMPATIBLE(limitName)                                    \
  assert(required_limits.limitName == supported_limits.limitName)
  ASSERT_LIMITS_COMPATIBLE(maxTextureDimension1D);
  ASSERT_LIMITS_COMPATIBLE(maxTextureDimension2D);
  ASSERT_LIMITS_COMPATIBLE(maxTextureDimension3D);
  ASSERT_LIMITS_COMPATIBLE(maxTextureArrayLayers);
  ASSERT_LIMITS_COMPATIBLE(maxBindGroups);
  ASSERT_LIMITS_COMPATIBLE(maxBindGroupsPlusVertexBuffers);
  ASSERT_LIMITS_COMPATIBLE(maxBindingsPerBindGroup);
  ASSERT_LIMITS_COMPATIBLE(maxDynamicUniformBuffersPerPipelineLayout);
  ASSERT_LIMITS_COMPATIBLE(maxDynamicStorageBuffersPerPipelineLayout);
  ASSERT_LIMITS_COMPATIBLE(maxSampledTexturesPerShaderStage);
  ASSERT_LIMITS_COMPATIBLE(maxSamplersPerShaderStage);
  ASSERT_LIMITS_COMPATIBLE(maxStorageBuffersPerShaderStage);
  ASSERT_LIMITS_COMPATIBLE(maxStorageTexturesPerShaderStage);
  ASSERT_LIMITS_COMPATIBLE(maxUniformBuffersPerShaderStage);
  ASSERT_LIMITS_COMPATIBLE(minUniformBufferOffsetAlignment);
  ASSERT_LIMITS_COMPATIBLE(minStorageBufferOffsetAlignment);
  ASSERT_LIMITS_COMPATIBLE(maxUniformBufferBindingSize);
  ASSERT_LIMITS_COMPATIBLE(maxStorageBufferBindingSize);
  ASSERT_LIMITS_COMPATIBLE(maxVertexBuffers);
  ASSERT_LIMITS_COMPATIBLE(maxBufferSize);
  ASSERT_LIMITS_COMPATIBLE(maxVertexAttributes);
  ASSERT_LIMITS_COMPATIBLE(maxVertexBufferArrayStride);
  ASSERT_LIMITS_COMPATIBLE(maxInterStageShaderVariables);
  ASSERT_LIMITS_COMPATIBLE(maxColorAttachments);
  ASSERT_LIMITS_COMPATIBLE(maxColorAttachmentBytesPerSample);
  ASSERT_LIMITS_COMPATIBLE(maxComputeWorkgroupStorageSize);
  ASSERT_LIMITS_COMPATIBLE(maxComputeInvocationsPerWorkgroup);
  ASSERT_LIMITS_COMPATIBLE(maxComputeWorkgroupSizeX);
  ASSERT_LIMITS_COMPATIBLE(maxComputeWorkgroupSizeY);
  ASSERT_LIMITS_COMPATIBLE(maxComputeWorkgroupSizeZ);
  ASSERT_LIMITS_COMPATIBLE(maxComputeWorkgroupsPerDimension);
#undef ASSERT_LIMITS_COMPATIBLE
}

static void on_device_request_ended(WGPURequestDeviceStatus status,
                                    WGPUDevice device,
                                    WGPUStringView message,
                                    void* userdata1, void* userdata2) {
  assert(status == WGPURequestDeviceStatus_Success);

  WGPULimits device_supported_limits = {0};
  wgpuDeviceGetLimits(device, &device_supported_limits);

  // verify that the obtained device fullfils required limits 
  assertLimitsCompatible(adapter_supported_limits,
                         device_supported_limits);

  wgpuDeviceRelease(device);
  exit(0);
}

static void on_adapter_request_ended(WGPURequestAdapterStatus status,
                                     WGPUAdapter adapter,
                                     WGPUStringView message,
                                     void* userdata1, void* userdata2) {
  assert(status == WGPURequestAdapterStatus_Success);

  wgpuAdapterGetLimits(adapter, &adapter_supported_limits);

  WGPUDeviceDescriptor device_desc = {
    // for device limits, require the limits supported by adapter
    .requiredLimits = &adapter_supported_limits,
  };
  wgpuAdapterRequestDevice(adapter, &device_desc, (WGPURequestDeviceCallbackInfo){
    .mode = WGPUCallbackMode_AllowSpontaneous,
    .callback = on_device_request_ended,
  });

  wgpuAdapterRelease(adapter);
}

int main() {
  const WGPUInstance instance = wgpuCreateInstance(NULL);

  wgpuInstanceRequestAdapter(instance, NULL, (WGPURequestAdapterCallbackInfo){
    .mode = WGPUCallbackMode_AllowSpontaneous,
    .callback = on_adapter_request_ended,
  });

  // This code is returned when the runtime exits unless something else sets
  // it, like exit(0).
  return 99;
}
