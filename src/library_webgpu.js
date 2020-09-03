/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
 * WebGPU support.
 *
 * This file implements the common C header <webgpu/webgpu.h> on top of the
 * browser's native JS WebGPU implementation. This allows applications targeting
 * wgpu-native (https://github.com/gfx-rs/wgpu) or
 * Dawn (https://dawn.googlesource.com/dawn/) to also target the Web with the
 * same graphics API and fairly minimal changes - similar to OpenGL ES 2.0/3.0
 * on WebGL 1.0/2.0.
 */

{{{ (function() {
  // Helper functions for code generation
  global.gpu = {
    makeInitManager: function(type) {
      var mgr = 'this.mgr' + type
      return mgr + ' = ' + mgr + ' || makeManager();';
    },

    makeReferenceRelease: function(type) {
      var s = '';
      s += 'wgpu' + type + 'Reference: function(id) {\n';
      s += '  WebGPU.mgr' + type + '.reference(id);\n'
      s += '},\n';
      s += 'wgpu' + type + 'Release: function(id) {\n';
      s += '  WebGPU.mgr' + type + '.release(id);\n'
      s += '},';
      return s;
    },

    makeU64ToNumber: function(lowName, highName) {
      var ret = '('
      if (ASSERTIONS) {
        ret += 'assert(' + highName + ' < 0x200000), ';
      }
      ret += highName + ' * 0x100000000 + ' + lowName + ')\n'
      return ret;
    },

    makeGetBool: function(struct, offset) {
      // In an actual build, bool seems to be i8. But on the off-chance it's i32, on little-endian
      // this will still work as long as the value of 'true' isn't zero in the lowest byte.
      return '(' + makeGetValue(struct, offset, 'i8') + ' !== 0)';
    },
    makeGetU32: function(struct, offset) {
      return makeGetValue(struct, offset, 'i32', false, true);
    },
    makeGetU64: function(struct, offset) {
      var l = makeGetValue(struct, offset, 'i32', false, true);
      var h = makeGetValue('(' + struct + ' + 4)', offset, 'i32', false, true)
      return h + ' * 0x100000000 + ' + l
    },
    makeCheck: function(str) {
      if (!ASSERTIONS) return '';
      return 'assert(' + str + ');';
    },
    makeCheckDefined: function(name) {
      return this.makeCheck('typeof ' + name + ' !== "undefined"');
    },
    makeCheckDescriptor: function(descriptor) {
      // Assert descriptor is non-null, then that its nextInChain is null.
      // For descriptors that aren't the first in the chain (e.g
      // ShaderModuleSPIRVDescriptor), there is no .nextInChain pointer, but
      // instead a ChainedStruct object: .chain. So we need to check if
      // .chain.nextInChain is null. As long as nextInChain and chain are always
      // the first member in the struct, descriptor.nextInChain and
      // descriptor.chain.nextInChain should have the same offset (0) to the
      // descriptor pointer and we can check it to be null.
      var OffsetOfNextInChainMember = 0;
      return this.makeCheck(descriptor) + this.makeCheck(makeGetValue(descriptor, OffsetOfNextInChainMember, '*') + ' === 0');
    },

    // Must be in sync with webgpu.h.
    PresentMode: {
        Fifo: 2,
    },
    SType: {
        SurfaceDescriptorFromCanvasHTMLSelector: 4,
        ShaderModuleSPIRVDescriptor: 5,
        ShaderModuleWGSLDescriptor: 6,
    },
  };
  return null;
})(); }}}

var LibraryWebGPU = {
  $WebGPU: {
    initManagers: function() {
      if (this["mgrDevice"]) return;

      function makeManager() {
        return {
          objects: {},
          nextId: 1,
          create: function(object, wrapper /* = {} */) {
            wrapper = wrapper || {};

            var id = this.nextId++;
            {{{ gpu.makeCheck("typeof this.objects[id] === 'undefined'") }}}
            wrapper.refcount = 1;
            wrapper.object = object;
            this.objects[id] = wrapper;
            return id;
          },
          get: function(id) {
            if (id === 0) return undefined;
            var o = this.objects[id];
            {{{ gpu.makeCheckDefined('o') }}}
            return o.object;
          },
          reference: function(id) {
            var o = this.objects[id];
            {{{ gpu.makeCheckDefined('o') }}}
            o.refcount++;
          },
          release: function(id) {
            var o = this.objects[id];
            {{{ gpu.makeCheckDefined('o') }}}
            {{{ gpu.makeCheck('o.refcount > 0') }}}
            o.refcount--;
            if (o.refcount <= 0) {
              delete this.objects[id];
            }
          },
        };
      }

      {{{ gpu.makeInitManager('Surface') }}}
      {{{ gpu.makeInitManager('SwapChain') }}}

      this["mgrDevice"] = this["mgrDevice"] || makeManager();
      {{{ gpu.makeInitManager('Queue') }}}
      {{{ gpu.makeInitManager('Fence') }}}

      {{{ gpu.makeInitManager('CommandBuffer') }}}
      {{{ gpu.makeInitManager('CommandEncoder') }}}
      {{{ gpu.makeInitManager('RenderPassEncoder') }}}
      {{{ gpu.makeInitManager('ComputePassEncoder') }}}

      {{{ gpu.makeInitManager('BindGroup') }}}
      {{{ gpu.makeInitManager('Buffer') }}}
      {{{ gpu.makeInitManager('Sampler') }}}
      {{{ gpu.makeInitManager('Texture') }}}
      {{{ gpu.makeInitManager('TextureView') }}}
      {{{ gpu.makeInitManager('QuerySet') }}}

      {{{ gpu.makeInitManager('BindGroupLayout') }}}
      {{{ gpu.makeInitManager('PipelineLayout') }}}
      {{{ gpu.makeInitManager('RenderPipeline') }}}
      {{{ gpu.makeInitManager('ComputePipeline') }}}
      {{{ gpu.makeInitManager('ShaderModule') }}}

      {{{ gpu.makeInitManager('RenderBundleEncoder') }}}
      {{{ gpu.makeInitManager('RenderBundle') }}}
    },

    makeColor: function(ptr) {
      return {
        "r": {{{ makeGetValue('ptr', 0, 'float') }}},
        "g": {{{ makeGetValue('ptr', 4, 'float') }}},
        "b": {{{ makeGetValue('ptr', 8, 'float') }}},
        "a": {{{ makeGetValue('ptr', 12, 'float') }}},
      };
    },

    makeExtent3D: function(ptr) {
      return {
        "width": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.width) }}},
        "height": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.height) }}},
        "depth": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.depth) }}},
      };
    },

    makeOrigin3D: function(ptr) {
      return {
        "x": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.x) }}},
        "y": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.y) }}},
        "z": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.z) }}},
      };
    },

    makeTextureCopyView: function(ptr) {
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      return {
        "texture": this.mgrTexture.get(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUTextureCopyView.texture, '*') }}}),
        "mipLevel": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTextureCopyView.mipLevel, '*') }}},
        "origin": WebGPU.makeOrigin3D(ptr + {{{ C_STRUCTS.WGPUTextureCopyView.origin }}}),
      };
    },

    makeTextureDataLayout: function(ptr) {
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      return {
        "offset": {{{ gpu.makeGetU64('ptr', C_STRUCTS.WGPUTextureDataLayout.offset) }}},
        "bytesPerRow": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTextureDataLayout.bytesPerRow) }}},
        "rowsPerImage": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTextureDataLayout.rowsPerImage) }}},
      };
    },

    makeBufferCopyView: function(ptr) {
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      var layoutPtr = ptr + {{{ C_STRUCTS.WGPUBufferCopyView.layout }}};
      var bufferCopyView = this.makeTextureDataLayout(layoutPtr);
      bufferCopyView["buffer"] = this.mgrBuffer.get(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUBufferCopyView.buffer, '*') }}});
      return bufferCopyView;
    },

    makeProgrammableStageDescriptor: function(ptr) {
      if (ptr === 0) return undefined;
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      return {
        "module": WebGPU.mgrShaderModule.get(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUProgrammableStageDescriptor.module, '*') }}}),
        "entryPoint": UTF8ToString(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUProgrammableStageDescriptor.entryPoint, '*') }}}),
      };
    },

    // maps deviceId to the queueId of the device's defaultQueue
    defaultQueues: {
      0: 0
    },

    // This section is auto-generated:
    // https://dawn.googlesource.com/dawn/+/refs/heads/master/generator/templates/library_webgpu_enum_tables.json
    AddressMode: [
      'repeat',
      'mirror-repeat',
      'clamp-to-edge',
    ],
    BindingType: [
      'uniform-buffer',
      'storage-buffer',
      'readonly-storage-buffer',
      'sampler',
      'comparison-sampler',
      'sampled-texture',
      'readonly-storage-texture',
      'writeonly-storage-texture',
    ],
    BlendFactor: [
      'zero',
      'one',
      'src-color',
      'one-minus-src-color',
      'src-alpha',
      'one-minus-src-alpha',
      'dst-color',
      'one-minus-dst-color',
      'dst-alpha',
      'one-minus-dst-alpha',
      'src-alpha-saturated',
      'blend-color',
      'one-minus-blend-color',
    ],
    BlendOperation: [
      'add',
      'subtract',
      'reverse-subtract',
      'min',
      'max',
    ],
    BufferMapAsyncStatus: [
      'success',
      'error',
      'unknown',
      'device-lost',
    ],
    CompareFunction: [
      undefined,
      'never',
      'less',
      'less-equal',
      'greater',
      'greater-equal',
      'equal',
      'not-equal',
      'always',
    ],
    CullMode: [
      'none',
      'front',
      'back',
    ],
    ErrorFilter: [
      'none',
      'validation',
      'out-of-memory',
    ],
    ErrorType: [
      'no-error',
      'validation',
      'out-of-memory',
      'unknown',
      'device-lost',
    ],
    FenceCompletionStatus: [
      'success',
      'error',
      'unknown',
      'device-lost',
    ],
    FilterMode: [
      'nearest',
      'linear',
    ],
    FrontFace: [
      'ccw',
      'cw',
    ],
    IndexFormat: [
      'uint16',
      'uint32',
    ],
    InputStepMode: [
      'vertex',
      'instance',
    ],
    LoadOp: [
      'clear',
      'load',
    ],
    PipelineStatisticName: [
      'vertex-shader-invocations',
      'clipper-invocations',
      'clipper-primitives-out',
      'fragment-shader-invocations',
      'compute-shader-invocations',
    ],
    PrimitiveTopology: [
      'point-list',
      'line-list',
      'line-strip',
      'triangle-list',
      'triangle-strip',
    ],
    QueryType: [
      'occlusion',
      'pipeline-statistics',
      'timestamp',
    ],
    StencilOperation: [
      'keep',
      'zero',
      'replace',
      'invert',
      'increment-clamp',
      'decrement-clamp',
      'increment-wrap',
      'decrement-wrap',
    ],
    StoreOp: [
      'store',
      'clear',
    ],
    TextureAspect: [
      'all',
      'stencil-only',
      'depth-only',
    ],
    TextureComponentType: [
      'float',
      'sint',
      'uint',
    ],
    TextureDimension: [
      '1d',
      '2d',
      '3d',
    ],
    TextureFormat: [
      undefined,
      'r8unorm',
      'r8snorm',
      'r8uint',
      'r8sint',
      'r16uint',
      'r16sint',
      'r16float',
      'rg8unorm',
      'rg8snorm',
      'rg8uint',
      'rg8sint',
      'r32float',
      'r32uint',
      'r32sint',
      'rg16uint',
      'rg16sint',
      'rg16float',
      'rgba8unorm',
      'rgba8unorm-srgb',
      'rgba8snorm',
      'rgba8uint',
      'rgba8sint',
      'bgra8unorm',
      'bgra8unorm-srgb',
      'rgb10a2unorm',
      'rg11b10float',
      'rg32float',
      'rg32uint',
      'rg32sint',
      'rgba16uint',
      'rgba16sint',
      'rgba16float',
      'rgba32float',
      'rgba32uint',
      'rgba32sint',
      'depth32float',
      'depth24plus',
      'depth24plus-stencil8',
      'bc1-rgba-unorm',
      'bc1-rgba-unorm-srgb',
      'bc2-rgba-unorm',
      'bc2-rgba-unorm-srgb',
      'bc3-rgba-unorm',
      'bc3-rgba-unorm-srgb',
      'bc4-r-unorm',
      'bc4-r-snorm',
      'bc5-rg-unorm',
      'bc5-rg-snorm',
      'bc6h-rgb-ufloat',
      'bc6h-rgb-sfloat',
      'bc7-rgba-unorm',
      'bc7-rgba-unorm-srgb',
    ],
    TextureViewDimension: [
      undefined,
      '1d',
      '2d',
      '2d-array',
      'cube',
      'cube-array',
      '3d',
    ],
    VertexFormat: [
      'uchar2',
      'uchar4',
      'char2',
      'char4',
      'uchar2norm',
      'uchar4norm',
      'char2norm',
      'char4norm',
      'ushort2',
      'ushort4',
      'short2',
      'short4',
      'ushort2norm',
      'ushort4norm',
      'short2norm',
      'short4norm',
      'half2',
      'half4',
      'float',
      'float2',
      'float3',
      'float4',
      'uint',
      'uint2',
      'uint3',
      'uint4',
      'int',
      'int2',
      'int3',
      'int4',
    ],
  },

  // *Reference/*Release

  {{{ gpu.makeReferenceRelease('Surface') }}}
  {{{ gpu.makeReferenceRelease('SwapChain') }}}

  {{{ gpu.makeReferenceRelease('Device') }}}
  {{{ gpu.makeReferenceRelease('Queue') }}}
  {{{ gpu.makeReferenceRelease('Fence') }}}

  {{{ gpu.makeReferenceRelease('CommandBuffer') }}}
  {{{ gpu.makeReferenceRelease('CommandEncoder') }}}
  {{{ gpu.makeReferenceRelease('RenderPassEncoder') }}}
  {{{ gpu.makeReferenceRelease('ComputePassEncoder') }}}

  {{{ gpu.makeReferenceRelease('BindGroup') }}}
  {{{ gpu.makeReferenceRelease('Buffer') }}}
  {{{ gpu.makeReferenceRelease('Sampler') }}}
  {{{ gpu.makeReferenceRelease('Texture') }}}
  {{{ gpu.makeReferenceRelease('TextureView') }}}
  {{{ gpu.makeReferenceRelease('QuerySet') }}}

  {{{ gpu.makeReferenceRelease('BindGroupLayout') }}}
  {{{ gpu.makeReferenceRelease('PipelineLayout') }}}
  {{{ gpu.makeReferenceRelease('RenderPipeline') }}}
  {{{ gpu.makeReferenceRelease('ComputePipeline') }}}
  {{{ gpu.makeReferenceRelease('ShaderModule') }}}

  {{{ gpu.makeReferenceRelease('RenderBundleEncoder') }}}
  {{{ gpu.makeReferenceRelease('RenderBundle') }}}

  // *Destroy

  wgpuBufferDestroy: function(bufferId) { WebGPU.mgrBuffer.get(bufferId)["destroy"](); },
  wgpuTextureDestroy: function(textureId) { WebGPU.mgrTexture.get(textureId)["destroy"](); },
  wgpuQuerySetDestroy: function(querySetId) { WebGPU.mgrQuerySet.get(querySetId)["destroy"](); },

  // wgpuDevice

  wgpuDeviceGetDefaultQueue: function(deviceId) {
    var queueId = WebGPU.defaultQueues[deviceId];
#if ASSERTIONS
    assert(queueId != 0, 'got invalid queue');
#endif
    if (queueId === undefined) {
      var device = WebGPU["mgrDevice"].get(deviceId);
      WebGPU.defaultQueues[deviceId] = WebGPU.mgrQueue.create(device["defaultQueue"]);
      queueId = WebGPU.defaultQueues[deviceId];
    } else {
      WebGPU.mgrQueue.reference(queueId);
    }
    return queueId;
  },

  // wgpuDeviceCreate*

  wgpuDeviceCreateCommandEncoder: function(deviceId, descriptor) {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {
        "label": undefined,
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUCommandEncoderDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }
    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrCommandEncoder.create(device["createCommandEncoder"](desc));
  },

  wgpuDeviceCreateBuffer: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var mappedAtCreation = {{{ gpu.makeGetBool('descriptor', C_STRUCTS.WGPUBufferDescriptor.mappedAtCreation) }}};

    var desc = {
      "label": undefined,
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBufferDescriptor.usage) }}},
      "size": {{{ gpu.makeGetU64('descriptor', C_STRUCTS.WGPUBufferDescriptor.size) }}},
      "mappedAtCreation": mappedAtCreation,
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBufferDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    var id = WebGPU.mgrBuffer.create(device["createBuffer"](desc));
    if (mappedAtCreation) {
      var bufferWrapper = WebGPU.mgrBuffer.objects[id];
      bufferWrapper.mapMode = 2 /* WGPUMapMode_Write */;
      bufferWrapper.onUnmap = [];
    }
    return id;
  },

  wgpuDeviceCreateTexture: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "label": undefined,
      "size": WebGPU.makeExtent3D(descriptor + {{{ C_STRUCTS.WGPUTextureDescriptor.size }}}),
      "mipLevelCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.mipLevelCount) }}},
      "sampleCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.sampleCount) }}},
      "dimension": WebGPU.TextureDimension[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.dimension) }}}],
      "format": WebGPU.TextureFormat[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.format) }}}],
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.usage) }}},
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrTexture.create(device["createTexture"](desc));
  },

  wgpuDeviceCreateSampler: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "label": undefined,
      "addressModeU": WebGPU.AddressMode[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeU) }}}],
      "addressModeV": WebGPU.AddressMode[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeV) }}}],
      "addressModeW": WebGPU.AddressMode[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeW) }}}],
      "magFilter": WebGPU.FilterMode[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.magFilter) }}}],
      "minFilter": WebGPU.FilterMode[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.minFilter) }}}],
      "mipmapFilter": WebGPU.FilterMode[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.mipmapFilter) }}}],
      "lodMinClamp": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.lodMinClamp, 'float') }}},
      "lodMaxClamp": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.lodMaxClamp, 'float') }}},
      "compare": WebGPU.CompareFunction[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.compare) }}}],
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrSampler.create(device["createSampler"](desc));
  },

  wgpuDeviceCreateBindGroupLayout: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      return {
        "binding":
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.binding) }}},
        "visibility":
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.visibility) }}},
        "type": WebGPU.BindingType[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.type) }}}],
        "viewDimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.viewDimension) }}}],
        "textureComponentType": WebGPU.TextureComponentType[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.textureComponentType) }}}],
        "storageTextureFormat": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.storageTextureFormat) }}}],
        "multisampled":
          {{{ gpu.makeGetBool('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.multisampled) }}},
        "hasDynamicOffset":
          {{{ gpu.makeGetBool('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.hasDynamicOffset) }}},
        "minBufferBindingSize":
          {{{ gpu.makeGetU64('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.minBufferBindingSize) }}},
      };
    }

    function makeEntries(count, entriesPtrs) {
      var entries = [];
      for (var i = 0; i < count; ++i) {
        entries.push(makeEntry(entriesPtrs +
            {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.__size__ }}} * i));
      }
      return entries;
    }

    var desc = {
      "entries": makeEntries(
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.entryCount) }}},
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.entries, '*') }}}
      ),
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrBindGroupLayout.create(device["createBindGroupLayout"](desc));
  },

  wgpuDeviceCreateBindGroup: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var bufferId = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.buffer) }}};
      var samplerId = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.sampler) }}};
      var textureViewId = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.textureView) }}};
#if ASSERTIONS
      assert((bufferId != 0) + (samplerId != 0) + (textureViewId != 0) == 1);
#endif

      var binding = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.binding) }}};

      if (bufferId != 0) {
        var size = undefined;

        // Handle WGPU_WHOLE_SIZE.
        var sizePart1 = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.size) }}};
        var sizePart2 = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.size + 4) }}};
        if (sizePart1 != 0xFFFFFFFF || sizePart2 != 0xFFFFFFFF) {
          size = {{{ gpu.makeGetU64('entryPtr', C_STRUCTS.WGPUBindGroupEntry.size) }}};
        }

        return {
          "binding": binding,
          "resource": {
            "buffer": WebGPU.mgrBuffer.get(bufferId),
            "offset": {{{ gpu.makeGetU64('entryPtr', C_STRUCTS.WGPUBindGroupEntry.offset) }}},
            "size": size,
          },
        };
      } else if (samplerId != 0) {
        return {
          "binding": binding,
          "resource": WebGPU.mgrSampler.get(samplerId),
        };
      } else {
        return {
          "binding": binding,
          "resource": WebGPU.mgrTextureView.get(textureViewId),
        };
      }
    }

    function makeEntries(count, entriesPtrs) {
      var entries = [];
      for (var i = 0; i < count; ++i) {
        entries.push(makeEntry(entriesPtrs +
            {{{C_STRUCTS.WGPUBindGroupEntry.__size__}}} * i));
      }
      return entries;
    }

    var desc = {
      "label": undefined,
      "layout": WebGPU.mgrBindGroupLayout.get(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.layout, '*') }}}),
      "entries": makeEntries(
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.entryCount) }}},
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.entries, '*') }}}
      ),
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrBindGroup.create(device["createBindGroup"](desc));
  },

  wgpuDeviceCreatePipelineLayout: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var bglCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.bindGroupLayoutCount) }}};
    var bglPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.bindGroupLayouts, '*') }}};
    var bgls = [];
    for (var i = 0; i < bglCount; ++i) {
      bgls.push(WebGPU.mgrBindGroupLayout.get(
        {{{ makeGetValue('bglPtr', '4 * i', '*') }}}));
    }
    var desc = {
      "label": undefined,
      "bindGroupLayouts": bgls,
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrPipelineLayout.create(device["createPipelineLayout"](desc));
  },

  wgpuDeviceCreateQuerySet: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var pipelineStatistics;
    var pipelineStatisticsCount =
      {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.pipelineStatisticsCount) }}};
    if (pipelineStatisticsCount) {
      var pipelineStatisticsPtr =
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.pipelineStatistics, '*') }}};
      pipelineStatistics = [];
      for (var i = 0; i < pipelineStatisticsCount; ++i) {
        pipelineStatistics.push(WebGPU.PipelineStatisticName[
          {{{ gpu.makeGetU32('pipelineStatisticsPtr', '4 * i') }}}]);
      }
    }

    var desc = {
      "type": WebGPU.QueryType[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.type) }}}],
      "count": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.count) }}},
      "pipelineStatistics": pipelineStatistics,
    };

    var device = WebGPU["mgrDevice"].get(deviceId);
    device["createQuerySet"](descriptor);
  },

  wgpuDeviceCreateComputePipeline: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "label": undefined,
      "layout":  WebGPU.mgrPipelineLayout.get(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePipelineDescriptor.layout, '*') }}}),
      "computeStage": WebGPU.makeProgrammableStageDescriptor(
        descriptor + {{{ C_STRUCTS.WGPUComputePipelineDescriptor.computeStage }}}),
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePipelineDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrComputePipeline.create(device["createComputePipeline"](desc));
  },

  wgpuDeviceCreateRenderPipeline: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeRasterizationState(rsPtr) {
      if (rsPtr === 0) return undefined;
      {{{ gpu.makeCheckDescriptor('rsPtr') }}}
      return {
        "frontFace": WebGPU.FrontFace[
          {{{ gpu.makeGetU32('rsPtr', C_STRUCTS.WGPURasterizationStateDescriptor.frontFace) }}}],
        "cullMode": WebGPU.CullMode[
          {{{ gpu.makeGetU32('rsPtr', C_STRUCTS.WGPURasterizationStateDescriptor.cullMode) }}}],
      };
    }

    function makeBlendDescriptor(bdPtr) {
      if (bdPtr === 0) return undefined;
      return {
        "operation": WebGPU.BlendOperation[
          {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendDescriptor.operation) }}}],
        "srcFactor": WebGPU.BlendFactor[
          {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendDescriptor.srcFactor) }}}],
        "dstFactor": WebGPU.BlendFactor[
          {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendDescriptor.dstFactor) }}}],
      };
    }

    function makeColorState(csPtr) {
      {{{ gpu.makeCheckDescriptor('csPtr') }}}
      return {
        "format": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('csPtr', C_STRUCTS.WGPUColorStateDescriptor.format) }}}],
        "alphaBlend": makeBlendDescriptor(csPtr + {{{ C_STRUCTS.WGPUColorStateDescriptor.alphaBlend }}}),
        "colorBlend": makeBlendDescriptor(csPtr + {{{ C_STRUCTS.WGPUColorStateDescriptor.colorBlend }}}),
        "writeMask": {{{ gpu.makeGetU32('csPtr', C_STRUCTS.WGPUColorStateDescriptor.writeMask) }}},
      };
    }

    function makeColorStates(count, csPtr) {
      if (count === 0) return undefined;

      var states = [];
      for (var i = 0; i < count; ++i) {
        states.push(makeColorState(csPtr + {{{ C_STRUCTS.WGPUColorStateDescriptor.__size__ }}} * i));
      }
      return states;
    }

    function makeStencilStateFace(ssfPtr) {
      {{{ gpu.makeCheck('ssfPtr') }}}
      return {
        "compare": WebGPU.CompareFunction[
          {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilStateFaceDescriptor.compare) }}}],
        "failOp": WebGPU.StencilOperation[
          {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilStateFaceDescriptor.failOp) }}}],
        "depthFailOp": WebGPU.StencilOperation[
          {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilStateFaceDescriptor.depthFailOp) }}}],
        "passOp": WebGPU.StencilOperation[
          {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilStateFaceDescriptor.passOp) }}}],
      };
    }

    function makeDepthStencilState(dssPtr) {
      if (dssPtr === 0) return undefined;

      {{{ gpu.makeCheck('dssPtr') }}}
      return {
        "format": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilStateDescriptor.format) }}}],
        "depthWriteEnabled": {{{ gpu.makeGetBool('dssPtr', C_STRUCTS.WGPUDepthStencilStateDescriptor.depthWriteEnabled) }}},
        "depthCompare": WebGPU.CompareFunction[
          {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilStateDescriptor.depthCompare) }}}],
        "stencilFront": makeStencilStateFace(dssPtr + {{{ C_STRUCTS.WGPUDepthStencilStateDescriptor.stencilFront }}}),
        "stencilBack": makeStencilStateFace(dssPtr + {{{ C_STRUCTS.WGPUDepthStencilStateDescriptor.stencilBack }}}),
        "stencilReadMask": {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilStateDescriptor.stencilReadMask) }}},
        "stencilWriteMask": {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilStateDescriptor.stencilWriteMask) }}},
      };
    }

    function makeVertexAttribute(vaPtr) {
      {{{ gpu.makeCheck('vaPtr') }}}
      return {
        "format": WebGPU.VertexFormat[
          {{{ gpu.makeGetU32('vaPtr', C_STRUCTS.WGPUVertexAttributeDescriptor.format) }}}],
        "offset": {{{ gpu.makeGetU64('vaPtr', C_STRUCTS.WGPUVertexAttributeDescriptor.offset) }}},
        "shaderLocation": {{{ gpu.makeGetU32('vaPtr', C_STRUCTS.WGPUVertexAttributeDescriptor.shaderLocation) }}},
      };
    }

    function makeVertexAttributes(count, vaArrayPtr) {
      var vas = [];
      for (var i = 0; i < count; ++i) {
        vas.push(makeVertexAttribute(vaArrayPtr + i * {{{ C_STRUCTS.WGPUVertexAttributeDescriptor.__size__ }}}));
      }
      return vas;
    }

    function makeVertexBuffer(vbPtr) {
      if (vbPtr === 0) return undefined;

      return {
        "arrayStride": {{{ gpu.makeGetU64('vbPtr', C_STRUCTS.WGPUVertexBufferLayoutDescriptor.arrayStride) }}},
        "stepMode": WebGPU.InputStepMode[
          {{{ gpu.makeGetU32('vbPtr', C_STRUCTS.WGPUVertexBufferLayoutDescriptor.stepMode) }}}],
        "attributes": makeVertexAttributes(
          {{{ gpu.makeGetU32('vbPtr', C_STRUCTS.WGPUVertexBufferLayoutDescriptor.attributeCount) }}},
          {{{ makeGetValue('vbPtr', C_STRUCTS.WGPUVertexBufferLayoutDescriptor.attributes, '*') }}}),
      };
    }

    function makeVertexBuffers(count, vbArrayPtr) {
      if (count === 0) return undefined;

      var vbs = [];
      for (var i = 0; i < count; ++i) {
        vbs.push(makeVertexBuffer(vbArrayPtr + i * {{{ C_STRUCTS.WGPUVertexBufferLayoutDescriptor.__size__ }}}));
      }
      return vbs;
    }

    function makeVertexState(viPtr) {
      if (viPtr === 0) return undefined;
      {{{ gpu.makeCheckDescriptor('viPtr') }}}
      return {
        "indexFormat": WebGPU.IndexFormat[
          {{{ gpu.makeGetU32('viPtr', C_STRUCTS.WGPUVertexStateDescriptor.indexFormat) }}}],
        "vertexBuffers": makeVertexBuffers(
          {{{ gpu.makeGetU32('viPtr', C_STRUCTS.WGPUVertexStateDescriptor.vertexBufferCount) }}},
          {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexStateDescriptor.vertexBuffers, '*') }}}),
      };
    }

    var desc = {
      "label": undefined,
      "layout": WebGPU.mgrPipelineLayout.get(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.layout, '*') }}}),
      "vertexStage": WebGPU.makeProgrammableStageDescriptor(
        descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.vertexStage }}}),
      "fragmentStage": WebGPU.makeProgrammableStageDescriptor(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.fragmentStage, '*') }}}),
      "primitiveTopology": WebGPU.PrimitiveTopology[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.primitiveTopology) }}}],
      "rasterizationState": makeRasterizationState(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.rasterizationState, '*') }}}),
      "colorStates": makeColorStates(
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.colorStateCount) }}},
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.colorStates, '*') }}}),
      "depthStencilState": makeDepthStencilState(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.depthStencilState, '*') }}}),
      "vertexState": makeVertexState(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.vertexState, '*') }}}),
      "sampleCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.sampleCount) }}},
      "sampleMask": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.sampleMask) }}},
      "alphaToCoverageEnabled": {{{ gpu.makeGetBool('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.alphaToCoverageEnabled) }}},
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrRenderPipeline.create(device["createRenderPipeline"](desc));
  },

  wgpuDeviceCreateShaderModule: function(deviceId, descriptor) {
    {{{ gpu.makeCheck('descriptor') }}}
    var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUShaderModuleDescriptor.nextInChain, '*') }}};
#if ASSERTIONS
    assert(nextInChainPtr !== 0);
#endif
    var sType = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}};
#if ASSERTIONS
    assert(sType === {{{ gpu.SType.ShaderModuleSPIRVDescriptor }}}
        || sType === {{{ gpu.SType.ShaderModuleWGSLDescriptor }}});
#endif
    var desc = {
      "label": undefined,
      "code": "",
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUShaderModuleDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    if (sType === {{{ gpu.SType.ShaderModuleSPIRVDescriptor }}}) {
      var count = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUShaderModuleSPIRVDescriptor.codeSize) }}};
      var start = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUShaderModuleSPIRVDescriptor.code, '*') }}};
#if USE_PTHREADS
      // Chrome can't currently handle a SharedArrayBuffer view here, so make a copy.
      desc["code"] = HEAPU32.slice(start >> 2, (start >> 2) + count);
#else
      desc["code"] = HEAPU32.subarray(start >> 2, (start >> 2) + count);
#endif
    } else if (sType === {{{ gpu.SType.ShaderModuleWGSLDescriptor }}}) {
      var sourcePtr = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUShaderModuleWGSLDescriptor.source, '*') }}};
      if (sourcePtr) {
        desc["code"] = UTF8ToString(sourcePtr);
      }
    }

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrShaderModule.create(device["createShaderModule"](desc));
  },

#if MINIMAL_RUNTIME
  wgpuDeviceSetUncapturedErrorCallback__deps: ['$allocateUTF8'],
#endif
  wgpuDeviceSetUncapturedErrorCallback: function(deviceId, callback, userdata) {
    var device = WebGPU["mgrDevice"].get(deviceId);
    device["onuncapturederror"] = function(ev) {
      // WGPUErrorType type, const char* message, void* userdata
      var Validation = 0x00000001;
      var OutOfMemory = 0x00000002;
      var type;
#if ASSERTIONS
      assert(typeof GPUValidationError !== 'undefined');
      assert(typeof GPUOutOfMemoryError !== 'undefined');
#endif
      if (ev.error instanceof GPUValidationError) type = Validation;
      else if (ev.error instanceof GPUOutOfMemoryError) type = OutOfMemory;
      var messagePtr = allocateUTF8(ev.error.message);
      {{{ makeDynCall('viii', 'callback') }}}(type, messagePtr, userdata);
      _free(messagePtr);
    };
  },

  // wgpuFence

  wgpuFenceOnCompletion: function(fenceId, {{{ defineI64Param('completionValue') }}}, callback, userdata) {
    {{{ receiveI64ParamAsI32s('completionValue') }}}
    var fence = WebGPU.mgrFence.get(fenceId);
    var completionValue = {{{ gpu.makeU64ToNumber('completionValue_low', 'completionValue_high') }}};

    fence.onCompletion(completionValue).then(function() {
      {{{ makeDynCall('vii', 'callback') }}}(0 /* WEBGPU_FENCE_COMPLETION_STATUS_SUCCESS */, userdata);
    }, function() {
      {{{ makeDynCall('vii', 'callback') }}}(1 /* WEBGPU_FENCE_COMPLETION_STATUS_ERROR */, userdata);
    });
  },

  // wgpuQueue

  wgpuQueueCreateFence: function(queueId, descriptor) {
    var queue = WebGPU.mgrQueue.get(queueId);

    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {
        "label": UTF8ToString(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPUFenceDescriptor.label, '*') }}}),
        "initialValue": {{{ gpu.makeGetU64('descriptor', C_STRUCTS.WGPUFenceDescriptor.initialValue) }}},
      };
    }

    return WebGPU.mgrFence.create(queue.createFence(desc));
  },

  wgpuQueueSignal: function(queueId, fenceId, {{{ defineI64Param('signalValue') }}}) {
    {{{ receiveI64ParamAsI32s('signalValue') }}}
    var queue = WebGPU.mgrQueue.get(queueId);
    var fence = WebGPU.mgrFence.get(fenceId);
    var signalValue = {{{ gpu.makeU64ToNumber('signalValue_low', 'signalValue_high') }}};
    queue["signal"](fence, signalValue);
  },

  wgpuQueueSubmit: function(queueId, commandCount, commands) {
#if ASSERTIONS
    assert(commands % 4 === 0);
#endif
    var queue = WebGPU.mgrQueue.get(queueId);
    var cmds = Array.from(HEAP32.subarray(commands >> 2, (commands >> 2) + commandCount),
      function(id) { return WebGPU.mgrCommandBuffer.get(id); });
    queue["submit"](cmds);
  },

  wgpuQueueWriteBuffer: function(queueId,
      bufferId, {{{ defineI64Param('bufferOffset') }}}, data, size) {
    {{{ receiveI64ParamAsI32s('bufferOffset') }}}

    var queue = WebGPU.mgrQueue.get(queueId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    var bufferOffset = {{{ gpu.makeU64ToNumber('bufferOffset_low', 'bufferOffset_high') }}};
    queue["writeBuffer"](buffer, bufferOffset, HEAPU8, data, size);
  },

  wgpuQueueWriteTexture: function(queueId,
      destinationPtr, data, _dataSize, dataLayoutPtr, writeSizePtr) {
    var queue = WebGPU.mgrQueue.get(queueId);

    var destination = WebGPU.makeTextureCopyView(destinationPtr);
    var dataLayout = WebGPU.makeTextureDataLayout(dataLayoutPtr);
    dataLayout["offset"] += data;
    var writeSize = WebGPU.makeExtent3D(writeSizePtr);
    queue["writeTexture"](destination, HEAPU8, dataLayout, writeSize);
  },

  // wgpuCommandEncoder

  wgpuDeviceCreateRenderBundleEncoder: function(deviceId, descriptor) {
    {{{ gpu.makeCheck('descriptor') }}}

    function makeRenderBundleEncoderDescriptor(descriptor) {
      {{{ gpu.makeCheck('descriptor') }}}

      function makeColorFormats(count, formatsPtr) {
        var formats = [];
        for (var i = 0; i < count; ++i, formatsPtr += 4) {
          formats.push(WebGPU.TextureFormat[{{{ gpu.makeGetU32('formatsPtr', 0) }}}]);
        }
        return formats;
      }

      var desc = {
        "label": undefined,
        "colorFormats": makeColorFormats(
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.colorFormatsCount) }}},
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.colorFormats, '*') }}}),
        "depthStencilFormat": WebGPU.TextureFormat[{{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.depthStencilFormat) }}}],
        "sampleCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.sampleCount) }}},
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
      return desc;
    }

    var desc = makeRenderBundleEncoderDescriptor(descriptor);
    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrRenderBundleEncoder.create(device["createRenderBundleEncoder"](desc));
  },

  wgpuCommandEncoderBeginComputePass: function(encoderId, descriptor) {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {};
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePassDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    return WebGPU.mgrComputePassEncoder.create(commandEncoder["beginComputePass"](desc));
  },

  wgpuCommandEncoderBeginRenderPass: function(encoderId, descriptor) {
    {{{ gpu.makeCheck('descriptor') }}}

    function makeColorAttachment(caPtr) {
      var loadValue = WebGPU.LoadOp[
          {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachmentDescriptor.loadOp) }}}];
      if (loadValue === 'clear') {
        loadValue = WebGPU.makeColor(caPtr + {{{ C_STRUCTS.WGPURenderPassColorAttachmentDescriptor.clearColor }}});
      }

      return {
        "attachment": WebGPU.mgrTextureView.get(
          {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachmentDescriptor.attachment) }}}),
        "resolveTarget": WebGPU.mgrTextureView.get(
          {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachmentDescriptor.resolveTarget) }}}),
        "storeOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachmentDescriptor.storeOp) }}}],
        "loadValue": loadValue,
      };
    }

    function makeColorAttachments(count, caPtr) {
      var attachments = [];
      for (var i = 0; i < count; ++i) {
        attachments.push(makeColorAttachment(caPtr + {{{ C_STRUCTS.WGPURenderPassColorAttachmentDescriptor.__size__ }}} * i));
      }
      return attachments;
    }

    function makeDepthStencilAttachment(dsaPtr) {
      if (dsaPtr === 0) return undefined;

      var depthLoadValue = WebGPU.LoadOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachmentDescriptor.depthLoadOp) }}}];
      if (depthLoadValue === 'clear') {
        depthLoadValue = {{{ makeGetValue('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachmentDescriptor.clearDepth, 'float') }}};
      }

      var stencilLoadValue = WebGPU.LoadOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachmentDescriptor.stencilLoadOp) }}}];
      if (stencilLoadValue === 'clear') {
        stencilLoadValue = {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachmentDescriptor.clearStencil) }}};
      }

      return {
        "attachment": WebGPU.mgrTextureView.get(
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachmentDescriptor.attachment) }}}),
        "depthStoreOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachmentDescriptor.depthStoreOp) }}}],
        "depthLoadValue": depthLoadValue,
        "depthReadOnly": {{{ gpu.makeGetBool('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachmentDescriptor.depthReadOnly) }}},
        "stencilStoreOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachmentDescriptor.stencilStoreOp) }}}],
        "stencilLoadValue": stencilLoadValue,
        "stencilReadOnly": {{{ gpu.makeGetBool('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachmentDescriptor.stencilReadOnly) }}},
      };
    }

    function makeRenderPassDescriptor(descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      var desc = {
        "label": undefined,
        "colorAttachments": makeColorAttachments(
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderPassDescriptor.colorAttachmentCount) }}},
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.colorAttachments, '*') }}}),
        "depthStencilAttachment": makeDepthStencilAttachment(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.depthStencilAttachment, '*') }}}),
        "occlusionQuerySet": WebGPU.mgrQuerySet.get(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.occlusionQuerySet, '*') }}}),
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
      return desc;
    }

    var desc = makeRenderPassDescriptor(descriptor);

    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    return WebGPU.mgrRenderPassEncoder.create(commandEncoder["beginRenderPass"](desc));
  },

  wgpuCommandEncoderCopyBufferToBuffer: function(encoderId, srcId, {{{ defineI64Param('srcOffset') }}}, dstId, {{{ defineI64Param('dstOffset') }}}, {{{ defineI64Param('size') }}}) {
    {{{ receiveI64ParamAsI32s('srcOffset') }}}
    {{{ receiveI64ParamAsI32s('dstOffset') }}}
    {{{ receiveI64ParamAsI32s('size') }}}
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var src = WebGPU.mgrBuffer.get(srcId);
    var dst = WebGPU.mgrBuffer.get(dstId);
    commandEncoder["copyBufferToBuffer"](
      src, {{{ gpu.makeU64ToNumber('srcOffset_low', 'srcOffset_high') }}},
      dst, {{{ gpu.makeU64ToNumber('dstOffset_low', 'dstOffset_high') }}},
      {{{ gpu.makeU64ToNumber('size_low', 'size_high') }}});
  },

  wgpuCommandEncoderCopyBufferToTexture: function(encoderId, srcPtr, dstPtr, copySizePtr) {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder["copyBufferToTexture"](
      WebGPU.makeBufferCopyView(srcPtr), WebGPU.makeTextureCopyView(dstPtr), copySize);
  },

  wgpuCommandEncoderCopyTextureToBuffer: function(encoderId, srcPtr, dstPtr, copySizePtr) {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder["copyTextureToBuffer"](
      WebGPU.makeTextureCopyView(srcPtr), WebGPU.makeBufferCopyView(dstPtr), copySize);
  },

  wgpuCommandEncoderCopyTextureToTexture: function(encoderId, srcPtr, dstPtr, copySizePtr) {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder["copyTextureToTexture"](
      WebGPU.makeTextureCopyView(srcPtr), WebGPU.makeTextureCopyView(dstPtr), copySize);
  },

  wgpuCommandEncoderResolveQuerySet: function(encoderId, querySetId, firstQuery, queryCount,
      destinationId, {{{ defineI64Param('destinationOffset') }}}) {
    {{{ receiveI64ParamAsI32s('destinationOffset') }}}
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    var destination = WebGPU.mgrBuffer.get(destinationId);
    var destinationOffset = {{{ gpu.makeU64ToNumber('destinationOffset_low', 'destinationOffset_high') }}};

    commandEncoder["resolveQuerySet"](querySet, firstQuery, queryCount, destination, destinationOffset);
  },

  wgpuCommandEncoderWriteTimestamp: function(encoderId, querySetId, queryIndex) {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    commandEncoder["writeTimestamp"](querySet, queryIndex);
  },

  wgpuCommandEncoderPushDebugGroup: function(encoderId, groupLabelPtr) {
    var encoder = WebGPU.mgrCommandEncoder.get(encoderId);
    encoder["pushDebugGroup"](UTF8ToString(groupLabelPtr));
  },
  wgpuCommandEncoderPopDebugGroup: function(encoderId) {
    var encoder = WebGPU.mgrCommandEncoder.get(encoderId);
    encoder["popDebugGroup"]();
  },
  wgpuCommandEncoderInsertDebugMarker: function(encoderId, markerLabelPtr) {
    var encoder = WebGPU.mgrCommandEncoder.get(encoderId);
    encoder["insertDebugMarker"](UTF8ToString(markerLabelPtr));
  },

  wgpuCommandEncoderFinish: function(encoderId) {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    return WebGPU.mgrCommandBuffer.create(commandEncoder["finish"]());
  },

  // wgpuComputePipeline

  wgpuComputePipelineGetBindGroupLayout(pipelineId, groupIndex) {
    var pipeline = WebGPU.mgrComputePipeline.get(pipelineId);
    return WebGPU.mgrBindGroupLayout.create(pipeline["getBindGroupLayout"](groupIndex));
  },

  // wgpuRenderPipeline

  wgpuRenderPipelineGetBindGroupLayout(pipelineId, groupIndex) {
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    return WebGPU.mgrBindGroupLayout.create(pipeline["getBindGroupLayout"](groupIndex));
  },

  // wgpuBuffer

  wgpuBufferGetConstMappedRange: function(bufferId, offset, size) {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];

    // TODO: if the sentinel value becomes WGPU_WHOLE_SIZE instead of 0, update this.
    if (size === 0) size = undefined;

    var mapped;
    try {
      mapped = bufferWrapper.object["getMappedRange"](offset, size);
    } catch (ex) {
#if ASSERTIONS
      err("wgpuBufferGetConstMappedRange(" + offset + ", " + size + ") failed: " + ex);
#endif
      // TODO(kainino0x): Somehow inject a validation error?
      return 0;
    }

    var data = _malloc(mapped.byteLength);
    HEAPU8.set(new Uint8Array(mapped), data);
    bufferWrapper.onUnmap.push(function() {
      _free(data);
    });
    return data;
  },

  wgpuBufferGetMappedRange: function(bufferId, offset, size) {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];

    // TODO: if the sentinel value becomes WGPU_WHOLE_SIZE instead of 0, update this.
    if (size === 0) size = undefined;

    if (bufferWrapper.mapMode !== 2 /* WGPUMapMode_Write */) {
#if ASSERTIONS
      abort("GetMappedRange called, but buffer not mapped for writing");
#endif
      // TODO(kainino0x): Somehow inject a validation error?
      return 0;
    }

    var mapped;
    try {
      mapped = bufferWrapper.object["getMappedRange"](offset, size);
    } catch (ex) {
#if ASSERTIONS
      err("wgpuBufferGetMappedRange(" + offset + ", " + size + ") failed: " + ex);
#endif
      // TODO(kainino0x): Somehow inject a validation error?
      return 0;
    }

    var data = _malloc(mapped.byteLength);
    HEAPU8.fill(0, data, mapped.byteLength);
    bufferWrapper.onUnmap.push(function() {
      new Uint8Array(mapped).set(HEAPU8.subarray(data, data + mapped.byteLength));
      _free(data);
    });
    return data;
  },

  wgpuBufferMapAsync: function(bufferId, mode, offset, size, callback, userdata) {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    bufferWrapper.mapMode = mode;
    bufferWrapper.onUnmap = [];
    var buffer = bufferWrapper.object;

    // `callback` takes (WGPUBufferMapAsyncStatus status, void * userdata)

    buffer["mapAsync"](mode, offset, size).then(function() {
      {{{ makeDynCall('vii', 'callback') }}}(0 /* WGPUBufferMapAsyncStatus_Success */, userdata);
    }, function() {
      // TODO(kainino0x): Figure out how to pick other error status values.
      {{{ makeDynCall('vii', 'callback') }}}(1 /* WGPUBufferMapAsyncStatus_Error */, userdata);
    });
  },

  wgpuBufferUnmap: function(bufferId) {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];

    if (!bufferWrapper.onUnmap) {
      // Already unmapped
      return;
    }

    for (var i = 0; i < bufferWrapper.onUnmap.length; ++i) {
      bufferWrapper.onUnmap[i]();
    }
    bufferWrapper.onUnmap = undefined;

    bufferWrapper.object["unmap"]();
  },

  // wgpuTexture

  wgpuTextureCreateView: function(textureId, descriptor) {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {
        "format": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.format) }}}],
        "dimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.dimension) }}}],
        "baseMipLevel": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.baseMipLevel) }}},
        "mipLevelCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.mipLevelCount) }}},
        "baseArrayLayer": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.baseArrayLayer) }}},
        "arrayLayerCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.arrayLayerCount) }}},
        "aspect": WebGPU.TextureAspect[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.aspect) }}}],
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }

    var texture = WebGPU.mgrTexture.get(textureId);
    return WebGPU.mgrTextureView.create(texture["createView"](desc));
  },

  // wgpuComputePass

  wgpuComputePassEncoderSetBindGroup: function(passId, groupIndex, groupId, dynamicOffsetCount, dynamicOffsetsPtr) {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    var group = WebGPU.mgrBindGroup.get(groupId);
    if (dynamicOffsetCount == 0) {
      pass["setBindGroup"](groupIndex, group);
    } else {
      var offsets = [];
      for (var i = 0; i < dynamicOffsetCount; i++, dynamicOffsetsPtr += 4) {
        offsets.push({{{ gpu.makeGetU32('dynamicOffsetsPtr', 0) }}});
      }
      pass["setBindGroup"](groupIndex, group, offsets);
    }
  },
  wgpuComputePassEncoderSetPipeline: function(passId, pipelineId) {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    var pipeline = WebGPU.mgrComputePipeline.get(pipelineId);
    pass["setPipeline"](pipeline);
  },

  wgpuComputePassEncoderDispatch: function(passId, x, y, z) {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    pass["dispatch"](x, y, z);
  },
  wgpuComputePassEncoderDispatchIndirect: function(passId, indirectBufferId, {{{ defineI64Param('indirectOffset') }}}) {
    {{{ receiveI64ParamAsI32s('indirectOffset') }}}
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var indirectOffset = {{{ gpu.makeU64ToNumber('indirectOffset_low', 'indirectOffset_high') }}};
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    pass["dispatchIndirect"](indirectBuffer, indirectOffset);
  },

  wgpuComputePassEncoderBeginPipelineStatisticsQuery: function(passId, querySetId, queryIndex) {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    pass["beginPipelineStatisticsQuery"](querySet, queryIndex);
  },
  wgpuComputePassEncoderEndPipelineStatisticsQuery: function(passId) {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    pass["endPipelineStatisticsQuery"]();
  },

  wgpuComputePassEncoderWriteTimestamp: function(encoderId, querySetId, queryIndex) {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    pass["writeTimestamp"](querySet, queryIndex);
  },

  wgpuComputePassEncoderPushDebugGroup: function(encoderId, groupLabelPtr) {
    var encoder = WebGPU.mgrComputePassEncoder.get(encoderId);
    encoder["pushDebugGroup"](UTF8ToString(groupLabelPtr));
  },
  wgpuComputePassEncoderPopDebugGroup: function(encoderId) {
    var encoder = WebGPU.mgrComputePassEncoder.get(encoderId);
    encoder["popDebugGroup"]();
  },
  wgpuComputePassEncoderInsertDebugMarker: function(encoderId, markerLabelPtr) {
    var encoder = WebGPU.mgrComputePassEncoder.get(encoderId);
    encoder["insertDebugMarker"](UTF8ToString(markerLabelPtr));
  },

  wgpuComputePassEncoderEndPass: function(passId) {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    pass["endPass"]();
  },

  // wgpuRenderPass

  wgpuRenderPassEncoderSetBindGroup: function(passId, groupIndex, groupId, dynamicOffsetCount, dynamicOffsetsPtr) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var group = WebGPU.mgrBindGroup.get(groupId);
    if (dynamicOffsetCount == 0) {
      pass["setBindGroup"](groupIndex, group);
    } else {
      var offsets = [];
      for (var i = 0; i < dynamicOffsetCount; i++, dynamicOffsetsPtr += 4) {
        offsets.push({{{ gpu.makeGetU32('dynamicOffsetsPtr', 0) }}});
      }
      pass["setBindGroup"](groupIndex, group, offsets);
    }
  },
  wgpuRenderPassEncoderSetBlendColor: function(passId, colorPtr) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var color = WebGPU.makeColor(colorPtr);
    pass["setBlendColor"](color);
  },
  wgpuRenderPassEncoderSetIndexBuffer: function(passId, bufferId, offset, size) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    pass["setIndexBuffer"](buffer, offset, size);
  },
  wgpuRenderPassEncoderSetPipeline: function(passId, pipelineId) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    pass["setPipeline"](pipeline);
  },
  wgpuRenderPassEncoderSetScissorRect: function(passId, x, y, w, h) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["setScissorRect"](x, y, w, h);
  },
  wgpuRenderPassEncoderSetViewport: function(passId, x, y, w, h, minDepth, maxDepth) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["setViewport"](x, y, w, h, minDepth, maxDepth);
  },
  wgpuRenderPassEncoderSetStencilReference: function(passId, reference) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["setStencilReference"](reference);
  },
  wgpuRenderPassEncoderSetVertexBuffer: function(passId, slot, bufferId, offset, size) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["setVertexBuffer"](slot, WebGPU.mgrBuffer.get(bufferId), offset, size);
  },

  wgpuRenderPassEncoderDraw: function(passId, vertexCount, instanceCount, firstVertex, firstInstance) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["draw"](vertexCount, instanceCount, firstVertex, firstInstance);
  },
  wgpuRenderPassEncoderDrawIndexed: function(passId, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["drawIndexed"](indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },
  wgpuRenderPassEncoderDrawIndirect: function(passId, indirectBufferId, {{{ defineI64Param('indirectOffset') }}}) {
    {{{ receiveI64ParamAsI32s('indirectOffset') }}}
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var indirectOffset = {{{ gpu.makeU64ToNumber('indirectOffset_low', 'indirectOffset_high') }}};
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["drawIndirect"](indirectBuffer, indirectOffset);
  },
  wgpuRenderPassEncoderDrawIndexedIndirect: function(passId, indirectBufferId, {{{ defineI64Param('indirectOffset') }}}) {
    {{{ receiveI64ParamAsI32s('indirectOffset') }}}
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var indirectOffset = {{{ gpu.makeU64ToNumber('indirectOffset_low', 'indirectOffset_high') }}};
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["drawIndexedIndirect"](indirectBuffer, indirectOffset);
  },

  wgpuRenderPassEncoderExecuteBundles: function(passId, count, bundlesPtr) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);

#if ASSERTIONS
    assert(bundlesPtr % 4 === 0);
#endif

    var bundles = Array.from(HEAP32.subarray(bundlesPtr >> 2, (bundlesPtr >> 2) + count),
      function(id) { return WebGPU.mgrRenderBundle.get(id); });
    pass["executeBundles"](bundles);
  },

  wgpuRenderPassEncoderBeginOcclusionQuery: function(passId, queryIndex) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["beginOcclusionQuery"](queryIndex);
  },
  wgpuRenderPassEncoderEndOcclusionQuery: function(passId) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["endOcclusionQuery"]();
  },

  wgpuRenderPassEncoderBeginPipelineStatisticsQuery: function(passId, querySetId, queryIndex) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    pass["beginPipelineStatisticsQuery"](querySet, queryIndex);
  },
  wgpuRenderPassEncoderEndPipelineStatisticsQuery: function(passId) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["endPipelineStatisticsQuery"]();
  },

  wgpuRenderPassEncoderWriteTimestamp: function(encoderId, querySetId, queryIndex) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var querySet = WebGPU.mgrQuerySet.get(querySetId);
    pass["writeTimestamp"](querySet, queryIndex);
  },

  wgpuRenderPassEncoderPushDebugGroup: function(encoderId, groupLabelPtr) {
    var encoder = WebGPU.mgrRenderPassEncoder.get(encoderId);
    encoder["pushDebugGroup"](UTF8ToString(groupLabelPtr));
  },
  wgpuRenderPassEncoderPopDebugGroup: function(encoderId) {
    var encoder = WebGPU.mgrRenderPassEncoder.get(encoderId);
    encoder["popDebugGroup"]();
  },
  wgpuRenderPassEncoderInsertDebugMarker: function(encoderId, markerLabelPtr) {
    var encoder = WebGPU.mgrRenderPassEncoder.get(encoderId);
    encoder["insertDebugMarker"](UTF8ToString(markerLabelPtr));
  },

  wgpuRenderPassEncoderEndPass: function(passId) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["endPass"]();
  },

  // Render bundle encoder

  wgpuRenderBundleEncoderSetBindGroup: function(bundleId, groupIndex, groupId, dynamicOffsetCount, dynamicOffsetsPtr) {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var group = WebGPU.mgrBindGroup.get(groupId);
    if (dynamicOffsetCount == 0) {
      pass["setBindGroup"](groupIndex, group);
    } else {
      var offsets = [];
      for (var i = 0; i < dynamicOffsetCount; i++, dynamicOffsetsPtr += 4) {
        offsets.push({{{ gpu.makeGetU32('dynamicOffsetsPtr', 0) }}});
      }
      pass["setBindGroup"](groupIndex, group, offsets);
    }
  },
  wgpuRenderBundleEncoderSetIndexBuffer: function(bundleId, bufferId, {{{ defineI64Param('offset') }}}) {
    {{{ receiveI64ParamAsI32s('offset') }}}
    var offset = {{{ gpu.makeU64ToNumber('offset_low', 'offset_high') }}};
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    pass["setIndexBuffer"](buffer, offset);
  },
  wgpuRenderBundleEncoderSetPipeline: function(bundleId, pipelineId) {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    pass["setPipeline"](pipeline);
  },
  wgpuRenderBundleEncoderSetVertexBuffer: function(bundleId, slot, bufferId, {{{ defineI64Param('offset') }}}) {
    {{{ receiveI64ParamAsI32s('offset') }}}
    var offset = {{{ gpu.makeU64ToNumber('offset_low', 'offset_high') }}};
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass["setVertexBuffer"](slot, WebGPU.mgrBuffer.get(bufferId), offset);
  },

  wgpuRenderBundleEncoderDraw: function(bundleId, vertexCount, instanceCount, firstVertex, firstInstance) {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass["draw"](vertexCount, instanceCount, firstVertex, firstInstance);
  },
  wgpuRenderBundleEncoderDrawIndexed: function(bundleId, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass["drawIndexed"](indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },

  wgpuRenderBundleEncoderPushDebugGroup: function(encoderId, groupLabelPtr) {
    var encoder = WebGPU.mgrRenderBundleEncoder.get(encoderId);
    encoder["pushDebugGroup"](UTF8ToString(groupLabelPtr));
  },
  wgpuRenderBundleEncoderPopDebugGroup: function(encoderId) {
    var encoder = WebGPU.mgrRenderBundleEncoder.get(encoderId);
    encoder["popDebugGroup"]();
  },
  wgpuRenderBundleEncoderInsertDebugMarker: function(encoderId, markerLabelPtr) {
    var encoder = WebGPU.mgrRenderBundleEncoder.get(encoderId);
    encoder["insertDebugMarker"](UTF8ToString(markerLabelPtr));
  },

  wgpuRenderBundleEncoderFinish: function(bundleId, descriptor) {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {};
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    }
    var encoder = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    return WebGPU.mgrRenderBundle.create(encoder["finish"](desc));
  },

  // Instance

  wgpuCreateInstance: function() {
#if ASSERTIONS
    // Once implemented, Instance should create and own the object management tables.
    abort('unimplemented (TODO)');
#endif
  },

  wgpuInstanceReference: function() {
#if ASSERTIONS
    abort('No WGPUInstance object should exist (TODO).');
#endif
  },
  wgpuInstanceRelease: function() {
#if ASSERTIONS
    abort('No WGPUInstance object should exist (TODO).');
#endif
  },

  wgpuInstanceCreateSurface__deps: ['$findCanvasEventTarget'],
  wgpuInstanceCreateSurface: function(instanceId, descriptor) {
    {{{ gpu.makeCheck('descriptor') }}}
    {{{ gpu.makeCheck('instanceId === 0, "WGPUInstance is ignored"') }}}
    var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSurfaceDescriptor.nextInChain, '*') }}};
#if ASSERTIONS
    assert(nextInChainPtr !== 0);
    assert({{{ gpu.SType.SurfaceDescriptorFromCanvasHTMLSelector }}} ===
      {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}});
#endif
    var descriptorFromCanvasHTMLSelector = nextInChainPtr;

    {{{ gpu.makeCheckDescriptor('descriptorFromCanvasHTMLSelector') }}}
    var selectorPtr = {{{ makeGetValue('descriptorFromCanvasHTMLSelector', C_STRUCTS.WGPUSurfaceDescriptorFromCanvasHTMLSelector.selector, '*') }}};
    {{{ gpu.makeCheck('selectorPtr') }}}
    var canvas = findCanvasEventTarget(selectorPtr);
#if ASSERTIONS
    assert(canvas instanceof HTMLCanvasElement);
#endif

    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSurfaceDescriptor.label, '*') }}};
    if (labelPtr) canvas.surfaceLabelWebGPU = UTF8ToString(labelPtr);

    return WebGPU.mgrSurface.create(canvas);
  },

  wgpuInstanceProcessEvents: function() {
#if ASSERTIONS
    abort('wgpuInstanceProcessEvents is unsupported (use requestAnimationFrame via html5.h instead)');
#endif
  },
  wgpuInstanceRequestAdapter: function() {
    abort('unimplemented (TODO)');
  },

  // WGPUAdapter

  wgpuAdapterGetProperties: function() {
    abort('unimplemented (TODO)');
  },
  wgpuAdapterRequestDevice: function() {
    abort('unimplemented (TODO)');
  },

  // WGPUSurface

  wgpuSurfaceGetPreferredFormat: function(surfaceId, adapterId, callback, userdata) {
    abort('unimplemented (TODO)');
  },

  // WGPUSwapChain

  wgpuDeviceCreateSwapChain: function(deviceId, surfaceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var device = WebGPU["mgrDevice"].get(deviceId);
    var canvas = WebGPU.mgrSurface.get(surfaceId);

    canvas.width = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.width) }}};
    canvas.height = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.height) }}};

    var ctx = canvas.getContext('gpupresent');
#if ASSERTIONS
    assert({{{ gpu.PresentMode.Fifo }}} ===
      {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.presentMode) }}});
#endif

    var desc = {
      "label": undefined,
      "device": device,
      "format": WebGPU.TextureFormat[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.format) }}}],
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.usage) }}},
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
    var swapChain = ctx["configureSwapChain"](desc);

    return WebGPU.mgrSwapChain.create(swapChain);
  },

  wgpuSwapChainGetCurrentTextureView: function(swapChainId) {
    var swapChain = WebGPU.mgrSwapChain.get(swapChainId);
    return WebGPU.mgrTextureView.create(swapChain["getCurrentTexture"]()["createView"]());
  },
  wgpuSwapChainPresent: function() {
    // TODO: This could probably be emulated with ASYNCIFY.
#if ASSERTIONS
    abort('wgpuSwapChainPresent is unsupported (use requestAnimationFrame via html5.h instead)');
#endif
  },

  // wgpuGetProcAddress

  wgpuGetProcAddress: function() {
#if ASSERTIONS
    abort('unimplemented (TODO: issue #11526)');
#endif
    return 0;
  },
};

autoAddDeps(LibraryWebGPU, '$WebGPU');
mergeInto(LibraryManager.library, LibraryWebGPU);
