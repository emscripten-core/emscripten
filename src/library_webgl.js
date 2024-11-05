/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Specifies the size of the GL temp buffer pool, in bytes. Must be a multiple
// of 9 and 16.
{{{ GL_POOL_TEMP_BUFFERS_SIZE = 2*9*16 }}} // = 288

{{{
  globalThis.isCurrentContextWebGL2 = () => {
    // This function should only be called inside of `#if MAX_WEBGL_VERSION >= 2` blocks
    assert(MAX_WEBGL_VERSION >= 2, 'isCurrentContextWebGL2 called without webgl2 support');
    if (MIN_WEBGL_VERSION >= 2) return 'true';
    return 'GL.currentContext.version >= 2';
  };
  null;
}}}

var LibraryGL = {
  // For functions such as glDrawBuffers, glInvalidateFramebuffer and
  // glInvalidateSubFramebuffer that need to pass a short array to the WebGL
  // API, create a set of short fixed-length arrays to avoid having to generate
  // any garbage when calling those functions.
  $tempFixedLengthArray__postset: 'for (var i = 0; i < 32; ++i) tempFixedLengthArray.push(new Array(i));',
  $tempFixedLengthArray: [],

  $miniTempWebGLFloatBuffers: [],
  $miniTempWebGLFloatBuffers__postset: `var miniTempWebGLFloatBuffersStorage = new Float32Array({{{ GL_POOL_TEMP_BUFFERS_SIZE }}});
// Create GL_POOL_TEMP_BUFFERS_SIZE+1 temporary buffers, for uploads of size 0 through GL_POOL_TEMP_BUFFERS_SIZE inclusive
for (/**@suppress{duplicate}*/var i = 0; i <= {{{ GL_POOL_TEMP_BUFFERS_SIZE }}}; ++i) {
  miniTempWebGLFloatBuffers[i] = miniTempWebGLFloatBuffersStorage.subarray(0, i);
}`,

  $miniTempWebGLIntBuffers: [],
  $miniTempWebGLIntBuffers__postset: `var miniTempWebGLIntBuffersStorage = new Int32Array({{{ GL_POOL_TEMP_BUFFERS_SIZE }}});
// Create GL_POOL_TEMP_BUFFERS_SIZE+1 temporary buffers, for uploads of size 0 through GL_POOL_TEMP_BUFFERS_SIZE inclusive
for (/**@suppress{duplicate}*/var i = 0; i <= {{{ GL_POOL_TEMP_BUFFERS_SIZE }}}; ++i) {
  miniTempWebGLIntBuffers[i] = miniTempWebGLIntBuffersStorage.subarray(0, i);
}`,

  $heapObjectForWebGLType: (type) => {
    // Micro-optimization for size: Subtract lowest GL enum number (0x1400/* GL_BYTE */) from type to compare
    // smaller values for the heap, for shorter generated code size.
    // Also the type HEAPU16 is not tested for explicitly, but any unrecognized type will return out HEAPU16.
    // (since most types are HEAPU16)
    type -= 0x1400;
#if MAX_WEBGL_VERSION >= 2
    if (type == {{{ 0x1400 - 0x1400/* GL_BYTE */ }}}) return HEAP8;
#endif

    if (type == {{{ 0x1401 - 0x1400/* GL_UNSIGNED_BYTE */ }}}) return HEAPU8;

#if MAX_WEBGL_VERSION >= 2
    if (type == {{{ 0x1402 - 0x1400/* GL_SHORT */ }}}) return HEAP16;
#endif

    if (type == {{{ 0x1404 - 0x1400/* GL_INT */ }}}) return HEAP32;

    if (type == {{{ 0x1406 - 0x1400/* GL_FLOAT */ }}}) return HEAPF32;

    if (type == {{{ 0x1405 - 0x1400 /* GL_UNSIGNED_INT */ }}}
      || type == {{{ 0x84FA - 0x1400 /* GL_UNSIGNED_INT_24_8_WEBGL/GL_UNSIGNED_INT_24_8 */ }}}
#if MAX_WEBGL_VERSION >= 2
      || type == {{{ 0x8368 - 0x1400 /* GL_UNSIGNED_INT_2_10_10_10_REV */ }}}
      || type == {{{ 0x8C3B - 0x1400 /* GL_UNSIGNED_INT_10F_11F_11F_REV */ }}}
      || type == {{{ 0x8C3E - 0x1400 /* GL_UNSIGNED_INT_5_9_9_9_REV */ }}}
#endif
      )
      return HEAPU32;

#if GL_ASSERTIONS
      if (type != {{{ 0x1403 - 0x1400 /* GL_UNSIGNED_SHORT */ }}}
#if MAX_WEBGL_VERSION >= 2
        && type != {{{ 0x140B - 0x1400 /* GL_HALF_FLOAT */ }}}
#endif
        && type != {{{ 0x8033 - 0x1400 /* GL_UNSIGNED_SHORT_4_4_4_4 */ }}}
        && type != {{{ 0x8034 - 0x1400 /* GL_UNSIGNED_SHORT_5_5_5_1 */ }}}
        && type != {{{ 0x8363 - 0x1400 /* GL_UNSIGNED_SHORT_5_6_5 */ }}}
        && type != {{{ 0x8D61 - 0x1400 /* GL_HALF_FLOAT_OES */ }}}) {
        err(`Invalid WebGL type 0x${(type+0x1400).toString()} passed to $heapObjectForWebGLType!`);
      }
#endif
    return HEAPU16;
  },

  $toTypedArrayIndex: (pointer, heap) =>
#if MEMORY64
    pointer / heap.BYTES_PER_ELEMENT,
#else
    pointer >>> (31 - Math.clz32(heap.BYTES_PER_ELEMENT)),
#endif

#if MIN_WEBGL_VERSION == 1
  $webgl_enable_ANGLE_instanced_arrays: (ctx) => {
    // Extension available in WebGL 1 from Firefox 26 and Google Chrome 30 onwards. Core feature in WebGL 2.
    var ext = ctx.getExtension('ANGLE_instanced_arrays');
    // Because this extension is a core function in WebGL 2, assign the extension entry points in place of
    // where the core functions will reside in WebGL 2. This way the calling code can call these without
    // having to dynamically branch depending if running against WebGL 1 or WebGL 2.
    if (ext) {
      ctx['vertexAttribDivisor'] = (index, divisor) => ext['vertexAttribDivisorANGLE'](index, divisor);
      ctx['drawArraysInstanced'] = (mode, first, count, primcount) => ext['drawArraysInstancedANGLE'](mode, first, count, primcount);
      ctx['drawElementsInstanced'] = (mode, count, type, indices, primcount) => ext['drawElementsInstancedANGLE'](mode, count, type, indices, primcount);
      return 1;
    }
  },

  emscripten_webgl_enable_ANGLE_instanced_arrays__deps: ['$webgl_enable_ANGLE_instanced_arrays'],
  emscripten_webgl_enable_ANGLE_instanced_arrays: (ctx) => webgl_enable_ANGLE_instanced_arrays(GL.contexts[ctx].GLctx),

  $webgl_enable_OES_vertex_array_object: (ctx) => {
    // Extension available in WebGL 1 from Firefox 25 and WebKit 536.28/desktop Safari 6.0.3 onwards. Core feature in WebGL 2.
    var ext = ctx.getExtension('OES_vertex_array_object');
    if (ext) {
      ctx['createVertexArray'] = () => ext['createVertexArrayOES']();
      ctx['deleteVertexArray'] = (vao) => ext['deleteVertexArrayOES'](vao);
      ctx['bindVertexArray'] = (vao) => ext['bindVertexArrayOES'](vao);
      ctx['isVertexArray'] = (vao) => ext['isVertexArrayOES'](vao);
      return 1;
    }
  },

  emscripten_webgl_enable_OES_vertex_array_object__deps: ['$webgl_enable_OES_vertex_array_object'],
  emscripten_webgl_enable_OES_vertex_array_object: (ctx) => webgl_enable_OES_vertex_array_object(GL.contexts[ctx].GLctx),

  $webgl_enable_WEBGL_draw_buffers: (ctx) => {
    // Extension available in WebGL 1 from Firefox 28 onwards. Core feature in WebGL 2.
    var ext = ctx.getExtension('WEBGL_draw_buffers');
    if (ext) {
      ctx['drawBuffers'] = (n, bufs) => ext['drawBuffersWEBGL'](n, bufs);
      return 1;
    }
  },

  emscripten_webgl_enable_WEBGL_draw_buffers__deps: ['$webgl_enable_WEBGL_draw_buffers'],
  emscripten_webgl_enable_WEBGL_draw_buffers: (ctx) => webgl_enable_WEBGL_draw_buffers(GL.contexts[ctx].GLctx),
#endif

  $webgl_enable_WEBGL_multi_draw: (ctx) => {
    // Closure is expected to be allowed to minify the '.multiDrawWebgl' property, so not accessing it quoted.
    return !!(ctx.multiDrawWebgl = ctx.getExtension('WEBGL_multi_draw'));
  },

  emscripten_webgl_enable_WEBGL_multi_draw__deps: ['$webgl_enable_WEBGL_multi_draw'],
  emscripten_webgl_enable_WEBGL_multi_draw: (ctx) => webgl_enable_WEBGL_multi_draw(GL.contexts[ctx].GLctx),

  $webgl_enable_EXT_polygon_offset_clamp: (ctx) => {
    return !!(ctx.extPolygonOffsetClamp = ctx.getExtension('EXT_polygon_offset_clamp'));
  },

  emscripten_webgl_enable_EXT_polygon_offset_clamp__deps: ['$webgl_enable_EXT_polygon_offset_clamp'],
  emscripten_webgl_enable_EXT_polygon_offset_clamp: (ctx) => webgl_enable_EXT_polygon_offset_clamp(GL.contexts[ctx].GLctx),

  $webgl_enable_EXT_clip_control: (ctx) => {
    return !!(ctx.extClipControl = ctx.getExtension('EXT_clip_control'));
  },

  emscripten_webgl_enable_EXT_clip_control__deps: ['$webgl_enable_EXT_clip_control'],
  emscripten_webgl_enable_EXT_clip_control: (ctx) => webgl_enable_EXT_clip_control(GL.contexts[ctx].GLctx),

  $webgl_enable_WEBGL_polygon_mode: (ctx) => {
    return !!(ctx.webglPolygonMode = ctx.getExtension('WEBGL_polygon_mode'));
  },

  emscripten_webgl_enable_WEBGL_polygon_mode__deps: ['$webgl_enable_WEBGL_polygon_mode'],
  emscripten_webgl_enable_WEBGL_polygon_mode: (ctx) => webgl_enable_WEBGL_polygon_mode(GL.contexts[ctx].GLctx),

  $getEmscriptenSupportedExtensions__internal: true,
  $getEmscriptenSupportedExtensions: (ctx) => {
    // Restrict the list of advertised extensions to those that we actually
    // support.
    var supportedExtensions = [
#if MIN_WEBGL_VERSION == 1
      // WebGL 1 extensions
      'ANGLE_instanced_arrays',
      'EXT_blend_minmax',
      'EXT_disjoint_timer_query',
      'EXT_frag_depth',
      'EXT_shader_texture_lod',
      'EXT_sRGB',
      'OES_element_index_uint',
      'OES_fbo_render_mipmap',
      'OES_standard_derivatives',
      'OES_texture_float',
      'OES_texture_half_float',
      'OES_texture_half_float_linear',
      'OES_vertex_array_object',
      'WEBGL_color_buffer_float',
      'WEBGL_depth_texture',
      'WEBGL_draw_buffers',
#endif
#if MAX_WEBGL_VERSION >= 2
      // WebGL 2 extensions
      'EXT_color_buffer_float',
      'EXT_conservative_depth',
      'EXT_disjoint_timer_query_webgl2',
      'EXT_texture_norm16',
      'NV_shader_noperspective_interpolation',
      'WEBGL_clip_cull_distance',
#endif
      // WebGL 1 and WebGL 2 extensions
      'EXT_clip_control',
      'EXT_color_buffer_half_float',
      'EXT_depth_clamp',
      'EXT_float_blend',
      'EXT_polygon_offset_clamp',
      'EXT_texture_compression_bptc',
      'EXT_texture_compression_rgtc',
      'EXT_texture_filter_anisotropic',
      'KHR_parallel_shader_compile',
      'OES_texture_float_linear',
      'WEBGL_blend_func_extended',
      'WEBGL_compressed_texture_astc',
      'WEBGL_compressed_texture_etc',
      'WEBGL_compressed_texture_etc1',
      'WEBGL_compressed_texture_s3tc',
      'WEBGL_compressed_texture_s3tc_srgb',
      'WEBGL_debug_renderer_info',
      'WEBGL_debug_shaders',
      'WEBGL_lose_context',
      'WEBGL_multi_draw',
      'WEBGL_polygon_mode'
    ];
    // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
    return (ctx.getSupportedExtensions() || []).filter(ext => supportedExtensions.includes(ext));
  },

  $GLctx__internal: true,
  $GLctx: undefined,
  $GL__deps: [
    '$GLctx',
#if GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS
  // If GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS is enabled, GL.initExtensions() will call to initialize these.
#if PTHREADS
    'malloc', // Needed by registerContext
    'free', // Needed by deleteContext
#endif
#if MIN_WEBGL_VERSION == 1
    '$webgl_enable_ANGLE_instanced_arrays',
    '$webgl_enable_OES_vertex_array_object',
    '$webgl_enable_WEBGL_draw_buffers',
#endif
#if MAX_WEBGL_VERSION >= 2
    '$webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance',
    '$webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance',
#endif
    '$webgl_enable_EXT_polygon_offset_clamp',
    '$webgl_enable_EXT_clip_control',
    '$webgl_enable_WEBGL_polygon_mode',
    '$webgl_enable_WEBGL_multi_draw',
    '$getEmscriptenSupportedExtensions',
#endif // GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS
#if FULL_ES2 || LEGACY_GL_EMULATION
    '$registerPreMainLoop',
#endif
  ],
#if FULL_ES2 || LEGACY_GL_EMULATION
  $GL__postset: `
    // Signal GL rendering layer that processing of a new frame is about to
    // start. This helps it optimize VBO double-buffering and reduce GPU stalls.
    registerPreMainLoop(() => GL.newRenderingFrameStarted());
  `,
#endif
  $GL: {
#if GL_DEBUG
    debug: true,
#endif

/* We do not depend on the exact initial values of falsey member fields - these
   fields can be populated on-demand to save code size.
   (but still documented here to keep track of what is supposed to be present)
#if GL_TRACK_ERRORS
    lastError: 0,
#endif
    currentContext: null,

#if FULL_ES2 || LEGACY_GL_EMULATION
    currArrayBuffer: 0,
    currElementArrayBuffer: 0,
#endif
*/

    counter: 1, // 0 is reserved as 'null' in gl
    buffers: [],
#if FULL_ES3
    mappedBuffers: {},
#endif
    programs: [],
    framebuffers: [],
    renderbuffers: [],
    textures: [],
    shaders: [],
    vaos: [],
#if PTHREADS // with pthreads a context is a location in memory with some synchronized data between threads
    contexts: {},
#else            // without pthreads, it's just an integer ID
    contexts: [],
#endif
    // DOM ID -> OffscreenCanvas mappings of <canvas> elements that have their
    // rendering control transferred to offscreen.
    offscreenCanvases: {},
    // on WebGL1 stores WebGLTimerQueryEXT, on WebGL2 WebGLQuery
    queries: [],
#if MAX_WEBGL_VERSION >= 2
    samplers: [],
    transformFeedbacks: [],
    syncs: [],
#endif

#if FULL_ES2 || LEGACY_GL_EMULATION
    byteSizeByTypeRoot: 0x1400, // GL_BYTE
    byteSizeByType: [
      1, // GL_BYTE
      1, // GL_UNSIGNED_BYTE
      2, // GL_SHORT
      2, // GL_UNSIGNED_SHORT
      4, // GL_INT
      4, // GL_UNSIGNED_INT
      4, // GL_FLOAT
      2, // GL_2_BYTES
      3, // GL_3_BYTES
      4, // GL_4_BYTES
      8  // GL_DOUBLE
    ],
#endif

    stringCache: {},
#if MAX_WEBGL_VERSION >= 2
    stringiCache: {},
#endif

    unpackAlignment: 4, // default alignment is 4 bytes
    unpackRowLength: 0,

    // Records a GL error condition that occurred, stored until user calls
    // glGetError() to fetch it. As per GLES2 spec, only the first error is
    // remembered, and subsequent errors are discarded until the user has
    // cleared the stored error by a call to glGetError().
    recordError: (errorCode) => {
#if GL_TRACK_ERRORS
      if (!GL.lastError) {
        GL.lastError = errorCode;
      }
#endif
    },
    // Get a new ID for a texture/buffer/etc., while keeping the table dense and
    // fast. Creation is fairly rare so it is worth optimizing lookups later.
    getNewId: (table) => {
      var ret = GL.counter++;
      for (var i = table.length; i < ret; i++) {
        table[i] = null;
      }
      return ret;
    },

    // The code path for creating textures, buffers, framebuffers and other
    // objects the same (and not in fast path), so we merge the functions
    // together.
    // 'createFunction' refers to the WebGL context function name to do the actual
    // creation, 'objectTable' points to the GL object table where to populate the
    // created objects, and 'functionName' carries the name of the caller for
    // debug information.
    genObject: (n, buffers, createFunction, objectTable
#if GL_ASSERTIONS
      , functionName
#endif
      ) => {
      for (var i = 0; i < n; i++) {
        var buffer = GLctx[createFunction]();
        var id = buffer && GL.getNewId(objectTable);
        if (buffer) {
          buffer.name = id;
          objectTable[id] = buffer;
        } else {
          GL.recordError(0x502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
          err(`GL_INVALID_OPERATION in ${functionName}: GLctx.${createFunction} returned null - most likely GL context is lost!`);
#endif
        }
        {{{ makeSetValue('buffers', 'i*4', 'id', 'i32') }}};
      }
    },

#if FULL_ES2 || LEGACY_GL_EMULATION
    // When user GL code wants to render from client-side memory, we need to
    // upload the vertex data to a temp VBO for rendering. Maintain a set of
    // temp VBOs that are created-on-demand to appropriate sizes, and never
    // destroyed.  Also, for best performance the VBOs are double-buffered, i.e.
    // every second frame we switch the set of VBOs we upload to, so that
    // rendering from the previous frame is not disturbed by uploading from new
    // data to it, which could cause a GPU-CPU pipeline stall.
    // Note that index buffers are not double-buffered (at the moment) in this
    // manner.
    MAX_TEMP_BUFFER_SIZE: {{{ GL_MAX_TEMP_BUFFER_SIZE }}},
    // Maximum number of temp VBOs of one size to maintain, after that we start
    // reusing old ones, which is safe but can give a performance impact. If
    // CPU-GPU stalls are a problem, increasing this might help.
    numTempVertexBuffersPerSize: 64, // (const)

    // Precompute a lookup table for the function ceil(log2(x)), i.e. how many
    // bits are needed to represent x, or, if x was rounded up to next pow2,
    // which index is the single '1' bit at?
    // Then log2ceilLookup[x] returns ceil(log2(x)).
    log2ceilLookup: (i) => 32 - Math.clz32(i === 0 ? 0 : i - 1),

    generateTempBuffers: (quads, context) => {
      var largestIndex = GL.log2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);
      context.tempVertexBufferCounters1 = [];
      context.tempVertexBufferCounters2 = [];
      context.tempVertexBufferCounters1.length = context.tempVertexBufferCounters2.length = largestIndex+1;
      context.tempVertexBuffers1 = [];
      context.tempVertexBuffers2 = [];
      context.tempVertexBuffers1.length = context.tempVertexBuffers2.length = largestIndex+1;
      context.tempIndexBuffers = [];
      context.tempIndexBuffers.length = largestIndex+1;
      for (var i = 0; i <= largestIndex; ++i) {
        context.tempIndexBuffers[i] = null; // Created on-demand
        context.tempVertexBufferCounters1[i] = context.tempVertexBufferCounters2[i] = 0;
        var ringbufferLength = GL.numTempVertexBuffersPerSize;
        context.tempVertexBuffers1[i] = [];
        context.tempVertexBuffers2[i] = [];
        var ringbuffer1 = context.tempVertexBuffers1[i];
        var ringbuffer2 = context.tempVertexBuffers2[i];
        ringbuffer1.length = ringbuffer2.length = ringbufferLength;
        for (var j = 0; j < ringbufferLength; ++j) {
          ringbuffer1[j] = ringbuffer2[j] = null; // Created on-demand
        }
      }

      if (quads) {
        // GL_QUAD indexes can be precalculated
        context.tempQuadIndexBuffer = GLctx.createBuffer();
        context.GLctx.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, context.tempQuadIndexBuffer);
        var numIndexes = GL.MAX_TEMP_BUFFER_SIZE >> 1;
        var quadIndexes = new Uint16Array(numIndexes);
        var i = 0, v = 0;
        while (1) {
          quadIndexes[i++] = v;
          if (i >= numIndexes) break;
          quadIndexes[i++] = v+1;
          if (i >= numIndexes) break;
          quadIndexes[i++] = v+2;
          if (i >= numIndexes) break;
          quadIndexes[i++] = v;
          if (i >= numIndexes) break;
          quadIndexes[i++] = v+2;
          if (i >= numIndexes) break;
          quadIndexes[i++] = v+3;
          if (i >= numIndexes) break;
          v += 4;
        }
        context.GLctx.bufferData(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, quadIndexes, 0x88E4 /*GL_STATIC_DRAW*/);
        context.GLctx.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, null);
      }
    },

    getTempVertexBuffer: (sizeBytes) => {
      var idx = GL.log2ceilLookup(sizeBytes);
      var ringbuffer = GL.currentContext.tempVertexBuffers1[idx];
      var nextFreeBufferIndex = GL.currentContext.tempVertexBufferCounters1[idx];
      GL.currentContext.tempVertexBufferCounters1[idx] = (GL.currentContext.tempVertexBufferCounters1[idx]+1) & (GL.numTempVertexBuffersPerSize-1);
      var vbo = ringbuffer[nextFreeBufferIndex];
      if (vbo) {
        return vbo;
      }
      var prevVBO = GLctx.getParameter(0x8894 /*GL_ARRAY_BUFFER_BINDING*/);
      ringbuffer[nextFreeBufferIndex] = GLctx.createBuffer();
      GLctx.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, ringbuffer[nextFreeBufferIndex]);
      GLctx.bufferData(0x8892 /*GL_ARRAY_BUFFER*/, 1 << idx, 0x88E8 /*GL_DYNAMIC_DRAW*/);
      GLctx.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, prevVBO);
      return ringbuffer[nextFreeBufferIndex];
    },

    getTempIndexBuffer: (sizeBytes) => {
      var idx = GL.log2ceilLookup(sizeBytes);
      var ibo = GL.currentContext.tempIndexBuffers[idx];
      if (ibo) {
        return ibo;
      }
      var prevIBO = GLctx.getParameter(0x8895 /*ELEMENT_ARRAY_BUFFER_BINDING*/);
      GL.currentContext.tempIndexBuffers[idx] = GLctx.createBuffer();
      GLctx.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, GL.currentContext.tempIndexBuffers[idx]);
      GLctx.bufferData(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, 1 << idx, 0x88E8 /*GL_DYNAMIC_DRAW*/);
      GLctx.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, prevIBO);
      return GL.currentContext.tempIndexBuffers[idx];
    },

    // Called at start of each new WebGL rendering frame. This swaps the
    // doublebuffered temp VB memory pointers, so that every second frame
    // utilizes different set of temp buffers. The aim is to keep the set of
    // buffers being rendered, and the set of buffers being updated disjoint.
    newRenderingFrameStarted: () => {
      if (!GL.currentContext) {
        return;
      }
      var vb = GL.currentContext.tempVertexBuffers1;
      GL.currentContext.tempVertexBuffers1 = GL.currentContext.tempVertexBuffers2;
      GL.currentContext.tempVertexBuffers2 = vb;
      vb = GL.currentContext.tempVertexBufferCounters1;
      GL.currentContext.tempVertexBufferCounters1 = GL.currentContext.tempVertexBufferCounters2;
      GL.currentContext.tempVertexBufferCounters2 = vb;
      var largestIndex = GL.log2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);
      for (var i = 0; i <= largestIndex; ++i) {
        GL.currentContext.tempVertexBufferCounters1[i] = 0;
      }
    },
#endif

    getSource: (shader, count, string, length) => {
      var source = '';
      for (var i = 0; i < count; ++i) {
        var len = length ? {{{ makeGetValue('length', 'i*' + POINTER_SIZE, '*') }}} : undefined;
        source += UTF8ToString({{{ makeGetValue('string', 'i*' + POINTER_SIZE, '*') }}}, len);
      }
#if LEGACY_GL_EMULATION
      // Let's see if we need to enable the standard derivatives extension
      var type = GLctx.getShaderParameter(GL.shaders[shader], 0x8B4F /* GL_SHADER_TYPE */);
      if (type == 0x8B30 /* GL_FRAGMENT_SHADER */) {
        if (GLEmulation.findToken(source, "dFdx") ||
            GLEmulation.findToken(source, "dFdy") ||
            GLEmulation.findToken(source, "fwidth")) {
          source = "#extension GL_OES_standard_derivatives : enable\n" + source;
          var extension = GLctx.getExtension("OES_standard_derivatives");
#if GL_DEBUG
          if (!extension) {
            dbg("Shader attempts to use the standard derivatives extension which is not available.");
          }
#endif
        }
      }
#endif
      return source;
    },

#if GL_FFP_ONLY
    enabledClientAttribIndices: [],
    enableVertexAttribArray: (index) => {
      if (!GL.enabledClientAttribIndices[index]) {
        GL.enabledClientAttribIndices[index] = true;
        GLctx.enableVertexAttribArray(index);
      }
    },
    disableVertexAttribArray: (index) => {
      if (GL.enabledClientAttribIndices[index]) {
        GL.enabledClientAttribIndices[index] = false;
        GLctx.disableVertexAttribArray(index);
      }
    },
#endif

#if FULL_ES2
    calcBufLength: (size, type, stride, count) => {
      if (stride > 0) {
        return count * stride;  // XXXvlad this is not exactly correct I don't think
      }
      var typeSize = GL.byteSizeByType[type - GL.byteSizeByTypeRoot];
      return size * typeSize * count;
    },

    usedTempBuffers: [],

    preDrawHandleClientVertexAttribBindings: (count) => {
      GL.resetBufferBinding = false;

      // TODO: initial pass to detect ranges we need to upload, might not need
      // an upload per attrib
      for (var i = 0; i < GL.currentContext.maxVertexAttribs; ++i) {
        var cb = GL.currentContext.clientBuffers[i];
        if (!cb.clientside || !cb.enabled) continue;

        GL.resetBufferBinding = true;

        var size = GL.calcBufLength(cb.size, cb.type, cb.stride, count);
        var buf = GL.getTempVertexBuffer(size);
        GLctx.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, buf);
        GLctx.bufferSubData(0x8892 /*GL_ARRAY_BUFFER*/,
                                 0,
                                 HEAPU8.subarray(cb.ptr, cb.ptr + size));
#if GL_ASSERTIONS
        GL.validateVertexAttribPointer(cb.size, cb.type, cb.stride, 0);
#endif
        cb.vertexAttribPointerAdaptor.call(GLctx, i, cb.size, cb.type, cb.normalized, cb.stride, 0);
      }
    },

    postDrawHandleClientVertexAttribBindings: () => {
      if (GL.resetBufferBinding) {
        GLctx.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, GL.buffers[GLctx.currentArrayBufferBinding]);
      }
    },
#endif

#if GL_ASSERTIONS
    validateGLObjectID: (objectHandleArray, objectID, callerFunctionName, objectReadableType) => {
      if (objectID != 0) {
        if (objectHandleArray[objectID] === null) {
          err(`${callerFunctionName} called with an already deleted ${objectReadableType} ID ${objectID}!`);
        } else if (!(objectID in objectHandleArray)) {
          err(`${callerFunctionName} called with a nonexisting ${objectReadableType} ID ${objectID}!`);
        }
      }
    },
    // Validates that user obeys GL spec #6.4: http://www.khronos.org/registry/webgl/specs/latest/1.0/#6.4
    validateVertexAttribPointer: (dimension, dataType, stride, offset) => {
      var sizeBytes = 1;
      switch (dataType) {
        case 0x1400 /* GL_BYTE */:
        case 0x1401 /* GL_UNSIGNED_BYTE */:
          sizeBytes = 1;
          break;
        case 0x1402 /* GL_SHORT */:
        case 0x1403 /* GL_UNSIGNED_SHORT */:
          sizeBytes = 2;
          break;
        case 0x1404 /* GL_INT */:
        case 0x1405 /* GL_UNSIGNED_INT */:
        case 0x1406 /* GL_FLOAT */:
          sizeBytes = 4;
          break;
        case 0x140A /* GL_DOUBLE */:
          sizeBytes = 8;
          break;
        default:
#if MAX_WEBGL_VERSION >= 2
          if ({{{ isCurrentContextWebGL2() }}}) {
            if (dataType == 0x8368 /* GL_UNSIGNED_INT_2_10_10_10_REV */ || dataType == 0x8D9F /* GL_INT_2_10_10_10_REV */) {
              sizeBytes = 4;
              break;
            } else if (dataType == 0x140B /* GL_HALF_FLOAT */) {
              sizeBytes = 2;
              break;
            } else {
              // else fall through
            }
          }
#endif
          err(`Invalid vertex attribute data type GLenum ${dataType} passed to GL function!`);
      }
      if (dimension == 0x80E1 /* GL_BGRA */) {
        err('WebGL does not support size=GL_BGRA in a call to glVertexAttribPointer! Please use size=4 and type=GL_UNSIGNED_BYTE instead!');
      } else if (dimension < 1 || dimension > 4) {
        err(`Invalid dimension=${dimension} in call to glVertexAttribPointer, must be 1,2,3 or 4.`);
      }
      if (stride < 0 || stride > 255) {
        err(`Invalid stride=${stride} in call to glVertexAttribPointer. Note that maximum supported stride in WebGL is 255!`);
      }
      if (offset % sizeBytes != 0) {
        err(`GL spec section 6.4 error: vertex attribute data offset of ${offset} bytes should have been a multiple of the data type size that was used: GLenum ${dataType} has size of ${sizeBytes} bytes!`);
      }
      if (stride % sizeBytes != 0) {
        err(`GL spec section 6.4 error: vertex attribute data stride of ${stride} bytes should have been a multiple of the data type size that was used: GLenum ${dataType} has size of ${sizeBytes} bytes!`);
      }
    },
#endif

#if TRACE_WEBGL_CALLS
    hookWebGLFunction: (f, glCtx) => {
      var orig = glCtx[f];
      var contextHandle = glCtx.canvas.GLctxObject.handle;
      glCtx[f] = function(...args) {
        var ret = orig.apply(this, args);
        // Some GL functions take a view of the entire linear memory.  Replace
        // such arguments with the string 'HEAP' to avoid serializing all of
        // memory.
        for (var i in args) {
          if (ArrayBuffer.isView(args[i]) && args[i].byteLength === HEAPU8.byteLength) {
            args[i] = 'HEAP';
          }
        }
#if PTHREADS
        err(`[Thread ${_pthread_self()}, GL ctx: ${contextHandle}]: ${f}(${args}) -> ${ret}`);
#else
        err(`[ctx: ${contextHandle}]: ${f}(${args}) -> ${ret}`);
#endif
        return ret;
      };
    },

    hookWebGL: function(glCtx) {
      glCtx ??= this.detectWebGLContext();
      if (!glCtx) return;
      if (!((typeof WebGLRenderingContext != 'undefined' && glCtx instanceof WebGLRenderingContext)
            || (typeof WebGL2RenderingContext != 'undefined' && glCtx instanceof WebGL2RenderingContext))) {
        return;
      }

      if (glCtx.webGlTracerAlreadyHooked) return;
      glCtx.webGlTracerAlreadyHooked = true;

      for (var f in glCtx) {
        if (typeof glCtx[f] == 'function') {
          this.hookWebGLFunction(f, glCtx);
        }
      }
    },
#endif
    // Returns the context handle to the new context.
    createContext: (/** @type {HTMLCanvasElement} */ canvas, webGLContextAttributes) => {
#if OFFSCREEN_FRAMEBUFFER
      // In proxied operation mode, rAF()/setTimeout() functions do not delimit
      // frame boundaries, so can't have WebGL implementation try to detect when
      // it's ok to discard contents of the rendered backbuffer.
      if (webGLContextAttributes.renderViaOffscreenBackBuffer) webGLContextAttributes['preserveDrawingBuffer'] = true;
#endif

#if GL_TESTING
      webGLContextAttributes['preserveDrawingBuffer'] = true;
#endif

#if MAX_WEBGL_VERSION >= 2 && MIN_CHROME_VERSION <= 57
      // BUG: Workaround Chrome WebGL 2 issue: the first shipped versions of
      // WebGL 2 in Chrome 57 did not actually implement the new garbage free
      // WebGL 2 entry points that take an offset and a length to an existing
      // heap (instead of having to create a completely new heap view). In
      // Chrome the entry points only were added in to Chrome 58 and newer. For
      // Chrome 57 (and older), disable WebGL 2 support altogether.
      function getChromeVersion() {
        var chromeVersion = navigator.userAgent.match(/Chrom(e|ium)\/([0-9]+)\./);
        if (chromeVersion) return chromeVersion[2]|0;
        // If not chrome, fall through to return undefined. (undefined <= integer will yield false)
      }
#endif

#if GL_DEBUG
      var errorInfo = '?';
      function onContextCreationError(event) {
        errorInfo = event.statusMessage || errorInfo;
      }
      canvas.addEventListener('webglcontextcreationerror', onContextCreationError, false);
#endif

#if GL_PREINITIALIZED_CONTEXT
      // If WebGL context has already been preinitialized for the page on the JS
      // side, reuse that context instead. This is useful for example when the
      // main page precompiles shaders for the application, in which case the
      // WebGL context is created already before any Emscripten compiled code
      // has been downloaded.
      if (Module['preinitializedWebGLContext']) {
        var ctx = Module['preinitializedWebGLContext'];
#if MAX_WEBGL_VERSION >= 2
        // The ctx object may not be of a known class (e.g. it may be a debug
        // wrapper), so we ask it for its version rather than use instanceof.
        webGLContextAttributes.majorVersion = Number(ctx.getParameter(ctx.VERSION).match(/^WebGL (\d+).\d+/)[1]);
#else
        webGLContextAttributes.majorVersion = 1;
#endif
      } else {
#endif

#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED && GL_WORKAROUND_SAFARI_GETCONTEXT_BUG
      // BUG: Workaround Safari WebGL issue: After successfully acquiring WebGL
      // context on a canvas, calling .getContext() will always return that
      // context independent of which 'webgl' or 'webgl2'
      // context version was passed. See:
      //   https://bugs.webkit.org/show_bug.cgi?id=222758
      // and:
      //   https://github.com/emscripten-core/emscripten/issues/13295.
      // TODO: Once the bug is fixed and shipped in Safari, adjust the Safari
      // version field in above check.
      if (!canvas.getContextSafariWebGL2Fixed) {
        canvas.getContextSafariWebGL2Fixed = canvas.getContext;
        /** @type {function(this:HTMLCanvasElement, string, (Object|null)=): (Object|null)} */
        function fixedGetContext(ver, attrs) {
          var gl = canvas.getContextSafariWebGL2Fixed(ver, attrs);
          return ((ver == 'webgl') == (gl instanceof WebGLRenderingContext)) ? gl : null;
        }
        canvas.getContext = fixedGetContext;
      }
#endif

#if MIN_WEBGL_VERSION >= 2
      var ctx = canvas.getContext("webgl2", webGLContextAttributes);
#else
      var ctx =
#if MAX_WEBGL_VERSION >= 2
        (webGLContextAttributes.majorVersion > 1)
        ?
#if MIN_CHROME_VERSION <= 57
          !(getChromeVersion() <= 57) && canvas.getContext("webgl2", webGLContextAttributes)
#else
          canvas.getContext("webgl2", webGLContextAttributes)
#endif
        :
#endif
        (canvas.getContext("webgl", webGLContextAttributes)
          // https://caniuse.com/#feat=webgl
#if MIN_FIREFOX_VERSION <= 23 || MIN_CHROME_VERSION <= 32 || MIN_SAFARI_VERSION <= 70101
          || canvas.getContext("experimental-webgl", webGLContextAttributes)
#endif
          );
#endif // MAX_WEBGL_VERSION >= 2

#if GL_PREINITIALIZED_CONTEXT
      }
#endif

#if GL_DEBUG
      canvas.removeEventListener('webglcontextcreationerror', onContextCreationError, false);
      if (!ctx) {
        dbg('Could not create canvas: ' + [errorInfo, JSON.stringify(webGLContextAttributes)]);
        return 0;
      }
#else
      if (!ctx) return 0;
#endif

      var handle = GL.registerContext(ctx, webGLContextAttributes);

#if TRACE_WEBGL_CALLS
      GL.hookWebGL(ctx);
#endif

#if GL_DISABLE_HALF_FLOAT_EXTENSION_IF_BROKEN
      const disableHalfFloatExtensionIfBroken = (ctx) => {
        var t = ctx.createTexture();
        ctx.bindTexture(0xDE1/*GL_TEXTURE_2D*/, t);
        for (var i = 0; i < 8 && ctx.getError(); ++i) /*no-op*/;
        var ext = ctx.getExtension('OES_texture_half_float');
        if (!ext) return; // no half-float extension - nothing needed to fix.
        // Bug on Safari on iOS and macOS: texImage2D() and texSubImage2D() do
        // not allow uploading pixel data to half float textures, rendering them
        // useless.
        // See https://bugs.webkit.org/show_bug.cgi?id=183321, https://bugs.webkit.org/show_bug.cgi?id=169999,
        // https://stackoverflow.com/questions/54248633/cannot-create-half-float-oes-texture-from-uint16array-on-ipad
        ctx.texImage2D(0xDE1/*GL_TEXTURE_2D*/, 0, 0x1908/*GL_RGBA*/, 1, 1, 0, 0x1908/*GL_RGBA*/, 0x8d61/*HALF_FLOAT_OES*/, new Uint16Array(4));
        var broken = ctx.getError();
        ctx.bindTexture(0xDE1/*GL_TEXTURE_2D*/, null);
        ctx.deleteTexture(t);
        if (broken) {
          ctx.realGetSupportedExtensions = ctx.getSupportedExtensions;
          ctx.getSupportedExtensions = function() {
#if GL_ASSERTIONS
            warnOnce('Removed broken support for half-float textures. See e.g. https://bugs.webkit.org/show_bug.cgi?id=183321');
#endif
            // .getSupportedExtensions() can return null if context is lost, so
            // coerce to empty array.
            return (this.realGetSupportedExtensions() || []).filter((ext) => !ext.includes('texture_half_float'));
          }
        }
      }
      disableHalfFloatExtensionIfBroken(ctx);
#endif

      return handle;
    },

#if OFFSCREEN_FRAMEBUFFER
    enableOffscreenFramebufferAttributes: (webGLContextAttributes) => {
      webGLContextAttributes.renderViaOffscreenBackBuffer = true;
      webGLContextAttributes.preserveDrawingBuffer = true;
    },

    // If WebGL is being proxied from a pthread to the main thread, we can't
    // directly render to the WebGL default back buffer because of WebGL's
    // implicit swap behavior. Therefore in such modes, create an offscreen
    // render target surface to which rendering is performed to, and finally
    // flipped to the main screen.
    createOffscreenFramebuffer: (context) => {
      var gl = context.GLctx;

      // Create FBO
      var fbo = gl.createFramebuffer();
      gl.bindFramebuffer(0x8D40 /*GL_FRAMEBUFFER*/, fbo);
      context.defaultFbo = fbo;

#if MAX_WEBGL_VERSION >= 2
      context.defaultFboForbidBlitFramebuffer = false;
      if (gl.getContextAttributes().antialias) {
        context.defaultFboForbidBlitFramebuffer = true;
      }
#if MIN_FIREFOX_VERSION < 67
      else {
        // The WebGL 2 blit path doesn't work in Firefox < 67 (except in fullscreen).
        // https://bugzilla.mozilla.org/show_bug.cgi?id=1523030
        var firefoxMatch = navigator.userAgent.toLowerCase().match(/firefox\/(\d\d)/);
        if (firefoxMatch != null) {
          var firefoxVersion = firefoxMatch[1];
          context.defaultFboForbidBlitFramebuffer = firefoxVersion < 67;
        }
      }
#endif
#endif

      // Create render targets to the FBO
      context.defaultColorTarget = gl.createTexture();
      context.defaultDepthTarget = gl.createRenderbuffer();
      // Size them up correctly (use the same mechanism when resizing on demand)
      GL.resizeOffscreenFramebuffer(context);

      gl.bindTexture(0xDE1 /*GL_TEXTURE_2D*/, context.defaultColorTarget);
      gl.texParameteri(0xDE1 /*GL_TEXTURE_2D*/, 0x2801 /*GL_TEXTURE_MIN_FILTER*/, 0x2600 /*GL_NEAREST*/);
      gl.texParameteri(0xDE1 /*GL_TEXTURE_2D*/, 0x2800 /*GL_TEXTURE_MAG_FILTER*/, 0x2600 /*GL_NEAREST*/);
      gl.texParameteri(0xDE1 /*GL_TEXTURE_2D*/, 0x2802 /*GL_TEXTURE_WRAP_S*/, 0x812F /*GL_CLAMP_TO_EDGE*/);
      gl.texParameteri(0xDE1 /*GL_TEXTURE_2D*/, 0x2803 /*GL_TEXTURE_WRAP_T*/, 0x812F /*GL_CLAMP_TO_EDGE*/);
      gl.texImage2D(0xDE1 /*GL_TEXTURE_2D*/, 0, 0x1908 /*GL_RGBA*/, gl.canvas.width, gl.canvas.height, 0, 0x1908 /*GL_RGBA*/, 0x1401 /*GL_UNSIGNED_BYTE*/, null);
      gl.framebufferTexture2D(0x8D40 /*GL_FRAMEBUFFER*/, 0x8CE0 /*GL_COLOR_ATTACHMENT0*/, 0xDE1 /*GL_TEXTURE_2D*/, context.defaultColorTarget, 0);
      gl.bindTexture(0xDE1 /*GL_TEXTURE_2D*/, null);

      // Create depth render target to the FBO
      var depthTarget = gl.createRenderbuffer();
      gl.bindRenderbuffer(0x8D41 /*GL_RENDERBUFFER*/, context.defaultDepthTarget);
      gl.renderbufferStorage(0x8D41 /*GL_RENDERBUFFER*/, 0x81A5 /*GL_DEPTH_COMPONENT16*/, gl.canvas.width, gl.canvas.height);
      gl.framebufferRenderbuffer(0x8D40 /*GL_FRAMEBUFFER*/, 0x8D00 /*GL_DEPTH_ATTACHMENT*/, 0x8D41 /*GL_RENDERBUFFER*/, context.defaultDepthTarget);
      gl.bindRenderbuffer(0x8D41 /*GL_RENDERBUFFER*/, null);

      // Create blitter
      var vertices = [
        -1, -1,
        -1,  1,
         1, -1,
         1,  1
      ];
      var vb = gl.createBuffer();
      gl.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, vb);
      gl.bufferData(0x8892 /*GL_ARRAY_BUFFER*/, new Float32Array(vertices), 0x88E4 /*GL_STATIC_DRAW*/);
      gl.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, null);
      context.blitVB = vb;

      var vsCode =
        'attribute vec2 pos;' +
        'varying lowp vec2 tex;' +
        'void main() { tex = pos * 0.5 + vec2(0.5,0.5); gl_Position = vec4(pos, 0.0, 1.0); }';
      var vs = gl.createShader(0x8B31 /*GL_VERTEX_SHADER*/);
      gl.shaderSource(vs, vsCode);
      gl.compileShader(vs);

      var fsCode =
        'varying lowp vec2 tex;' +
        'uniform sampler2D sampler;' +
        'void main() { gl_FragColor = texture2D(sampler, tex); }';
      var fs = gl.createShader(0x8B30 /*GL_FRAGMENT_SHADER*/);
      gl.shaderSource(fs, fsCode);
      gl.compileShader(fs);

      var blitProgram = gl.createProgram();
      gl.attachShader(blitProgram, vs);
      gl.attachShader(blitProgram, fs);
      gl.linkProgram(blitProgram);
      context.blitProgram = blitProgram;
      context.blitPosLoc = gl.getAttribLocation(blitProgram, "pos");
      gl.useProgram(blitProgram);
      gl.uniform1i(gl.getUniformLocation(blitProgram, "sampler"), 0);
      gl.useProgram(null);

      context.defaultVao = undefined;
      if (gl.createVertexArray) {
        context.defaultVao = gl.createVertexArray();
        gl.bindVertexArray(context.defaultVao);
        gl.enableVertexAttribArray(context.blitPosLoc);
        gl.bindVertexArray(null);
      }
    },

    resizeOffscreenFramebuffer: (context) => {
      var gl = context.GLctx;

      // Resize color buffer
      if (context.defaultColorTarget) {
        var prevTextureBinding = gl.getParameter(0x8069 /*GL_TEXTURE_BINDING_2D*/);
        gl.bindTexture(0xDE1 /*GL_TEXTURE_2D*/, context.defaultColorTarget);
        gl.texImage2D(0xDE1 /*GL_TEXTURE_2D*/, 0, 0x1908 /*GL_RGBA*/, gl.drawingBufferWidth, gl.drawingBufferHeight, 0, 0x1908 /*GL_RGBA*/, 0x1401 /*GL_UNSIGNED_BYTE*/, null);
        gl.bindTexture(0xDE1 /*GL_TEXTURE_2D*/, prevTextureBinding);
      }

      // Resize depth buffer
      if (context.defaultDepthTarget) {
        var prevRenderBufferBinding = gl.getParameter(0x8CA7 /*GL_RENDERBUFFER_BINDING*/);
        gl.bindRenderbuffer(0x8D41 /*GL_RENDERBUFFER*/, context.defaultDepthTarget);
        gl.renderbufferStorage(0x8D41 /*GL_RENDERBUFFER*/, 0x81A5 /*GL_DEPTH_COMPONENT16*/, gl.drawingBufferWidth, gl.drawingBufferHeight); // TODO: Read context creation parameters for what type of depth and stencil to use
        gl.bindRenderbuffer(0x8D41 /*GL_RENDERBUFFER*/, prevRenderBufferBinding);
      }
    },

    // Renders the contents of the offscreen render target onto the visible screen.
    blitOffscreenFramebuffer: (context) => {
      var gl = context.GLctx;

      var prevScissorTest = gl.getParameter(0xC11 /*GL_SCISSOR_TEST*/);
      if (prevScissorTest) gl.disable(0xC11 /*GL_SCISSOR_TEST*/);

      var prevFbo = gl.getParameter(0x8CA6 /*GL_FRAMEBUFFER_BINDING*/);

#if MAX_WEBGL_VERSION >= 2
      if (gl.blitFramebuffer && !context.defaultFboForbidBlitFramebuffer) {
        gl.bindFramebuffer(0x8CA8 /*GL_READ_FRAMEBUFFER*/, context.defaultFbo);
        gl.bindFramebuffer(0x8CA9 /*GL_DRAW_FRAMEBUFFER*/, null);
        gl.blitFramebuffer(0, 0, gl.canvas.width, gl.canvas.height,
                           0, 0, gl.canvas.width, gl.canvas.height,
                           0x4000 /*GL_COLOR_BUFFER_BIT*/, 0x2600/*GL_NEAREST*/);
      }
      else
#endif
      {
        gl.bindFramebuffer(0x8D40 /*GL_FRAMEBUFFER*/, null);

        var prevProgram = gl.getParameter(0x8B8D /*GL_CURRENT_PROGRAM*/);
        gl.useProgram(context.blitProgram);

        var prevVB = gl.getParameter(0x8894 /*GL_ARRAY_BUFFER_BINDING*/);
        gl.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, context.blitVB);

        var prevActiveTexture = gl.getParameter(0x84E0 /*GL_ACTIVE_TEXTURE*/);
        gl.activeTexture(0x84C0 /*GL_TEXTURE0*/);

        var prevTextureBinding = gl.getParameter(0x8069 /*GL_TEXTURE_BINDING_2D*/);
        gl.bindTexture(0xDE1 /*GL_TEXTURE_2D*/, context.defaultColorTarget);

        var prevBlend = gl.getParameter(0xBE2 /*GL_BLEND*/);
        if (prevBlend) gl.disable(0xBE2 /*GL_BLEND*/);

        var prevCullFace = gl.getParameter(0xB44 /*GL_CULL_FACE*/);
        if (prevCullFace) gl.disable(0xB44 /*GL_CULL_FACE*/);

        var prevDepthTest = gl.getParameter(0xB71 /*GL_DEPTH_TEST*/);
        if (prevDepthTest) gl.disable(0xB71 /*GL_DEPTH_TEST*/);

        var prevStencilTest = gl.getParameter(0xB90 /*GL_STENCIL_TEST*/);
        if (prevStencilTest) gl.disable(0xB90 /*GL_STENCIL_TEST*/);

        function draw() {
          gl.vertexAttribPointer(context.blitPosLoc, 2, 0x1406 /*GL_FLOAT*/, false, 0, 0);
          gl.drawArrays(5/*GL_TRIANGLE_STRIP*/, 0, 4);
        }

#if !OFFSCREEN_FRAMEBUFFER_FORBID_VAO_PATH
        if (context.defaultVao) {
          // WebGL 2 or OES_vertex_array_object
          var prevVAO = gl.getParameter(0x85B5 /*GL_VERTEX_ARRAY_BINDING*/);
          gl.bindVertexArray(context.defaultVao);
          draw();
          gl.bindVertexArray(prevVAO);
        }
        else
#endif
        {
          var prevVertexAttribPointer = {
            buffer: gl.getVertexAttrib(context.blitPosLoc, 0x889F /*GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING*/),
            size: gl.getVertexAttrib(context.blitPosLoc, 0x8623 /*GL_VERTEX_ATTRIB_ARRAY_SIZE*/),
            stride: gl.getVertexAttrib(context.blitPosLoc, 0x8624 /*GL_VERTEX_ATTRIB_ARRAY_STRIDE*/),
            type: gl.getVertexAttrib(context.blitPosLoc, 0x8625 /*GL_VERTEX_ATTRIB_ARRAY_TYPE*/),
            normalized: gl.getVertexAttrib(context.blitPosLoc, 0x886A /*GL_VERTEX_ATTRIB_ARRAY_NORMALIZED*/),
            pointer: gl.getVertexAttribOffset(context.blitPosLoc, 0x8645 /*GL_VERTEX_ATTRIB_ARRAY_POINTER*/),
          };
          var maxVertexAttribs = gl.getParameter(0x8869 /*GL_MAX_VERTEX_ATTRIBS*/);
          var prevVertexAttribEnables = [];
          for (var i = 0; i < maxVertexAttribs; ++i) {
            var prevEnabled = gl.getVertexAttrib(i, 0x8622 /*GL_VERTEX_ATTRIB_ARRAY_ENABLED*/);
            var wantEnabled = i == context.blitPosLoc;
            if (prevEnabled && !wantEnabled) {
              gl.disableVertexAttribArray(i);
            }
            if (!prevEnabled && wantEnabled) {
              gl.enableVertexAttribArray(i);
            }
            prevVertexAttribEnables[i] = prevEnabled;
          }

          draw();

          for (var i = 0; i < maxVertexAttribs; ++i) {
            var prevEnabled = prevVertexAttribEnables[i];
            var nowEnabled = i == context.blitPosLoc;
            if (prevEnabled && !nowEnabled) {
              gl.enableVertexAttribArray(i);
            }
            if (!prevEnabled && nowEnabled) {
              gl.disableVertexAttribArray(i);
            }
          }
          gl.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, prevVertexAttribPointer.buffer);
          gl.vertexAttribPointer(context.blitPosLoc,
                                 prevVertexAttribPointer.size,
                                 prevVertexAttribPointer.type,
                                 prevVertexAttribPointer.normalized,
                                 prevVertexAttribPointer.stride,
                                 prevVertexAttribPointer.offset);
        }

        if (prevStencilTest) gl.enable(0xB90 /*GL_STENCIL_TEST*/);
        if (prevDepthTest) gl.enable(0xB71 /*GL_DEPTH_TEST*/);
        if (prevCullFace) gl.enable(0xB44 /*GL_CULL_FACE*/);
        if (prevBlend) gl.enable(0xBE2 /*GL_BLEND*/);

        gl.bindTexture(0xDE1 /*GL_TEXTURE_2D*/, prevTextureBinding);
        gl.activeTexture(prevActiveTexture);
        gl.bindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, prevVB);
        gl.useProgram(prevProgram);
      }
      gl.bindFramebuffer(0x8D40 /*GL_FRAMEBUFFER*/, prevFbo);
      if (prevScissorTest) gl.enable(0xC11 /*GL_SCISSOR_TEST*/);
    },
#endif

    registerContext: (ctx, webGLContextAttributes) => {
#if PTHREADS
      // with pthreads a context is a location in memory with some synchronized
      // data between threads
      var handle = _malloc({{{ 2 * POINTER_SIZE }}});
#if GL_ASSERTIONS
      assert(handle, 'malloc() failed in GL.registerContext!');
#endif
#if GL_SUPPORT_EXPLICIT_SWAP_CONTROL
      {{{ makeSetValue('handle', 0, 'webGLContextAttributes.explicitSwapControl', 'i8')}}};
#endif
      {{{ makeSetValue('handle', POINTER_SIZE, '_pthread_self()', '*')}}}; // the thread pointer of the thread that owns the control of the context
#else // PTHREADS
      // without pthreads a context is just an integer ID
      var handle = GL.getNewId(GL.contexts);
#endif // PTHREADS

      var context = {
        handle,
        attributes: webGLContextAttributes,
        version: webGLContextAttributes.majorVersion,
        GLctx: ctx
      };

      // Store the created context object so that we can access the context
      // given a canvas without having to pass the parameters again.
      if (ctx.canvas) ctx.canvas.GLctxObject = context;
      GL.contexts[handle] = context;
#if GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS
      if (typeof webGLContextAttributes.enableExtensionsByDefault == 'undefined' || webGLContextAttributes.enableExtensionsByDefault) {
        GL.initExtensions(context);
      }
#endif

#if FULL_ES2
      context.maxVertexAttribs = context.GLctx.getParameter(0x8869 /*GL_MAX_VERTEX_ATTRIBS*/);
      context.clientBuffers = [];
      for (var i = 0; i < context.maxVertexAttribs; i++) {
        context.clientBuffers[i] = {
          enabled: false,
          clientside: false,
          size: 0,
          type: 0,
          normalized: 0,
          stride: 0,
          ptr: 0,
          vertexAttribPointerAdaptor: null,
        };
      }

      GL.generateTempBuffers(false, context);
#endif

#if OFFSCREEN_FRAMEBUFFER
      if (webGLContextAttributes.renderViaOffscreenBackBuffer) GL.createOffscreenFramebuffer(context);
#else

#if GL_DEBUG
      if (webGLContextAttributes.renderViaOffscreenBackBuffer) {
        dbg('renderViaOffscreenBackBuffer=true specified in WebGL context creation attributes, pass linker flag -sOFFSCREEN_FRAMEBUFFER to enable support!');
      }
#endif

#endif
      return handle;
    },

    makeContextCurrent: (contextHandle) => {
#if GL_DEBUG
      if (contextHandle && !GL.contexts[contextHandle]) {
#if PTHREADS
        dbg(`GL.makeContextCurrent() failed! WebGL context ${contextHandle} does not exist, or was created on another thread!`);
#else
        dbg(`GL.makeContextCurrent() failed! WebGL context ${contextHandle} does not exist!`);
#endif
      }
#endif

      // Active Emscripten GL layer context object.
      GL.currentContext = GL.contexts[contextHandle];
      // Active WebGL context object.
      Module.ctx = GLctx = GL.currentContext?.GLctx;
      return !(contextHandle && !GLctx);
    },

    getContext: (contextHandle) => {
      return GL.contexts[contextHandle];
    },

    deleteContext: (contextHandle) => {
      if (GL.currentContext === GL.contexts[contextHandle]) {
        GL.currentContext = null;
      }
      if (typeof JSEvents == 'object') {
        // Release all JS event handlers on the DOM element that the GL context is
        // associated with since the context is now deleted.
        JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas);
      }
      // Make sure the canvas object no longer refers to the context object so
      // there are no GC surprises.
      if (GL.contexts[contextHandle] && GL.contexts[contextHandle].GLctx.canvas) {
        GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined;
      }
#if PTHREADS
      _free(GL.contexts[contextHandle].handle);
#endif
      GL.contexts[contextHandle] = null;
    },

#if GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS
    // In WebGL, extensions must be explicitly enabled to be active, see
    // http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.14.14
    // In GLES2, all extensions are enabled by default without additional
    // operations. Init all extensions we need to give to GLES2 user code here,
    // so that GLES2 code can operate without changing behavior.
    initExtensions: (context) => {
      // If this function is called without a specific context object, init the
      // extensions of the currently active context.
      context ||= GL.currentContext;

      if (context.initExtensionsDone) return;
      context.initExtensionsDone = true;

      var GLctx = context.GLctx;

      // Detect the presence of a few extensions manually, ction GL interop
      // layer itself will need to know if they exist.
#if LEGACY_GL_EMULATION
      context.compressionExt = GLctx.getExtension('WEBGL_compressed_texture_s3tc');
      context.anisotropicExt = GLctx.getExtension('EXT_texture_filter_anisotropic');
#endif

      // Extensions that are available in both WebGL 1 and WebGL 2
      webgl_enable_WEBGL_multi_draw(GLctx);
      webgl_enable_EXT_polygon_offset_clamp(GLctx);
      webgl_enable_EXT_clip_control(GLctx);
      webgl_enable_WEBGL_polygon_mode(GLctx);
#if MIN_WEBGL_VERSION == 1
      // Extensions that are only available in WebGL 1 (the calls will be no-ops
      // if called on a WebGL 2 context active)
      webgl_enable_ANGLE_instanced_arrays(GLctx);
      webgl_enable_OES_vertex_array_object(GLctx);
      webgl_enable_WEBGL_draw_buffers(GLctx);
#endif
#if MAX_WEBGL_VERSION >= 2
      // Extensions that are available from WebGL >= 2 (no-op if called on a WebGL 1 context active)
      webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);
      webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GLctx);

      // On WebGL 2, EXT_disjoint_timer_query is replaced with an alternative
      // that's based on core APIs, and exposes only the queryCounterEXT()
      // entrypoint.
      if (context.version >= 2) {
        GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query_webgl2");
      }

      // However, Firefox exposes the WebGL 1 version on WebGL 2 as well and
      // thus we look for the WebGL 1 version again if the WebGL 2 version
      // isn't present. https://bugzilla.mozilla.org/show_bug.cgi?id=1328882
      if (context.version < 2 || !GLctx.disjointTimerQueryExt)
#endif
      {
        GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query");
      }

      getEmscriptenSupportedExtensions(GLctx).forEach((ext) => {
        // WEBGL_lose_context, WEBGL_debug_renderer_info and WEBGL_debug_shaders
        // are not enabled by default.
        if (!ext.includes('lose_context') && !ext.includes('debug')) {
          // Call .getExtension() to enable that extension permanently.
          GLctx.getExtension(ext);
        }
      });
    },
#endif

  },

  $webglGetExtensions__internal: true,
  $webglGetExtensions__deps: ['$getEmscriptenSupportedExtensions'],
  $webglGetExtensions() {
    var exts = getEmscriptenSupportedExtensions(GLctx);
#if GL_EXTENSIONS_IN_PREFIXED_FORMAT
    exts = exts.concat(exts.map((e) => "GL_" + e));
#endif
    return exts;
  },

  glPixelStorei: (pname, param) => {
    if (pname == {{{ cDefs.GL_UNPACK_ALIGNMENT }}}) {
      GL.unpackAlignment = param;
    } else if (pname == {{{ cDefs.GL_UNPACK_ROW_LENGTH }}}) {
      GL.unpackRowLength = param;
    }
    GLctx.pixelStorei(pname, param);
  },

  glGetString__deps: ['$stringToNewUTF8', '$webglGetExtensions'],
  glGetString: (name_) => {
    var ret = GL.stringCache[name_];
    if (!ret) {
      switch (name_) {
        case 0x1F03 /* GL_EXTENSIONS */:
          ret = stringToNewUTF8(webglGetExtensions().join(' '));
          break;
        case 0x1F00 /* GL_VENDOR */:
        case 0x1F01 /* GL_RENDERER */:
        case 0x9245 /* UNMASKED_VENDOR_WEBGL */:
        case 0x9246 /* UNMASKED_RENDERER_WEBGL */:
#if !GL_EMULATE_GLES_VERSION_STRING_FORMAT
        case 0x1F02 /* GL_VERSION */:
        case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
#endif
          var s = GLctx.getParameter(name_);
#if GL_TRACK_ERRORS
          if (!s) {
            GL.recordError(0x500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
            // This occurs e.g. if one attempts GL_UNMASKED_VENDOR_WEBGL when it is not supported.
            err(`GL_INVALID_ENUM in glGetString: Received empty parameter for query name ${name_}!`);
#endif
          }
#endif
          ret = s ? stringToNewUTF8(s) : 0;
          break;

#if GL_EMULATE_GLES_VERSION_STRING_FORMAT
        case 0x1F02 /* GL_VERSION */:
          var webGLVersion = GLctx.getParameter(0x1F02 /*GL_VERSION*/);
          // return GLES version string corresponding to the version of the WebGL context
          var glVersion = `OpenGL ES 2.0 (${webGLVersion})`;
#if MAX_WEBGL_VERSION >= 2
          if ({{{ isCurrentContextWebGL2() }}}) glVersion = `OpenGL ES 3.0 (${webGLVersion})`;
#endif
          ret = stringToNewUTF8(glVersion);
          break;
        case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
          var glslVersion = GLctx.getParameter(0x8B8C /*GL_SHADING_LANGUAGE_VERSION*/);
          // extract the version number 'N.M' from the string 'WebGL GLSL ES N.M ...'
          var ver_re = /^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;
          var ver_num = glslVersion.match(ver_re);
          if (ver_num !== null) {
            if (ver_num[1].length == 3) ver_num[1] = ver_num[1] + '0'; // ensure minor version has 2 digits
            glslVersion = `OpenGL ES GLSL ES ${ver_num[1]} (${glslVersion})`;
          }
          ret = stringToNewUTF8(glslVersion);
          break;
#endif
#if GL_TRACK_ERRORS
        default:
          GL.recordError(0x500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
          err(`GL_INVALID_ENUM in glGetString: Unknown parameter ${name_}!`);
#endif
#endif
          // fall through
      }
      GL.stringCache[name_] = ret;
    }
    return ret;
  },

  $emscriptenWebGLGet__deps: ['$writeI53ToI64',
#if MAX_WEBGL_VERSION >= 2
    '$webglGetExtensions', // For GL_NUM_EXTENSIONS
#endif
  ],
  $emscriptenWebGLGet: (name_, p, type) => {
    // Guard against user passing a null pointer.
    // Note that GLES2 spec does not say anything about how passing a null
    // pointer should be treated.  Testing on desktop core GL 3, the application
    // crashes on glGetIntegerv to a null pointer, but better to report an error
    // instead of doing anything random.
    if (!p) {
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGet${type}v(name=${name_}: Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    var ret = undefined;
    switch (name_) { // Handle a few trivial GLES values
      case 0x8DFA: // GL_SHADER_COMPILER
        ret = 1;
        break;
      case 0x8DF8: // GL_SHADER_BINARY_FORMATS
#if GL_TRACK_ERRORS
        if (type != {{{ cDefs.EM_FUNC_SIG_PARAM_I }}} && type != {{{ cDefs.EM_FUNC_SIG_PARAM_J }}}) {
          GL.recordError(0x500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
          err(`GL_INVALID_ENUM in glGet${type}v(GL_SHADER_BINARY_FORMATS): Invalid parameter type!`);
#endif
        }
#endif
        // Do not write anything to the out pointer, since no binary formats are
        // supported.
        return;
#if MAX_WEBGL_VERSION >= 2
      case 0x87FE: // GL_NUM_PROGRAM_BINARY_FORMATS
#endif
      case 0x8DF9: // GL_NUM_SHADER_BINARY_FORMATS
        ret = 0;
        break;
      case 0x86A2: // GL_NUM_COMPRESSED_TEXTURE_FORMATS
        // WebGL doesn't have GL_NUM_COMPRESSED_TEXTURE_FORMATS (it's obsolete
        // since GL_COMPRESSED_TEXTURE_FORMATS returns a JS array that can be
        // queried for length), so implement it ourselves to allow C++ GLES2
        // code get the length.
        var formats = GLctx.getParameter(0x86A3 /*GL_COMPRESSED_TEXTURE_FORMATS*/);
        ret = formats ? formats.length : 0;
        break;
#if GL_EXPLICIT_UNIFORM_LOCATION
      case 0x826E: // GL_MAX_UNIFORM_LOCATIONS
        // This is an arbitrary limit, must be large enough to allow practical
        // use, but small enough to still keep a range for automatic uniform
        // locations, which get assigned numbers larger than this.
        ret = 1048576;
        break;
#endif

#if MAX_WEBGL_VERSION >= 2
      case 0x821D: // GL_NUM_EXTENSIONS
#if GL_TRACK_ERRORS
        if (GL.currentContext.version < 2) {
          // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
          GL.recordError(0x502 /* GL_INVALID_OPERATION */);
          return;
        }
#endif
        ret = webglGetExtensions().length;
        break;
      case 0x821B: // GL_MAJOR_VERSION
      case 0x821C: // GL_MINOR_VERSION
#if GL_TRACK_ERRORS
        if (GL.currentContext.version < 2) {
          GL.recordError(0x500); // GL_INVALID_ENUM
          return;
        }
#endif
        ret = name_ == 0x821B ? 3 : 0; // return version 3.0
        break;
#endif // ~MAX_WEBGL_VERSION >= 2
    }

    if (ret === undefined) {
      var result = GLctx.getParameter(name_);
      switch (typeof result) {
        case "number":
          ret = result;
          break;
        case "boolean":
          ret = result ? 1 : 0;
          break;
        case "string":
          GL.recordError(0x500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
          err(`GL_INVALID_ENUM in glGet${type}v(${name}) on a name which returns a string!`);
#endif
          return;
        case "object":
          if (result === null) {
            // null is a valid result for some (e.g., which buffer is bound -
            // perhaps nothing is bound), but otherwise can mean an invalid
            // name_, which we need to report as an error
            switch (name_) {
              case 0x8894: // ARRAY_BUFFER_BINDING
              case 0x8B8D: // CURRENT_PROGRAM
              case 0x8895: // ELEMENT_ARRAY_BUFFER_BINDING
              case 0x8CA6: // FRAMEBUFFER_BINDING or DRAW_FRAMEBUFFER_BINDING
              case 0x8CA7: // RENDERBUFFER_BINDING
              case 0x8069: // TEXTURE_BINDING_2D
              case 0x85B5: // WebGL 2 GL_VERTEX_ARRAY_BINDING, or WebGL 1 extension OES_vertex_array_object GL_VERTEX_ARRAY_BINDING_OES
#if MAX_WEBGL_VERSION >= 2
              case 0x8F36: // COPY_READ_BUFFER_BINDING or COPY_READ_BUFFER
              case 0x8F37: // COPY_WRITE_BUFFER_BINDING or COPY_WRITE_BUFFER
              case 0x88ED: // PIXEL_PACK_BUFFER_BINDING
              case 0x88EF: // PIXEL_UNPACK_BUFFER_BINDING
              case 0x8CAA: // READ_FRAMEBUFFER_BINDING
              case 0x8919: // SAMPLER_BINDING
              case 0x8C1D: // TEXTURE_BINDING_2D_ARRAY
              case 0x806A: // TEXTURE_BINDING_3D
              case 0x8E25: // TRANSFORM_FEEDBACK_BINDING
              case 0x8C8F: // TRANSFORM_FEEDBACK_BUFFER_BINDING
              case 0x8A28: // UNIFORM_BUFFER_BINDING
#endif
              case 0x8514: { // TEXTURE_BINDING_CUBE_MAP
                ret = 0;
                break;
              }
              default: {
                GL.recordError(0x500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
                err(`GL_INVALID_ENUM in glGet${type}v(${name}) and it returns null!`);
#endif
                return;
              }
            }
          } else if (result instanceof Float32Array ||
                     result instanceof Uint32Array ||
                     result instanceof Int32Array ||
                     result instanceof Array) {
            for (var i = 0; i < result.length; ++i) {
              switch (type) {
                case {{{ cDefs.EM_FUNC_SIG_PARAM_I }}}: {{{ makeSetValue('p', 'i*4', 'result[i]', 'i32') }}}; break;
                case {{{ cDefs.EM_FUNC_SIG_PARAM_F }}}: {{{ makeSetValue('p', 'i*4', 'result[i]', 'float') }}}; break;
                case {{{ cDefs.EM_FUNC_SIG_PARAM_B }}}: {{{ makeSetValue('p', 'i',   'result[i] ? 1 : 0', 'i8') }}}; break;
#if GL_ASSERTIONS
                default: throw `internal glGet error, bad type: ${type}`;
#endif
              }
            }
            return;
          } else {
#if GL_TRACK_ERRORS
            try {
#endif
              ret = result.name | 0;
#if GL_TRACK_ERRORS
            } catch(e) {
              GL.recordError(0x500); // GL_INVALID_ENUM
              err(`GL_INVALID_ENUM in glGet${type}v: Unknown object returned from WebGL getParameter(${name_})! (error: ${e})`);
              return;
            }
#endif
          }
          break;
#if GL_TRACK_ERRORS
        default:
          GL.recordError(0x500); // GL_INVALID_ENUM
          err(`GL_INVALID_ENUM in glGet${type}v: Native code calling glGet${type}v(${name_}) and it returns ${result} of type ${typeof(result)}!`);
          return;
#endif
      }
    }

    switch (type) {
      case {{{ cDefs.EM_FUNC_SIG_PARAM_J }}}: writeI53ToI64(p, ret); break;
      case {{{ cDefs.EM_FUNC_SIG_PARAM_I }}}: {{{ makeSetValue('p', '0', 'ret', 'i32') }}}; break;
      case {{{ cDefs.EM_FUNC_SIG_PARAM_F }}}:   {{{ makeSetValue('p', '0', 'ret', 'float') }}}; break;
      case {{{ cDefs.EM_FUNC_SIG_PARAM_B }}}: {{{ makeSetValue('p', '0', 'ret ? 1 : 0', 'i8') }}}; break;
#if GL_ASSERTIONS
      default: throw `internal glGet error, bad type: ${type}`;
#endif
    }
  },

  glGetIntegerv__deps: ['$emscriptenWebGLGet'],
  glGetIntegerv: (name_, p) => emscriptenWebGLGet(name_, p, {{{ cDefs.EM_FUNC_SIG_PARAM_I }}}),

  glGetFloatv__deps: ['$emscriptenWebGLGet'],
  glGetFloatv: (name_, p) => emscriptenWebGLGet(name_, p, {{{ cDefs.EM_FUNC_SIG_PARAM_F }}}),

  glGetBooleanv__deps: ['$emscriptenWebGLGet'],
  glGetBooleanv: (name_, p) => emscriptenWebGLGet(name_, p, {{{ cDefs.EM_FUNC_SIG_PARAM_B }}}),

  glDeleteTextures: (n, textures) => {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('textures', 'i*4', 'i32') }}};
      var texture = GL.textures[id];
      // GL spec: "glDeleteTextures silently ignores 0s and names that do not
      // correspond to existing textures".
      if (!texture) continue;
      GLctx.deleteTexture(texture);
      texture.name = 0;
      GL.textures[id] = null;
    }
  },

  glCompressedTexImage2D: (target, level, internalFormat, width, height, border, imageSize, data) => {
    // `data` may be null here, which means "allocate uniniitalized space but
    // don't upload" in GLES parlance, but `compressedTexImage2D` requires the
    // final data parameter, so we simply pass a heap view starting at zero
    // effectively uploading whatever happens to be near address zero.  See
    // https://github.com/emscripten-core/emscripten/issues/19300.
#if MAX_WEBGL_VERSION >= 2
    if ({{{ isCurrentContextWebGL2() }}}) {
      if (GLctx.currentPixelUnpackBufferBinding || !imageSize) {
        GLctx.compressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data);
        return;
      }
#if WEBGL_USE_GARBAGE_FREE_APIS
      GLctx.compressedTexImage2D(target, level, internalFormat, width, height, border, HEAPU8, data, imageSize);
      return;
#endif
    }
#endif
#if INCLUDE_WEBGL1_FALLBACK
    GLctx.compressedTexImage2D(target, level, internalFormat, width, height, border, {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}});
#endif
  },


  glCompressedTexSubImage2D: (target, level, xoffset, yoffset, width, height, format, imageSize, data) => {
#if MAX_WEBGL_VERSION >= 2
    if ({{{ isCurrentContextWebGL2() }}}) {
      if (GLctx.currentPixelUnpackBufferBinding || !imageSize) {
        GLctx.compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
        return;
      }
#if WEBGL_USE_GARBAGE_FREE_APIS
      GLctx.compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, HEAPU8, data, imageSize);
      return;
#endif
    }
#endif
#if INCLUDE_WEBGL1_FALLBACK
    GLctx.compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}});
#endif
  },

  $computeUnpackAlignedImageSize: (width, height, sizePerPixel) => {
    function roundedToNextMultipleOf(x, y) {
#if GL_ASSERTIONS
      assert((y & (y-1)) === 0, 'Unpack alignment must be a power of 2! (Allowed values per WebGL spec are 1, 2, 4 or 8)');
#endif
      return (x + y - 1) & -y;
    }
    var plainRowSize = (GL.unpackRowLength || width) * sizePerPixel;
    var alignedRowSize = roundedToNextMultipleOf(plainRowSize, GL.unpackAlignment);
    return height * alignedRowSize;
  },

  $colorChannelsInGlTextureFormat: (format) => {
    // Micro-optimizations for size: map format to size by subtracting smallest
    // enum value (0x1902) from all values first.  Also omit the most common
    // size value (1) from the list, which is assumed by formats not on the
    // list.
    var colorChannels = {
      // 0x1902 /* GL_DEPTH_COMPONENT */ - 0x1902: 1,
      // 0x1906 /* GL_ALPHA */ - 0x1902: 1,
      {{{ 0x1907 /* GL_RGB */ - 0x1902 }}}: 3,
      {{{ 0x1908 /* GL_RGBA */ - 0x1902 }}}: 4,
      // 0x1909 /* GL_LUMINANCE */ - 0x1902: 1,
      {{{ 0x190A /*GL_LUMINANCE_ALPHA*/ - 0x1902 }}}: 2,
      {{{ 0x8C40 /*(GL_SRGB_EXT)*/ - 0x1902 }}}: 3,
      {{{ 0x8C42 /*(GL_SRGB_ALPHA_EXT*/ - 0x1902 }}}: 4,
#if MAX_WEBGL_VERSION >= 2
      // 0x1903 /* GL_RED */ - 0x1902: 1,
      {{{ 0x8227 /*GL_RG*/ - 0x1902 }}}: 2,
      {{{ 0x8228 /*GL_RG_INTEGER*/ - 0x1902 }}}: 2,
      // 0x8D94 /* GL_RED_INTEGER */ - 0x1902: 1,
      {{{ 0x8D98 /*GL_RGB_INTEGER*/ - 0x1902 }}}: 3,
      {{{ 0x8D99 /*GL_RGBA_INTEGER*/ - 0x1902 }}}: 4
#endif
    };
#if GL_ASSERTIONS
    if (!colorChannels[format - 0x1902]
      && format != 0x1902 /* GL_DEPTH_COMPONENT */
      && format != 0x1906 /* GL_ALPHA */
      && format != 0x1909 /* GL_LUMINANCE */
      && format != 0x1903 /* GL_RED */
      && format != 0x8D94 /* GL_RED_INTEGER */) {
      err(`Invalid format=${ptrToString(format)} passed to function colorChannelsInGlTextureFormat()!`);
    }
#endif
    return colorChannels[format - 0x1902]||1;
  },

  $emscriptenWebGLGetTexPixelData__deps: ['$computeUnpackAlignedImageSize', '$colorChannelsInGlTextureFormat', '$heapObjectForWebGLType', '$toTypedArrayIndex'],
  $emscriptenWebGLGetTexPixelData: (type, format, width, height, pixels, internalFormat) => {
    var heap = heapObjectForWebGLType(type);
    var sizePerPixel = colorChannelsInGlTextureFormat(format) * heap.BYTES_PER_ELEMENT;
    var bytes = computeUnpackAlignedImageSize(width, height, sizePerPixel);
#if GL_ASSERTIONS
    assert(pixels % heap.BYTES_PER_ELEMENT == 0, 'Pointer to texture data passed to texture get function must be aligned to the byte size of the pixel type!');
#endif
    return heap.subarray(toTypedArrayIndex(pixels, heap), toTypedArrayIndex(pixels + bytes, heap));
  },

  glTexImage2D__deps: ['$emscriptenWebGLGetTexPixelData'
#if MAX_WEBGL_VERSION >= 2
                       , '$heapObjectForWebGLType', '$toTypedArrayIndex'
#endif
  ],
  glTexImage2D: (target, level, internalFormat, width, height, border, format, type, pixels) => {
#if MAX_WEBGL_VERSION >= 2
#if WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION
    if ({{{ isCurrentContextWebGL2() }}}) {
      // WebGL 1 unsized texture internalFormats are no longer supported in
      // WebGL 2, so patch those format enums to the ones that are present in
      // WebGL 2.
      if (format == 0x1902/*GL_DEPTH_COMPONENT*/ && internalFormat == 0x1902/*GL_DEPTH_COMPONENT*/ && type == 0x1405/*GL_UNSIGNED_INT*/) {
        internalFormat = 0x81A6 /*GL_DEPTH_COMPONENT24*/;
      }
      if (type == 0x8d61/*GL_HALF_FLOAT_OES*/) {
        type = 0x140B /*GL_HALF_FLOAT*/;
        if (format == 0x1908/*GL_RGBA*/ && internalFormat == 0x1908/*GL_RGBA*/) {
          internalFormat = 0x881A/*GL_RGBA16F*/;
        }
      }
      if (internalFormat == 0x84f9 /*GL_DEPTH_STENCIL*/) {
        internalFormat = 0x88F0 /*GL_DEPTH24_STENCIL8*/;
      }
      if (internalFormat == 0x1908 /*GL_RGBA*/ && type == 0x1406 /*GL_FLOAT*/) {
        internalFormat = 0x8814 /*GL_RGBA32F*/;
      }
    }
#endif
    if ({{{ isCurrentContextWebGL2() }}}) {
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
        return;
      }
#if WEBGL_USE_GARBAGE_FREE_APIS
      if (pixels) {
        var heap = heapObjectForWebGLType(type);
        var index = toTypedArrayIndex(pixels, heap);
        GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, heap, index);
        return;
      }
#endif
    }
#endif
    var pixelData = pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat) : null;
    GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixelData);
  },

  glTexSubImage2D__deps: ['$emscriptenWebGLGetTexPixelData'
#if MAX_WEBGL_VERSION >= 2
                          , '$heapObjectForWebGLType', '$toTypedArrayIndex'
#endif
  ],
  glTexSubImage2D: (target, level, xoffset, yoffset, width, height, format, type, pixels) => {
#if MAX_WEBGL_VERSION >= 2
#if WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION
    if ({{{ isCurrentContextWebGL2() }}}) {
      // In WebGL 1 to do half float textures, one uses the type enum
      // GL_HALF_FLOAT_OES, but in WebGL 2 when half float textures were adopted
      // to the core spec, the enum changed value which breaks backwards
      // compatibility. Route old enum number to the new one.
      if (type == 0x8d61/*GL_HALF_FLOAT_OES*/) type = 0x140B /*GL_HALF_FLOAT*/;
    }
#endif
    if ({{{ isCurrentContextWebGL2() }}}) {
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
        return;
      }
#if WEBGL_USE_GARBAGE_FREE_APIS
      if (pixels) {
        var heap = heapObjectForWebGLType(type);
        GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, heap, toTypedArrayIndex(pixels, heap));
        return;
      }
#endif
    }
#endif
    var pixelData = pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, 0) : null;
    GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixelData);
  },

  glReadPixels__deps: ['$emscriptenWebGLGetTexPixelData'
#if MAX_WEBGL_VERSION >= 2
                       , '$heapObjectForWebGLType', '$toTypedArrayIndex'
#endif
  ],
  glReadPixels: (x, y, width, height, format, type, pixels) => {
#if MAX_WEBGL_VERSION >= 2
    if ({{{ isCurrentContextWebGL2() }}}) {
      if (GLctx.currentPixelPackBufferBinding) {
        GLctx.readPixels(x, y, width, height, format, type, pixels);
        return;
      }
#if WEBGL_USE_GARBAGE_FREE_APIS
      var heap = heapObjectForWebGLType(type);
      var target = toTypedArrayIndex(pixels, heap);
      GLctx.readPixels(x, y, width, height, format, type, heap, target);
      return;
#endif
    }
#endif
#if INCLUDE_WEBGL1_FALLBACK
    var pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, format);
    if (!pixelData) {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
      err(`GL_INVALID_ENUM in glReadPixels: Unrecognized combination of type=${type} and format=${format}!`);
#endif
      return;
    }
    GLctx.readPixels(x, y, width, height, format, type, pixelData);
#endif
  },

  glBindTexture: (target, texture) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.textures, texture, 'glBindTexture', 'texture');
#endif
    GLctx.bindTexture(target, GL.textures[texture]);
  },

  glGetTexParameterfv: (target, pname, params) => {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null
      // pointer. Since calling this function does not make sense if p == null,
      // issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetTexParameterfv(target=${target}, pname=${pname}, params=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('params', '0', 'GLctx.getTexParameter(target, pname)', 'float') }}};
  },

  glGetTexParameteriv: (target, pname, params) => {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null
      // pointer. Since calling this function does not make sense if p == null,
      // issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetTexParameteriv(target=${target}, pname=${pname}, params=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('params', '0', 'GLctx.getTexParameter(target, pname)', 'i32') }}};
  },

  glTexParameterfv: (target, pname, params) => {
    var param = {{{ makeGetValue('params', '0', 'float') }}};
    GLctx.texParameterf(target, pname, param);
  },

  glTexParameteriv: (target, pname, params) => {
    var param = {{{ makeGetValue('params', '0', 'i32') }}};
    GLctx.texParameteri(target, pname, param);
  },

  glIsTexture: (id) => {
    var texture = GL.textures[id];
    if (!texture) return 0;
    return GLctx.isTexture(texture);
  },

  glGenBuffers: (n, buffers) => {
    GL.genObject(n, buffers, 'createBuffer', GL.buffers
#if GL_ASSERTIONS
    , 'glGenBuffers'
#endif
      );
  },

  glGenTextures: (n, textures) => {
    GL.genObject(n, textures, 'createTexture', GL.textures
#if GL_ASSERTIONS
    , 'glGenTextures'
#endif
      );
  },

  glDeleteBuffers: (n, buffers) => {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('buffers', 'i*4', 'i32') }}};
      var buffer = GL.buffers[id];

      // From spec: "glDeleteBuffers silently ignores 0's and names that do not
      // correspond to existing buffer objects."
      if (!buffer) continue;

      GLctx.deleteBuffer(buffer);
      buffer.name = 0;
      GL.buffers[id] = null;

#if FULL_ES2 || LEGACY_GL_EMULATION
      if (id == GLctx.currentArrayBufferBinding) GLctx.currentArrayBufferBinding = 0;
      if (id == GLctx.currentElementArrayBufferBinding) GLctx.currentElementArrayBufferBinding = 0;
#endif
#if MAX_WEBGL_VERSION >= 2
      if (id == GLctx.currentPixelPackBufferBinding) GLctx.currentPixelPackBufferBinding = 0;
      if (id == GLctx.currentPixelUnpackBufferBinding) GLctx.currentPixelUnpackBufferBinding = 0;
#endif
    }
  },

  glGetBufferParameteriv: (target, value, data) => {
    if (!data) {
      // GLES2 specification does not specify how to behave if data is a null
      // pointer. Since calling this function does not make sense if data ==
      // null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetBufferParameteriv(target=${target}, value=${value}, data=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('data', '0', 'GLctx.getBufferParameter(target, value)', 'i32') }}};
  },

  glBufferData: (target, size, data, usage) => {
#if LEGACY_GL_EMULATION
    switch (usage) { // fix usages, WebGL 1 only has *_DRAW
      case 0x88E1: // GL_STREAM_READ
      case 0x88E2: // GL_STREAM_COPY
        usage = 0x88E0; // GL_STREAM_DRAW
        break;
      case 0x88E5: // GL_STATIC_READ
      case 0x88E6: // GL_STATIC_COPY
        usage = 0x88E4; // GL_STATIC_DRAW
        break;
      case 0x88E9: // GL_DYNAMIC_READ
      case 0x88EA: // GL_DYNAMIC_COPY
        usage = 0x88E8; // GL_DYNAMIC_DRAW
        break;
    }
#endif

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      // If size is zero, WebGL would interpret uploading the whole input
      // arraybuffer (starting from given offset), which would not make sense in
      // WebAssembly, so avoid uploading if size is zero. However we must still
      // call bufferData to establish a backing storage of zero bytes.
      if (data && size) {
        GLctx.bufferData(target, HEAPU8, usage, data, size);
      } else {
        GLctx.bufferData(target, size, usage);
      }
      return;
    }
#endif
#if INCLUDE_WEBGL1_FALLBACK
    // N.b. here first form specifies a heap subarray, second form an integer
    // size, so the ?: code here is polymorphic. It is advised to avoid
    // randomly mixing both uses in calling code, to avoid any potential JS
    // engine JIT issues.
    GLctx.bufferData(target, data ? HEAPU8.subarray(data, data+size) : size, usage);
#endif
  },

  glBufferSubData: (target, offset, size, data) => {
#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      size && GLctx.bufferSubData(target, offset, HEAPU8, data, size);
      return;
    }
#endif
#if INCLUDE_WEBGL1_FALLBACK
    GLctx.bufferSubData(target, offset, HEAPU8.subarray(data, data+size));
#endif
  },

  // Queries EXT
  glGenQueriesEXT__sig: 'vip',
  glGenQueriesEXT: (n, ids) => {
    for (var i = 0; i < n; i++) {
      var query = GLctx.disjointTimerQueryExt['createQueryEXT']();
      if (!query) {
        GL.recordError(0x502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
        err('GL_INVALID_OPERATION in glGenQueriesEXT: GLctx.disjointTimerQueryExt.createQueryEXT returned null - most likely GL context is lost!');
#endif
        while (i < n) {{{ makeSetValue('ids', 'i++*4', 0, 'i32') }}};
        return;
      }
      var id = GL.getNewId(GL.queries);
      query.name = id;
      GL.queries[id] = query;
      {{{ makeSetValue('ids', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteQueriesEXT__sig: 'vip',
  glDeleteQueriesEXT: (n, ids) => {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('ids', 'i*4', 'i32') }}};
      var query = GL.queries[id];
      if (!query) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
      GLctx.disjointTimerQueryExt['deleteQueryEXT'](query);
      GL.queries[id] = null;
    }
  },

  glIsQueryEXT__sig: 'ii',
  glIsQueryEXT: (id) => {
    var query = GL.queries[id];
    if (!query) return 0;
    return GLctx.disjointTimerQueryExt['isQueryEXT'](query);
  },

  glBeginQueryEXT__sig: 'vii',
  glBeginQueryEXT: (target, id) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.queries, id, 'glBeginQueryEXT', 'id');
#endif
    GLctx.disjointTimerQueryExt['beginQueryEXT'](target, GL.queries[id]);
  },

  glEndQueryEXT__sig: 'vi',
  glEndQueryEXT: (target) => {
    GLctx.disjointTimerQueryExt['endQueryEXT'](target);
  },

  // This one is either from EXT_disjoint_timer_query on WebGL 1 (taking a
  // WebGLTimerQueryEXT) or from EXT_disjoint_timer_query_webgl2 (taking a
  // WebGLQuery)
  glQueryCounterEXT__sig: 'vii',
  glQueryCounterEXT: (id, target) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.queries, id, 'glQueryCounterEXT', 'id');
#endif
    GLctx.disjointTimerQueryExt['queryCounterEXT'](GL.queries[id], target);
  },

  glGetQueryivEXT__sig: 'viip',
  glGetQueryivEXT: (target, pname, params) => {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetQueryivEXT(target=${target}, pname=${pname}, params=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('params', '0', 'GLctx.disjointTimerQueryExt[\'getQueryEXT\'](target, pname)', 'i32') }}};
  },

  glGetQueryObjectivEXT__sig: 'viip',
  glGetQueryObjectivEXT: (id, pname, params) => {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetQueryObject(u)ivEXT(id=${id}, pname=${pname}, params=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.queries, id, 'glGetQueryObjectivEXT', 'id');
#endif
    var query = GL.queries[id];
    var param = GLctx.disjointTimerQueryExt['getQueryObjectEXT'](query, pname);
    var ret;
    if (typeof param == 'boolean') {
      ret = param ? 1 : 0;
    } else {
      ret = param;
    }
    {{{ makeSetValue('params', '0', 'ret', 'i32') }}};
  },
  glGetQueryObjectuivEXT: 'glGetQueryObjectivEXT',

  glGetQueryObjecti64vEXT__sig: 'viip',
  glGetQueryObjecti64vEXT__deps: ['$writeI53ToI64'],
  glGetQueryObjecti64vEXT: (id, pname, params) => {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetQueryObject(u)i64vEXT(id=${id}, pname=${pname}, params=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.queries, id, 'glGetQueryObjecti64vEXT', 'id');
#endif
    var query = GL.queries[id];
    var param;
#if MAX_WEBGL_VERSION >= 2
    if (GL.currentContext.version < 2)
#endif
    {
      param = GLctx.disjointTimerQueryExt['getQueryObjectEXT'](query, pname);
    }
#if MAX_WEBGL_VERSION >= 2
    else {
      param = GLctx.getQueryParameter(query, pname);
    }
#endif
    var ret;
    if (typeof param == 'boolean') {
      ret = param ? 1 : 0;
    } else {
      ret = param;
    }
    writeI53ToI64(params, ret);
  },
  glGetQueryObjectui64vEXT: 'glGetQueryObjecti64vEXT',

  glIsBuffer: (buffer) => {
    var b = GL.buffers[buffer];
    if (!b) return 0;
    return GLctx.isBuffer(b);
  },

  glGenRenderbuffers: (n, renderbuffers) => {
    GL.genObject(n, renderbuffers, 'createRenderbuffer', GL.renderbuffers
#if GL_ASSERTIONS
    , 'glGenRenderbuffers'
#endif
      );
  },

  glDeleteRenderbuffers: (n, renderbuffers) => {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('renderbuffers', 'i*4', 'i32') }}};
      var renderbuffer = GL.renderbuffers[id];
      if (!renderbuffer) continue; // GL spec: "glDeleteRenderbuffers silently ignores 0s and names that do not correspond to existing renderbuffer objects".
      GLctx.deleteRenderbuffer(renderbuffer);
      renderbuffer.name = 0;
      GL.renderbuffers[id] = null;
    }
  },

  glBindRenderbuffer: (target, renderbuffer) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.renderbuffers, renderbuffer, 'glBindRenderbuffer', 'renderbuffer');
#endif
    GLctx.bindRenderbuffer(target, GL.renderbuffers[renderbuffer]);
  },

  glGetRenderbufferParameteriv: (target, pname, params) => {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetRenderbufferParameteriv(target=${target}, pname=${pname}, params=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('params', '0', 'GLctx.getRenderbufferParameter(target, pname)', 'i32') }}};
  },

  glIsRenderbuffer: (renderbuffer) => {
    var rb = GL.renderbuffers[renderbuffer];
    if (!rb) return 0;
    return GLctx.isRenderbuffer(rb);
  },

  // This function intentionally assigns `HEAP32[x] = someBoolean;` Don't let
  // Closure mind about that.
  $emscriptenWebGLGetUniform__docs: '/** @suppress{checkTypes} */',
  $emscriptenWebGLGetUniform__deps: ['$webglGetUniformLocation', '$webglPrepareUniformLocationsBeforeFirstUse'],
  $emscriptenWebGLGetUniform: (program, location, params, type) => {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null
      // pointer. Since calling this function does not make sense if params ==
      // null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetUniform*v(program=${program}, location=${location}, params=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniform*v', 'program');
    GL.validateGLObjectID(program.uniformLocsById, location, 'glGetUniform*v', 'location');
#endif
    program = GL.programs[program];
    webglPrepareUniformLocationsBeforeFirstUse(program);
    var data = GLctx.getUniform(program, webglGetUniformLocation(location));
    if (typeof data == 'number' || typeof data == 'boolean') {
      switch (type) {
        case {{{ cDefs.EM_FUNC_SIG_PARAM_I }}}: {{{ makeSetValue('params', '0', 'data', 'i32') }}}; break;
        case {{{ cDefs.EM_FUNC_SIG_PARAM_F }}}: {{{ makeSetValue('params', '0', 'data', 'float') }}}; break;
#if GL_ASSERTIONS
        default: throw 'internal emscriptenWebGLGetUniform() error, bad type: ' + type;
#endif
      }
    } else {
      for (var i = 0; i < data.length; i++) {
        switch (type) {
          case {{{ cDefs.EM_FUNC_SIG_PARAM_I }}}: {{{ makeSetValue('params', 'i*4', 'data[i]', 'i32') }}}; break;
          case {{{ cDefs.EM_FUNC_SIG_PARAM_F }}}: {{{ makeSetValue('params', 'i*4', 'data[i]', 'float') }}}; break;
#if GL_ASSERTIONS
          default: throw 'internal emscriptenWebGLGetUniform() error, bad type: ' + type;
#endif
        }
      }
    }
  },

  glGetUniformfv__deps: ['$emscriptenWebGLGetUniform'],
  glGetUniformfv: (program, location, params) => {
    emscriptenWebGLGetUniform(program, location, params, {{{ cDefs.EM_FUNC_SIG_PARAM_F }}});
  },

  glGetUniformiv__deps: ['$emscriptenWebGLGetUniform'],
  glGetUniformiv: (program, location, params) => {
    emscriptenWebGLGetUniform(program, location, params, {{{ cDefs.EM_FUNC_SIG_PARAM_I }}});
  },

  // Returns the WebGLUniformLocation object corresponding to the location index
  // integer on the currently active shader in this GL context.
  $webglGetUniformLocation: (location) => {
    var p = GLctx.currentProgram;

#if !GL_TRACK_ERRORS && ASSERTIONS
    // In -sGL_TRACK_ERRORS=0 build mode do not allow calling glUniform*()
    // without an active GL program.
    assert(p, 'Attempted to call glUniform*() without an active GL program set! (build with -sGL_TRACK_ERRORS for standards-conformant behavior)');
#endif

#if GL_TRACK_ERRORS
    if (p) {
#endif
      var webglLoc = p.uniformLocsById[location];
      // p.uniformLocsById[location] stores either an integer, or a
      // WebGLUniformLocation.
      // If an integer, we have not yet bound the location, so do it now. The
      // integer value specifies the array index we should bind to.
      if (typeof webglLoc == 'number') {
        p.uniformLocsById[location] = webglLoc = GLctx.getUniformLocation(p, p.uniformArrayNamesById[location] + (webglLoc > 0 ? `[${webglLoc}]` : ''));
      }
      // Else an already cached WebGLUniformLocation, return it.
      return webglLoc;
#if GL_TRACK_ERRORS
    } else {
      GL.recordError(0x502/*GL_INVALID_OPERATION*/);
    }
#endif
  },

  $webglPrepareUniformLocationsBeforeFirstUse__deps: ['$webglGetLeftBracePos'],
  $webglPrepareUniformLocationsBeforeFirstUse: (program) => {
    var uniformLocsById = program.uniformLocsById, // Maps GLuint -> WebGLUniformLocation
      uniformSizeAndIdsByName = program.uniformSizeAndIdsByName, // Maps name -> [uniform array length, GLuint]
      i, j;

    // On the first time invocation of glGetUniformLocation on this shader program:
    // initialize cache data structures and discover which uniforms are arrays.
    if (!uniformLocsById) {
      // maps GLint integer locations to WebGLUniformLocations
      program.uniformLocsById = uniformLocsById = {};
      // maps integer locations back to uniform name strings, so that we can lazily fetch uniform array locations
      program.uniformArrayNamesById = {};

      var numActiveUniforms = GLctx.getProgramParameter(program, 0x8B86/*GL_ACTIVE_UNIFORMS*/);
      for (i = 0; i < numActiveUniforms; ++i) {
        var u = GLctx.getActiveUniform(program, i);
        var nm = u.name;
        var sz = u.size;
        var lb = webglGetLeftBracePos(nm);
        var arrayName = lb > 0 ? nm.slice(0, lb) : nm;

#if GL_EXPLICIT_UNIFORM_LOCATION
        // Acquire the preset location from the explicit uniform location if one was specified, or
        // programmatically assign a new one if not.
        var id = uniformSizeAndIdsByName[arrayName] ? uniformSizeAndIdsByName[arrayName][1] : program.uniformIdCounter;
        program.uniformIdCounter = Math.max(id + sz, program.uniformIdCounter);
#else
        // Assign a new location.
        var id = program.uniformIdCounter;
        program.uniformIdCounter += sz;
#endif
        // Eagerly get the location of the uniformArray[0] base element.
        // The remaining indices >0 will be left for lazy evaluation to
        // improve performance. Those may never be needed to fetch, if the
        // application fills arrays always in full starting from the first
        // element of the array.
        uniformSizeAndIdsByName[arrayName] = [sz, id];

        // Store placeholder integers in place that highlight that these
        // >0 index locations are array indices pending population.
        for (j = 0; j < sz; ++j) {
          uniformLocsById[id] = j;
          program.uniformArrayNamesById[id++] = arrayName;
        }
      }
    }
  },

  // Returns the index of '[' character in an uniform that represents an array
  // of uniforms (e.g. colors[10])
  // Closure does counterproductive inlining:
  // https://github.com/google/closure-compiler/issues/3203, so prevent inlining
  // manually.
  $webglGetLeftBracePos__docs: '/** @noinline */',
  $webglGetLeftBracePos: (name) => name.slice(-1) == ']' && name.lastIndexOf('['),

  glGetUniformLocation__deps: ['$jstoi_q', '$webglPrepareUniformLocationsBeforeFirstUse', '$webglGetLeftBracePos'],
  glGetUniformLocation: (program, name) => {

#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformLocation', 'program');
#endif
    name = UTF8ToString(name);

#if GL_ASSERTIONS
    assert(!name.includes(' '), `Uniform names passed to glGetUniformLocation() should not contain spaces! (received "${name}")`);
#endif

    if (program = GL.programs[program]) {
      webglPrepareUniformLocationsBeforeFirstUse(program);
      var uniformLocsById = program.uniformLocsById; // Maps GLuint -> WebGLUniformLocation
      var arrayIndex = 0;
      var uniformBaseName = name;

      // Invariant: when populating integer IDs for uniform locations, we must
      // maintain the precondition that arrays reside in contiguous addresses,
      // i.e. for a 'vec4 colors[10];', colors[4] must be at location
      // colors[0]+4.  However, user might call glGetUniformLocation(program,
      // "colors") for an array, so we cannot discover based on the user input
      // arguments whether the uniform we are dealing with is an array. The only
      // way to discover which uniforms are arrays is to enumerate over all the
      // active uniforms in the program.
      var leftBrace = webglGetLeftBracePos(name);

      // If user passed an array accessor "[index]", parse the array index off the accessor.
      if (leftBrace > 0) {
  #if GL_ASSERTIONS
        assert(name.slice(leftBrace + 1).length == 1 || !isNaN(jstoi_q(name.slice(leftBrace + 1))), `Malformed input parameter name "${name}" passed to glGetUniformLocation!`);
  #endif
        arrayIndex = jstoi_q(name.slice(leftBrace + 1)) >>> 0; // "index]", coerce parseInt(']') with >>>0 to treat "foo[]" as "foo[0]" and foo[-1] as unsigned out-of-bounds.
        uniformBaseName = name.slice(0, leftBrace);
      }

      // Have we cached the location of this uniform before?
      // A pair [array length, GLint of the uniform location]
      var sizeAndId = program.uniformSizeAndIdsByName[uniformBaseName];

      // If an uniform with this name exists, and if its index is within the
      // array limits (if it's even an array), query the WebGLlocation, or
      // return an existing cached location.
      if (sizeAndId && arrayIndex < sizeAndId[0]) {
        arrayIndex += sizeAndId[1]; // Add the base location of the uniform to the array index offset.
        if ((uniformLocsById[arrayIndex] = uniformLocsById[arrayIndex] || GLctx.getUniformLocation(program, name))) {
          return arrayIndex;
        }
      }
    }
#if GL_TRACK_ERRORS
    else {
      // N.b. we are currently unable to distinguish between GL program IDs that
      // never existed vs GL program IDs that have been deleted, so report
      // GL_INVALID_VALUE in both cases.
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
    }
#endif
    return -1;
  },

  // This function intentionally assigns `HEAP32[x] = someBoolean;` Don't let
  // Closure mind about that.
  $emscriptenWebGLGetVertexAttrib__docs: '/** @suppress{checkTypes} */',
  $emscriptenWebGLGetVertexAttrib: (index, pname, params, type) => {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null
      // pointer. Since calling this function does not make sense if params ==
      // null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetVertexAttrib*v(index=${index}, pname=${pname}, params=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#if FULL_ES2
    if (GL.currentContext.clientBuffers[index].enabled) {
      err("glGetVertexAttrib*v on client-side array: not supported, bad data returned");
    }
#endif
    var data = GLctx.getVertexAttrib(index, pname);
    if (pname == 0x889F/*VERTEX_ATTRIB_ARRAY_BUFFER_BINDING*/) {
      {{{ makeSetValue('params', '0', 'data && data["name"]', 'i32') }}};
    } else if (typeof data == 'number' || typeof data == 'boolean') {
      switch (type) {
        case {{{ cDefs.EM_FUNC_SIG_PARAM_I }}}: {{{ makeSetValue('params', '0', 'data', 'i32') }}}; break;
        case {{{ cDefs.EM_FUNC_SIG_PARAM_F }}}: {{{ makeSetValue('params', '0', 'data', 'float') }}}; break;
        case {{{ cDefs.EM_FUNC_SIG_PARAM_F2I }}}: {{{ makeSetValue('params', '0', 'Math.fround(data)', 'i32') }}}; break;
#if GL_ASSERTIONS
        default: throw 'internal emscriptenWebGLGetVertexAttrib() error, bad type: ' + type;
#endif
      }
    } else {
      for (var i = 0; i < data.length; i++) {
        switch (type) {
          case {{{ cDefs.EM_FUNC_SIG_PARAM_I }}}: {{{ makeSetValue('params', 'i*4', 'data[i]', 'i32') }}}; break;
          case {{{ cDefs.EM_FUNC_SIG_PARAM_F }}}: {{{ makeSetValue('params', 'i*4', 'data[i]', 'float') }}}; break;
          case {{{ cDefs.EM_FUNC_SIG_PARAM_F2I }}}: {{{ makeSetValue('params', 'i*4', 'Math.fround(data[i])', 'i32') }}}; break;
#if GL_ASSERTIONS
          default: throw 'internal emscriptenWebGLGetVertexAttrib() error, bad type: ' + type;
#endif
        }
      }
    }
  },

  glGetVertexAttribfv__deps: ['$emscriptenWebGLGetVertexAttrib'],
  glGetVertexAttribfv: (index, pname, params) => {
    // N.B. This function may only be called if the vertex attribute was
    // specified using the function glVertexAttrib*f(), otherwise the results
    // are undefined. (GLES3 spec 6.1.12)
    emscriptenWebGLGetVertexAttrib(index, pname, params, {{{ cDefs.EM_FUNC_SIG_PARAM_F }}});
  },

  glGetVertexAttribiv__deps: ['$emscriptenWebGLGetVertexAttrib'],
  glGetVertexAttribiv: (index, pname, params) => {
    // N.B. This function may only be called if the vertex attribute was
    // specified using the function glVertexAttrib*f(), otherwise the results
    // are undefined. (GLES3 spec 6.1.12)
    emscriptenWebGLGetVertexAttrib(index, pname, params, {{{ cDefs.EM_FUNC_SIG_PARAM_F2I }}});
  },

  glGetVertexAttribPointerv: (index, pname, pointer) => {
    if (!pointer) {
      // GLES2 specification does not specify how to behave if pointer is a null
      // pointer. Since calling this function does not make sense if pointer ==
      // null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetVertexAttribPointerv(index=${index}, pname=${pname}, pointer=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#if FULL_ES2
    if (GL.currentContext.clientBuffers[index].enabled) {
      err("glGetVertexAttribPointer on client-side array: not supported, bad data returned");
    }
#endif
    {{{ makeSetValue('pointer', '0', 'GLctx.getVertexAttribOffset(index, pname)', 'i32') }}};
  },

  glUniform1f__deps: ['$webglGetUniformLocation'],
  glUniform1f: (location, v0) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform1f', 'location');
#endif
    GLctx.uniform1f(webglGetUniformLocation(location), v0);
  },

  glUniform2f__deps: ['$webglGetUniformLocation'],
  glUniform2f: (location, v0, v1) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform2f', 'location');
#endif
    GLctx.uniform2f(webglGetUniformLocation(location), v0, v1);
  },

  glUniform3f__deps: ['$webglGetUniformLocation'],
  glUniform3f: (location, v0, v1, v2) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform3f', 'location');
#endif
    GLctx.uniform3f(webglGetUniformLocation(location), v0, v1, v2);
  },

  glUniform4f__deps: ['$webglGetUniformLocation'],
  glUniform4f: (location, v0, v1, v2, v3) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform4f', 'location');
#endif
    GLctx.uniform4f(webglGetUniformLocation(location), v0, v1, v2, v3);
  },

  glUniform1i__deps: ['$webglGetUniformLocation'],
  glUniform1i: (location, v0) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform1i', 'location');
#endif
    GLctx.uniform1i(webglGetUniformLocation(location), v0);
  },

  glUniform2i__deps: ['$webglGetUniformLocation'],
  glUniform2i: (location, v0, v1) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform2i', 'location');
#endif
    GLctx.uniform2i(webglGetUniformLocation(location), v0, v1);
  },

  glUniform3i__deps: ['$webglGetUniformLocation'],
  glUniform3i: (location, v0, v1, v2) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform3i', 'location');
#endif
    GLctx.uniform3i(webglGetUniformLocation(location), v0, v1, v2);
  },

  glUniform4i__deps: ['$webglGetUniformLocation'],
  glUniform4i: (location, v0, v1, v2, v3) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform4i', 'location');
#endif
    GLctx.uniform4i(webglGetUniformLocation(location), v0, v1, v2, v3);
  },

  glUniform1iv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLIntBuffers'
#endif
  ],
  glUniform1iv: (location, count, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform1iv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform1iv must be aligned to four bytes!');
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniform1iv(webglGetUniformLocation(location), HEAP32, {{{ getHeapOffset('value', 'i32') }}}, count);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniform1iv(webglGetUniformLocation(location), HEAP32, {{{ getHeapOffset('value', 'i32') }}}, count);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE }}}) {
      // avoid allocation when uploading few enough uniforms
      var view = miniTempWebGLIntBuffers[count];
      for (var i = 0; i < count; ++i) {
        view[i] = {{{ makeGetValue('value', '4*i', 'i32') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    }
    GLctx.uniform1iv(webglGetUniformLocation(location), view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glUniform2iv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLIntBuffers'
#endif
  ],
  glUniform2iv: (location, count, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform2iv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform2iv must be aligned to four bytes!');
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniform2iv(webglGetUniformLocation(location), HEAP32, {{{ getHeapOffset('value', 'i32') }}}, count*2);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniform2iv(webglGetUniformLocation(location), HEAP32, {{{ getHeapOffset('value', 'i32') }}}, count*2);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE / 2 }}}) {
      // avoid allocation when uploading few enough uniforms
      count *= 2;
      var view = miniTempWebGLIntBuffers[count];
      for (var i = 0; i < count; i += 2) {
        view[i] = {{{ makeGetValue('value', '4*i', 'i32') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'i32') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('32', 'value', 'value+count*8') }}};
    }
    GLctx.uniform2iv(webglGetUniformLocation(location), view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glUniform3iv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLIntBuffers'
#endif
  ],
  glUniform3iv: (location, count, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform3iv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform3iv must be aligned to four bytes!');
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniform3iv(webglGetUniformLocation(location), HEAP32, {{{ getHeapOffset('value', 'i32') }}}, count*3);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniform3iv(webglGetUniformLocation(location), HEAP32, {{{ getHeapOffset('value', 'i32') }}}, count*3);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE / 3 }}}) {
      // avoid allocation when uploading few enough uniforms
      count *= 3;
      var view = miniTempWebGLIntBuffers[count];
      for (var i = 0; i < count; i += 3) {
        view[i] = {{{ makeGetValue('value', '4*i', 'i32') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'i32') }}};
        view[i+2] = {{{ makeGetValue('value', '4*i+8', 'i32') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('32', 'value', 'value+count*12') }}};
    }
    GLctx.uniform3iv(webglGetUniformLocation(location), view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glUniform4iv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLIntBuffers'
#endif
  ],
  glUniform4iv: (location, count, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform4iv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform4iv must be aligned to four bytes!');
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniform4iv(webglGetUniformLocation(location), HEAP32, {{{ getHeapOffset('value', 'i32') }}}, count*4);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniform4iv(webglGetUniformLocation(location), HEAP32, {{{ getHeapOffset('value', 'i32') }}}, count*4);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE / 4 }}}) {
      // avoid allocation when uploading few enough uniforms
      count *= 4;
      var view = miniTempWebGLIntBuffers[count];
      for (var i = 0; i < count; i += 4) {
        view[i] = {{{ makeGetValue('value', '4*i', 'i32') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'i32') }}};
        view[i+2] = {{{ makeGetValue('value', '4*i+8', 'i32') }}};
        view[i+3] = {{{ makeGetValue('value', '4*i+12', 'i32') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('32', 'value', 'value+count*16') }}};
    }
    GLctx.uniform4iv(webglGetUniformLocation(location), view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glUniform1fv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLFloatBuffers'
#endif
  ],
  glUniform1fv: (location, count, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform1fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniform1fv must be aligned to four bytes!');
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniform1fv(webglGetUniformLocation(location), HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniform1fv(webglGetUniformLocation(location), HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE }}}) {
      // avoid allocation when uploading few enough uniforms
      var view = miniTempWebGLFloatBuffers[count];
      for (var i = 0; i < count; ++i) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
    }
    GLctx.uniform1fv(webglGetUniformLocation(location), view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glUniform2fv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLFloatBuffers'
#endif
  ],
  glUniform2fv: (location, count, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform2fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniform2fv must be aligned to four bytes!');
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniform2fv(webglGetUniformLocation(location), HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*2);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniform2fv(webglGetUniformLocation(location), HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*2);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE / 2 }}}) {
      // avoid allocation when uploading few enough uniforms
      count *= 2;
      var view = miniTempWebGLFloatBuffers[count];
      for (var i = 0; i < count; i += 2) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*8') }}};
    }
    GLctx.uniform2fv(webglGetUniformLocation(location), view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glUniform3fv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLFloatBuffers'
#endif
  ],
  glUniform3fv: (location, count, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform3fv', 'location');
    assert((value % 4) == 0, 'Pointer to float data passed to glUniform3fv must be aligned to four bytes!' + value);
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniform3fv(webglGetUniformLocation(location), HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*3);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniform3fv(webglGetUniformLocation(location), HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*3);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE / 3 }}}) {
      // avoid allocation when uploading few enough uniforms
      count *= 3;
      var view = miniTempWebGLFloatBuffers[count];
      for (var i = 0; i < count; i += 3) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'float') }}};
        view[i+2] = {{{ makeGetValue('value', '4*i+8', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*12') }}};
    }
    GLctx.uniform3fv(webglGetUniformLocation(location), view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glUniform4fv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLFloatBuffers'
#endif
  ],
  glUniform4fv: (location, count, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform4fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniform4fv must be aligned to four bytes!');
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniform4fv(webglGetUniformLocation(location), HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*4);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniform4fv(webglGetUniformLocation(location), HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*4);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE / 4 }}}) {
      // avoid allocation when uploading few enough uniforms
      var view = miniTempWebGLFloatBuffers[4*count];
      // hoist the heap out of the loop for size and for pthreads+growth.
      var heap = HEAPF32;
      value = {{{ getHeapOffset('value', 'float') }}};
      count *= 4;
      for (var i = 0; i < count; i += 4) {
        var dst = value + i;
        view[i] = heap[dst];
        view[i + 1] = heap[dst + 1];
        view[i + 2] = heap[dst + 2];
        view[i + 3] = heap[dst + 3];
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*16') }}};
    }
    GLctx.uniform4fv(webglGetUniformLocation(location), view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glUniformMatrix2fv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLFloatBuffers'
#endif
  ],
  glUniformMatrix2fv: (location, count, transpose, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniformMatrix2fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix2fv must be aligned to four bytes!');
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniformMatrix2fv(webglGetUniformLocation(location), !!transpose, HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*4);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniformMatrix2fv(webglGetUniformLocation(location), !!transpose, HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*4);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE / 4 }}}) {
      // avoid allocation when uploading few enough uniforms
      count *= 4;
      var view = miniTempWebGLFloatBuffers[count];
      for (var i = 0; i < count; i += 4) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'float') }}};
        view[i+2] = {{{ makeGetValue('value', '4*i+8', 'float') }}};
        view[i+3] = {{{ makeGetValue('value', '4*i+12', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*16') }}};
    }
    GLctx.uniformMatrix2fv(webglGetUniformLocation(location), !!transpose, view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glUniformMatrix3fv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLFloatBuffers'
#endif
  ],
  glUniformMatrix3fv: (location, count, transpose, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniformMatrix3fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix3fv must be aligned to four bytes!');
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniformMatrix3fv(webglGetUniformLocation(location), !!transpose, HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*9);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniformMatrix3fv(webglGetUniformLocation(location), !!transpose, HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*9);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE / 9 }}}) {
      // avoid allocation when uploading few enough uniforms
      count *= 9;
      var view = miniTempWebGLFloatBuffers[count];
      for (var i = 0; i < count; i += 9) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'float') }}};
        view[i+2] = {{{ makeGetValue('value', '4*i+8', 'float') }}};
        view[i+3] = {{{ makeGetValue('value', '4*i+12', 'float') }}};
        view[i+4] = {{{ makeGetValue('value', '4*i+16', 'float') }}};
        view[i+5] = {{{ makeGetValue('value', '4*i+20', 'float') }}};
        view[i+6] = {{{ makeGetValue('value', '4*i+24', 'float') }}};
        view[i+7] = {{{ makeGetValue('value', '4*i+28', 'float') }}};
        view[i+8] = {{{ makeGetValue('value', '4*i+32', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*36') }}};
    }
    GLctx.uniformMatrix3fv(webglGetUniformLocation(location), !!transpose, view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glUniformMatrix4fv__deps: ['$webglGetUniformLocation'
#if GL_POOL_TEMP_BUFFERS && (MIN_WEBGL_VERSION == 1 || !WEBGL_USE_GARBAGE_FREE_APIS)
    , '$miniTempWebGLFloatBuffers'
#endif
  ],
  glUniformMatrix4fv: (location, count, transpose, value) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniformMatrix4fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix4fv must be aligned to four bytes!');
#endif

#if MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
#if GL_ASSERTIONS
    assert(GL.currentContext.version >= 2);
#endif
    count && GLctx.uniformMatrix4fv(webglGetUniformLocation(location), !!transpose, HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*16);
#else

#if WEBGL_USE_GARBAGE_FREE_APIS
    if ({{{ isCurrentContextWebGL2() }}}) {
      count && GLctx.uniformMatrix4fv(webglGetUniformLocation(location), !!transpose, HEAPF32, {{{ getHeapOffset('value', 'float') }}}, count*16);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= {{{ GL_POOL_TEMP_BUFFERS_SIZE / 16 }}}) {
      // avoid allocation when uploading few enough uniforms
      var view = miniTempWebGLFloatBuffers[16*count];
      // hoist the heap out of the loop for size and for pthreads+growth.
      var heap = HEAPF32;
      value = {{{ getHeapOffset('value', 'float') }}};
      count *= 16;
      for (var i = 0; i < count; i += 16) {
        var dst = value + i;
        view[i] = heap[dst];
        view[i + 1] = heap[dst + 1];
        view[i + 2] = heap[dst + 2];
        view[i + 3] = heap[dst + 3];
        view[i + 4] = heap[dst + 4];
        view[i + 5] = heap[dst + 5];
        view[i + 6] = heap[dst + 6];
        view[i + 7] = heap[dst + 7];
        view[i + 8] = heap[dst + 8];
        view[i + 9] = heap[dst + 9];
        view[i + 10] = heap[dst + 10];
        view[i + 11] = heap[dst + 11];
        view[i + 12] = heap[dst + 12];
        view[i + 13] = heap[dst + 13];
        view[i + 14] = heap[dst + 14];
        view[i + 15] = heap[dst + 15];
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*64') }}};
    }
    GLctx.uniformMatrix4fv(webglGetUniformLocation(location), !!transpose, view);
#endif // MIN_WEBGL_VERSION >= 2 && WEBGL_USE_GARBAGE_FREE_APIS
  },

  glBindBuffer: (target, buffer) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.buffers, buffer, 'glBindBuffer', 'buffer');
#endif
#if FULL_ES2 || LEGACY_GL_EMULATION
    if (target == 0x8892 /*GL_ARRAY_BUFFER*/) {
      GLctx.currentArrayBufferBinding = buffer;
#if LEGACY_GL_EMULATION
      GLImmediate.lastArrayBuffer = buffer;
#endif
    } else if (target == 0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/) {
      GLctx.currentElementArrayBufferBinding = buffer;
    }
#endif

#if MAX_WEBGL_VERSION >= 2
    if (target == 0x88EB /*GL_PIXEL_PACK_BUFFER*/) {
      // In WebGL 2 glReadPixels entry point, we need to use a different WebGL 2
      // API function call when a buffer is bound to
      // GL_PIXEL_PACK_BUFFER_BINDING point, so must keep track whether that
      // binding point is non-null to know what is the proper API function to
      // call.
      GLctx.currentPixelPackBufferBinding = buffer;
    } else if (target == 0x88EC /*GL_PIXEL_UNPACK_BUFFER*/) {
      // In WebGL 2 gl(Compressed)Tex(Sub)Image[23]D entry points, we need to
      // use a different WebGL 2 API function call when a buffer is bound to
      // GL_PIXEL_UNPACK_BUFFER_BINDING point, so must keep track whether that
      // binding point is non-null to know what is the proper API function to
      // call.
      GLctx.currentPixelUnpackBufferBinding = buffer;
    }
#endif
    GLctx.bindBuffer(target, GL.buffers[buffer]);
  },

  glVertexAttrib1fv: (index, v) => {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib1fv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttrib1fv!');
#endif

    GLctx.vertexAttrib1f(index, HEAPF32[v>>2]);
  },

  glVertexAttrib2fv: (index, v) => {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib2fv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttrib2fv!');
#endif

    GLctx.vertexAttrib2f(index, HEAPF32[v>>2], HEAPF32[v+4>>2]);
  },

  glVertexAttrib3fv: (index, v) => {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib3fv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttrib3fv!');
#endif

    GLctx.vertexAttrib3f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2]);
  },

  glVertexAttrib4fv: (index, v) => {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib4fv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttrib4fv!');
#endif

    GLctx.vertexAttrib4f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2], HEAPF32[v+12>>2]);
  },

  glGetAttribLocation: (program, name) => {
    return GLctx.getAttribLocation(GL.programs[program], UTF8ToString(name));
  },

  $__glGetActiveAttribOrUniform__deps: ['$stringToUTF8'],
  $__glGetActiveAttribOrUniform: (funcName, program, index, bufSize, length, size, type, name) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, funcName, 'program');
#endif
    program = GL.programs[program];
    var info = GLctx[funcName](program, index);
    if (info) {
      // If an error occurs, nothing will be written to length, size and type and name.
      var numBytesWrittenExclNull = name && stringToUTF8(info.name, name, bufSize);
      if (length) {{{ makeSetValue('length', '0', 'numBytesWrittenExclNull', 'i32') }}};
      if (size) {{{ makeSetValue('size', '0', 'info.size', 'i32') }}};
      if (type) {{{ makeSetValue('type', '0', 'info.type', 'i32') }}};
    }
  },

  glGetActiveAttrib__deps: ['$__glGetActiveAttribOrUniform'],
  glGetActiveAttrib: (program, index, bufSize, length, size, type, name) => {
    __glGetActiveAttribOrUniform('getActiveAttrib', program, index, bufSize, length, size, type, name);
  },

  glGetActiveUniform__deps: ['$__glGetActiveAttribOrUniform'],
  glGetActiveUniform: (program, index, bufSize, length, size, type, name) => {
    __glGetActiveAttribOrUniform('getActiveUniform', program, index, bufSize, length, size, type, name);
  },

  glCreateShader: (shaderType) => {
    var id = GL.getNewId(GL.shaders);
    GL.shaders[id] = GLctx.createShader(shaderType);

#if GL_EXPLICIT_UNIFORM_LOCATION || GL_EXPLICIT_UNIFORM_BINDING
    // GL_VERTEX_SHADER = 0x8B31, GL_FRAGMENT_SHADER = 0x8B30
    GL.shaders[id].shaderType = shaderType&1?'vs':'fs';
#endif

    return id;
  },

  glDeleteShader: (id) => {
    if (!id) return;
    var shader = GL.shaders[id];
    if (!shader) {
      // glDeleteShader actually signals an error when deleting a nonexisting
      // object, unlike some other GL delete functions.
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    GLctx.deleteShader(shader);
    GL.shaders[id] = null;
  },

  glGetAttachedShaders: (program, maxCount, count, shaders) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetAttachedShaders', 'program');
#endif
    var result = GLctx.getAttachedShaders(GL.programs[program]);
    var len = result.length;
    if (len > maxCount) {
      len = maxCount;
    }
    {{{ makeSetValue('count', '0', 'len', 'i32') }}};
    for (var i = 0; i < len; ++i) {
      var id = GL.shaders.indexOf(result[i]);
#if GL_ASSERTIONS
      assert(id !== -1, 'shader not bound to local id');
#endif
      {{{ makeSetValue('shaders', 'i*4', 'id', 'i32') }}};
    }
  },

#if GL_EXPLICIT_UNIFORM_LOCATION || GL_EXPLICIT_UNIFORM_BINDING
  glShaderSource__deps: ['$preprocess_c_code', '$remove_cpp_comments_in_shaders', '$jstoi_q', '$find_closing_parens_index'],
#endif
  glShaderSource: (shader, count, string, length) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glShaderSource', 'shader');
#endif
    var source = GL.getSource(shader, count, string, length);

#if WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION
    if ({{{ isCurrentContextWebGL2() }}}) {
      // If a WebGL 1 shader happens to use GL_EXT_shader_texture_lod extension,
      // it will not compile on WebGL 2, because WebGL 2 no longer supports that
      // extension for WebGL 1 shaders. Therefore upgrade shaders to WebGL 2
      // by doing a bunch of dirty hacks. Not guaranteed to work on all shaders.
      // One might consider doing this for only the shaders that actually use
      // the GL_EXT_shader_texture_lod extension, but the problem is that
      // vertex and fragment shader versions need to match, and when compiling
      // the corresponding vertex shader, we would not know if that needed to
      // be compiled with or without the patch, so we must patch all shaders.
      if (source.includes('#version 100')) {
        source = source.replace(/#extension GL_OES_standard_derivatives : enable/g, "");
        source = source.replace(/#extension GL_EXT_shader_texture_lod : enable/g, '');
        var prelude = '';
        if (source.includes('gl_FragColor')) {
          prelude += 'out mediump vec4 GL_FragColor;\n';
          source = source.replace(/gl_FragColor/g, 'GL_FragColor');
        }
        if (source.includes('attribute')) {
          source = source.replace(/attribute/g, 'in');
          source = source.replace(/varying/g, 'out');
        } else {
          source = source.replace(/varying/g, 'in');
        }

        source = source.replace(/textureCubeLodEXT/g, 'textureCubeLod');
        source = source.replace(/texture2DLodEXT/g, 'texture2DLod');
        source = source.replace(/texture2DProjLodEXT/g, 'texture2DProjLod');
        source = source.replace(/texture2DGradEXT/g, 'texture2DGrad');
        source = source.replace(/texture2DProjGradEXT/g, 'texture2DProjGrad');
        source = source.replace(/textureCubeGradEXT/g, 'textureCubeGrad');

        source = source.replace(/textureCube/g, 'texture');
        source = source.replace(/texture1D/g, 'texture');
        source = source.replace(/texture2D/g, 'texture');
        source = source.replace(/texture3D/g, 'texture');
        source = source.replace(/#version 100/g, '#version 300 es\n' + prelude);
      }
    }
#endif

#if GL_EXPLICIT_UNIFORM_LOCATION || GL_EXPLICIT_UNIFORM_BINDING
#if GL_DEBUG
    dbg(`Input shader source: ${source}`;
#endif

#if ASSERTIONS
    // These are not expected to be meaningful in WebGL, but issue a warning if
    // they are present, to give some diagnostics about if they are present.
    if (source.includes('__FILE__')) warnOnce(`When compiling shader: ${source}: Preprocessor variable __FILE__ is not handled by -sGL_EXPLICIT_UNIFORM_LOCATION/-sGL_EXPLICIT_UNIFORM_BINDING options!`);
    if (source.includes('__LINE__')) warnOnce(`When compiling shader: ${source}: Preprocessor variable __LINE__ is not handled by -sGL_EXPLICIT_UNIFORM_LOCATION/-sGL_EXPLICIT_UNIFORM_BINDING options!`);
#endif
    // Remove comments and C-preprocess the input shader first, so that we can
    // appropriately parse the layout location directives.
    source = preprocess_c_code(remove_cpp_comments_in_shaders(source), {
      'GL_FRAGMENT_PRECISION_HIGH': () => 1,
      'GL_ES': () => 1,
      '__VERSION__': () => source.includes('#version 300') ? 300 : 100
    });
#if GL_DEBUG
    dbg(`Shader source after preprocessing: ${source}`;
#endif
#endif // ~GL_EXPLICIT_UNIFORM_LOCATION || GL_EXPLICIT_UNIFORM_BINDING

#if GL_EXPLICIT_UNIFORM_LOCATION
    // Extract the layout(location = x) directives.
    var regex = /layout\s*\(\s*location\s*=\s*(-?\d+)\s*\)\s*(uniform\s+((lowp|mediump|highp)\s+)?\w+\s+(\w+))/g, explicitUniformLocations = {}, match;
    while (match = regex.exec(source)) {
#if GL_DEBUG
      console.dir(match);
#endif
      explicitUniformLocations[match[5]] = jstoi_q(match[1]);
#if GL_TRACK_ERRORS
      if (!(explicitUniformLocations[match[5]] >= 0 && explicitUniformLocations[match[5]] < 1048576)) {
        err(`Specified an out of range layout(location=x) directive "${explicitUniformLocations[match[5]]}"! (${match[0]})`);
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
#endif
    }

    // Remove all the layout(location = x) directives so that they do not make
    // their way to the actual WebGL shader compiler.
    source = source.replace(regex, '$2');

    // Remember all the directives to be handled after glLinkProgram is called.
    GL.shaders[shader].explicitUniformLocations = explicitUniformLocations;

#if GL_DEBUG
    dbg(`Shader source after removing layout location directives: ${source}`;
    dbg('Explicit uniform locations recorded in the shader:');
    console.dir(explicitUniformLocations);
#endif

#endif // ~GL_EXPLICIT_UNIFORM_LOCATION

#if GL_EXPLICIT_UNIFORM_BINDING
    // Extract the layout(binding = x) directives. Four types we need to handle:
    // layout(binding = 3) uniform sampler2D mainTexture;
    // layout(binding = 1, std140) uniform MainBlock { ... };
    // layout(std140, binding = 1) uniform MainBlock { ... };
    // layout(binding = 1) uniform MainBlock { ... };
    var bindingRegex = /layout\s*\(.*?binding\s*=\s*(-?\d+).*?\)\s*uniform\s+(\w+)\s+(\w+)?/g, samplerBindings = {}, uniformBindings = {}, bindingMatch;
    while (bindingMatch = bindingRegex.exec(source)) {
      // We have a layout(binding=x) enabled uniform. Parse the array length of
      // that uniform, if it is an array, i.e. a
      //    layout(binding = 3) uniform sampler2D mainTexture[arrayLength];
      // or
      //    layout(binding = 1, std140) uniform MainBlock { ... } name[arrayLength];
      var arrayLength = 1;
      for (var i = bindingMatch.index; i < source.length && source[i] != ';'; ++i) {
        if (source[i] == '[') {
          arrayLength = jstoi_q(source.slice(i+1));
          break;
        }
        if (source[i] == '{') i = find_closing_parens_index(source, i, '{', '}') - 1;
      }
#if GL_DEBUG
      console.dir(bindingMatch);
#endif
      var binding = jstoi_q(bindingMatch[1]);
#if GL_TRACK_ERRORS
      var bindingsType = 0x8872/*GL_MAX_TEXTURE_IMAGE_UNITS*/;
#endif
      if (bindingMatch[3] && bindingMatch[2].indexOf('sampler') != -1) {
        samplerBindings[bindingMatch[3]] = [binding, arrayLength];
      } else {
#if GL_TRACK_ERRORS
        bindingsType = 0x8A2E/*GL_MAX_COMBINED_UNIFORM_BLOCKS*/;
#endif
        uniformBindings[bindingMatch[2]] = [binding, arrayLength];
      }
#if GL_TRACK_ERRORS
      var numBindingPoints = GLctx.getParameter(bindingsType);
      if (!(binding >= 0 && binding + arrayLength <= numBindingPoints)) {
        err(`Specified an out of range layout(binding=x) directive "${binding}"! (${bindingMatch[0]}). Valid range is [0, ${numBindingPoints}-1]`);
        GL.recordError(0x501 /* GL_INVALID_VALUE */);
        return;
      }
#endif
    }

    // Remove all the layout(binding = x) directives so that they do not make
    // their way to the actual WebGL shader compiler. These regexes get quite
    // hairy, check against https://regex101.com/ when working on these.
    source = source.replace(/layout\s*\(.*?binding\s*=\s*([-\d]+).*?\)/g, ''); // "layout(binding = 3)" -> ""
    source = source.replace(/(layout\s*\((.*?)),\s*binding\s*=\s*([-\d]+)\)/g, '$1)'); // "layout(std140, binding = 1)" -> "layout(std140)"
    source = source.replace(/layout\s*\(\s*binding\s*=\s*([-\d]+)\s*,(.*?)\)/g, 'layout($2)'); // "layout(binding = 1, std140)" -> "layout(std140)"

#if GL_DEBUG
    dbg(`Shader source after removing layout binding directives: ${source}`;
    dbg('Sampler binding locations recorded in the shader:');
    console.dir(samplerBindings);
    dbg('Uniform binding locations recorded in the shader:');
    console.dir(uniformBindings);
#endif

    // Remember all the directives to be handled after glLinkProgram is called.
    GL.shaders[shader].explicitSamplerBindings = samplerBindings;
    GL.shaders[shader].explicitUniformBindings = uniformBindings;

#endif // ~GL_EXPLICIT_UNIFORM_BINDING

    GLctx.shaderSource(GL.shaders[shader], source);
  },

  glGetShaderSource: (shader, bufSize, length, source) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderSource', 'shader');
#endif
    var result = GLctx.getShaderSource(GL.shaders[shader]);
    if (!result) return; // If an error occurs, nothing will be written to length or source.
    var numBytesWrittenExclNull = (bufSize > 0 && source) ? stringToUTF8(result, source, bufSize) : 0;
    if (length) {{{ makeSetValue('length', '0', 'numBytesWrittenExclNull', 'i32') }}};
  },

  glCompileShader: (shader) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glCompileShader', 'shader');
#endif
    GLctx.compileShader(GL.shaders[shader]);
#if GL_DEBUG
    var log = (GLctx.getShaderInfoLog(GL.shaders[shader]) || '').trim();
    if (log) dbg(`glCompileShader: ${log}`);
#endif
  },

  glGetShaderInfoLog__deps: ['$stringToUTF8'],
  glGetShaderInfoLog: (shader, maxLength, length, infoLog) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderInfoLog', 'shader');
#endif
    var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
#if GL_ASSERTIONS || GL_TRACK_ERRORS
    if (log === null) log = '(unknown error)';
#endif
    var numBytesWrittenExclNull = (maxLength > 0 && infoLog) ? stringToUTF8(log, infoLog, maxLength) : 0;
    if (length) {{{ makeSetValue('length', '0', 'numBytesWrittenExclNull', 'i32') }}};
  },

  glGetShaderiv: (shader, pname, p) => {
    if (!p) {
      // GLES2 specification does not specify how to behave if p is a null
      // pointer. Since calling this function does not make sense if p == null,
      // issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetShaderiv(shader=${shader}, pname=${pname}, p=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderiv', 'shader');
#endif
    if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
      var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
#if GL_ASSERTIONS || GL_TRACK_ERRORS
      if (log === null) log = '(unknown error)';
#endif
      // The GLES2 specification says that if the shader has an empty info log,
      // a value of 0 is returned. Otherwise the log has a null char appended.
      // (An empty string is falsey, so we can just check that instead of
      // looking at log.length.)
      var logLength = log ? log.length + 1 : 0;
      {{{ makeSetValue('p', '0', 'logLength', 'i32') }}};
    } else if (pname == 0x8B88) { // GL_SHADER_SOURCE_LENGTH
      var source = GLctx.getShaderSource(GL.shaders[shader]);
      // source may be a null, or the empty string, both of which are falsey
      // values that we report a 0 length for.
      var sourceLength = source ? source.length + 1 : 0;
      {{{ makeSetValue('p', '0', 'sourceLength', 'i32') }}};
    } else {
      {{{ makeSetValue('p', '0', 'GLctx.getShaderParameter(GL.shaders[shader], pname)', 'i32') }}};
    }
  },

  glGetProgramiv : (program, pname, p) => {
    if (!p) {
      // GLES2 specification does not specify how to behave if p is a null
      // pointer. Since calling this function does not make sense if p == null,
      // issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetProgramiv(program=${program}, pname=${pname}, p=0): Function called with null out pointer!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramiv', 'program');
#endif

    if (program >= GL.counter) {
#if GL_ASSERTIONS
      err(`GL_INVALID_VALUE in glGetProgramiv(program=${program}, pname=${pname}, p=${ptrToString(p)}): The specified program object name was not generated by GL!`);
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }

    program = GL.programs[program];

    if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
      var log = GLctx.getProgramInfoLog(program);
#if GL_ASSERTIONS || GL_TRACK_ERRORS
      if (log === null) log = '(unknown error)';
#endif
      {{{ makeSetValue('p', '0', 'log.length + 1', 'i32') }}};
    } else if (pname == 0x8B87 /* GL_ACTIVE_UNIFORM_MAX_LENGTH */) {
      if (!program.maxUniformLength) {
        var numActiveUniforms = GLctx.getProgramParameter(program, 0x8B86/*GL_ACTIVE_UNIFORMS*/);
        for (var i = 0; i < numActiveUniforms; ++i) {
          program.maxUniformLength = Math.max(program.maxUniformLength, GLctx.getActiveUniform(program, i).name.length+1);
        }
      }
      {{{ makeSetValue('p', '0', 'program.maxUniformLength', 'i32') }}};
    } else if (pname == 0x8B8A /* GL_ACTIVE_ATTRIBUTE_MAX_LENGTH */) {
      if (!program.maxAttributeLength) {
        var numActiveAttributes = GLctx.getProgramParameter(program, 0x8B89/*GL_ACTIVE_ATTRIBUTES*/);
        for (var i = 0; i < numActiveAttributes; ++i) {
          program.maxAttributeLength = Math.max(program.maxAttributeLength, GLctx.getActiveAttrib(program, i).name.length+1);
        }
      }
      {{{ makeSetValue('p', '0', 'program.maxAttributeLength', 'i32') }}};
    } else if (pname == 0x8A35 /* GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH */) {
      if (!program.maxUniformBlockNameLength) {
        var numActiveUniformBlocks = GLctx.getProgramParameter(program, 0x8A36/*GL_ACTIVE_UNIFORM_BLOCKS*/);
        for (var i = 0; i < numActiveUniformBlocks; ++i) {
          program.maxUniformBlockNameLength = Math.max(program.maxUniformBlockNameLength, GLctx.getActiveUniformBlockName(program, i).length+1);
        }
      }
      {{{ makeSetValue('p', '0', 'program.maxUniformBlockNameLength', 'i32') }}};
    } else {
      {{{ makeSetValue('p', '0', 'GLctx.getProgramParameter(program, pname)', 'i32') }}};
    }
  },

  glIsShader: (shader) => {
    var s = GL.shaders[shader];
    if (!s) return 0;
    return GLctx.isShader(s);
  },

  glCreateProgram: () => {
    var id = GL.getNewId(GL.programs);
    var program = GLctx.createProgram();
    // Store additional information needed for each shader program:
    program.name = id;
    // Lazy cache results of
    // glGetProgramiv(GL_ACTIVE_UNIFORM_MAX_LENGTH/GL_ACTIVE_ATTRIBUTE_MAX_LENGTH/GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH)
    program.maxUniformLength = program.maxAttributeLength = program.maxUniformBlockNameLength = 0;
    program.uniformIdCounter = 1;
    GL.programs[id] = program;
    return id;
  },

  glDeleteProgram: (id) => {
    if (!id) return;
    var program = GL.programs[id];
    if (!program) {
      // glDeleteProgram actually signals an error when deleting a nonexisting
      // object, unlike some other GL delete functions.
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    GLctx.deleteProgram(program);
    program.name = 0;
    GL.programs[id] = null;
  },

  glAttachShader: (program, shader) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glAttachShader', 'program');
    GL.validateGLObjectID(GL.shaders, shader, 'glAttachShader', 'shader');
#endif
#if GL_EXPLICIT_UNIFORM_LOCATION || GL_EXPLICIT_UNIFORM_BINDING
    program = GL.programs[program];
    shader = GL.shaders[shader];
    program[shader.shaderType] = shader;
    GLctx.attachShader(program, shader);
#else
    GLctx.attachShader(GL.programs[program], GL.shaders[shader]);
#endif
  },

  glDetachShader: (program, shader) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glDetachShader', 'program');
    GL.validateGLObjectID(GL.shaders, shader, 'glDetachShader', 'shader');
#endif
    GLctx.detachShader(GL.programs[program], GL.shaders[shader]);
  },

  glGetShaderPrecisionFormat: (shaderType, precisionType, range, precision) => {
    var result = GLctx.getShaderPrecisionFormat(shaderType, precisionType);
    {{{ makeSetValue('range', '0', 'result.rangeMin', 'i32') }}};
    {{{ makeSetValue('range', '4', 'result.rangeMax', 'i32') }}};
    {{{ makeSetValue('precision', '0', 'result.precision', 'i32') }}};
  },

  glLinkProgram: (program) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glLinkProgram', 'program');
#endif
    program = GL.programs[program];
    GLctx.linkProgram(program);
#if GL_DEBUG
    var log = (GLctx.getProgramInfoLog(program) || '').trim();
    if (log) dbg(`glLinkProgram: ${log}`);
    if (program.uniformLocsById) dbg(`glLinkProgram invalidated ${Object.keys(program.uniformLocsById).length} uniform location mappings`);
#endif
    // Invalidate earlier computed uniform->ID mappings, those have now become stale
    program.uniformLocsById = 0; // Mark as null-like so that glGetUniformLocation() knows to populate this again.
    program.uniformSizeAndIdsByName = {};

#if GL_EXPLICIT_UNIFORM_LOCATION
    // Collect explicit uniform locations from the vertex and fragment shaders.
    [program['vs'], program['fs']].forEach((s) => {
      Object.keys(s.explicitUniformLocations).forEach((shaderLocation) => {
        var loc = s.explicitUniformLocations[shaderLocation];
        // Record each explicit uniform location temporarily as a non-array uniform
        // with size=1. This is not true, but on the first glGetUniformLocation() call
        // the array sizes will get populated to correct sizes.
        program.uniformSizeAndIdsByName[shaderLocation] = [1, loc];
#if GL_DEBUG
        dbg(`Marking uniform ${loc} to location ${shaderLocation}`);
#endif

        // Make sure we will never automatically assign locations within the range
        // used for explicit layout(location=x) variables.
        program.uniformIdCounter = Math.max(program.uniformIdCounter, loc + 1);
      });
    });
#endif

#if GL_EXPLICIT_UNIFORM_BINDING
    function copyKeys(dst, src) {
      Object.keys(src).forEach((key) => { dst[key] = src[key] });
    }
    // Collect sampler and ubo binding locations from the vertex and fragment shaders.
    program.explicitUniformBindings = {};
    program.explicitSamplerBindings = {};
    [program['vs'], program['fs']].forEach((s) => {
      copyKeys(program.explicitUniformBindings, s.explicitUniformBindings);
      copyKeys(program.explicitSamplerBindings, s.explicitSamplerBindings);
    });
    // Record that we need to apply these explicit bindings when glUseProgram() is
    // first called on this program.
    program.explicitProgramBindingsApplied = 0;
#endif
  },

  glGetProgramInfoLog: (program, maxLength, length, infoLog) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramInfoLog', 'program');
#endif
    var log = GLctx.getProgramInfoLog(GL.programs[program]);
#if GL_ASSERTIONS || GL_TRACK_ERRORS
    if (log === null) log = '(unknown error)';
#endif
    var numBytesWrittenExclNull = (maxLength > 0 && infoLog) ? stringToUTF8(log, infoLog, maxLength) : 0;
    if (length) {{{ makeSetValue('length', '0', 'numBytesWrittenExclNull', 'i32') }}};
  },

#if GL_EXPLICIT_UNIFORM_BINDING
  // Applies the explicit sampler and ubo binding locations to the current program. This is done
  // lazily the first time we glUseProgram() so that parallel shader compilation is not disturbed.
  $webglApplyExplicitProgramBindings: () => {
    var p = GLctx.currentProgram;
    if (!p.explicitProgramBindingsApplied) {
#if MAX_WEBGL_VERSION >= 2
#if MIN_WEBGL_VERSION < 2
      if (GL.currentContext.version >= 2) {
#endif
        Object.keys(p.explicitUniformBindings).forEach((ubo) => {
          var bindings = p.explicitUniformBindings[ubo];
          for (var i = 0; i < bindings[1]; ++i) {
            var blockIndex = GLctx.getUniformBlockIndex(p, ubo + (bindings[1] > 1 ? `[${i}]` : ''));
#if GL_DEBUG
            dbg('Applying initial UBO binding point ' + (bindings[0]+i) + ' for UBO "' + (ubo + (bindings[1] > 1 ? '[' + i + ']' : '')) + '" at block index ' + blockIndex + ' ' + (bindings[1] > 1 ? ' (array index='+i+')' : ''));
#endif
            GLctx.uniformBlockBinding(p, blockIndex, bindings[0]+i);
          }
        });
#if MIN_WEBGL_VERSION < 2
      }
#endif
#endif
      Object.keys(p.explicitSamplerBindings).forEach((sampler) => {
        var bindings = p.explicitSamplerBindings[sampler];
        for (var i = 0; i < bindings[1]; ++i) {
#if GL_DEBUG
          dbg('Applying initial sampler binding point ' + (bindings[0]+i) + ' for sampler "' + sampler + (i > 0 ? '['+i+']' : '') +  '"');
#endif
          GLctx.uniform1i(GLctx.getUniformLocation(p, sampler + (i ? `[${i}]` : '')), bindings[0]+i);
        }
      });
      p.explicitProgramBindingsApplied = 1;
    }
  },
#endif

#if GL_EXPLICIT_UNIFORM_BINDING
  glUseProgram__deps: ['$webglApplyExplicitProgramBindings'],
#endif
  glUseProgram: (program) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glUseProgram', 'program');
#endif
    program = GL.programs[program];
    GLctx.useProgram(program);
    // Record the currently active program so that we can access the uniform
    // mapping table of that program.
#if GL_EXPLICIT_UNIFORM_BINDING
    if ((GLctx.currentProgram = program)) {
      webglApplyExplicitProgramBindings();
    }
#else
    GLctx.currentProgram = program;
#endif
  },

  glValidateProgram: (program) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glValidateProgram', 'program');
#endif
    GLctx.validateProgram(GL.programs[program]);
  },

  glIsProgram: (program) => {
    program = GL.programs[program];
    if (!program) return 0;
    return GLctx.isProgram(program);
  },

  glBindAttribLocation: (program, index, name) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glBindAttribLocation', 'program');
#endif
    GLctx.bindAttribLocation(GL.programs[program], index, UTF8ToString(name));
  },

  glBindFramebuffer: (target, framebuffer) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.framebuffers, framebuffer, 'glBindFramebuffer', 'framebuffer');
#endif

#if OFFSCREEN_FRAMEBUFFER
    // defaultFbo may not be present if 'renderViaOffscreenBackBuffer' was not enabled during context creation time,
    // i.e. setting -sOFFSCREEN_FRAMEBUFFER at compilation time does not yet mandate that offscreen back buffer
    // is being used, but that is ultimately decided at context creation time.
    GLctx.bindFramebuffer(target, framebuffer ? GL.framebuffers[framebuffer] : GL.currentContext.defaultFbo);
#else
    GLctx.bindFramebuffer(target, GL.framebuffers[framebuffer]);
#endif

  },

  glGenFramebuffers: (n, ids) => {
    GL.genObject(n, ids, 'createFramebuffer', GL.framebuffers
#if GL_ASSERTIONS
    , 'glGenFramebuffers'
#endif
      );
  },

  glDeleteFramebuffers: (n, framebuffers) => {
    for (var i = 0; i < n; ++i) {
      var id = {{{ makeGetValue('framebuffers', 'i*4', 'i32') }}};
      var framebuffer = GL.framebuffers[id];
      if (!framebuffer) continue; // GL spec: "glDeleteFramebuffers silently ignores 0s and names that do not correspond to existing framebuffer objects".
      GLctx.deleteFramebuffer(framebuffer);
      framebuffer.name = 0;
      GL.framebuffers[id] = null;
    }
  },

  glFramebufferRenderbuffer: (target, attachment, renderbuffertarget, renderbuffer) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.renderbuffers, renderbuffer, 'glFramebufferRenderbuffer', 'renderbuffer');
#endif
    GLctx.framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                       GL.renderbuffers[renderbuffer]);
  },

  glFramebufferTexture2D: (target, attachment, textarget, texture, level) => {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.textures, texture, 'glFramebufferTexture2D', 'texture');
#endif
    GLctx.framebufferTexture2D(target, attachment, textarget,
                                    GL.textures[texture], level);
  },

  glGetFramebufferAttachmentParameteriv: (target, attachment, pname, params) => {
    var result = GLctx.getFramebufferAttachmentParameter(target, attachment, pname);
    if (result instanceof WebGLRenderbuffer ||
        result instanceof WebGLTexture) {
      result = result.name | 0;
    }
    {{{ makeSetValue('params', '0', 'result', 'i32') }}};
  },

  glIsFramebuffer: (framebuffer) => {
    var fb = GL.framebuffers[framebuffer];
    if (!fb) return 0;
    return GLctx.isFramebuffer(fb);
  },

#if LEGACY_GL_EMULATION
  glGenVertexArrays__deps: ['$emulGlGenVertexArrays'],
#endif
  glGenVertexArrays: (n, arrays) => {
#if LEGACY_GL_EMULATION
    emulGlGenVertexArrays(n, arrays);
#else
#if GL_ASSERTIONS
    assert(GLctx.createVertexArray, 'Must have WebGL2 or OES_vertex_array_object to use vao');
#endif
    GL.genObject(n, arrays, 'createVertexArray', GL.vaos
#if GL_ASSERTIONS
    , 'glGenVertexArrays'
#endif
      );
#endif
  },

#if LEGACY_GL_EMULATION
  glDeleteVertexArrays__deps: ['$emulGlDeleteVertexArrays'],
#endif
  glDeleteVertexArrays: (n, vaos) => {
#if LEGACY_GL_EMULATION
    emulGlDeleteVertexArrays(n, vaos);
#else
#if GL_ASSERTIONS
    assert(GLctx.deleteVertexArray, 'Must have WebGL2 or OES_vertex_array_object to use vao');
#endif
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('vaos', 'i*4', 'i32') }}};
      GLctx.deleteVertexArray(GL.vaos[id]);
      GL.vaos[id] = null;
    }
#endif
  },

#if LEGACY_GL_EMULATION
  glBindVertexArray__deps: ['$emulGlBindVertexArray'],
#endif
  glBindVertexArray: (vao) => {
#if LEGACY_GL_EMULATION
    emulGlBindVertexArray(vao);
#else
#if GL_ASSERTIONS
    assert(GLctx.bindVertexArray, 'Must have WebGL2 or OES_vertex_array_object to use vao');
#endif
    GLctx.bindVertexArray(GL.vaos[vao]);
#endif
#if FULL_ES2 || LEGACY_GL_EMULATION
    var ibo = GLctx.getParameter(0x8895 /*ELEMENT_ARRAY_BUFFER_BINDING*/);
    GLctx.currentElementArrayBufferBinding = ibo ? (ibo.name | 0) : 0;
#endif
  },

#if LEGACY_GL_EMULATION
  glIsVertexArray__deps: ['$emulGlIsVertexArray'],
#endif
  glIsVertexArray: (array) => {
#if LEGACY_GL_EMULATION
    return emulGlIsVertexArray(array);
#else
#if GL_ASSERTIONS
    assert(GLctx.isVertexArray, 'Must have WebGL2 or OES_vertex_array_object to use vao');
#endif

    var vao = GL.vaos[array];
    if (!vao) return 0;
    return GLctx.isVertexArray(vao);
#endif
  },

#if !LEGACY_GL_EMULATION

  glVertexPointer: (size, type, stride, ptr) => {
    throw 'Legacy GL function (glVertexPointer) called. If you want legacy GL emulation, you need to compile with -sLEGACY_GL_EMULATION to enable legacy GL emulation.';
  },
  glMatrixMode: () => {
    throw 'Legacy GL function (glMatrixMode) called. If you want legacy GL emulation, you need to compile with -sLEGACY_GL_EMULATION to enable legacy GL emulation.';
  },
  glBegin: () => {
    throw 'Legacy GL function (glBegin) called. If you want legacy GL emulation, you need to compile with -sLEGACY_GL_EMULATION to enable legacy GL emulation.';
  },
  glLoadIdentity: () => {
    throw 'Legacy GL function (glLoadIdentity) called. If you want legacy GL emulation, you need to compile with -sLEGACY_GL_EMULATION to enable legacy GL emulation.';
  },

#endif // LEGACY_GL_EMULATION

  // Open GLES1.1 vao compatibility (Could work w/o -sLEGACY_GL_EMULATION)

  glGenVertexArraysOES: 'glGenVertexArrays',
  glDeleteVertexArraysOES: 'glDeleteVertexArrays',
  glBindVertexArrayOES: 'glBindVertexArray',
  glIsVertexArrayOES: 'glIsVertexArray',

  // GLES2 emulation

  glVertexAttribPointer: (index, size, type, normalized, stride, ptr) => {
#if FULL_ES2
    var cb = GL.currentContext.clientBuffers[index];
#if GL_ASSERTIONS
    assert(cb, index);
#endif
    if (!GLctx.currentArrayBufferBinding) {
      cb.size = size;
      cb.type = type;
      cb.normalized = normalized;
      cb.stride = stride;
      cb.ptr = ptr;
      cb.clientside = true;
      cb.vertexAttribPointerAdaptor = function(index, size, type, normalized, stride, ptr) {
        this.vertexAttribPointer(index, size, type, normalized, stride, ptr);
      };
      return;
    }
    cb.clientside = false;
#endif
#if GL_ASSERTIONS
    GL.validateVertexAttribPointer(size, type, stride, ptr);
#endif
    GLctx.vertexAttribPointer(index, size, type, !!normalized, stride, ptr);
  },

  glEnableVertexAttribArray: (index) => {
#if FULL_ES2
    var cb = GL.currentContext.clientBuffers[index];
#if GL_ASSERTIONS
    assert(cb, index);
#endif
    cb.enabled = true;
#endif
    GLctx.enableVertexAttribArray(index);
  },

  glDisableVertexAttribArray: (index) => {
#if FULL_ES2
    var cb = GL.currentContext.clientBuffers[index];
#if GL_ASSERTIONS
    assert(cb, index);
#endif
    cb.enabled = false;
#endif
    GLctx.disableVertexAttribArray(index);
  },

#if !LEGACY_GL_EMULATION
  glDrawArrays: (mode, first, count) => {
#if FULL_ES2
    // bind any client-side buffers
    GL.preDrawHandleClientVertexAttribBindings(first + count);
#endif

    GLctx.drawArrays(mode, first, count);

#if FULL_ES2
    GL.postDrawHandleClientVertexAttribBindings();
#endif
  },

  glDrawElements: (mode, count, type, indices) => {
#if FULL_ES2
    var buf;
    var vertexes = 0;
    if (!GLctx.currentElementArrayBufferBinding) {
      var size = GL.calcBufLength(1, type, 0, count);
      buf = GL.getTempIndexBuffer(size);
      GLctx.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, buf);
      GLctx.bufferSubData(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/,
                          0,
                          HEAPU8.subarray(indices, indices + size));
      
      // Calculating vertex count if shader's attribute data is on client side
      if (count > 0) {
        for (var i = 0; i < GL.currentContext.maxVertexAttribs; ++i) {
          var cb = GL.currentContext.clientBuffers[i];
          if (cb.clientside && cb.enabled) {
            let arrayClass;
            switch(type) {
              case 0x1401 /* GL_UNSIGNED_BYTE */: arrayClass = Uint8Array; break;
              case 0x1403 /* GL_UNSIGNED_SHORT */: arrayClass = Uint16Array; break;
#if FULL_ES3
              case 0x1405 /* GL_UNSIGNED_INT */: arrayClass = Uint32Array; break;
#endif
              default:
                GL.recordError(0x502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
                err('type is not supported in glDrawElements');
#endif
                return;
            }

            vertexes = new arrayClass(HEAPU8.buffer, indices, count).reduce((max, current) => Math.max(max, current)) + 1;
            break;
          }
        }
      }

      // the index is now 0
      indices = 0;
    }

    // bind any client-side buffers
    GL.preDrawHandleClientVertexAttribBindings(vertexes);
#endif

    GLctx.drawElements(mode, count, type, indices);

#if FULL_ES2
    GL.postDrawHandleClientVertexAttribBindings(count);

    if (!GLctx.currentElementArrayBufferBinding) {
      GLctx.bindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, null);
    }
#endif
  },
#endif // ~#if !LEGACY_GL_EMULATION

  glShaderBinary: (count, shaders, binaryformat, binary, length) => {
    GL.recordError(0x500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
    err("GL_INVALID_ENUM in glShaderBinary: WebGL does not support binary shader formats! Calls to glShaderBinary always fail.");
#endif
  },

  glReleaseShaderCompiler: () => {
    // NOP (as allowed by GLES 2.0 spec)
  },

  glGetError: () => {
#if GL_TRACK_ERRORS
    var error = GLctx.getError() || GL.lastError;
    GL.lastError = 0/*GL_NO_ERROR*/;
    return error;
#else
    return GLctx.getError();
#endif
  },

  // ANGLE_instanced_arrays WebGL extension related functions (in core in WebGL 2)

  glVertexAttribDivisor: (index, divisor) => {
#if GL_ASSERTIONS
    assert(GLctx.vertexAttribDivisor, 'Must have ANGLE_instanced_arrays extension or WebGL 2 to use WebGL instancing');
#endif
    GLctx.vertexAttribDivisor(index, divisor);
  },

  glDrawArraysInstanced: (mode, first, count, primcount) => {
#if GL_ASSERTIONS
    assert(GLctx.drawArraysInstanced, 'Must have ANGLE_instanced_arrays extension or WebGL 2 to use WebGL instancing');
#endif
    GLctx.drawArraysInstanced(mode, first, count, primcount);
  },

  glDrawElementsInstanced: (mode, count, type, indices, primcount) => {
#if GL_ASSERTIONS
    assert(GLctx.drawElementsInstanced, 'Must have ANGLE_instanced_arrays extension or WebGL 2 to use WebGL instancing');
#endif
    GLctx.drawElementsInstanced(mode, count, type, indices, primcount);
  },

  // OpenGL Desktop/ES 2.0 instancing extensions compatibility

  glVertexAttribDivisorNV: 'glVertexAttribDivisor',
  glDrawArraysInstancedNV: 'glDrawArraysInstanced',
  glDrawElementsInstancedNV: 'glDrawElementsInstanced',
  glVertexAttribDivisorEXT: 'glVertexAttribDivisor',
  glDrawArraysInstancedEXT: 'glDrawArraysInstanced',
  glDrawElementsInstancedEXT: 'glDrawElementsInstanced',
  glVertexAttribDivisorARB: 'glVertexAttribDivisor',
  glDrawArraysInstancedARB: 'glDrawArraysInstanced',
  glDrawElementsInstancedARB: 'glDrawElementsInstanced',
  glVertexAttribDivisorANGLE: 'glVertexAttribDivisor',
  glDrawArraysInstancedANGLE: 'glDrawArraysInstanced',
  glDrawElementsInstancedANGLE: 'glDrawElementsInstanced',


  glDrawBuffers__deps: ['$tempFixedLengthArray'],
  glDrawBuffers: (n, bufs) => {
#if GL_ASSERTIONS
    assert(GLctx.drawBuffers, 'Must have WebGL2 or WEBGL_draw_buffers extension to use drawBuffers');
#endif
#if GL_ASSERTIONS
    assert(n < tempFixedLengthArray.length, `Invalid count of numBuffers=${n} passed to glDrawBuffers (that many draw buffer points do not exist in GL)`);
#endif

    var bufArray = tempFixedLengthArray[n];
    for (var i = 0; i < n; i++) {
      bufArray[i] = {{{ makeGetValue('bufs', 'i*4', 'i32') }}};
    }

    GLctx.drawBuffers(bufArray);
  },

  // OpenGL ES 2.0 draw buffer extensions compatibility

  glDrawBuffersEXT: 'glDrawBuffers',
  glDrawBuffersWEBGL: 'glDrawBuffers',

  // passthrough functions with GLboolean parameters

  glColorMask: (red, green, blue, alpha) => {
    GLctx.colorMask(!!red, !!green, !!blue, !!alpha);
  },

  glDepthMask: (flag) => {
    GLctx.depthMask(!!flag);
  },

  glSampleCoverage: (value, invert) => {
    GLctx.sampleCoverage(value, !!invert);
  },

  glMultiDrawArraysWEBGL__sig: 'vippi',
  glMultiDrawArrays: 'glMultiDrawArraysWEBGL',
  glMultiDrawArraysANGLE: 'glMultiDrawArraysWEBGL',
  glMultiDrawArraysWEBGL: (mode, firsts, counts, drawcount) => {
    GLctx.multiDrawWebgl['multiDrawArraysWEBGL'](
      mode,
      HEAP32,
      {{{ getHeapOffset('firsts', 'i32') }}},
      HEAP32,
      {{{ getHeapOffset('counts', 'i32') }}},
      drawcount);
  },

  glMultiDrawArraysInstancedWEBGL__sig: 'vipppi',
  glMultiDrawArraysInstancedANGLE: 'glMultiDrawArraysInstancedWEBGL',
  glMultiDrawArraysInstancedWEBGL: (mode, firsts, counts, instanceCounts, drawcount) => {
    GLctx.multiDrawWebgl['multiDrawArraysInstancedWEBGL'](
      mode,
      HEAP32,
      {{{ getHeapOffset('firsts', 'i32') }}},
      HEAP32,
      {{{ getHeapOffset('counts', 'i32') }}},
      HEAP32,
      {{{ getHeapOffset('instanceCounts', 'i32') }}},
      drawcount);
  },

#if MEMORY64
  // Convert an array of i64 offsets to an array of i32 offsets returning a
  // pointer to the new (stack allocated) array.
  $convertOffsets__deps: ['$stackAlloc'],
  $convertOffsets__internal: true,
  $convertOffsets: (offsets, count) => {
    var offsets32 = stackAlloc(count * 4);
    var i64ptr = offsets >> 3;
    var i32ptr = offsets32 >> 2;
    for (var i = 0; i < count; i++, i32ptr++, i64ptr++) {
      var i64val = HEAPU64[i64ptr];
      assert(i64val >= 0 && i32ptr <= 0xffffffff);
      HEAPU32[i32ptr] = Number(i64val);
    }
    return offsets32;
  },
#endif

  glMultiDrawElementsWEBGL__sig: 'vipipi',
  glMultiDrawElements: 'glMultiDrawElementsWEBGL',
  glMultiDrawElementsANGLE: 'glMultiDrawElementsWEBGL',
#if MEMORY64
  glMultiDrawElementsWEBGL__deps: ['$convertOffsets', '$stackSave', '$stackRestore'],
#endif
  glMultiDrawElementsWEBGL: (mode, counts, type, offsets, drawcount) => {
#if MEMORY64
    var stack = stackSave();
    offsets = convertOffsets(offsets, drawcount);
#endif
    GLctx.multiDrawWebgl['multiDrawElementsWEBGL'](
      mode,
      HEAP32,
      {{{ getHeapOffset('counts', 'i32') }}},
      type,
      HEAP32,
      {{{ getHeapOffset('offsets', 'i32') }}},
      drawcount);
#if MEMORY64
    stackRestore(stack);
#endif
  },

  glMultiDrawElementsInstancedWEBGL__sig: 'vipippi',
  glMultiDrawElementsInstancedANGLE: 'glMultiDrawElementsInstancedWEBGL',
#if MEMORY64
  glMultiDrawElementsInstancedWEBGL__deps: ['$convertOffsets', '$stackSave', '$stackRestore'],
#endif
  glMultiDrawElementsInstancedWEBGL: (mode, counts, type, offsets, instanceCounts, drawcount) => {
#if MEMORY64
    var stack = stackSave();
    offsets = convertOffsets(offsets, drawcount);
#endif
    GLctx.multiDrawWebgl['multiDrawElementsInstancedWEBGL'](
      mode,
      HEAP32,
      {{{ getHeapOffset('counts', 'i32') }}},
      type,
      HEAP32,
      {{{ getHeapOffset('offsets', 'i32') }}},
      HEAP32,
      {{{ getHeapOffset('instanceCounts', 'i32') }}},
      drawcount);
#if MEMORY64
    stackRestore(stack);
#endif
  },

  // As a small peculiarity, we currently allow building with -sFULL_ES3 to emulate client side arrays,
  // but without targeting WebGL 2, so this FULL_ES3 block is in library_webgl.js instead of library_webgl2.js
#if FULL_ES3
  $emscriptenWebGLGetBufferBinding: (target) => {
    switch (target) {
      case 0x8892 /*GL_ARRAY_BUFFER*/: target = 0x8894 /*GL_ARRAY_BUFFER_BINDING*/; break;
      case 0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/: target = 0x8895 /*GL_ELEMENT_ARRAY_BUFFER_BINDING*/; break;
      case 0x88EB /*GL_PIXEL_PACK_BUFFER*/: target = 0x88ED /*GL_PIXEL_PACK_BUFFER_BINDING*/; break;
      case 0x88EC /*GL_PIXEL_UNPACK_BUFFER*/: target = 0x88EF /*GL_PIXEL_UNPACK_BUFFER_BINDING*/; break;
      case 0x8C8E /*GL_TRANSFORM_FEEDBACK_BUFFER*/: target = 0x8C8F /*GL_TRANSFORM_FEEDBACK_BUFFER_BINDING*/; break;
      case 0x8F36 /*GL_COPY_READ_BUFFER*/: target = 0x8F36 /*GL_COPY_READ_BUFFER_BINDING*/; break;
      case 0x8F37 /*GL_COPY_WRITE_BUFFER*/: target = 0x8F37 /*GL_COPY_WRITE_BUFFER_BINDING*/; break;
      case 0x8A11 /*GL_UNIFORM_BUFFER*/: target = 0x8A28 /*GL_UNIFORM_BUFFER_BINDING*/; break;
      // In default case, fall through and assume passed one of the _BINDING enums directly.
    }
    var buffer = GLctx.getParameter(target);
    if (buffer) return buffer.name|0;
    else return 0;
  },

  $emscriptenWebGLValidateMapBufferTarget: (target) => {
    switch (target) {
      case 0x8892: // GL_ARRAY_BUFFER
      case 0x8893: // GL_ELEMENT_ARRAY_BUFFER
      case 0x8F36: // GL_COPY_READ_BUFFER
      case 0x8F37: // GL_COPY_WRITE_BUFFER
      case 0x88EB: // GL_PIXEL_PACK_BUFFER
      case 0x88EC: // GL_PIXEL_UNPACK_BUFFER
      case 0x8C2A: // GL_TEXTURE_BUFFER
      case 0x8C8E: // GL_TRANSFORM_FEEDBACK_BUFFER
      case 0x8A11: // GL_UNIFORM_BUFFER
        return true;
      default:
        return false;
    }
  },

  glMapBufferRange__deps: ['$emscriptenWebGLGetBufferBinding', '$emscriptenWebGLValidateMapBufferTarget', 'malloc'],
  glMapBufferRange: (target, offset, length, access) => {
    if ((access & (0x1/*GL_MAP_READ_BIT*/ | 0x20/*GL_MAP_UNSYNCHRONIZED_BIT*/)) != 0) {
      err("glMapBufferRange access does not support MAP_READ or MAP_UNSYNCHRONIZED");
      return 0;
    }

    if ((access & 0x2/*GL_MAP_WRITE_BIT*/) == 0) {
      err("glMapBufferRange access must include MAP_WRITE");
      return 0;
    }

    if ((access & (0x4/*GL_MAP_INVALIDATE_BUFFER_BIT*/ | 0x8/*GL_MAP_INVALIDATE_RANGE_BIT*/)) == 0) {
      err("glMapBufferRange access must include INVALIDATE_BUFFER or INVALIDATE_RANGE");
      return 0;
    }

    if (!emscriptenWebGLValidateMapBufferTarget(target)) {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glMapBufferRange');
      return 0;
    }

    var mem = _malloc(length), binding = emscriptenWebGLGetBufferBinding(target);
    if (!mem) return 0;

    binding = GL.mappedBuffers[binding] ??= {};
    binding.offset = offset;
    binding.length = length;
    binding.mem = mem;
    binding.access = access;
    return mem;
  },

  glGetBufferPointerv__deps: ['$emscriptenWebGLGetBufferBinding'],
  glGetBufferPointerv: (target, pname, params) => {
    if (pname == 0x88BD/*GL_BUFFER_MAP_POINTER*/) {
      var ptr = 0;
      var mappedBuffer = GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];
      if (mappedBuffer) {
        ptr = mappedBuffer.mem;
      }
      {{{ makeSetValue('params', '0', 'ptr', 'i32') }}};
    } else {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glGetBufferPointerv');
    }
  },

  glFlushMappedBufferRange__deps: ['$emscriptenWebGLGetBufferBinding', '$emscriptenWebGLValidateMapBufferTarget'],
  glFlushMappedBufferRange: (target, offset, length) => {
    if (!emscriptenWebGLValidateMapBufferTarget(target)) {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glFlushMappedBufferRange');
      return;
    }

    var mapping = GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];
    if (!mapping) {
      GL.recordError(0x502 /* GL_INVALID_OPERATION */);
      err('buffer was never mapped in glFlushMappedBufferRange');
      return;
    }

    if (!(mapping.access & 0x10)) {
      GL.recordError(0x502 /* GL_INVALID_OPERATION */);
      err('buffer was not mapped with GL_MAP_FLUSH_EXPLICIT_BIT in glFlushMappedBufferRange');
      return;
    }
    if (offset < 0 || length < 0 || offset + length > mapping.length) {
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      err('invalid range in glFlushMappedBufferRange');
      return;
    }

    GLctx.bufferSubData(
      target,
      mapping.offset,
      HEAPU8.subarray(mapping.mem + offset, mapping.mem + offset + length));
  },

  glUnmapBuffer__deps: ['$emscriptenWebGLGetBufferBinding', '$emscriptenWebGLValidateMapBufferTarget', 'free'],
  glUnmapBuffer: (target) => {
    if (!emscriptenWebGLValidateMapBufferTarget(target)) {
      GL.recordError(0x500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glUnmapBuffer');
      return 0;
    }

    var buffer = emscriptenWebGLGetBufferBinding(target);
    var mapping = GL.mappedBuffers[buffer];
    if (!mapping || !mapping.mem) {
      GL.recordError(0x502 /* GL_INVALID_OPERATION */);
      err('buffer was never mapped in glUnmapBuffer');
      return 0;
    }

    if (!(mapping.access & 0x10)) { /* GL_MAP_FLUSH_EXPLICIT_BIT */
#if WEBGL_USE_GARBAGE_FREE_APIS
      if ({{{ isCurrentContextWebGL2() }}}) {
        GLctx.bufferSubData(target, mapping.offset, HEAPU8, mapping.mem, mapping.length);
      } else
#endif
      GLctx.bufferSubData(target, mapping.offset, HEAPU8.subarray(mapping.mem, mapping.mem+mapping.length));
    }
    _free(mapping.mem);
    mapping.mem = 0;
    return 1;
  },
#endif

  glPolygonOffsetClampEXT__sig: 'vfff',
  glPolygonOffsetClampEXT: (factor, units, clamp) => {
#if GL_ASSERTIONS
    assert(GLctx.extPolygonOffsetClamp, "EXT_polygon_offset_clamp not supported, or not enabled. Before calling glPolygonOffsetClampEXT(), call emscripten_webgl_enable_EXT_polygon_offset_clamp() to enable this extension, and verify that it returns true to indicate support. (alternatively, build with -sGL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1 to enable all GL extensions by default)");
#endif
    GLctx.extPolygonOffsetClamp['polygonOffsetClampEXT'](factor, units, clamp);
  },

  glClipControlEXT__sig: 'vii',
  glClipControlEXT: (origin, depth) => {
#if GL_ASSERTIONS
    assert(GLctx.extClipControl, "EXT_clip_control not supported, or not enabled. Before calling glClipControlEXT(), call emscripten_webgl_enable_EXT_clip_control() to enable this extension, and verify that it returns true to indicate support. (alternatively, build with -sGL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1 to enable all GL extensions by default)");
#endif
    GLctx.extClipControl['clipControlEXT'](origin, depth);
  },

  glPolygonModeWEBGL__sig: 'vii',
  glPolygonModeWEBGL: (face, mode) => {
#if GL_ASSERTIONS
    assert(GLctx.webglPolygonMode, "WEBGL_polygon_mode not supported, or not enabled. Before calling glPolygonModeWEBGL(), call emscripten_webgl_enable_WEBGL_polygon_mode() to enable this extension, and verify that it returns true to indicate support. (alternatively, build with -sGL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1 to enable all GL extensions by default)");
#endif
    GLctx.webglPolygonMode['polygonModeWEBGL'](face, mode);
  },
};

#if !GL_ENABLE_GET_PROC_ADDRESS
[
  'emscripten_webgl1_get_proc_address',
  'emscripten_webgl2_get_proc_address',
  'emscripten_webgl_get_proc_address',
  'SDL_GL_GetProcAddress',
  'eglGetProcAddress',
  'glfwGetProcAddress'
].forEach((name) => {
  LibraryGL[name] = (name) => { abort(); return 0; };
  // Due to the two pass nature of compiling .js files,
  // in INCLUDE_FULL_LIBRARY mode, we must include the above
  // stub functions, but not their __deps message handlers.
#if !INCLUDE_FULL_LIBRARY
  LibraryGL[name + '__deps'] = [() => {
    error(`linker: Undefined symbol: ${name}(). Please pass -sGL_ENABLE_GET_PROC_ADDRESS at link time to link in ${name}().`);
  }];
#endif
});
#endif

// Simple pass-through functions.
// - Starred ones have return values.
// - [X] ones have X in the C name but not in the JS name
var glPassthroughFuncs = [
  [0, 'finish flush'],
  [1, 'clearDepth clearDepth[f] depthFunc enable disable frontFace cullFace clear lineWidth clearStencil stencilMask checkFramebufferStatus* generateMipmap activeTexture blendEquation isEnabled*'],
  [2, 'blendFunc blendEquationSeparate depthRange depthRange[f] stencilMaskSeparate hint polygonOffset vertexAttrib1f'],
  [3, 'texParameteri texParameterf vertexAttrib2f stencilFunc stencilOp'],
  [4, 'viewport clearColor scissor vertexAttrib3f renderbufferStorage blendFuncSeparate blendColor stencilFuncSeparate stencilOpSeparate'],
  [5, 'vertexAttrib4f'],
  [8, 'copyTexImage2D copyTexSubImage2D'],
];

function createGLPassthroughFunctions(lib, funcs) {
  funcs.forEach((data) => {
    const num = data[0];
    const names = data[1];
    const args = range(num).map((i) => 'x' + i ).join(', ');
    const stub = `(${args}) => GLctx.NAME(${args})`;
    const sigEnd = range(num).map(() => 'i').join('');
    names.split(' ').forEach((name) => {
      let sig;
      if (name.endsWith('*')) {
        name = name.slice(0, -1);
        sig = 'i' + sigEnd;
      } else {
        sig = 'v' + sigEnd;
      }
      let cName = name;
      if (name.includes('[')) {
        cName = name.replace('[', '').replace(']', '');
        name = cName.slice(0, -1);
      }
      cName = 'gl' + cName[0].toUpperCase() + cName.substr(1);
      assert(!(cName in lib), "Cannot reimplement the existing function " + cName);
      lib[cName] = eval(stub.replace('NAME', name));
      assert(lib[cName + '__sig'] || LibraryManager.library[cName + '__sig'], 'missing sig for ' + cName);
    });
  });
}

createGLPassthroughFunctions(LibraryGL, glPassthroughFuncs);

autoAddDeps(LibraryGL, '$GL');

function recordGLProcAddressGet(lib) {
  // GL proc address retrieval - allow access through glX and emscripten_glX, to
  // allow name collisions with user-implemented things having the same name
  // (see gl.c)
  Object.keys(lib).forEach((x) => {
    if (x.startsWith('gl') && !isDecorator(x)) {
      lib['emscripten_' + x] = x;
      var sig = LibraryManager.library[x + '__sig'];
      if (sig) {
        lib['emscripten_' + x + '__sig'] = sig;
      }
    }
  });
}

recordGLProcAddressGet(LibraryGL);

// Final merge
addToLibrary(LibraryGL);
