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
 *
 * To test this, run the following tests:
 * - tests/runner.py other.test_webgpu_compiletest
 * - EMTEST_BROWSERS="/path/to/chrome --user-data-dir=chromeuserdata --enable-unsafe-webgpu" \
 *   tests/runner.py browser.test_webgpu_basic_rendering
 *   (requires WebGPU to be available - otherwise the test will skip itself and pass)
 */

{{{ (function() {
  // Helper functions for code generation
  global.gpu = {
    makeInitManager: function(type) {
      var mgr = 'WebGPU.mgr' + type
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
      var ret = '(';
      if (ASSERTIONS) {
        ret += `assert(${highName} < 0x200000), `;
      }
      ret += `${highName} * 0x100000000 + ${lowName})\n`;
      return ret;
    },

    makeU64ToNumberWithSentinelAsUndefined: function(lowName, highName) {
      var ret = `((${highName} === -1 && ${lowName} === -1) ? undefined : \
          ${this.makeU64ToNumber(lowName, highName)})`;
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
    COPY_STRIDE_UNDEFINED: 0xFFFFFFFF,
    LIMIT_U32_UNDEFINED: 0xFFFFFFFF,
    AdapterType: {
      Unknown: 3,
    },
    BackendType: {
      WebGPU: 1,
    },
    BufferMapAsyncStatus: {
      Success: 0,
      Error: 1,
    },
    ErrorType: {
      NoError: 0,
      Validation: 1,
      OutOfMemory: 2,
      Unknown: 3,
    },
    PresentMode: {
      Fifo: 2,
    },
    LoadOp: {
      Clear: 0,
      Load: 1,
    },
    MapMode: {
      None: 0,
      Read: 1,
      Write: 2
    },
    RequestAdapterStatus: {
      Success: 0,
      Unavailable: 1,
      Error: 2,
    },
    RequestDeviceStatus: {
      Success: 0,
      Error: 1,
    },
    SType: {
      SurfaceDescriptorFromCanvasHTMLSelector: 4,
      ShaderModuleSPIRVDescriptor: 5,
      ShaderModuleWGSLDescriptor: 6,
    },
    QueueWorkDoneStatus: {
      Success: 0,
      Error: 1,
    },
  };
  return null;
})(); }}}

var LibraryWebGPU = {
  $WebGPU__postset: 'WebGPU.initManagers();',
  $WebGPU: {
    initManagers: function() {
      if (WebGPU.mgrDevice) return;

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
            if (!id) return undefined;
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

      {{{ gpu.makeInitManager('Adapter') }}}
      // TODO: Release() the device's default queue when the device is freed.
      {{{ gpu.makeInitManager('Device') }}}
      {{{ gpu.makeInitManager('Queue') }}}

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
        "r": {{{ makeGetValue('ptr', 0, 'double') }}},
        "g": {{{ makeGetValue('ptr', 8, 'double') }}},
        "b": {{{ makeGetValue('ptr', 16, 'double') }}},
        "a": {{{ makeGetValue('ptr', 24, 'double') }}},
      };
    },

    makeExtent3D: function(ptr) {
      return {
        "width": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.width) }}},
        "height": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.height) }}},
        "depthOrArrayLayers": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.depthOrArrayLayers) }}},
      };
    },

    makeOrigin3D: function(ptr) {
      return {
        "x": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.x) }}},
        "y": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.y) }}},
        "z": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.z) }}},
      };
    },

    makeImageCopyTexture: function(ptr) {
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      return {
        "texture": WebGPU.mgrTexture.get(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUImageCopyTexture.texture, '*') }}}),
        "mipLevel": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUImageCopyTexture.mipLevel) }}},
        "origin": WebGPU.makeOrigin3D(ptr + {{{ C_STRUCTS.WGPUImageCopyTexture.origin }}}),
        "aspect": WebGPU.TextureAspect[{{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUImageCopyTexture.aspect) }}}],
      };
    },

    makeTextureDataLayout: function(ptr) {
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      var bytesPerRow = {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTextureDataLayout.bytesPerRow) }}};
      var rowsPerImage = {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTextureDataLayout.rowsPerImage) }}};
      return {
        "offset": {{{ gpu.makeGetU64('ptr', C_STRUCTS.WGPUTextureDataLayout.offset) }}},
        "bytesPerRow": bytesPerRow === {{{ gpu.COPY_STRIDE_UNDEFINED }}} ? undefined : bytesPerRow,
        "rowsPerImage": rowsPerImage === {{{ gpu.COPY_STRIDE_UNDEFINED }}} ? undefined : rowsPerImage,
      };
    },

    makeImageCopyBuffer: function(ptr) {
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      var layoutPtr = ptr + {{{ C_STRUCTS.WGPUImageCopyBuffer.layout }}};
      var bufferCopyView = WebGPU.makeTextureDataLayout(layoutPtr);
      bufferCopyView["buffer"] = WebGPU.mgrBuffer.get(
        {{{ makeGetValue('ptr', C_STRUCTS.WGPUImageCopyBuffer.buffer, '*') }}});
      return bufferCopyView;
    },

    makePipelineConstants: function(constantCount, constantsPtr) {
      if (!constantCount) return;
      var constants = {};
      for (var i = 0; i < constantCount; ++i) {
        var entryPtr = constantsPtr + {{{ C_STRUCTS.WGPUConstantEntry.__size__ }}} * i;
        var key = UTF8ToString({{{ makeGetValue('entryPtr', C_STRUCTS.WGPUConstantEntry.key, '*') }}});
        constants[key] = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUConstantEntry.value, 'double') }}};
      }
      return constants;
    },

    makeProgrammableStageDescriptor: function(ptr) {
      if (!ptr) return undefined;
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      return {
        "module": WebGPU.mgrShaderModule.get(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUProgrammableStageDescriptor.module, '*') }}}),
        "entryPoint": UTF8ToString(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUProgrammableStageDescriptor.entryPoint, '*') }}}),
        "constants": WebGPU.makePipelineConstants(
          {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUProgrammableStageDescriptor.constantCount) }}},
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUProgrammableStageDescriptor.constants, '*') }}}),
      };
    },

    // Map from enum string back to enum number, for callbacks.
    DeviceLostReason: {
      'undefined': 0,
      'destroyed': 1,
    },
    PreferredFormat: {
      'rgba8unorm': 0x12,
      'bgra8unorm': 0x17,
    },

    // This section is auto-generated. See system/include/webgpu/README.md for details.
    AddressMode: [
      'repeat',
      'mirror-repeat',
      'clamp-to-edge',
    ],
    BlendFactor: [
      'zero',
      'one',
      'src',
      'one-minus-src',
      'src-alpha',
      'one-minus-src-alpha',
      'dst',
      'one-minus-dst',
      'dst-alpha',
      'one-minus-dst-alpha',
      'src-alpha-saturated',
      'constant',
      'one-minus-constant',
    ],
    BlendOperation: [
      'add',
      'subtract',
      'reverse-subtract',
      'min',
      'max',
    ],
    BufferBindingType: [
      undefined,
      'uniform',
      'storage',
      'read-only-storage',
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
      'validation',
      'out-of-memory',
    ],
    FeatureName: [
      undefined,
      'depth-clamping',
      'depth24unorm-stencil8',
      'depth32float-stencil8',
      'timestamp-query',
      'pipeline-statistics-query',
      'texture-compression-bc',
      'texture-compression-etc2',
      'texture-compression-astc',
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
      undefined,
      'uint16',
      'uint32',
    ],
    PipelineStatisticName: [
      'vertex-shader-invocations',
      'clipper-invocations',
      'clipper-primitives-out',
      'fragment-shader-invocations',
      'compute-shader-invocations',
    ],
    PowerPreference: [
      'low-power',
      'high-performance',
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
    SamplerBindingType: [
      undefined,
      'filtering',
      'non-filtering',
      'comparison',
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
    StorageTextureAccess: [
      undefined,
      'write-only',
    ],
    StoreOp: [
      'store',
      'discard',
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
      'depth-comparison',
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
      'rg11b10ufloat',
      'rgb9e5ufloat',
      'rg32float',
      'rg32uint',
      'rg32sint',
      'rgba16uint',
      'rgba16sint',
      'rgba16float',
      'rgba32float',
      'rgba32uint',
      'rgba32sint',
      'stencil8',
      'depth16unorm',
      'depth24plus',
      'depth24plus-stencil8',
      'depth32float',
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
      'bc6h-rgb-float',
      'bc7-rgba-unorm',
      'bc7-rgba-unorm-srgb',
      'etc2-rgb8unorm',
      'etc2-rgb8unorm-srgb',
      'etc2-rgb8a1unorm',
      'etc2-rgb8a1unorm-srgb',
      'etc2-rgba8unorm',
      'etc2-rgba8unorm-srgb',
      'eac-r11unorm',
      'eac-r11snorm',
      'eac-rg11unorm',
      'eac-rg11snorm',
      'astc-4x4-unorm',
      'astc-4x4-unorm-srgb',
      'astc-5x4-unorm',
      'astc-5x4-unorm-srgb',
      'astc-5x5-unorm',
      'astc-5x5-unorm-srgb',
      'astc-6x5-unorm',
      'astc-6x5-unorm-srgb',
      'astc-6x6-unorm',
      'astc-6x6-unorm-srgb',
      'astc-8x5-unorm',
      'astc-8x5-unorm-srgb',
      'astc-8x6-unorm',
      'astc-8x6-unorm-srgb',
      'astc-8x8-unorm',
      'astc-8x8-unorm-srgb',
      'astc-10x5-unorm',
      'astc-10x5-unorm-srgb',
      'astc-10x6-unorm',
      'astc-10x6-unorm-srgb',
      'astc-10x8-unorm',
      'astc-10x8-unorm-srgb',
      'astc-10x10-unorm',
      'astc-10x10-unorm-srgb',
      'astc-12x10-unorm',
      'astc-12x10-unorm-srgb',
      'astc-12x12-unorm',
      'astc-12x12-unorm-srgb',
    ],
    TextureSampleType: [
      undefined,
      'float',
      'unfilterable-float',
      'depth',
      'sint',
      'uint',
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
      undefined,
      'uint8x2',
      'uint8x4',
      'sint8x2',
      'sint8x4',
      'unorm8x2',
      'unorm8x4',
      'snorm8x2',
      'snorm8x4',
      'uint16x2',
      'uint16x4',
      'sint16x2',
      'sint16x4',
      'unorm16x2',
      'unorm16x4',
      'snorm16x2',
      'snorm16x4',
      'float16x2',
      'float16x4',
      'float32',
      'float32x2',
      'float32x3',
      'float32x4',
      'uint32',
      'uint32x2',
      'uint32x3',
      'uint32x4',
      'sint32',
      'sint32x2',
      'sint32x3',
      'sint32x4',
    ],
    VertexStepMode: [
      'vertex',
      'instance',
    ],
  },

  // *Reference/*Release

  {{{ gpu.makeReferenceRelease('Surface') }}}
  {{{ gpu.makeReferenceRelease('SwapChain') }}}

  {{{ gpu.makeReferenceRelease('Adapter') }}}
  {{{ gpu.makeReferenceRelease('Device') }}}
  {{{ gpu.makeReferenceRelease('Queue') }}}

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

  wgpuDeviceDestroy: function(deviceId) { WebGPU.mgrDevice.get(deviceId)["destroy"](); },

  wgpuDeviceGetLimits: function(deviceId, limitsOutPtr) {
#if ASSERTIONS
    abort('TODO: wgpuDeviceGetLimits unimplemented');
#endif
  },

  wgpuDeviceGetQueue: function(deviceId) {
    var queueId = WebGPU.mgrDevice.objects[deviceId].queueId;
#if ASSERTIONS
    assert(queueId, 'wgpuDeviceGetQueue: queue was missing or null');
#endif
    // Returns a new reference to the existing queue.
    WebGPU.mgrQueue.reference(queueId);
    return queueId;
  },

  wgpuDevicePushErrorScope: function(deviceId, filter) {
    var device = WebGPU.mgrDevice.get(deviceId);
    device["pushErrorScope"](WebGPU.ErrorFilter[filter]);
  },

  wgpuDevicePopErrorScope__deps: [
    '$callUserCallback',
#if MINIMAL_RUNTIME
    '$allocateUTF8',
#else
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  wgpuDevicePopErrorScope: function(deviceId, callback, userdata) {
    var device = WebGPU.mgrDevice.get(deviceId);
    {{{ runtimeKeepalivePush() }}}
    device["popErrorScope"]().then(function(gpuError) {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        if (!gpuError) {
          {{{ makeDynCall('viii', 'callback') }}}(
            {{{ gpu.ErrorType.NoError }}}, 0, userdata);
        } else if (gpuError instanceof GPUOutOfMemoryError) {
          {{{ makeDynCall('viii', 'callback') }}}(
            {{{ gpu.ErrorType.OutOfMemory }}}, 0, userdata);
        } else {
#if ASSERTIONS
          assert(gpuError instanceof GPUValidationError);
#endif
          var messagePtr = allocateUTF8(gpuError.message);
          {{{ makeDynCall('viii', 'callback') }}}({{{ gpu.ErrorType.Validation }}}, messagePtr, userdata);
          _free(messagePtr);
        }
      });
    }, function(ex) {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        var messagePtr = allocateUTF8(ex.message);
        // TODO: This can mean either the device was lost or the error scope stack was empty. Figure
        // out how to synthesize the DeviceLost error type. (Could be by simply tracking the error
        // scope depth, but that isn't ideal.)
        {{{ makeDynCall('viii', 'callback') }}}({{{ gpu.ErrorType.Unknown }}}, messagePtr, userdata);
        _free(messagePtr);
      });
    });
  },

  wgpuDeviceSetDeviceLostCallback__deps: [
    '$callUserCallback',
#if MINIMAL_RUNTIME
    '$allocateUTF8',
#endif
  ],
  wgpuDeviceSetDeviceLostCallback: function(deviceId, callback, userdata) {
    var deviceWrapper = WebGPU.mgrDevice.objects[deviceId];
    {{{ gpu.makeCheckDefined('deviceWrapper') }}}
    if (!deviceWrapper.lostCallback) {
      // device.lost hasn't been registered yet - register it.
      deviceWrapper.object["lost"].then(function(info) {
        deviceWrapper.lostCallback(info);
      });
    }
    deviceWrapper.lostCallback = function(info) {
      // This will skip the callback if the runtime is no longer alive.
      callUserCallback(function() {
        var messagePtr = allocateUTF8(info.message);
        {{{ makeDynCall('viii', 'callback') }}}(WebGPU.DeviceLostReason[info.reason], messagePtr, userdata);
        _free(messagePtr);
      });
    };
  },

  wgpuDeviceSetUncapturedErrorCallback__deps: [
    '$callUserCallback',
#if MINIMAL_RUNTIME
    '$allocateUTF8',
#endif
  ],
  wgpuDeviceSetUncapturedErrorCallback: function(deviceId, callback, userdata) {
    var device = WebGPU.mgrDevice.get(deviceId);
    device["onuncapturederror"] = function(ev) {
      // This will skip the callback if the runtime is no longer alive.
      callUserCallback(function() {
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
      });
    };
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
    var device = WebGPU.mgrDevice.get(deviceId);
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

    var device = WebGPU.mgrDevice.get(deviceId);
    var bufferWrapper = {};
    var id = WebGPU.mgrBuffer.create(device["createBuffer"](desc), bufferWrapper);
    if (mappedAtCreation) {
      bufferWrapper.mapMode = {{{ gpu.MapMode.Write }}};
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

    var device = WebGPU.mgrDevice.get(deviceId);
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

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrSampler.create(device["createSampler"](desc));
  },

  wgpuDeviceCreateBindGroupLayout: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeBufferEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var typeInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBufferBindingLayout.type) }}};
      if (!typeInt) return undefined;

      return {
        "type": WebGPU.BufferBindingType[typeInt],
        "hasDynamicOffset":
          {{{ gpu.makeGetBool('entryPtr', C_STRUCTS.WGPUBufferBindingLayout.hasDynamicOffset) }}},
        "minBindingSize":
          {{{ gpu.makeGetU64('entryPtr', C_STRUCTS.WGPUBufferBindingLayout.minBindingSize) }}},
      };
    }

    function makeSamplerEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var typeInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUSamplerBindingLayout.type) }}};
      if (!typeInt) return undefined;

      return {
        "type": WebGPU.SamplerBindingType[typeInt],
      };
    }

    function makeTextureEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var sampleTypeInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUTextureBindingLayout.sampleType) }}};
      if (!sampleTypeInt) return undefined;

      return {
        "sampleType": WebGPU.TextureSampleType[sampleTypeInt],
        "viewDimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUTextureBindingLayout.viewDimension) }}}],
        "multisampled":
          {{{ gpu.makeGetBool('entryPtr', C_STRUCTS.WGPUTextureBindingLayout.multisampled) }}},
      };
    }

    function makeStorageTextureEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var accessInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.access) }}}
      if (!accessInt) return undefined;

      return {
        "access": WebGPU.StorageTextureAccess[accessInt],
        "format": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.format) }}}],
        "viewDimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.viewDimension) }}}],
      };
    }

    function makeEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      return {
        "binding":
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.binding) }}},
        "visibility":
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.visibility) }}},
        "buffer": makeBufferEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.buffer }}}),
        "sampler": makeSamplerEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.sampler }}}),
        "texture": makeTextureEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.texture }}}),
        "storageTexture": makeStorageTextureEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.storageTexture }}}),
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

    var device = WebGPU.mgrDevice.get(deviceId);
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
      assert((bufferId !== 0) + (samplerId !== 0) + (textureViewId !== 0) === 1);
#endif

      var binding = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.binding) }}};

      if (bufferId) {
        var size_low = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.size) }}};
        var size_high = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.size + 4) }}};
        var size = {{{ gpu.makeU64ToNumberWithSentinelAsUndefined('size_low', 'size_high') }}};

        return {
          "binding": binding,
          "resource": {
            "buffer": WebGPU.mgrBuffer.get(bufferId),
            "offset": {{{ gpu.makeGetU64('entryPtr', C_STRUCTS.WGPUBindGroupEntry.offset) }}},
            "size": size,
          },
        };
      } else if (samplerId) {
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

    var device = WebGPU.mgrDevice.get(deviceId);
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

    var device = WebGPU.mgrDevice.get(deviceId);
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

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrQuerySet.create(device["createQuerySet"](desc));
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
    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrRenderBundleEncoder.create(device["createRenderBundleEncoder"](desc));
  },

  wgpuDeviceCreateComputePipeline: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "label": undefined,
      "layout":  WebGPU.mgrPipelineLayout.get(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePipelineDescriptor.layout, '*') }}}),
      "compute": WebGPU.makeProgrammableStageDescriptor(
        descriptor + {{{ C_STRUCTS.WGPUComputePipelineDescriptor.compute }}}),
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePipelineDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrComputePipeline.create(device["createComputePipeline"](desc));
  },

  wgpuDeviceCreateComputePipelineAsync: function(deviceId, descriptor, callback, userdata) {
    abort('TODO: wgpuDeviceCreateComputePipelineAsync unimplemented');
  },

  wgpuDeviceCreateRenderPipeline: function(deviceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makePrimitiveState(rsPtr) {
      if (!rsPtr) return undefined;
      {{{ gpu.makeCheckDescriptor('rsPtr') }}}
      return {
        "topology": WebGPU.PrimitiveTopology[
          {{{ gpu.makeGetU32('rsPtr', C_STRUCTS.WGPUPrimitiveState.topology) }}}],
        "stripIndexFormat": WebGPU.IndexFormat[
          {{{ gpu.makeGetU32('rsPtr', C_STRUCTS.WGPUPrimitiveState.stripIndexFormat) }}}],
        "frontFace": WebGPU.FrontFace[
          {{{ gpu.makeGetU32('rsPtr', C_STRUCTS.WGPUPrimitiveState.frontFace) }}}],
        "cullMode": WebGPU.CullMode[
          {{{ gpu.makeGetU32('rsPtr', C_STRUCTS.WGPUPrimitiveState.cullMode) }}}],
      };
    }

    function makeBlendComponent(bdPtr) {
      if (!bdPtr) return undefined;
      return {
        "operation": WebGPU.BlendOperation[
          {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendComponent.operation) }}}],
        "srcFactor": WebGPU.BlendFactor[
          {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendComponent.srcFactor) }}}],
        "dstFactor": WebGPU.BlendFactor[
          {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendComponent.dstFactor) }}}],
      };
    }

    function makeBlendState(bsPtr) {
      if (!bsPtr) return undefined;
      {{{ gpu.makeCheckDescriptor('bsPtr') }}}
      return {
        "alpha": makeBlendComponent(bsPtr + {{{ C_STRUCTS.WGPUBlendState.alpha }}}),
        "color": makeBlendComponent(bsPtr + {{{ C_STRUCTS.WGPUBlendState.color }}}),
      };
    }

    function makeColorState(csPtr) {
      {{{ gpu.makeCheckDescriptor('csPtr') }}}
      return {
        "format": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('csPtr', C_STRUCTS.WGPUColorTargetState.format) }}}],
        "blend": makeBlendState({{{ makeGetValue('csPtr', C_STRUCTS.WGPUColorTargetState.blend, '*') }}}),
        "writeMask": {{{ gpu.makeGetU32('csPtr', C_STRUCTS.WGPUColorTargetState.writeMask) }}},
      };
    }

    function makeColorStates(count, csArrayPtr) {
      var states = [];
      for (var i = 0; i < count; ++i) {
        states.push(makeColorState(csArrayPtr + {{{ C_STRUCTS.WGPUColorTargetState.__size__ }}} * i));
      }
      return states;
    }

    function makeStencilStateFace(ssfPtr) {
      {{{ gpu.makeCheck('ssfPtr') }}}
      return {
        "compare": WebGPU.CompareFunction[
          {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.compare) }}}],
        "failOp": WebGPU.StencilOperation[
          {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.failOp) }}}],
        "depthFailOp": WebGPU.StencilOperation[
          {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.depthFailOp) }}}],
        "passOp": WebGPU.StencilOperation[
          {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.passOp) }}}],
      };
    }

    function makeDepthStencilState(dssPtr) {
      if (!dssPtr) return undefined;

      {{{ gpu.makeCheck('dssPtr') }}}
      return {
        "format": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.format) }}}],
        "depthWriteEnabled": {{{ gpu.makeGetBool('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthWriteEnabled) }}},
        "depthCompare": WebGPU.CompareFunction[
          {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthCompare) }}}],
        "stencilFront": makeStencilStateFace(dssPtr + {{{ C_STRUCTS.WGPUDepthStencilState.stencilFront }}}),
        "stencilBack": makeStencilStateFace(dssPtr + {{{ C_STRUCTS.WGPUDepthStencilState.stencilBack }}}),
        "stencilReadMask": {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.stencilReadMask) }}},
        "stencilWriteMask": {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.stencilWriteMask) }}},
        "depthBias": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBias, '*') }}},
        "depthBiasSlopeScale": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBiasSlopeScale, 'float') }}},
        "depthBiasClamp": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBiasClamp, 'float') }}},
      };
    }

    function makeVertexAttribute(vaPtr) {
      {{{ gpu.makeCheck('vaPtr') }}}
      return {
        "format": WebGPU.VertexFormat[
          {{{ gpu.makeGetU32('vaPtr', C_STRUCTS.WGPUVertexAttribute.format) }}}],
        "offset": {{{ gpu.makeGetU64('vaPtr', C_STRUCTS.WGPUVertexAttribute.offset) }}},
        "shaderLocation": {{{ gpu.makeGetU32('vaPtr', C_STRUCTS.WGPUVertexAttribute.shaderLocation) }}},
      };
    }

    function makeVertexAttributes(count, vaArrayPtr) {
      var vas = [];
      for (var i = 0; i < count; ++i) {
        vas.push(makeVertexAttribute(vaArrayPtr + i * {{{ C_STRUCTS.WGPUVertexAttribute.__size__ }}}));
      }
      return vas;
    }

    function makeVertexBuffer(vbPtr) {
      if (!vbPtr) return undefined;

      return {
        "arrayStride": {{{ gpu.makeGetU64('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.arrayStride) }}},
        "stepMode": WebGPU.VertexStepMode[
          {{{ gpu.makeGetU32('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.stepMode) }}}],
        "attributes": makeVertexAttributes(
          {{{ gpu.makeGetU32('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.attributeCount) }}},
          {{{ makeGetValue('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.attributes, '*') }}}),
      };
    }

    function makeVertexBuffers(count, vbArrayPtr) {
      if (!count) return undefined;

      var vbs = [];
      for (var i = 0; i < count; ++i) {
        vbs.push(makeVertexBuffer(vbArrayPtr + i * {{{ C_STRUCTS.WGPUVertexBufferLayout.__size__ }}}));
      }
      return vbs;
    }

    function makeVertexState(viPtr) {
      if (!viPtr) return undefined;
      {{{ gpu.makeCheckDescriptor('viPtr') }}}
      return {
        "module": WebGPU.mgrShaderModule.get(
          {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.module, '*') }}}),
        "entryPoint": UTF8ToString(
          {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.entryPoint, '*') }}}),
        "constants": WebGPU.makePipelineConstants(
          {{{ gpu.makeGetU32('viPtr', C_STRUCTS.WGPUVertexState.constantCount) }}},
          {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.constants, '*') }}}),
        "buffers": makeVertexBuffers(
          {{{ gpu.makeGetU32('viPtr', C_STRUCTS.WGPUVertexState.bufferCount) }}},
          {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.buffers, '*') }}}),
      };
    }

    function makeMultisampleState(msPtr) {
      if (!msPtr) return undefined;
      {{{ gpu.makeCheckDescriptor('msPtr') }}}
      return {
        "count": {{{ gpu.makeGetU32('msPtr', C_STRUCTS.WGPUMultisampleState.count) }}},
        "mask": {{{ gpu.makeGetU32('msPtr', C_STRUCTS.WGPUMultisampleState.mask) }}},
        "alphaToCoverageEnabled": {{{ gpu.makeGetBool('msPtr', C_STRUCTS.WGPUMultisampleState.alphaToCoverageEnabled) }}},
      };
    }

    function makeFragmentState(fsPtr) {
      if (!fsPtr) return undefined;
      {{{ gpu.makeCheckDescriptor('fsPtr') }}}
      return {
        "module": WebGPU.mgrShaderModule.get(
          {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.module, '*') }}}),
        "entryPoint": UTF8ToString(
          {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.entryPoint, '*') }}}),
        "constants": WebGPU.makePipelineConstants(
          {{{ gpu.makeGetU32('fsPtr', C_STRUCTS.WGPUFragmentState.constantCount) }}},
          {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.constants, '*') }}}),
        "targets": makeColorStates(
          {{{ gpu.makeGetU32('fsPtr', C_STRUCTS.WGPUFragmentState.targetCount) }}},
          {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.targets, '*') }}}),
      };
    }

    var desc = {
      "label": undefined,
      "layout": WebGPU.mgrPipelineLayout.get(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.layout, '*') }}}),
      "vertex": makeVertexState(
        descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.vertex }}}),
      "primitive": makePrimitiveState(
        descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.primitive }}}),
      "depthStencil": makeDepthStencilState(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.depthStencil, '*') }}}),
      "multisample": makeMultisampleState(
        descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.multisample }}}),
      "fragment": makeFragmentState(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.fragment, '*') }}}),
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrRenderPipeline.create(device["createRenderPipeline"](desc));
  },

  wgpuDeviceCreateRenderPipelineAsync: function(deviceId, descriptor, callback, userdata) {
    abort('TODO: wgpuDeviceCreateRenderPipelineAsync unimplemented');
  },

  wgpuDeviceCreateShaderModule: function(deviceId, descriptor) {
    {{{ gpu.makeCheck('descriptor') }}}
    var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUShaderModuleDescriptor.nextInChain, '*') }}};
#if ASSERTIONS
    assert(nextInChainPtr !== 0);
#endif
    var sType = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}};

    var desc = {
      "label": undefined,
      "code": "",
    };
    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUShaderModuleDescriptor.label, '*') }}};
    if (labelPtr) desc["label"] = UTF8ToString(labelPtr);

    switch (sType) {
      case {{{ gpu.SType.ShaderModuleSPIRVDescriptor }}}: {
        var count = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUShaderModuleSPIRVDescriptor.codeSize) }}};
        var start = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUShaderModuleSPIRVDescriptor.code, '*') }}};
#if USE_PTHREADS
        // Chrome can't currently handle a SharedArrayBuffer view here, so make a copy.
        desc["code"] = HEAPU32.slice(start >> 2, (start >> 2) + count);
#else
        desc["code"] = HEAPU32.subarray(start >> 2, (start >> 2) + count);
#endif
        break;
      }
      case {{{ gpu.SType.ShaderModuleWGSLDescriptor }}}: {
        var sourcePtr = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUShaderModuleWGSLDescriptor.source, '*') }}};
        if (sourcePtr) {
          desc["code"] = UTF8ToString(sourcePtr);
        }
        break;
      }
#if ASSERTIONS
      default: abort('unrecognized ShaderModule sType');
#endif
    }

    var device = WebGPU.mgrDevice.get(deviceId);
    return WebGPU.mgrShaderModule.create(device["createShaderModule"](desc));
  },

  // wgpuQueue

  wgpuQueueSubmit: function(queueId, commandCount, commands) {
#if ASSERTIONS
    assert(commands % 4 === 0);
#endif
    var queue = WebGPU.mgrQueue.get(queueId);
    var cmds = Array.from(HEAP32.subarray(commands >> 2, (commands >> 2) + commandCount),
      function(id) { return WebGPU.mgrCommandBuffer.get(id); });
    queue["submit"](cmds);
  },

  wgpuQueueOnSubmittedWorkDone__deps: [
    '$callUserCallback',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  wgpuQueueOnSubmittedWorkDone: function(queueId, {{{ defineI64Param('signalValue') }}}, callback, userdata) {
    var queue = WebGPU.mgrQueue.get(queueId);
#if ASSERTIONS
    assert(signalValue_low === 0 && signalValue_high === 0, 'signalValue not supported, must be 0');
#endif

    {{{ runtimeKeepalivePush() }}}
    queue["onSubmittedWorkDone"]().then(function() {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        {{{ makeDynCall('vii', 'callback') }}}({{{ gpu.QueueWorkDoneStatus.Success }}}, userdata);
      });
    }, function() {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        {{{ makeDynCall('vii', 'callback') }}}({{{ gpu.QueueWorkDoneStatus.Error }}}, userdata);
      });
    });
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
      destinationPtr, data, dataSize, dataLayoutPtr, writeSizePtr) {
    var queue = WebGPU.mgrQueue.get(queueId);

    var destination = WebGPU.makeImageCopyTexture(destinationPtr);
    var dataLayout = WebGPU.makeTextureDataLayout(dataLayoutPtr);
    var writeSize = WebGPU.makeExtent3D(writeSizePtr);
    // This subarray isn't strictly necessary, but helps work around an issue
    // where Chromium makes a copy of the entire heap. crbug.com/1134457
    var subarray = HEAPU8.subarray(data, data + dataSize);
    queue["writeTexture"](destination, subarray, dataLayout, writeSize);
  },

  // wgpuCommandEncoder

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
      var loadOpInt = {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.loadOp) }}};
      #if ASSERTIONS
          assert(loadOpInt === {{{ gpu.LoadOp.Clear }}} || loadOpInt === {{{ gpu.LoadOp.Load }}});
      #endif
      var loadValue = loadOpInt ? 'load' :
        WebGPU.makeColor(caPtr + {{{ C_STRUCTS.WGPURenderPassColorAttachment.clearColor }}});

      return {
        "view": WebGPU.mgrTextureView.get(
          {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.view) }}}),
        "resolveTarget": WebGPU.mgrTextureView.get(
          {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.resolveTarget) }}}),
        "storeOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.storeOp) }}}],
        "loadValue": loadValue,
      };
    }

    function makeColorAttachments(count, caPtr) {
      var attachments = [];
      for (var i = 0; i < count; ++i) {
        attachments.push(makeColorAttachment(caPtr + {{{ C_STRUCTS.WGPURenderPassColorAttachment.__size__ }}} * i));
      }
      return attachments;
    }

    function makeDepthStencilAttachment(dsaPtr) {
      if (dsaPtr === 0) return undefined;

      var depthLoadOpInt = {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthLoadOp) }}};
      #if ASSERTIONS
          assert(depthLoadOpInt === {{{ gpu.LoadOp.Clear }}} || depthLoadOpInt === {{{ gpu.LoadOp.Load }}});
      #endif
      var depthLoadValue = depthLoadOpInt ? 'load' :
        {{{ makeGetValue('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.clearDepth, 'float') }}};

      var stencilLoadOpInt = {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilLoadOp) }}};
      #if ASSERTIONS
          assert(stencilLoadOpInt === {{{ gpu.LoadOp.Clear }}} || stencilLoadOpInt === {{{ gpu.LoadOp.Load }}});
      #endif
      var stencilLoadValue = stencilLoadOpInt ? 'load' :
        {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.clearStencil) }}};

      return {
        "view": WebGPU.mgrTextureView.get(
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.view) }}}),
        "depthStoreOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthStoreOp) }}}],
        "depthLoadValue": depthLoadValue,
        "depthReadOnly": {{{ gpu.makeGetBool('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthReadOnly) }}},
        "stencilStoreOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilStoreOp) }}}],
        "stencilLoadValue": stencilLoadValue,
        "stencilReadOnly": {{{ gpu.makeGetBool('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilReadOnly) }}},
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

  wgpuCommandEncoderClearBuffer: function(encoderId, bufferId, {{{ defineI64Param('offset') }}}, {{{ defineI64Param('size') }}}) {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    {{{ receiveI64ParamAsI32s('offset') }}}
    {{{ receiveI64ParamAsI32s('size') }}}

    var buffer = WebGPU.mgrBuffer.get(bufferId);
    commandEncoder["clearBuffer"](
      buffer, 
      {{{ gpu.makeU64ToNumber('offset_low', 'offset_high') }}},
      {{{ gpu.makeU64ToNumber('size_low', 'size_high') }}} 
    );
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
      WebGPU.makeImageCopyBuffer(srcPtr), WebGPU.makeImageCopyTexture(dstPtr), copySize);
  },

  wgpuCommandEncoderCopyTextureToBuffer: function(encoderId, srcPtr, dstPtr, copySizePtr) {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder["copyTextureToBuffer"](
      WebGPU.makeImageCopyTexture(srcPtr), WebGPU.makeImageCopyBuffer(dstPtr), copySize);
  },

  wgpuCommandEncoderCopyTextureToTexture: function(encoderId, srcPtr, dstPtr, copySizePtr) {
    var commandEncoder = WebGPU.mgrCommandEncoder.get(encoderId);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder["copyTextureToTexture"](
      WebGPU.makeImageCopyTexture(srcPtr), WebGPU.makeImageCopyTexture(dstPtr), copySize);
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

  // wgpuShaderModule

  wgpuShaderModuleSetLabel: function(shaderModuleId, labelPtr) {
    var shaderModule = WebGPU.mgrShaderModule.get(shaderModuleId);
    shaderModule.label = UTF8ToString(labelPtr);
  },

  // wgpuComputePipeline

  wgpuComputePipelineGetBindGroupLayout: function(pipelineId, groupIndex) {
    var pipeline = WebGPU.mgrComputePipeline.get(pipelineId);
    return WebGPU.mgrBindGroupLayout.create(pipeline["getBindGroupLayout"](groupIndex));
  },
  wgpuComputePipelineSetLabel: function(pipelineId, labelPtr) {
    var pipeline = WebGPU.mgrComputePipeline.get(pipelineId);
    pipeline.label = UTF8ToString(labelPtr);
  },

  // wgpuRenderPipeline

  wgpuRenderPipelineGetBindGroupLayout: function(pipelineId, groupIndex) {
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    return WebGPU.mgrBindGroupLayout.create(pipeline["getBindGroupLayout"](groupIndex));
  },
  wgpuRenderPipelineSetLabel: function(pipelineId, labelPtr) {
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    pipeline.label = UTF8ToString(labelPtr);
  },

  // wgpuBuffer

  wgpuBufferGetConstMappedRange: function(bufferId, offset, size) {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    {{{ gpu.makeCheckDefined('bufferWrapper') }}}

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
    {{{ gpu.makeCheckDefined('bufferWrapper') }}}

    // TODO: if the sentinel value becomes WGPU_WHOLE_SIZE instead of 0, update this.
    if (size === 0) size = undefined;

    if (bufferWrapper.mapMode !== {{{ gpu.MapMode.Write }}}) {
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

  wgpuBufferMapAsync__deps: [
    '$callUserCallback',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  wgpuBufferMapAsync: function(bufferId, mode, offset, size, callback, userdata) {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    {{{ gpu.makeCheckDefined('bufferWrapper') }}}
    bufferWrapper.mapMode = mode;
    bufferWrapper.onUnmap = [];
    var buffer = bufferWrapper.object;

    // `callback` takes (WGPUBufferMapAsyncStatus status, void * userdata)

    {{{ runtimeKeepalivePush() }}}
    buffer["mapAsync"](mode, offset, size).then(function() {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        {{{ makeDynCall('vii', 'callback') }}}({{{ gpu.BufferMapAsyncStatus.Success }}}, userdata);
      });
    }, function() {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        // TODO(kainino0x): Figure out how to pick other error status values.
        {{{ makeDynCall('vii', 'callback') }}}({{{ gpu.BufferMapAsyncStatus.Error }}}, userdata);
      });
    });
  },

  wgpuBufferUnmap: function(bufferId) {
    var bufferWrapper = WebGPU.mgrBuffer.objects[bufferId];
    {{{ gpu.makeCheckDefined('bufferWrapper') }}}

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
    var pass = WebGPU.mgrComputePassEncoder.get(encoderId);
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
  wgpuRenderPassEncoderSetBlendConstant: function(passId, colorPtr) {
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var color = WebGPU.makeColor(colorPtr);
    pass["setBlendConstant"](color);
  },
  wgpuRenderPassEncoderSetIndexBuffer: function(passId, bufferId, format, {{{ defineI64Param('offset') }}}, {{{ defineI64Param('size') }}}) {
    {{{ receiveI64ParamAsI32s('offset') }}}
    {{{ receiveI64ParamAsI32s('size') }}}
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    var offset = {{{ gpu.makeU64ToNumber('offset_low', 'offset_high') }}};
    var size = {{{ gpu.makeU64ToNumberWithSentinelAsUndefined('size_low', 'size_high') }}};
    pass["setIndexBuffer"](buffer, WebGPU.IndexFormat[format], offset, size);
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
  wgpuRenderPassEncoderSetVertexBuffer: function(passId, slot, bufferId, {{{ defineI64Param('offset') }}}, {{{ defineI64Param('size') }}}) {
    {{{ receiveI64ParamAsI32s('offset') }}}
    {{{ receiveI64ParamAsI32s('size') }}}
    var pass = WebGPU.mgrRenderPassEncoder.get(passId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    var offset = {{{ gpu.makeU64ToNumber('offset_low', 'offset_high') }}};
    var size = {{{ gpu.makeU64ToNumberWithSentinelAsUndefined('size_low', 'size_high') }}};
    pass["setVertexBuffer"](slot, buffer, offset, size);
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
    var pass = WebGPU.mgrRenderPassEncoder.get(encoderId);
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
  wgpuRenderBundleEncoderSetIndexBuffer: function(bundleId, bufferId, format, {{{ defineI64Param('offset') }}}, {{{ defineI64Param('size') }}}) {
    {{{ receiveI64ParamAsI32s('offset') }}}
    {{{ receiveI64ParamAsI32s('size') }}}
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    var offset = {{{ gpu.makeU64ToNumber('offset_low', 'offset_high') }}};
    var size = {{{ gpu.makeU64ToNumberWithSentinelAsUndefined('size_low', 'size_high') }}};
    pass["setIndexBuffer"](buffer, WebGPU.IndexFormat[format], offset, size);
  },
  wgpuRenderBundleEncoderSetPipeline: function(bundleId, pipelineId) {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var pipeline = WebGPU.mgrRenderPipeline.get(pipelineId);
    pass["setPipeline"](pipeline);
  },
  wgpuRenderBundleEncoderSetVertexBuffer: function(bundleId, slot, bufferId, {{{ defineI64Param('offset') }}}, {{{ defineI64Param('size') }}}) {
    {{{ receiveI64ParamAsI32s('offset') }}}
    {{{ receiveI64ParamAsI32s('size') }}}
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    var buffer = WebGPU.mgrBuffer.get(bufferId);
    var offset = {{{ gpu.makeU64ToNumber('offset_low', 'offset_high') }}};
    var size = {{{ gpu.makeU64ToNumberWithSentinelAsUndefined('size_low', 'size_high') }}};
    pass["setVertexBuffer"](slot, buffer, offset, size);
  },

  wgpuRenderBundleEncoderDraw: function(bundleId, vertexCount, instanceCount, firstVertex, firstInstance) {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass["draw"](vertexCount, instanceCount, firstVertex, firstInstance);
  },
  wgpuRenderBundleEncoderDrawIndexed: function(bundleId, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) {
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass["drawIndexed"](indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },
  wgpuRenderBundleEncoderDrawIndirect: function(bundleId, indirectBufferId, {{{ defineI64Param('indirectOffset') }}}) {
    {{{ receiveI64ParamAsI32s('indirectOffset') }}}
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var indirectOffset = {{{ gpu.makeU64ToNumber('indirectOffset_low', 'indirectOffset_high') }}};
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass["drawIndirect"](indirectBuffer, indirectOffset);
  },
  wgpuRenderBundleEncoderDrawIndexedIndirect: function(bundleId, indirectBufferId, {{{ defineI64Param('indirectOffset') }}}) {
    {{{ receiveI64ParamAsI32s('indirectOffset') }}}
    var indirectBuffer = WebGPU.mgrBuffer.get(indirectBufferId);
    var indirectOffset = {{{ gpu.makeU64ToNumber('indirectOffset_low', 'indirectOffset_high') }}};
    var pass = WebGPU.mgrRenderBundleEncoder.get(bundleId);
    pass["drawIndexedIndirect"](indirectBuffer, indirectOffset);
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
    abort('TODO: wgpuCreateInstance unimplemented');
#endif
  },

  wgpuInstanceReference: function() {
#if ASSERTIONS
    abort('TODO: no WGPUInstance object should exist');
#endif
  },
  wgpuInstanceRelease: function() {
#if ASSERTIONS
    abort('TODO: no WGPUInstance object should exist');
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
    const context = canvas.getContext('webgpu');
#if ASSERTIONS
    assert(context);
#endif
    if (!context) return 0;

    var labelPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSurfaceDescriptor.label, '*') }}};
    if (labelPtr) context.surfaceLabelWebGPU = UTF8ToString(labelPtr);

    return WebGPU.mgrSurface.create(context);
  },

  wgpuInstanceProcessEvents: function() {
    // TODO: This could probably be emulated with ASYNCIFY.
#if ASSERTIONS
    abort('wgpuInstanceProcessEvents is unsupported (use requestAnimationFrame via html5.h instead)');
#endif
  },

  wgpuInstanceRequestAdapter__deps: [
    '$callUserCallback',
#if MINIMAL_RUNTIME
    '$allocateUTF8',
#else
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  wgpuInstanceRequestAdapter: function(instanceId, options, callback, userdata) {
    {{{ gpu.makeCheck('instanceId === 0, "WGPUInstance is ignored"') }}}

    var opts;
    if (options) {
      {{{ gpu.makeCheckDescriptor('options') }}}
      opts = {
        "powerPreference": WebGPU.PowerPreference[
          {{{ gpu.makeGetU32('options', C_STRUCTS.WGPURequestAdapterOptions.powerPreference) }}}],
        "forceFallbackAdapter":
          {{{ gpu.makeGetBool('options', C_STRUCTS.WGPURequestAdapterOptions.forceFallbackAdapter) }}},
      };
    }

    if (!('gpu' in navigator)) {
      var messagePtr = allocateUTF8('WebGPU not available on this browser (navigator.gpu is not available)');
      {{{ makeDynCall('viiii', 'callback') }}}({{{ gpu.RequestAdapterStatus.Unavailable }}}, 0, messagePtr, userdata);
      _free(messagePtr);
      return;
    }

    {{{ runtimeKeepalivePush() }}}
    navigator["gpu"]["requestAdapter"](opts).then(function(adapter) {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        if (adapter) {
          var adapterId = WebGPU.mgrAdapter.create(adapter);
          {{{ makeDynCall('viiii', 'callback') }}}({{{ gpu.RequestAdapterStatus.Success }}}, adapterId, 0, userdata);
        } else {
          var messagePtr = allocateUTF8('WebGPU not available on this system (requestAdapter returned null)');
          {{{ makeDynCall('viiii', 'callback') }}}({{{ gpu.RequestAdapterStatus.Unavailable }}}, 0, messagePtr, userdata);
          _free(messagePtr);
        }
      });
    }, function(ex) {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        var messagePtr = allocateUTF8(ex.message);
        {{{ makeDynCall('viiii', 'callback') }}}({{{ gpu.RequestAdapterStatus.Error }}}, 0, messagePtr, userdata);
        _free(messagePtr);
      });
    });
  },

  // WGPUAdapter

  wgpuAdapterGetProperties: function(adapterId, properties) {
    {{{ gpu.makeCheckDescriptor('properties') }}}
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.vendorID, '0', 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.deviceID, '0', 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.name, '0', 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.driverDescription, '0', 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.adapterType, gpu.AdapterType.Unknown, 'i32') }}};
    {{{ makeSetValue('properties', C_STRUCTS.WGPUAdapterProperties.backendType, gpu.BackendType.WebGPU, 'i32') }}};
  },

  wgpuAdapterGetLimits: function(adapterId, limitsOutPtr) {
    abort('TODO: wgpuAdapterGetLimits unimplemented');
  },

  wgpuAdapterHasFeature: function(adapterId, feature) {
    abort('TODO: wgpuAdapterHasFeature unimplemented');
  },

  wgpuAdapterRequestDevice__deps: [
    '$callUserCallback',
#if MINIMAL_RUNTIME
    '$allocateUTF8',
#else
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  wgpuAdapterRequestDevice: function(adapterId, descriptor, callback, userdata) {
    var adapter = WebGPU.mgrAdapter.get(adapterId);

    var desc = {};
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      var requiredFeaturesCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredFeaturesCount) }}};
      if (requiredFeaturesCount) {
        var requiredFeaturesPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredFeatures, '*') }}};
        desc["requiredFeatures"] = Array.from(HEAP32.subarray(requiredFeaturesPtr >> 2, (requiredFeaturesPtr >> 2) + requiredFeaturesCount),
          function(feature) { return WebGPU.FeatureName[feature]; });
      }
      var requiredLimitsPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredLimits, '*') }}};
      if (requiredLimitsPtr) {
        {{{ gpu.makeCheckDescriptor('requiredLimitsPtr') }}}
        var limitsPtr = requiredLimitsPtr + {{{ C_STRUCTS.WGPURequiredLimits.limits }}};
        var requiredLimits = {};
        function setLimitU32IfDefined(name, limitOffset) {
          var ptr = limitsPtr + limitOffset;
          var value = {{{ gpu.makeGetU32('ptr', 0) }}};
          if (value != {{{ gpu.LIMIT_U32_UNDEFINED }}}) {
            requiredLimits[name] = value;
          }
        }
        function setLimitU64IfDefined(name, limitOffset) {
          var ptr = limitsPtr + limitOffset;
          // Handle WGPU_LIMIT_U64_UNDEFINED.
          var limitPart1 = {{{ gpu.makeGetU32('ptr', 0) }}};
          var limitPart2 = {{{ gpu.makeGetU32('ptr', 4) }}};
          if (limitPart1 != 0xFFFFFFFF || limitPart2 != 0xFFFFFFFF) {
            requiredLimits[name] = {{{ gpu.makeGetU64('ptr', 0) }}}
          }
        }

        setLimitU32IfDefined("maxTextureDimension1D", {{{ C_STRUCTS.WGPULimits.maxTextureDimension1D }}});
        setLimitU32IfDefined("maxTextureDimension2D", {{{ C_STRUCTS.WGPULimits.maxTextureDimension2D }}});
        setLimitU32IfDefined("maxTextureDimension3D", {{{ C_STRUCTS.WGPULimits.maxTextureDimension3D }}});
        setLimitU32IfDefined("maxTextureArrayLayers", {{{ C_STRUCTS.WGPULimits.maxTextureArrayLayers }}});
        setLimitU32IfDefined("maxBindGroups", {{{ C_STRUCTS.WGPULimits.maxBindGroups }}});
        setLimitU32IfDefined("maxDynamicUniformBuffersPerPipelineLayout", {{{ C_STRUCTS.WGPULimits.maxDynamicUniformBuffersPerPipelineLayout }}});
        setLimitU32IfDefined("maxDynamicStorageBuffersPerPipelineLayout", {{{ C_STRUCTS.WGPULimits.maxDynamicStorageBuffersPerPipelineLayout }}});
        setLimitU32IfDefined("maxSampledTexturesPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxSampledTexturesPerShaderStage }}});
        setLimitU32IfDefined("maxSamplersPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxSamplersPerShaderStage }}});
        setLimitU32IfDefined("maxStorageBuffersPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxStorageBuffersPerShaderStage }}});
        setLimitU32IfDefined("maxStorageTexturesPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxStorageTexturesPerShaderStage }}});
        setLimitU32IfDefined("maxUniformBuffersPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxUniformBuffersPerShaderStage }}});
        setLimitU32IfDefined("minUniformBufferOffsetAlignment", {{{ C_STRUCTS.WGPULimits.minUniformBufferOffsetAlignment }}});
        setLimitU32IfDefined("minStorageBufferOffsetAlignment", {{{ C_STRUCTS.WGPULimits.minStorageBufferOffsetAlignment }}});
        setLimitU64IfDefined("maxUniformBufferBindingSize", {{{ C_STRUCTS.WGPULimits.maxUniformBufferBindingSize }}});
        setLimitU64IfDefined("maxStorageBufferBindingSize", {{{ C_STRUCTS.WGPULimits.maxStorageBufferBindingSize }}});
        setLimitU32IfDefined("maxVertexBuffers", {{{ C_STRUCTS.WGPULimits.maxVertexBuffers }}});
        setLimitU32IfDefined("maxVertexAttributes", {{{ C_STRUCTS.WGPULimits.maxVertexAttributes }}});
        setLimitU32IfDefined("maxVertexBufferArrayStride", {{{ C_STRUCTS.WGPULimits.maxVertexBufferArrayStride }}});
        setLimitU32IfDefined("maxInterStageShaderComponents", {{{ C_STRUCTS.WGPULimits.maxInterStageShaderComponents }}});
        setLimitU32IfDefined("maxComputeWorkgroupStorageSize", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupStorageSize }}});
        setLimitU32IfDefined("maxComputeInvocationsPerWorkgroup", {{{ C_STRUCTS.WGPULimits.maxComputeInvocationsPerWorkgroup }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeX", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeX }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeY", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeY }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeZ", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeZ }}});
        setLimitU32IfDefined("maxComputeWorkgroupsPerDimension", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupsPerDimension }}});
        desc["requiredLimits"] = requiredLimits;
      }
    }

    {{{ runtimeKeepalivePush() }}}
    adapter["requestDevice"](desc).then(function(device) {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        var deviceWrapper = { queueId: WebGPU.mgrQueue.create(device["queue"]) };
        var deviceId = WebGPU.mgrDevice.create(device, deviceWrapper);
        {{{ makeDynCall('viiii', 'callback') }}}({{{ gpu.RequestDeviceStatus.Success }}}, deviceId, 0, userdata);
      });
    }, function(ex) {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function() {
        var messagePtr = allocateUTF8(ex.message);
        {{{ makeDynCall('viiii', 'callback') }}}({{{ gpu.RequestDeviceStatus.Error }}}, 0, messagePtr, userdata);
        _free(messagePtr);
      });
    });
  },

  // WGPUSurface

  wgpuSurfaceGetPreferredFormat: function(surfaceId, adapterId) {
    var context = WebGPU.mgrSurface.get(surfaceId);
    var adapter = WebGPU.mgrAdapter.get(adapterId);
    const format = context["getPreferredFormat"](adapter);
    return WebGPU.PreferredFormat[format];
  },

  // WGPUSwapChain

  wgpuDeviceCreateSwapChain: function(deviceId, surfaceId, descriptor) {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var device = WebGPU.mgrDevice.get(deviceId);
    var context = WebGPU.mgrSurface.get(surfaceId);


#if ASSERTIONS
    assert({{{ gpu.PresentMode.Fifo }}} ===
      {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.presentMode) }}});
#endif

    var configuration = {
      "device": device,
      "format": WebGPU.TextureFormat[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.format) }}}],
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.usage) }}},
      "size": [
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.width) }}},
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSwapChainDescriptor.height) }}},
      ],
    };
    context["configure"](configuration);

    return WebGPU.mgrSwapChain.create(context);
  },

  wgpuSwapChainGetCurrentTextureView: function(swapChainId) {
    var context = WebGPU.mgrSwapChain.get(swapChainId);
    return WebGPU.mgrTextureView.create(context["getCurrentTexture"]()["createView"]());
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
    abort('TODO(#11526): wgpuGetProcAddress unimplemented');
#endif
    return 0;
  },
};

autoAddDeps(LibraryWebGPU, '$WebGPU');
mergeInto(LibraryManager.library, LibraryWebGPU);
