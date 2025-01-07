// Copyright 2024 The Dawn & Tint Authors
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

#ifndef WEBGPU_CPP_H_
#define WEBGPU_CPP_H_

#include "webgpu/webgpu.h"
#include "webgpu/webgpu_cpp_chained_struct.h"
#include "webgpu/webgpu_enum_class_bitmasks.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace wgpu {

    namespace detail {
        constexpr size_t ConstexprMax(size_t a, size_t b) {
            return a > b ? a : b;
        }
    }  // namespace detail

    static constexpr uint32_t kArrayLayerCountUndefined = WGPU_ARRAY_LAYER_COUNT_UNDEFINED;
    static constexpr uint32_t kCopyStrideUndefined = WGPU_COPY_STRIDE_UNDEFINED;
    static constexpr uint32_t kDepthSliceUndefined = WGPU_DEPTH_SLICE_UNDEFINED;
    static constexpr uint32_t kLimitU32Undefined = WGPU_LIMIT_U32_UNDEFINED;
    static constexpr uint64_t kLimitU64Undefined = WGPU_LIMIT_U64_UNDEFINED;
    static constexpr uint32_t kMipLevelCountUndefined = WGPU_MIP_LEVEL_COUNT_UNDEFINED;
    static constexpr uint32_t kQuerySetIndexUndefined = WGPU_QUERY_SET_INDEX_UNDEFINED;
    static constexpr size_t kWholeMapSize = WGPU_WHOLE_MAP_SIZE;
    static constexpr uint64_t kWholeSize = WGPU_WHOLE_SIZE;

    enum class WGSLFeatureName : uint32_t {
        Undefined = 0x00000000,
        ReadonlyAndReadwriteStorageTextures = 0x00000001,
        Packed4x8IntegerDotProduct = 0x00000002,
        UnrestrictedPointerParameters = 0x00000003,
        PointerCompositeAccess = 0x00000004,
    };

    enum class AdapterType : uint32_t {
        DiscreteGPU = 0x00000001,
        IntegratedGPU = 0x00000002,
        CPU = 0x00000003,
        Unknown = 0x00000004,
    };

    enum class AddressMode : uint32_t {
        Undefined = 0x00000000,
        ClampToEdge = 0x00000001,
        Repeat = 0x00000002,
        MirrorRepeat = 0x00000003,
    };

    enum class BackendType : uint32_t {
        Undefined = 0x00000000,
        Null = 0x00000001,
        WebGPU = 0x00000002,
        D3D11 = 0x00000003,
        D3D12 = 0x00000004,
        Metal = 0x00000005,
        Vulkan = 0x00000006,
        OpenGL = 0x00000007,
        OpenGLES = 0x00000008,
    };

    enum class BlendFactor : uint32_t {
        Undefined = 0x00000000,
        Zero = 0x00000001,
        One = 0x00000002,
        Src = 0x00000003,
        OneMinusSrc = 0x00000004,
        SrcAlpha = 0x00000005,
        OneMinusSrcAlpha = 0x00000006,
        Dst = 0x00000007,
        OneMinusDst = 0x00000008,
        DstAlpha = 0x00000009,
        OneMinusDstAlpha = 0x0000000A,
        SrcAlphaSaturated = 0x0000000B,
        Constant = 0x0000000C,
        OneMinusConstant = 0x0000000D,
    };

    enum class BlendOperation : uint32_t {
        Undefined = 0x00000000,
        Add = 0x00000001,
        Subtract = 0x00000002,
        ReverseSubtract = 0x00000003,
        Min = 0x00000004,
        Max = 0x00000005,
    };

    enum class BufferBindingType : uint32_t {
        Undefined = 0x00000000,
        Uniform = 0x00000001,
        Storage = 0x00000002,
        ReadOnlyStorage = 0x00000003,
    };

    enum class BufferMapAsyncStatus : uint32_t {
        Success = 0x00000000,
        ValidationError = 0x00000001,
        Unknown = 0x00000002,
        DeviceLost = 0x00000003,
        DestroyedBeforeCallback = 0x00000004,
        UnmappedBeforeCallback = 0x00000005,
        MappingAlreadyPending = 0x00000006,
        OffsetOutOfRange = 0x00000007,
        SizeOutOfRange = 0x00000008,
    };

    enum class BufferMapState : uint32_t {
        Unmapped = 0x00000001,
        Pending = 0x00000002,
        Mapped = 0x00000003,
    };

    enum class CallbackMode : uint32_t {
        WaitAnyOnly = 0x00000000,
        AllowProcessEvents = 0x00000001,
        AllowSpontaneous = 0x00000002,
    };

    enum class CompareFunction : uint32_t {
        Undefined = 0x00000000,
        Never = 0x00000001,
        Less = 0x00000002,
        Equal = 0x00000003,
        LessEqual = 0x00000004,
        Greater = 0x00000005,
        NotEqual = 0x00000006,
        GreaterEqual = 0x00000007,
        Always = 0x00000008,
    };

    enum class CompilationInfoRequestStatus : uint32_t {
        Success = 0x00000000,
        Error = 0x00000001,
        DeviceLost = 0x00000002,
        Unknown = 0x00000003,
    };

    enum class CompilationMessageType : uint32_t {
        Error = 0x00000001,
        Warning = 0x00000002,
        Info = 0x00000003,
    };

    enum class CompositeAlphaMode : uint32_t {
        Auto = 0x00000000,
        Opaque = 0x00000001,
        Premultiplied = 0x00000002,
        Unpremultiplied = 0x00000003,
        Inherit = 0x00000004,
    };

    enum class CreatePipelineAsyncStatus : uint32_t {
        Success = 0x00000000,
        ValidationError = 0x00000001,
        InternalError = 0x00000002,
        DeviceLost = 0x00000003,
        DeviceDestroyed = 0x00000004,
        Unknown = 0x00000005,
    };

    enum class CullMode : uint32_t {
        Undefined = 0x00000000,
        None = 0x00000001,
        Front = 0x00000002,
        Back = 0x00000003,
    };

    enum class DeviceLostReason : uint32_t {
        Undefined = 0x00000001,
        Unknown = 0x00000001,
        Destroyed = 0x00000002,
    };

    enum class ErrorFilter : uint32_t {
        Validation = 0x00000001,
        OutOfMemory = 0x00000002,
        Internal = 0x00000003,
    };

    enum class ErrorType : uint32_t {
        NoError = 0x00000000,
        Validation = 0x00000001,
        OutOfMemory = 0x00000002,
        Internal = 0x00000003,
        Unknown = 0x00000004,
        DeviceLost = 0x00000005,
    };

    enum class FeatureName : uint32_t {
        Undefined = 0x00000000,
        DepthClipControl = 0x00000001,
        Depth32FloatStencil8 = 0x00000002,
        TimestampQuery = 0x00000003,
        TextureCompressionBC = 0x00000004,
        TextureCompressionETC2 = 0x00000005,
        TextureCompressionASTC = 0x00000006,
        IndirectFirstInstance = 0x00000007,
        ShaderF16 = 0x00000008,
        RG11B10UfloatRenderable = 0x00000009,
        BGRA8UnormStorage = 0x0000000A,
        Float32Filterable = 0x0000000B,
    };

    enum class FilterMode : uint32_t {
        Undefined = 0x00000000,
        Nearest = 0x00000001,
        Linear = 0x00000002,
    };

    enum class FrontFace : uint32_t {
        Undefined = 0x00000000,
        CCW = 0x00000001,
        CW = 0x00000002,
    };

    enum class IndexFormat : uint32_t {
        Undefined = 0x00000000,
        Uint16 = 0x00000001,
        Uint32 = 0x00000002,
    };

    enum class LoadOp : uint32_t {
        Undefined = 0x00000000,
        Clear = 0x00000001,
        Load = 0x00000002,
    };

    enum class MipmapFilterMode : uint32_t {
        Undefined = 0x00000000,
        Nearest = 0x00000001,
        Linear = 0x00000002,
    };

    enum class PowerPreference : uint32_t {
        Undefined = 0x00000000,
        LowPower = 0x00000001,
        HighPerformance = 0x00000002,
    };

    enum class PresentMode : uint32_t {
        Fifo = 0x00000001,
        Immediate = 0x00000003,
        Mailbox = 0x00000004,
    };

    enum class PrimitiveTopology : uint32_t {
        Undefined = 0x00000000,
        PointList = 0x00000001,
        LineList = 0x00000002,
        LineStrip = 0x00000003,
        TriangleList = 0x00000004,
        TriangleStrip = 0x00000005,
    };

    enum class QueryType : uint32_t {
        Occlusion = 0x00000001,
        Timestamp = 0x00000002,
    };

    enum class QueueWorkDoneStatus : uint32_t {
        Success = 0x00000000,
        Error = 0x00000001,
        Unknown = 0x00000002,
        DeviceLost = 0x00000003,
    };

    enum class RequestAdapterStatus : uint32_t {
        Success = 0x00000000,
        Unavailable = 0x00000001,
        Error = 0x00000002,
        Unknown = 0x00000003,
    };

    enum class RequestDeviceStatus : uint32_t {
        Success = 0x00000000,
        Error = 0x00000001,
        Unknown = 0x00000002,
    };

    enum class SType : uint32_t {
        Invalid = 0x00000000,
        SurfaceDescriptorFromCanvasHTMLSelector = 0x00000004,
        ShaderModuleSPIRVDescriptor = 0x00000005,
        ShaderModuleWGSLDescriptor = 0x00000006,
        PrimitiveDepthClipControl = 0x00000007,
        RenderPassDescriptorMaxDrawCount = 0x0000000F,
        TextureBindingViewDimensionDescriptor = 0x00000011,
    };

    enum class SamplerBindingType : uint32_t {
        Undefined = 0x00000000,
        Filtering = 0x00000001,
        NonFiltering = 0x00000002,
        Comparison = 0x00000003,
    };

    enum class StencilOperation : uint32_t {
        Undefined = 0x00000000,
        Keep = 0x00000001,
        Zero = 0x00000002,
        Replace = 0x00000003,
        Invert = 0x00000004,
        IncrementClamp = 0x00000005,
        DecrementClamp = 0x00000006,
        IncrementWrap = 0x00000007,
        DecrementWrap = 0x00000008,
    };

    enum class StorageTextureAccess : uint32_t {
        Undefined = 0x00000000,
        WriteOnly = 0x00000001,
        ReadOnly = 0x00000002,
        ReadWrite = 0x00000003,
    };

    enum class StoreOp : uint32_t {
        Undefined = 0x00000000,
        Store = 0x00000001,
        Discard = 0x00000002,
    };

    enum class SurfaceGetCurrentTextureStatus : uint32_t {
        Success = WGPUSurfaceGetCurrentTextureStatus_Success,
        Timeout = WGPUSurfaceGetCurrentTextureStatus_Timeout,
        Outdated = WGPUSurfaceGetCurrentTextureStatus_Outdated,
        Lost = WGPUSurfaceGetCurrentTextureStatus_Lost,
        OutOfMemory = WGPUSurfaceGetCurrentTextureStatus_OutOfMemory,
        DeviceLost = WGPUSurfaceGetCurrentTextureStatus_DeviceLost,
    };

    enum class TextureAspect : uint32_t {
        Undefined = 0x00000000,
        All = 0x00000001,
        StencilOnly = 0x00000002,
        DepthOnly = 0x00000003,
    };

    enum class TextureDimension : uint32_t {
        Undefined = 0x00000000,
        e1D = 0x00000001,
        e2D = 0x00000002,
        e3D = 0x00000003,
    };

    enum class TextureFormat : uint32_t {
        Undefined = 0x00000000,
        R8Unorm = 0x00000001,
        R8Snorm = 0x00000002,
        R8Uint = 0x00000003,
        R8Sint = 0x00000004,
        R16Uint = 0x00000005,
        R16Sint = 0x00000006,
        R16Float = 0x00000007,
        RG8Unorm = 0x00000008,
        RG8Snorm = 0x00000009,
        RG8Uint = 0x0000000A,
        RG8Sint = 0x0000000B,
        R32Float = 0x0000000C,
        R32Uint = 0x0000000D,
        R32Sint = 0x0000000E,
        RG16Uint = 0x0000000F,
        RG16Sint = 0x00000010,
        RG16Float = 0x00000011,
        RGBA8Unorm = 0x00000012,
        RGBA8UnormSrgb = 0x00000013,
        RGBA8Snorm = 0x00000014,
        RGBA8Uint = 0x00000015,
        RGBA8Sint = 0x00000016,
        BGRA8Unorm = 0x00000017,
        BGRA8UnormSrgb = 0x00000018,
        RGB10A2Uint = 0x00000019,
        RGB10A2Unorm = 0x0000001A,
        RG11B10Ufloat = 0x0000001B,
        RGB9E5Ufloat = 0x0000001C,
        RG32Float = 0x0000001D,
        RG32Uint = 0x0000001E,
        RG32Sint = 0x0000001F,
        RGBA16Uint = 0x00000020,
        RGBA16Sint = 0x00000021,
        RGBA16Float = 0x00000022,
        RGBA32Float = 0x00000023,
        RGBA32Uint = 0x00000024,
        RGBA32Sint = 0x00000025,
        Stencil8 = 0x00000026,
        Depth16Unorm = 0x00000027,
        Depth24Plus = 0x00000028,
        Depth24PlusStencil8 = 0x00000029,
        Depth32Float = 0x0000002A,
        Depth32FloatStencil8 = 0x0000002B,
        BC1RGBAUnorm = 0x0000002C,
        BC1RGBAUnormSrgb = 0x0000002D,
        BC2RGBAUnorm = 0x0000002E,
        BC2RGBAUnormSrgb = 0x0000002F,
        BC3RGBAUnorm = 0x00000030,
        BC3RGBAUnormSrgb = 0x00000031,
        BC4RUnorm = 0x00000032,
        BC4RSnorm = 0x00000033,
        BC5RGUnorm = 0x00000034,
        BC5RGSnorm = 0x00000035,
        BC6HRGBUfloat = 0x00000036,
        BC6HRGBFloat = 0x00000037,
        BC7RGBAUnorm = 0x00000038,
        BC7RGBAUnormSrgb = 0x00000039,
        ETC2RGB8Unorm = 0x0000003A,
        ETC2RGB8UnormSrgb = 0x0000003B,
        ETC2RGB8A1Unorm = 0x0000003C,
        ETC2RGB8A1UnormSrgb = 0x0000003D,
        ETC2RGBA8Unorm = 0x0000003E,
        ETC2RGBA8UnormSrgb = 0x0000003F,
        EACR11Unorm = 0x00000040,
        EACR11Snorm = 0x00000041,
        EACRG11Unorm = 0x00000042,
        EACRG11Snorm = 0x00000043,
        ASTC4x4Unorm = 0x00000044,
        ASTC4x4UnormSrgb = 0x00000045,
        ASTC5x4Unorm = 0x00000046,
        ASTC5x4UnormSrgb = 0x00000047,
        ASTC5x5Unorm = 0x00000048,
        ASTC5x5UnormSrgb = 0x00000049,
        ASTC6x5Unorm = 0x0000004A,
        ASTC6x5UnormSrgb = 0x0000004B,
        ASTC6x6Unorm = 0x0000004C,
        ASTC6x6UnormSrgb = 0x0000004D,
        ASTC8x5Unorm = 0x0000004E,
        ASTC8x5UnormSrgb = 0x0000004F,
        ASTC8x6Unorm = 0x00000050,
        ASTC8x6UnormSrgb = 0x00000051,
        ASTC8x8Unorm = 0x00000052,
        ASTC8x8UnormSrgb = 0x00000053,
        ASTC10x5Unorm = 0x00000054,
        ASTC10x5UnormSrgb = 0x00000055,
        ASTC10x6Unorm = 0x00000056,
        ASTC10x6UnormSrgb = 0x00000057,
        ASTC10x8Unorm = 0x00000058,
        ASTC10x8UnormSrgb = 0x00000059,
        ASTC10x10Unorm = 0x0000005A,
        ASTC10x10UnormSrgb = 0x0000005B,
        ASTC12x10Unorm = 0x0000005C,
        ASTC12x10UnormSrgb = 0x0000005D,
        ASTC12x12Unorm = 0x0000005E,
        ASTC12x12UnormSrgb = 0x0000005F,
    };

    enum class TextureSampleType : uint32_t {
        Undefined = 0x00000000,
        Float = 0x00000001,
        UnfilterableFloat = 0x00000002,
        Depth = 0x00000003,
        Sint = 0x00000004,
        Uint = 0x00000005,
    };

    enum class TextureViewDimension : uint32_t {
        Undefined = 0x00000000,
        e1D = 0x00000001,
        e2D = 0x00000002,
        e2DArray = 0x00000003,
        Cube = 0x00000004,
        CubeArray = 0x00000005,
        e3D = 0x00000006,
    };

    enum class VertexFormat : uint32_t {
        Undefined = 0x00000000,
        Uint8x2 = 0x00000001,
        Uint8x4 = 0x00000002,
        Sint8x2 = 0x00000003,
        Sint8x4 = 0x00000004,
        Unorm8x2 = 0x00000005,
        Unorm8x4 = 0x00000006,
        Snorm8x2 = 0x00000007,
        Snorm8x4 = 0x00000008,
        Uint16x2 = 0x00000009,
        Uint16x4 = 0x0000000A,
        Sint16x2 = 0x0000000B,
        Sint16x4 = 0x0000000C,
        Unorm16x2 = 0x0000000D,
        Unorm16x4 = 0x0000000E,
        Snorm16x2 = 0x0000000F,
        Snorm16x4 = 0x00000010,
        Float16x2 = 0x00000011,
        Float16x4 = 0x00000012,
        Float32 = 0x00000013,
        Float32x2 = 0x00000014,
        Float32x3 = 0x00000015,
        Float32x4 = 0x00000016,
        Uint32 = 0x00000017,
        Uint32x2 = 0x00000018,
        Uint32x3 = 0x00000019,
        Uint32x4 = 0x0000001A,
        Sint32 = 0x0000001B,
        Sint32x2 = 0x0000001C,
        Sint32x3 = 0x0000001D,
        Sint32x4 = 0x0000001E,
        Unorm10_10_10_2 = 0x0000001F,
    };

    enum class VertexStepMode : uint32_t {
        Undefined = 0x00000000,
        VertexBufferNotUsed = 0x00000001,
        Vertex = 0x00000002,
        Instance = 0x00000003,
    };

    enum class WaitStatus : uint32_t {
        Success = 0x00000000,
        TimedOut = 0x00000001,
        UnsupportedTimeout = 0x00000002,
        UnsupportedCount = 0x00000003,
        UnsupportedMixedSources = 0x00000004,
        Unknown = 0x00000005,
    };


    enum class BufferUsage : uint32_t {
        None = 0x00000000,
        MapRead = 0x00000001,
        MapWrite = 0x00000002,
        CopySrc = 0x00000004,
        CopyDst = 0x00000008,
        Index = 0x00000010,
        Vertex = 0x00000020,
        Uniform = 0x00000040,
        Storage = 0x00000080,
        Indirect = 0x00000100,
        QueryResolve = 0x00000200,
    };

    enum class ColorWriteMask : uint32_t {
        None = 0x00000000,
        Red = 0x00000001,
        Green = 0x00000002,
        Blue = 0x00000004,
        Alpha = 0x00000008,
        All = 0x0000000F,
    };

    enum class MapMode : uint32_t {
        None = 0x00000000,
        Read = 0x00000001,
        Write = 0x00000002,
    };

    enum class ShaderStage : uint32_t {
        None = 0x00000000,
        Vertex = 0x00000001,
        Fragment = 0x00000002,
        Compute = 0x00000004,
    };

    enum class TextureUsage : uint32_t {
        None = 0x00000000,
        CopySrc = 0x00000001,
        CopyDst = 0x00000002,
        TextureBinding = 0x00000004,
        StorageBinding = 0x00000008,
        RenderAttachment = 0x00000010,
    };


    using BufferMapCallback = WGPUBufferMapCallback;
    using CompilationInfoCallback = WGPUCompilationInfoCallback;
    using CreateComputePipelineAsyncCallback = WGPUCreateComputePipelineAsyncCallback;
    using CreateRenderPipelineAsyncCallback = WGPUCreateRenderPipelineAsyncCallback;
    using DeviceLostCallback = WGPUDeviceLostCallback;
    using ErrorCallback = WGPUErrorCallback;
    using Proc = WGPUProc;
    using QueueWorkDoneCallback = WGPUQueueWorkDoneCallback;
    using RequestAdapterCallback = WGPURequestAdapterCallback;
    using RequestDeviceCallback = WGPURequestDeviceCallback;

    class Adapter;
    class BindGroup;
    class BindGroupLayout;
    class Buffer;
    class CommandBuffer;
    class CommandEncoder;
    class ComputePassEncoder;
    class ComputePipeline;
    class Device;
    class Instance;
    class PipelineLayout;
    class QuerySet;
    class Queue;
    class RenderBundle;
    class RenderBundleEncoder;
    class RenderPassEncoder;
    class RenderPipeline;
    class Sampler;
    class ShaderModule;
    class Surface;
    class SwapChain;
    class Texture;
    class TextureView;

    struct AdapterInfo;
    struct AdapterProperties;
    struct BindGroupEntry;
    struct BlendComponent;
    struct BufferBindingLayout;
    struct BufferDescriptor;
    struct BufferMapCallbackInfo;
    struct Color;
    struct CommandBufferDescriptor;
    struct CommandEncoderDescriptor;
    struct CompilationMessage;
    struct ComputePassTimestampWrites;
    struct ConstantEntry;
    struct Extent3D;
    struct Future;
    struct InstanceFeatures;
    struct Limits;
    struct MultisampleState;
    struct Origin3D;
    struct PipelineLayoutDescriptor;
    struct PrimitiveDepthClipControl;
    struct PrimitiveState;
    struct QuerySetDescriptor;
    struct QueueDescriptor;
    struct QueueWorkDoneCallbackInfo;
    struct RenderBundleDescriptor;
    struct RenderBundleEncoderDescriptor;
    struct RenderPassDepthStencilAttachment;
    struct RenderPassDescriptorMaxDrawCount;
    struct RenderPassTimestampWrites;
    struct RequestAdapterCallbackInfo;
    struct RequestAdapterOptions;
    struct SamplerBindingLayout;
    struct SamplerDescriptor;
    struct ShaderModuleSPIRVDescriptor;
    struct ShaderModuleWGSLDescriptor;
    struct ShaderModuleDescriptor;
    struct StencilFaceState;
    struct StorageTextureBindingLayout;
    struct SurfaceCapabilities;
    struct SurfaceConfiguration;
    struct SurfaceDescriptor;
    struct SurfaceDescriptorFromCanvasHTMLSelector;
    struct SurfaceTexture;
    struct SwapChainDescriptor;
    struct TextureBindingLayout;
    struct TextureBindingViewDimensionDescriptor;
    struct TextureDataLayout;
    struct TextureViewDescriptor;
    struct VertexAttribute;
    struct BindGroupDescriptor;
    struct BindGroupLayoutEntry;
    struct BlendState;
    struct CompilationInfo;
    struct ComputePassDescriptor;
    struct DepthStencilState;
    struct FutureWaitInfo;
    struct ImageCopyBuffer;
    struct ImageCopyTexture;
    struct InstanceDescriptor;
    struct ProgrammableStageDescriptor;
    struct RenderPassColorAttachment;
    struct RequiredLimits;
    struct SupportedLimits;
    struct TextureDescriptor;
    struct VertexBufferLayout;
    struct BindGroupLayoutDescriptor;
    struct ColorTargetState;
    struct ComputePipelineDescriptor;
    struct DeviceDescriptor;
    struct RenderPassDescriptor;
    struct VertexState;
    struct FragmentState;
    struct RenderPipelineDescriptor;


    // Special class for booleans in order to allow implicit conversions.
    class Bool {
      public:
        constexpr Bool() = default;
        // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
        constexpr Bool(bool value) : mValue(static_cast<WGPUBool>(value)) {}
        // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
        Bool(WGPUBool value): mValue(value) {}

        constexpr operator bool() const { return static_cast<bool>(mValue); }

      private:
        friend struct std::hash<Bool>;
        // Default to false.
        WGPUBool mValue = static_cast<WGPUBool>(false);
    };

    template<typename Derived, typename CType>
    class ObjectBase {
      public:
        ObjectBase() = default;
        ObjectBase(CType handle): mHandle(handle) {
            if (mHandle) Derived::WGPUReference(mHandle);
        }
        ~ObjectBase() {
            if (mHandle) Derived::WGPURelease(mHandle);
        }

        ObjectBase(ObjectBase const& other)
            : ObjectBase(other.Get()) {
        }
        Derived& operator=(ObjectBase const& other) {
            if (&other != this) {
                if (mHandle) Derived::WGPURelease(mHandle);
                mHandle = other.mHandle;
                if (mHandle) Derived::WGPUReference(mHandle);
            }

            return static_cast<Derived&>(*this);
        }

        ObjectBase(ObjectBase&& other) {
            mHandle = other.mHandle;
            other.mHandle = 0;
        }
        Derived& operator=(ObjectBase&& other) {
            if (&other != this) {
                if (mHandle) Derived::WGPURelease(mHandle);
                mHandle = other.mHandle;
                other.mHandle = 0;
            }

            return static_cast<Derived&>(*this);
        }

        ObjectBase(std::nullptr_t) {}
        Derived& operator=(std::nullptr_t) {
            if (mHandle != nullptr) {
                Derived::WGPURelease(mHandle);
                mHandle = nullptr;
            }
            return static_cast<Derived&>(*this);
        }

        bool operator==(std::nullptr_t) const {
            return mHandle == nullptr;
        }
        bool operator!=(std::nullptr_t) const {
            return mHandle != nullptr;
        }

        explicit operator bool() const {
            return mHandle != nullptr;
        }
        CType Get() const {
            return mHandle;
        }
        CType MoveToCHandle() {
            CType result = mHandle;
            mHandle = 0;
            return result;
        }
        static Derived Acquire(CType handle) {
            Derived result;
            result.mHandle = handle;
            return result;
        }

      protected:
        CType mHandle = nullptr;
    };



    class Adapter : public ObjectBase<Adapter, WGPUAdapter> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        size_t EnumerateFeatures(FeatureName * features) const;
        void GetInfo(AdapterInfo * info) const;
        Bool GetLimits(SupportedLimits * limits) const;
        void GetProperties(AdapterProperties * properties) const;
        Bool HasFeature(FeatureName feature) const;
        void RequestDevice(DeviceDescriptor const * descriptor, RequestDeviceCallback callback, void * userdata) const;

      private:
        friend ObjectBase<Adapter, WGPUAdapter>;
        static void WGPUReference(WGPUAdapter handle);
        static void WGPURelease(WGPUAdapter handle);
    };

    class BindGroup : public ObjectBase<BindGroup, WGPUBindGroup> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<BindGroup, WGPUBindGroup>;
        static void WGPUReference(WGPUBindGroup handle);
        static void WGPURelease(WGPUBindGroup handle);
    };

    class BindGroupLayout : public ObjectBase<BindGroupLayout, WGPUBindGroupLayout> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<BindGroupLayout, WGPUBindGroupLayout>;
        static void WGPUReference(WGPUBindGroupLayout handle);
        static void WGPURelease(WGPUBindGroupLayout handle);
    };

    class Buffer : public ObjectBase<Buffer, WGPUBuffer> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void Destroy() const;
        void const * GetConstMappedRange(size_t offset = 0, size_t size = WGPU_WHOLE_MAP_SIZE) const;
        BufferMapState GetMapState() const;
        void * GetMappedRange(size_t offset = 0, size_t size = WGPU_WHOLE_MAP_SIZE) const;
        uint64_t GetSize() const;
        BufferUsage GetUsage() const;
        void MapAsync(MapMode mode, size_t offset, size_t size, BufferMapCallback callback, void * userdata) const;
        void SetLabel(char const * label) const;
        void Unmap() const;

      private:
        friend ObjectBase<Buffer, WGPUBuffer>;
        static void WGPUReference(WGPUBuffer handle);
        static void WGPURelease(WGPUBuffer handle);
    };

    class CommandBuffer : public ObjectBase<CommandBuffer, WGPUCommandBuffer> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<CommandBuffer, WGPUCommandBuffer>;
        static void WGPUReference(WGPUCommandBuffer handle);
        static void WGPURelease(WGPUCommandBuffer handle);
    };

    class CommandEncoder : public ObjectBase<CommandEncoder, WGPUCommandEncoder> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        ComputePassEncoder BeginComputePass(ComputePassDescriptor const * descriptor = nullptr) const;
        RenderPassEncoder BeginRenderPass(RenderPassDescriptor const * descriptor) const;
        void ClearBuffer(Buffer const& buffer, uint64_t offset = 0, uint64_t size = WGPU_WHOLE_SIZE) const;
        void CopyBufferToBuffer(Buffer const& source, uint64_t sourceOffset, Buffer const& destination, uint64_t destinationOffset, uint64_t size) const;
        void CopyBufferToTexture(ImageCopyBuffer const * source, ImageCopyTexture const * destination, Extent3D const * copySize) const;
        void CopyTextureToBuffer(ImageCopyTexture const * source, ImageCopyBuffer const * destination, Extent3D const * copySize) const;
        void CopyTextureToTexture(ImageCopyTexture const * source, ImageCopyTexture const * destination, Extent3D const * copySize) const;
        CommandBuffer Finish(CommandBufferDescriptor const * descriptor = nullptr) const;
        void InsertDebugMarker(char const * markerLabel) const;
        void PopDebugGroup() const;
        void PushDebugGroup(char const * groupLabel) const;
        void ResolveQuerySet(QuerySet const& querySet, uint32_t firstQuery, uint32_t queryCount, Buffer const& destination, uint64_t destinationOffset) const;
        void SetLabel(char const * label) const;
        void WriteTimestamp(QuerySet const& querySet, uint32_t queryIndex) const;

      private:
        friend ObjectBase<CommandEncoder, WGPUCommandEncoder>;
        static void WGPUReference(WGPUCommandEncoder handle);
        static void WGPURelease(WGPUCommandEncoder handle);
    };

    class ComputePassEncoder : public ObjectBase<ComputePassEncoder, WGPUComputePassEncoder> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void DispatchWorkgroups(uint32_t workgroupCountX, uint32_t workgroupCountY = 1, uint32_t workgroupCountZ = 1) const;
        void DispatchWorkgroupsIndirect(Buffer const& indirectBuffer, uint64_t indirectOffset) const;
        void End() const;
        void InsertDebugMarker(char const * markerLabel) const;
        void PopDebugGroup() const;
        void PushDebugGroup(char const * groupLabel) const;
        void SetBindGroup(uint32_t groupIndex, BindGroup const& group, size_t dynamicOffsetCount = 0, uint32_t const * dynamicOffsets = nullptr) const;
        void SetLabel(char const * label) const;
        void SetPipeline(ComputePipeline const& pipeline) const;
        void WriteTimestamp(QuerySet const& querySet, uint32_t queryIndex) const;

      private:
        friend ObjectBase<ComputePassEncoder, WGPUComputePassEncoder>;
        static void WGPUReference(WGPUComputePassEncoder handle);
        static void WGPURelease(WGPUComputePassEncoder handle);
    };

    class ComputePipeline : public ObjectBase<ComputePipeline, WGPUComputePipeline> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        BindGroupLayout GetBindGroupLayout(uint32_t groupIndex) const;
        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<ComputePipeline, WGPUComputePipeline>;
        static void WGPUReference(WGPUComputePipeline handle);
        static void WGPURelease(WGPUComputePipeline handle);
    };

    class Device : public ObjectBase<Device, WGPUDevice> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        BindGroup CreateBindGroup(BindGroupDescriptor const * descriptor) const;
        BindGroupLayout CreateBindGroupLayout(BindGroupLayoutDescriptor const * descriptor) const;
        Buffer CreateBuffer(BufferDescriptor const * descriptor) const;
        CommandEncoder CreateCommandEncoder(CommandEncoderDescriptor const * descriptor = nullptr) const;
        ComputePipeline CreateComputePipeline(ComputePipelineDescriptor const * descriptor) const;
        void CreateComputePipelineAsync(ComputePipelineDescriptor const * descriptor, CreateComputePipelineAsyncCallback callback, void * userdata) const;
        PipelineLayout CreatePipelineLayout(PipelineLayoutDescriptor const * descriptor) const;
        QuerySet CreateQuerySet(QuerySetDescriptor const * descriptor) const;
        RenderBundleEncoder CreateRenderBundleEncoder(RenderBundleEncoderDescriptor const * descriptor) const;
        RenderPipeline CreateRenderPipeline(RenderPipelineDescriptor const * descriptor) const;
        void CreateRenderPipelineAsync(RenderPipelineDescriptor const * descriptor, CreateRenderPipelineAsyncCallback callback, void * userdata) const;
        Sampler CreateSampler(SamplerDescriptor const * descriptor = nullptr) const;
        ShaderModule CreateShaderModule(ShaderModuleDescriptor const * descriptor) const;
        SwapChain CreateSwapChain(Surface const& surface, SwapChainDescriptor const * descriptor) const;
        Texture CreateTexture(TextureDescriptor const * descriptor) const;
        void Destroy() const;
        size_t EnumerateFeatures(FeatureName * features) const;
        Bool GetLimits(SupportedLimits * limits) const;
        Queue GetQueue() const;
        Bool HasFeature(FeatureName feature) const;
        void PopErrorScope(ErrorCallback callback, void * userdata) const;
        void PushErrorScope(ErrorFilter filter) const;
        void SetLabel(char const * label) const;
        void SetUncapturedErrorCallback(ErrorCallback callback, void * userdata) const;

      private:
        friend ObjectBase<Device, WGPUDevice>;
        static void WGPUReference(WGPUDevice handle);
        static void WGPURelease(WGPUDevice handle);
    };

    class Instance : public ObjectBase<Instance, WGPUInstance> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        Surface CreateSurface(SurfaceDescriptor const * descriptor) const;
        Bool HasWGSLLanguageFeature(WGSLFeatureName feature) const;
        void ProcessEvents() const;
        void RequestAdapter(RequestAdapterOptions const * options, RequestAdapterCallback callback, void * userdata) const;

      private:
        friend ObjectBase<Instance, WGPUInstance>;
        static void WGPUReference(WGPUInstance handle);
        static void WGPURelease(WGPUInstance handle);
    };

    class PipelineLayout : public ObjectBase<PipelineLayout, WGPUPipelineLayout> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<PipelineLayout, WGPUPipelineLayout>;
        static void WGPUReference(WGPUPipelineLayout handle);
        static void WGPURelease(WGPUPipelineLayout handle);
    };

    class QuerySet : public ObjectBase<QuerySet, WGPUQuerySet> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void Destroy() const;
        uint32_t GetCount() const;
        QueryType GetType() const;
        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<QuerySet, WGPUQuerySet>;
        static void WGPUReference(WGPUQuerySet handle);
        static void WGPURelease(WGPUQuerySet handle);
    };

    class Queue : public ObjectBase<Queue, WGPUQueue> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void OnSubmittedWorkDone(QueueWorkDoneCallback callback, void * userdata) const;
        void SetLabel(char const * label) const;
        void Submit(size_t commandCount, CommandBuffer const * commands) const;
        void WriteBuffer(Buffer const& buffer, uint64_t bufferOffset, void const * data, size_t size) const;
        void WriteTexture(ImageCopyTexture const * destination, void const * data, size_t dataSize, TextureDataLayout const * dataLayout, Extent3D const * writeSize) const;

      private:
        friend ObjectBase<Queue, WGPUQueue>;
        static void WGPUReference(WGPUQueue handle);
        static void WGPURelease(WGPUQueue handle);
    };

    class RenderBundle : public ObjectBase<RenderBundle, WGPURenderBundle> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<RenderBundle, WGPURenderBundle>;
        static void WGPUReference(WGPURenderBundle handle);
        static void WGPURelease(WGPURenderBundle handle);
    };

    class RenderBundleEncoder : public ObjectBase<RenderBundleEncoder, WGPURenderBundleEncoder> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t baseVertex = 0, uint32_t firstInstance = 0) const;
        void DrawIndexedIndirect(Buffer const& indirectBuffer, uint64_t indirectOffset) const;
        void DrawIndirect(Buffer const& indirectBuffer, uint64_t indirectOffset) const;
        RenderBundle Finish(RenderBundleDescriptor const * descriptor = nullptr) const;
        void InsertDebugMarker(char const * markerLabel) const;
        void PopDebugGroup() const;
        void PushDebugGroup(char const * groupLabel) const;
        void SetBindGroup(uint32_t groupIndex, BindGroup const& group, size_t dynamicOffsetCount = 0, uint32_t const * dynamicOffsets = nullptr) const;
        void SetIndexBuffer(Buffer const& buffer, IndexFormat format, uint64_t offset = 0, uint64_t size = WGPU_WHOLE_SIZE) const;
        void SetLabel(char const * label) const;
        void SetPipeline(RenderPipeline const& pipeline) const;
        void SetVertexBuffer(uint32_t slot, Buffer const& buffer, uint64_t offset = 0, uint64_t size = WGPU_WHOLE_SIZE) const;

      private:
        friend ObjectBase<RenderBundleEncoder, WGPURenderBundleEncoder>;
        static void WGPUReference(WGPURenderBundleEncoder handle);
        static void WGPURelease(WGPURenderBundleEncoder handle);
    };

    class RenderPassEncoder : public ObjectBase<RenderPassEncoder, WGPURenderPassEncoder> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void BeginOcclusionQuery(uint32_t queryIndex) const;
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t baseVertex = 0, uint32_t firstInstance = 0) const;
        void DrawIndexedIndirect(Buffer const& indirectBuffer, uint64_t indirectOffset) const;
        void DrawIndirect(Buffer const& indirectBuffer, uint64_t indirectOffset) const;
        void End() const;
        void EndOcclusionQuery() const;
        void ExecuteBundles(size_t bundleCount, RenderBundle const * bundles) const;
        void InsertDebugMarker(char const * markerLabel) const;
        void PopDebugGroup() const;
        void PushDebugGroup(char const * groupLabel) const;
        void SetBindGroup(uint32_t groupIndex, BindGroup const& group, size_t dynamicOffsetCount = 0, uint32_t const * dynamicOffsets = nullptr) const;
        void SetBlendConstant(Color const * color) const;
        void SetIndexBuffer(Buffer const& buffer, IndexFormat format, uint64_t offset = 0, uint64_t size = WGPU_WHOLE_SIZE) const;
        void SetLabel(char const * label) const;
        void SetPipeline(RenderPipeline const& pipeline) const;
        void SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;
        void SetStencilReference(uint32_t reference) const;
        void SetVertexBuffer(uint32_t slot, Buffer const& buffer, uint64_t offset = 0, uint64_t size = WGPU_WHOLE_SIZE) const;
        void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth) const;
        void WriteTimestamp(QuerySet const& querySet, uint32_t queryIndex) const;

      private:
        friend ObjectBase<RenderPassEncoder, WGPURenderPassEncoder>;
        static void WGPUReference(WGPURenderPassEncoder handle);
        static void WGPURelease(WGPURenderPassEncoder handle);
    };

    class RenderPipeline : public ObjectBase<RenderPipeline, WGPURenderPipeline> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        BindGroupLayout GetBindGroupLayout(uint32_t groupIndex) const;
        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<RenderPipeline, WGPURenderPipeline>;
        static void WGPUReference(WGPURenderPipeline handle);
        static void WGPURelease(WGPURenderPipeline handle);
    };

    class Sampler : public ObjectBase<Sampler, WGPUSampler> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<Sampler, WGPUSampler>;
        static void WGPUReference(WGPUSampler handle);
        static void WGPURelease(WGPUSampler handle);
    };

    class ShaderModule : public ObjectBase<ShaderModule, WGPUShaderModule> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void GetCompilationInfo(CompilationInfoCallback callback, void * userdata) const;
        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<ShaderModule, WGPUShaderModule>;
        static void WGPUReference(WGPUShaderModule handle);
        static void WGPURelease(WGPUShaderModule handle);
    };

    class Surface : public ObjectBase<Surface, WGPUSurface> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void Configure(SurfaceConfiguration const * config) const;
        void GetCapabilities(Adapter const& adapter, SurfaceCapabilities * capabilities) const;
        void GetCurrentTexture(SurfaceTexture * surfaceTexture) const;
        TextureFormat GetPreferredFormat(Adapter const& adapter) const;
        void Present() const;
        void Unconfigure() const;

      private:
        friend ObjectBase<Surface, WGPUSurface>;
        static void WGPUReference(WGPUSurface handle);
        static void WGPURelease(WGPUSurface handle);
    };

    class SwapChain : public ObjectBase<SwapChain, WGPUSwapChain> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        Texture GetCurrentTexture() const;
        TextureView GetCurrentTextureView() const;
        void Present() const;

      private:
        friend ObjectBase<SwapChain, WGPUSwapChain>;
        static void WGPUReference(WGPUSwapChain handle);
        static void WGPURelease(WGPUSwapChain handle);
    };

    class Texture : public ObjectBase<Texture, WGPUTexture> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        TextureView CreateView(TextureViewDescriptor const * descriptor = nullptr) const;
        void Destroy() const;
        uint32_t GetDepthOrArrayLayers() const;
        TextureDimension GetDimension() const;
        TextureFormat GetFormat() const;
        uint32_t GetHeight() const;
        uint32_t GetMipLevelCount() const;
        uint32_t GetSampleCount() const;
        TextureUsage GetUsage() const;
        uint32_t GetWidth() const;
        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<Texture, WGPUTexture>;
        static void WGPUReference(WGPUTexture handle);
        static void WGPURelease(WGPUTexture handle);
    };

    class TextureView : public ObjectBase<TextureView, WGPUTextureView> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        void SetLabel(char const * label) const;

      private:
        friend ObjectBase<TextureView, WGPUTextureView>;
        static void WGPUReference(WGPUTextureView handle);
        static void WGPURelease(WGPUTextureView handle);
    };


    Instance CreateInstance(InstanceDescriptor const * descriptor = nullptr);
    Bool GetInstanceFeatures(InstanceFeatures * features);
    Proc GetProcAddress(Device device, char const * procName);

    struct AdapterInfo {
        AdapterInfo() = default;
        ~AdapterInfo();
        AdapterInfo(const AdapterInfo&) = delete;
        AdapterInfo& operator=(const AdapterInfo&) = delete;
        AdapterInfo(AdapterInfo&&);
        AdapterInfo& operator=(AdapterInfo&&);
        ChainedStructOut  * nextInChain = nullptr;
        char const * const vendor = nullptr;
        char const * const architecture = nullptr;
        char const * const device = nullptr;
        char const * const description = nullptr;
        BackendType const backendType = {};
        AdapterType const adapterType = {};
        uint32_t const vendorID = {};
        uint32_t const deviceID = {};
    };

    struct AdapterProperties {
        AdapterProperties() = default;
        ~AdapterProperties();
        AdapterProperties(const AdapterProperties&) = delete;
        AdapterProperties& operator=(const AdapterProperties&) = delete;
        AdapterProperties(AdapterProperties&&);
        AdapterProperties& operator=(AdapterProperties&&);
        ChainedStructOut  * nextInChain = nullptr;
        uint32_t const vendorID = {};
        char const * const vendorName = nullptr;
        char const * const architecture = nullptr;
        uint32_t const deviceID = {};
        char const * const name = nullptr;
        char const * const driverDescription = nullptr;
        AdapterType const adapterType = {};
        BackendType const backendType = {};
        Bool const compatibilityMode = false;
    };

    struct BindGroupEntry {
        ChainedStruct const * nextInChain = nullptr;
        uint32_t binding;
        Buffer buffer = nullptr;
        uint64_t offset = 0;
        uint64_t size = WGPU_WHOLE_SIZE;
        Sampler sampler = nullptr;
        TextureView textureView = nullptr;
    };

    struct BlendComponent {
        BlendOperation operation = BlendOperation::Add;
        BlendFactor srcFactor = BlendFactor::One;
        BlendFactor dstFactor = BlendFactor::Zero;
    };

    struct BufferBindingLayout {
        ChainedStruct const * nextInChain = nullptr;
        BufferBindingType type = BufferBindingType::Undefined;
        Bool hasDynamicOffset = false;
        uint64_t minBindingSize = 0;
    };

    struct BufferDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        BufferUsage usage;
        uint64_t size;
        Bool mappedAtCreation = false;
    };

    struct BufferMapCallbackInfo {
        ChainedStruct const * nextInChain = nullptr;
        CallbackMode mode;
        BufferMapCallback callback;
        void * userdata;
    };

    struct Color {
        double r;
        double g;
        double b;
        double a;
    };

    struct CommandBufferDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
    };

    struct CommandEncoderDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
    };

    struct CompilationMessage {
        ChainedStruct const * nextInChain = nullptr;
        char const * message = nullptr;
        CompilationMessageType type;
        uint64_t lineNum;
        uint64_t linePos;
        uint64_t offset;
        uint64_t length;
        uint64_t utf16LinePos;
        uint64_t utf16Offset;
        uint64_t utf16Length;
    };

    struct ComputePassTimestampWrites {
        QuerySet querySet;
        uint32_t beginningOfPassWriteIndex = WGPU_QUERY_SET_INDEX_UNDEFINED;
        uint32_t endOfPassWriteIndex = WGPU_QUERY_SET_INDEX_UNDEFINED;
    };

    struct ConstantEntry {
        ChainedStruct const * nextInChain = nullptr;
        char const * key;
        double value;
    };

    struct Extent3D {
        uint32_t width;
        uint32_t height = 1;
        uint32_t depthOrArrayLayers = 1;
    };

    struct Future {
        uint64_t id;
    };

    struct InstanceFeatures {
        ChainedStruct const * nextInChain = nullptr;
        Bool timedWaitAnyEnable = false;
        size_t timedWaitAnyMaxCount = 0;
    };

    struct Limits {
        uint32_t maxTextureDimension1D = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxTextureDimension2D = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxTextureDimension3D = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxTextureArrayLayers = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxBindGroups = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxBindGroupsPlusVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxBindingsPerBindGroup = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxDynamicUniformBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxDynamicStorageBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxSampledTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxSamplersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxStorageBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxStorageTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxUniformBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
        uint64_t maxUniformBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
        uint64_t maxStorageBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
        uint32_t minUniformBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t minStorageBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
        uint64_t maxBufferSize = WGPU_LIMIT_U64_UNDEFINED;
        uint32_t maxVertexAttributes = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxVertexBufferArrayStride = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxInterStageShaderComponents = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxInterStageShaderVariables = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxColorAttachments = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxColorAttachmentBytesPerSample = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxComputeWorkgroupStorageSize = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxComputeInvocationsPerWorkgroup = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxComputeWorkgroupSizeX = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxComputeWorkgroupSizeY = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxComputeWorkgroupSizeZ = WGPU_LIMIT_U32_UNDEFINED;
        uint32_t maxComputeWorkgroupsPerDimension = WGPU_LIMIT_U32_UNDEFINED;
    };

    struct MultisampleState {
        ChainedStruct const * nextInChain = nullptr;
        uint32_t count = 1;
        uint32_t mask = 0xFFFFFFFF;
        Bool alphaToCoverageEnabled = false;
    };

    struct Origin3D {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
    };

    struct PipelineLayoutDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        size_t bindGroupLayoutCount;
        BindGroupLayout const * bindGroupLayouts;
    };

    // Can be chained in PrimitiveState
    struct PrimitiveDepthClipControl : ChainedStruct {
        PrimitiveDepthClipControl() {
            sType = SType::PrimitiveDepthClipControl;
        }
        static constexpr size_t kFirstMemberAlignment = detail::ConstexprMax(alignof(ChainedStruct), alignof(Bool ));
        alignas(kFirstMemberAlignment) Bool unclippedDepth = false;
    };

    struct PrimitiveState {
        ChainedStruct const * nextInChain = nullptr;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        IndexFormat stripIndexFormat = IndexFormat::Undefined;
        FrontFace frontFace = FrontFace::CCW;
        CullMode cullMode = CullMode::None;
    };

    struct QuerySetDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        QueryType type;
        uint32_t count;
    };

    struct QueueDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
    };

    struct QueueWorkDoneCallbackInfo {
        ChainedStruct const * nextInChain = nullptr;
        CallbackMode mode;
        QueueWorkDoneCallback callback;
        void * userdata;
    };

    struct RenderBundleDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
    };

    struct RenderBundleEncoderDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        size_t colorFormatCount;
        TextureFormat const * colorFormats;
        TextureFormat depthStencilFormat = TextureFormat::Undefined;
        uint32_t sampleCount = 1;
        Bool depthReadOnly = false;
        Bool stencilReadOnly = false;
    };

    struct RenderPassDepthStencilAttachment {
        TextureView view;
        LoadOp depthLoadOp = LoadOp::Undefined;
        StoreOp depthStoreOp = StoreOp::Undefined;
        float depthClearValue = NAN;
        Bool depthReadOnly = false;
        LoadOp stencilLoadOp = LoadOp::Undefined;
        StoreOp stencilStoreOp = StoreOp::Undefined;
        uint32_t stencilClearValue = 0;
        Bool stencilReadOnly = false;
    };

    // Can be chained in RenderPassDescriptor
    struct RenderPassDescriptorMaxDrawCount : ChainedStruct {
        RenderPassDescriptorMaxDrawCount() {
            sType = SType::RenderPassDescriptorMaxDrawCount;
        }
        static constexpr size_t kFirstMemberAlignment = detail::ConstexprMax(alignof(ChainedStruct), alignof(uint64_t ));
        alignas(kFirstMemberAlignment) uint64_t maxDrawCount = 50000000;
    };

    struct RenderPassTimestampWrites {
        QuerySet querySet;
        uint32_t beginningOfPassWriteIndex = WGPU_QUERY_SET_INDEX_UNDEFINED;
        uint32_t endOfPassWriteIndex = WGPU_QUERY_SET_INDEX_UNDEFINED;
    };

    struct RequestAdapterCallbackInfo {
        ChainedStruct const * nextInChain = nullptr;
        CallbackMode mode;
        RequestAdapterCallback callback;
        void * userdata;
    };

    struct RequestAdapterOptions {
        ChainedStruct const * nextInChain = nullptr;
        Surface compatibleSurface = nullptr;
        PowerPreference powerPreference = PowerPreference::Undefined;
        BackendType backendType = BackendType::Undefined;
        Bool forceFallbackAdapter = false;
        Bool compatibilityMode = false;
    };

    struct SamplerBindingLayout {
        ChainedStruct const * nextInChain = nullptr;
        SamplerBindingType type = SamplerBindingType::Undefined;
    };

    struct SamplerDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        AddressMode addressModeU = AddressMode::ClampToEdge;
        AddressMode addressModeV = AddressMode::ClampToEdge;
        AddressMode addressModeW = AddressMode::ClampToEdge;
        FilterMode magFilter = FilterMode::Nearest;
        FilterMode minFilter = FilterMode::Nearest;
        MipmapFilterMode mipmapFilter = MipmapFilterMode::Nearest;
        float lodMinClamp = 0.0f;
        float lodMaxClamp = 32.0f;
        CompareFunction compare = CompareFunction::Undefined;
        uint16_t maxAnisotropy = 1;
    };

    // Can be chained in ShaderModuleDescriptor
    struct ShaderModuleSPIRVDescriptor : ChainedStruct {
        ShaderModuleSPIRVDescriptor() {
            sType = SType::ShaderModuleSPIRVDescriptor;
        }
        static constexpr size_t kFirstMemberAlignment = detail::ConstexprMax(alignof(ChainedStruct), alignof(uint32_t ));
        alignas(kFirstMemberAlignment) uint32_t codeSize;
        uint32_t const * code;
    };

    // Can be chained in ShaderModuleDescriptor
    struct ShaderModuleWGSLDescriptor : ChainedStruct {
        ShaderModuleWGSLDescriptor() {
            sType = SType::ShaderModuleWGSLDescriptor;
        }
        static constexpr size_t kFirstMemberAlignment = detail::ConstexprMax(alignof(ChainedStruct), alignof(char const * ));
        alignas(kFirstMemberAlignment) char const * code;
    };

    struct ShaderModuleDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
    };

    struct StencilFaceState {
        CompareFunction compare = CompareFunction::Always;
        StencilOperation failOp = StencilOperation::Keep;
        StencilOperation depthFailOp = StencilOperation::Keep;
        StencilOperation passOp = StencilOperation::Keep;
    };

    struct StorageTextureBindingLayout {
        ChainedStruct const * nextInChain = nullptr;
        StorageTextureAccess access = StorageTextureAccess::Undefined;
        TextureFormat format = TextureFormat::Undefined;
        TextureViewDimension viewDimension = TextureViewDimension::e2D;
    };

    struct SurfaceCapabilities {
        SurfaceCapabilities() = default;
        ~SurfaceCapabilities();
        SurfaceCapabilities(const SurfaceCapabilities&) = delete;
        SurfaceCapabilities& operator=(const SurfaceCapabilities&) = delete;
        SurfaceCapabilities(SurfaceCapabilities&&);
        SurfaceCapabilities& operator=(SurfaceCapabilities&&);
        ChainedStructOut  * nextInChain = nullptr;
        size_t const formatCount = {};
        TextureFormat const * const formats = {};
        size_t const presentModeCount = {};
        PresentMode const * const presentModes = {};
        size_t const alphaModeCount = {};
        CompositeAlphaMode const * const alphaModes = {};
    };

    struct SurfaceConfiguration {
        ChainedStruct const * nextInChain = nullptr;
        Device device;
        TextureFormat format;
        TextureUsage usage = TextureUsage::RenderAttachment;
        size_t viewFormatCount = 0;
        TextureFormat const * viewFormats;
        CompositeAlphaMode alphaMode = CompositeAlphaMode::Auto;
        uint32_t width;
        uint32_t height;
        PresentMode presentMode = PresentMode::Fifo;
    };

    struct SurfaceDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
    };

    // Can be chained in SurfaceDescriptor
    struct SurfaceDescriptorFromCanvasHTMLSelector : ChainedStruct {
        SurfaceDescriptorFromCanvasHTMLSelector() {
            sType = SType::SurfaceDescriptorFromCanvasHTMLSelector;
        }
        static constexpr size_t kFirstMemberAlignment = detail::ConstexprMax(alignof(ChainedStruct), alignof(char const * ));
        alignas(kFirstMemberAlignment) char const * selector;
    };

    struct SurfaceTexture {
        Texture texture;
        Bool suboptimal;
        SurfaceGetCurrentTextureStatus status;
    };

    struct SwapChainDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        TextureUsage usage;
        TextureFormat format;
        uint32_t width;
        uint32_t height;
        PresentMode presentMode;
    };

    struct TextureBindingLayout {
        ChainedStruct const * nextInChain = nullptr;
        TextureSampleType sampleType = TextureSampleType::Undefined;
        TextureViewDimension viewDimension = TextureViewDimension::e2D;
        Bool multisampled = false;
    };

    // Can be chained in TextureDescriptor
    struct TextureBindingViewDimensionDescriptor : ChainedStruct {
        TextureBindingViewDimensionDescriptor() {
            sType = SType::TextureBindingViewDimensionDescriptor;
        }
        static constexpr size_t kFirstMemberAlignment = detail::ConstexprMax(alignof(ChainedStruct), alignof(TextureViewDimension ));
        alignas(kFirstMemberAlignment) TextureViewDimension textureBindingViewDimension = TextureViewDimension::Undefined;
    };

    struct TextureDataLayout {
        ChainedStruct const * nextInChain = nullptr;
        uint64_t offset = 0;
        uint32_t bytesPerRow = WGPU_COPY_STRIDE_UNDEFINED;
        uint32_t rowsPerImage = WGPU_COPY_STRIDE_UNDEFINED;
    };

    struct TextureViewDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        TextureFormat format = TextureFormat::Undefined;
        TextureViewDimension dimension = TextureViewDimension::Undefined;
        uint32_t baseMipLevel = 0;
        uint32_t mipLevelCount = WGPU_MIP_LEVEL_COUNT_UNDEFINED;
        uint32_t baseArrayLayer = 0;
        uint32_t arrayLayerCount = WGPU_ARRAY_LAYER_COUNT_UNDEFINED;
        TextureAspect aspect = TextureAspect::All;
    };

    struct VertexAttribute {
        VertexFormat format;
        uint64_t offset;
        uint32_t shaderLocation;
    };

    struct BindGroupDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        BindGroupLayout layout;
        size_t entryCount;
        BindGroupEntry const * entries;
    };

    struct BindGroupLayoutEntry {
        ChainedStruct const * nextInChain = nullptr;
        uint32_t binding;
        ShaderStage visibility;
        BufferBindingLayout buffer;
        SamplerBindingLayout sampler;
        TextureBindingLayout texture;
        StorageTextureBindingLayout storageTexture;
    };

    struct BlendState {
        BlendComponent color;
        BlendComponent alpha;
    };

    struct CompilationInfo {
        ChainedStruct const * nextInChain = nullptr;
        size_t messageCount;
        CompilationMessage const * messages;
    };

    struct ComputePassDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        ComputePassTimestampWrites const * timestampWrites = nullptr;
    };

    struct DepthStencilState {
        ChainedStruct const * nextInChain = nullptr;
        TextureFormat format;
        Bool depthWriteEnabled = false;
        CompareFunction depthCompare = CompareFunction::Undefined;
        StencilFaceState stencilFront;
        StencilFaceState stencilBack;
        uint32_t stencilReadMask = 0xFFFFFFFF;
        uint32_t stencilWriteMask = 0xFFFFFFFF;
        int32_t depthBias = 0;
        float depthBiasSlopeScale = 0.0f;
        float depthBiasClamp = 0.0f;
    };

    struct FutureWaitInfo {
        Future future;
        Bool completed = false;
    };

    struct ImageCopyBuffer {
        ChainedStruct const * nextInChain = nullptr;
        TextureDataLayout layout;
        Buffer buffer;
    };

    struct ImageCopyTexture {
        ChainedStruct const * nextInChain = nullptr;
        Texture texture;
        uint32_t mipLevel = 0;
        Origin3D origin;
        TextureAspect aspect = TextureAspect::All;
    };

    struct InstanceDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        InstanceFeatures features;
    };

    struct ProgrammableStageDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        ShaderModule module;
        char const * entryPoint = nullptr;
        size_t constantCount = 0;
        ConstantEntry const * constants;
    };

    struct RenderPassColorAttachment {
        ChainedStruct const * nextInChain = nullptr;
        TextureView view = nullptr;
        uint32_t depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        TextureView resolveTarget = nullptr;
        LoadOp loadOp;
        StoreOp storeOp;
        Color clearValue;
    };

    struct RequiredLimits {
        ChainedStruct const * nextInChain = nullptr;
        Limits limits;
    };

    struct SupportedLimits {
        ChainedStructOut  * nextInChain = nullptr;
        Limits limits;
    };

    struct TextureDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        TextureUsage usage;
        TextureDimension dimension = TextureDimension::e2D;
        Extent3D size;
        TextureFormat format;
        uint32_t mipLevelCount = 1;
        uint32_t sampleCount = 1;
        size_t viewFormatCount = 0;
        TextureFormat const * viewFormats;
    };

    struct VertexBufferLayout {
        uint64_t arrayStride;
        VertexStepMode stepMode = VertexStepMode::Vertex;
        size_t attributeCount;
        VertexAttribute const * attributes;
    };

    struct BindGroupLayoutDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        size_t entryCount;
        BindGroupLayoutEntry const * entries;
    };

    struct ColorTargetState {
        ChainedStruct const * nextInChain = nullptr;
        TextureFormat format;
        BlendState const * blend = nullptr;
        ColorWriteMask writeMask = ColorWriteMask::All;
    };

    struct ComputePipelineDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        PipelineLayout layout = nullptr;
        ProgrammableStageDescriptor compute;
    };

    struct DeviceDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        size_t requiredFeatureCount = 0;
        FeatureName const * requiredFeatures = nullptr;
        RequiredLimits const * requiredLimits = nullptr;
        QueueDescriptor defaultQueue;
        DeviceLostCallback deviceLostCallback = nullptr;
        void * deviceLostUserdata = nullptr;
    };

    struct RenderPassDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        size_t colorAttachmentCount;
        RenderPassColorAttachment const * colorAttachments;
        RenderPassDepthStencilAttachment const * depthStencilAttachment = nullptr;
        QuerySet occlusionQuerySet = nullptr;
        RenderPassTimestampWrites const * timestampWrites = nullptr;
    };

    struct VertexState {
        ChainedStruct const * nextInChain = nullptr;
        ShaderModule module;
        char const * entryPoint = nullptr;
        size_t constantCount = 0;
        ConstantEntry const * constants;
        size_t bufferCount = 0;
        VertexBufferLayout const * buffers;
    };

    struct FragmentState {
        ChainedStruct const * nextInChain = nullptr;
        ShaderModule module;
        char const * entryPoint = nullptr;
        size_t constantCount = 0;
        ConstantEntry const * constants;
        size_t targetCount;
        ColorTargetState const * targets;
    };

    struct RenderPipelineDescriptor {
        ChainedStruct const * nextInChain = nullptr;
        char const * label = nullptr;
        PipelineLayout layout = nullptr;
        VertexState vertex;
        PrimitiveState primitive;
        DepthStencilState const * depthStencil = nullptr;
        MultisampleState multisample;
        FragmentState const * fragment = nullptr;
    };

}  // namespace wgpu

namespace wgpu {
    template<>
    struct IsWGPUBitmask<wgpu::BufferUsage> {
        static constexpr bool enable = true;
    };

    template<>
    struct IsWGPUBitmask<wgpu::ColorWriteMask> {
        static constexpr bool enable = true;
    };

    template<>
    struct IsWGPUBitmask<wgpu::MapMode> {
        static constexpr bool enable = true;
    };

    template<>
    struct IsWGPUBitmask<wgpu::ShaderStage> {
        static constexpr bool enable = true;
    };

    template<>
    struct IsWGPUBitmask<wgpu::TextureUsage> {
        static constexpr bool enable = true;
    };

} // namespace wgpu

namespace std {
// Custom boolean class needs corresponding hash function so that it appears as a transparent bool.
template <>
struct hash<wgpu::Bool> {
  public:
    size_t operator()(const wgpu::Bool &v) const {
        return hash<bool>()(v);
    }
};
}  // namespace std

#endif // WEBGPU_CPP_H_
