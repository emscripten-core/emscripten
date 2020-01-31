/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
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
      var OffsetOfNextInChainMember = 0;
      return this.makeCheck(descriptor) + this.makeCheck(makeGetValue(descriptor, OffsetOfNextInChainMember, '*') + ' === 0');
    },

    // Must be in sync with webgpu.h.
    PresentMode: {
        VSync: 1,
    },
    SType: {
        SurfaceDescriptorFromHTMLCanvasId: 4,
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
          objects: [undefined],
          create: function(object, wrapper /* = {} */) {
            wrapper = wrapper || {};

            var id = this.objects.length;
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

      {{{ gpu.makeInitManager('BindGroup') }}}
      {{{ gpu.makeInitManager('Buffer') }}}
      {{{ gpu.makeInitManager('Sampler') }}}
      {{{ gpu.makeInitManager('Texture') }}}
      {{{ gpu.makeInitManager('TextureView') }}}

      {{{ gpu.makeInitManager('BindGroupLayout') }}}
      {{{ gpu.makeInitManager('PipelineLayout') }}}
      {{{ gpu.makeInitManager('RenderPipeline') }}}
      {{{ gpu.makeInitManager('ShaderModule') }}}

      {{{ gpu.makeInitManager('RenderBundleEncoder') }}}
      {{{ gpu.makeInitManager('RenderBundle') }}}
    },

    trackMapWrite: function(obj, mapped) {
      var data = _malloc(mapped.byteLength);
      HEAPU8.fill(0, data, mapped.byteLength);
      obj.mapWriteSrc = data;
      obj.mapWriteDst = mapped;
    },
    trackUnmap: function(obj) {
      if (obj.mapWriteSrc) {
        new Uint8Array(obj.mapWriteDst).set(HEAPU8.subarray(obj.mapWriteSrc, obj.mapWriteSrc + obj.mapWriteDst.byteLength));
        _free(obj.mapWriteSrc);
      }
      obj.mapWriteSrc = undefined;
      obj.mapWriteDst = undefined;
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
        "arrayLayer": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTextureCopyView.arrayLayer, '*') }}},
        "origin": WebGPU.makeOrigin3D(ptr + {{{ C_STRUCTS.WGPUTextureCopyView.origin }}}),
      };
    },

    makeBufferCopyView: function(ptr) {
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      return {
        "buffer": this.mgrBuffer.get(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUBufferCopyView.buffer, '*') }}}),
        "offset": {{{ gpu.makeGetU64('ptr', C_STRUCTS.WGPUBufferCopyView.offset) }}},
        "rowPitch": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUBufferCopyView.rowPitch) }}},
        "imageHeight": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUBufferCopyView.imageHeight) }}},
      };
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
      'sampled-texture',
      'storage-texture',
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
    PresentMode: [
      'no-v-sync',
      'v-sync',
    ],
    PrimitiveTopology: [
      'point-list',
      'line-list',
      'line-strip',
      'triangle-list',
      'triangle-strip',
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
      'bc1rgba-unorm',
      'bc1rgba-unorm-srgb',
      'bc2rgba-unorm',
      'bc2rgba-unorm-srgb',
      'bc3rgba-unorm',
      'bc3rgba-unorm-srgb',
      'bc4r-unorm',
      'bc4r-snorm',
      'bc5rg-unorm',
      'bc5rg-snorm',
      'bc6h-rgb-ufloat',
      'bc6h-rgb-sfloat',
      'bc7rgba-unorm',
      'bc7rgba-unorm-srgb',
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

  {{{ gpu.makeReferenceRelease('BindGroup') }}}
  {{{ gpu.makeReferenceRelease('Buffer') }}}
  {{{ gpu.makeReferenceRelease('Sampler') }}}
  {{{ gpu.makeReferenceRelease('Texture') }}}
  {{{ gpu.makeReferenceRelease('TextureView') }}}

  {{{ gpu.makeReferenceRelease('BindGroupLayout') }}}
  {{{ gpu.makeReferenceRelease('PipelineLayout') }}}
  {{{ gpu.makeReferenceRelease('RenderPipeline') }}}
  {{{ gpu.makeReferenceRelease('ShaderModule') }}}

  {{{ gpu.makeReferenceRelease('RenderBundleEncoder') }}}
  {{{ gpu.makeReferenceRelease('RenderBundle') }}}

  // *Destroy

  wgpuBufferDestroy: function(bufferId) { WebGPU.mgrBuffer.get(bufferId).destroy(); },
  wgpuTextureDestroy: function(textureId) { WebGPU.mgrTexture.get(textureId).destroy(); },

  // wgpuDevice

  // wgpuDeviceCreate*

  wgpuDeviceCreateQueue: function(deviceId) {
    assert(WebGPU.mgrQueue.objects.length === 1, 'there is only one queue');
    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrQueue.create(device["defaultQueue"]);
  },

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
    var desc = {
      "label": undefined,
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBufferDescriptor.usage) }}},
      "size": {{{ gpu.makeGetU64('descriptor', C_STRUCTS.WGPUBufferDescriptor.size) }}},
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBufferDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrBuffer.create(device["createBuffer"](desc));
  },

  wgpuDeviceCreateBufferMapped: function(returnPtr, deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var desc = {
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBufferDescriptor.usage) }}},
      "size": {{{ gpu.makeGetU64('descriptor', C_STRUCTS.WGPUBufferDescriptor.size) }}},
    };

    var device = WebGPU["mgrDevice"].get(deviceId);
    var bufferMapped = device["createBufferMapped"](desc);
    var buffer = bufferMapped[0];
    var mapped = bufferMapped[1];

    var bufferWrapper = {};
    var bufferId = WebGPU.mgrBuffer.create(buffer, bufferWrapper);
    WebGPU.trackMapWrite(bufferWrapper, mapped);

    var dataLength_h = (mapped.byteLength / 0x100000000) | 0;
    var dataLength_l = mapped.byteLength | 0;

    {{{ makeSetValue('returnPtr', C_STRUCTS.WGPUCreateBufferMappedResult.buffer, 'bufferId', '*') }}}
    {{{ makeSetValue('returnPtr', C_STRUCTS.WGPUCreateBufferMappedResult.dataLength + 0, 'dataLength_l', 'i32') }}}
    {{{ makeSetValue('returnPtr', C_STRUCTS.WGPUCreateBufferMappedResult.dataLength + 4, 'dataLength_h', 'i32') }}}
    {{{ makeSetValue('returnPtr', C_STRUCTS.WGPUCreateBufferMappedResult.data, 'bufferWrapper.mapWriteSrc', '*') }}}
  },

  wgpuDeviceCreateTexture: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "label": undefined,
      "size": WebGPU.makeExtent3D(descriptor + {{{ C_STRUCTS.WGPUTextureDescriptor.size }}}),
      "arrayLayerCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.arrayLayerCount) }}},
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

    function makeBinding(bindingPtr) {
      {{{ gpu.makeCheck('bindingPtr') }}}

      return {
        "binding":
          {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupLayoutBinding.binding) }}},
        "visibility":
          {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupLayoutBinding.visibility) }}},
        "type": WebGPU.BindingType[
          {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupLayoutBinding.type) }}}],
        "textureDimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupLayoutBinding.textureDimension) }}}],
        "textureComponentType": WebGPU.TextureComponentType[
          {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupLayoutBinding.textureComponentType) }}}],
        "multisampled":
          {{{ gpu.makeGetBool('bindingPtr', C_STRUCTS.WGPUBindGroupLayoutBinding.multisampled) }}},
        "hasDynamicOffset":
          {{{ gpu.makeGetBool('bindingPtr', C_STRUCTS.WGPUBindGroupLayoutBinding.hasDynamicOffset) }}},
      };
    }

    function makeBindings(count, bindingsPtrs) {
      var bindings = [];
      for (var i = 0; i < count; ++i) {
        bindings.push(makeBinding(bindingsPtrs +
            {{{ C_STRUCTS.WGPUBindGroupLayoutBinding.__size__ }}} * i));
      }
      return bindings;
    }

    var desc = {
      "bindings": makeBindings(
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.bindingCount) }}},
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.bindings, '*') }}}
      ),
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrBindGroupLayout.create(device["createBindGroupLayout"](desc));
  },

  wgpuDeviceCreateBindGroup: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeBinding(bindingPtr) {
      {{{ gpu.makeCheck('bindingPtr') }}}

      var bufferId = {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupBinding.buffer) }}};
      var samplerId = {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupBinding.sampler) }}};
      var textureViewId = {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupBinding.textureView) }}};
#if ASSERTIONS
      assert((bufferId != 0) + (samplerId != 0) + (textureViewId != 0) == 1);
#endif

      var binding = {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupLayoutBinding.binding) }}};

      if (bufferId != 0) {
        var size = undefined;

        // Handle WGPU_WHOLE_SIZE.
        var sizePart1 = {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupBinding.size) }}};
        var sizePart2 = {{{ gpu.makeGetU32('bindingPtr', C_STRUCTS.WGPUBindGroupBinding.size + 4) }}};
        if (sizePart1 != 0xFFFFFFFF || sizePart2 != 0xFFFFFFFF) {
          size = {{{ gpu.makeGetU64('bindingPtr', C_STRUCTS.WGPUBindGroupBinding.size) }}};
        }

        return {
          "binding": binding,
          "resource": {
            "buffer": WebGPU.mgrBuffer.get(bufferId),
            "offset": {{{ gpu.makeGetU64('bindingPtr', C_STRUCTS.WGPUBindGroupBinding.offset) }}},
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

    function makeBindings(count, bindingsPtrs) {
      var bindings = [];
      for (var i = 0; i < count; ++i) {
        bindings.push(makeBinding(bindingsPtrs +
            {{{C_STRUCTS.WGPUBindGroupBinding.__size__}}} * i));
      }
      return bindings;
    }

    var desc = {
      "label": undefined,
      "layout": WebGPU.mgrBindGroupLayout.get(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.layout, '*') }}}),
      "bindings": makeBindings(
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.bindingCount) }}},
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.bindings, '*') }}}
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

  wgpuDeviceCreateComputePipeline: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "label": undefined,
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
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var count = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUShaderModuleDescriptor.codeSize) }}};
    var start = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUShaderModuleDescriptor.code, '*') }}};
    var desc = {
      "label": undefined,
      "code": HEAPU32.subarray(start >> 2, (start >> 2) + count),
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUShaderModuleDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU["mgrDevice"].get(deviceId);
    return WebGPU.mgrShaderModule.create(device["createShaderModule"](desc));
  },

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
      dynCall('viii', callback, [type, messagePtr, userdata]);
      _free(messagePtr);
    };
  },

  // wgpuFence

  wgpuFenceOnCompletion: function(fenceId, completionValue_l, completionValue_h, callback, userdata) {
    var fence = WebGPU.mgrFence.get(fenceId);
    var completionValue = {{{ gpu.makeU64ToNumber('completionValue_l', 'completionValue_h') }}};

    var DAWN_FENCE_COMPLETION_STATUS_SUCCESS = 0;
    var DAWN_FENCE_COMPLETION_STATUS_ERROR = 1;

    fence.onCompletion(completionValue).then(function() {
      dynCall('vii', callback, [DAWN_FENCE_COMPLETION_STATUS_SUCCESS, userdata]);
    }, function() {
      dynCall('vii', callback, [DAWN_FENCE_COMPLETION_STATUS_ERROR, userdata]);
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

  wgpuQueueSignal: function(queueId, fenceId, signalValue_l, signalValue_h) {
    var queue = WebGPU.mgrQueue.get(queueId);
    var fence = WebGPU.mgrFence.get(fenceId);
    var signalValue = {{{ gpu.makeU64ToNumber('signalValue_l', 'signalValue_h') }}};
    queue.signal(fence, signalValue);
  },

  wgpuQueueSubmit: function(queueId, commandCount, commands) {
#if ASSERTIONS
    assert(commands % 4 === 0);
#endif
    var queue = WebGPU.mgrQueue.get(queueId);
    var cmds = Array.from(HEAP32.subarray(commands >> 2, (commands >> 2) + commandCount),
      function(id) { return WebGPU.mgrCommandBuffer.get(id); });
    queue.submit(cmds);
  },

  // wgpuCommandEncoder

  wgpuCommandEncoderFinish: function(encoderId) {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    return WebGPU.mgrCommandBuffer.create(commandEncoder["finish"]());
  },

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
        "stencilStoreOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachmentDescriptor.stencilStoreOp) }}}],
        "stencilLoadValue": stencilLoadValue,
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
      };
      var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.label, '*') }}};
      if (labelPtr) desc["label"] = UTF8ToString(labelPtr);
      return desc;
    }

    var desc = makeRenderPassDescriptor(descriptor);

    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    return WebGPU.mgrRenderPassEncoder.create(commandEncoder["beginRenderPass"](desc));
  },

  wgpuCommandEncoderCopyBufferToBuffer: function(encoderId, srcId, srcOffset_l, srcOffset_h, dstId, dstOffset_l, dstOffset_h, size_l, size_h) {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var src = WebGPU.mgrBuffer.get(srcId);
    var dst = WebGPU.mgrBuffer.get(dstId);
    commandEncoder["copyBufferToBuffer"](
      src, {{{ gpu.makeU64ToNumber('srcOffset_l', 'srcOffset_h') }}},
      dst, {{{ gpu.makeU64ToNumber('dstOffset_l', 'dstOffset_h') }}},
      {{{ gpu.makeU64ToNumber('size_l', 'size_h') }}});
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

  // wgpuBuffer

  wgpuBufferSetSubData: function(bufferId, start_l, start_h, count_l, count_h, data) {
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    var start = {{{ gpu.makeU64ToNumber('start_l', 'start_h') }}};
    var count = {{{ gpu.makeU64ToNumber('count_l', 'count_h') }}};
    buffer["setSubData"](start, HEAPU8, data, count);
  },

  wgpuBufferMapReadAsync: function(bufferId, callback, userdata) {
    var bufferEntry = WebGPU.mgrBuffer.objects[bufferId];
    bufferEntry.mapped = 'write';
    var buffer = bufferEntry.object;

    buffer["mapReadAsync"]().then(function(mapped) {
      var DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS = 0;
      var data = _malloc(mapped.byteLength);
      HEAPU8.set(new Uint8Array(mapped), data);
      var dataLength_h = (mapped.byteLength / 0x100000000) | 0;
      var dataLength_l = mapped.byteLength | 0;
      // WGPUBufferMapAsyncStatus status, const void* data, uint64_t dataLength, void* userdata
      dynCall('viiji', callback, [DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data, dataLength_l, dataLength_h, userdata]);
    }, function() {
      // TODO(kainino0x): Figure out how to pick other error status values.
      var DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR = 1;
      dynCall('viiji', callback, [DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, 0, 0, 0, userdata]);
    });
  },

  wgpuBufferMapWriteAsync: function(bufferId, callback, userdata) {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    var buffer = bufferWrapper.object;

    var DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS = 0;
    var DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR = 1;
    buffer["mapWriteAsync"]().then(function(mapped) {
      WebGPU.trackMapWrite(bufferWrapper, mapped);

      var data = bufferWrapper.mapWriteSrc;
      var dataLength_h = (mapped.byteLength / 0x100000000) | 0;
      var dataLength_l = mapped.byteLength | 0;
      // WGPUBufferMapAsyncStatus status, void* data, uint64_t dataLength, void* userdata
      dynCall('viiji', callback, [DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data, dataLength_l, dataLength_h, userdata]);
    }, function() {
      // TODO(kainino0x): Figure out how to pick other error status values.
      dynCall('viiji', callback, [DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, 0, 0, 0, userdata]);
    });
  },

  wgpuBufferUnmap: function(bufferId) {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    WebGPU.trackUnmap(bufferWrapper);
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

  wgpuComputePassEncoderSetPipeline: function(passId, pipelineId) {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    var pipeline = WebGPU.mgrComputePipeline.get(pipelineId);
    pass["setPipeline"](pipeline);
  },
  wgpuComputePassEncoderDispatch: function(passId, x, y, z) {
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    pass["dispatch"](x, y, z);
  },
  wgpuComputePassEncoderDispatchIndirect: function(passId, indirectBufferId, indirectOffset_l, indirectOffset_h) {
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var indirectOffset = {{{ gpu.makeU64ToNumber('indirectOffset_l', 'indirectOffset_h') }}};
    var pass = WebGPU.mgrComputePassEncoder.get(passId);
    pass["dispatchIndirect"](indirectBuffer, indirectOffset);
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
  wgpuRenderPassEncoderSetIndexBuffer: function(passId, bufferId, offset) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    pass["setIndexBuffer"](buffer, offset);
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
  wgpuRenderPassEncoderSetVertexBuffer: function(passId, slot, bufferId, offset) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["setVertexBuffer"](slot, WebGPU.mgrBuffer.get(bufferId), offset);
  },
  wgpuRenderPassEncoderDraw: function(passId, vertexCount, instanceCount, firstVertex, firstInstance) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["draw"](vertexCount, instanceCount, firstVertex, firstInstance);
  },
  wgpuRenderPassEncoderDrawIndexed: function(passId, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["drawIndexed"](indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },
  wgpuRenderPassEncoderDrawIndirect: function(passId, indirectBufferId, indirectOffset_l, indirectOffset_h) {
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var indirectOffset = {{{ gpu.makeU64ToNumber('indirectOffset_l', 'indirectOffset_h') }}};
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    pass["drawIndirect"](indirectBuffer, indirectOffset);
  },
  wgpuRenderPassEncoderDrawIndexedIndirect: function(passId, indirectBufferId, indirectOffset_l, indirectOffset_h) {
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var indirectOffset = {{{ gpu.makeU64ToNumber('indirectOffset_l', 'indirectOffset_h') }}};
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
  wgpuRenderBundleEncoderSetIndexBuffer: function(bundleId, bufferId, offset_l, offset_h) {
    var offset = {{{ gpu.makeU64ToNumber('offset_l', 'offset_h') }}};
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    pass["setIndexBuffer"](buffer, offset);
  },
  wgpuRenderBundleEncoderSetPipeline: function(bundleId, pipelineId) {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    pass["setPipeline"](pipeline);
  },
  wgpuRenderBundleEncoderSetVertexBuffer: function(bundleId, slot, bufferId, offset_l, offset_h) {
    var offset = {{{ gpu.makeU64ToNumber('offset_l', 'offset_h') }}};
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

  wgpuInstanceReference: function() {
#if ASSERTIONS
    assert(false, 'No WGPUInstance object should exist.');
#endif
  },
  wgpuInstanceRelease: function() {
#if ASSERTIONS
    assert(false, 'No WGPUInstance object should exist.');
#endif
  },

  wgpuInstanceCreateSurface: function(instanceId, descriptor) {
    {{{ gpu.makeCheck('descriptor') }}}
    {{{ gpu.makeCheck('instanceId === 0, "WGPUInstance is ignored"') }}}
    var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSurfaceDescriptor.nextInChain, '*') }}};
#if ASSERTIONS
    assert(nextInChainPtr !== 0);
    assert({{{ gpu.SType.SurfaceDescriptorFromHTMLCanvasId }}} ===
      {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}});
#endif
    var descriptorFromHTMLCanvasId = nextInChainPtr;

    {{{ gpu.makeCheckDescriptor('descriptorFromHTMLCanvasId') }}}
    var idPtr = {{{ makeGetValue('descriptorFromHTMLCanvasId', C_STRUCTS.WGPUSurfaceDescriptorFromHTMLCanvasId.id, '*') }}};
    {{{ gpu.makeCheck('idPtr') }}}
    var id = UTF8ToString(idPtr);
    var canvas = document.getElementById(id);
    assert(canvas instanceof HTMLCanvasElement);

    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSurfaceDescriptor.label, '*') }}};
    if (labelPtr) canvas.surfaceLabelWebGPU = UTF8ToString(labelPtr);

    return WebGPU.mgrSurface.create(canvas);
  },

  // wgpuDeviceCreateSwapChain + WGPUSwapChain

  wgpuDeviceCreateSwapChain: function(deviceId, surfaceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var device = WebGPU["mgrDevice"].get(deviceId);
    var canvas = WebGPU.mgrSurface.get(surfaceId);

    canvas.width = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.width) }}};
    canvas.height = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.height) }}};

    var ctx = canvas.getContext('gpupresent');
    assert({{{ gpu.PresentMode.VSync }}} ===
      {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.presentMode) }}});

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

  // Unsupported (won't be implemented)

  wgpuDeviceTick: function() {
    assert(false, 'wgpuDeviceTick is unsupported (use requestAnimationFrame via html5.h instead)');
  },
  wgpuSwapChainPresent: function() {
    assert(false, 'wgpuSwapChainPresent is unsupported (use requestAnimationFrame via html5.h instead)');
  },
};

autoAddDeps(LibraryWebGPU, '$WebGPU');
mergeInto(LibraryManager.library, LibraryWebGPU);
