#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <webgpu/webgpu.h>

WGPUSupportedLimits adapter_supported_limits = {
  0,
};

void assertLimitsCompatible(WGPULimits required_limits,
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
  ASSERT_LIMITS_COMPATIBLE(maxInterStageShaderComponents);
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

void on_device_request_ended(WGPURequestDeviceStatus status,
                             WGPUDevice device,
                             char const* message,
                             void* userdata) {
  assert(status == WGPURequestDeviceStatus_Success);

  WGPUSupportedLimits device_supported_limits;
  wgpuDeviceGetLimits(device, &device_supported_limits);

  // verify that the obtained device fullfils required limits 
  assertLimitsCompatible(adapter_supported_limits.limits,
                         device_supported_limits.limits);
}

void on_adapter_request_ended(WGPURequestAdapterStatus status,
                              WGPUAdapter adapter,
                              char const* message,
                              void* userdata) {
  assert(status == WGPURequestAdapterStatus_Success);

  wgpuAdapterGetLimits(adapter, &adapter_supported_limits);

  // for device limits, require the limits supported by adapter
  WGPURequiredLimits device_required_limits = {0,};
  device_required_limits.limits = adapter_supported_limits.limits;

  WGPUDeviceDescriptor device_desc = {0,};
  device_desc.requiredFeatureCount = 0;
  device_desc.requiredLimits = &device_required_limits;
  wgpuAdapterRequestDevice(adapter, &device_desc, on_device_request_ended, NULL);
}

int main() {
  const WGPUInstance instance = wgpuCreateInstance(NULL);

  WGPURequestAdapterOptions adapter_options = {0,};
  wgpuInstanceRequestAdapter(instance, &adapter_options, on_adapter_request_ended, NULL);

  // This code is returned when the runtime exits unless something else sets
  // it, like exit(0).
  return 99;
}
