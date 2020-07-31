#include "webgpu/webgpu_cpp.h"

namespace wgpu {

    // AdapterType

    static_assert(sizeof(AdapterType) == sizeof(WGPUAdapterType), "sizeof mismatch for AdapterType");
    static_assert(alignof(AdapterType) == alignof(WGPUAdapterType), "alignof mismatch for AdapterType");

    static_assert(static_cast<uint32_t>(AdapterType::DiscreteGPU) == WGPUAdapterType_DiscreteGPU, "value mismatch for AdapterType::DiscreteGPU");
    static_assert(static_cast<uint32_t>(AdapterType::IntegratedGPU) == WGPUAdapterType_IntegratedGPU, "value mismatch for AdapterType::IntegratedGPU");
    static_assert(static_cast<uint32_t>(AdapterType::CPU) == WGPUAdapterType_CPU, "value mismatch for AdapterType::CPU");
    static_assert(static_cast<uint32_t>(AdapterType::Unknown) == WGPUAdapterType_Unknown, "value mismatch for AdapterType::Unknown");

    // AddressMode

    static_assert(sizeof(AddressMode) == sizeof(WGPUAddressMode), "sizeof mismatch for AddressMode");
    static_assert(alignof(AddressMode) == alignof(WGPUAddressMode), "alignof mismatch for AddressMode");

    static_assert(static_cast<uint32_t>(AddressMode::Repeat) == WGPUAddressMode_Repeat, "value mismatch for AddressMode::Repeat");
    static_assert(static_cast<uint32_t>(AddressMode::MirrorRepeat) == WGPUAddressMode_MirrorRepeat, "value mismatch for AddressMode::MirrorRepeat");
    static_assert(static_cast<uint32_t>(AddressMode::ClampToEdge) == WGPUAddressMode_ClampToEdge, "value mismatch for AddressMode::ClampToEdge");

    // BackendType

    static_assert(sizeof(BackendType) == sizeof(WGPUBackendType), "sizeof mismatch for BackendType");
    static_assert(alignof(BackendType) == alignof(WGPUBackendType), "alignof mismatch for BackendType");

    static_assert(static_cast<uint32_t>(BackendType::Null) == WGPUBackendType_Null, "value mismatch for BackendType::Null");
    static_assert(static_cast<uint32_t>(BackendType::D3D11) == WGPUBackendType_D3D11, "value mismatch for BackendType::D3D11");
    static_assert(static_cast<uint32_t>(BackendType::D3D12) == WGPUBackendType_D3D12, "value mismatch for BackendType::D3D12");
    static_assert(static_cast<uint32_t>(BackendType::Metal) == WGPUBackendType_Metal, "value mismatch for BackendType::Metal");
    static_assert(static_cast<uint32_t>(BackendType::Vulkan) == WGPUBackendType_Vulkan, "value mismatch for BackendType::Vulkan");
    static_assert(static_cast<uint32_t>(BackendType::OpenGL) == WGPUBackendType_OpenGL, "value mismatch for BackendType::OpenGL");
    static_assert(static_cast<uint32_t>(BackendType::OpenGLES) == WGPUBackendType_OpenGLES, "value mismatch for BackendType::OpenGLES");

    // BindingType

    static_assert(sizeof(BindingType) == sizeof(WGPUBindingType), "sizeof mismatch for BindingType");
    static_assert(alignof(BindingType) == alignof(WGPUBindingType), "alignof mismatch for BindingType");

    static_assert(static_cast<uint32_t>(BindingType::UniformBuffer) == WGPUBindingType_UniformBuffer, "value mismatch for BindingType::UniformBuffer");
    static_assert(static_cast<uint32_t>(BindingType::StorageBuffer) == WGPUBindingType_StorageBuffer, "value mismatch for BindingType::StorageBuffer");
    static_assert(static_cast<uint32_t>(BindingType::ReadonlyStorageBuffer) == WGPUBindingType_ReadonlyStorageBuffer, "value mismatch for BindingType::ReadonlyStorageBuffer");
    static_assert(static_cast<uint32_t>(BindingType::Sampler) == WGPUBindingType_Sampler, "value mismatch for BindingType::Sampler");
    static_assert(static_cast<uint32_t>(BindingType::ComparisonSampler) == WGPUBindingType_ComparisonSampler, "value mismatch for BindingType::ComparisonSampler");
    static_assert(static_cast<uint32_t>(BindingType::SampledTexture) == WGPUBindingType_SampledTexture, "value mismatch for BindingType::SampledTexture");
    static_assert(static_cast<uint32_t>(BindingType::ReadonlyStorageTexture) == WGPUBindingType_ReadonlyStorageTexture, "value mismatch for BindingType::ReadonlyStorageTexture");
    static_assert(static_cast<uint32_t>(BindingType::WriteonlyStorageTexture) == WGPUBindingType_WriteonlyStorageTexture, "value mismatch for BindingType::WriteonlyStorageTexture");

    // BlendFactor

    static_assert(sizeof(BlendFactor) == sizeof(WGPUBlendFactor), "sizeof mismatch for BlendFactor");
    static_assert(alignof(BlendFactor) == alignof(WGPUBlendFactor), "alignof mismatch for BlendFactor");

    static_assert(static_cast<uint32_t>(BlendFactor::Zero) == WGPUBlendFactor_Zero, "value mismatch for BlendFactor::Zero");
    static_assert(static_cast<uint32_t>(BlendFactor::One) == WGPUBlendFactor_One, "value mismatch for BlendFactor::One");
    static_assert(static_cast<uint32_t>(BlendFactor::SrcColor) == WGPUBlendFactor_SrcColor, "value mismatch for BlendFactor::SrcColor");
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusSrcColor) == WGPUBlendFactor_OneMinusSrcColor, "value mismatch for BlendFactor::OneMinusSrcColor");
    static_assert(static_cast<uint32_t>(BlendFactor::SrcAlpha) == WGPUBlendFactor_SrcAlpha, "value mismatch for BlendFactor::SrcAlpha");
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusSrcAlpha) == WGPUBlendFactor_OneMinusSrcAlpha, "value mismatch for BlendFactor::OneMinusSrcAlpha");
    static_assert(static_cast<uint32_t>(BlendFactor::DstColor) == WGPUBlendFactor_DstColor, "value mismatch for BlendFactor::DstColor");
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusDstColor) == WGPUBlendFactor_OneMinusDstColor, "value mismatch for BlendFactor::OneMinusDstColor");
    static_assert(static_cast<uint32_t>(BlendFactor::DstAlpha) == WGPUBlendFactor_DstAlpha, "value mismatch for BlendFactor::DstAlpha");
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusDstAlpha) == WGPUBlendFactor_OneMinusDstAlpha, "value mismatch for BlendFactor::OneMinusDstAlpha");
    static_assert(static_cast<uint32_t>(BlendFactor::SrcAlphaSaturated) == WGPUBlendFactor_SrcAlphaSaturated, "value mismatch for BlendFactor::SrcAlphaSaturated");
    static_assert(static_cast<uint32_t>(BlendFactor::BlendColor) == WGPUBlendFactor_BlendColor, "value mismatch for BlendFactor::BlendColor");
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusBlendColor) == WGPUBlendFactor_OneMinusBlendColor, "value mismatch for BlendFactor::OneMinusBlendColor");

    // BlendOperation

    static_assert(sizeof(BlendOperation) == sizeof(WGPUBlendOperation), "sizeof mismatch for BlendOperation");
    static_assert(alignof(BlendOperation) == alignof(WGPUBlendOperation), "alignof mismatch for BlendOperation");

    static_assert(static_cast<uint32_t>(BlendOperation::Add) == WGPUBlendOperation_Add, "value mismatch for BlendOperation::Add");
    static_assert(static_cast<uint32_t>(BlendOperation::Subtract) == WGPUBlendOperation_Subtract, "value mismatch for BlendOperation::Subtract");
    static_assert(static_cast<uint32_t>(BlendOperation::ReverseSubtract) == WGPUBlendOperation_ReverseSubtract, "value mismatch for BlendOperation::ReverseSubtract");
    static_assert(static_cast<uint32_t>(BlendOperation::Min) == WGPUBlendOperation_Min, "value mismatch for BlendOperation::Min");
    static_assert(static_cast<uint32_t>(BlendOperation::Max) == WGPUBlendOperation_Max, "value mismatch for BlendOperation::Max");

    // BufferMapAsyncStatus

    static_assert(sizeof(BufferMapAsyncStatus) == sizeof(WGPUBufferMapAsyncStatus), "sizeof mismatch for BufferMapAsyncStatus");
    static_assert(alignof(BufferMapAsyncStatus) == alignof(WGPUBufferMapAsyncStatus), "alignof mismatch for BufferMapAsyncStatus");

    static_assert(static_cast<uint32_t>(BufferMapAsyncStatus::Success) == WGPUBufferMapAsyncStatus_Success, "value mismatch for BufferMapAsyncStatus::Success");
    static_assert(static_cast<uint32_t>(BufferMapAsyncStatus::Error) == WGPUBufferMapAsyncStatus_Error, "value mismatch for BufferMapAsyncStatus::Error");
    static_assert(static_cast<uint32_t>(BufferMapAsyncStatus::Unknown) == WGPUBufferMapAsyncStatus_Unknown, "value mismatch for BufferMapAsyncStatus::Unknown");
    static_assert(static_cast<uint32_t>(BufferMapAsyncStatus::DeviceLost) == WGPUBufferMapAsyncStatus_DeviceLost, "value mismatch for BufferMapAsyncStatus::DeviceLost");

    // CompareFunction

    static_assert(sizeof(CompareFunction) == sizeof(WGPUCompareFunction), "sizeof mismatch for CompareFunction");
    static_assert(alignof(CompareFunction) == alignof(WGPUCompareFunction), "alignof mismatch for CompareFunction");

    static_assert(static_cast<uint32_t>(CompareFunction::Undefined) == WGPUCompareFunction_Undefined, "value mismatch for CompareFunction::Undefined");
    static_assert(static_cast<uint32_t>(CompareFunction::Never) == WGPUCompareFunction_Never, "value mismatch for CompareFunction::Never");
    static_assert(static_cast<uint32_t>(CompareFunction::Less) == WGPUCompareFunction_Less, "value mismatch for CompareFunction::Less");
    static_assert(static_cast<uint32_t>(CompareFunction::LessEqual) == WGPUCompareFunction_LessEqual, "value mismatch for CompareFunction::LessEqual");
    static_assert(static_cast<uint32_t>(CompareFunction::Greater) == WGPUCompareFunction_Greater, "value mismatch for CompareFunction::Greater");
    static_assert(static_cast<uint32_t>(CompareFunction::GreaterEqual) == WGPUCompareFunction_GreaterEqual, "value mismatch for CompareFunction::GreaterEqual");
    static_assert(static_cast<uint32_t>(CompareFunction::Equal) == WGPUCompareFunction_Equal, "value mismatch for CompareFunction::Equal");
    static_assert(static_cast<uint32_t>(CompareFunction::NotEqual) == WGPUCompareFunction_NotEqual, "value mismatch for CompareFunction::NotEqual");
    static_assert(static_cast<uint32_t>(CompareFunction::Always) == WGPUCompareFunction_Always, "value mismatch for CompareFunction::Always");

    // CullMode

    static_assert(sizeof(CullMode) == sizeof(WGPUCullMode), "sizeof mismatch for CullMode");
    static_assert(alignof(CullMode) == alignof(WGPUCullMode), "alignof mismatch for CullMode");

    static_assert(static_cast<uint32_t>(CullMode::None) == WGPUCullMode_None, "value mismatch for CullMode::None");
    static_assert(static_cast<uint32_t>(CullMode::Front) == WGPUCullMode_Front, "value mismatch for CullMode::Front");
    static_assert(static_cast<uint32_t>(CullMode::Back) == WGPUCullMode_Back, "value mismatch for CullMode::Back");

    // ErrorFilter

    static_assert(sizeof(ErrorFilter) == sizeof(WGPUErrorFilter), "sizeof mismatch for ErrorFilter");
    static_assert(alignof(ErrorFilter) == alignof(WGPUErrorFilter), "alignof mismatch for ErrorFilter");

    static_assert(static_cast<uint32_t>(ErrorFilter::None) == WGPUErrorFilter_None, "value mismatch for ErrorFilter::None");
    static_assert(static_cast<uint32_t>(ErrorFilter::Validation) == WGPUErrorFilter_Validation, "value mismatch for ErrorFilter::Validation");
    static_assert(static_cast<uint32_t>(ErrorFilter::OutOfMemory) == WGPUErrorFilter_OutOfMemory, "value mismatch for ErrorFilter::OutOfMemory");

    // ErrorType

    static_assert(sizeof(ErrorType) == sizeof(WGPUErrorType), "sizeof mismatch for ErrorType");
    static_assert(alignof(ErrorType) == alignof(WGPUErrorType), "alignof mismatch for ErrorType");

    static_assert(static_cast<uint32_t>(ErrorType::NoError) == WGPUErrorType_NoError, "value mismatch for ErrorType::NoError");
    static_assert(static_cast<uint32_t>(ErrorType::Validation) == WGPUErrorType_Validation, "value mismatch for ErrorType::Validation");
    static_assert(static_cast<uint32_t>(ErrorType::OutOfMemory) == WGPUErrorType_OutOfMemory, "value mismatch for ErrorType::OutOfMemory");
    static_assert(static_cast<uint32_t>(ErrorType::Unknown) == WGPUErrorType_Unknown, "value mismatch for ErrorType::Unknown");
    static_assert(static_cast<uint32_t>(ErrorType::DeviceLost) == WGPUErrorType_DeviceLost, "value mismatch for ErrorType::DeviceLost");

    // FenceCompletionStatus

    static_assert(sizeof(FenceCompletionStatus) == sizeof(WGPUFenceCompletionStatus), "sizeof mismatch for FenceCompletionStatus");
    static_assert(alignof(FenceCompletionStatus) == alignof(WGPUFenceCompletionStatus), "alignof mismatch for FenceCompletionStatus");

    static_assert(static_cast<uint32_t>(FenceCompletionStatus::Success) == WGPUFenceCompletionStatus_Success, "value mismatch for FenceCompletionStatus::Success");
    static_assert(static_cast<uint32_t>(FenceCompletionStatus::Error) == WGPUFenceCompletionStatus_Error, "value mismatch for FenceCompletionStatus::Error");
    static_assert(static_cast<uint32_t>(FenceCompletionStatus::Unknown) == WGPUFenceCompletionStatus_Unknown, "value mismatch for FenceCompletionStatus::Unknown");
    static_assert(static_cast<uint32_t>(FenceCompletionStatus::DeviceLost) == WGPUFenceCompletionStatus_DeviceLost, "value mismatch for FenceCompletionStatus::DeviceLost");

    // FilterMode

    static_assert(sizeof(FilterMode) == sizeof(WGPUFilterMode), "sizeof mismatch for FilterMode");
    static_assert(alignof(FilterMode) == alignof(WGPUFilterMode), "alignof mismatch for FilterMode");

    static_assert(static_cast<uint32_t>(FilterMode::Nearest) == WGPUFilterMode_Nearest, "value mismatch for FilterMode::Nearest");
    static_assert(static_cast<uint32_t>(FilterMode::Linear) == WGPUFilterMode_Linear, "value mismatch for FilterMode::Linear");

    // FrontFace

    static_assert(sizeof(FrontFace) == sizeof(WGPUFrontFace), "sizeof mismatch for FrontFace");
    static_assert(alignof(FrontFace) == alignof(WGPUFrontFace), "alignof mismatch for FrontFace");

    static_assert(static_cast<uint32_t>(FrontFace::CCW) == WGPUFrontFace_CCW, "value mismatch for FrontFace::CCW");
    static_assert(static_cast<uint32_t>(FrontFace::CW) == WGPUFrontFace_CW, "value mismatch for FrontFace::CW");

    // IndexFormat

    static_assert(sizeof(IndexFormat) == sizeof(WGPUIndexFormat), "sizeof mismatch for IndexFormat");
    static_assert(alignof(IndexFormat) == alignof(WGPUIndexFormat), "alignof mismatch for IndexFormat");

    static_assert(static_cast<uint32_t>(IndexFormat::Uint16) == WGPUIndexFormat_Uint16, "value mismatch for IndexFormat::Uint16");
    static_assert(static_cast<uint32_t>(IndexFormat::Uint32) == WGPUIndexFormat_Uint32, "value mismatch for IndexFormat::Uint32");

    // InputStepMode

    static_assert(sizeof(InputStepMode) == sizeof(WGPUInputStepMode), "sizeof mismatch for InputStepMode");
    static_assert(alignof(InputStepMode) == alignof(WGPUInputStepMode), "alignof mismatch for InputStepMode");

    static_assert(static_cast<uint32_t>(InputStepMode::Vertex) == WGPUInputStepMode_Vertex, "value mismatch for InputStepMode::Vertex");
    static_assert(static_cast<uint32_t>(InputStepMode::Instance) == WGPUInputStepMode_Instance, "value mismatch for InputStepMode::Instance");

    // LoadOp

    static_assert(sizeof(LoadOp) == sizeof(WGPULoadOp), "sizeof mismatch for LoadOp");
    static_assert(alignof(LoadOp) == alignof(WGPULoadOp), "alignof mismatch for LoadOp");

    static_assert(static_cast<uint32_t>(LoadOp::Clear) == WGPULoadOp_Clear, "value mismatch for LoadOp::Clear");
    static_assert(static_cast<uint32_t>(LoadOp::Load) == WGPULoadOp_Load, "value mismatch for LoadOp::Load");

    // PipelineStatisticName

    static_assert(sizeof(PipelineStatisticName) == sizeof(WGPUPipelineStatisticName), "sizeof mismatch for PipelineStatisticName");
    static_assert(alignof(PipelineStatisticName) == alignof(WGPUPipelineStatisticName), "alignof mismatch for PipelineStatisticName");

    static_assert(static_cast<uint32_t>(PipelineStatisticName::VertexShaderInvocations) == WGPUPipelineStatisticName_VertexShaderInvocations, "value mismatch for PipelineStatisticName::VertexShaderInvocations");
    static_assert(static_cast<uint32_t>(PipelineStatisticName::ClipperInvocations) == WGPUPipelineStatisticName_ClipperInvocations, "value mismatch for PipelineStatisticName::ClipperInvocations");
    static_assert(static_cast<uint32_t>(PipelineStatisticName::ClipperPrimitivesOut) == WGPUPipelineStatisticName_ClipperPrimitivesOut, "value mismatch for PipelineStatisticName::ClipperPrimitivesOut");
    static_assert(static_cast<uint32_t>(PipelineStatisticName::FragmentShaderInvocations) == WGPUPipelineStatisticName_FragmentShaderInvocations, "value mismatch for PipelineStatisticName::FragmentShaderInvocations");
    static_assert(static_cast<uint32_t>(PipelineStatisticName::ComputeShaderInvocations) == WGPUPipelineStatisticName_ComputeShaderInvocations, "value mismatch for PipelineStatisticName::ComputeShaderInvocations");

    // PresentMode

    static_assert(sizeof(PresentMode) == sizeof(WGPUPresentMode), "sizeof mismatch for PresentMode");
    static_assert(alignof(PresentMode) == alignof(WGPUPresentMode), "alignof mismatch for PresentMode");

    static_assert(static_cast<uint32_t>(PresentMode::Immediate) == WGPUPresentMode_Immediate, "value mismatch for PresentMode::Immediate");
    static_assert(static_cast<uint32_t>(PresentMode::Mailbox) == WGPUPresentMode_Mailbox, "value mismatch for PresentMode::Mailbox");
    static_assert(static_cast<uint32_t>(PresentMode::Fifo) == WGPUPresentMode_Fifo, "value mismatch for PresentMode::Fifo");

    // PrimitiveTopology

    static_assert(sizeof(PrimitiveTopology) == sizeof(WGPUPrimitiveTopology), "sizeof mismatch for PrimitiveTopology");
    static_assert(alignof(PrimitiveTopology) == alignof(WGPUPrimitiveTopology), "alignof mismatch for PrimitiveTopology");

    static_assert(static_cast<uint32_t>(PrimitiveTopology::PointList) == WGPUPrimitiveTopology_PointList, "value mismatch for PrimitiveTopology::PointList");
    static_assert(static_cast<uint32_t>(PrimitiveTopology::LineList) == WGPUPrimitiveTopology_LineList, "value mismatch for PrimitiveTopology::LineList");
    static_assert(static_cast<uint32_t>(PrimitiveTopology::LineStrip) == WGPUPrimitiveTopology_LineStrip, "value mismatch for PrimitiveTopology::LineStrip");
    static_assert(static_cast<uint32_t>(PrimitiveTopology::TriangleList) == WGPUPrimitiveTopology_TriangleList, "value mismatch for PrimitiveTopology::TriangleList");
    static_assert(static_cast<uint32_t>(PrimitiveTopology::TriangleStrip) == WGPUPrimitiveTopology_TriangleStrip, "value mismatch for PrimitiveTopology::TriangleStrip");

    // QueryType

    static_assert(sizeof(QueryType) == sizeof(WGPUQueryType), "sizeof mismatch for QueryType");
    static_assert(alignof(QueryType) == alignof(WGPUQueryType), "alignof mismatch for QueryType");

    static_assert(static_cast<uint32_t>(QueryType::Occlusion) == WGPUQueryType_Occlusion, "value mismatch for QueryType::Occlusion");
    static_assert(static_cast<uint32_t>(QueryType::PipelineStatistics) == WGPUQueryType_PipelineStatistics, "value mismatch for QueryType::PipelineStatistics");
    static_assert(static_cast<uint32_t>(QueryType::Timestamp) == WGPUQueryType_Timestamp, "value mismatch for QueryType::Timestamp");

    // SType

    static_assert(sizeof(SType) == sizeof(WGPUSType), "sizeof mismatch for SType");
    static_assert(alignof(SType) == alignof(WGPUSType), "alignof mismatch for SType");

    static_assert(static_cast<uint32_t>(SType::Invalid) == WGPUSType_Invalid, "value mismatch for SType::Invalid");
    static_assert(static_cast<uint32_t>(SType::SurfaceDescriptorFromMetalLayer) == WGPUSType_SurfaceDescriptorFromMetalLayer, "value mismatch for SType::SurfaceDescriptorFromMetalLayer");
    static_assert(static_cast<uint32_t>(SType::SurfaceDescriptorFromWindowsHWND) == WGPUSType_SurfaceDescriptorFromWindowsHWND, "value mismatch for SType::SurfaceDescriptorFromWindowsHWND");
    static_assert(static_cast<uint32_t>(SType::SurfaceDescriptorFromXlib) == WGPUSType_SurfaceDescriptorFromXlib, "value mismatch for SType::SurfaceDescriptorFromXlib");
    static_assert(static_cast<uint32_t>(SType::SurfaceDescriptorFromCanvasHTMLSelector) == WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector, "value mismatch for SType::SurfaceDescriptorFromCanvasHTMLSelector");
    static_assert(static_cast<uint32_t>(SType::ShaderModuleSPIRVDescriptor) == WGPUSType_ShaderModuleSPIRVDescriptor, "value mismatch for SType::ShaderModuleSPIRVDescriptor");
    static_assert(static_cast<uint32_t>(SType::ShaderModuleWGSLDescriptor) == WGPUSType_ShaderModuleWGSLDescriptor, "value mismatch for SType::ShaderModuleWGSLDescriptor");

    // StencilOperation

    static_assert(sizeof(StencilOperation) == sizeof(WGPUStencilOperation), "sizeof mismatch for StencilOperation");
    static_assert(alignof(StencilOperation) == alignof(WGPUStencilOperation), "alignof mismatch for StencilOperation");

    static_assert(static_cast<uint32_t>(StencilOperation::Keep) == WGPUStencilOperation_Keep, "value mismatch for StencilOperation::Keep");
    static_assert(static_cast<uint32_t>(StencilOperation::Zero) == WGPUStencilOperation_Zero, "value mismatch for StencilOperation::Zero");
    static_assert(static_cast<uint32_t>(StencilOperation::Replace) == WGPUStencilOperation_Replace, "value mismatch for StencilOperation::Replace");
    static_assert(static_cast<uint32_t>(StencilOperation::Invert) == WGPUStencilOperation_Invert, "value mismatch for StencilOperation::Invert");
    static_assert(static_cast<uint32_t>(StencilOperation::IncrementClamp) == WGPUStencilOperation_IncrementClamp, "value mismatch for StencilOperation::IncrementClamp");
    static_assert(static_cast<uint32_t>(StencilOperation::DecrementClamp) == WGPUStencilOperation_DecrementClamp, "value mismatch for StencilOperation::DecrementClamp");
    static_assert(static_cast<uint32_t>(StencilOperation::IncrementWrap) == WGPUStencilOperation_IncrementWrap, "value mismatch for StencilOperation::IncrementWrap");
    static_assert(static_cast<uint32_t>(StencilOperation::DecrementWrap) == WGPUStencilOperation_DecrementWrap, "value mismatch for StencilOperation::DecrementWrap");

    // StoreOp

    static_assert(sizeof(StoreOp) == sizeof(WGPUStoreOp), "sizeof mismatch for StoreOp");
    static_assert(alignof(StoreOp) == alignof(WGPUStoreOp), "alignof mismatch for StoreOp");

    static_assert(static_cast<uint32_t>(StoreOp::Store) == WGPUStoreOp_Store, "value mismatch for StoreOp::Store");
    static_assert(static_cast<uint32_t>(StoreOp::Clear) == WGPUStoreOp_Clear, "value mismatch for StoreOp::Clear");

    // TextureAspect

    static_assert(sizeof(TextureAspect) == sizeof(WGPUTextureAspect), "sizeof mismatch for TextureAspect");
    static_assert(alignof(TextureAspect) == alignof(WGPUTextureAspect), "alignof mismatch for TextureAspect");

    static_assert(static_cast<uint32_t>(TextureAspect::All) == WGPUTextureAspect_All, "value mismatch for TextureAspect::All");
    static_assert(static_cast<uint32_t>(TextureAspect::StencilOnly) == WGPUTextureAspect_StencilOnly, "value mismatch for TextureAspect::StencilOnly");
    static_assert(static_cast<uint32_t>(TextureAspect::DepthOnly) == WGPUTextureAspect_DepthOnly, "value mismatch for TextureAspect::DepthOnly");

    // TextureComponentType

    static_assert(sizeof(TextureComponentType) == sizeof(WGPUTextureComponentType), "sizeof mismatch for TextureComponentType");
    static_assert(alignof(TextureComponentType) == alignof(WGPUTextureComponentType), "alignof mismatch for TextureComponentType");

    static_assert(static_cast<uint32_t>(TextureComponentType::Float) == WGPUTextureComponentType_Float, "value mismatch for TextureComponentType::Float");
    static_assert(static_cast<uint32_t>(TextureComponentType::Sint) == WGPUTextureComponentType_Sint, "value mismatch for TextureComponentType::Sint");
    static_assert(static_cast<uint32_t>(TextureComponentType::Uint) == WGPUTextureComponentType_Uint, "value mismatch for TextureComponentType::Uint");

    // TextureDimension

    static_assert(sizeof(TextureDimension) == sizeof(WGPUTextureDimension), "sizeof mismatch for TextureDimension");
    static_assert(alignof(TextureDimension) == alignof(WGPUTextureDimension), "alignof mismatch for TextureDimension");

    static_assert(static_cast<uint32_t>(TextureDimension::e1D) == WGPUTextureDimension_1D, "value mismatch for TextureDimension::e1D");
    static_assert(static_cast<uint32_t>(TextureDimension::e2D) == WGPUTextureDimension_2D, "value mismatch for TextureDimension::e2D");
    static_assert(static_cast<uint32_t>(TextureDimension::e3D) == WGPUTextureDimension_3D, "value mismatch for TextureDimension::e3D");

    // TextureFormat

    static_assert(sizeof(TextureFormat) == sizeof(WGPUTextureFormat), "sizeof mismatch for TextureFormat");
    static_assert(alignof(TextureFormat) == alignof(WGPUTextureFormat), "alignof mismatch for TextureFormat");

    static_assert(static_cast<uint32_t>(TextureFormat::Undefined) == WGPUTextureFormat_Undefined, "value mismatch for TextureFormat::Undefined");
    static_assert(static_cast<uint32_t>(TextureFormat::R8Unorm) == WGPUTextureFormat_R8Unorm, "value mismatch for TextureFormat::R8Unorm");
    static_assert(static_cast<uint32_t>(TextureFormat::R8Snorm) == WGPUTextureFormat_R8Snorm, "value mismatch for TextureFormat::R8Snorm");
    static_assert(static_cast<uint32_t>(TextureFormat::R8Uint) == WGPUTextureFormat_R8Uint, "value mismatch for TextureFormat::R8Uint");
    static_assert(static_cast<uint32_t>(TextureFormat::R8Sint) == WGPUTextureFormat_R8Sint, "value mismatch for TextureFormat::R8Sint");
    static_assert(static_cast<uint32_t>(TextureFormat::R16Uint) == WGPUTextureFormat_R16Uint, "value mismatch for TextureFormat::R16Uint");
    static_assert(static_cast<uint32_t>(TextureFormat::R16Sint) == WGPUTextureFormat_R16Sint, "value mismatch for TextureFormat::R16Sint");
    static_assert(static_cast<uint32_t>(TextureFormat::R16Float) == WGPUTextureFormat_R16Float, "value mismatch for TextureFormat::R16Float");
    static_assert(static_cast<uint32_t>(TextureFormat::RG8Unorm) == WGPUTextureFormat_RG8Unorm, "value mismatch for TextureFormat::RG8Unorm");
    static_assert(static_cast<uint32_t>(TextureFormat::RG8Snorm) == WGPUTextureFormat_RG8Snorm, "value mismatch for TextureFormat::RG8Snorm");
    static_assert(static_cast<uint32_t>(TextureFormat::RG8Uint) == WGPUTextureFormat_RG8Uint, "value mismatch for TextureFormat::RG8Uint");
    static_assert(static_cast<uint32_t>(TextureFormat::RG8Sint) == WGPUTextureFormat_RG8Sint, "value mismatch for TextureFormat::RG8Sint");
    static_assert(static_cast<uint32_t>(TextureFormat::R32Float) == WGPUTextureFormat_R32Float, "value mismatch for TextureFormat::R32Float");
    static_assert(static_cast<uint32_t>(TextureFormat::R32Uint) == WGPUTextureFormat_R32Uint, "value mismatch for TextureFormat::R32Uint");
    static_assert(static_cast<uint32_t>(TextureFormat::R32Sint) == WGPUTextureFormat_R32Sint, "value mismatch for TextureFormat::R32Sint");
    static_assert(static_cast<uint32_t>(TextureFormat::RG16Uint) == WGPUTextureFormat_RG16Uint, "value mismatch for TextureFormat::RG16Uint");
    static_assert(static_cast<uint32_t>(TextureFormat::RG16Sint) == WGPUTextureFormat_RG16Sint, "value mismatch for TextureFormat::RG16Sint");
    static_assert(static_cast<uint32_t>(TextureFormat::RG16Float) == WGPUTextureFormat_RG16Float, "value mismatch for TextureFormat::RG16Float");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA8Unorm) == WGPUTextureFormat_RGBA8Unorm, "value mismatch for TextureFormat::RGBA8Unorm");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA8UnormSrgb) == WGPUTextureFormat_RGBA8UnormSrgb, "value mismatch for TextureFormat::RGBA8UnormSrgb");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA8Snorm) == WGPUTextureFormat_RGBA8Snorm, "value mismatch for TextureFormat::RGBA8Snorm");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA8Uint) == WGPUTextureFormat_RGBA8Uint, "value mismatch for TextureFormat::RGBA8Uint");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA8Sint) == WGPUTextureFormat_RGBA8Sint, "value mismatch for TextureFormat::RGBA8Sint");
    static_assert(static_cast<uint32_t>(TextureFormat::BGRA8Unorm) == WGPUTextureFormat_BGRA8Unorm, "value mismatch for TextureFormat::BGRA8Unorm");
    static_assert(static_cast<uint32_t>(TextureFormat::BGRA8UnormSrgb) == WGPUTextureFormat_BGRA8UnormSrgb, "value mismatch for TextureFormat::BGRA8UnormSrgb");
    static_assert(static_cast<uint32_t>(TextureFormat::RGB10A2Unorm) == WGPUTextureFormat_RGB10A2Unorm, "value mismatch for TextureFormat::RGB10A2Unorm");
    static_assert(static_cast<uint32_t>(TextureFormat::RG11B10Float) == WGPUTextureFormat_RG11B10Float, "value mismatch for TextureFormat::RG11B10Float");
    static_assert(static_cast<uint32_t>(TextureFormat::RG32Float) == WGPUTextureFormat_RG32Float, "value mismatch for TextureFormat::RG32Float");
    static_assert(static_cast<uint32_t>(TextureFormat::RG32Uint) == WGPUTextureFormat_RG32Uint, "value mismatch for TextureFormat::RG32Uint");
    static_assert(static_cast<uint32_t>(TextureFormat::RG32Sint) == WGPUTextureFormat_RG32Sint, "value mismatch for TextureFormat::RG32Sint");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA16Uint) == WGPUTextureFormat_RGBA16Uint, "value mismatch for TextureFormat::RGBA16Uint");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA16Sint) == WGPUTextureFormat_RGBA16Sint, "value mismatch for TextureFormat::RGBA16Sint");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA16Float) == WGPUTextureFormat_RGBA16Float, "value mismatch for TextureFormat::RGBA16Float");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA32Float) == WGPUTextureFormat_RGBA32Float, "value mismatch for TextureFormat::RGBA32Float");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA32Uint) == WGPUTextureFormat_RGBA32Uint, "value mismatch for TextureFormat::RGBA32Uint");
    static_assert(static_cast<uint32_t>(TextureFormat::RGBA32Sint) == WGPUTextureFormat_RGBA32Sint, "value mismatch for TextureFormat::RGBA32Sint");
    static_assert(static_cast<uint32_t>(TextureFormat::Depth32Float) == WGPUTextureFormat_Depth32Float, "value mismatch for TextureFormat::Depth32Float");
    static_assert(static_cast<uint32_t>(TextureFormat::Depth24Plus) == WGPUTextureFormat_Depth24Plus, "value mismatch for TextureFormat::Depth24Plus");
    static_assert(static_cast<uint32_t>(TextureFormat::Depth24PlusStencil8) == WGPUTextureFormat_Depth24PlusStencil8, "value mismatch for TextureFormat::Depth24PlusStencil8");
    static_assert(static_cast<uint32_t>(TextureFormat::BC1RGBAUnorm) == WGPUTextureFormat_BC1RGBAUnorm, "value mismatch for TextureFormat::BC1RGBAUnorm");
    static_assert(static_cast<uint32_t>(TextureFormat::BC1RGBAUnormSrgb) == WGPUTextureFormat_BC1RGBAUnormSrgb, "value mismatch for TextureFormat::BC1RGBAUnormSrgb");
    static_assert(static_cast<uint32_t>(TextureFormat::BC2RGBAUnorm) == WGPUTextureFormat_BC2RGBAUnorm, "value mismatch for TextureFormat::BC2RGBAUnorm");
    static_assert(static_cast<uint32_t>(TextureFormat::BC2RGBAUnormSrgb) == WGPUTextureFormat_BC2RGBAUnormSrgb, "value mismatch for TextureFormat::BC2RGBAUnormSrgb");
    static_assert(static_cast<uint32_t>(TextureFormat::BC3RGBAUnorm) == WGPUTextureFormat_BC3RGBAUnorm, "value mismatch for TextureFormat::BC3RGBAUnorm");
    static_assert(static_cast<uint32_t>(TextureFormat::BC3RGBAUnormSrgb) == WGPUTextureFormat_BC3RGBAUnormSrgb, "value mismatch for TextureFormat::BC3RGBAUnormSrgb");
    static_assert(static_cast<uint32_t>(TextureFormat::BC4RUnorm) == WGPUTextureFormat_BC4RUnorm, "value mismatch for TextureFormat::BC4RUnorm");
    static_assert(static_cast<uint32_t>(TextureFormat::BC4RSnorm) == WGPUTextureFormat_BC4RSnorm, "value mismatch for TextureFormat::BC4RSnorm");
    static_assert(static_cast<uint32_t>(TextureFormat::BC5RGUnorm) == WGPUTextureFormat_BC5RGUnorm, "value mismatch for TextureFormat::BC5RGUnorm");
    static_assert(static_cast<uint32_t>(TextureFormat::BC5RGSnorm) == WGPUTextureFormat_BC5RGSnorm, "value mismatch for TextureFormat::BC5RGSnorm");
    static_assert(static_cast<uint32_t>(TextureFormat::BC6HRGBUfloat) == WGPUTextureFormat_BC6HRGBUfloat, "value mismatch for TextureFormat::BC6HRGBUfloat");
    static_assert(static_cast<uint32_t>(TextureFormat::BC6HRGBSfloat) == WGPUTextureFormat_BC6HRGBSfloat, "value mismatch for TextureFormat::BC6HRGBSfloat");
    static_assert(static_cast<uint32_t>(TextureFormat::BC7RGBAUnorm) == WGPUTextureFormat_BC7RGBAUnorm, "value mismatch for TextureFormat::BC7RGBAUnorm");
    static_assert(static_cast<uint32_t>(TextureFormat::BC7RGBAUnormSrgb) == WGPUTextureFormat_BC7RGBAUnormSrgb, "value mismatch for TextureFormat::BC7RGBAUnormSrgb");

    // TextureViewDimension

    static_assert(sizeof(TextureViewDimension) == sizeof(WGPUTextureViewDimension), "sizeof mismatch for TextureViewDimension");
    static_assert(alignof(TextureViewDimension) == alignof(WGPUTextureViewDimension), "alignof mismatch for TextureViewDimension");

    static_assert(static_cast<uint32_t>(TextureViewDimension::Undefined) == WGPUTextureViewDimension_Undefined, "value mismatch for TextureViewDimension::Undefined");
    static_assert(static_cast<uint32_t>(TextureViewDimension::e1D) == WGPUTextureViewDimension_1D, "value mismatch for TextureViewDimension::e1D");
    static_assert(static_cast<uint32_t>(TextureViewDimension::e2D) == WGPUTextureViewDimension_2D, "value mismatch for TextureViewDimension::e2D");
    static_assert(static_cast<uint32_t>(TextureViewDimension::e2DArray) == WGPUTextureViewDimension_2DArray, "value mismatch for TextureViewDimension::e2DArray");
    static_assert(static_cast<uint32_t>(TextureViewDimension::Cube) == WGPUTextureViewDimension_Cube, "value mismatch for TextureViewDimension::Cube");
    static_assert(static_cast<uint32_t>(TextureViewDimension::CubeArray) == WGPUTextureViewDimension_CubeArray, "value mismatch for TextureViewDimension::CubeArray");
    static_assert(static_cast<uint32_t>(TextureViewDimension::e3D) == WGPUTextureViewDimension_3D, "value mismatch for TextureViewDimension::e3D");

    // VertexFormat

    static_assert(sizeof(VertexFormat) == sizeof(WGPUVertexFormat), "sizeof mismatch for VertexFormat");
    static_assert(alignof(VertexFormat) == alignof(WGPUVertexFormat), "alignof mismatch for VertexFormat");

    static_assert(static_cast<uint32_t>(VertexFormat::UChar2) == WGPUVertexFormat_UChar2, "value mismatch for VertexFormat::UChar2");
    static_assert(static_cast<uint32_t>(VertexFormat::UChar4) == WGPUVertexFormat_UChar4, "value mismatch for VertexFormat::UChar4");
    static_assert(static_cast<uint32_t>(VertexFormat::Char2) == WGPUVertexFormat_Char2, "value mismatch for VertexFormat::Char2");
    static_assert(static_cast<uint32_t>(VertexFormat::Char4) == WGPUVertexFormat_Char4, "value mismatch for VertexFormat::Char4");
    static_assert(static_cast<uint32_t>(VertexFormat::UChar2Norm) == WGPUVertexFormat_UChar2Norm, "value mismatch for VertexFormat::UChar2Norm");
    static_assert(static_cast<uint32_t>(VertexFormat::UChar4Norm) == WGPUVertexFormat_UChar4Norm, "value mismatch for VertexFormat::UChar4Norm");
    static_assert(static_cast<uint32_t>(VertexFormat::Char2Norm) == WGPUVertexFormat_Char2Norm, "value mismatch for VertexFormat::Char2Norm");
    static_assert(static_cast<uint32_t>(VertexFormat::Char4Norm) == WGPUVertexFormat_Char4Norm, "value mismatch for VertexFormat::Char4Norm");
    static_assert(static_cast<uint32_t>(VertexFormat::UShort2) == WGPUVertexFormat_UShort2, "value mismatch for VertexFormat::UShort2");
    static_assert(static_cast<uint32_t>(VertexFormat::UShort4) == WGPUVertexFormat_UShort4, "value mismatch for VertexFormat::UShort4");
    static_assert(static_cast<uint32_t>(VertexFormat::Short2) == WGPUVertexFormat_Short2, "value mismatch for VertexFormat::Short2");
    static_assert(static_cast<uint32_t>(VertexFormat::Short4) == WGPUVertexFormat_Short4, "value mismatch for VertexFormat::Short4");
    static_assert(static_cast<uint32_t>(VertexFormat::UShort2Norm) == WGPUVertexFormat_UShort2Norm, "value mismatch for VertexFormat::UShort2Norm");
    static_assert(static_cast<uint32_t>(VertexFormat::UShort4Norm) == WGPUVertexFormat_UShort4Norm, "value mismatch for VertexFormat::UShort4Norm");
    static_assert(static_cast<uint32_t>(VertexFormat::Short2Norm) == WGPUVertexFormat_Short2Norm, "value mismatch for VertexFormat::Short2Norm");
    static_assert(static_cast<uint32_t>(VertexFormat::Short4Norm) == WGPUVertexFormat_Short4Norm, "value mismatch for VertexFormat::Short4Norm");
    static_assert(static_cast<uint32_t>(VertexFormat::Half2) == WGPUVertexFormat_Half2, "value mismatch for VertexFormat::Half2");
    static_assert(static_cast<uint32_t>(VertexFormat::Half4) == WGPUVertexFormat_Half4, "value mismatch for VertexFormat::Half4");
    static_assert(static_cast<uint32_t>(VertexFormat::Float) == WGPUVertexFormat_Float, "value mismatch for VertexFormat::Float");
    static_assert(static_cast<uint32_t>(VertexFormat::Float2) == WGPUVertexFormat_Float2, "value mismatch for VertexFormat::Float2");
    static_assert(static_cast<uint32_t>(VertexFormat::Float3) == WGPUVertexFormat_Float3, "value mismatch for VertexFormat::Float3");
    static_assert(static_cast<uint32_t>(VertexFormat::Float4) == WGPUVertexFormat_Float4, "value mismatch for VertexFormat::Float4");
    static_assert(static_cast<uint32_t>(VertexFormat::UInt) == WGPUVertexFormat_UInt, "value mismatch for VertexFormat::UInt");
    static_assert(static_cast<uint32_t>(VertexFormat::UInt2) == WGPUVertexFormat_UInt2, "value mismatch for VertexFormat::UInt2");
    static_assert(static_cast<uint32_t>(VertexFormat::UInt3) == WGPUVertexFormat_UInt3, "value mismatch for VertexFormat::UInt3");
    static_assert(static_cast<uint32_t>(VertexFormat::UInt4) == WGPUVertexFormat_UInt4, "value mismatch for VertexFormat::UInt4");
    static_assert(static_cast<uint32_t>(VertexFormat::Int) == WGPUVertexFormat_Int, "value mismatch for VertexFormat::Int");
    static_assert(static_cast<uint32_t>(VertexFormat::Int2) == WGPUVertexFormat_Int2, "value mismatch for VertexFormat::Int2");
    static_assert(static_cast<uint32_t>(VertexFormat::Int3) == WGPUVertexFormat_Int3, "value mismatch for VertexFormat::Int3");
    static_assert(static_cast<uint32_t>(VertexFormat::Int4) == WGPUVertexFormat_Int4, "value mismatch for VertexFormat::Int4");

    // BufferUsage

    static_assert(sizeof(BufferUsage) == sizeof(WGPUBufferUsageFlags), "sizeof mismatch for BufferUsage");
    static_assert(alignof(BufferUsage) == alignof(WGPUBufferUsageFlags), "alignof mismatch for BufferUsage");

    static_assert(static_cast<uint32_t>(BufferUsage::None) == WGPUBufferUsage_None, "value mismatch for BufferUsage::None");
    static_assert(static_cast<uint32_t>(BufferUsage::MapRead) == WGPUBufferUsage_MapRead, "value mismatch for BufferUsage::MapRead");
    static_assert(static_cast<uint32_t>(BufferUsage::MapWrite) == WGPUBufferUsage_MapWrite, "value mismatch for BufferUsage::MapWrite");
    static_assert(static_cast<uint32_t>(BufferUsage::CopySrc) == WGPUBufferUsage_CopySrc, "value mismatch for BufferUsage::CopySrc");
    static_assert(static_cast<uint32_t>(BufferUsage::CopyDst) == WGPUBufferUsage_CopyDst, "value mismatch for BufferUsage::CopyDst");
    static_assert(static_cast<uint32_t>(BufferUsage::Index) == WGPUBufferUsage_Index, "value mismatch for BufferUsage::Index");
    static_assert(static_cast<uint32_t>(BufferUsage::Vertex) == WGPUBufferUsage_Vertex, "value mismatch for BufferUsage::Vertex");
    static_assert(static_cast<uint32_t>(BufferUsage::Uniform) == WGPUBufferUsage_Uniform, "value mismatch for BufferUsage::Uniform");
    static_assert(static_cast<uint32_t>(BufferUsage::Storage) == WGPUBufferUsage_Storage, "value mismatch for BufferUsage::Storage");
    static_assert(static_cast<uint32_t>(BufferUsage::Indirect) == WGPUBufferUsage_Indirect, "value mismatch for BufferUsage::Indirect");
    static_assert(static_cast<uint32_t>(BufferUsage::QueryResolve) == WGPUBufferUsage_QueryResolve, "value mismatch for BufferUsage::QueryResolve");

    // ColorWriteMask

    static_assert(sizeof(ColorWriteMask) == sizeof(WGPUColorWriteMaskFlags), "sizeof mismatch for ColorWriteMask");
    static_assert(alignof(ColorWriteMask) == alignof(WGPUColorWriteMaskFlags), "alignof mismatch for ColorWriteMask");

    static_assert(static_cast<uint32_t>(ColorWriteMask::None) == WGPUColorWriteMask_None, "value mismatch for ColorWriteMask::None");
    static_assert(static_cast<uint32_t>(ColorWriteMask::Red) == WGPUColorWriteMask_Red, "value mismatch for ColorWriteMask::Red");
    static_assert(static_cast<uint32_t>(ColorWriteMask::Green) == WGPUColorWriteMask_Green, "value mismatch for ColorWriteMask::Green");
    static_assert(static_cast<uint32_t>(ColorWriteMask::Blue) == WGPUColorWriteMask_Blue, "value mismatch for ColorWriteMask::Blue");
    static_assert(static_cast<uint32_t>(ColorWriteMask::Alpha) == WGPUColorWriteMask_Alpha, "value mismatch for ColorWriteMask::Alpha");
    static_assert(static_cast<uint32_t>(ColorWriteMask::All) == WGPUColorWriteMask_All, "value mismatch for ColorWriteMask::All");

    // MapMode

    static_assert(sizeof(MapMode) == sizeof(WGPUMapModeFlags), "sizeof mismatch for MapMode");
    static_assert(alignof(MapMode) == alignof(WGPUMapModeFlags), "alignof mismatch for MapMode");

    static_assert(static_cast<uint32_t>(MapMode::Read) == WGPUMapMode_Read, "value mismatch for MapMode::Read");
    static_assert(static_cast<uint32_t>(MapMode::Write) == WGPUMapMode_Write, "value mismatch for MapMode::Write");

    // ShaderStage

    static_assert(sizeof(ShaderStage) == sizeof(WGPUShaderStageFlags), "sizeof mismatch for ShaderStage");
    static_assert(alignof(ShaderStage) == alignof(WGPUShaderStageFlags), "alignof mismatch for ShaderStage");

    static_assert(static_cast<uint32_t>(ShaderStage::None) == WGPUShaderStage_None, "value mismatch for ShaderStage::None");
    static_assert(static_cast<uint32_t>(ShaderStage::Vertex) == WGPUShaderStage_Vertex, "value mismatch for ShaderStage::Vertex");
    static_assert(static_cast<uint32_t>(ShaderStage::Fragment) == WGPUShaderStage_Fragment, "value mismatch for ShaderStage::Fragment");
    static_assert(static_cast<uint32_t>(ShaderStage::Compute) == WGPUShaderStage_Compute, "value mismatch for ShaderStage::Compute");

    // TextureUsage

    static_assert(sizeof(TextureUsage) == sizeof(WGPUTextureUsageFlags), "sizeof mismatch for TextureUsage");
    static_assert(alignof(TextureUsage) == alignof(WGPUTextureUsageFlags), "alignof mismatch for TextureUsage");

    static_assert(static_cast<uint32_t>(TextureUsage::None) == WGPUTextureUsage_None, "value mismatch for TextureUsage::None");
    static_assert(static_cast<uint32_t>(TextureUsage::CopySrc) == WGPUTextureUsage_CopySrc, "value mismatch for TextureUsage::CopySrc");
    static_assert(static_cast<uint32_t>(TextureUsage::CopyDst) == WGPUTextureUsage_CopyDst, "value mismatch for TextureUsage::CopyDst");
    static_assert(static_cast<uint32_t>(TextureUsage::Sampled) == WGPUTextureUsage_Sampled, "value mismatch for TextureUsage::Sampled");
    static_assert(static_cast<uint32_t>(TextureUsage::Storage) == WGPUTextureUsage_Storage, "value mismatch for TextureUsage::Storage");
    static_assert(static_cast<uint32_t>(TextureUsage::OutputAttachment) == WGPUTextureUsage_OutputAttachment, "value mismatch for TextureUsage::OutputAttachment");

    // ChainedStruct

    static_assert(sizeof(ChainedStruct) == sizeof(WGPUChainedStruct),
            "sizeof mismatch for ChainedStruct");
    static_assert(alignof(ChainedStruct) == alignof(WGPUChainedStruct),
            "alignof mismatch for ChainedStruct");
    static_assert(offsetof(ChainedStruct, nextInChain) == offsetof(WGPUChainedStruct, next),
            "offsetof mismatch for ChainedStruct::nextInChain");
    static_assert(offsetof(ChainedStruct, sType) == offsetof(WGPUChainedStruct, sType),
            "offsetof mismatch for ChainedStruct::sType");

    // AdapterProperties

    static_assert(sizeof(AdapterProperties) == sizeof(WGPUAdapterProperties), "sizeof mismatch for AdapterProperties");
    static_assert(alignof(AdapterProperties) == alignof(WGPUAdapterProperties), "alignof mismatch for AdapterProperties");

    static_assert(offsetof(AdapterProperties, nextInChain) == offsetof(WGPUAdapterProperties, nextInChain),
            "offsetof mismatch for AdapterProperties::nextInChain");
    static_assert(offsetof(AdapterProperties, deviceID) == offsetof(WGPUAdapterProperties, deviceID),
            "offsetof mismatch for AdapterProperties::deviceID");
    static_assert(offsetof(AdapterProperties, vendorID) == offsetof(WGPUAdapterProperties, vendorID),
            "offsetof mismatch for AdapterProperties::vendorID");
    static_assert(offsetof(AdapterProperties, name) == offsetof(WGPUAdapterProperties, name),
            "offsetof mismatch for AdapterProperties::name");
    static_assert(offsetof(AdapterProperties, adapterType) == offsetof(WGPUAdapterProperties, adapterType),
            "offsetof mismatch for AdapterProperties::adapterType");
    static_assert(offsetof(AdapterProperties, backendType) == offsetof(WGPUAdapterProperties, backendType),
            "offsetof mismatch for AdapterProperties::backendType");

    // BindGroupEntry

    static_assert(sizeof(BindGroupEntry) == sizeof(WGPUBindGroupEntry), "sizeof mismatch for BindGroupEntry");
    static_assert(alignof(BindGroupEntry) == alignof(WGPUBindGroupEntry), "alignof mismatch for BindGroupEntry");

    static_assert(offsetof(BindGroupEntry, binding) == offsetof(WGPUBindGroupEntry, binding),
            "offsetof mismatch for BindGroupEntry::binding");
    static_assert(offsetof(BindGroupEntry, buffer) == offsetof(WGPUBindGroupEntry, buffer),
            "offsetof mismatch for BindGroupEntry::buffer");
    static_assert(offsetof(BindGroupEntry, offset) == offsetof(WGPUBindGroupEntry, offset),
            "offsetof mismatch for BindGroupEntry::offset");
    static_assert(offsetof(BindGroupEntry, size) == offsetof(WGPUBindGroupEntry, size),
            "offsetof mismatch for BindGroupEntry::size");
    static_assert(offsetof(BindGroupEntry, sampler) == offsetof(WGPUBindGroupEntry, sampler),
            "offsetof mismatch for BindGroupEntry::sampler");
    static_assert(offsetof(BindGroupEntry, textureView) == offsetof(WGPUBindGroupEntry, textureView),
            "offsetof mismatch for BindGroupEntry::textureView");

    // BindGroupLayoutEntry

    static_assert(sizeof(BindGroupLayoutEntry) == sizeof(WGPUBindGroupLayoutEntry), "sizeof mismatch for BindGroupLayoutEntry");
    static_assert(alignof(BindGroupLayoutEntry) == alignof(WGPUBindGroupLayoutEntry), "alignof mismatch for BindGroupLayoutEntry");

    static_assert(offsetof(BindGroupLayoutEntry, binding) == offsetof(WGPUBindGroupLayoutEntry, binding),
            "offsetof mismatch for BindGroupLayoutEntry::binding");
    static_assert(offsetof(BindGroupLayoutEntry, visibility) == offsetof(WGPUBindGroupLayoutEntry, visibility),
            "offsetof mismatch for BindGroupLayoutEntry::visibility");
    static_assert(offsetof(BindGroupLayoutEntry, type) == offsetof(WGPUBindGroupLayoutEntry, type),
            "offsetof mismatch for BindGroupLayoutEntry::type");
    static_assert(offsetof(BindGroupLayoutEntry, hasDynamicOffset) == offsetof(WGPUBindGroupLayoutEntry, hasDynamicOffset),
            "offsetof mismatch for BindGroupLayoutEntry::hasDynamicOffset");
    static_assert(offsetof(BindGroupLayoutEntry, minBufferBindingSize) == offsetof(WGPUBindGroupLayoutEntry, minBufferBindingSize),
            "offsetof mismatch for BindGroupLayoutEntry::minBufferBindingSize");
    static_assert(offsetof(BindGroupLayoutEntry, multisampled) == offsetof(WGPUBindGroupLayoutEntry, multisampled),
            "offsetof mismatch for BindGroupLayoutEntry::multisampled");
    static_assert(offsetof(BindGroupLayoutEntry, viewDimension) == offsetof(WGPUBindGroupLayoutEntry, viewDimension),
            "offsetof mismatch for BindGroupLayoutEntry::viewDimension");
    static_assert(offsetof(BindGroupLayoutEntry, textureComponentType) == offsetof(WGPUBindGroupLayoutEntry, textureComponentType),
            "offsetof mismatch for BindGroupLayoutEntry::textureComponentType");
    static_assert(offsetof(BindGroupLayoutEntry, storageTextureFormat) == offsetof(WGPUBindGroupLayoutEntry, storageTextureFormat),
            "offsetof mismatch for BindGroupLayoutEntry::storageTextureFormat");

    // BlendDescriptor

    static_assert(sizeof(BlendDescriptor) == sizeof(WGPUBlendDescriptor), "sizeof mismatch for BlendDescriptor");
    static_assert(alignof(BlendDescriptor) == alignof(WGPUBlendDescriptor), "alignof mismatch for BlendDescriptor");

    static_assert(offsetof(BlendDescriptor, operation) == offsetof(WGPUBlendDescriptor, operation),
            "offsetof mismatch for BlendDescriptor::operation");
    static_assert(offsetof(BlendDescriptor, srcFactor) == offsetof(WGPUBlendDescriptor, srcFactor),
            "offsetof mismatch for BlendDescriptor::srcFactor");
    static_assert(offsetof(BlendDescriptor, dstFactor) == offsetof(WGPUBlendDescriptor, dstFactor),
            "offsetof mismatch for BlendDescriptor::dstFactor");

    // BufferDescriptor

    static_assert(sizeof(BufferDescriptor) == sizeof(WGPUBufferDescriptor), "sizeof mismatch for BufferDescriptor");
    static_assert(alignof(BufferDescriptor) == alignof(WGPUBufferDescriptor), "alignof mismatch for BufferDescriptor");

    static_assert(offsetof(BufferDescriptor, nextInChain) == offsetof(WGPUBufferDescriptor, nextInChain),
            "offsetof mismatch for BufferDescriptor::nextInChain");
    static_assert(offsetof(BufferDescriptor, label) == offsetof(WGPUBufferDescriptor, label),
            "offsetof mismatch for BufferDescriptor::label");
    static_assert(offsetof(BufferDescriptor, usage) == offsetof(WGPUBufferDescriptor, usage),
            "offsetof mismatch for BufferDescriptor::usage");
    static_assert(offsetof(BufferDescriptor, size) == offsetof(WGPUBufferDescriptor, size),
            "offsetof mismatch for BufferDescriptor::size");
    static_assert(offsetof(BufferDescriptor, mappedAtCreation) == offsetof(WGPUBufferDescriptor, mappedAtCreation),
            "offsetof mismatch for BufferDescriptor::mappedAtCreation");

    // Color

    static_assert(sizeof(Color) == sizeof(WGPUColor), "sizeof mismatch for Color");
    static_assert(alignof(Color) == alignof(WGPUColor), "alignof mismatch for Color");

    static_assert(offsetof(Color, r) == offsetof(WGPUColor, r),
            "offsetof mismatch for Color::r");
    static_assert(offsetof(Color, g) == offsetof(WGPUColor, g),
            "offsetof mismatch for Color::g");
    static_assert(offsetof(Color, b) == offsetof(WGPUColor, b),
            "offsetof mismatch for Color::b");
    static_assert(offsetof(Color, a) == offsetof(WGPUColor, a),
            "offsetof mismatch for Color::a");

    // CommandBufferDescriptor

    static_assert(sizeof(CommandBufferDescriptor) == sizeof(WGPUCommandBufferDescriptor), "sizeof mismatch for CommandBufferDescriptor");
    static_assert(alignof(CommandBufferDescriptor) == alignof(WGPUCommandBufferDescriptor), "alignof mismatch for CommandBufferDescriptor");

    static_assert(offsetof(CommandBufferDescriptor, nextInChain) == offsetof(WGPUCommandBufferDescriptor, nextInChain),
            "offsetof mismatch for CommandBufferDescriptor::nextInChain");
    static_assert(offsetof(CommandBufferDescriptor, label) == offsetof(WGPUCommandBufferDescriptor, label),
            "offsetof mismatch for CommandBufferDescriptor::label");

    // CommandEncoderDescriptor

    static_assert(sizeof(CommandEncoderDescriptor) == sizeof(WGPUCommandEncoderDescriptor), "sizeof mismatch for CommandEncoderDescriptor");
    static_assert(alignof(CommandEncoderDescriptor) == alignof(WGPUCommandEncoderDescriptor), "alignof mismatch for CommandEncoderDescriptor");

    static_assert(offsetof(CommandEncoderDescriptor, nextInChain) == offsetof(WGPUCommandEncoderDescriptor, nextInChain),
            "offsetof mismatch for CommandEncoderDescriptor::nextInChain");
    static_assert(offsetof(CommandEncoderDescriptor, label) == offsetof(WGPUCommandEncoderDescriptor, label),
            "offsetof mismatch for CommandEncoderDescriptor::label");

    // ComputePassDescriptor

    static_assert(sizeof(ComputePassDescriptor) == sizeof(WGPUComputePassDescriptor), "sizeof mismatch for ComputePassDescriptor");
    static_assert(alignof(ComputePassDescriptor) == alignof(WGPUComputePassDescriptor), "alignof mismatch for ComputePassDescriptor");

    static_assert(offsetof(ComputePassDescriptor, nextInChain) == offsetof(WGPUComputePassDescriptor, nextInChain),
            "offsetof mismatch for ComputePassDescriptor::nextInChain");
    static_assert(offsetof(ComputePassDescriptor, label) == offsetof(WGPUComputePassDescriptor, label),
            "offsetof mismatch for ComputePassDescriptor::label");

    // Extent3D

    static_assert(sizeof(Extent3D) == sizeof(WGPUExtent3D), "sizeof mismatch for Extent3D");
    static_assert(alignof(Extent3D) == alignof(WGPUExtent3D), "alignof mismatch for Extent3D");

    static_assert(offsetof(Extent3D, width) == offsetof(WGPUExtent3D, width),
            "offsetof mismatch for Extent3D::width");
    static_assert(offsetof(Extent3D, height) == offsetof(WGPUExtent3D, height),
            "offsetof mismatch for Extent3D::height");
    static_assert(offsetof(Extent3D, depth) == offsetof(WGPUExtent3D, depth),
            "offsetof mismatch for Extent3D::depth");

    // FenceDescriptor

    static_assert(sizeof(FenceDescriptor) == sizeof(WGPUFenceDescriptor), "sizeof mismatch for FenceDescriptor");
    static_assert(alignof(FenceDescriptor) == alignof(WGPUFenceDescriptor), "alignof mismatch for FenceDescriptor");

    static_assert(offsetof(FenceDescriptor, nextInChain) == offsetof(WGPUFenceDescriptor, nextInChain),
            "offsetof mismatch for FenceDescriptor::nextInChain");
    static_assert(offsetof(FenceDescriptor, label) == offsetof(WGPUFenceDescriptor, label),
            "offsetof mismatch for FenceDescriptor::label");
    static_assert(offsetof(FenceDescriptor, initialValue) == offsetof(WGPUFenceDescriptor, initialValue),
            "offsetof mismatch for FenceDescriptor::initialValue");

    // InstanceDescriptor

    static_assert(sizeof(InstanceDescriptor) == sizeof(WGPUInstanceDescriptor), "sizeof mismatch for InstanceDescriptor");
    static_assert(alignof(InstanceDescriptor) == alignof(WGPUInstanceDescriptor), "alignof mismatch for InstanceDescriptor");

    static_assert(offsetof(InstanceDescriptor, nextInChain) == offsetof(WGPUInstanceDescriptor, nextInChain),
            "offsetof mismatch for InstanceDescriptor::nextInChain");

    // Origin3D

    static_assert(sizeof(Origin3D) == sizeof(WGPUOrigin3D), "sizeof mismatch for Origin3D");
    static_assert(alignof(Origin3D) == alignof(WGPUOrigin3D), "alignof mismatch for Origin3D");

    static_assert(offsetof(Origin3D, x) == offsetof(WGPUOrigin3D, x),
            "offsetof mismatch for Origin3D::x");
    static_assert(offsetof(Origin3D, y) == offsetof(WGPUOrigin3D, y),
            "offsetof mismatch for Origin3D::y");
    static_assert(offsetof(Origin3D, z) == offsetof(WGPUOrigin3D, z),
            "offsetof mismatch for Origin3D::z");

    // PipelineLayoutDescriptor

    static_assert(sizeof(PipelineLayoutDescriptor) == sizeof(WGPUPipelineLayoutDescriptor), "sizeof mismatch for PipelineLayoutDescriptor");
    static_assert(alignof(PipelineLayoutDescriptor) == alignof(WGPUPipelineLayoutDescriptor), "alignof mismatch for PipelineLayoutDescriptor");

    static_assert(offsetof(PipelineLayoutDescriptor, nextInChain) == offsetof(WGPUPipelineLayoutDescriptor, nextInChain),
            "offsetof mismatch for PipelineLayoutDescriptor::nextInChain");
    static_assert(offsetof(PipelineLayoutDescriptor, label) == offsetof(WGPUPipelineLayoutDescriptor, label),
            "offsetof mismatch for PipelineLayoutDescriptor::label");
    static_assert(offsetof(PipelineLayoutDescriptor, bindGroupLayoutCount) == offsetof(WGPUPipelineLayoutDescriptor, bindGroupLayoutCount),
            "offsetof mismatch for PipelineLayoutDescriptor::bindGroupLayoutCount");
    static_assert(offsetof(PipelineLayoutDescriptor, bindGroupLayouts) == offsetof(WGPUPipelineLayoutDescriptor, bindGroupLayouts),
            "offsetof mismatch for PipelineLayoutDescriptor::bindGroupLayouts");

    // ProgrammableStageDescriptor

    static_assert(sizeof(ProgrammableStageDescriptor) == sizeof(WGPUProgrammableStageDescriptor), "sizeof mismatch for ProgrammableStageDescriptor");
    static_assert(alignof(ProgrammableStageDescriptor) == alignof(WGPUProgrammableStageDescriptor), "alignof mismatch for ProgrammableStageDescriptor");

    static_assert(offsetof(ProgrammableStageDescriptor, nextInChain) == offsetof(WGPUProgrammableStageDescriptor, nextInChain),
            "offsetof mismatch for ProgrammableStageDescriptor::nextInChain");
    static_assert(offsetof(ProgrammableStageDescriptor, module) == offsetof(WGPUProgrammableStageDescriptor, module),
            "offsetof mismatch for ProgrammableStageDescriptor::module");
    static_assert(offsetof(ProgrammableStageDescriptor, entryPoint) == offsetof(WGPUProgrammableStageDescriptor, entryPoint),
            "offsetof mismatch for ProgrammableStageDescriptor::entryPoint");

    // QuerySetDescriptor

    static_assert(sizeof(QuerySetDescriptor) == sizeof(WGPUQuerySetDescriptor), "sizeof mismatch for QuerySetDescriptor");
    static_assert(alignof(QuerySetDescriptor) == alignof(WGPUQuerySetDescriptor), "alignof mismatch for QuerySetDescriptor");

    static_assert(offsetof(QuerySetDescriptor, nextInChain) == offsetof(WGPUQuerySetDescriptor, nextInChain),
            "offsetof mismatch for QuerySetDescriptor::nextInChain");
    static_assert(offsetof(QuerySetDescriptor, label) == offsetof(WGPUQuerySetDescriptor, label),
            "offsetof mismatch for QuerySetDescriptor::label");
    static_assert(offsetof(QuerySetDescriptor, type) == offsetof(WGPUQuerySetDescriptor, type),
            "offsetof mismatch for QuerySetDescriptor::type");
    static_assert(offsetof(QuerySetDescriptor, count) == offsetof(WGPUQuerySetDescriptor, count),
            "offsetof mismatch for QuerySetDescriptor::count");
    static_assert(offsetof(QuerySetDescriptor, pipelineStatistics) == offsetof(WGPUQuerySetDescriptor, pipelineStatistics),
            "offsetof mismatch for QuerySetDescriptor::pipelineStatistics");
    static_assert(offsetof(QuerySetDescriptor, pipelineStatisticsCount) == offsetof(WGPUQuerySetDescriptor, pipelineStatisticsCount),
            "offsetof mismatch for QuerySetDescriptor::pipelineStatisticsCount");

    // RasterizationStateDescriptor

    static_assert(sizeof(RasterizationStateDescriptor) == sizeof(WGPURasterizationStateDescriptor), "sizeof mismatch for RasterizationStateDescriptor");
    static_assert(alignof(RasterizationStateDescriptor) == alignof(WGPURasterizationStateDescriptor), "alignof mismatch for RasterizationStateDescriptor");

    static_assert(offsetof(RasterizationStateDescriptor, nextInChain) == offsetof(WGPURasterizationStateDescriptor, nextInChain),
            "offsetof mismatch for RasterizationStateDescriptor::nextInChain");
    static_assert(offsetof(RasterizationStateDescriptor, frontFace) == offsetof(WGPURasterizationStateDescriptor, frontFace),
            "offsetof mismatch for RasterizationStateDescriptor::frontFace");
    static_assert(offsetof(RasterizationStateDescriptor, cullMode) == offsetof(WGPURasterizationStateDescriptor, cullMode),
            "offsetof mismatch for RasterizationStateDescriptor::cullMode");
    static_assert(offsetof(RasterizationStateDescriptor, depthBias) == offsetof(WGPURasterizationStateDescriptor, depthBias),
            "offsetof mismatch for RasterizationStateDescriptor::depthBias");
    static_assert(offsetof(RasterizationStateDescriptor, depthBiasSlopeScale) == offsetof(WGPURasterizationStateDescriptor, depthBiasSlopeScale),
            "offsetof mismatch for RasterizationStateDescriptor::depthBiasSlopeScale");
    static_assert(offsetof(RasterizationStateDescriptor, depthBiasClamp) == offsetof(WGPURasterizationStateDescriptor, depthBiasClamp),
            "offsetof mismatch for RasterizationStateDescriptor::depthBiasClamp");

    // RenderBundleDescriptor

    static_assert(sizeof(RenderBundleDescriptor) == sizeof(WGPURenderBundleDescriptor), "sizeof mismatch for RenderBundleDescriptor");
    static_assert(alignof(RenderBundleDescriptor) == alignof(WGPURenderBundleDescriptor), "alignof mismatch for RenderBundleDescriptor");

    static_assert(offsetof(RenderBundleDescriptor, nextInChain) == offsetof(WGPURenderBundleDescriptor, nextInChain),
            "offsetof mismatch for RenderBundleDescriptor::nextInChain");
    static_assert(offsetof(RenderBundleDescriptor, label) == offsetof(WGPURenderBundleDescriptor, label),
            "offsetof mismatch for RenderBundleDescriptor::label");

    // RenderBundleEncoderDescriptor

    static_assert(sizeof(RenderBundleEncoderDescriptor) == sizeof(WGPURenderBundleEncoderDescriptor), "sizeof mismatch for RenderBundleEncoderDescriptor");
    static_assert(alignof(RenderBundleEncoderDescriptor) == alignof(WGPURenderBundleEncoderDescriptor), "alignof mismatch for RenderBundleEncoderDescriptor");

    static_assert(offsetof(RenderBundleEncoderDescriptor, nextInChain) == offsetof(WGPURenderBundleEncoderDescriptor, nextInChain),
            "offsetof mismatch for RenderBundleEncoderDescriptor::nextInChain");
    static_assert(offsetof(RenderBundleEncoderDescriptor, label) == offsetof(WGPURenderBundleEncoderDescriptor, label),
            "offsetof mismatch for RenderBundleEncoderDescriptor::label");
    static_assert(offsetof(RenderBundleEncoderDescriptor, colorFormatsCount) == offsetof(WGPURenderBundleEncoderDescriptor, colorFormatsCount),
            "offsetof mismatch for RenderBundleEncoderDescriptor::colorFormatsCount");
    static_assert(offsetof(RenderBundleEncoderDescriptor, colorFormats) == offsetof(WGPURenderBundleEncoderDescriptor, colorFormats),
            "offsetof mismatch for RenderBundleEncoderDescriptor::colorFormats");
    static_assert(offsetof(RenderBundleEncoderDescriptor, depthStencilFormat) == offsetof(WGPURenderBundleEncoderDescriptor, depthStencilFormat),
            "offsetof mismatch for RenderBundleEncoderDescriptor::depthStencilFormat");
    static_assert(offsetof(RenderBundleEncoderDescriptor, sampleCount) == offsetof(WGPURenderBundleEncoderDescriptor, sampleCount),
            "offsetof mismatch for RenderBundleEncoderDescriptor::sampleCount");

    // RenderPassDepthStencilAttachmentDescriptor

    static_assert(sizeof(RenderPassDepthStencilAttachmentDescriptor) == sizeof(WGPURenderPassDepthStencilAttachmentDescriptor), "sizeof mismatch for RenderPassDepthStencilAttachmentDescriptor");
    static_assert(alignof(RenderPassDepthStencilAttachmentDescriptor) == alignof(WGPURenderPassDepthStencilAttachmentDescriptor), "alignof mismatch for RenderPassDepthStencilAttachmentDescriptor");

    static_assert(offsetof(RenderPassDepthStencilAttachmentDescriptor, attachment) == offsetof(WGPURenderPassDepthStencilAttachmentDescriptor, attachment),
            "offsetof mismatch for RenderPassDepthStencilAttachmentDescriptor::attachment");
    static_assert(offsetof(RenderPassDepthStencilAttachmentDescriptor, depthLoadOp) == offsetof(WGPURenderPassDepthStencilAttachmentDescriptor, depthLoadOp),
            "offsetof mismatch for RenderPassDepthStencilAttachmentDescriptor::depthLoadOp");
    static_assert(offsetof(RenderPassDepthStencilAttachmentDescriptor, depthStoreOp) == offsetof(WGPURenderPassDepthStencilAttachmentDescriptor, depthStoreOp),
            "offsetof mismatch for RenderPassDepthStencilAttachmentDescriptor::depthStoreOp");
    static_assert(offsetof(RenderPassDepthStencilAttachmentDescriptor, clearDepth) == offsetof(WGPURenderPassDepthStencilAttachmentDescriptor, clearDepth),
            "offsetof mismatch for RenderPassDepthStencilAttachmentDescriptor::clearDepth");
    static_assert(offsetof(RenderPassDepthStencilAttachmentDescriptor, depthReadOnly) == offsetof(WGPURenderPassDepthStencilAttachmentDescriptor, depthReadOnly),
            "offsetof mismatch for RenderPassDepthStencilAttachmentDescriptor::depthReadOnly");
    static_assert(offsetof(RenderPassDepthStencilAttachmentDescriptor, stencilLoadOp) == offsetof(WGPURenderPassDepthStencilAttachmentDescriptor, stencilLoadOp),
            "offsetof mismatch for RenderPassDepthStencilAttachmentDescriptor::stencilLoadOp");
    static_assert(offsetof(RenderPassDepthStencilAttachmentDescriptor, stencilStoreOp) == offsetof(WGPURenderPassDepthStencilAttachmentDescriptor, stencilStoreOp),
            "offsetof mismatch for RenderPassDepthStencilAttachmentDescriptor::stencilStoreOp");
    static_assert(offsetof(RenderPassDepthStencilAttachmentDescriptor, clearStencil) == offsetof(WGPURenderPassDepthStencilAttachmentDescriptor, clearStencil),
            "offsetof mismatch for RenderPassDepthStencilAttachmentDescriptor::clearStencil");
    static_assert(offsetof(RenderPassDepthStencilAttachmentDescriptor, stencilReadOnly) == offsetof(WGPURenderPassDepthStencilAttachmentDescriptor, stencilReadOnly),
            "offsetof mismatch for RenderPassDepthStencilAttachmentDescriptor::stencilReadOnly");

    // SamplerDescriptor

    static_assert(sizeof(SamplerDescriptor) == sizeof(WGPUSamplerDescriptor), "sizeof mismatch for SamplerDescriptor");
    static_assert(alignof(SamplerDescriptor) == alignof(WGPUSamplerDescriptor), "alignof mismatch for SamplerDescriptor");

    static_assert(offsetof(SamplerDescriptor, nextInChain) == offsetof(WGPUSamplerDescriptor, nextInChain),
            "offsetof mismatch for SamplerDescriptor::nextInChain");
    static_assert(offsetof(SamplerDescriptor, label) == offsetof(WGPUSamplerDescriptor, label),
            "offsetof mismatch for SamplerDescriptor::label");
    static_assert(offsetof(SamplerDescriptor, addressModeU) == offsetof(WGPUSamplerDescriptor, addressModeU),
            "offsetof mismatch for SamplerDescriptor::addressModeU");
    static_assert(offsetof(SamplerDescriptor, addressModeV) == offsetof(WGPUSamplerDescriptor, addressModeV),
            "offsetof mismatch for SamplerDescriptor::addressModeV");
    static_assert(offsetof(SamplerDescriptor, addressModeW) == offsetof(WGPUSamplerDescriptor, addressModeW),
            "offsetof mismatch for SamplerDescriptor::addressModeW");
    static_assert(offsetof(SamplerDescriptor, magFilter) == offsetof(WGPUSamplerDescriptor, magFilter),
            "offsetof mismatch for SamplerDescriptor::magFilter");
    static_assert(offsetof(SamplerDescriptor, minFilter) == offsetof(WGPUSamplerDescriptor, minFilter),
            "offsetof mismatch for SamplerDescriptor::minFilter");
    static_assert(offsetof(SamplerDescriptor, mipmapFilter) == offsetof(WGPUSamplerDescriptor, mipmapFilter),
            "offsetof mismatch for SamplerDescriptor::mipmapFilter");
    static_assert(offsetof(SamplerDescriptor, lodMinClamp) == offsetof(WGPUSamplerDescriptor, lodMinClamp),
            "offsetof mismatch for SamplerDescriptor::lodMinClamp");
    static_assert(offsetof(SamplerDescriptor, lodMaxClamp) == offsetof(WGPUSamplerDescriptor, lodMaxClamp),
            "offsetof mismatch for SamplerDescriptor::lodMaxClamp");
    static_assert(offsetof(SamplerDescriptor, compare) == offsetof(WGPUSamplerDescriptor, compare),
            "offsetof mismatch for SamplerDescriptor::compare");

    // ShaderModuleDescriptor

    static_assert(sizeof(ShaderModuleDescriptor) == sizeof(WGPUShaderModuleDescriptor), "sizeof mismatch for ShaderModuleDescriptor");
    static_assert(alignof(ShaderModuleDescriptor) == alignof(WGPUShaderModuleDescriptor), "alignof mismatch for ShaderModuleDescriptor");

    static_assert(offsetof(ShaderModuleDescriptor, nextInChain) == offsetof(WGPUShaderModuleDescriptor, nextInChain),
            "offsetof mismatch for ShaderModuleDescriptor::nextInChain");
    static_assert(offsetof(ShaderModuleDescriptor, label) == offsetof(WGPUShaderModuleDescriptor, label),
            "offsetof mismatch for ShaderModuleDescriptor::label");

    // ShaderModuleSPIRVDescriptor

    static_assert(sizeof(ShaderModuleSPIRVDescriptor) == sizeof(WGPUShaderModuleSPIRVDescriptor), "sizeof mismatch for ShaderModuleSPIRVDescriptor");
    static_assert(alignof(ShaderModuleSPIRVDescriptor) == alignof(WGPUShaderModuleSPIRVDescriptor), "alignof mismatch for ShaderModuleSPIRVDescriptor");

    static_assert(offsetof(ShaderModuleSPIRVDescriptor, codeSize) == offsetof(WGPUShaderModuleSPIRVDescriptor, codeSize),
            "offsetof mismatch for ShaderModuleSPIRVDescriptor::codeSize");
    static_assert(offsetof(ShaderModuleSPIRVDescriptor, code) == offsetof(WGPUShaderModuleSPIRVDescriptor, code),
            "offsetof mismatch for ShaderModuleSPIRVDescriptor::code");

    // ShaderModuleWGSLDescriptor

    static_assert(sizeof(ShaderModuleWGSLDescriptor) == sizeof(WGPUShaderModuleWGSLDescriptor), "sizeof mismatch for ShaderModuleWGSLDescriptor");
    static_assert(alignof(ShaderModuleWGSLDescriptor) == alignof(WGPUShaderModuleWGSLDescriptor), "alignof mismatch for ShaderModuleWGSLDescriptor");

    static_assert(offsetof(ShaderModuleWGSLDescriptor, source) == offsetof(WGPUShaderModuleWGSLDescriptor, source),
            "offsetof mismatch for ShaderModuleWGSLDescriptor::source");

    // StencilStateFaceDescriptor

    static_assert(sizeof(StencilStateFaceDescriptor) == sizeof(WGPUStencilStateFaceDescriptor), "sizeof mismatch for StencilStateFaceDescriptor");
    static_assert(alignof(StencilStateFaceDescriptor) == alignof(WGPUStencilStateFaceDescriptor), "alignof mismatch for StencilStateFaceDescriptor");

    static_assert(offsetof(StencilStateFaceDescriptor, compare) == offsetof(WGPUStencilStateFaceDescriptor, compare),
            "offsetof mismatch for StencilStateFaceDescriptor::compare");
    static_assert(offsetof(StencilStateFaceDescriptor, failOp) == offsetof(WGPUStencilStateFaceDescriptor, failOp),
            "offsetof mismatch for StencilStateFaceDescriptor::failOp");
    static_assert(offsetof(StencilStateFaceDescriptor, depthFailOp) == offsetof(WGPUStencilStateFaceDescriptor, depthFailOp),
            "offsetof mismatch for StencilStateFaceDescriptor::depthFailOp");
    static_assert(offsetof(StencilStateFaceDescriptor, passOp) == offsetof(WGPUStencilStateFaceDescriptor, passOp),
            "offsetof mismatch for StencilStateFaceDescriptor::passOp");

    // SurfaceDescriptor

    static_assert(sizeof(SurfaceDescriptor) == sizeof(WGPUSurfaceDescriptor), "sizeof mismatch for SurfaceDescriptor");
    static_assert(alignof(SurfaceDescriptor) == alignof(WGPUSurfaceDescriptor), "alignof mismatch for SurfaceDescriptor");

    static_assert(offsetof(SurfaceDescriptor, nextInChain) == offsetof(WGPUSurfaceDescriptor, nextInChain),
            "offsetof mismatch for SurfaceDescriptor::nextInChain");
    static_assert(offsetof(SurfaceDescriptor, label) == offsetof(WGPUSurfaceDescriptor, label),
            "offsetof mismatch for SurfaceDescriptor::label");

    // SurfaceDescriptorFromCanvasHTMLSelector

    static_assert(sizeof(SurfaceDescriptorFromCanvasHTMLSelector) == sizeof(WGPUSurfaceDescriptorFromCanvasHTMLSelector), "sizeof mismatch for SurfaceDescriptorFromCanvasHTMLSelector");
    static_assert(alignof(SurfaceDescriptorFromCanvasHTMLSelector) == alignof(WGPUSurfaceDescriptorFromCanvasHTMLSelector), "alignof mismatch for SurfaceDescriptorFromCanvasHTMLSelector");

    static_assert(offsetof(SurfaceDescriptorFromCanvasHTMLSelector, selector) == offsetof(WGPUSurfaceDescriptorFromCanvasHTMLSelector, selector),
            "offsetof mismatch for SurfaceDescriptorFromCanvasHTMLSelector::selector");

    // SurfaceDescriptorFromMetalLayer

    static_assert(sizeof(SurfaceDescriptorFromMetalLayer) == sizeof(WGPUSurfaceDescriptorFromMetalLayer), "sizeof mismatch for SurfaceDescriptorFromMetalLayer");
    static_assert(alignof(SurfaceDescriptorFromMetalLayer) == alignof(WGPUSurfaceDescriptorFromMetalLayer), "alignof mismatch for SurfaceDescriptorFromMetalLayer");

    static_assert(offsetof(SurfaceDescriptorFromMetalLayer, layer) == offsetof(WGPUSurfaceDescriptorFromMetalLayer, layer),
            "offsetof mismatch for SurfaceDescriptorFromMetalLayer::layer");

    // SurfaceDescriptorFromWindowsHWND

    static_assert(sizeof(SurfaceDescriptorFromWindowsHWND) == sizeof(WGPUSurfaceDescriptorFromWindowsHWND), "sizeof mismatch for SurfaceDescriptorFromWindowsHWND");
    static_assert(alignof(SurfaceDescriptorFromWindowsHWND) == alignof(WGPUSurfaceDescriptorFromWindowsHWND), "alignof mismatch for SurfaceDescriptorFromWindowsHWND");

    static_assert(offsetof(SurfaceDescriptorFromWindowsHWND, hinstance) == offsetof(WGPUSurfaceDescriptorFromWindowsHWND, hinstance),
            "offsetof mismatch for SurfaceDescriptorFromWindowsHWND::hinstance");
    static_assert(offsetof(SurfaceDescriptorFromWindowsHWND, hwnd) == offsetof(WGPUSurfaceDescriptorFromWindowsHWND, hwnd),
            "offsetof mismatch for SurfaceDescriptorFromWindowsHWND::hwnd");

    // SurfaceDescriptorFromXlib

    static_assert(sizeof(SurfaceDescriptorFromXlib) == sizeof(WGPUSurfaceDescriptorFromXlib), "sizeof mismatch for SurfaceDescriptorFromXlib");
    static_assert(alignof(SurfaceDescriptorFromXlib) == alignof(WGPUSurfaceDescriptorFromXlib), "alignof mismatch for SurfaceDescriptorFromXlib");

    static_assert(offsetof(SurfaceDescriptorFromXlib, display) == offsetof(WGPUSurfaceDescriptorFromXlib, display),
            "offsetof mismatch for SurfaceDescriptorFromXlib::display");
    static_assert(offsetof(SurfaceDescriptorFromXlib, window) == offsetof(WGPUSurfaceDescriptorFromXlib, window),
            "offsetof mismatch for SurfaceDescriptorFromXlib::window");

    // SwapChainDescriptor

    static_assert(sizeof(SwapChainDescriptor) == sizeof(WGPUSwapChainDescriptor), "sizeof mismatch for SwapChainDescriptor");
    static_assert(alignof(SwapChainDescriptor) == alignof(WGPUSwapChainDescriptor), "alignof mismatch for SwapChainDescriptor");

    static_assert(offsetof(SwapChainDescriptor, nextInChain) == offsetof(WGPUSwapChainDescriptor, nextInChain),
            "offsetof mismatch for SwapChainDescriptor::nextInChain");
    static_assert(offsetof(SwapChainDescriptor, label) == offsetof(WGPUSwapChainDescriptor, label),
            "offsetof mismatch for SwapChainDescriptor::label");
    static_assert(offsetof(SwapChainDescriptor, usage) == offsetof(WGPUSwapChainDescriptor, usage),
            "offsetof mismatch for SwapChainDescriptor::usage");
    static_assert(offsetof(SwapChainDescriptor, format) == offsetof(WGPUSwapChainDescriptor, format),
            "offsetof mismatch for SwapChainDescriptor::format");
    static_assert(offsetof(SwapChainDescriptor, width) == offsetof(WGPUSwapChainDescriptor, width),
            "offsetof mismatch for SwapChainDescriptor::width");
    static_assert(offsetof(SwapChainDescriptor, height) == offsetof(WGPUSwapChainDescriptor, height),
            "offsetof mismatch for SwapChainDescriptor::height");
    static_assert(offsetof(SwapChainDescriptor, presentMode) == offsetof(WGPUSwapChainDescriptor, presentMode),
            "offsetof mismatch for SwapChainDescriptor::presentMode");

    // TextureDataLayout

    static_assert(sizeof(TextureDataLayout) == sizeof(WGPUTextureDataLayout), "sizeof mismatch for TextureDataLayout");
    static_assert(alignof(TextureDataLayout) == alignof(WGPUTextureDataLayout), "alignof mismatch for TextureDataLayout");

    static_assert(offsetof(TextureDataLayout, nextInChain) == offsetof(WGPUTextureDataLayout, nextInChain),
            "offsetof mismatch for TextureDataLayout::nextInChain");
    static_assert(offsetof(TextureDataLayout, offset) == offsetof(WGPUTextureDataLayout, offset),
            "offsetof mismatch for TextureDataLayout::offset");
    static_assert(offsetof(TextureDataLayout, bytesPerRow) == offsetof(WGPUTextureDataLayout, bytesPerRow),
            "offsetof mismatch for TextureDataLayout::bytesPerRow");
    static_assert(offsetof(TextureDataLayout, rowsPerImage) == offsetof(WGPUTextureDataLayout, rowsPerImage),
            "offsetof mismatch for TextureDataLayout::rowsPerImage");

    // TextureViewDescriptor

    static_assert(sizeof(TextureViewDescriptor) == sizeof(WGPUTextureViewDescriptor), "sizeof mismatch for TextureViewDescriptor");
    static_assert(alignof(TextureViewDescriptor) == alignof(WGPUTextureViewDescriptor), "alignof mismatch for TextureViewDescriptor");

    static_assert(offsetof(TextureViewDescriptor, nextInChain) == offsetof(WGPUTextureViewDescriptor, nextInChain),
            "offsetof mismatch for TextureViewDescriptor::nextInChain");
    static_assert(offsetof(TextureViewDescriptor, label) == offsetof(WGPUTextureViewDescriptor, label),
            "offsetof mismatch for TextureViewDescriptor::label");
    static_assert(offsetof(TextureViewDescriptor, format) == offsetof(WGPUTextureViewDescriptor, format),
            "offsetof mismatch for TextureViewDescriptor::format");
    static_assert(offsetof(TextureViewDescriptor, dimension) == offsetof(WGPUTextureViewDescriptor, dimension),
            "offsetof mismatch for TextureViewDescriptor::dimension");
    static_assert(offsetof(TextureViewDescriptor, baseMipLevel) == offsetof(WGPUTextureViewDescriptor, baseMipLevel),
            "offsetof mismatch for TextureViewDescriptor::baseMipLevel");
    static_assert(offsetof(TextureViewDescriptor, mipLevelCount) == offsetof(WGPUTextureViewDescriptor, mipLevelCount),
            "offsetof mismatch for TextureViewDescriptor::mipLevelCount");
    static_assert(offsetof(TextureViewDescriptor, baseArrayLayer) == offsetof(WGPUTextureViewDescriptor, baseArrayLayer),
            "offsetof mismatch for TextureViewDescriptor::baseArrayLayer");
    static_assert(offsetof(TextureViewDescriptor, aspect) == offsetof(WGPUTextureViewDescriptor, aspect),
            "offsetof mismatch for TextureViewDescriptor::aspect");

    // VertexAttributeDescriptor

    static_assert(sizeof(VertexAttributeDescriptor) == sizeof(WGPUVertexAttributeDescriptor), "sizeof mismatch for VertexAttributeDescriptor");
    static_assert(alignof(VertexAttributeDescriptor) == alignof(WGPUVertexAttributeDescriptor), "alignof mismatch for VertexAttributeDescriptor");

    static_assert(offsetof(VertexAttributeDescriptor, format) == offsetof(WGPUVertexAttributeDescriptor, format),
            "offsetof mismatch for VertexAttributeDescriptor::format");
    static_assert(offsetof(VertexAttributeDescriptor, offset) == offsetof(WGPUVertexAttributeDescriptor, offset),
            "offsetof mismatch for VertexAttributeDescriptor::offset");
    static_assert(offsetof(VertexAttributeDescriptor, shaderLocation) == offsetof(WGPUVertexAttributeDescriptor, shaderLocation),
            "offsetof mismatch for VertexAttributeDescriptor::shaderLocation");

    // BindGroupDescriptor

    static_assert(sizeof(BindGroupDescriptor) == sizeof(WGPUBindGroupDescriptor), "sizeof mismatch for BindGroupDescriptor");
    static_assert(alignof(BindGroupDescriptor) == alignof(WGPUBindGroupDescriptor), "alignof mismatch for BindGroupDescriptor");

    static_assert(offsetof(BindGroupDescriptor, nextInChain) == offsetof(WGPUBindGroupDescriptor, nextInChain),
            "offsetof mismatch for BindGroupDescriptor::nextInChain");
    static_assert(offsetof(BindGroupDescriptor, label) == offsetof(WGPUBindGroupDescriptor, label),
            "offsetof mismatch for BindGroupDescriptor::label");
    static_assert(offsetof(BindGroupDescriptor, layout) == offsetof(WGPUBindGroupDescriptor, layout),
            "offsetof mismatch for BindGroupDescriptor::layout");
    static_assert(offsetof(BindGroupDescriptor, entryCount) == offsetof(WGPUBindGroupDescriptor, entryCount),
            "offsetof mismatch for BindGroupDescriptor::entryCount");
    static_assert(offsetof(BindGroupDescriptor, entries) == offsetof(WGPUBindGroupDescriptor, entries),
            "offsetof mismatch for BindGroupDescriptor::entries");

    // BindGroupLayoutDescriptor

    static_assert(sizeof(BindGroupLayoutDescriptor) == sizeof(WGPUBindGroupLayoutDescriptor), "sizeof mismatch for BindGroupLayoutDescriptor");
    static_assert(alignof(BindGroupLayoutDescriptor) == alignof(WGPUBindGroupLayoutDescriptor), "alignof mismatch for BindGroupLayoutDescriptor");

    static_assert(offsetof(BindGroupLayoutDescriptor, nextInChain) == offsetof(WGPUBindGroupLayoutDescriptor, nextInChain),
            "offsetof mismatch for BindGroupLayoutDescriptor::nextInChain");
    static_assert(offsetof(BindGroupLayoutDescriptor, label) == offsetof(WGPUBindGroupLayoutDescriptor, label),
            "offsetof mismatch for BindGroupLayoutDescriptor::label");
    static_assert(offsetof(BindGroupLayoutDescriptor, entryCount) == offsetof(WGPUBindGroupLayoutDescriptor, entryCount),
            "offsetof mismatch for BindGroupLayoutDescriptor::entryCount");
    static_assert(offsetof(BindGroupLayoutDescriptor, entries) == offsetof(WGPUBindGroupLayoutDescriptor, entries),
            "offsetof mismatch for BindGroupLayoutDescriptor::entries");

    // BufferCopyView

    static_assert(sizeof(BufferCopyView) == sizeof(WGPUBufferCopyView), "sizeof mismatch for BufferCopyView");
    static_assert(alignof(BufferCopyView) == alignof(WGPUBufferCopyView), "alignof mismatch for BufferCopyView");

    static_assert(offsetof(BufferCopyView, nextInChain) == offsetof(WGPUBufferCopyView, nextInChain),
            "offsetof mismatch for BufferCopyView::nextInChain");
    static_assert(offsetof(BufferCopyView, layout) == offsetof(WGPUBufferCopyView, layout),
            "offsetof mismatch for BufferCopyView::layout");
    static_assert(offsetof(BufferCopyView, buffer) == offsetof(WGPUBufferCopyView, buffer),
            "offsetof mismatch for BufferCopyView::buffer");

    // ColorStateDescriptor

    static_assert(sizeof(ColorStateDescriptor) == sizeof(WGPUColorStateDescriptor), "sizeof mismatch for ColorStateDescriptor");
    static_assert(alignof(ColorStateDescriptor) == alignof(WGPUColorStateDescriptor), "alignof mismatch for ColorStateDescriptor");

    static_assert(offsetof(ColorStateDescriptor, nextInChain) == offsetof(WGPUColorStateDescriptor, nextInChain),
            "offsetof mismatch for ColorStateDescriptor::nextInChain");
    static_assert(offsetof(ColorStateDescriptor, format) == offsetof(WGPUColorStateDescriptor, format),
            "offsetof mismatch for ColorStateDescriptor::format");
    static_assert(offsetof(ColorStateDescriptor, alphaBlend) == offsetof(WGPUColorStateDescriptor, alphaBlend),
            "offsetof mismatch for ColorStateDescriptor::alphaBlend");
    static_assert(offsetof(ColorStateDescriptor, colorBlend) == offsetof(WGPUColorStateDescriptor, colorBlend),
            "offsetof mismatch for ColorStateDescriptor::colorBlend");
    static_assert(offsetof(ColorStateDescriptor, writeMask) == offsetof(WGPUColorStateDescriptor, writeMask),
            "offsetof mismatch for ColorStateDescriptor::writeMask");

    // ComputePipelineDescriptor

    static_assert(sizeof(ComputePipelineDescriptor) == sizeof(WGPUComputePipelineDescriptor), "sizeof mismatch for ComputePipelineDescriptor");
    static_assert(alignof(ComputePipelineDescriptor) == alignof(WGPUComputePipelineDescriptor), "alignof mismatch for ComputePipelineDescriptor");

    static_assert(offsetof(ComputePipelineDescriptor, nextInChain) == offsetof(WGPUComputePipelineDescriptor, nextInChain),
            "offsetof mismatch for ComputePipelineDescriptor::nextInChain");
    static_assert(offsetof(ComputePipelineDescriptor, label) == offsetof(WGPUComputePipelineDescriptor, label),
            "offsetof mismatch for ComputePipelineDescriptor::label");
    static_assert(offsetof(ComputePipelineDescriptor, layout) == offsetof(WGPUComputePipelineDescriptor, layout),
            "offsetof mismatch for ComputePipelineDescriptor::layout");
    static_assert(offsetof(ComputePipelineDescriptor, computeStage) == offsetof(WGPUComputePipelineDescriptor, computeStage),
            "offsetof mismatch for ComputePipelineDescriptor::computeStage");

    // DepthStencilStateDescriptor

    static_assert(sizeof(DepthStencilStateDescriptor) == sizeof(WGPUDepthStencilStateDescriptor), "sizeof mismatch for DepthStencilStateDescriptor");
    static_assert(alignof(DepthStencilStateDescriptor) == alignof(WGPUDepthStencilStateDescriptor), "alignof mismatch for DepthStencilStateDescriptor");

    static_assert(offsetof(DepthStencilStateDescriptor, nextInChain) == offsetof(WGPUDepthStencilStateDescriptor, nextInChain),
            "offsetof mismatch for DepthStencilStateDescriptor::nextInChain");
    static_assert(offsetof(DepthStencilStateDescriptor, format) == offsetof(WGPUDepthStencilStateDescriptor, format),
            "offsetof mismatch for DepthStencilStateDescriptor::format");
    static_assert(offsetof(DepthStencilStateDescriptor, depthWriteEnabled) == offsetof(WGPUDepthStencilStateDescriptor, depthWriteEnabled),
            "offsetof mismatch for DepthStencilStateDescriptor::depthWriteEnabled");
    static_assert(offsetof(DepthStencilStateDescriptor, depthCompare) == offsetof(WGPUDepthStencilStateDescriptor, depthCompare),
            "offsetof mismatch for DepthStencilStateDescriptor::depthCompare");
    static_assert(offsetof(DepthStencilStateDescriptor, stencilFront) == offsetof(WGPUDepthStencilStateDescriptor, stencilFront),
            "offsetof mismatch for DepthStencilStateDescriptor::stencilFront");
    static_assert(offsetof(DepthStencilStateDescriptor, stencilBack) == offsetof(WGPUDepthStencilStateDescriptor, stencilBack),
            "offsetof mismatch for DepthStencilStateDescriptor::stencilBack");
    static_assert(offsetof(DepthStencilStateDescriptor, stencilReadMask) == offsetof(WGPUDepthStencilStateDescriptor, stencilReadMask),
            "offsetof mismatch for DepthStencilStateDescriptor::stencilReadMask");
    static_assert(offsetof(DepthStencilStateDescriptor, stencilWriteMask) == offsetof(WGPUDepthStencilStateDescriptor, stencilWriteMask),
            "offsetof mismatch for DepthStencilStateDescriptor::stencilWriteMask");

    // RenderPassColorAttachmentDescriptor

    static_assert(sizeof(RenderPassColorAttachmentDescriptor) == sizeof(WGPURenderPassColorAttachmentDescriptor), "sizeof mismatch for RenderPassColorAttachmentDescriptor");
    static_assert(alignof(RenderPassColorAttachmentDescriptor) == alignof(WGPURenderPassColorAttachmentDescriptor), "alignof mismatch for RenderPassColorAttachmentDescriptor");

    static_assert(offsetof(RenderPassColorAttachmentDescriptor, attachment) == offsetof(WGPURenderPassColorAttachmentDescriptor, attachment),
            "offsetof mismatch for RenderPassColorAttachmentDescriptor::attachment");
    static_assert(offsetof(RenderPassColorAttachmentDescriptor, resolveTarget) == offsetof(WGPURenderPassColorAttachmentDescriptor, resolveTarget),
            "offsetof mismatch for RenderPassColorAttachmentDescriptor::resolveTarget");
    static_assert(offsetof(RenderPassColorAttachmentDescriptor, loadOp) == offsetof(WGPURenderPassColorAttachmentDescriptor, loadOp),
            "offsetof mismatch for RenderPassColorAttachmentDescriptor::loadOp");
    static_assert(offsetof(RenderPassColorAttachmentDescriptor, storeOp) == offsetof(WGPURenderPassColorAttachmentDescriptor, storeOp),
            "offsetof mismatch for RenderPassColorAttachmentDescriptor::storeOp");
    static_assert(offsetof(RenderPassColorAttachmentDescriptor, clearColor) == offsetof(WGPURenderPassColorAttachmentDescriptor, clearColor),
            "offsetof mismatch for RenderPassColorAttachmentDescriptor::clearColor");

    // TextureCopyView

    static_assert(sizeof(TextureCopyView) == sizeof(WGPUTextureCopyView), "sizeof mismatch for TextureCopyView");
    static_assert(alignof(TextureCopyView) == alignof(WGPUTextureCopyView), "alignof mismatch for TextureCopyView");

    static_assert(offsetof(TextureCopyView, nextInChain) == offsetof(WGPUTextureCopyView, nextInChain),
            "offsetof mismatch for TextureCopyView::nextInChain");
    static_assert(offsetof(TextureCopyView, texture) == offsetof(WGPUTextureCopyView, texture),
            "offsetof mismatch for TextureCopyView::texture");
    static_assert(offsetof(TextureCopyView, mipLevel) == offsetof(WGPUTextureCopyView, mipLevel),
            "offsetof mismatch for TextureCopyView::mipLevel");
    static_assert(offsetof(TextureCopyView, origin) == offsetof(WGPUTextureCopyView, origin),
            "offsetof mismatch for TextureCopyView::origin");

    // TextureDescriptor

    static_assert(sizeof(TextureDescriptor) == sizeof(WGPUTextureDescriptor), "sizeof mismatch for TextureDescriptor");
    static_assert(alignof(TextureDescriptor) == alignof(WGPUTextureDescriptor), "alignof mismatch for TextureDescriptor");

    static_assert(offsetof(TextureDescriptor, nextInChain) == offsetof(WGPUTextureDescriptor, nextInChain),
            "offsetof mismatch for TextureDescriptor::nextInChain");
    static_assert(offsetof(TextureDescriptor, label) == offsetof(WGPUTextureDescriptor, label),
            "offsetof mismatch for TextureDescriptor::label");
    static_assert(offsetof(TextureDescriptor, usage) == offsetof(WGPUTextureDescriptor, usage),
            "offsetof mismatch for TextureDescriptor::usage");
    static_assert(offsetof(TextureDescriptor, dimension) == offsetof(WGPUTextureDescriptor, dimension),
            "offsetof mismatch for TextureDescriptor::dimension");
    static_assert(offsetof(TextureDescriptor, size) == offsetof(WGPUTextureDescriptor, size),
            "offsetof mismatch for TextureDescriptor::size");
    static_assert(offsetof(TextureDescriptor, format) == offsetof(WGPUTextureDescriptor, format),
            "offsetof mismatch for TextureDescriptor::format");
    static_assert(offsetof(TextureDescriptor, mipLevelCount) == offsetof(WGPUTextureDescriptor, mipLevelCount),
            "offsetof mismatch for TextureDescriptor::mipLevelCount");
    static_assert(offsetof(TextureDescriptor, sampleCount) == offsetof(WGPUTextureDescriptor, sampleCount),
            "offsetof mismatch for TextureDescriptor::sampleCount");

    // VertexBufferLayoutDescriptor

    static_assert(sizeof(VertexBufferLayoutDescriptor) == sizeof(WGPUVertexBufferLayoutDescriptor), "sizeof mismatch for VertexBufferLayoutDescriptor");
    static_assert(alignof(VertexBufferLayoutDescriptor) == alignof(WGPUVertexBufferLayoutDescriptor), "alignof mismatch for VertexBufferLayoutDescriptor");

    static_assert(offsetof(VertexBufferLayoutDescriptor, arrayStride) == offsetof(WGPUVertexBufferLayoutDescriptor, arrayStride),
            "offsetof mismatch for VertexBufferLayoutDescriptor::arrayStride");
    static_assert(offsetof(VertexBufferLayoutDescriptor, stepMode) == offsetof(WGPUVertexBufferLayoutDescriptor, stepMode),
            "offsetof mismatch for VertexBufferLayoutDescriptor::stepMode");
    static_assert(offsetof(VertexBufferLayoutDescriptor, attributeCount) == offsetof(WGPUVertexBufferLayoutDescriptor, attributeCount),
            "offsetof mismatch for VertexBufferLayoutDescriptor::attributeCount");
    static_assert(offsetof(VertexBufferLayoutDescriptor, attributes) == offsetof(WGPUVertexBufferLayoutDescriptor, attributes),
            "offsetof mismatch for VertexBufferLayoutDescriptor::attributes");

    // RenderPassDescriptor

    static_assert(sizeof(RenderPassDescriptor) == sizeof(WGPURenderPassDescriptor), "sizeof mismatch for RenderPassDescriptor");
    static_assert(alignof(RenderPassDescriptor) == alignof(WGPURenderPassDescriptor), "alignof mismatch for RenderPassDescriptor");

    static_assert(offsetof(RenderPassDescriptor, nextInChain) == offsetof(WGPURenderPassDescriptor, nextInChain),
            "offsetof mismatch for RenderPassDescriptor::nextInChain");
    static_assert(offsetof(RenderPassDescriptor, label) == offsetof(WGPURenderPassDescriptor, label),
            "offsetof mismatch for RenderPassDescriptor::label");
    static_assert(offsetof(RenderPassDescriptor, colorAttachmentCount) == offsetof(WGPURenderPassDescriptor, colorAttachmentCount),
            "offsetof mismatch for RenderPassDescriptor::colorAttachmentCount");
    static_assert(offsetof(RenderPassDescriptor, colorAttachments) == offsetof(WGPURenderPassDescriptor, colorAttachments),
            "offsetof mismatch for RenderPassDescriptor::colorAttachments");
    static_assert(offsetof(RenderPassDescriptor, depthStencilAttachment) == offsetof(WGPURenderPassDescriptor, depthStencilAttachment),
            "offsetof mismatch for RenderPassDescriptor::depthStencilAttachment");
    static_assert(offsetof(RenderPassDescriptor, occlusionQuerySet) == offsetof(WGPURenderPassDescriptor, occlusionQuerySet),
            "offsetof mismatch for RenderPassDescriptor::occlusionQuerySet");

    // VertexStateDescriptor

    static_assert(sizeof(VertexStateDescriptor) == sizeof(WGPUVertexStateDescriptor), "sizeof mismatch for VertexStateDescriptor");
    static_assert(alignof(VertexStateDescriptor) == alignof(WGPUVertexStateDescriptor), "alignof mismatch for VertexStateDescriptor");

    static_assert(offsetof(VertexStateDescriptor, nextInChain) == offsetof(WGPUVertexStateDescriptor, nextInChain),
            "offsetof mismatch for VertexStateDescriptor::nextInChain");
    static_assert(offsetof(VertexStateDescriptor, indexFormat) == offsetof(WGPUVertexStateDescriptor, indexFormat),
            "offsetof mismatch for VertexStateDescriptor::indexFormat");
    static_assert(offsetof(VertexStateDescriptor, vertexBufferCount) == offsetof(WGPUVertexStateDescriptor, vertexBufferCount),
            "offsetof mismatch for VertexStateDescriptor::vertexBufferCount");
    static_assert(offsetof(VertexStateDescriptor, vertexBuffers) == offsetof(WGPUVertexStateDescriptor, vertexBuffers),
            "offsetof mismatch for VertexStateDescriptor::vertexBuffers");

    // RenderPipelineDescriptor

    static_assert(sizeof(RenderPipelineDescriptor) == sizeof(WGPURenderPipelineDescriptor), "sizeof mismatch for RenderPipelineDescriptor");
    static_assert(alignof(RenderPipelineDescriptor) == alignof(WGPURenderPipelineDescriptor), "alignof mismatch for RenderPipelineDescriptor");

    static_assert(offsetof(RenderPipelineDescriptor, nextInChain) == offsetof(WGPURenderPipelineDescriptor, nextInChain),
            "offsetof mismatch for RenderPipelineDescriptor::nextInChain");
    static_assert(offsetof(RenderPipelineDescriptor, label) == offsetof(WGPURenderPipelineDescriptor, label),
            "offsetof mismatch for RenderPipelineDescriptor::label");
    static_assert(offsetof(RenderPipelineDescriptor, layout) == offsetof(WGPURenderPipelineDescriptor, layout),
            "offsetof mismatch for RenderPipelineDescriptor::layout");
    static_assert(offsetof(RenderPipelineDescriptor, vertexStage) == offsetof(WGPURenderPipelineDescriptor, vertexStage),
            "offsetof mismatch for RenderPipelineDescriptor::vertexStage");
    static_assert(offsetof(RenderPipelineDescriptor, fragmentStage) == offsetof(WGPURenderPipelineDescriptor, fragmentStage),
            "offsetof mismatch for RenderPipelineDescriptor::fragmentStage");
    static_assert(offsetof(RenderPipelineDescriptor, vertexState) == offsetof(WGPURenderPipelineDescriptor, vertexState),
            "offsetof mismatch for RenderPipelineDescriptor::vertexState");
    static_assert(offsetof(RenderPipelineDescriptor, primitiveTopology) == offsetof(WGPURenderPipelineDescriptor, primitiveTopology),
            "offsetof mismatch for RenderPipelineDescriptor::primitiveTopology");
    static_assert(offsetof(RenderPipelineDescriptor, rasterizationState) == offsetof(WGPURenderPipelineDescriptor, rasterizationState),
            "offsetof mismatch for RenderPipelineDescriptor::rasterizationState");
    static_assert(offsetof(RenderPipelineDescriptor, sampleCount) == offsetof(WGPURenderPipelineDescriptor, sampleCount),
            "offsetof mismatch for RenderPipelineDescriptor::sampleCount");
    static_assert(offsetof(RenderPipelineDescriptor, depthStencilState) == offsetof(WGPURenderPipelineDescriptor, depthStencilState),
            "offsetof mismatch for RenderPipelineDescriptor::depthStencilState");
    static_assert(offsetof(RenderPipelineDescriptor, colorStateCount) == offsetof(WGPURenderPipelineDescriptor, colorStateCount),
            "offsetof mismatch for RenderPipelineDescriptor::colorStateCount");
    static_assert(offsetof(RenderPipelineDescriptor, colorStates) == offsetof(WGPURenderPipelineDescriptor, colorStates),
            "offsetof mismatch for RenderPipelineDescriptor::colorStates");
    static_assert(offsetof(RenderPipelineDescriptor, sampleMask) == offsetof(WGPURenderPipelineDescriptor, sampleMask),
            "offsetof mismatch for RenderPipelineDescriptor::sampleMask");
    static_assert(offsetof(RenderPipelineDescriptor, alphaToCoverageEnabled) == offsetof(WGPURenderPipelineDescriptor, alphaToCoverageEnabled),
            "offsetof mismatch for RenderPipelineDescriptor::alphaToCoverageEnabled");

    // BindGroup

    static_assert(sizeof(BindGroup) == sizeof(WGPUBindGroup), "sizeof mismatch for BindGroup");
    static_assert(alignof(BindGroup) == alignof(WGPUBindGroup), "alignof mismatch for BindGroup");

    void BindGroup::WGPUReference(WGPUBindGroup handle) {
        if (handle != nullptr) {
            wgpuBindGroupReference(handle);
        }
    }
    void BindGroup::WGPURelease(WGPUBindGroup handle) {
        if (handle != nullptr) {
            wgpuBindGroupRelease(handle);
        }
    }

    // BindGroupLayout

    static_assert(sizeof(BindGroupLayout) == sizeof(WGPUBindGroupLayout), "sizeof mismatch for BindGroupLayout");
    static_assert(alignof(BindGroupLayout) == alignof(WGPUBindGroupLayout), "alignof mismatch for BindGroupLayout");

    void BindGroupLayout::WGPUReference(WGPUBindGroupLayout handle) {
        if (handle != nullptr) {
            wgpuBindGroupLayoutReference(handle);
        }
    }
    void BindGroupLayout::WGPURelease(WGPUBindGroupLayout handle) {
        if (handle != nullptr) {
            wgpuBindGroupLayoutRelease(handle);
        }
    }

    // Buffer

    static_assert(sizeof(Buffer) == sizeof(WGPUBuffer), "sizeof mismatch for Buffer");
    static_assert(alignof(Buffer) == alignof(WGPUBuffer), "alignof mismatch for Buffer");

    void Buffer::Destroy() const {
        wgpuBufferDestroy(Get());
    }
    void const * Buffer::GetConstMappedRange(size_t offset, size_t size) const {
        auto result = wgpuBufferGetConstMappedRange(Get(), offset, size);
        return result;
    }
    void * Buffer::GetMappedRange(size_t offset, size_t size) const {
        auto result = wgpuBufferGetMappedRange(Get(), offset, size);
        return result;
    }
    void Buffer::MapAsync(MapMode mode, size_t offset, size_t size, BufferMapCallback callback, void * userdata) const {
        wgpuBufferMapAsync(Get(), static_cast<WGPUMapMode>(mode), offset, size, callback, reinterpret_cast<void * >(userdata));
    }
    void Buffer::Unmap() const {
        wgpuBufferUnmap(Get());
    }
    void Buffer::WGPUReference(WGPUBuffer handle) {
        if (handle != nullptr) {
            wgpuBufferReference(handle);
        }
    }
    void Buffer::WGPURelease(WGPUBuffer handle) {
        if (handle != nullptr) {
            wgpuBufferRelease(handle);
        }
    }

    // CommandBuffer

    static_assert(sizeof(CommandBuffer) == sizeof(WGPUCommandBuffer), "sizeof mismatch for CommandBuffer");
    static_assert(alignof(CommandBuffer) == alignof(WGPUCommandBuffer), "alignof mismatch for CommandBuffer");

    void CommandBuffer::WGPUReference(WGPUCommandBuffer handle) {
        if (handle != nullptr) {
            wgpuCommandBufferReference(handle);
        }
    }
    void CommandBuffer::WGPURelease(WGPUCommandBuffer handle) {
        if (handle != nullptr) {
            wgpuCommandBufferRelease(handle);
        }
    }

    // CommandEncoder

    static_assert(sizeof(CommandEncoder) == sizeof(WGPUCommandEncoder), "sizeof mismatch for CommandEncoder");
    static_assert(alignof(CommandEncoder) == alignof(WGPUCommandEncoder), "alignof mismatch for CommandEncoder");

    ComputePassEncoder CommandEncoder::BeginComputePass(ComputePassDescriptor const * descriptor) const {
        auto result = wgpuCommandEncoderBeginComputePass(Get(), reinterpret_cast<WGPUComputePassDescriptor const * >(descriptor));
        return ComputePassEncoder::Acquire(result);
    }
    RenderPassEncoder CommandEncoder::BeginRenderPass(RenderPassDescriptor const * descriptor) const {
        auto result = wgpuCommandEncoderBeginRenderPass(Get(), reinterpret_cast<WGPURenderPassDescriptor const * >(descriptor));
        return RenderPassEncoder::Acquire(result);
    }
    void CommandEncoder::CopyBufferToBuffer(Buffer const& source, uint64_t sourceOffset, Buffer const& destination, uint64_t destinationOffset, uint64_t size) const {
        wgpuCommandEncoderCopyBufferToBuffer(Get(), source.Get(), sourceOffset, destination.Get(), destinationOffset, size);
    }
    void CommandEncoder::CopyBufferToTexture(BufferCopyView const * source, TextureCopyView const * destination, Extent3D const * copySize) const {
        wgpuCommandEncoderCopyBufferToTexture(Get(), reinterpret_cast<WGPUBufferCopyView const * >(source), reinterpret_cast<WGPUTextureCopyView const * >(destination), reinterpret_cast<WGPUExtent3D const * >(copySize));
    }
    void CommandEncoder::CopyTextureToBuffer(TextureCopyView const * source, BufferCopyView const * destination, Extent3D const * copySize) const {
        wgpuCommandEncoderCopyTextureToBuffer(Get(), reinterpret_cast<WGPUTextureCopyView const * >(source), reinterpret_cast<WGPUBufferCopyView const * >(destination), reinterpret_cast<WGPUExtent3D const * >(copySize));
    }
    void CommandEncoder::CopyTextureToTexture(TextureCopyView const * source, TextureCopyView const * destination, Extent3D const * copySize) const {
        wgpuCommandEncoderCopyTextureToTexture(Get(), reinterpret_cast<WGPUTextureCopyView const * >(source), reinterpret_cast<WGPUTextureCopyView const * >(destination), reinterpret_cast<WGPUExtent3D const * >(copySize));
    }
    CommandBuffer CommandEncoder::Finish(CommandBufferDescriptor const * descriptor) const {
        auto result = wgpuCommandEncoderFinish(Get(), reinterpret_cast<WGPUCommandBufferDescriptor const * >(descriptor));
        return CommandBuffer::Acquire(result);
    }
    void CommandEncoder::InsertDebugMarker(char const * markerLabel) const {
        wgpuCommandEncoderInsertDebugMarker(Get(), reinterpret_cast<char const * >(markerLabel));
    }
    void CommandEncoder::PopDebugGroup() const {
        wgpuCommandEncoderPopDebugGroup(Get());
    }
    void CommandEncoder::PushDebugGroup(char const * groupLabel) const {
        wgpuCommandEncoderPushDebugGroup(Get(), reinterpret_cast<char const * >(groupLabel));
    }
    void CommandEncoder::ResolveQuerySet(QuerySet const& querySet, uint32_t firstQuery, uint32_t queryCount, Buffer const& destination, uint64_t destinationOffset) const {
        wgpuCommandEncoderResolveQuerySet(Get(), querySet.Get(), firstQuery, queryCount, destination.Get(), destinationOffset);
    }
    void CommandEncoder::WriteTimestamp(QuerySet const& querySet, uint32_t queryIndex) const {
        wgpuCommandEncoderWriteTimestamp(Get(), querySet.Get(), queryIndex);
    }
    void CommandEncoder::WGPUReference(WGPUCommandEncoder handle) {
        if (handle != nullptr) {
            wgpuCommandEncoderReference(handle);
        }
    }
    void CommandEncoder::WGPURelease(WGPUCommandEncoder handle) {
        if (handle != nullptr) {
            wgpuCommandEncoderRelease(handle);
        }
    }

    // ComputePassEncoder

    static_assert(sizeof(ComputePassEncoder) == sizeof(WGPUComputePassEncoder), "sizeof mismatch for ComputePassEncoder");
    static_assert(alignof(ComputePassEncoder) == alignof(WGPUComputePassEncoder), "alignof mismatch for ComputePassEncoder");

    void ComputePassEncoder::Dispatch(uint32_t x, uint32_t y, uint32_t z) const {
        wgpuComputePassEncoderDispatch(Get(), x, y, z);
    }
    void ComputePassEncoder::DispatchIndirect(Buffer const& indirectBuffer, uint64_t indirectOffset) const {
        wgpuComputePassEncoderDispatchIndirect(Get(), indirectBuffer.Get(), indirectOffset);
    }
    void ComputePassEncoder::EndPass() const {
        wgpuComputePassEncoderEndPass(Get());
    }
    void ComputePassEncoder::InsertDebugMarker(char const * markerLabel) const {
        wgpuComputePassEncoderInsertDebugMarker(Get(), reinterpret_cast<char const * >(markerLabel));
    }
    void ComputePassEncoder::PopDebugGroup() const {
        wgpuComputePassEncoderPopDebugGroup(Get());
    }
    void ComputePassEncoder::PushDebugGroup(char const * groupLabel) const {
        wgpuComputePassEncoderPushDebugGroup(Get(), reinterpret_cast<char const * >(groupLabel));
    }
    void ComputePassEncoder::SetBindGroup(uint32_t groupIndex, BindGroup const& group, uint32_t dynamicOffsetCount, uint32_t const * dynamicOffsets) const {
        wgpuComputePassEncoderSetBindGroup(Get(), groupIndex, group.Get(), dynamicOffsetCount, reinterpret_cast<uint32_t const * >(dynamicOffsets));
    }
    void ComputePassEncoder::SetPipeline(ComputePipeline const& pipeline) const {
        wgpuComputePassEncoderSetPipeline(Get(), pipeline.Get());
    }
    void ComputePassEncoder::WriteTimestamp(QuerySet const& querySet, uint32_t queryIndex) const {
        wgpuComputePassEncoderWriteTimestamp(Get(), querySet.Get(), queryIndex);
    }
    void ComputePassEncoder::WGPUReference(WGPUComputePassEncoder handle) {
        if (handle != nullptr) {
            wgpuComputePassEncoderReference(handle);
        }
    }
    void ComputePassEncoder::WGPURelease(WGPUComputePassEncoder handle) {
        if (handle != nullptr) {
            wgpuComputePassEncoderRelease(handle);
        }
    }

    // ComputePipeline

    static_assert(sizeof(ComputePipeline) == sizeof(WGPUComputePipeline), "sizeof mismatch for ComputePipeline");
    static_assert(alignof(ComputePipeline) == alignof(WGPUComputePipeline), "alignof mismatch for ComputePipeline");

    BindGroupLayout ComputePipeline::GetBindGroupLayout(uint32_t groupIndex) const {
        auto result = wgpuComputePipelineGetBindGroupLayout(Get(), groupIndex);
        return BindGroupLayout::Acquire(result);
    }
    void ComputePipeline::WGPUReference(WGPUComputePipeline handle) {
        if (handle != nullptr) {
            wgpuComputePipelineReference(handle);
        }
    }
    void ComputePipeline::WGPURelease(WGPUComputePipeline handle) {
        if (handle != nullptr) {
            wgpuComputePipelineRelease(handle);
        }
    }

    // Device

    static_assert(sizeof(Device) == sizeof(WGPUDevice), "sizeof mismatch for Device");
    static_assert(alignof(Device) == alignof(WGPUDevice), "alignof mismatch for Device");

    BindGroup Device::CreateBindGroup(BindGroupDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateBindGroup(Get(), reinterpret_cast<WGPUBindGroupDescriptor const * >(descriptor));
        return BindGroup::Acquire(result);
    }
    BindGroupLayout Device::CreateBindGroupLayout(BindGroupLayoutDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateBindGroupLayout(Get(), reinterpret_cast<WGPUBindGroupLayoutDescriptor const * >(descriptor));
        return BindGroupLayout::Acquire(result);
    }
    Buffer Device::CreateBuffer(BufferDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateBuffer(Get(), reinterpret_cast<WGPUBufferDescriptor const * >(descriptor));
        return Buffer::Acquire(result);
    }
    CommandEncoder Device::CreateCommandEncoder(CommandEncoderDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateCommandEncoder(Get(), reinterpret_cast<WGPUCommandEncoderDescriptor const * >(descriptor));
        return CommandEncoder::Acquire(result);
    }
    ComputePipeline Device::CreateComputePipeline(ComputePipelineDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateComputePipeline(Get(), reinterpret_cast<WGPUComputePipelineDescriptor const * >(descriptor));
        return ComputePipeline::Acquire(result);
    }
    PipelineLayout Device::CreatePipelineLayout(PipelineLayoutDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreatePipelineLayout(Get(), reinterpret_cast<WGPUPipelineLayoutDescriptor const * >(descriptor));
        return PipelineLayout::Acquire(result);
    }
    QuerySet Device::CreateQuerySet(QuerySetDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateQuerySet(Get(), reinterpret_cast<WGPUQuerySetDescriptor const * >(descriptor));
        return QuerySet::Acquire(result);
    }
    RenderBundleEncoder Device::CreateRenderBundleEncoder(RenderBundleEncoderDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateRenderBundleEncoder(Get(), reinterpret_cast<WGPURenderBundleEncoderDescriptor const * >(descriptor));
        return RenderBundleEncoder::Acquire(result);
    }
    RenderPipeline Device::CreateRenderPipeline(RenderPipelineDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateRenderPipeline(Get(), reinterpret_cast<WGPURenderPipelineDescriptor const * >(descriptor));
        return RenderPipeline::Acquire(result);
    }
    Sampler Device::CreateSampler(SamplerDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateSampler(Get(), reinterpret_cast<WGPUSamplerDescriptor const * >(descriptor));
        return Sampler::Acquire(result);
    }
    ShaderModule Device::CreateShaderModule(ShaderModuleDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateShaderModule(Get(), reinterpret_cast<WGPUShaderModuleDescriptor const * >(descriptor));
        return ShaderModule::Acquire(result);
    }
    SwapChain Device::CreateSwapChain(Surface const& surface, SwapChainDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateSwapChain(Get(), surface.Get(), reinterpret_cast<WGPUSwapChainDescriptor const * >(descriptor));
        return SwapChain::Acquire(result);
    }
    Texture Device::CreateTexture(TextureDescriptor const * descriptor) const {
        auto result = wgpuDeviceCreateTexture(Get(), reinterpret_cast<WGPUTextureDescriptor const * >(descriptor));
        return Texture::Acquire(result);
    }
    Queue Device::GetDefaultQueue() const {
        auto result = wgpuDeviceGetDefaultQueue(Get());
        return Queue::Acquire(result);
    }
    bool Device::PopErrorScope(ErrorCallback callback, void * userdata) const {
        auto result = wgpuDevicePopErrorScope(Get(), callback, reinterpret_cast<void * >(userdata));
        return result;
    }
    void Device::PushErrorScope(ErrorFilter filter) const {
        wgpuDevicePushErrorScope(Get(), static_cast<WGPUErrorFilter>(filter));
    }
    void Device::SetDeviceLostCallback(DeviceLostCallback callback, void * userdata) const {
        wgpuDeviceSetDeviceLostCallback(Get(), callback, reinterpret_cast<void * >(userdata));
    }
    void Device::SetUncapturedErrorCallback(ErrorCallback callback, void * userdata) const {
        wgpuDeviceSetUncapturedErrorCallback(Get(), callback, reinterpret_cast<void * >(userdata));
    }
    void Device::WGPUReference(WGPUDevice handle) {
        if (handle != nullptr) {
            wgpuDeviceReference(handle);
        }
    }
    void Device::WGPURelease(WGPUDevice handle) {
        if (handle != nullptr) {
            wgpuDeviceRelease(handle);
        }
    }

    // Fence

    static_assert(sizeof(Fence) == sizeof(WGPUFence), "sizeof mismatch for Fence");
    static_assert(alignof(Fence) == alignof(WGPUFence), "alignof mismatch for Fence");

    uint64_t Fence::GetCompletedValue() const {
        auto result = wgpuFenceGetCompletedValue(Get());
        return result;
    }
    void Fence::OnCompletion(uint64_t value, FenceOnCompletionCallback callback, void * userdata) const {
        wgpuFenceOnCompletion(Get(), value, callback, reinterpret_cast<void * >(userdata));
    }
    void Fence::WGPUReference(WGPUFence handle) {
        if (handle != nullptr) {
            wgpuFenceReference(handle);
        }
    }
    void Fence::WGPURelease(WGPUFence handle) {
        if (handle != nullptr) {
            wgpuFenceRelease(handle);
        }
    }

    // Instance

    static_assert(sizeof(Instance) == sizeof(WGPUInstance), "sizeof mismatch for Instance");
    static_assert(alignof(Instance) == alignof(WGPUInstance), "alignof mismatch for Instance");

    Surface Instance::CreateSurface(SurfaceDescriptor const * descriptor) const {
        auto result = wgpuInstanceCreateSurface(Get(), reinterpret_cast<WGPUSurfaceDescriptor const * >(descriptor));
        return Surface::Acquire(result);
    }
    void Instance::WGPUReference(WGPUInstance handle) {
        if (handle != nullptr) {
            wgpuInstanceReference(handle);
        }
    }
    void Instance::WGPURelease(WGPUInstance handle) {
        if (handle != nullptr) {
            wgpuInstanceRelease(handle);
        }
    }

    // PipelineLayout

    static_assert(sizeof(PipelineLayout) == sizeof(WGPUPipelineLayout), "sizeof mismatch for PipelineLayout");
    static_assert(alignof(PipelineLayout) == alignof(WGPUPipelineLayout), "alignof mismatch for PipelineLayout");

    void PipelineLayout::WGPUReference(WGPUPipelineLayout handle) {
        if (handle != nullptr) {
            wgpuPipelineLayoutReference(handle);
        }
    }
    void PipelineLayout::WGPURelease(WGPUPipelineLayout handle) {
        if (handle != nullptr) {
            wgpuPipelineLayoutRelease(handle);
        }
    }

    // QuerySet

    static_assert(sizeof(QuerySet) == sizeof(WGPUQuerySet), "sizeof mismatch for QuerySet");
    static_assert(alignof(QuerySet) == alignof(WGPUQuerySet), "alignof mismatch for QuerySet");

    void QuerySet::Destroy() const {
        wgpuQuerySetDestroy(Get());
    }
    void QuerySet::WGPUReference(WGPUQuerySet handle) {
        if (handle != nullptr) {
            wgpuQuerySetReference(handle);
        }
    }
    void QuerySet::WGPURelease(WGPUQuerySet handle) {
        if (handle != nullptr) {
            wgpuQuerySetRelease(handle);
        }
    }

    // Queue

    static_assert(sizeof(Queue) == sizeof(WGPUQueue), "sizeof mismatch for Queue");
    static_assert(alignof(Queue) == alignof(WGPUQueue), "alignof mismatch for Queue");

    Fence Queue::CreateFence(FenceDescriptor const * descriptor) const {
        auto result = wgpuQueueCreateFence(Get(), reinterpret_cast<WGPUFenceDescriptor const * >(descriptor));
        return Fence::Acquire(result);
    }
    void Queue::Signal(Fence const& fence, uint64_t signalValue) const {
        wgpuQueueSignal(Get(), fence.Get(), signalValue);
    }
    void Queue::Submit(uint32_t commandCount, CommandBuffer const * commands) const {
        wgpuQueueSubmit(Get(), commandCount, reinterpret_cast<WGPUCommandBuffer const * >(commands));
    }
    void Queue::WriteBuffer(Buffer const& buffer, uint64_t bufferOffset, void const * data, size_t size) const {
        wgpuQueueWriteBuffer(Get(), buffer.Get(), bufferOffset, reinterpret_cast<void const * >(data), size);
    }
    void Queue::WriteTexture(TextureCopyView const * destination, void const * data, size_t dataSize, TextureDataLayout const * dataLayout, Extent3D const * writeSize) const {
        wgpuQueueWriteTexture(Get(), reinterpret_cast<WGPUTextureCopyView const * >(destination), reinterpret_cast<void const * >(data), dataSize, reinterpret_cast<WGPUTextureDataLayout const * >(dataLayout), reinterpret_cast<WGPUExtent3D const * >(writeSize));
    }
    void Queue::WGPUReference(WGPUQueue handle) {
        if (handle != nullptr) {
            wgpuQueueReference(handle);
        }
    }
    void Queue::WGPURelease(WGPUQueue handle) {
        if (handle != nullptr) {
            wgpuQueueRelease(handle);
        }
    }

    // RenderBundle

    static_assert(sizeof(RenderBundle) == sizeof(WGPURenderBundle), "sizeof mismatch for RenderBundle");
    static_assert(alignof(RenderBundle) == alignof(WGPURenderBundle), "alignof mismatch for RenderBundle");

    void RenderBundle::WGPUReference(WGPURenderBundle handle) {
        if (handle != nullptr) {
            wgpuRenderBundleReference(handle);
        }
    }
    void RenderBundle::WGPURelease(WGPURenderBundle handle) {
        if (handle != nullptr) {
            wgpuRenderBundleRelease(handle);
        }
    }

    // RenderBundleEncoder

    static_assert(sizeof(RenderBundleEncoder) == sizeof(WGPURenderBundleEncoder), "sizeof mismatch for RenderBundleEncoder");
    static_assert(alignof(RenderBundleEncoder) == alignof(WGPURenderBundleEncoder), "alignof mismatch for RenderBundleEncoder");

    void RenderBundleEncoder::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const {
        wgpuRenderBundleEncoderDraw(Get(), vertexCount, instanceCount, firstVertex, firstInstance);
    }
    void RenderBundleEncoder::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) const {
        wgpuRenderBundleEncoderDrawIndexed(Get(), indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }
    void RenderBundleEncoder::DrawIndexedIndirect(Buffer const& indirectBuffer, uint64_t indirectOffset) const {
        wgpuRenderBundleEncoderDrawIndexedIndirect(Get(), indirectBuffer.Get(), indirectOffset);
    }
    void RenderBundleEncoder::DrawIndirect(Buffer const& indirectBuffer, uint64_t indirectOffset) const {
        wgpuRenderBundleEncoderDrawIndirect(Get(), indirectBuffer.Get(), indirectOffset);
    }
    RenderBundle RenderBundleEncoder::Finish(RenderBundleDescriptor const * descriptor) const {
        auto result = wgpuRenderBundleEncoderFinish(Get(), reinterpret_cast<WGPURenderBundleDescriptor const * >(descriptor));
        return RenderBundle::Acquire(result);
    }
    void RenderBundleEncoder::InsertDebugMarker(char const * markerLabel) const {
        wgpuRenderBundleEncoderInsertDebugMarker(Get(), reinterpret_cast<char const * >(markerLabel));
    }
    void RenderBundleEncoder::PopDebugGroup() const {
        wgpuRenderBundleEncoderPopDebugGroup(Get());
    }
    void RenderBundleEncoder::PushDebugGroup(char const * groupLabel) const {
        wgpuRenderBundleEncoderPushDebugGroup(Get(), reinterpret_cast<char const * >(groupLabel));
    }
    void RenderBundleEncoder::SetBindGroup(uint32_t groupIndex, BindGroup const& group, uint32_t dynamicOffsetCount, uint32_t const * dynamicOffsets) const {
        wgpuRenderBundleEncoderSetBindGroup(Get(), groupIndex, group.Get(), dynamicOffsetCount, reinterpret_cast<uint32_t const * >(dynamicOffsets));
    }
    void RenderBundleEncoder::SetIndexBuffer(Buffer const& buffer, uint64_t offset, uint64_t size) const {
        wgpuRenderBundleEncoderSetIndexBuffer(Get(), buffer.Get(), offset, size);
    }
    void RenderBundleEncoder::SetPipeline(RenderPipeline const& pipeline) const {
        wgpuRenderBundleEncoderSetPipeline(Get(), pipeline.Get());
    }
    void RenderBundleEncoder::SetVertexBuffer(uint32_t slot, Buffer const& buffer, uint64_t offset, uint64_t size) const {
        wgpuRenderBundleEncoderSetVertexBuffer(Get(), slot, buffer.Get(), offset, size);
    }
    void RenderBundleEncoder::WGPUReference(WGPURenderBundleEncoder handle) {
        if (handle != nullptr) {
            wgpuRenderBundleEncoderReference(handle);
        }
    }
    void RenderBundleEncoder::WGPURelease(WGPURenderBundleEncoder handle) {
        if (handle != nullptr) {
            wgpuRenderBundleEncoderRelease(handle);
        }
    }

    // RenderPassEncoder

    static_assert(sizeof(RenderPassEncoder) == sizeof(WGPURenderPassEncoder), "sizeof mismatch for RenderPassEncoder");
    static_assert(alignof(RenderPassEncoder) == alignof(WGPURenderPassEncoder), "alignof mismatch for RenderPassEncoder");

    void RenderPassEncoder::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const {
        wgpuRenderPassEncoderDraw(Get(), vertexCount, instanceCount, firstVertex, firstInstance);
    }
    void RenderPassEncoder::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) const {
        wgpuRenderPassEncoderDrawIndexed(Get(), indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }
    void RenderPassEncoder::DrawIndexedIndirect(Buffer const& indirectBuffer, uint64_t indirectOffset) const {
        wgpuRenderPassEncoderDrawIndexedIndirect(Get(), indirectBuffer.Get(), indirectOffset);
    }
    void RenderPassEncoder::DrawIndirect(Buffer const& indirectBuffer, uint64_t indirectOffset) const {
        wgpuRenderPassEncoderDrawIndirect(Get(), indirectBuffer.Get(), indirectOffset);
    }
    void RenderPassEncoder::EndPass() const {
        wgpuRenderPassEncoderEndPass(Get());
    }
    void RenderPassEncoder::ExecuteBundles(uint32_t bundlesCount, RenderBundle const * bundles) const {
        wgpuRenderPassEncoderExecuteBundles(Get(), bundlesCount, reinterpret_cast<WGPURenderBundle const * >(bundles));
    }
    void RenderPassEncoder::InsertDebugMarker(char const * markerLabel) const {
        wgpuRenderPassEncoderInsertDebugMarker(Get(), reinterpret_cast<char const * >(markerLabel));
    }
    void RenderPassEncoder::PopDebugGroup() const {
        wgpuRenderPassEncoderPopDebugGroup(Get());
    }
    void RenderPassEncoder::PushDebugGroup(char const * groupLabel) const {
        wgpuRenderPassEncoderPushDebugGroup(Get(), reinterpret_cast<char const * >(groupLabel));
    }
    void RenderPassEncoder::SetBindGroup(uint32_t groupIndex, BindGroup const& group, uint32_t dynamicOffsetCount, uint32_t const * dynamicOffsets) const {
        wgpuRenderPassEncoderSetBindGroup(Get(), groupIndex, group.Get(), dynamicOffsetCount, reinterpret_cast<uint32_t const * >(dynamicOffsets));
    }
    void RenderPassEncoder::SetBlendColor(Color const * color) const {
        wgpuRenderPassEncoderSetBlendColor(Get(), reinterpret_cast<WGPUColor const * >(color));
    }
    void RenderPassEncoder::SetIndexBuffer(Buffer const& buffer, uint64_t offset, uint64_t size) const {
        wgpuRenderPassEncoderSetIndexBuffer(Get(), buffer.Get(), offset, size);
    }
    void RenderPassEncoder::SetPipeline(RenderPipeline const& pipeline) const {
        wgpuRenderPassEncoderSetPipeline(Get(), pipeline.Get());
    }
    void RenderPassEncoder::SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const {
        wgpuRenderPassEncoderSetScissorRect(Get(), x, y, width, height);
    }
    void RenderPassEncoder::SetStencilReference(uint32_t reference) const {
        wgpuRenderPassEncoderSetStencilReference(Get(), reference);
    }
    void RenderPassEncoder::SetVertexBuffer(uint32_t slot, Buffer const& buffer, uint64_t offset, uint64_t size) const {
        wgpuRenderPassEncoderSetVertexBuffer(Get(), slot, buffer.Get(), offset, size);
    }
    void RenderPassEncoder::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth) const {
        wgpuRenderPassEncoderSetViewport(Get(), x, y, width, height, minDepth, maxDepth);
    }
    void RenderPassEncoder::WriteTimestamp(QuerySet const& querySet, uint32_t queryIndex) const {
        wgpuRenderPassEncoderWriteTimestamp(Get(), querySet.Get(), queryIndex);
    }
    void RenderPassEncoder::WGPUReference(WGPURenderPassEncoder handle) {
        if (handle != nullptr) {
            wgpuRenderPassEncoderReference(handle);
        }
    }
    void RenderPassEncoder::WGPURelease(WGPURenderPassEncoder handle) {
        if (handle != nullptr) {
            wgpuRenderPassEncoderRelease(handle);
        }
    }

    // RenderPipeline

    static_assert(sizeof(RenderPipeline) == sizeof(WGPURenderPipeline), "sizeof mismatch for RenderPipeline");
    static_assert(alignof(RenderPipeline) == alignof(WGPURenderPipeline), "alignof mismatch for RenderPipeline");

    BindGroupLayout RenderPipeline::GetBindGroupLayout(uint32_t groupIndex) const {
        auto result = wgpuRenderPipelineGetBindGroupLayout(Get(), groupIndex);
        return BindGroupLayout::Acquire(result);
    }
    void RenderPipeline::WGPUReference(WGPURenderPipeline handle) {
        if (handle != nullptr) {
            wgpuRenderPipelineReference(handle);
        }
    }
    void RenderPipeline::WGPURelease(WGPURenderPipeline handle) {
        if (handle != nullptr) {
            wgpuRenderPipelineRelease(handle);
        }
    }

    // Sampler

    static_assert(sizeof(Sampler) == sizeof(WGPUSampler), "sizeof mismatch for Sampler");
    static_assert(alignof(Sampler) == alignof(WGPUSampler), "alignof mismatch for Sampler");

    void Sampler::WGPUReference(WGPUSampler handle) {
        if (handle != nullptr) {
            wgpuSamplerReference(handle);
        }
    }
    void Sampler::WGPURelease(WGPUSampler handle) {
        if (handle != nullptr) {
            wgpuSamplerRelease(handle);
        }
    }

    // ShaderModule

    static_assert(sizeof(ShaderModule) == sizeof(WGPUShaderModule), "sizeof mismatch for ShaderModule");
    static_assert(alignof(ShaderModule) == alignof(WGPUShaderModule), "alignof mismatch for ShaderModule");

    void ShaderModule::WGPUReference(WGPUShaderModule handle) {
        if (handle != nullptr) {
            wgpuShaderModuleReference(handle);
        }
    }
    void ShaderModule::WGPURelease(WGPUShaderModule handle) {
        if (handle != nullptr) {
            wgpuShaderModuleRelease(handle);
        }
    }

    // Surface

    static_assert(sizeof(Surface) == sizeof(WGPUSurface), "sizeof mismatch for Surface");
    static_assert(alignof(Surface) == alignof(WGPUSurface), "alignof mismatch for Surface");

    void Surface::WGPUReference(WGPUSurface handle) {
        if (handle != nullptr) {
            wgpuSurfaceReference(handle);
        }
    }
    void Surface::WGPURelease(WGPUSurface handle) {
        if (handle != nullptr) {
            wgpuSurfaceRelease(handle);
        }
    }

    // SwapChain

    static_assert(sizeof(SwapChain) == sizeof(WGPUSwapChain), "sizeof mismatch for SwapChain");
    static_assert(alignof(SwapChain) == alignof(WGPUSwapChain), "alignof mismatch for SwapChain");



    TextureView SwapChain::GetCurrentTextureView() const {
        auto result = wgpuSwapChainGetCurrentTextureView(Get());
        return TextureView::Acquire(result);
    }
    void SwapChain::Present() const {
        wgpuSwapChainPresent(Get());
    }
    void SwapChain::WGPUReference(WGPUSwapChain handle) {
        if (handle != nullptr) {
            wgpuSwapChainReference(handle);
        }
    }
    void SwapChain::WGPURelease(WGPUSwapChain handle) {
        if (handle != nullptr) {
            wgpuSwapChainRelease(handle);
        }
    }

    // Texture

    static_assert(sizeof(Texture) == sizeof(WGPUTexture), "sizeof mismatch for Texture");
    static_assert(alignof(Texture) == alignof(WGPUTexture), "alignof mismatch for Texture");

    TextureView Texture::CreateView(TextureViewDescriptor const * descriptor) const {
        auto result = wgpuTextureCreateView(Get(), reinterpret_cast<WGPUTextureViewDescriptor const * >(descriptor));
        return TextureView::Acquire(result);
    }
    void Texture::Destroy() const {
        wgpuTextureDestroy(Get());
    }
    void Texture::WGPUReference(WGPUTexture handle) {
        if (handle != nullptr) {
            wgpuTextureReference(handle);
        }
    }
    void Texture::WGPURelease(WGPUTexture handle) {
        if (handle != nullptr) {
            wgpuTextureRelease(handle);
        }
    }

    // TextureView

    static_assert(sizeof(TextureView) == sizeof(WGPUTextureView), "sizeof mismatch for TextureView");
    static_assert(alignof(TextureView) == alignof(WGPUTextureView), "alignof mismatch for TextureView");

    void TextureView::WGPUReference(WGPUTextureView handle) {
        if (handle != nullptr) {
            wgpuTextureViewReference(handle);
        }
    }
    void TextureView::WGPURelease(WGPUTextureView handle) {
        if (handle != nullptr) {
            wgpuTextureViewRelease(handle);
        }
    }

    // Instance

    Instance CreateInstance(const InstanceDescriptor* descriptor) {
        const WGPUInstanceDescriptor* cDescriptor =
            reinterpret_cast<const WGPUInstanceDescriptor*>(descriptor);
        return Instance::Acquire(wgpuCreateInstance(cDescriptor));
    }

    Proc GetProcAddress(Device const& device, const char* procName) {
        return reinterpret_cast<Proc>(wgpuGetProcAddress(device.Get(), procName));
    }

}
