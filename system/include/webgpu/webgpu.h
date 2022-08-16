// BSD 3-Clause License
//
// Copyright (c) 2019, "WebGPU native" developers
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef WEBGPU_H_
#define WEBGPU_H_

#if defined(WGPU_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(WGPU_IMPLEMENTATION)
#            define WGPU_EXPORT __declspec(dllexport)
#        else
#            define WGPU_EXPORT __declspec(dllimport)
#        endif
#    else  // defined(_WIN32)
#        if defined(WGPU_IMPLEMENTATION)
#            define WGPU_EXPORT __attribute__((visibility("default")))
#        else
#            define WGPU_EXPORT
#        endif
#    endif  // defined(_WIN32)
#else       // defined(WGPU_SHARED_LIBRARY)
#    define WGPU_EXPORT
#endif  // defined(WGPU_SHARED_LIBRARY)

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define WGPU_ARRAY_LAYER_COUNT_UNDEFINED (0xffffffffUL)
#define WGPU_COPY_STRIDE_UNDEFINED (0xffffffffUL)
#define WGPU_LIMIT_U32_UNDEFINED (0xffffffffUL)
#define WGPU_LIMIT_U64_UNDEFINED (0xffffffffffffffffULL)
#define WGPU_MIP_LEVEL_COUNT_UNDEFINED (0xffffffffUL)
#define WGPU_WHOLE_MAP_SIZE SIZE_MAX
#define WGPU_WHOLE_SIZE (0xffffffffffffffffULL)

typedef uint32_t WGPUFlags;

typedef struct WGPUAdapterImpl* WGPUAdapter;
typedef struct WGPUBindGroupImpl* WGPUBindGroup;
typedef struct WGPUBindGroupLayoutImpl* WGPUBindGroupLayout;
typedef struct WGPUBufferImpl* WGPUBuffer;
typedef struct WGPUCommandBufferImpl* WGPUCommandBuffer;
typedef struct WGPUCommandEncoderImpl* WGPUCommandEncoder;
typedef struct WGPUComputePassEncoderImpl* WGPUComputePassEncoder;
typedef struct WGPUComputePipelineImpl* WGPUComputePipeline;
typedef struct WGPUDeviceImpl* WGPUDevice;
typedef struct WGPUInstanceImpl* WGPUInstance;
typedef struct WGPUPipelineLayoutImpl* WGPUPipelineLayout;
typedef struct WGPUQuerySetImpl* WGPUQuerySet;
typedef struct WGPUQueueImpl* WGPUQueue;
typedef struct WGPURenderBundleImpl* WGPURenderBundle;
typedef struct WGPURenderBundleEncoderImpl* WGPURenderBundleEncoder;
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder;
typedef struct WGPURenderPipelineImpl* WGPURenderPipeline;
typedef struct WGPUSamplerImpl* WGPUSampler;
typedef struct WGPUShaderModuleImpl* WGPUShaderModule;
typedef struct WGPUSurfaceImpl* WGPUSurface;
typedef struct WGPUSwapChainImpl* WGPUSwapChain;
typedef struct WGPUTextureImpl* WGPUTexture;
typedef struct WGPUTextureViewImpl* WGPUTextureView;

typedef enum WGPUAdapterType {
    WGPUAdapterType_DiscreteGPU = 0x00000000,
    WGPUAdapterType_IntegratedGPU = 0x00000001,
    WGPUAdapterType_CPU = 0x00000002,
    WGPUAdapterType_Unknown = 0x00000003,
    WGPUAdapterType_Force32 = 0x7FFFFFFF
} WGPUAdapterType;

typedef enum WGPUAddressMode {
    WGPUAddressMode_Repeat = 0x00000000,
    WGPUAddressMode_MirrorRepeat = 0x00000001,
    WGPUAddressMode_ClampToEdge = 0x00000002,
    WGPUAddressMode_Force32 = 0x7FFFFFFF
} WGPUAddressMode;

typedef enum WGPUBackendType {
    WGPUBackendType_Null = 0x00000000,
    WGPUBackendType_WebGPU = 0x00000001,
    WGPUBackendType_D3D11 = 0x00000002,
    WGPUBackendType_D3D12 = 0x00000003,
    WGPUBackendType_Metal = 0x00000004,
    WGPUBackendType_Vulkan = 0x00000005,
    WGPUBackendType_OpenGL = 0x00000006,
    WGPUBackendType_OpenGLES = 0x00000007,
    WGPUBackendType_Force32 = 0x7FFFFFFF
} WGPUBackendType;

typedef enum WGPUBlendFactor {
    WGPUBlendFactor_Zero = 0x00000000,
    WGPUBlendFactor_One = 0x00000001,
    WGPUBlendFactor_Src = 0x00000002,
    WGPUBlendFactor_OneMinusSrc = 0x00000003,
    WGPUBlendFactor_SrcAlpha = 0x00000004,
    WGPUBlendFactor_OneMinusSrcAlpha = 0x00000005,
    WGPUBlendFactor_Dst = 0x00000006,
    WGPUBlendFactor_OneMinusDst = 0x00000007,
    WGPUBlendFactor_DstAlpha = 0x00000008,
    WGPUBlendFactor_OneMinusDstAlpha = 0x00000009,
    WGPUBlendFactor_SrcAlphaSaturated = 0x0000000A,
    WGPUBlendFactor_Constant = 0x0000000B,
    WGPUBlendFactor_OneMinusConstant = 0x0000000C,
    WGPUBlendFactor_Force32 = 0x7FFFFFFF
} WGPUBlendFactor;

typedef enum WGPUBlendOperation {
    WGPUBlendOperation_Add = 0x00000000,
    WGPUBlendOperation_Subtract = 0x00000001,
    WGPUBlendOperation_ReverseSubtract = 0x00000002,
    WGPUBlendOperation_Min = 0x00000003,
    WGPUBlendOperation_Max = 0x00000004,
    WGPUBlendOperation_Force32 = 0x7FFFFFFF
} WGPUBlendOperation;

typedef enum WGPUBufferBindingType {
    WGPUBufferBindingType_Undefined = 0x00000000,
    WGPUBufferBindingType_Uniform = 0x00000001,
    WGPUBufferBindingType_Storage = 0x00000002,
    WGPUBufferBindingType_ReadOnlyStorage = 0x00000003,
    WGPUBufferBindingType_Force32 = 0x7FFFFFFF
} WGPUBufferBindingType;

typedef enum WGPUBufferMapAsyncStatus {
    WGPUBufferMapAsyncStatus_Success = 0x00000000,
    WGPUBufferMapAsyncStatus_Error = 0x00000001,
    WGPUBufferMapAsyncStatus_Unknown = 0x00000002,
    WGPUBufferMapAsyncStatus_DeviceLost = 0x00000003,
    WGPUBufferMapAsyncStatus_DestroyedBeforeCallback = 0x00000004,
    WGPUBufferMapAsyncStatus_UnmappedBeforeCallback = 0x00000005,
    WGPUBufferMapAsyncStatus_Force32 = 0x7FFFFFFF
} WGPUBufferMapAsyncStatus;

typedef enum WGPUCompareFunction {
    WGPUCompareFunction_Undefined = 0x00000000,
    WGPUCompareFunction_Never = 0x00000001,
    WGPUCompareFunction_Less = 0x00000002,
    WGPUCompareFunction_LessEqual = 0x00000003,
    WGPUCompareFunction_Greater = 0x00000004,
    WGPUCompareFunction_GreaterEqual = 0x00000005,
    WGPUCompareFunction_Equal = 0x00000006,
    WGPUCompareFunction_NotEqual = 0x00000007,
    WGPUCompareFunction_Always = 0x00000008,
    WGPUCompareFunction_Force32 = 0x7FFFFFFF
} WGPUCompareFunction;

typedef enum WGPUCompilationInfoRequestStatus {
    WGPUCompilationInfoRequestStatus_Success = 0x00000000,
    WGPUCompilationInfoRequestStatus_Error = 0x00000001,
    WGPUCompilationInfoRequestStatus_DeviceLost = 0x00000002,
    WGPUCompilationInfoRequestStatus_Unknown = 0x00000003,
    WGPUCompilationInfoRequestStatus_Force32 = 0x7FFFFFFF
} WGPUCompilationInfoRequestStatus;

typedef enum WGPUCompilationMessageType {
    WGPUCompilationMessageType_Error = 0x00000000,
    WGPUCompilationMessageType_Warning = 0x00000001,
    WGPUCompilationMessageType_Info = 0x00000002,
    WGPUCompilationMessageType_Force32 = 0x7FFFFFFF
} WGPUCompilationMessageType;

typedef enum WGPUComputePassTimestampLocation {
    WGPUComputePassTimestampLocation_Beginning = 0x00000000,
    WGPUComputePassTimestampLocation_End = 0x00000001,
    WGPUComputePassTimestampLocation_Force32 = 0x7FFFFFFF
} WGPUComputePassTimestampLocation;

typedef enum WGPUCreatePipelineAsyncStatus {
    WGPUCreatePipelineAsyncStatus_Success = 0x00000000,
    WGPUCreatePipelineAsyncStatus_Error = 0x00000001,
    WGPUCreatePipelineAsyncStatus_DeviceLost = 0x00000002,
    WGPUCreatePipelineAsyncStatus_DeviceDestroyed = 0x00000003,
    WGPUCreatePipelineAsyncStatus_Unknown = 0x00000004,
    WGPUCreatePipelineAsyncStatus_Force32 = 0x7FFFFFFF
} WGPUCreatePipelineAsyncStatus;

typedef enum WGPUCullMode {
    WGPUCullMode_None = 0x00000000,
    WGPUCullMode_Front = 0x00000001,
    WGPUCullMode_Back = 0x00000002,
    WGPUCullMode_Force32 = 0x7FFFFFFF
} WGPUCullMode;

typedef enum WGPUDeviceLostReason {
    WGPUDeviceLostReason_Undefined = 0x00000000,
    WGPUDeviceLostReason_Destroyed = 0x00000001,
    WGPUDeviceLostReason_Force32 = 0x7FFFFFFF
} WGPUDeviceLostReason;

typedef enum WGPUErrorFilter {
    WGPUErrorFilter_Validation = 0x00000000,
    WGPUErrorFilter_OutOfMemory = 0x00000001,
    WGPUErrorFilter_Force32 = 0x7FFFFFFF
} WGPUErrorFilter;

typedef enum WGPUErrorType {
    WGPUErrorType_NoError = 0x00000000,
    WGPUErrorType_Validation = 0x00000001,
    WGPUErrorType_OutOfMemory = 0x00000002,
    WGPUErrorType_Unknown = 0x00000003,
    WGPUErrorType_DeviceLost = 0x00000004,
    WGPUErrorType_Force32 = 0x7FFFFFFF
} WGPUErrorType;

typedef enum WGPUFeatureName {
    WGPUFeatureName_Undefined = 0x00000000,
    WGPUFeatureName_DepthClipControl = 0x00000001,
    WGPUFeatureName_Depth32FloatStencil8 = 0x00000002,
    WGPUFeatureName_TimestampQuery = 0x00000003,
    WGPUFeatureName_PipelineStatisticsQuery = 0x00000004,
    WGPUFeatureName_TextureCompressionBC = 0x00000005,
    WGPUFeatureName_TextureCompressionETC2 = 0x00000006,
    WGPUFeatureName_TextureCompressionASTC = 0x00000007,
    WGPUFeatureName_IndirectFirstInstance = 0x00000008,
    WGPUFeatureName_Force32 = 0x7FFFFFFF
} WGPUFeatureName;

typedef enum WGPUFilterMode {
    WGPUFilterMode_Nearest = 0x00000000,
    WGPUFilterMode_Linear = 0x00000001,
    WGPUFilterMode_Force32 = 0x7FFFFFFF
} WGPUFilterMode;

typedef enum WGPUFrontFace {
    WGPUFrontFace_CCW = 0x00000000,
    WGPUFrontFace_CW = 0x00000001,
    WGPUFrontFace_Force32 = 0x7FFFFFFF
} WGPUFrontFace;

typedef enum WGPUIndexFormat {
    WGPUIndexFormat_Undefined = 0x00000000,
    WGPUIndexFormat_Uint16 = 0x00000001,
    WGPUIndexFormat_Uint32 = 0x00000002,
    WGPUIndexFormat_Force32 = 0x7FFFFFFF
} WGPUIndexFormat;

typedef enum WGPULoadOp {
    WGPULoadOp_Undefined = 0x00000000,
    WGPULoadOp_Clear = 0x00000001,
    WGPULoadOp_Load = 0x00000002,
    WGPULoadOp_Force32 = 0x7FFFFFFF
} WGPULoadOp;

typedef enum WGPUPipelineStatisticName {
    WGPUPipelineStatisticName_VertexShaderInvocations = 0x00000000,
    WGPUPipelineStatisticName_ClipperInvocations = 0x00000001,
    WGPUPipelineStatisticName_ClipperPrimitivesOut = 0x00000002,
    WGPUPipelineStatisticName_FragmentShaderInvocations = 0x00000003,
    WGPUPipelineStatisticName_ComputeShaderInvocations = 0x00000004,
    WGPUPipelineStatisticName_Force32 = 0x7FFFFFFF
} WGPUPipelineStatisticName;

typedef enum WGPUPowerPreference {
    WGPUPowerPreference_Undefined = 0x00000000,
    WGPUPowerPreference_LowPower = 0x00000001,
    WGPUPowerPreference_HighPerformance = 0x00000002,
    WGPUPowerPreference_Force32 = 0x7FFFFFFF
} WGPUPowerPreference;

typedef enum WGPUPresentMode {
    WGPUPresentMode_Immediate = 0x00000000,
    WGPUPresentMode_Mailbox = 0x00000001,
    WGPUPresentMode_Fifo = 0x00000002,
    WGPUPresentMode_Force32 = 0x7FFFFFFF
} WGPUPresentMode;

typedef enum WGPUPrimitiveTopology {
    WGPUPrimitiveTopology_PointList = 0x00000000,
    WGPUPrimitiveTopology_LineList = 0x00000001,
    WGPUPrimitiveTopology_LineStrip = 0x00000002,
    WGPUPrimitiveTopology_TriangleList = 0x00000003,
    WGPUPrimitiveTopology_TriangleStrip = 0x00000004,
    WGPUPrimitiveTopology_Force32 = 0x7FFFFFFF
} WGPUPrimitiveTopology;

typedef enum WGPUQueryType {
    WGPUQueryType_Occlusion = 0x00000000,
    WGPUQueryType_PipelineStatistics = 0x00000001,
    WGPUQueryType_Timestamp = 0x00000002,
    WGPUQueryType_Force32 = 0x7FFFFFFF
} WGPUQueryType;

typedef enum WGPUQueueWorkDoneStatus {
    WGPUQueueWorkDoneStatus_Success = 0x00000000,
    WGPUQueueWorkDoneStatus_Error = 0x00000001,
    WGPUQueueWorkDoneStatus_Unknown = 0x00000002,
    WGPUQueueWorkDoneStatus_DeviceLost = 0x00000003,
    WGPUQueueWorkDoneStatus_Force32 = 0x7FFFFFFF
} WGPUQueueWorkDoneStatus;

typedef enum WGPURenderPassTimestampLocation {
    WGPURenderPassTimestampLocation_Beginning = 0x00000000,
    WGPURenderPassTimestampLocation_End = 0x00000001,
    WGPURenderPassTimestampLocation_Force32 = 0x7FFFFFFF
} WGPURenderPassTimestampLocation;

typedef enum WGPURequestAdapterStatus {
    WGPURequestAdapterStatus_Success = 0x00000000,
    WGPURequestAdapterStatus_Unavailable = 0x00000001,
    WGPURequestAdapterStatus_Error = 0x00000002,
    WGPURequestAdapterStatus_Unknown = 0x00000003,
    WGPURequestAdapterStatus_Force32 = 0x7FFFFFFF
} WGPURequestAdapterStatus;

typedef enum WGPURequestDeviceStatus {
    WGPURequestDeviceStatus_Success = 0x00000000,
    WGPURequestDeviceStatus_Error = 0x00000001,
    WGPURequestDeviceStatus_Unknown = 0x00000002,
    WGPURequestDeviceStatus_Force32 = 0x7FFFFFFF
} WGPURequestDeviceStatus;

typedef enum WGPUSType {
    WGPUSType_Invalid = 0x00000000,
    WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector = 0x00000004,
    WGPUSType_ShaderModuleSPIRVDescriptor = 0x00000005,
    WGPUSType_ShaderModuleWGSLDescriptor = 0x00000006,
    WGPUSType_PrimitiveDepthClipControl = 0x00000007,
    WGPUSType_RenderPassDescriptorMaxDrawCount = 0x0000000F,
    WGPUSType_Force32 = 0x7FFFFFFF
} WGPUSType;

typedef enum WGPUSamplerBindingType {
    WGPUSamplerBindingType_Undefined = 0x00000000,
    WGPUSamplerBindingType_Filtering = 0x00000001,
    WGPUSamplerBindingType_NonFiltering = 0x00000002,
    WGPUSamplerBindingType_Comparison = 0x00000003,
    WGPUSamplerBindingType_Force32 = 0x7FFFFFFF
} WGPUSamplerBindingType;

typedef enum WGPUStencilOperation {
    WGPUStencilOperation_Keep = 0x00000000,
    WGPUStencilOperation_Zero = 0x00000001,
    WGPUStencilOperation_Replace = 0x00000002,
    WGPUStencilOperation_Invert = 0x00000003,
    WGPUStencilOperation_IncrementClamp = 0x00000004,
    WGPUStencilOperation_DecrementClamp = 0x00000005,
    WGPUStencilOperation_IncrementWrap = 0x00000006,
    WGPUStencilOperation_DecrementWrap = 0x00000007,
    WGPUStencilOperation_Force32 = 0x7FFFFFFF
} WGPUStencilOperation;

typedef enum WGPUStorageTextureAccess {
    WGPUStorageTextureAccess_Undefined = 0x00000000,
    WGPUStorageTextureAccess_WriteOnly = 0x00000001,
    WGPUStorageTextureAccess_Force32 = 0x7FFFFFFF
} WGPUStorageTextureAccess;

typedef enum WGPUStoreOp {
    WGPUStoreOp_Undefined = 0x00000000,
    WGPUStoreOp_Store = 0x00000001,
    WGPUStoreOp_Discard = 0x00000002,
    WGPUStoreOp_Force32 = 0x7FFFFFFF
} WGPUStoreOp;

typedef enum WGPUTextureAspect {
    WGPUTextureAspect_All = 0x00000000,
    WGPUTextureAspect_StencilOnly = 0x00000001,
    WGPUTextureAspect_DepthOnly = 0x00000002,
    WGPUTextureAspect_Force32 = 0x7FFFFFFF
} WGPUTextureAspect;

typedef enum WGPUTextureComponentType {
    WGPUTextureComponentType_Float = 0x00000000,
    WGPUTextureComponentType_Sint = 0x00000001,
    WGPUTextureComponentType_Uint = 0x00000002,
    WGPUTextureComponentType_DepthComparison = 0x00000003,
    WGPUTextureComponentType_Force32 = 0x7FFFFFFF
} WGPUTextureComponentType;

typedef enum WGPUTextureDimension {
    WGPUTextureDimension_1D = 0x00000000,
    WGPUTextureDimension_2D = 0x00000001,
    WGPUTextureDimension_3D = 0x00000002,
    WGPUTextureDimension_Force32 = 0x7FFFFFFF
} WGPUTextureDimension;

typedef enum WGPUTextureFormat {
    WGPUTextureFormat_Undefined = 0x00000000,
    WGPUTextureFormat_R8Unorm = 0x00000001,
    WGPUTextureFormat_R8Snorm = 0x00000002,
    WGPUTextureFormat_R8Uint = 0x00000003,
    WGPUTextureFormat_R8Sint = 0x00000004,
    WGPUTextureFormat_R16Uint = 0x00000005,
    WGPUTextureFormat_R16Sint = 0x00000006,
    WGPUTextureFormat_R16Float = 0x00000007,
    WGPUTextureFormat_RG8Unorm = 0x00000008,
    WGPUTextureFormat_RG8Snorm = 0x00000009,
    WGPUTextureFormat_RG8Uint = 0x0000000A,
    WGPUTextureFormat_RG8Sint = 0x0000000B,
    WGPUTextureFormat_R32Float = 0x0000000C,
    WGPUTextureFormat_R32Uint = 0x0000000D,
    WGPUTextureFormat_R32Sint = 0x0000000E,
    WGPUTextureFormat_RG16Uint = 0x0000000F,
    WGPUTextureFormat_RG16Sint = 0x00000010,
    WGPUTextureFormat_RG16Float = 0x00000011,
    WGPUTextureFormat_RGBA8Unorm = 0x00000012,
    WGPUTextureFormat_RGBA8UnormSrgb = 0x00000013,
    WGPUTextureFormat_RGBA8Snorm = 0x00000014,
    WGPUTextureFormat_RGBA8Uint = 0x00000015,
    WGPUTextureFormat_RGBA8Sint = 0x00000016,
    WGPUTextureFormat_BGRA8Unorm = 0x00000017,
    WGPUTextureFormat_BGRA8UnormSrgb = 0x00000018,
    WGPUTextureFormat_RGB10A2Unorm = 0x00000019,
    WGPUTextureFormat_RG11B10Ufloat = 0x0000001A,
    WGPUTextureFormat_RGB9E5Ufloat = 0x0000001B,
    WGPUTextureFormat_RG32Float = 0x0000001C,
    WGPUTextureFormat_RG32Uint = 0x0000001D,
    WGPUTextureFormat_RG32Sint = 0x0000001E,
    WGPUTextureFormat_RGBA16Uint = 0x0000001F,
    WGPUTextureFormat_RGBA16Sint = 0x00000020,
    WGPUTextureFormat_RGBA16Float = 0x00000021,
    WGPUTextureFormat_RGBA32Float = 0x00000022,
    WGPUTextureFormat_RGBA32Uint = 0x00000023,
    WGPUTextureFormat_RGBA32Sint = 0x00000024,
    WGPUTextureFormat_Stencil8 = 0x00000025,
    WGPUTextureFormat_Depth16Unorm = 0x00000026,
    WGPUTextureFormat_Depth24Plus = 0x00000027,
    WGPUTextureFormat_Depth24PlusStencil8 = 0x00000028,
    WGPUTextureFormat_Depth32Float = 0x00000029,
    WGPUTextureFormat_Depth32FloatStencil8 = 0x0000002A,
    WGPUTextureFormat_BC1RGBAUnorm = 0x0000002B,
    WGPUTextureFormat_BC1RGBAUnormSrgb = 0x0000002C,
    WGPUTextureFormat_BC2RGBAUnorm = 0x0000002D,
    WGPUTextureFormat_BC2RGBAUnormSrgb = 0x0000002E,
    WGPUTextureFormat_BC3RGBAUnorm = 0x0000002F,
    WGPUTextureFormat_BC3RGBAUnormSrgb = 0x00000030,
    WGPUTextureFormat_BC4RUnorm = 0x00000031,
    WGPUTextureFormat_BC4RSnorm = 0x00000032,
    WGPUTextureFormat_BC5RGUnorm = 0x00000033,
    WGPUTextureFormat_BC5RGSnorm = 0x00000034,
    WGPUTextureFormat_BC6HRGBUfloat = 0x00000035,
    WGPUTextureFormat_BC6HRGBFloat = 0x00000036,
    WGPUTextureFormat_BC7RGBAUnorm = 0x00000037,
    WGPUTextureFormat_BC7RGBAUnormSrgb = 0x00000038,
    WGPUTextureFormat_ETC2RGB8Unorm = 0x00000039,
    WGPUTextureFormat_ETC2RGB8UnormSrgb = 0x0000003A,
    WGPUTextureFormat_ETC2RGB8A1Unorm = 0x0000003B,
    WGPUTextureFormat_ETC2RGB8A1UnormSrgb = 0x0000003C,
    WGPUTextureFormat_ETC2RGBA8Unorm = 0x0000003D,
    WGPUTextureFormat_ETC2RGBA8UnormSrgb = 0x0000003E,
    WGPUTextureFormat_EACR11Unorm = 0x0000003F,
    WGPUTextureFormat_EACR11Snorm = 0x00000040,
    WGPUTextureFormat_EACRG11Unorm = 0x00000041,
    WGPUTextureFormat_EACRG11Snorm = 0x00000042,
    WGPUTextureFormat_ASTC4x4Unorm = 0x00000043,
    WGPUTextureFormat_ASTC4x4UnormSrgb = 0x00000044,
    WGPUTextureFormat_ASTC5x4Unorm = 0x00000045,
    WGPUTextureFormat_ASTC5x4UnormSrgb = 0x00000046,
    WGPUTextureFormat_ASTC5x5Unorm = 0x00000047,
    WGPUTextureFormat_ASTC5x5UnormSrgb = 0x00000048,
    WGPUTextureFormat_ASTC6x5Unorm = 0x00000049,
    WGPUTextureFormat_ASTC6x5UnormSrgb = 0x0000004A,
    WGPUTextureFormat_ASTC6x6Unorm = 0x0000004B,
    WGPUTextureFormat_ASTC6x6UnormSrgb = 0x0000004C,
    WGPUTextureFormat_ASTC8x5Unorm = 0x0000004D,
    WGPUTextureFormat_ASTC8x5UnormSrgb = 0x0000004E,
    WGPUTextureFormat_ASTC8x6Unorm = 0x0000004F,
    WGPUTextureFormat_ASTC8x6UnormSrgb = 0x00000050,
    WGPUTextureFormat_ASTC8x8Unorm = 0x00000051,
    WGPUTextureFormat_ASTC8x8UnormSrgb = 0x00000052,
    WGPUTextureFormat_ASTC10x5Unorm = 0x00000053,
    WGPUTextureFormat_ASTC10x5UnormSrgb = 0x00000054,
    WGPUTextureFormat_ASTC10x6Unorm = 0x00000055,
    WGPUTextureFormat_ASTC10x6UnormSrgb = 0x00000056,
    WGPUTextureFormat_ASTC10x8Unorm = 0x00000057,
    WGPUTextureFormat_ASTC10x8UnormSrgb = 0x00000058,
    WGPUTextureFormat_ASTC10x10Unorm = 0x00000059,
    WGPUTextureFormat_ASTC10x10UnormSrgb = 0x0000005A,
    WGPUTextureFormat_ASTC12x10Unorm = 0x0000005B,
    WGPUTextureFormat_ASTC12x10UnormSrgb = 0x0000005C,
    WGPUTextureFormat_ASTC12x12Unorm = 0x0000005D,
    WGPUTextureFormat_ASTC12x12UnormSrgb = 0x0000005E,
    WGPUTextureFormat_Force32 = 0x7FFFFFFF
} WGPUTextureFormat;

typedef enum WGPUTextureSampleType {
    WGPUTextureSampleType_Undefined = 0x00000000,
    WGPUTextureSampleType_Float = 0x00000001,
    WGPUTextureSampleType_UnfilterableFloat = 0x00000002,
    WGPUTextureSampleType_Depth = 0x00000003,
    WGPUTextureSampleType_Sint = 0x00000004,
    WGPUTextureSampleType_Uint = 0x00000005,
    WGPUTextureSampleType_Force32 = 0x7FFFFFFF
} WGPUTextureSampleType;

typedef enum WGPUTextureViewDimension {
    WGPUTextureViewDimension_Undefined = 0x00000000,
    WGPUTextureViewDimension_1D = 0x00000001,
    WGPUTextureViewDimension_2D = 0x00000002,
    WGPUTextureViewDimension_2DArray = 0x00000003,
    WGPUTextureViewDimension_Cube = 0x00000004,
    WGPUTextureViewDimension_CubeArray = 0x00000005,
    WGPUTextureViewDimension_3D = 0x00000006,
    WGPUTextureViewDimension_Force32 = 0x7FFFFFFF
} WGPUTextureViewDimension;

typedef enum WGPUVertexFormat {
    WGPUVertexFormat_Undefined = 0x00000000,
    WGPUVertexFormat_Uint8x2 = 0x00000001,
    WGPUVertexFormat_Uint8x4 = 0x00000002,
    WGPUVertexFormat_Sint8x2 = 0x00000003,
    WGPUVertexFormat_Sint8x4 = 0x00000004,
    WGPUVertexFormat_Unorm8x2 = 0x00000005,
    WGPUVertexFormat_Unorm8x4 = 0x00000006,
    WGPUVertexFormat_Snorm8x2 = 0x00000007,
    WGPUVertexFormat_Snorm8x4 = 0x00000008,
    WGPUVertexFormat_Uint16x2 = 0x00000009,
    WGPUVertexFormat_Uint16x4 = 0x0000000A,
    WGPUVertexFormat_Sint16x2 = 0x0000000B,
    WGPUVertexFormat_Sint16x4 = 0x0000000C,
    WGPUVertexFormat_Unorm16x2 = 0x0000000D,
    WGPUVertexFormat_Unorm16x4 = 0x0000000E,
    WGPUVertexFormat_Snorm16x2 = 0x0000000F,
    WGPUVertexFormat_Snorm16x4 = 0x00000010,
    WGPUVertexFormat_Float16x2 = 0x00000011,
    WGPUVertexFormat_Float16x4 = 0x00000012,
    WGPUVertexFormat_Float32 = 0x00000013,
    WGPUVertexFormat_Float32x2 = 0x00000014,
    WGPUVertexFormat_Float32x3 = 0x00000015,
    WGPUVertexFormat_Float32x4 = 0x00000016,
    WGPUVertexFormat_Uint32 = 0x00000017,
    WGPUVertexFormat_Uint32x2 = 0x00000018,
    WGPUVertexFormat_Uint32x3 = 0x00000019,
    WGPUVertexFormat_Uint32x4 = 0x0000001A,
    WGPUVertexFormat_Sint32 = 0x0000001B,
    WGPUVertexFormat_Sint32x2 = 0x0000001C,
    WGPUVertexFormat_Sint32x3 = 0x0000001D,
    WGPUVertexFormat_Sint32x4 = 0x0000001E,
    WGPUVertexFormat_Force32 = 0x7FFFFFFF
} WGPUVertexFormat;

typedef enum WGPUVertexStepMode {
    WGPUVertexStepMode_Vertex = 0x00000000,
    WGPUVertexStepMode_Instance = 0x00000001,
    WGPUVertexStepMode_VertexBufferNotUsed = 0x00000002,
    WGPUVertexStepMode_Force32 = 0x7FFFFFFF
} WGPUVertexStepMode;

typedef enum WGPUBufferUsage {
    WGPUBufferUsage_None = 0x00000000,
    WGPUBufferUsage_MapRead = 0x00000001,
    WGPUBufferUsage_MapWrite = 0x00000002,
    WGPUBufferUsage_CopySrc = 0x00000004,
    WGPUBufferUsage_CopyDst = 0x00000008,
    WGPUBufferUsage_Index = 0x00000010,
    WGPUBufferUsage_Vertex = 0x00000020,
    WGPUBufferUsage_Uniform = 0x00000040,
    WGPUBufferUsage_Storage = 0x00000080,
    WGPUBufferUsage_Indirect = 0x00000100,
    WGPUBufferUsage_QueryResolve = 0x00000200,
    WGPUBufferUsage_Force32 = 0x7FFFFFFF
} WGPUBufferUsage;
typedef WGPUFlags WGPUBufferUsageFlags;

typedef enum WGPUColorWriteMask {
    WGPUColorWriteMask_None = 0x00000000,
    WGPUColorWriteMask_Red = 0x00000001,
    WGPUColorWriteMask_Green = 0x00000002,
    WGPUColorWriteMask_Blue = 0x00000004,
    WGPUColorWriteMask_Alpha = 0x00000008,
    WGPUColorWriteMask_All = 0x0000000F,
    WGPUColorWriteMask_Force32 = 0x7FFFFFFF
} WGPUColorWriteMask;
typedef WGPUFlags WGPUColorWriteMaskFlags;

typedef enum WGPUMapMode {
    WGPUMapMode_None = 0x00000000,
    WGPUMapMode_Read = 0x00000001,
    WGPUMapMode_Write = 0x00000002,
    WGPUMapMode_Force32 = 0x7FFFFFFF
} WGPUMapMode;
typedef WGPUFlags WGPUMapModeFlags;

typedef enum WGPUShaderStage {
    WGPUShaderStage_None = 0x00000000,
    WGPUShaderStage_Vertex = 0x00000001,
    WGPUShaderStage_Fragment = 0x00000002,
    WGPUShaderStage_Compute = 0x00000004,
    WGPUShaderStage_Force32 = 0x7FFFFFFF
} WGPUShaderStage;
typedef WGPUFlags WGPUShaderStageFlags;

typedef enum WGPUTextureUsage {
    WGPUTextureUsage_None = 0x00000000,
    WGPUTextureUsage_CopySrc = 0x00000001,
    WGPUTextureUsage_CopyDst = 0x00000002,
    WGPUTextureUsage_TextureBinding = 0x00000004,
    WGPUTextureUsage_StorageBinding = 0x00000008,
    WGPUTextureUsage_RenderAttachment = 0x00000010,
    WGPUTextureUsage_Force32 = 0x7FFFFFFF
} WGPUTextureUsage;
typedef WGPUFlags WGPUTextureUsageFlags;

typedef struct WGPUChainedStruct {
    struct WGPUChainedStruct const * next;
    WGPUSType sType;
} WGPUChainedStruct;

typedef struct WGPUChainedStructOut {
    struct WGPUChainedStructOut * next;
    WGPUSType sType;
} WGPUChainedStructOut;

typedef struct WGPUAdapterProperties {
    WGPUChainedStructOut * nextInChain;
    uint32_t vendorID;
    char const * vendorName;
    char const * architecture;
    uint32_t deviceID;
    char const * name;
    char const * driverDescription;
    WGPUAdapterType adapterType;
    WGPUBackendType backendType;
} WGPUAdapterProperties;

typedef struct WGPUBindGroupEntry {
    WGPUChainedStruct const * nextInChain;
    uint32_t binding;
    WGPUBuffer buffer; // nullable
    uint64_t offset;
    uint64_t size;
    WGPUSampler sampler; // nullable
    WGPUTextureView textureView; // nullable
} WGPUBindGroupEntry;

typedef struct WGPUBlendComponent {
    WGPUBlendOperation operation;
    WGPUBlendFactor srcFactor;
    WGPUBlendFactor dstFactor;
} WGPUBlendComponent;

typedef struct WGPUBufferBindingLayout {
    WGPUChainedStruct const * nextInChain;
    WGPUBufferBindingType type;
    bool hasDynamicOffset;
    uint64_t minBindingSize;
} WGPUBufferBindingLayout;

typedef struct WGPUBufferDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    WGPUBufferUsageFlags usage;
    uint64_t size;
    bool mappedAtCreation;
} WGPUBufferDescriptor;

typedef struct WGPUColor {
    double r;
    double g;
    double b;
    double a;
} WGPUColor;

typedef struct WGPUCommandBufferDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
} WGPUCommandBufferDescriptor;

typedef struct WGPUCommandEncoderDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
} WGPUCommandEncoderDescriptor;

typedef struct WGPUCompilationMessage {
    WGPUChainedStruct const * nextInChain;
    char const * message; // nullable
    WGPUCompilationMessageType type;
    uint64_t lineNum;
    uint64_t linePos;
    uint64_t offset;
    uint64_t length;
} WGPUCompilationMessage;

typedef struct WGPUComputePassTimestampWrite {
    WGPUQuerySet querySet;
    uint32_t queryIndex;
    WGPUComputePassTimestampLocation location;
} WGPUComputePassTimestampWrite;

typedef struct WGPUConstantEntry {
    WGPUChainedStruct const * nextInChain;
    char const * key;
    double value;
} WGPUConstantEntry;

typedef struct WGPUExtent3D {
    uint32_t width;
    uint32_t height;
    uint32_t depthOrArrayLayers;
} WGPUExtent3D;

typedef struct WGPUInstanceDescriptor {
    WGPUChainedStruct const * nextInChain;
} WGPUInstanceDescriptor;

typedef struct WGPULimits {
    uint32_t maxTextureDimension1D;
    uint32_t maxTextureDimension2D;
    uint32_t maxTextureDimension3D;
    uint32_t maxTextureArrayLayers;
    uint32_t maxBindGroups;
    uint32_t maxDynamicUniformBuffersPerPipelineLayout;
    uint32_t maxDynamicStorageBuffersPerPipelineLayout;
    uint32_t maxSampledTexturesPerShaderStage;
    uint32_t maxSamplersPerShaderStage;
    uint32_t maxStorageBuffersPerShaderStage;
    uint32_t maxStorageTexturesPerShaderStage;
    uint32_t maxUniformBuffersPerShaderStage;
    uint64_t maxUniformBufferBindingSize;
    uint64_t maxStorageBufferBindingSize;
    uint32_t minUniformBufferOffsetAlignment;
    uint32_t minStorageBufferOffsetAlignment;
    uint32_t maxVertexBuffers;
    uint32_t maxVertexAttributes;
    uint32_t maxVertexBufferArrayStride;
    uint32_t maxInterStageShaderComponents;
    uint32_t maxInterStageShaderVariables;
    uint32_t maxColorAttachments;
    uint32_t maxComputeWorkgroupStorageSize;
    uint32_t maxComputeInvocationsPerWorkgroup;
    uint32_t maxComputeWorkgroupSizeX;
    uint32_t maxComputeWorkgroupSizeY;
    uint32_t maxComputeWorkgroupSizeZ;
    uint32_t maxComputeWorkgroupsPerDimension;
} WGPULimits;

typedef struct WGPUMultisampleState {
    WGPUChainedStruct const * nextInChain;
    uint32_t count;
    uint32_t mask;
    bool alphaToCoverageEnabled;
} WGPUMultisampleState;

typedef struct WGPUOrigin3D {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} WGPUOrigin3D;

typedef struct WGPUPipelineLayoutDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    uint32_t bindGroupLayoutCount;
    WGPUBindGroupLayout const * bindGroupLayouts;
} WGPUPipelineLayoutDescriptor;

// Can be chained in WGPUPrimitiveState
typedef struct WGPUPrimitiveDepthClipControl {
    WGPUChainedStruct chain;
    bool unclippedDepth;
} WGPUPrimitiveDepthClipControl;

typedef struct WGPUPrimitiveState {
    WGPUChainedStruct const * nextInChain;
    WGPUPrimitiveTopology topology;
    WGPUIndexFormat stripIndexFormat;
    WGPUFrontFace frontFace;
    WGPUCullMode cullMode;
} WGPUPrimitiveState;

typedef struct WGPUQuerySetDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    WGPUQueryType type;
    uint32_t count;
    WGPUPipelineStatisticName const * pipelineStatistics;
    uint32_t pipelineStatisticsCount;
} WGPUQuerySetDescriptor;

typedef struct WGPUQueueDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
} WGPUQueueDescriptor;

typedef struct WGPURenderBundleDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
} WGPURenderBundleDescriptor;

typedef struct WGPURenderBundleEncoderDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    uint32_t colorFormatsCount;
    WGPUTextureFormat const * colorFormats;
    WGPUTextureFormat depthStencilFormat;
    uint32_t sampleCount;
    bool depthReadOnly;
    bool stencilReadOnly;
} WGPURenderBundleEncoderDescriptor;

typedef struct WGPURenderPassDepthStencilAttachment {
    WGPUTextureView view;
    WGPULoadOp depthLoadOp;
    WGPUStoreOp depthStoreOp;
    float depthClearValue;
    bool depthReadOnly;
    WGPULoadOp stencilLoadOp;
    WGPUStoreOp stencilStoreOp;
    uint32_t stencilClearValue;
    bool stencilReadOnly;
} WGPURenderPassDepthStencilAttachment;

// Can be chained in WGPURenderPassDescriptor
typedef struct WGPURenderPassDescriptorMaxDrawCount {
    WGPUChainedStruct chain;
    uint64_t maxDrawCount;
} WGPURenderPassDescriptorMaxDrawCount;

typedef struct WGPURenderPassTimestampWrite {
    WGPUQuerySet querySet;
    uint32_t queryIndex;
    WGPURenderPassTimestampLocation location;
} WGPURenderPassTimestampWrite;

typedef struct WGPURequestAdapterOptions {
    WGPUChainedStruct const * nextInChain;
    WGPUSurface compatibleSurface; // nullable
    WGPUPowerPreference powerPreference;
    bool forceFallbackAdapter;
} WGPURequestAdapterOptions;

typedef struct WGPUSamplerBindingLayout {
    WGPUChainedStruct const * nextInChain;
    WGPUSamplerBindingType type;
} WGPUSamplerBindingLayout;

typedef struct WGPUSamplerDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    WGPUAddressMode addressModeU;
    WGPUAddressMode addressModeV;
    WGPUAddressMode addressModeW;
    WGPUFilterMode magFilter;
    WGPUFilterMode minFilter;
    WGPUFilterMode mipmapFilter;
    float lodMinClamp;
    float lodMaxClamp;
    WGPUCompareFunction compare;
    uint16_t maxAnisotropy;
} WGPUSamplerDescriptor;

typedef struct WGPUShaderModuleDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
} WGPUShaderModuleDescriptor;

// Can be chained in WGPUShaderModuleDescriptor
typedef struct WGPUShaderModuleSPIRVDescriptor {
    WGPUChainedStruct chain;
    uint32_t codeSize;
    uint32_t const * code;
} WGPUShaderModuleSPIRVDescriptor;

// Can be chained in WGPUShaderModuleDescriptor
typedef struct WGPUShaderModuleWGSLDescriptor {
    WGPUChainedStruct chain;
    char const * source;
} WGPUShaderModuleWGSLDescriptor;

typedef struct WGPUStencilFaceState {
    WGPUCompareFunction compare;
    WGPUStencilOperation failOp;
    WGPUStencilOperation depthFailOp;
    WGPUStencilOperation passOp;
} WGPUStencilFaceState;

typedef struct WGPUStorageTextureBindingLayout {
    WGPUChainedStruct const * nextInChain;
    WGPUStorageTextureAccess access;
    WGPUTextureFormat format;
    WGPUTextureViewDimension viewDimension;
} WGPUStorageTextureBindingLayout;

typedef struct WGPUSurfaceDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
} WGPUSurfaceDescriptor;

// Can be chained in WGPUSurfaceDescriptor
typedef struct WGPUSurfaceDescriptorFromCanvasHTMLSelector {
    WGPUChainedStruct chain;
    char const * selector;
} WGPUSurfaceDescriptorFromCanvasHTMLSelector;

typedef struct WGPUSwapChainDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    WGPUTextureUsageFlags usage;
    WGPUTextureFormat format;
    uint32_t width;
    uint32_t height;
    WGPUPresentMode presentMode;
} WGPUSwapChainDescriptor;

typedef struct WGPUTextureBindingLayout {
    WGPUChainedStruct const * nextInChain;
    WGPUTextureSampleType sampleType;
    WGPUTextureViewDimension viewDimension;
    bool multisampled;
} WGPUTextureBindingLayout;

typedef struct WGPUTextureDataLayout {
    WGPUChainedStruct const * nextInChain;
    uint64_t offset;
    uint32_t bytesPerRow;
    uint32_t rowsPerImage;
} WGPUTextureDataLayout;

typedef struct WGPUTextureViewDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    WGPUTextureFormat format;
    WGPUTextureViewDimension dimension;
    uint32_t baseMipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer;
    uint32_t arrayLayerCount;
    WGPUTextureAspect aspect;
} WGPUTextureViewDescriptor;

typedef struct WGPUVertexAttribute {
    WGPUVertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
} WGPUVertexAttribute;

typedef struct WGPUBindGroupDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    WGPUBindGroupLayout layout;
    uint32_t entryCount;
    WGPUBindGroupEntry const * entries;
} WGPUBindGroupDescriptor;

typedef struct WGPUBindGroupLayoutEntry {
    WGPUChainedStruct const * nextInChain;
    uint32_t binding;
    WGPUShaderStageFlags visibility;
    WGPUBufferBindingLayout buffer;
    WGPUSamplerBindingLayout sampler;
    WGPUTextureBindingLayout texture;
    WGPUStorageTextureBindingLayout storageTexture;
} WGPUBindGroupLayoutEntry;

typedef struct WGPUBlendState {
    WGPUBlendComponent color;
    WGPUBlendComponent alpha;
} WGPUBlendState;

typedef struct WGPUCompilationInfo {
    WGPUChainedStruct const * nextInChain;
    uint32_t messageCount;
    WGPUCompilationMessage const * messages;
} WGPUCompilationInfo;

typedef struct WGPUComputePassDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    uint32_t timestampWriteCount;
    WGPUComputePassTimestampWrite const * timestampWrites;
} WGPUComputePassDescriptor;

typedef struct WGPUDepthStencilState {
    WGPUChainedStruct const * nextInChain;
    WGPUTextureFormat format;
    bool depthWriteEnabled;
    WGPUCompareFunction depthCompare;
    WGPUStencilFaceState stencilFront;
    WGPUStencilFaceState stencilBack;
    uint32_t stencilReadMask;
    uint32_t stencilWriteMask;
    int32_t depthBias;
    float depthBiasSlopeScale;
    float depthBiasClamp;
} WGPUDepthStencilState;

typedef struct WGPUImageCopyBuffer {
    WGPUChainedStruct const * nextInChain;
    WGPUTextureDataLayout layout;
    WGPUBuffer buffer;
} WGPUImageCopyBuffer;

typedef struct WGPUImageCopyTexture {
    WGPUChainedStruct const * nextInChain;
    WGPUTexture texture;
    uint32_t mipLevel;
    WGPUOrigin3D origin;
    WGPUTextureAspect aspect;
} WGPUImageCopyTexture;

typedef struct WGPUProgrammableStageDescriptor {
    WGPUChainedStruct const * nextInChain;
    WGPUShaderModule module;
    char const * entryPoint;
    uint32_t constantCount;
    WGPUConstantEntry const * constants;
} WGPUProgrammableStageDescriptor;

typedef struct WGPURenderPassColorAttachment {
    WGPUTextureView view; // nullable
    WGPUTextureView resolveTarget; // nullable
    WGPULoadOp loadOp;
    WGPUStoreOp storeOp;
    WGPUColor clearValue;
} WGPURenderPassColorAttachment;

typedef struct WGPURequiredLimits {
    WGPUChainedStruct const * nextInChain;
    WGPULimits limits;
} WGPURequiredLimits;

typedef struct WGPUSupportedLimits {
    WGPUChainedStructOut * nextInChain;
    WGPULimits limits;
} WGPUSupportedLimits;

typedef struct WGPUTextureDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    WGPUTextureUsageFlags usage;
    WGPUTextureDimension dimension;
    WGPUExtent3D size;
    WGPUTextureFormat format;
    uint32_t mipLevelCount;
    uint32_t sampleCount;
    uint32_t viewFormatCount;
    WGPUTextureFormat const * viewFormats;
} WGPUTextureDescriptor;

typedef struct WGPUVertexBufferLayout {
    uint64_t arrayStride;
    WGPUVertexStepMode stepMode;
    uint32_t attributeCount;
    WGPUVertexAttribute const * attributes;
} WGPUVertexBufferLayout;

typedef struct WGPUBindGroupLayoutDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    uint32_t entryCount;
    WGPUBindGroupLayoutEntry const * entries;
} WGPUBindGroupLayoutDescriptor;

typedef struct WGPUColorTargetState {
    WGPUChainedStruct const * nextInChain;
    WGPUTextureFormat format;
    WGPUBlendState const * blend; // nullable
    WGPUColorWriteMaskFlags writeMask;
} WGPUColorTargetState;

typedef struct WGPUComputePipelineDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    WGPUPipelineLayout layout; // nullable
    WGPUProgrammableStageDescriptor compute;
} WGPUComputePipelineDescriptor;

typedef struct WGPUDeviceDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    uint32_t requiredFeaturesCount;
    WGPUFeatureName const * requiredFeatures;
    WGPURequiredLimits const * requiredLimits; // nullable
    WGPUQueueDescriptor defaultQueue;
} WGPUDeviceDescriptor;

typedef struct WGPURenderPassDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    uint32_t colorAttachmentCount;
    WGPURenderPassColorAttachment const * colorAttachments;
    WGPURenderPassDepthStencilAttachment const * depthStencilAttachment; // nullable
    WGPUQuerySet occlusionQuerySet; // nullable
    uint32_t timestampWriteCount;
    WGPURenderPassTimestampWrite const * timestampWrites;
} WGPURenderPassDescriptor;

typedef struct WGPUVertexState {
    WGPUChainedStruct const * nextInChain;
    WGPUShaderModule module;
    char const * entryPoint;
    uint32_t constantCount;
    WGPUConstantEntry const * constants;
    uint32_t bufferCount;
    WGPUVertexBufferLayout const * buffers;
} WGPUVertexState;

typedef struct WGPUFragmentState {
    WGPUChainedStruct const * nextInChain;
    WGPUShaderModule module;
    char const * entryPoint;
    uint32_t constantCount;
    WGPUConstantEntry const * constants;
    uint32_t targetCount;
    WGPUColorTargetState const * targets;
} WGPUFragmentState;

typedef struct WGPURenderPipelineDescriptor {
    WGPUChainedStruct const * nextInChain;
    char const * label; // nullable
    WGPUPipelineLayout layout; // nullable
    WGPUVertexState vertex;
    WGPUPrimitiveState primitive;
    WGPUDepthStencilState const * depthStencil; // nullable
    WGPUMultisampleState multisample;
    WGPUFragmentState const * fragment; // nullable
} WGPURenderPipelineDescriptor;

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*WGPUBufferMapCallback)(WGPUBufferMapAsyncStatus status, void * userdata);
typedef void (*WGPUCompilationInfoCallback)(WGPUCompilationInfoRequestStatus status, WGPUCompilationInfo const * compilationInfo, void * userdata);
typedef void (*WGPUCreateComputePipelineAsyncCallback)(WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline pipeline, char const * message, void * userdata);
typedef void (*WGPUCreateRenderPipelineAsyncCallback)(WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline pipeline, char const * message, void * userdata);
typedef void (*WGPUDeviceLostCallback)(WGPUDeviceLostReason reason, char const * message, void * userdata);
typedef void (*WGPUErrorCallback)(WGPUErrorType type, char const * message, void * userdata);
typedef void (*WGPUProc)(void);
typedef void (*WGPUQueueWorkDoneCallback)(WGPUQueueWorkDoneStatus status, void * userdata);
typedef void (*WGPURequestAdapterCallback)(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * userdata);
typedef void (*WGPURequestDeviceCallback)(WGPURequestDeviceStatus status, WGPUDevice device, char const * message, void * userdata);

#if !defined(WGPU_SKIP_PROCS)

typedef WGPUInstance (*WGPUProcCreateInstance)(WGPUInstanceDescriptor const * descriptor);
typedef WGPUProc (*WGPUProcGetProcAddress)(WGPUDevice device, char const * procName);

// Procs of Adapter
typedef size_t (*WGPUProcAdapterEnumerateFeatures)(WGPUAdapter adapter, WGPUFeatureName * features);
typedef bool (*WGPUProcAdapterGetLimits)(WGPUAdapter adapter, WGPUSupportedLimits * limits);
typedef void (*WGPUProcAdapterGetProperties)(WGPUAdapter adapter, WGPUAdapterProperties * properties);
typedef bool (*WGPUProcAdapterHasFeature)(WGPUAdapter adapter, WGPUFeatureName feature);
typedef void (*WGPUProcAdapterRequestDevice)(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor, WGPURequestDeviceCallback callback, void * userdata);
typedef void (*WGPUProcAdapterReference)(WGPUAdapter adapter);
typedef void (*WGPUProcAdapterRelease)(WGPUAdapter adapter);

// Procs of BindGroup
typedef void (*WGPUProcBindGroupSetLabel)(WGPUBindGroup bindGroup, char const * label);
typedef void (*WGPUProcBindGroupReference)(WGPUBindGroup bindGroup);
typedef void (*WGPUProcBindGroupRelease)(WGPUBindGroup bindGroup);

// Procs of BindGroupLayout
typedef void (*WGPUProcBindGroupLayoutSetLabel)(WGPUBindGroupLayout bindGroupLayout, char const * label);
typedef void (*WGPUProcBindGroupLayoutReference)(WGPUBindGroupLayout bindGroupLayout);
typedef void (*WGPUProcBindGroupLayoutRelease)(WGPUBindGroupLayout bindGroupLayout);

// Procs of Buffer
typedef void (*WGPUProcBufferDestroy)(WGPUBuffer buffer);
typedef void const * (*WGPUProcBufferGetConstMappedRange)(WGPUBuffer buffer, size_t offset, size_t size);
typedef void * (*WGPUProcBufferGetMappedRange)(WGPUBuffer buffer, size_t offset, size_t size);
typedef uint64_t (*WGPUProcBufferGetSize)(WGPUBuffer buffer);
typedef WGPUBufferUsage (*WGPUProcBufferGetUsage)(WGPUBuffer buffer);
typedef void (*WGPUProcBufferMapAsync)(WGPUBuffer buffer, WGPUMapModeFlags mode, size_t offset, size_t size, WGPUBufferMapCallback callback, void * userdata);
typedef void (*WGPUProcBufferSetLabel)(WGPUBuffer buffer, char const * label);
typedef void (*WGPUProcBufferUnmap)(WGPUBuffer buffer);
typedef void (*WGPUProcBufferReference)(WGPUBuffer buffer);
typedef void (*WGPUProcBufferRelease)(WGPUBuffer buffer);

// Procs of CommandBuffer
typedef void (*WGPUProcCommandBufferSetLabel)(WGPUCommandBuffer commandBuffer, char const * label);
typedef void (*WGPUProcCommandBufferReference)(WGPUCommandBuffer commandBuffer);
typedef void (*WGPUProcCommandBufferRelease)(WGPUCommandBuffer commandBuffer);

// Procs of CommandEncoder
typedef WGPUComputePassEncoder (*WGPUProcCommandEncoderBeginComputePass)(WGPUCommandEncoder commandEncoder, WGPUComputePassDescriptor const * descriptor /* nullable */);
typedef WGPURenderPassEncoder (*WGPUProcCommandEncoderBeginRenderPass)(WGPUCommandEncoder commandEncoder, WGPURenderPassDescriptor const * descriptor);
typedef void (*WGPUProcCommandEncoderClearBuffer)(WGPUCommandEncoder commandEncoder, WGPUBuffer buffer, uint64_t offset, uint64_t size);
typedef void (*WGPUProcCommandEncoderCopyBufferToBuffer)(WGPUCommandEncoder commandEncoder, WGPUBuffer source, uint64_t sourceOffset, WGPUBuffer destination, uint64_t destinationOffset, uint64_t size);
typedef void (*WGPUProcCommandEncoderCopyBufferToTexture)(WGPUCommandEncoder commandEncoder, WGPUImageCopyBuffer const * source, WGPUImageCopyTexture const * destination, WGPUExtent3D const * copySize);
typedef void (*WGPUProcCommandEncoderCopyTextureToBuffer)(WGPUCommandEncoder commandEncoder, WGPUImageCopyTexture const * source, WGPUImageCopyBuffer const * destination, WGPUExtent3D const * copySize);
typedef void (*WGPUProcCommandEncoderCopyTextureToTexture)(WGPUCommandEncoder commandEncoder, WGPUImageCopyTexture const * source, WGPUImageCopyTexture const * destination, WGPUExtent3D const * copySize);
typedef WGPUCommandBuffer (*WGPUProcCommandEncoderFinish)(WGPUCommandEncoder commandEncoder, WGPUCommandBufferDescriptor const * descriptor /* nullable */);
typedef void (*WGPUProcCommandEncoderInsertDebugMarker)(WGPUCommandEncoder commandEncoder, char const * markerLabel);
typedef void (*WGPUProcCommandEncoderPopDebugGroup)(WGPUCommandEncoder commandEncoder);
typedef void (*WGPUProcCommandEncoderPushDebugGroup)(WGPUCommandEncoder commandEncoder, char const * groupLabel);
typedef void (*WGPUProcCommandEncoderResolveQuerySet)(WGPUCommandEncoder commandEncoder, WGPUQuerySet querySet, uint32_t firstQuery, uint32_t queryCount, WGPUBuffer destination, uint64_t destinationOffset);
typedef void (*WGPUProcCommandEncoderSetLabel)(WGPUCommandEncoder commandEncoder, char const * label);
typedef void (*WGPUProcCommandEncoderWriteTimestamp)(WGPUCommandEncoder commandEncoder, WGPUQuerySet querySet, uint32_t queryIndex);
typedef void (*WGPUProcCommandEncoderReference)(WGPUCommandEncoder commandEncoder);
typedef void (*WGPUProcCommandEncoderRelease)(WGPUCommandEncoder commandEncoder);

// Procs of ComputePassEncoder
typedef void (*WGPUProcComputePassEncoderBeginPipelineStatisticsQuery)(WGPUComputePassEncoder computePassEncoder, WGPUQuerySet querySet, uint32_t queryIndex);
typedef void (*WGPUProcComputePassEncoderDispatchWorkgroups)(WGPUComputePassEncoder computePassEncoder, uint32_t workgroupCountX, uint32_t workgroupCountY, uint32_t workgroupCountZ);
typedef void (*WGPUProcComputePassEncoderDispatchWorkgroupsIndirect)(WGPUComputePassEncoder computePassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset);
typedef void (*WGPUProcComputePassEncoderEnd)(WGPUComputePassEncoder computePassEncoder);
typedef void (*WGPUProcComputePassEncoderEndPipelineStatisticsQuery)(WGPUComputePassEncoder computePassEncoder);
typedef void (*WGPUProcComputePassEncoderInsertDebugMarker)(WGPUComputePassEncoder computePassEncoder, char const * markerLabel);
typedef void (*WGPUProcComputePassEncoderPopDebugGroup)(WGPUComputePassEncoder computePassEncoder);
typedef void (*WGPUProcComputePassEncoderPushDebugGroup)(WGPUComputePassEncoder computePassEncoder, char const * groupLabel);
typedef void (*WGPUProcComputePassEncoderSetBindGroup)(WGPUComputePassEncoder computePassEncoder, uint32_t groupIndex, WGPUBindGroup group, uint32_t dynamicOffsetCount, uint32_t const * dynamicOffsets);
typedef void (*WGPUProcComputePassEncoderSetLabel)(WGPUComputePassEncoder computePassEncoder, char const * label);
typedef void (*WGPUProcComputePassEncoderSetPipeline)(WGPUComputePassEncoder computePassEncoder, WGPUComputePipeline pipeline);
typedef void (*WGPUProcComputePassEncoderWriteTimestamp)(WGPUComputePassEncoder computePassEncoder, WGPUQuerySet querySet, uint32_t queryIndex);
typedef void (*WGPUProcComputePassEncoderReference)(WGPUComputePassEncoder computePassEncoder);
typedef void (*WGPUProcComputePassEncoderRelease)(WGPUComputePassEncoder computePassEncoder);

// Procs of ComputePipeline
typedef WGPUBindGroupLayout (*WGPUProcComputePipelineGetBindGroupLayout)(WGPUComputePipeline computePipeline, uint32_t groupIndex);
typedef void (*WGPUProcComputePipelineSetLabel)(WGPUComputePipeline computePipeline, char const * label);
typedef void (*WGPUProcComputePipelineReference)(WGPUComputePipeline computePipeline);
typedef void (*WGPUProcComputePipelineRelease)(WGPUComputePipeline computePipeline);

// Procs of Device
typedef WGPUBindGroup (*WGPUProcDeviceCreateBindGroup)(WGPUDevice device, WGPUBindGroupDescriptor const * descriptor);
typedef WGPUBindGroupLayout (*WGPUProcDeviceCreateBindGroupLayout)(WGPUDevice device, WGPUBindGroupLayoutDescriptor const * descriptor);
typedef WGPUBuffer (*WGPUProcDeviceCreateBuffer)(WGPUDevice device, WGPUBufferDescriptor const * descriptor);
typedef WGPUCommandEncoder (*WGPUProcDeviceCreateCommandEncoder)(WGPUDevice device, WGPUCommandEncoderDescriptor const * descriptor /* nullable */);
typedef WGPUComputePipeline (*WGPUProcDeviceCreateComputePipeline)(WGPUDevice device, WGPUComputePipelineDescriptor const * descriptor);
typedef void (*WGPUProcDeviceCreateComputePipelineAsync)(WGPUDevice device, WGPUComputePipelineDescriptor const * descriptor, WGPUCreateComputePipelineAsyncCallback callback, void * userdata);
typedef WGPUPipelineLayout (*WGPUProcDeviceCreatePipelineLayout)(WGPUDevice device, WGPUPipelineLayoutDescriptor const * descriptor);
typedef WGPUQuerySet (*WGPUProcDeviceCreateQuerySet)(WGPUDevice device, WGPUQuerySetDescriptor const * descriptor);
typedef WGPURenderBundleEncoder (*WGPUProcDeviceCreateRenderBundleEncoder)(WGPUDevice device, WGPURenderBundleEncoderDescriptor const * descriptor);
typedef WGPURenderPipeline (*WGPUProcDeviceCreateRenderPipeline)(WGPUDevice device, WGPURenderPipelineDescriptor const * descriptor);
typedef void (*WGPUProcDeviceCreateRenderPipelineAsync)(WGPUDevice device, WGPURenderPipelineDescriptor const * descriptor, WGPUCreateRenderPipelineAsyncCallback callback, void * userdata);
typedef WGPUSampler (*WGPUProcDeviceCreateSampler)(WGPUDevice device, WGPUSamplerDescriptor const * descriptor /* nullable */);
typedef WGPUShaderModule (*WGPUProcDeviceCreateShaderModule)(WGPUDevice device, WGPUShaderModuleDescriptor const * descriptor);
typedef WGPUSwapChain (*WGPUProcDeviceCreateSwapChain)(WGPUDevice device, WGPUSurface surface, WGPUSwapChainDescriptor const * descriptor);
typedef WGPUTexture (*WGPUProcDeviceCreateTexture)(WGPUDevice device, WGPUTextureDescriptor const * descriptor);
typedef void (*WGPUProcDeviceDestroy)(WGPUDevice device);
typedef size_t (*WGPUProcDeviceEnumerateFeatures)(WGPUDevice device, WGPUFeatureName * features);
typedef bool (*WGPUProcDeviceGetLimits)(WGPUDevice device, WGPUSupportedLimits * limits);
typedef WGPUQueue (*WGPUProcDeviceGetQueue)(WGPUDevice device);
typedef bool (*WGPUProcDeviceHasFeature)(WGPUDevice device, WGPUFeatureName feature);
typedef bool (*WGPUProcDevicePopErrorScope)(WGPUDevice device, WGPUErrorCallback callback, void * userdata);
typedef void (*WGPUProcDevicePushErrorScope)(WGPUDevice device, WGPUErrorFilter filter);
typedef void (*WGPUProcDeviceSetDeviceLostCallback)(WGPUDevice device, WGPUDeviceLostCallback callback, void * userdata);
typedef void (*WGPUProcDeviceSetLabel)(WGPUDevice device, char const * label);
typedef void (*WGPUProcDeviceSetUncapturedErrorCallback)(WGPUDevice device, WGPUErrorCallback callback, void * userdata);
typedef void (*WGPUProcDeviceReference)(WGPUDevice device);
typedef void (*WGPUProcDeviceRelease)(WGPUDevice device);

// Procs of Instance
typedef WGPUSurface (*WGPUProcInstanceCreateSurface)(WGPUInstance instance, WGPUSurfaceDescriptor const * descriptor);
typedef void (*WGPUProcInstanceProcessEvents)(WGPUInstance instance);
typedef void (*WGPUProcInstanceRequestAdapter)(WGPUInstance instance, WGPURequestAdapterOptions const * options, WGPURequestAdapterCallback callback, void * userdata);
typedef void (*WGPUProcInstanceReference)(WGPUInstance instance);
typedef void (*WGPUProcInstanceRelease)(WGPUInstance instance);

// Procs of PipelineLayout
typedef void (*WGPUProcPipelineLayoutSetLabel)(WGPUPipelineLayout pipelineLayout, char const * label);
typedef void (*WGPUProcPipelineLayoutReference)(WGPUPipelineLayout pipelineLayout);
typedef void (*WGPUProcPipelineLayoutRelease)(WGPUPipelineLayout pipelineLayout);

// Procs of QuerySet
typedef void (*WGPUProcQuerySetDestroy)(WGPUQuerySet querySet);
typedef uint32_t (*WGPUProcQuerySetGetCount)(WGPUQuerySet querySet);
typedef WGPUQueryType (*WGPUProcQuerySetGetType)(WGPUQuerySet querySet);
typedef void (*WGPUProcQuerySetSetLabel)(WGPUQuerySet querySet, char const * label);
typedef void (*WGPUProcQuerySetReference)(WGPUQuerySet querySet);
typedef void (*WGPUProcQuerySetRelease)(WGPUQuerySet querySet);

// Procs of Queue
typedef void (*WGPUProcQueueOnSubmittedWorkDone)(WGPUQueue queue, uint64_t signalValue, WGPUQueueWorkDoneCallback callback, void * userdata);
typedef void (*WGPUProcQueueSetLabel)(WGPUQueue queue, char const * label);
typedef void (*WGPUProcQueueSubmit)(WGPUQueue queue, uint32_t commandCount, WGPUCommandBuffer const * commands);
typedef void (*WGPUProcQueueWriteBuffer)(WGPUQueue queue, WGPUBuffer buffer, uint64_t bufferOffset, void const * data, size_t size);
typedef void (*WGPUProcQueueWriteTexture)(WGPUQueue queue, WGPUImageCopyTexture const * destination, void const * data, size_t dataSize, WGPUTextureDataLayout const * dataLayout, WGPUExtent3D const * writeSize);
typedef void (*WGPUProcQueueReference)(WGPUQueue queue);
typedef void (*WGPUProcQueueRelease)(WGPUQueue queue);

// Procs of RenderBundle
typedef void (*WGPUProcRenderBundleReference)(WGPURenderBundle renderBundle);
typedef void (*WGPUProcRenderBundleRelease)(WGPURenderBundle renderBundle);

// Procs of RenderBundleEncoder
typedef void (*WGPUProcRenderBundleEncoderDraw)(WGPURenderBundleEncoder renderBundleEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
typedef void (*WGPUProcRenderBundleEncoderDrawIndexed)(WGPURenderBundleEncoder renderBundleEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance);
typedef void (*WGPUProcRenderBundleEncoderDrawIndexedIndirect)(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset);
typedef void (*WGPUProcRenderBundleEncoderDrawIndirect)(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset);
typedef WGPURenderBundle (*WGPUProcRenderBundleEncoderFinish)(WGPURenderBundleEncoder renderBundleEncoder, WGPURenderBundleDescriptor const * descriptor /* nullable */);
typedef void (*WGPUProcRenderBundleEncoderInsertDebugMarker)(WGPURenderBundleEncoder renderBundleEncoder, char const * markerLabel);
typedef void (*WGPUProcRenderBundleEncoderPopDebugGroup)(WGPURenderBundleEncoder renderBundleEncoder);
typedef void (*WGPUProcRenderBundleEncoderPushDebugGroup)(WGPURenderBundleEncoder renderBundleEncoder, char const * groupLabel);
typedef void (*WGPUProcRenderBundleEncoderSetBindGroup)(WGPURenderBundleEncoder renderBundleEncoder, uint32_t groupIndex, WGPUBindGroup group, uint32_t dynamicOffsetCount, uint32_t const * dynamicOffsets);
typedef void (*WGPUProcRenderBundleEncoderSetIndexBuffer)(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer buffer, WGPUIndexFormat format, uint64_t offset, uint64_t size);
typedef void (*WGPUProcRenderBundleEncoderSetLabel)(WGPURenderBundleEncoder renderBundleEncoder, char const * label);
typedef void (*WGPUProcRenderBundleEncoderSetPipeline)(WGPURenderBundleEncoder renderBundleEncoder, WGPURenderPipeline pipeline);
typedef void (*WGPUProcRenderBundleEncoderSetVertexBuffer)(WGPURenderBundleEncoder renderBundleEncoder, uint32_t slot, WGPUBuffer buffer, uint64_t offset, uint64_t size);
typedef void (*WGPUProcRenderBundleEncoderReference)(WGPURenderBundleEncoder renderBundleEncoder);
typedef void (*WGPUProcRenderBundleEncoderRelease)(WGPURenderBundleEncoder renderBundleEncoder);

// Procs of RenderPassEncoder
typedef void (*WGPUProcRenderPassEncoderBeginOcclusionQuery)(WGPURenderPassEncoder renderPassEncoder, uint32_t queryIndex);
typedef void (*WGPUProcRenderPassEncoderBeginPipelineStatisticsQuery)(WGPURenderPassEncoder renderPassEncoder, WGPUQuerySet querySet, uint32_t queryIndex);
typedef void (*WGPUProcRenderPassEncoderDraw)(WGPURenderPassEncoder renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
typedef void (*WGPUProcRenderPassEncoderDrawIndexed)(WGPURenderPassEncoder renderPassEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance);
typedef void (*WGPUProcRenderPassEncoderDrawIndexedIndirect)(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset);
typedef void (*WGPUProcRenderPassEncoderDrawIndirect)(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset);
typedef void (*WGPUProcRenderPassEncoderEnd)(WGPURenderPassEncoder renderPassEncoder);
typedef void (*WGPUProcRenderPassEncoderEndOcclusionQuery)(WGPURenderPassEncoder renderPassEncoder);
typedef void (*WGPUProcRenderPassEncoderEndPipelineStatisticsQuery)(WGPURenderPassEncoder renderPassEncoder);
typedef void (*WGPUProcRenderPassEncoderExecuteBundles)(WGPURenderPassEncoder renderPassEncoder, uint32_t bundlesCount, WGPURenderBundle const * bundles);
typedef void (*WGPUProcRenderPassEncoderInsertDebugMarker)(WGPURenderPassEncoder renderPassEncoder, char const * markerLabel);
typedef void (*WGPUProcRenderPassEncoderPopDebugGroup)(WGPURenderPassEncoder renderPassEncoder);
typedef void (*WGPUProcRenderPassEncoderPushDebugGroup)(WGPURenderPassEncoder renderPassEncoder, char const * groupLabel);
typedef void (*WGPUProcRenderPassEncoderSetBindGroup)(WGPURenderPassEncoder renderPassEncoder, uint32_t groupIndex, WGPUBindGroup group, uint32_t dynamicOffsetCount, uint32_t const * dynamicOffsets);
typedef void (*WGPUProcRenderPassEncoderSetBlendConstant)(WGPURenderPassEncoder renderPassEncoder, WGPUColor const * color);
typedef void (*WGPUProcRenderPassEncoderSetIndexBuffer)(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer buffer, WGPUIndexFormat format, uint64_t offset, uint64_t size);
typedef void (*WGPUProcRenderPassEncoderSetLabel)(WGPURenderPassEncoder renderPassEncoder, char const * label);
typedef void (*WGPUProcRenderPassEncoderSetPipeline)(WGPURenderPassEncoder renderPassEncoder, WGPURenderPipeline pipeline);
typedef void (*WGPUProcRenderPassEncoderSetScissorRect)(WGPURenderPassEncoder renderPassEncoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef void (*WGPUProcRenderPassEncoderSetStencilReference)(WGPURenderPassEncoder renderPassEncoder, uint32_t reference);
typedef void (*WGPUProcRenderPassEncoderSetVertexBuffer)(WGPURenderPassEncoder renderPassEncoder, uint32_t slot, WGPUBuffer buffer, uint64_t offset, uint64_t size);
typedef void (*WGPUProcRenderPassEncoderSetViewport)(WGPURenderPassEncoder renderPassEncoder, float x, float y, float width, float height, float minDepth, float maxDepth);
typedef void (*WGPUProcRenderPassEncoderWriteTimestamp)(WGPURenderPassEncoder renderPassEncoder, WGPUQuerySet querySet, uint32_t queryIndex);
typedef void (*WGPUProcRenderPassEncoderReference)(WGPURenderPassEncoder renderPassEncoder);
typedef void (*WGPUProcRenderPassEncoderRelease)(WGPURenderPassEncoder renderPassEncoder);

// Procs of RenderPipeline
typedef WGPUBindGroupLayout (*WGPUProcRenderPipelineGetBindGroupLayout)(WGPURenderPipeline renderPipeline, uint32_t groupIndex);
typedef void (*WGPUProcRenderPipelineSetLabel)(WGPURenderPipeline renderPipeline, char const * label);
typedef void (*WGPUProcRenderPipelineReference)(WGPURenderPipeline renderPipeline);
typedef void (*WGPUProcRenderPipelineRelease)(WGPURenderPipeline renderPipeline);

// Procs of Sampler
typedef void (*WGPUProcSamplerSetLabel)(WGPUSampler sampler, char const * label);
typedef void (*WGPUProcSamplerReference)(WGPUSampler sampler);
typedef void (*WGPUProcSamplerRelease)(WGPUSampler sampler);

// Procs of ShaderModule
typedef void (*WGPUProcShaderModuleGetCompilationInfo)(WGPUShaderModule shaderModule, WGPUCompilationInfoCallback callback, void * userdata);
typedef void (*WGPUProcShaderModuleSetLabel)(WGPUShaderModule shaderModule, char const * label);
typedef void (*WGPUProcShaderModuleReference)(WGPUShaderModule shaderModule);
typedef void (*WGPUProcShaderModuleRelease)(WGPUShaderModule shaderModule);

// Procs of Surface
typedef WGPUTextureFormat (*WGPUProcSurfaceGetPreferredFormat)(WGPUSurface surface, WGPUAdapter adapter);
typedef void (*WGPUProcSurfaceReference)(WGPUSurface surface);
typedef void (*WGPUProcSurfaceRelease)(WGPUSurface surface);

// Procs of SwapChain
typedef WGPUTextureView (*WGPUProcSwapChainGetCurrentTextureView)(WGPUSwapChain swapChain);
typedef void (*WGPUProcSwapChainPresent)(WGPUSwapChain swapChain);
typedef void (*WGPUProcSwapChainReference)(WGPUSwapChain swapChain);
typedef void (*WGPUProcSwapChainRelease)(WGPUSwapChain swapChain);

// Procs of Texture
typedef WGPUTextureView (*WGPUProcTextureCreateView)(WGPUTexture texture, WGPUTextureViewDescriptor const * descriptor /* nullable */);
typedef void (*WGPUProcTextureDestroy)(WGPUTexture texture);
typedef uint32_t (*WGPUProcTextureGetDepthOrArrayLayers)(WGPUTexture texture);
typedef WGPUTextureDimension (*WGPUProcTextureGetDimension)(WGPUTexture texture);
typedef WGPUTextureFormat (*WGPUProcTextureGetFormat)(WGPUTexture texture);
typedef uint32_t (*WGPUProcTextureGetHeight)(WGPUTexture texture);
typedef uint32_t (*WGPUProcTextureGetMipLevelCount)(WGPUTexture texture);
typedef uint32_t (*WGPUProcTextureGetSampleCount)(WGPUTexture texture);
typedef WGPUTextureUsage (*WGPUProcTextureGetUsage)(WGPUTexture texture);
typedef uint32_t (*WGPUProcTextureGetWidth)(WGPUTexture texture);
typedef void (*WGPUProcTextureSetLabel)(WGPUTexture texture, char const * label);
typedef void (*WGPUProcTextureReference)(WGPUTexture texture);
typedef void (*WGPUProcTextureRelease)(WGPUTexture texture);

// Procs of TextureView
typedef void (*WGPUProcTextureViewSetLabel)(WGPUTextureView textureView, char const * label);
typedef void (*WGPUProcTextureViewReference)(WGPUTextureView textureView);
typedef void (*WGPUProcTextureViewRelease)(WGPUTextureView textureView);

#endif  // !defined(WGPU_SKIP_PROCS)

#if !defined(WGPU_SKIP_DECLARATIONS)

WGPU_EXPORT WGPUInstance wgpuCreateInstance(WGPUInstanceDescriptor const * descriptor);
WGPU_EXPORT WGPUProc wgpuGetProcAddress(WGPUDevice device, char const * procName);

// Methods of Adapter
WGPU_EXPORT size_t wgpuAdapterEnumerateFeatures(WGPUAdapter adapter, WGPUFeatureName * features);
WGPU_EXPORT bool wgpuAdapterGetLimits(WGPUAdapter adapter, WGPUSupportedLimits * limits);
WGPU_EXPORT void wgpuAdapterGetProperties(WGPUAdapter adapter, WGPUAdapterProperties * properties);
WGPU_EXPORT bool wgpuAdapterHasFeature(WGPUAdapter adapter, WGPUFeatureName feature);
WGPU_EXPORT void wgpuAdapterRequestDevice(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor, WGPURequestDeviceCallback callback, void * userdata);
WGPU_EXPORT void wgpuAdapterReference(WGPUAdapter adapter);
WGPU_EXPORT void wgpuAdapterRelease(WGPUAdapter adapter);

// Methods of BindGroup
WGPU_EXPORT void wgpuBindGroupSetLabel(WGPUBindGroup bindGroup, char const * label);
WGPU_EXPORT void wgpuBindGroupReference(WGPUBindGroup bindGroup);
WGPU_EXPORT void wgpuBindGroupRelease(WGPUBindGroup bindGroup);

// Methods of BindGroupLayout
WGPU_EXPORT void wgpuBindGroupLayoutSetLabel(WGPUBindGroupLayout bindGroupLayout, char const * label);
WGPU_EXPORT void wgpuBindGroupLayoutReference(WGPUBindGroupLayout bindGroupLayout);
WGPU_EXPORT void wgpuBindGroupLayoutRelease(WGPUBindGroupLayout bindGroupLayout);

// Methods of Buffer
WGPU_EXPORT void wgpuBufferDestroy(WGPUBuffer buffer);
WGPU_EXPORT void const * wgpuBufferGetConstMappedRange(WGPUBuffer buffer, size_t offset, size_t size);
WGPU_EXPORT void * wgpuBufferGetMappedRange(WGPUBuffer buffer, size_t offset, size_t size);
WGPU_EXPORT uint64_t wgpuBufferGetSize(WGPUBuffer buffer);
WGPU_EXPORT WGPUBufferUsage wgpuBufferGetUsage(WGPUBuffer buffer);
WGPU_EXPORT void wgpuBufferMapAsync(WGPUBuffer buffer, WGPUMapModeFlags mode, size_t offset, size_t size, WGPUBufferMapCallback callback, void * userdata);
WGPU_EXPORT void wgpuBufferSetLabel(WGPUBuffer buffer, char const * label);
WGPU_EXPORT void wgpuBufferUnmap(WGPUBuffer buffer);
WGPU_EXPORT void wgpuBufferReference(WGPUBuffer buffer);
WGPU_EXPORT void wgpuBufferRelease(WGPUBuffer buffer);

// Methods of CommandBuffer
WGPU_EXPORT void wgpuCommandBufferSetLabel(WGPUCommandBuffer commandBuffer, char const * label);
WGPU_EXPORT void wgpuCommandBufferReference(WGPUCommandBuffer commandBuffer);
WGPU_EXPORT void wgpuCommandBufferRelease(WGPUCommandBuffer commandBuffer);

// Methods of CommandEncoder
WGPU_EXPORT WGPUComputePassEncoder wgpuCommandEncoderBeginComputePass(WGPUCommandEncoder commandEncoder, WGPUComputePassDescriptor const * descriptor /* nullable */);
WGPU_EXPORT WGPURenderPassEncoder wgpuCommandEncoderBeginRenderPass(WGPUCommandEncoder commandEncoder, WGPURenderPassDescriptor const * descriptor);
WGPU_EXPORT void wgpuCommandEncoderClearBuffer(WGPUCommandEncoder commandEncoder, WGPUBuffer buffer, uint64_t offset, uint64_t size);
WGPU_EXPORT void wgpuCommandEncoderCopyBufferToBuffer(WGPUCommandEncoder commandEncoder, WGPUBuffer source, uint64_t sourceOffset, WGPUBuffer destination, uint64_t destinationOffset, uint64_t size);
WGPU_EXPORT void wgpuCommandEncoderCopyBufferToTexture(WGPUCommandEncoder commandEncoder, WGPUImageCopyBuffer const * source, WGPUImageCopyTexture const * destination, WGPUExtent3D const * copySize);
WGPU_EXPORT void wgpuCommandEncoderCopyTextureToBuffer(WGPUCommandEncoder commandEncoder, WGPUImageCopyTexture const * source, WGPUImageCopyBuffer const * destination, WGPUExtent3D const * copySize);
WGPU_EXPORT void wgpuCommandEncoderCopyTextureToTexture(WGPUCommandEncoder commandEncoder, WGPUImageCopyTexture const * source, WGPUImageCopyTexture const * destination, WGPUExtent3D const * copySize);
WGPU_EXPORT WGPUCommandBuffer wgpuCommandEncoderFinish(WGPUCommandEncoder commandEncoder, WGPUCommandBufferDescriptor const * descriptor /* nullable */);
WGPU_EXPORT void wgpuCommandEncoderInsertDebugMarker(WGPUCommandEncoder commandEncoder, char const * markerLabel);
WGPU_EXPORT void wgpuCommandEncoderPopDebugGroup(WGPUCommandEncoder commandEncoder);
WGPU_EXPORT void wgpuCommandEncoderPushDebugGroup(WGPUCommandEncoder commandEncoder, char const * groupLabel);
WGPU_EXPORT void wgpuCommandEncoderResolveQuerySet(WGPUCommandEncoder commandEncoder, WGPUQuerySet querySet, uint32_t firstQuery, uint32_t queryCount, WGPUBuffer destination, uint64_t destinationOffset);
WGPU_EXPORT void wgpuCommandEncoderSetLabel(WGPUCommandEncoder commandEncoder, char const * label);
WGPU_EXPORT void wgpuCommandEncoderWriteTimestamp(WGPUCommandEncoder commandEncoder, WGPUQuerySet querySet, uint32_t queryIndex);
WGPU_EXPORT void wgpuCommandEncoderReference(WGPUCommandEncoder commandEncoder);
WGPU_EXPORT void wgpuCommandEncoderRelease(WGPUCommandEncoder commandEncoder);

// Methods of ComputePassEncoder
WGPU_EXPORT void wgpuComputePassEncoderBeginPipelineStatisticsQuery(WGPUComputePassEncoder computePassEncoder, WGPUQuerySet querySet, uint32_t queryIndex);
WGPU_EXPORT void wgpuComputePassEncoderDispatchWorkgroups(WGPUComputePassEncoder computePassEncoder, uint32_t workgroupCountX, uint32_t workgroupCountY, uint32_t workgroupCountZ);
WGPU_EXPORT void wgpuComputePassEncoderDispatchWorkgroupsIndirect(WGPUComputePassEncoder computePassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset);
WGPU_EXPORT void wgpuComputePassEncoderEnd(WGPUComputePassEncoder computePassEncoder);
WGPU_EXPORT void wgpuComputePassEncoderEndPipelineStatisticsQuery(WGPUComputePassEncoder computePassEncoder);
WGPU_EXPORT void wgpuComputePassEncoderInsertDebugMarker(WGPUComputePassEncoder computePassEncoder, char const * markerLabel);
WGPU_EXPORT void wgpuComputePassEncoderPopDebugGroup(WGPUComputePassEncoder computePassEncoder);
WGPU_EXPORT void wgpuComputePassEncoderPushDebugGroup(WGPUComputePassEncoder computePassEncoder, char const * groupLabel);
WGPU_EXPORT void wgpuComputePassEncoderSetBindGroup(WGPUComputePassEncoder computePassEncoder, uint32_t groupIndex, WGPUBindGroup group, uint32_t dynamicOffsetCount, uint32_t const * dynamicOffsets);
WGPU_EXPORT void wgpuComputePassEncoderSetLabel(WGPUComputePassEncoder computePassEncoder, char const * label);
WGPU_EXPORT void wgpuComputePassEncoderSetPipeline(WGPUComputePassEncoder computePassEncoder, WGPUComputePipeline pipeline);
WGPU_EXPORT void wgpuComputePassEncoderWriteTimestamp(WGPUComputePassEncoder computePassEncoder, WGPUQuerySet querySet, uint32_t queryIndex);
WGPU_EXPORT void wgpuComputePassEncoderReference(WGPUComputePassEncoder computePassEncoder);
WGPU_EXPORT void wgpuComputePassEncoderRelease(WGPUComputePassEncoder computePassEncoder);

// Methods of ComputePipeline
WGPU_EXPORT WGPUBindGroupLayout wgpuComputePipelineGetBindGroupLayout(WGPUComputePipeline computePipeline, uint32_t groupIndex);
WGPU_EXPORT void wgpuComputePipelineSetLabel(WGPUComputePipeline computePipeline, char const * label);
WGPU_EXPORT void wgpuComputePipelineReference(WGPUComputePipeline computePipeline);
WGPU_EXPORT void wgpuComputePipelineRelease(WGPUComputePipeline computePipeline);

// Methods of Device
WGPU_EXPORT WGPUBindGroup wgpuDeviceCreateBindGroup(WGPUDevice device, WGPUBindGroupDescriptor const * descriptor);
WGPU_EXPORT WGPUBindGroupLayout wgpuDeviceCreateBindGroupLayout(WGPUDevice device, WGPUBindGroupLayoutDescriptor const * descriptor);
WGPU_EXPORT WGPUBuffer wgpuDeviceCreateBuffer(WGPUDevice device, WGPUBufferDescriptor const * descriptor);
WGPU_EXPORT WGPUCommandEncoder wgpuDeviceCreateCommandEncoder(WGPUDevice device, WGPUCommandEncoderDescriptor const * descriptor /* nullable */);
WGPU_EXPORT WGPUComputePipeline wgpuDeviceCreateComputePipeline(WGPUDevice device, WGPUComputePipelineDescriptor const * descriptor);
WGPU_EXPORT void wgpuDeviceCreateComputePipelineAsync(WGPUDevice device, WGPUComputePipelineDescriptor const * descriptor, WGPUCreateComputePipelineAsyncCallback callback, void * userdata);
WGPU_EXPORT WGPUPipelineLayout wgpuDeviceCreatePipelineLayout(WGPUDevice device, WGPUPipelineLayoutDescriptor const * descriptor);
WGPU_EXPORT WGPUQuerySet wgpuDeviceCreateQuerySet(WGPUDevice device, WGPUQuerySetDescriptor const * descriptor);
WGPU_EXPORT WGPURenderBundleEncoder wgpuDeviceCreateRenderBundleEncoder(WGPUDevice device, WGPURenderBundleEncoderDescriptor const * descriptor);
WGPU_EXPORT WGPURenderPipeline wgpuDeviceCreateRenderPipeline(WGPUDevice device, WGPURenderPipelineDescriptor const * descriptor);
WGPU_EXPORT void wgpuDeviceCreateRenderPipelineAsync(WGPUDevice device, WGPURenderPipelineDescriptor const * descriptor, WGPUCreateRenderPipelineAsyncCallback callback, void * userdata);
WGPU_EXPORT WGPUSampler wgpuDeviceCreateSampler(WGPUDevice device, WGPUSamplerDescriptor const * descriptor /* nullable */);
WGPU_EXPORT WGPUShaderModule wgpuDeviceCreateShaderModule(WGPUDevice device, WGPUShaderModuleDescriptor const * descriptor);
WGPU_EXPORT WGPUSwapChain wgpuDeviceCreateSwapChain(WGPUDevice device, WGPUSurface surface, WGPUSwapChainDescriptor const * descriptor);
WGPU_EXPORT WGPUTexture wgpuDeviceCreateTexture(WGPUDevice device, WGPUTextureDescriptor const * descriptor);
WGPU_EXPORT void wgpuDeviceDestroy(WGPUDevice device);
WGPU_EXPORT size_t wgpuDeviceEnumerateFeatures(WGPUDevice device, WGPUFeatureName * features);
WGPU_EXPORT bool wgpuDeviceGetLimits(WGPUDevice device, WGPUSupportedLimits * limits);
WGPU_EXPORT WGPUQueue wgpuDeviceGetQueue(WGPUDevice device);
WGPU_EXPORT bool wgpuDeviceHasFeature(WGPUDevice device, WGPUFeatureName feature);
WGPU_EXPORT bool wgpuDevicePopErrorScope(WGPUDevice device, WGPUErrorCallback callback, void * userdata);
WGPU_EXPORT void wgpuDevicePushErrorScope(WGPUDevice device, WGPUErrorFilter filter);
WGPU_EXPORT void wgpuDeviceSetDeviceLostCallback(WGPUDevice device, WGPUDeviceLostCallback callback, void * userdata);
WGPU_EXPORT void wgpuDeviceSetLabel(WGPUDevice device, char const * label);
WGPU_EXPORT void wgpuDeviceSetUncapturedErrorCallback(WGPUDevice device, WGPUErrorCallback callback, void * userdata);
WGPU_EXPORT void wgpuDeviceReference(WGPUDevice device);
WGPU_EXPORT void wgpuDeviceRelease(WGPUDevice device);

// Methods of Instance
WGPU_EXPORT WGPUSurface wgpuInstanceCreateSurface(WGPUInstance instance, WGPUSurfaceDescriptor const * descriptor);
WGPU_EXPORT void wgpuInstanceProcessEvents(WGPUInstance instance);
WGPU_EXPORT void wgpuInstanceRequestAdapter(WGPUInstance instance, WGPURequestAdapterOptions const * options, WGPURequestAdapterCallback callback, void * userdata);
WGPU_EXPORT void wgpuInstanceReference(WGPUInstance instance);
WGPU_EXPORT void wgpuInstanceRelease(WGPUInstance instance);

// Methods of PipelineLayout
WGPU_EXPORT void wgpuPipelineLayoutSetLabel(WGPUPipelineLayout pipelineLayout, char const * label);
WGPU_EXPORT void wgpuPipelineLayoutReference(WGPUPipelineLayout pipelineLayout);
WGPU_EXPORT void wgpuPipelineLayoutRelease(WGPUPipelineLayout pipelineLayout);

// Methods of QuerySet
WGPU_EXPORT void wgpuQuerySetDestroy(WGPUQuerySet querySet);
WGPU_EXPORT uint32_t wgpuQuerySetGetCount(WGPUQuerySet querySet);
WGPU_EXPORT WGPUQueryType wgpuQuerySetGetType(WGPUQuerySet querySet);
WGPU_EXPORT void wgpuQuerySetSetLabel(WGPUQuerySet querySet, char const * label);
WGPU_EXPORT void wgpuQuerySetReference(WGPUQuerySet querySet);
WGPU_EXPORT void wgpuQuerySetRelease(WGPUQuerySet querySet);

// Methods of Queue
WGPU_EXPORT void wgpuQueueOnSubmittedWorkDone(WGPUQueue queue, uint64_t signalValue, WGPUQueueWorkDoneCallback callback, void * userdata);
WGPU_EXPORT void wgpuQueueSetLabel(WGPUQueue queue, char const * label);
WGPU_EXPORT void wgpuQueueSubmit(WGPUQueue queue, uint32_t commandCount, WGPUCommandBuffer const * commands);
WGPU_EXPORT void wgpuQueueWriteBuffer(WGPUQueue queue, WGPUBuffer buffer, uint64_t bufferOffset, void const * data, size_t size);
WGPU_EXPORT void wgpuQueueWriteTexture(WGPUQueue queue, WGPUImageCopyTexture const * destination, void const * data, size_t dataSize, WGPUTextureDataLayout const * dataLayout, WGPUExtent3D const * writeSize);
WGPU_EXPORT void wgpuQueueReference(WGPUQueue queue);
WGPU_EXPORT void wgpuQueueRelease(WGPUQueue queue);

// Methods of RenderBundle
WGPU_EXPORT void wgpuRenderBundleReference(WGPURenderBundle renderBundle);
WGPU_EXPORT void wgpuRenderBundleRelease(WGPURenderBundle renderBundle);

// Methods of RenderBundleEncoder
WGPU_EXPORT void wgpuRenderBundleEncoderDraw(WGPURenderBundleEncoder renderBundleEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
WGPU_EXPORT void wgpuRenderBundleEncoderDrawIndexed(WGPURenderBundleEncoder renderBundleEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance);
WGPU_EXPORT void wgpuRenderBundleEncoderDrawIndexedIndirect(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset);
WGPU_EXPORT void wgpuRenderBundleEncoderDrawIndirect(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset);
WGPU_EXPORT WGPURenderBundle wgpuRenderBundleEncoderFinish(WGPURenderBundleEncoder renderBundleEncoder, WGPURenderBundleDescriptor const * descriptor /* nullable */);
WGPU_EXPORT void wgpuRenderBundleEncoderInsertDebugMarker(WGPURenderBundleEncoder renderBundleEncoder, char const * markerLabel);
WGPU_EXPORT void wgpuRenderBundleEncoderPopDebugGroup(WGPURenderBundleEncoder renderBundleEncoder);
WGPU_EXPORT void wgpuRenderBundleEncoderPushDebugGroup(WGPURenderBundleEncoder renderBundleEncoder, char const * groupLabel);
WGPU_EXPORT void wgpuRenderBundleEncoderSetBindGroup(WGPURenderBundleEncoder renderBundleEncoder, uint32_t groupIndex, WGPUBindGroup group, uint32_t dynamicOffsetCount, uint32_t const * dynamicOffsets);
WGPU_EXPORT void wgpuRenderBundleEncoderSetIndexBuffer(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer buffer, WGPUIndexFormat format, uint64_t offset, uint64_t size);
WGPU_EXPORT void wgpuRenderBundleEncoderSetLabel(WGPURenderBundleEncoder renderBundleEncoder, char const * label);
WGPU_EXPORT void wgpuRenderBundleEncoderSetPipeline(WGPURenderBundleEncoder renderBundleEncoder, WGPURenderPipeline pipeline);
WGPU_EXPORT void wgpuRenderBundleEncoderSetVertexBuffer(WGPURenderBundleEncoder renderBundleEncoder, uint32_t slot, WGPUBuffer buffer, uint64_t offset, uint64_t size);
WGPU_EXPORT void wgpuRenderBundleEncoderReference(WGPURenderBundleEncoder renderBundleEncoder);
WGPU_EXPORT void wgpuRenderBundleEncoderRelease(WGPURenderBundleEncoder renderBundleEncoder);

// Methods of RenderPassEncoder
WGPU_EXPORT void wgpuRenderPassEncoderBeginOcclusionQuery(WGPURenderPassEncoder renderPassEncoder, uint32_t queryIndex);
WGPU_EXPORT void wgpuRenderPassEncoderBeginPipelineStatisticsQuery(WGPURenderPassEncoder renderPassEncoder, WGPUQuerySet querySet, uint32_t queryIndex);
WGPU_EXPORT void wgpuRenderPassEncoderDraw(WGPURenderPassEncoder renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
WGPU_EXPORT void wgpuRenderPassEncoderDrawIndexed(WGPURenderPassEncoder renderPassEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance);
WGPU_EXPORT void wgpuRenderPassEncoderDrawIndexedIndirect(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset);
WGPU_EXPORT void wgpuRenderPassEncoderDrawIndirect(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset);
WGPU_EXPORT void wgpuRenderPassEncoderEnd(WGPURenderPassEncoder renderPassEncoder);
WGPU_EXPORT void wgpuRenderPassEncoderEndOcclusionQuery(WGPURenderPassEncoder renderPassEncoder);
WGPU_EXPORT void wgpuRenderPassEncoderEndPipelineStatisticsQuery(WGPURenderPassEncoder renderPassEncoder);
WGPU_EXPORT void wgpuRenderPassEncoderExecuteBundles(WGPURenderPassEncoder renderPassEncoder, uint32_t bundlesCount, WGPURenderBundle const * bundles);
WGPU_EXPORT void wgpuRenderPassEncoderInsertDebugMarker(WGPURenderPassEncoder renderPassEncoder, char const * markerLabel);
WGPU_EXPORT void wgpuRenderPassEncoderPopDebugGroup(WGPURenderPassEncoder renderPassEncoder);
WGPU_EXPORT void wgpuRenderPassEncoderPushDebugGroup(WGPURenderPassEncoder renderPassEncoder, char const * groupLabel);
WGPU_EXPORT void wgpuRenderPassEncoderSetBindGroup(WGPURenderPassEncoder renderPassEncoder, uint32_t groupIndex, WGPUBindGroup group, uint32_t dynamicOffsetCount, uint32_t const * dynamicOffsets);
WGPU_EXPORT void wgpuRenderPassEncoderSetBlendConstant(WGPURenderPassEncoder renderPassEncoder, WGPUColor const * color);
WGPU_EXPORT void wgpuRenderPassEncoderSetIndexBuffer(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer buffer, WGPUIndexFormat format, uint64_t offset, uint64_t size);
WGPU_EXPORT void wgpuRenderPassEncoderSetLabel(WGPURenderPassEncoder renderPassEncoder, char const * label);
WGPU_EXPORT void wgpuRenderPassEncoderSetPipeline(WGPURenderPassEncoder renderPassEncoder, WGPURenderPipeline pipeline);
WGPU_EXPORT void wgpuRenderPassEncoderSetScissorRect(WGPURenderPassEncoder renderPassEncoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
WGPU_EXPORT void wgpuRenderPassEncoderSetStencilReference(WGPURenderPassEncoder renderPassEncoder, uint32_t reference);
WGPU_EXPORT void wgpuRenderPassEncoderSetVertexBuffer(WGPURenderPassEncoder renderPassEncoder, uint32_t slot, WGPUBuffer buffer, uint64_t offset, uint64_t size);
WGPU_EXPORT void wgpuRenderPassEncoderSetViewport(WGPURenderPassEncoder renderPassEncoder, float x, float y, float width, float height, float minDepth, float maxDepth);
WGPU_EXPORT void wgpuRenderPassEncoderWriteTimestamp(WGPURenderPassEncoder renderPassEncoder, WGPUQuerySet querySet, uint32_t queryIndex);
WGPU_EXPORT void wgpuRenderPassEncoderReference(WGPURenderPassEncoder renderPassEncoder);
WGPU_EXPORT void wgpuRenderPassEncoderRelease(WGPURenderPassEncoder renderPassEncoder);

// Methods of RenderPipeline
WGPU_EXPORT WGPUBindGroupLayout wgpuRenderPipelineGetBindGroupLayout(WGPURenderPipeline renderPipeline, uint32_t groupIndex);
WGPU_EXPORT void wgpuRenderPipelineSetLabel(WGPURenderPipeline renderPipeline, char const * label);
WGPU_EXPORT void wgpuRenderPipelineReference(WGPURenderPipeline renderPipeline);
WGPU_EXPORT void wgpuRenderPipelineRelease(WGPURenderPipeline renderPipeline);

// Methods of Sampler
WGPU_EXPORT void wgpuSamplerSetLabel(WGPUSampler sampler, char const * label);
WGPU_EXPORT void wgpuSamplerReference(WGPUSampler sampler);
WGPU_EXPORT void wgpuSamplerRelease(WGPUSampler sampler);

// Methods of ShaderModule
WGPU_EXPORT void wgpuShaderModuleGetCompilationInfo(WGPUShaderModule shaderModule, WGPUCompilationInfoCallback callback, void * userdata);
WGPU_EXPORT void wgpuShaderModuleSetLabel(WGPUShaderModule shaderModule, char const * label);
WGPU_EXPORT void wgpuShaderModuleReference(WGPUShaderModule shaderModule);
WGPU_EXPORT void wgpuShaderModuleRelease(WGPUShaderModule shaderModule);

// Methods of Surface
WGPU_EXPORT WGPUTextureFormat wgpuSurfaceGetPreferredFormat(WGPUSurface surface, WGPUAdapter adapter);
WGPU_EXPORT void wgpuSurfaceReference(WGPUSurface surface);
WGPU_EXPORT void wgpuSurfaceRelease(WGPUSurface surface);

// Methods of SwapChain
WGPU_EXPORT WGPUTextureView wgpuSwapChainGetCurrentTextureView(WGPUSwapChain swapChain);
WGPU_EXPORT void wgpuSwapChainPresent(WGPUSwapChain swapChain);
WGPU_EXPORT void wgpuSwapChainReference(WGPUSwapChain swapChain);
WGPU_EXPORT void wgpuSwapChainRelease(WGPUSwapChain swapChain);

// Methods of Texture
WGPU_EXPORT WGPUTextureView wgpuTextureCreateView(WGPUTexture texture, WGPUTextureViewDescriptor const * descriptor /* nullable */);
WGPU_EXPORT void wgpuTextureDestroy(WGPUTexture texture);
WGPU_EXPORT uint32_t wgpuTextureGetDepthOrArrayLayers(WGPUTexture texture);
WGPU_EXPORT WGPUTextureDimension wgpuTextureGetDimension(WGPUTexture texture);
WGPU_EXPORT WGPUTextureFormat wgpuTextureGetFormat(WGPUTexture texture);
WGPU_EXPORT uint32_t wgpuTextureGetHeight(WGPUTexture texture);
WGPU_EXPORT uint32_t wgpuTextureGetMipLevelCount(WGPUTexture texture);
WGPU_EXPORT uint32_t wgpuTextureGetSampleCount(WGPUTexture texture);
WGPU_EXPORT WGPUTextureUsage wgpuTextureGetUsage(WGPUTexture texture);
WGPU_EXPORT uint32_t wgpuTextureGetWidth(WGPUTexture texture);
WGPU_EXPORT void wgpuTextureSetLabel(WGPUTexture texture, char const * label);
WGPU_EXPORT void wgpuTextureReference(WGPUTexture texture);
WGPU_EXPORT void wgpuTextureRelease(WGPUTexture texture);

// Methods of TextureView
WGPU_EXPORT void wgpuTextureViewSetLabel(WGPUTextureView textureView, char const * label);
WGPU_EXPORT void wgpuTextureViewReference(WGPUTextureView textureView);
WGPU_EXPORT void wgpuTextureViewRelease(WGPUTextureView textureView);

#endif  // !defined(WGPU_SKIP_DECLARATIONS)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // WEBGPU_H_
