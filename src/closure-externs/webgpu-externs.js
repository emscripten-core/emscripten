/*
 * WebGPU globals
 * Generated using https://github.com/kainino0x/webidl-to-closure-externs
 * against the spec's WebIDL: https://gpuweb.github.io/gpuweb/webgpu.idl
 */

/** @type {?GPU} */
Navigator.prototype.gpu;

/** @type {?GPU} */
WorkerNavigator.prototype.gpu;

const GPUBufferUsage = {};
/** @type {number} */
GPUBufferUsage.MAP_READ;
/** @type {number} */
GPUBufferUsage.MAP_WRITE;
/** @type {number} */
GPUBufferUsage.COPY_SRC;
/** @type {number} */
GPUBufferUsage.COPY_DST;
/** @type {number} */
GPUBufferUsage.INDEX;
/** @type {number} */
GPUBufferUsage.VERTEX;
/** @type {number} */
GPUBufferUsage.UNIFORM;
/** @type {number} */
GPUBufferUsage.STORAGE;
/** @type {number} */
GPUBufferUsage.INDIRECT;
/** @type {number} */
GPUBufferUsage.QUERY_RESOLVE;

const GPUMapMode = {};
/** @type {number} */
GPUMapMode.READ;
/** @type {number} */
GPUMapMode.WRITE;

const GPUTextureUsage = {};
/** @type {number} */
GPUTextureUsage.COPY_SRC;
/** @type {number} */
GPUTextureUsage.COPY_DST;
/** @type {number} */
GPUTextureUsage.TEXTURE_BINDING;
/** @type {number} */
GPUTextureUsage.STORAGE_BINDING;
/** @type {number} */
GPUTextureUsage.RENDER_ATTACHMENT;

const GPUShaderStage = {};
/** @type {number} */
GPUShaderStage.VERTEX;
/** @type {number} */
GPUShaderStage.FRAGMENT;
/** @type {number} */
GPUShaderStage.COMPUTE;

const GPUColorWrite = {};
/** @type {number} */
GPUColorWrite.RED;
/** @type {number} */
GPUColorWrite.GREEN;
/** @type {number} */
GPUColorWrite.BLUE;
/** @type {number} */
GPUColorWrite.ALPHA;
/** @type {number} */
GPUColorWrite.ALL;

/** @constructor */
function GPUSupportedLimits() {}
/** @type {number} */
GPUSupportedLimits.prototype.maxTextureDimension1D;
/** @type {number} */
GPUSupportedLimits.prototype.maxTextureDimension2D;
/** @type {number} */
GPUSupportedLimits.prototype.maxTextureDimension3D;
/** @type {number} */
GPUSupportedLimits.prototype.maxTextureArrayLayers;
/** @type {number} */
GPUSupportedLimits.prototype.maxBindGroups;
/** @type {number} */
GPUSupportedLimits.prototype.maxBindGroupsPlusVertexBuffers;
/** @type {number} */
GPUSupportedLimits.prototype.maxBindingsPerBindGroup;
/** @type {number} */
GPUSupportedLimits.prototype.maxDynamicUniformBuffersPerPipelineLayout;
/** @type {number} */
GPUSupportedLimits.prototype.maxDynamicStorageBuffersPerPipelineLayout;
/** @type {number} */
GPUSupportedLimits.prototype.maxSampledTexturesPerShaderStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxSamplersPerShaderStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxStorageBuffersPerShaderStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxStorageTexturesPerShaderStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxUniformBuffersPerShaderStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxUniformBufferBindingSize;
/** @type {number} */
GPUSupportedLimits.prototype.maxStorageBufferBindingSize;
/** @type {number} */
GPUSupportedLimits.prototype.minUniformBufferOffsetAlignment;
/** @type {number} */
GPUSupportedLimits.prototype.minStorageBufferOffsetAlignment;
/** @type {number} */
GPUSupportedLimits.prototype.maxVertexBuffers;
/** @type {number} */
GPUSupportedLimits.prototype.maxBufferSize;
/** @type {number} */
GPUSupportedLimits.prototype.maxVertexAttributes;
/** @type {number} */
GPUSupportedLimits.prototype.maxVertexBufferArrayStride;
/** @type {number} */
GPUSupportedLimits.prototype.maxInterStageShaderComponents;
/** @type {number} */
GPUSupportedLimits.prototype.maxInterStageShaderVariables;
/** @type {number} */
GPUSupportedLimits.prototype.maxColorAttachments;
/** @type {number} */
GPUSupportedLimits.prototype.maxColorAttachmentBytesPerSample;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeWorkgroupStorageSize;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeInvocationsPerWorkgroup;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeWorkgroupSizeX;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeWorkgroupSizeY;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeWorkgroupSizeZ;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeWorkgroupsPerDimension;

/** @constructor */
function GPUSupportedFeatures() {}
/** @type {number} */
GPUSupportedFeatures.prototype.size;
/** @return {!Iterable<string>} */
GPUSupportedFeatures.prototype.entries = function() {};
/** @return {!Iterable<string>} */
GPUSupportedFeatures.prototype.keys = function() {};
/** @return {!Iterable<string>} */
GPUSupportedFeatures.prototype.values = function() {};
/** @return {undefined} */
GPUSupportedFeatures.prototype.forEach = function() {};
/** @return {boolean} */
GPUSupportedFeatures.prototype.has = function() {};

/** @constructor */
function WGSLLanguageFeatures() {}
/** @type {number} */
WGSLLanguageFeatures.prototype.size;
/** @return {!Iterable<string>} */
WGSLLanguageFeatures.prototype.entries = function() {};
/** @return {!Iterable<string>} */
WGSLLanguageFeatures.prototype.keys = function() {};
/** @return {!Iterable<string>} */
WGSLLanguageFeatures.prototype.values = function() {};
/** @return {undefined} */
WGSLLanguageFeatures.prototype.forEach = function() {};
/** @return {boolean} */
WGSLLanguageFeatures.prototype.has = function() {};

/** @constructor */
function GPUAdapterInfo() {}
/** @type {string} */
GPUAdapterInfo.prototype.vendor;
/** @type {string} */
GPUAdapterInfo.prototype.architecture;
/** @type {string} */
GPUAdapterInfo.prototype.device;
/** @type {string} */
GPUAdapterInfo.prototype.description;

/** @constructor */
function GPU() {}
/** @return {!Promise<?GPUAdapter>} */
GPU.prototype.requestAdapter = function() {};
/** @return {string} */
GPU.prototype.getPreferredCanvasFormat = function() {};
/** @type {!WGSLLanguageFeatures} */
GPU.prototype.wgslLanguageFeatures;

/** @constructor */
function GPUAdapter() {}
/** @type {!GPUSupportedFeatures} */
GPUAdapter.prototype.features;
/** @type {!GPUSupportedLimits} */
GPUAdapter.prototype.limits;
/** @type {boolean} */
GPUAdapter.prototype.isFallbackAdapter;
/** @return {!Promise<!GPUDevice>} */
GPUAdapter.prototype.requestDevice = function() {};
/** @return {!Promise<!GPUAdapterInfo>} */
GPUAdapter.prototype.requestAdapterInfo = function() {};

/** @constructor */
function GPUDevice() {}
/** @type {string} */
GPUDevice.prototype.label;
/** @type {!GPUSupportedFeatures} */
GPUDevice.prototype.features;
/** @type {!GPUSupportedLimits} */
GPUDevice.prototype.limits;
/** @type {!GPUQueue} */
GPUDevice.prototype.queue;
/** @return {undefined} */
GPUDevice.prototype.destroy = function() {};
/** @return {!GPUBuffer} */
GPUDevice.prototype.createBuffer = function() {};
/** @return {!GPUTexture} */
GPUDevice.prototype.createTexture = function() {};
/** @return {!GPUSampler} */
GPUDevice.prototype.createSampler = function() {};
/** @return {!GPUExternalTexture} */
GPUDevice.prototype.importExternalTexture = function() {};
/** @return {!GPUBindGroupLayout} */
GPUDevice.prototype.createBindGroupLayout = function() {};
/** @return {!GPUPipelineLayout} */
GPUDevice.prototype.createPipelineLayout = function() {};
/** @return {!GPUBindGroup} */
GPUDevice.prototype.createBindGroup = function() {};
/** @return {!GPUShaderModule} */
GPUDevice.prototype.createShaderModule = function() {};
/** @return {!GPUComputePipeline} */
GPUDevice.prototype.createComputePipeline = function() {};
/** @return {!GPURenderPipeline} */
GPUDevice.prototype.createRenderPipeline = function() {};
/** @return {!Promise<!GPUComputePipeline>} */
GPUDevice.prototype.createComputePipelineAsync = function() {};
/** @return {!Promise<!GPURenderPipeline>} */
GPUDevice.prototype.createRenderPipelineAsync = function() {};
/** @return {!GPUCommandEncoder} */
GPUDevice.prototype.createCommandEncoder = function() {};
/** @return {!GPURenderBundleEncoder} */
GPUDevice.prototype.createRenderBundleEncoder = function() {};
/** @return {!GPUQuerySet} */
GPUDevice.prototype.createQuerySet = function() {};
/** @type {!Promise<!GPUDeviceLostInfo>} */
GPUDevice.prototype.lost;
/** @return {undefined} */
GPUDevice.prototype.pushErrorScope = function() {};
/** @return {!Promise<?GPUError>} */
GPUDevice.prototype.popErrorScope = function() {};
/** @type {!Function} */
GPUDevice.prototype.onuncapturederror;

/** @constructor */
function GPUBuffer() {}
/** @type {string} */
GPUBuffer.prototype.label;
/** @type {number} */
GPUBuffer.prototype.size;
/** @type {number} */
GPUBuffer.prototype.usage;
/** @type {string} */
GPUBuffer.prototype.mapState;
/** @return {!Promise<undefined>} */
GPUBuffer.prototype.mapAsync = function() {};
/** @return {!ArrayBuffer} */
GPUBuffer.prototype.getMappedRange = function() {};
/** @return {undefined} */
GPUBuffer.prototype.unmap = function() {};
/** @return {undefined} */
GPUBuffer.prototype.destroy = function() {};

/** @constructor */
function GPUTexture() {}
/** @type {string} */
GPUTexture.prototype.label;
/** @return {!GPUTextureView} */
GPUTexture.prototype.createView = function() {};
/** @return {undefined} */
GPUTexture.prototype.destroy = function() {};
/** @type {number} */
GPUTexture.prototype.width;
/** @type {number} */
GPUTexture.prototype.height;
/** @type {number} */
GPUTexture.prototype.depthOrArrayLayers;
/** @type {number} */
GPUTexture.prototype.mipLevelCount;
/** @type {number} */
GPUTexture.prototype.sampleCount;
/** @type {string} */
GPUTexture.prototype.dimension;
/** @type {string} */
GPUTexture.prototype.format;
/** @type {number} */
GPUTexture.prototype.usage;

/** @constructor */
function GPUTextureView() {}
/** @type {string} */
GPUTextureView.prototype.label;

/** @constructor */
function GPUExternalTexture() {}
/** @type {string} */
GPUExternalTexture.prototype.label;

/** @constructor */
function GPUSampler() {}
/** @type {string} */
GPUSampler.prototype.label;

/** @constructor */
function GPUBindGroupLayout() {}
/** @type {string} */
GPUBindGroupLayout.prototype.label;

/** @constructor */
function GPUBindGroup() {}
/** @type {string} */
GPUBindGroup.prototype.label;

/** @constructor */
function GPUPipelineLayout() {}
/** @type {string} */
GPUPipelineLayout.prototype.label;

/** @constructor */
function GPUShaderModule() {}
/** @type {string} */
GPUShaderModule.prototype.label;
/** @return {!Promise<!GPUCompilationInfo>} */
GPUShaderModule.prototype.getCompilationInfo = function() {};

/** @constructor */
function GPUCompilationMessage() {}
/** @type {string} */
GPUCompilationMessage.prototype.message;
/** @type {string} */
GPUCompilationMessage.prototype.type;
/** @type {number} */
GPUCompilationMessage.prototype.lineNum;
/** @type {number} */
GPUCompilationMessage.prototype.linePos;
/** @type {number} */
GPUCompilationMessage.prototype.offset;
/** @type {number} */
GPUCompilationMessage.prototype.length;

/** @constructor */
function GPUCompilationInfo() {}
/** @type {!Array<!GPUCompilationMessage>} */
GPUCompilationInfo.prototype.messages;

/** @constructor */
function GPUPipelineError() {}
/** @type {string} */
GPUPipelineError.prototype.reason;

/** @constructor */
function GPUComputePipeline() {}
/** @type {string} */
GPUComputePipeline.prototype.label;
/** @return {!GPUBindGroupLayout} */
GPUComputePipeline.prototype.getBindGroupLayout = function() {};

/** @constructor */
function GPURenderPipeline() {}
/** @type {string} */
GPURenderPipeline.prototype.label;
/** @return {!GPUBindGroupLayout} */
GPURenderPipeline.prototype.getBindGroupLayout = function() {};

/** @constructor */
function GPUCommandBuffer() {}
/** @type {string} */
GPUCommandBuffer.prototype.label;

/** @constructor */
function GPUCommandEncoder() {}
/** @type {string} */
GPUCommandEncoder.prototype.label;
/** @return {undefined} */
GPUCommandEncoder.prototype.pushDebugGroup = function() {};
/** @return {undefined} */
GPUCommandEncoder.prototype.popDebugGroup = function() {};
/** @return {undefined} */
GPUCommandEncoder.prototype.insertDebugMarker = function() {};
/** @return {!GPURenderPassEncoder} */
GPUCommandEncoder.prototype.beginRenderPass = function() {};
/** @return {!GPUComputePassEncoder} */
GPUCommandEncoder.prototype.beginComputePass = function() {};
/** @return {undefined} */
GPUCommandEncoder.prototype.copyBufferToBuffer = function() {};
/** @return {undefined} */
GPUCommandEncoder.prototype.copyBufferToTexture = function() {};
/** @return {undefined} */
GPUCommandEncoder.prototype.copyTextureToBuffer = function() {};
/** @return {undefined} */
GPUCommandEncoder.prototype.copyTextureToTexture = function() {};
/** @return {undefined} */
GPUCommandEncoder.prototype.clearBuffer = function() {};
/** @return {undefined} */
GPUCommandEncoder.prototype.resolveQuerySet = function() {};
/** @return {!GPUCommandBuffer} */
GPUCommandEncoder.prototype.finish = function() {};

/** @constructor */
function GPUComputePassEncoder() {}
/** @type {string} */
GPUComputePassEncoder.prototype.label;
/** @return {undefined} */
GPUComputePassEncoder.prototype.pushDebugGroup = function() {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.popDebugGroup = function() {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.insertDebugMarker = function() {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.setBindGroup = function() {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.setBindGroup = function() {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.setPipeline = function() {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.dispatchWorkgroups = function() {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.dispatchWorkgroupsIndirect = function() {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.end = function() {};

/** @constructor */
function GPURenderPassEncoder() {}
/** @type {string} */
GPURenderPassEncoder.prototype.label;
/** @return {undefined} */
GPURenderPassEncoder.prototype.pushDebugGroup = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.popDebugGroup = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.insertDebugMarker = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setBindGroup = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setBindGroup = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setPipeline = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setIndexBuffer = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setVertexBuffer = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.draw = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.drawIndexed = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.drawIndirect = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.drawIndexedIndirect = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setViewport = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setScissorRect = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setBlendConstant = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setStencilReference = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.beginOcclusionQuery = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.endOcclusionQuery = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.executeBundles = function() {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.end = function() {};

/** @constructor */
function GPURenderBundle() {}
/** @type {string} */
GPURenderBundle.prototype.label;

/** @constructor */
function GPURenderBundleEncoder() {}
/** @type {string} */
GPURenderBundleEncoder.prototype.label;
/** @return {undefined} */
GPURenderBundleEncoder.prototype.pushDebugGroup = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.popDebugGroup = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.insertDebugMarker = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.setBindGroup = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.setBindGroup = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.setPipeline = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.setIndexBuffer = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.setVertexBuffer = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.draw = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.drawIndexed = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.drawIndirect = function() {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.drawIndexedIndirect = function() {};
/** @return {!GPURenderBundle} */
GPURenderBundleEncoder.prototype.finish = function() {};

/** @constructor */
function GPUQueue() {}
/** @type {string} */
GPUQueue.prototype.label;
/** @return {undefined} */
GPUQueue.prototype.submit = function() {};
/** @return {!Promise<undefined>} */
GPUQueue.prototype.onSubmittedWorkDone = function() {};
/** @return {undefined} */
GPUQueue.prototype.writeBuffer = function() {};
/** @return {undefined} */
GPUQueue.prototype.writeTexture = function() {};
/** @return {undefined} */
GPUQueue.prototype.copyExternalImageToTexture = function() {};

/** @constructor */
function GPUQuerySet() {}
/** @type {string} */
GPUQuerySet.prototype.label;
/** @return {undefined} */
GPUQuerySet.prototype.destroy = function() {};
/** @type {string} */
GPUQuerySet.prototype.type;
/** @type {number} */
GPUQuerySet.prototype.count;

/** @constructor */
function GPUCanvasContext() {}
/** @type {!HTMLCanvasElement|!OffscreenCanvas} */
GPUCanvasContext.prototype.canvas;
/** @return {undefined} */
GPUCanvasContext.prototype.configure = function() {};
/** @return {undefined} */
GPUCanvasContext.prototype.unconfigure = function() {};
/** @return {!GPUTexture} */
GPUCanvasContext.prototype.getCurrentTexture = function() {};

/** @constructor */
function GPUDeviceLostInfo() {}
/** @type {string} */
GPUDeviceLostInfo.prototype.reason;
/** @type {string} */
GPUDeviceLostInfo.prototype.message;

/** @constructor */
function GPUError() {}
/** @type {string} */
GPUError.prototype.message;

/** @constructor */
function GPUValidationError() {}

/** @constructor */
function GPUOutOfMemoryError() {}

/** @constructor */
function GPUInternalError() {}

/** @constructor */
function GPUUncapturedErrorEvent() {}
/** @type {!GPUError} */
GPUUncapturedErrorEvent.prototype.error;
