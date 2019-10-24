/*
 * Copyright 2010 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * GL support. See http://kripken.github.io/emscripten-site/docs/porting/multimedia_and_graphics/OpenGL-support.html
 * for current status.
 */

var LibraryGL = {

  // For functions such as glDrawBuffers, glInvalidateFramebuffer and glInvalidateSubFramebuffer that need to pass a short array to the WebGL API,
  // create a set of short fixed-length arrays to avoid having to generate any garbage when calling those functions.
  _tempFixedLengthArray__postset: 'for (var i = 0; i < 32; i++) __tempFixedLengthArray.push(new Array(i));',
  _tempFixedLengthArray: [],

  $GL__postset: 'var GLctx; GL.init()',
  $GL: {
#if GL_DEBUG
    debug: true,
#endif

    counter: 1, // 0 is reserved as 'null' in gl
#if GL_TRACK_ERRORS
    lastError: 0,
#endif
    buffers: [],
    mappedBuffers: {},
    programs: [],
    framebuffers: [],
    renderbuffers: [],
    textures: [],
    uniforms: [],
    shaders: [],
    vaos: [],
    contexts: {},
    currentContext: null,
    offscreenCanvases: {}, // DOM ID -> OffscreenCanvas mappings of <canvas> elements that have their rendering control transferred to offscreen.
    timerQueriesEXT: [],
#if USE_WEBGL2
    queries: [],
    samplers: [],
    transformFeedbacks: [],
    syncs: [],
#endif

#if FULL_ES2 || LEGACY_GL_EMULATION
    currArrayBuffer: 0,
    currElementArrayBuffer: 0,

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

    programInfos: {}, // Stores additional information needed for each shader program. Each entry is of form:
    /* { uniforms: {}, // Maps ints back to the opaque WebGLUniformLocation objects.
         maxUniformLength: int, // Cached in order to implement glGetProgramiv(GL_ACTIVE_UNIFORM_MAX_LENGTH)
         maxAttributeLength: int // Cached in order to implement glGetProgramiv(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH)
         maxUniformBlockNameLength: int // Cached in order to implement glGetProgramiv(GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH)
       } */

    stringCache: {},
#if USE_WEBGL2
    stringiCache: {},
#endif

    unpackAlignment: 4, // default alignment is 4 bytes

    init: function() {
#if FULL_ES2 || LEGACY_GL_EMULATION
      GL.createLog2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);
#endif
#if GL_POOL_TEMP_BUFFERS
      GL.miniTempBuffer = new Float32Array(GL.MINI_TEMP_BUFFER_SIZE);
      for (var i = 0; i < GL.MINI_TEMP_BUFFER_SIZE; i++) {
        GL.miniTempBufferViews[i] = GL.miniTempBuffer.subarray(0, i+1);
      }
#endif
    },

    // Records a GL error condition that occurred, stored until user calls glGetError() to fetch it. As per GLES2 spec, only the first error
    // is remembered, and subsequent errors are discarded until the user has cleared the stored error by a call to glGetError().
    recordError: function recordError(errorCode) {
#if GL_TRACK_ERRORS
      if (!GL.lastError) {
        GL.lastError = errorCode;
      }
#endif
    },
    // Get a new ID for a texture/buffer/etc., while keeping the table dense and fast. Creation is fairly rare so it is worth optimizing lookups later.
    getNewId: function(table) {
      var ret = GL.counter++;
      for (var i = table.length; i < ret; i++) {
        table[i] = null;
      }
      return ret;
    },

#if GL_POOL_TEMP_BUFFERS
    // Mini temp buffer
    MINI_TEMP_BUFFER_SIZE: 256,
    miniTempBuffer: null,
    miniTempBufferViews: [0], // index i has the view of size i+1
#endif

#if FULL_ES2 || LEGACY_GL_EMULATION
    // When user GL code wants to render from client-side memory, we need to upload the vertex data to a temp VBO
    // for rendering. Maintain a set of temp VBOs that are created-on-demand to appropriate sizes, and never destroyed.
    // Also, for best performance the VBOs are double-buffered, i.e. every second frame we switch the set of VBOs we
    // upload to, so that rendering from the previous frame is not disturbed by uploading from new data to it, which
    // could cause a GPU-CPU pipeline stall.
    // Note that index buffers are not double-buffered (at the moment) in this manner.
    MAX_TEMP_BUFFER_SIZE: {{{ GL_MAX_TEMP_BUFFER_SIZE }}},
    // Maximum number of temp VBOs of one size to maintain, after that we start reusing old ones, which is safe but can give
    // a performance impact. If CPU-GPU stalls are a problem, increasing this might help.
    numTempVertexBuffersPerSize: 64, // (const)

    // Precompute a lookup table for the function ceil(log2(x)), i.e. how many bits are needed to represent x, or,
    // if x was rounded up to next pow2, which index is the single '1' bit at?
    // Then log2ceilLookup[x] returns ceil(log2(x)).
    log2ceilLookup: null,
    createLog2ceilLookup: function(maxValue) {
      GL.log2ceilLookup = new Uint8Array(maxValue+1);
      var log2 = 0;
      var pow2 = 1;
      GL.log2ceilLookup[0] = 0;
      for (var i = 1; i <= maxValue; ++i) {
        if (i > pow2) {
          pow2 <<= 1;
          ++log2;
        }
        GL.log2ceilLookup[i] = log2;
      }
    },

    generateTempBuffers: function(quads, context) {
      var largestIndex = GL.log2ceilLookup[GL.MAX_TEMP_BUFFER_SIZE];
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
        context.GLctx.bindBuffer(context.GLctx.ELEMENT_ARRAY_BUFFER, context.tempQuadIndexBuffer);
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
        context.GLctx.bufferData(context.GLctx.ELEMENT_ARRAY_BUFFER, quadIndexes, context.GLctx.STATIC_DRAW);
        context.GLctx.bindBuffer(context.GLctx.ELEMENT_ARRAY_BUFFER, null);
      }
    },

    getTempVertexBuffer: function getTempVertexBuffer(sizeBytes) {
      var idx = GL.log2ceilLookup[sizeBytes];
      var ringbuffer = GL.currentContext.tempVertexBuffers1[idx];
      var nextFreeBufferIndex = GL.currentContext.tempVertexBufferCounters1[idx];
      GL.currentContext.tempVertexBufferCounters1[idx] = (GL.currentContext.tempVertexBufferCounters1[idx]+1) & (GL.numTempVertexBuffersPerSize-1);
      var vbo = ringbuffer[nextFreeBufferIndex];
      if (vbo) {
        return vbo;
      }
      var prevVBO = GLctx.getParameter(GLctx.ARRAY_BUFFER_BINDING);
      ringbuffer[nextFreeBufferIndex] = GLctx.createBuffer();
      GLctx.bindBuffer(GLctx.ARRAY_BUFFER, ringbuffer[nextFreeBufferIndex]);
      GLctx.bufferData(GLctx.ARRAY_BUFFER, 1 << idx, GLctx.DYNAMIC_DRAW);
      GLctx.bindBuffer(GLctx.ARRAY_BUFFER, prevVBO);
      return ringbuffer[nextFreeBufferIndex];
    },

    getTempIndexBuffer: function getTempIndexBuffer(sizeBytes) {
      var idx = GL.log2ceilLookup[sizeBytes];
      var ibo = GL.currentContext.tempIndexBuffers[idx];
      if (ibo) {
        return ibo;
      }
      var prevIBO = GLctx.getParameter(GLctx.ELEMENT_ARRAY_BUFFER_BINDING);
      GL.currentContext.tempIndexBuffers[idx] = GLctx.createBuffer();
      GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, GL.currentContext.tempIndexBuffers[idx]);
      GLctx.bufferData(GLctx.ELEMENT_ARRAY_BUFFER, 1 << idx, GLctx.DYNAMIC_DRAW);
      GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, prevIBO);
      return GL.currentContext.tempIndexBuffers[idx];
    },

    // Called at start of each new WebGL rendering frame. This swaps the doublebuffered temp VB memory pointers,
    // so that every second frame utilizes different set of temp buffers. The aim is to keep the set of buffers
    // being rendered, and the set of buffers being updated disjoint.
    newRenderingFrameStarted: function newRenderingFrameStarted() {
      if (!GL.currentContext) {
        return;
      }
      var vb = GL.currentContext.tempVertexBuffers1;
      GL.currentContext.tempVertexBuffers1 = GL.currentContext.tempVertexBuffers2;
      GL.currentContext.tempVertexBuffers2 = vb;
      vb = GL.currentContext.tempVertexBufferCounters1;
      GL.currentContext.tempVertexBufferCounters1 = GL.currentContext.tempVertexBufferCounters2;
      GL.currentContext.tempVertexBufferCounters2 = vb;
      var largestIndex = GL.log2ceilLookup[GL.MAX_TEMP_BUFFER_SIZE];
      for (var i = 0; i <= largestIndex; ++i) {
        GL.currentContext.tempVertexBufferCounters1[i] = 0;
      }
    },
#endif

    getSource: function(shader, count, string, length) {
      var source = '';
      for (var i = 0; i < count; ++i) {
        var len = length ? {{{ makeGetValue('length', 'i*4', 'i32') }}} : -1;
        source += UTF8ToString({{{ makeGetValue('string', 'i*4', 'i32') }}}, len < 0 ? undefined : len);
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
            err("Shader attempts to use the standard derivatives extension which is not available.");
          }
#endif
        }
      }
#endif
      return source;
    },

#if GL_FFP_ONLY
    enabledClientAttribIndices: [],
    enableVertexAttribArray: function enableVertexAttribArray(index) {
      if (!GL.enabledClientAttribIndices[index]) {
        GL.enabledClientAttribIndices[index] = true;
        GLctx.enableVertexAttribArray(index);
      }
    },
    disableVertexAttribArray: function disableVertexAttribArray(index) {
      if (GL.enabledClientAttribIndices[index]) {
        GL.enabledClientAttribIndices[index] = false;
        GLctx.disableVertexAttribArray(index);
      }
    },
#endif

#if FULL_ES2
    calcBufLength: function calcBufLength(size, type, stride, count) {
      if (stride > 0) {
        return count * stride;  // XXXvlad this is not exactly correct I don't think
      }
      var typeSize = GL.byteSizeByType[type - GL.byteSizeByTypeRoot];
      return size * typeSize * count;
    },

    usedTempBuffers: [],

    preDrawHandleClientVertexAttribBindings: function preDrawHandleClientVertexAttribBindings(count) {
      GL.resetBufferBinding = false;

      // TODO: initial pass to detect ranges we need to upload, might not need an upload per attrib
      for (var i = 0; i < GL.currentContext.maxVertexAttribs; ++i) {
        var cb = GL.currentContext.clientBuffers[i];
        if (!cb.clientside || !cb.enabled) continue;

        GL.resetBufferBinding = true;

        var size = GL.calcBufLength(cb.size, cb.type, cb.stride, count);
        var buf = GL.getTempVertexBuffer(size);
        GLctx.bindBuffer(GLctx.ARRAY_BUFFER, buf);
        GLctx.bufferSubData(GLctx.ARRAY_BUFFER,
                                 0,
                                 HEAPU8.subarray(cb.ptr, cb.ptr + size));
#if GL_ASSERTIONS
        GL.validateVertexAttribPointer(cb.size, cb.type, cb.stride, 0);
#endif
        cb.vertexAttribPointerAdaptor.call(GLctx, i, cb.size, cb.type, cb.normalized, cb.stride, 0);
      }
    },

    postDrawHandleClientVertexAttribBindings: function postDrawHandleClientVertexAttribBindings() {
      if (GL.resetBufferBinding) {
        GLctx.bindBuffer(GLctx.ARRAY_BUFFER, GL.buffers[GL.currArrayBuffer]);
      }
    },
#endif

#if GL_ASSERTIONS
    validateGLObjectID: function(objectHandleArray, objectID, callerFunctionName, objectReadableType) {
      if (objectID != 0) {
        if (objectHandleArray[objectID] === null) {
          console.error(callerFunctionName + ' called with an already deleted ' + objectReadableType + ' ID ' + objectID + '!');
        } else if (!objectHandleArray[objectID]) {
          console.error(callerFunctionName + ' called with an invalid ' + objectReadableType + ' ID ' + objectID + '!');
        }
      }
    },
    // Validates that user obeys GL spec #6.4: http://www.khronos.org/registry/webgl/specs/latest/1.0/#6.4
    validateVertexAttribPointer: function(dimension, dataType, stride, offset) {
      var sizeBytes = 1;
      switch(dataType) {
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
#if USE_WEBGL2
          if (GL.currentContext.version >= 2 && (dataType == 0x8368 /* GL_UNSIGNED_INT_2_10_10_10_REV */ || dataType == 0x8D9F /* GL_INT_2_10_10_10_REV */)) {
            sizeBytes = 4;
            break;
          } else {
            // else fall through
          }
#endif
          console.error('Invalid vertex attribute data type GLenum ' + dataType + ' passed to GL function!');
      }
      if (dimension == 0x80E1 /* GL_BGRA */) {
        console.error('WebGL does not support size=GL_BGRA in a call to glVertexAttribPointer! Please use size=4 and type=GL_UNSIGNED_BYTE instead!');
      } else if (dimension < 1 || dimension > 4) {
        console.error('Invalid dimension='+dimension+' in call to glVertexAttribPointer, must be 1,2,3 or 4.');
      }
      if (stride < 0 || stride > 255) {
        console.error('Invalid stride='+stride+' in call to glVertexAttribPointer. Note that maximum supported stride in WebGL is 255!');
      }
      if (offset % sizeBytes != 0) {
        console.error('GL spec section 6.4 error: vertex attribute data offset of ' + offset + ' bytes should have been a multiple of the data type size that was used: GLenum ' + dataType + ' has size of ' + sizeBytes + ' bytes!');
      }
      if (stride % sizeBytes != 0) {
        console.error('GL spec section 6.4 error: vertex attribute data stride of ' + stride + ' bytes should have been a multiple of the data type size that was used: GLenum ' + dataType + ' has size of ' + sizeBytes + ' bytes!');
      }
    },
#endif

#if TRACE_WEBGL_CALLS
    webGLFunctionLengths: { 'getContextAttributes': 0, 'isContextLost': 0, 'getSupportedExtensions': 0, 'createBuffer': 0, 'createFramebuffer': 0, 'createProgram': 0, 'createRenderbuffer': 0, 'createTexture': 0, 'finish': 0, 'flush': 0, 'getError': 0, 'createVertexArray': 0, 'createQuery': 0, 'createSampler': 0, 'createTransformFeedback': 0, 'endTransformFeedback': 0, 'pauseTransformFeedback': 0, 'resumeTransformFeedback': 0, 'commit': 0,
      'getExtension': 1, 'activeTexture': 1, 'blendEquation': 1, 'checkFramebufferStatus': 1, 'clear': 1, 'clearDepth': 1, 'clearStencil': 1, 'compileShader': 1, 'createShader': 1, 'cullFace': 1, 'deleteBuffer': 1, 'deleteFramebuffer': 1, 'deleteProgram': 1, 'deleteRenderbuffer': 1, 'deleteShader': 1, 'deleteTexture': 1, 'depthFunc': 1, 'depthMask': 1, 'disable': 1, 'disableVertexAttribArray': 1, 'enable': 1, 'enableVertexAttribArray': 1, 'frontFace': 1, 'generateMipmap': 1, 'getAttachedShaders': 1, 'getParameter': 1, 'getProgramInfoLog': 1, 'getShaderInfoLog': 1, 'getShaderSource': 1, 'isBuffer': 1, 'isEnabled': 1, 'isFramebuffer': 1, 'isProgram': 1, 'isRenderbuffer': 1, 'isShader': 1, 'isTexture': 1, 'lineWidth': 1, 'linkProgram': 1, 'stencilMask': 1, 'useProgram': 1, 'validateProgram': 1, 'deleteQuery': 1, 'isQuery': 1, 'deleteVertexArray': 1, 'bindVertexArray': 1, 'isVertexArray': 1, 'drawBuffers': 1, 'readBuffer': 1, 'endQuery': 1, 'deleteSampler': 1, 'isSampler': 1, 'isSync': 1, 'deleteSync': 1, 'deleteTransformFeedback': 1, 'isTransformFeedback': 1, 'beginTransformFeedback': 1,
      'attachShader': 2, 'bindBuffer': 2, 'bindFramebuffer': 2, 'bindRenderbuffer': 2, 'bindTexture': 2, 'blendEquationSeparate': 2, 'blendFunc': 2, 'depthRange': 2, 'detachShader': 2, 'getActiveAttrib': 2, 'getActiveUniform': 2, 'getAttribLocation': 2, 'getBufferParameter': 2, 'getProgramParameter': 2, 'getRenderbufferParameter': 2, 'getShaderParameter': 2, 'getShaderPrecisionFormat': 2, 'getTexParameter': 2, 'getUniform': 2, 'getUniformLocation': 2, 'getVertexAttrib': 2, 'getVertexAttribOffset': 2, 'hint': 2, 'pixelStorei': 2, 'polygonOffset': 2, 'sampleCoverage': 2, 'shaderSource': 2, 'stencilMaskSeparate': 2, 'uniform1f': 2, 'uniform1fv': 2, 'uniform1i': 2, 'uniform1iv': 2, 'uniform2fv': 2, 'uniform2iv': 2, 'uniform3fv': 2, 'uniform3iv': 2, 'uniform4fv': 2, 'uniform4iv': 2, 'vertexAttrib1f': 2, 'vertexAttrib1fv': 2, 'vertexAttrib2fv': 2, 'vertexAttrib3fv': 2, 'vertexAttrib4fv': 2, 'vertexAttribDivisor': 2, 'beginQuery': 2, 'invalidateFramebuffer': 2, 'getFragDataLocation': 2, 'uniform1ui': 2, 'uniform1uiv': 2, 'uniform2uiv': 2, 'uniform3uiv': 2, 'uniform4uiv': 2, 'vertexAttribI4iv': 2, 'vertexAttribI4uiv': 2, 'getQuery': 2, 'getQueryParameter': 2, 'bindSampler': 2, 'getSamplerParameter': 2, 'fenceSync': 2, 'getSyncParameter': 2, 'bindTransformFeedback': 2, 'getTransformFeedbackVarying': 2, 'getIndexedParameter': 2, 'getUniformIndices': 2, 'getUniformBlockIndex': 2, 'getActiveUniformBlockName': 2,
      'bindAttribLocation': 3, 'bufferData': 3, 'bufferSubData': 3, 'drawArrays': 3, 'getFramebufferAttachmentParameter': 3, 'stencilFunc': 3, 'stencilOp': 3, 'texParameterf': 3, 'texParameteri': 3, 'uniform2f': 3, 'uniform2i': 3, 'uniformMatrix2fv': 3, 'uniformMatrix3fv': 3, 'uniformMatrix4fv': 3, 'vertexAttrib2f': 3, 'getBufferSubData': 3, 'getInternalformatParameter': 3, 'uniform2ui': 3, 'uniformMatrix2x3fv': 3, 'uniformMatrix3x2fv': 3, 'uniformMatrix2x4fv': 3, 'uniformMatrix4x2fv': 3, 'uniformMatrix3x4fv': 3, 'uniformMatrix4x3fv': 3, 'clearBufferiv': 3, 'clearBufferuiv': 3, 'clearBufferfv': 3, 'samplerParameteri': 3, 'samplerParameterf': 3, 'clientWaitSync': 3, 'waitSync': 3, 'transformFeedbackVaryings': 3, 'bindBufferBase': 3, 'getActiveUniforms': 3, 'getActiveUniformBlockParameter': 3, 'uniformBlockBinding': 3,
      'blendColor': 4, 'blendFuncSeparate': 4, 'clearColor': 4, 'colorMask': 4, 'drawElements': 4, 'framebufferRenderbuffer': 4, 'renderbufferStorage': 4, 'scissor': 4, 'stencilFuncSeparate': 4, 'stencilOpSeparate': 4, 'uniform3f': 4, 'uniform3i': 4, 'vertexAttrib3f': 4, 'viewport': 4, 'drawArraysInstanced': 4, 'uniform3ui': 4, 'clearBufferfi': 4,
      'framebufferTexture2D': 5, 'uniform4f': 5, 'uniform4i': 5, 'vertexAttrib4f': 5, 'drawElementsInstanced': 5, 'copyBufferSubData': 5, 'framebufferTextureLayer': 5, 'renderbufferStorageMultisample': 5, 'texStorage2D': 5, 'uniform4ui': 5, 'vertexAttribI4i': 5, 'vertexAttribI4ui': 5, 'vertexAttribIPointer': 5, 'bindBufferRange': 5,
      'texImage2D': 6, 'vertexAttribPointer': 6, 'invalidateSubFramebuffer': 6, 'texStorage3D': 6, 'drawRangeElements': 6,
      'compressedTexImage2D': 7, 'readPixels': 7, 'texSubImage2D': 7,
      'compressedTexSubImage2D': 8, 'copyTexImage2D': 8, 'copyTexSubImage2D': 8, 'compressedTexImage3D': 8,
      'copyTexSubImage3D': 9,
      'blitFramebuffer': 10, 'texImage3D': 10, 'compressedTexSubImage3D': 10,
      'texSubImage3D': 11 },

    hookWebGLFunction: function(f, glCtx) {
      var realf = 'real_' + f;
      glCtx[realf] = glCtx[f];
      var numArgs = GL.webGLFunctionLengths[f]; // On Firefox & Chrome, could do "glCtx[realf].length", but that doesn't work on Edge, which always reports 0.
      if (numArgs === undefined) throw 'Unexpected WebGL function ' + f;
      var contextHandle = glCtx.canvas.GLctxObject.handle;
      var threadId = (typeof _pthread_self !== 'undefined') ? _pthread_self : function() { return 1; };
      // Accessing 'arguments' is super slow, so to avoid overhead, statically reason the number of arguments.
      switch(numArgs) {
        case 0: glCtx[f] = function webgl_0() { var ret = glCtx[realf](); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '() -> ' + ret); return ret; }; break;
        case 1: glCtx[f] = function webgl_1(a1) { var ret = glCtx[realf](a1); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+') -> ' + ret); return ret; }; break;
        case 2: glCtx[f] = function webgl_2(a1, a2) { var ret = glCtx[realf](a1, a2); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +') -> ' + ret); return ret; }; break;
        case 3: glCtx[f] = function webgl_3(a1, a2, a3) { var ret = glCtx[realf](a1, a2, a3); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +') -> ' + ret); return ret; }; break;
        case 4: glCtx[f] = function webgl_4(a1, a2, a3, a4) { var ret = glCtx[realf](a1, a2, a3, a4); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +') -> ' + ret); return ret; }; break;
        case 5: glCtx[f] = function webgl_5(a1, a2, a3, a4, a5) { var ret = glCtx[realf](a1, a2, a3, a4, a5); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +') -> ' + ret); return ret; }; break;
        case 6: glCtx[f] = function webgl_6(a1, a2, a3, a4, a5, a6) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +') -> ' + ret); return ret; }; break;
        case 7: glCtx[f] = function webgl_7(a1, a2, a3, a4, a5, a6, a7) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6, a7); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +', ' + a7 +') -> ' + ret); return ret; }; break;
        case 8: glCtx[f] = function webgl_8(a1, a2, a3, a4, a5, a6, a7, a8) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +', ' + a7 +', ' + a8 +') -> ' + ret); return ret; }; break;
        case 9: glCtx[f] = function webgl_9(a1, a2, a3, a4, a5, a6, a7, a8, a9) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8, a9); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +', ' + a7 +', ' + a8 +', ' + a9 +') -> ' + ret); return ret; }; break;
        case 10: glCtx[f] = function webgl_10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +', ' + a7 +', ' + a8 +', ' + a9 +', ' + a10 +') -> ' + ret); return ret; }; break;
        case 11: glCtx[f] = function webgl_11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +', ' + a7 +', ' + a8 +', ' + a9 +', ' + a10 +', ' + a11 +') -> ' + ret); return ret; }; break;
        default: throw 'hookWebGL failed! Unexpected length ' + glCtx[realf].length;
      }
    },

    hookWebGL: function(glCtx) {
      if (!glCtx) glCtx = this.detectWebGLContext();
      if (!glCtx) return;
      if (!((typeof WebGLRenderingContext !== 'undefined' && glCtx instanceof WebGLRenderingContext)
       || (typeof WebGL2RenderingContext !== 'undefined' && glCtx instanceof WebGL2RenderingContext))) {
        return;
      }

      if (glCtx.webGlTracerAlreadyHooked) return;
      glCtx.webGlTracerAlreadyHooked = true;

      // Hot GL functions are ones that you'd expect to find during render loops (render calls, dynamic resource uploads), cold GL functions are load time functions (shader compilation, texture/mesh creation)
      // Distinguishing between these two allows pinpointing locations of troublesome GL usage that might cause performance issues.
      for(var f in glCtx) {
        if (typeof glCtx[f] !== 'function' || f.indexOf('real_') == 0) continue;
        this.hookWebGLFunction(f, glCtx);
      }
      // The above injection won't work for texImage2D and texSubImage2D, which have multiple overloads.
      glCtx['texImage2D'] = function(a1, a2, a3, a4, a5, a6, a7, a8, a9) {
        var ret = (a7 !== undefined) ? glCtx['real_texImage2D'](a1, a2, a3, a4, a5, a6, a7, a8, a9) : glCtx['real_texImage2D'](a1, a2, a3, a4, a5, a6);
        return ret;
      };
      glCtx['texSubImage2D'] = function(a1, a2, a3, a4, a5, a6, a7, a8, a9) {
        var ret = (a8 !== undefined) ? glCtx['real_texSubImage2D'](a1, a2, a3, a4, a5, a6, a7, a8, a9) : glCtx['real_texSubImage2D'](a1, a2, a3, a4, a5, a6, a7);
        return ret;
      };
      glCtx['texSubImage3D'] = function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) {
        var ret = (a9 !== undefined) ? glCtx['real_texSubImage3D'](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) : glCtx['real_texSubImage2D'](a1, a2, a3, a4, a5, a6, a7, a8);
        return ret;
      };
    },
#endif
    // Returns the context handle to the new context.
    createContext: function(canvas, webGLContextAttributes) {
#if OFFSCREEN_FRAMEBUFFER
      // In proxied operation mode, rAF()/setTimeout() functions do not delimit frame boundaries, so can't have WebGL implementation
      // try to detect when it's ok to discard contents of the rendered backbuffer.
      if (webGLContextAttributes.renderViaOffscreenBackBuffer) webGLContextAttributes['preserveDrawingBuffer'] = true;
#endif

#if GL_TESTING
      webGLContextAttributes['preserveDrawingBuffer'] = true;
#endif

#if GL_DEBUG
      var errorInfo = '?';
      function onContextCreationError(event) {
        errorInfo = event.statusMessage || errorInfo;
      }
      canvas.addEventListener('webglcontextcreationerror', onContextCreationError, false);
#endif

#if GL_PREINITIALIZED_CONTEXT
      // If WebGL context has already been preinitialized for the page on the JS side, reuse that context instead. This is useful for example when
      // the main page precompiles shaders for the application, in which case the WebGL context is created already before any Emscripten compiled
      // code has been downloaded.
      if (Module['preinitializedWebGLContext']) {
        var ctx = Module['preinitializedWebGLContext'];
#if USE_WEBGL2
        webGLContextAttributes.majorVersion = (typeof WebGL2RenderingContext !== 'undefined' && ctx instanceof WebGL2RenderingContext) ? 2 : 1;
#else
        webGLContextAttributes.majorVersion = 1;
#endif
      } else {
#endif

      var ctx = 
#if USE_WEBGL2
        (webGLContextAttributes.majorVersion > 1) ? canvas.getContext("webgl2", webGLContextAttributes) :
#endif
        (canvas.getContext("webgl", webGLContextAttributes) || canvas.getContext("experimental-webgl", webGLContextAttributes));

#if GL_PREINITIALIZED_CONTEXT
      }
#endif

#if GL_DEBUG
      canvas.removeEventListener('webglcontextcreationerror', onContextCreationError, false);
      if (!ctx) {
        err('Could not create canvas: ' + [errorInfo, JSON.stringify(webGLContextAttributes)]);
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
      function disableHalfFloatExtensionIfBroken(ctx) {
        var t = ctx.createTexture();
        ctx.bindTexture(0x0de1/*GL_TEXTURE_2D*/, t);
        for (var i = 0; i < 8 && ctx.getError(); ++i) /*no-op*/;
        var ext = ctx.getExtension('OES_texture_half_float');
        if (!ext) return; // no half-float extension - nothing needed to fix.
        // Bug on Safari on iOS and macOS: texImage2D() and texSubImage2D() do not allow uploading pixel data to half float textures,
        // rendering them useless.
        // See https://bugs.webkit.org/show_bug.cgi?id=183321, https://bugs.webkit.org/show_bug.cgi?id=169999,
        // https://stackoverflow.com/questions/54248633/cannot-create-half-float-oes-texture-from-uint16array-on-ipad
        ctx.texImage2D(0x0de1/*GL_TEXTURE_2D*/, 0, 0x1908/*GL_RGBA*/, 1, 1, 0, 0x1908/*GL_RGBA*/, 0x8d61/*HALF_FLOAT_OES*/, new Uint16Array(4));
        var broken = ctx.getError();
        ctx.bindTexture(0x0de1/*GL_TEXTURE_2D*/, null);
        ctx.deleteTexture(t);
        if (broken) {
          ctx.realGetSupportedExtensions = ctx.getSupportedExtensions;
          ctx.getSupportedExtensions = function() {
#if GL_ASSERTIONS
            warnOnce('Removed broken support for half-float textures. See e.g. https://bugs.webkit.org/show_bug.cgi?id=183321');
#endif
            // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
            return (this.realGetSupportedExtensions() || []).filter(function(ext) {
              return ext.indexOf('texture_half_float') == -1;
            });
          }
        }
      }
      disableHalfFloatExtensionIfBroken(ctx);
#endif

      return handle;
    },

#if OFFSCREEN_FRAMEBUFFER
    enableOffscreenFramebufferAttributes: function(webGLContextAttributes) {
      webGLContextAttributes.renderViaOffscreenBackBuffer = true;
      webGLContextAttributes.preserveDrawingBuffer = true;
    },

    // If WebGL is being proxied from a pthread to the main thread, we can't directly render to the WebGL default back buffer
    // because of WebGL's implicit swap behavior. Therefore in such modes, create an offscreen render target surface to
    // which rendering is performed to, and finally flipped to the main screen.
    createOffscreenFramebuffer: function(context) {
      var gl = context.GLctx;

      // Create FBO
      var fbo = gl.createFramebuffer();
      gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
      context.defaultFbo = fbo;

#if USE_WEBGL2
      context.defaultFboForbidBlitFramebuffer = false;
      if (gl.getContextAttributes().antialias) {
        context.defaultFboForbidBlitFramebuffer = true;
      } else {
        // The WebGL 2 blit path doesn't work in Firefox < 67 (except in fullscreen).
        // https://bugzilla.mozilla.org/show_bug.cgi?id=1523030
        // TODO: Remove this workaround once Firefox 67 is obsolete.
        var firefoxMatch = navigator.userAgent.toLowerCase().match(/firefox\/(\d\d)/);
        if (firefoxMatch != null) {
          var firefoxVersion = firefoxMatch[1];
          context.defaultFboForbidBlitFramebuffer = firefoxVersion < 67;
        }
      }
#endif

      // Create render targets to the FBO
      context.defaultColorTarget = gl.createTexture();
      context.defaultDepthTarget = gl.createRenderbuffer();
      GL.resizeOffscreenFramebuffer(context); // Size them up correctly (use the same mechanism when resizing on demand)

      gl.bindTexture(gl.TEXTURE_2D, context.defaultColorTarget);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
      gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.canvas.width, gl.canvas.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
      gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, context.defaultColorTarget, 0);
      gl.bindTexture(gl.TEXTURE_2D, null);

      // Create depth render target to the FBO
      var depthTarget = gl.createRenderbuffer();
      gl.bindRenderbuffer(gl.RENDERBUFFER, context.defaultDepthTarget);
      gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, gl.canvas.width, gl.canvas.height);
      gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, context.defaultDepthTarget);
      gl.bindRenderbuffer(gl.RENDERBUFFER, null);

      // Create blitter
      var vertices = [
        -1, -1,
        -1,  1,
         1, -1,
         1,  1
      ];
      var vb = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, vb);
      gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
      gl.bindBuffer(gl.ARRAY_BUFFER, null);
      context.blitVB = vb;

      var vsCode =
        'attribute vec2 pos;' +
        'varying lowp vec2 tex;' +
        'void main() { tex = pos * 0.5 + vec2(0.5,0.5); gl_Position = vec4(pos, 0.0, 1.0); }';
      var vs = gl.createShader(gl.VERTEX_SHADER);
      gl.shaderSource(vs, vsCode);
      gl.compileShader(vs);

      var fsCode =
        'varying lowp vec2 tex;' +
        'uniform sampler2D sampler;' +
        'void main() { gl_FragColor = texture2D(sampler, tex); }';
      var fs = gl.createShader(gl.FRAGMENT_SHADER);
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

    resizeOffscreenFramebuffer: function(context) {
      var gl = context.GLctx;

      // Resize color buffer
      if (context.defaultColorTarget) {
        var prevTextureBinding = gl.getParameter(gl.TEXTURE_BINDING_2D);
        gl.bindTexture(gl.TEXTURE_2D, context.defaultColorTarget);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.drawingBufferWidth, gl.drawingBufferHeight, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
        gl.bindTexture(gl.TEXTURE_2D, prevTextureBinding);
      }

      // Resize depth buffer
      if (context.defaultDepthTarget) {
        var prevRenderBufferBinding = gl.getParameter(gl.RENDERBUFFER_BINDING);
        gl.bindRenderbuffer(gl.RENDERBUFFER, context.defaultDepthTarget);
        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, gl.drawingBufferWidth, gl.drawingBufferHeight); // TODO: Read context creation parameters for what type of depth and stencil to use
        gl.bindRenderbuffer(gl.RENDERBUFFER, prevRenderBufferBinding);
      }
    },

    // Renders the contents of the offscreen render target onto the visible screen.
    blitOffscreenFramebuffer: function(context) {
      var gl = context.GLctx;

      var prevScissorTest = gl.getParameter(gl.SCISSOR_TEST);
      if (prevScissorTest) gl.disable(gl.SCISSOR_TEST);

      var prevFbo = gl.getParameter(gl.FRAMEBUFFER_BINDING);

#if USE_WEBGL2
      if (gl.blitFramebuffer && !context.defaultFboForbidBlitFramebuffer) {
        gl.bindFramebuffer(gl.READ_FRAMEBUFFER, context.defaultFbo);
        gl.bindFramebuffer(gl.DRAW_FRAMEBUFFER, null);
        gl.blitFramebuffer(0, 0, gl.canvas.width, gl.canvas.height,
                           0, 0, gl.canvas.width, gl.canvas.height,
                           gl.COLOR_BUFFER_BIT, gl.NEAREST);
      }
      else
#endif
      {
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);

        var prevProgram = gl.getParameter(gl.CURRENT_PROGRAM);
        gl.useProgram(context.blitProgram);

        var prevVB = gl.getParameter(gl.ARRAY_BUFFER_BINDING);
        gl.bindBuffer(gl.ARRAY_BUFFER, context.blitVB);

        var prevActiveTexture = gl.getParameter(gl.ACTIVE_TEXTURE);
        gl.activeTexture(gl.TEXTURE0);

        var prevTextureBinding = gl.getParameter(gl.TEXTURE_BINDING_2D);
        gl.bindTexture(gl.TEXTURE_2D, context.defaultColorTarget);

        var prevBlend = gl.getParameter(gl.BLEND);
        if (prevBlend) gl.disable(gl.BLEND);

        var prevCullFace = gl.getParameter(gl.CULL_FACE);
        if (prevCullFace) gl.disable(gl.CULL_FACE);

        var prevDepthTest = gl.getParameter(gl.DEPTH_TEST);
        if (prevDepthTest) gl.disable(gl.DEPTH_TEST);

        var prevStencilTest = gl.getParameter(gl.STENCIL_TEST);
        if (prevStencilTest) gl.disable(gl.STENCIL_TEST);

        function draw() {
          gl.vertexAttribPointer(context.blitPosLoc, 2, gl.FLOAT, false, 0, 0);
          gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        }

#if !OFFSCREEN_FRAMEBUFFER_FORBID_VAO_PATH
        if (context.defaultVao) {
          // WebGL 2 or OES_vertex_array_object
          var prevVAO = gl.getParameter(0x85B5); // GL_VERTEX_ARRAY_BINDING
          gl.bindVertexArray(context.defaultVao);
          draw();
          gl.bindVertexArray(prevVAO);
        }
        else
#endif
        {
          var prevVertexAttribPointer = {
            buffer: gl.getVertexAttrib(context.blitPosLoc, gl.VERTEX_ATTRIB_ARRAY_BUFFER_BINDING),
            size: gl.getVertexAttrib(context.blitPosLoc, gl.VERTEX_ATTRIB_ARRAY_SIZE),
            stride: gl.getVertexAttrib(context.blitPosLoc, gl.VERTEX_ATTRIB_ARRAY_STRIDE),
            type: gl.getVertexAttrib(context.blitPosLoc, gl.VERTEX_ATTRIB_ARRAY_TYPE),
            normalized: gl.getVertexAttrib(context.blitPosLoc, gl.VERTEX_ATTRIB_ARRAY_NORMALIZED),
            pointer: gl.getVertexAttribOffset(context.blitPosLoc, gl.VERTEX_ATTRIB_ARRAY_POINTER),
          };
          var maxVertexAttribs = gl.getParameter(gl.MAX_VERTEX_ATTRIBS);
          var prevVertexAttribEnables = [];
          for (var i = 0; i < maxVertexAttribs; ++i) {
            var prevEnabled = gl.getVertexAttrib(i, gl.VERTEX_ATTRIB_ARRAY_ENABLED);
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
          gl.bindBuffer(gl.ARRAY_BUFFER, prevVertexAttribPointer.buffer);
          gl.vertexAttribPointer(context.blitPosLoc,
                                 prevVertexAttribPointer.size,
                                 prevVertexAttribPointer.type,
                                 prevVertexAttribPointer.normalized,
                                 prevVertexAttribPointer.stride,
                                 prevVertexAttribPointer.offset);
        }

        if (prevStencilTest) gl.enable(gl.STENCIL_TEST);
        if (prevDepthTest) gl.enable(gl.DEPTH_TEST);
        if (prevCullFace) gl.enable(gl.CULL_FACE);
        if (prevBlend) gl.enable(gl.BLEND);

        gl.bindTexture(gl.TEXTURE_2D, prevTextureBinding);
        gl.activeTexture(prevActiveTexture);
        gl.bindBuffer(gl.ARRAY_BUFFER, prevVB);
        gl.useProgram(prevProgram);
      }
      gl.bindFramebuffer(gl.FRAMEBUFFER, prevFbo);
      if (prevScissorTest) gl.enable(gl.SCISSOR_TEST);
    },
#endif

    registerContext: function(ctx, webGLContextAttributes) {
      var handle = _malloc(8); // Make space on the heap to store GL context attributes that need to be accessible as shared between threads.
#if GL_ASSERTIONS
      assert(handle, 'malloc() failed in GL.registerContext!');
#endif
#if GL_SUPPORT_EXPLICIT_SWAP_CONTROL
      {{{ makeSetValue('handle', 0, 'webGLContextAttributes.explicitSwapControl', 'i32')}}}; // explicitSwapControl
#endif
#if USE_PTHREADS
      {{{ makeSetValue('handle', 4, '_pthread_self()', 'i32')}}}; // the thread pointer of the thread that owns the control of the context
#endif
      var context = {
        handle: handle,
        attributes: webGLContextAttributes,
        version: webGLContextAttributes.majorVersion,
        GLctx: ctx
      };

#if USE_WEBGL2
      // BUG: Workaround Chrome WebGL 2 issue: the first shipped versions of WebGL 2 in Chrome did not actually implement the new WebGL 2 functions.
      //      Those are supported only in Chrome 58 and newer.
      function getChromeVersion() {
        var raw = navigator.userAgent.match(/Chrom(e|ium)\/([0-9]+)\./);
        return raw ? parseInt(raw[2], 10) : false;
      }
      context.supportsWebGL2EntryPoints = (context.version >= 2) && (getChromeVersion() === false || getChromeVersion() >= 58);
#endif

#if WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG
      context.cannotHandleOffsetsInUniformArrayViews = (function(g) {
        function b(c, t) {
          var s = g.createShader(t);
          g.shaderSource(s, c);
          g.compileShader(s);
          return s;
        }
        try {
          var p = g.createProgram(); // Note: we do not delete this program so it stays part of the context we created, but that is ok - it does not do anything and we want to keep this detection size minimal.
          g.attachShader(p, b("attribute vec4 p;void main(){gl_Position=p;}", g.VERTEX_SHADER));
          g.attachShader(p, b("precision lowp float;uniform vec4 u;void main(){gl_FragColor=u;}", g.FRAGMENT_SHADER));
          g.linkProgram(p);
          var h = new Float32Array(8);
          h[4] = 1;
          g.useProgram(p);
          var l = g.getUniformLocation(p, "u");
          g.uniform4fv(l, h.subarray(4, 8)); // Uploading a 4-vector GL uniform from last four elements of array [0,0,0,0,1,0,0,0], i.e. uploading vec4=(1,0,0,0) at offset=4.
          return !g.getUniform(p, l)[0]; // in proper WebGL we expect to read back the vector we just uploaded: (1,0,0,0). On buggy browser would instead have uploaded offset=0 of above array, i.e. vec4=(0,0,0,0)
        } catch(e) { return false; } // If we get an exception, we assume we got some other error, and do not trigger this workaround.
      })();
#endif

      // Store the created context object so that we can access the context given a canvas without having to pass the parameters again.
      if (ctx.canvas) ctx.canvas.GLctxObject = context;
      GL.contexts[handle] = context;
#if GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS
      if (typeof webGLContextAttributes.enableExtensionsByDefault === 'undefined' || webGLContextAttributes.enableExtensionsByDefault) {
        GL.initExtensions(context);
      }
#endif

#if FULL_ES2
      context.maxVertexAttribs = context.GLctx.getParameter(context.GLctx.MAX_VERTEX_ATTRIBS);
      context.clientBuffers = [];
      for (var i = 0; i < context.maxVertexAttribs; i++) {
        context.clientBuffers[i] = { enabled: false, clientside: false, size: 0, type: 0, normalized: 0, stride: 0, ptr: 0, vertexAttribPointerAdaptor: null };
      }

      GL.generateTempBuffers(false, context);
#endif

#if OFFSCREEN_FRAMEBUFFER
      if (webGLContextAttributes.renderViaOffscreenBackBuffer) GL.createOffscreenFramebuffer(context);
#else

#if GL_DEBUG
      if (webGLContextAttributes.renderViaOffscreenBackBuffer) err('renderViaOffscreenBackBuffer=true specified in WebGL context creation attributes, pass linker flag -s OFFSCREEN_FRAMEBUFFER=1 to enable support!');
#endif

#endif
      return handle;
    },

    makeContextCurrent: function(contextHandle) {
#if GL_DEBUG
      if (contextHandle && !GL.contexts[contextHandle]) {
#if USE_PTHREADS
        console.error('GL.makeContextCurrent() failed! WebGL context ' + contextHandle + ' does not exist, or was created on another thread!');
#else
        console.error('GL.makeContextCurrent() failed! WebGL context ' + contextHandle + ' does not exist!');
#endif
      }
#endif

      GL.currentContext = GL.contexts[contextHandle]; // Active Emscripten GL layer context object.
      Module.ctx = GLctx = GL.currentContext && GL.currentContext.GLctx; // Active WebGL context object.
      return !(contextHandle && !GLctx);
    },

    getContext: function(contextHandle) {
      return GL.contexts[contextHandle];
    },

    deleteContext: function(contextHandle) {
      if (GL.currentContext === GL.contexts[contextHandle]) GL.currentContext = null;
      if (typeof JSEvents === 'object') JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas); // Release all JS event handlers on the DOM element that the GL context is associated with since the context is now deleted.
      if (GL.contexts[contextHandle] && GL.contexts[contextHandle].GLctx.canvas) GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined; // Make sure the canvas object no longer refers to the context object so there are no GC surprises.
      _free(GL.contexts[contextHandle]);
      GL.contexts[contextHandle] = null;
    },

    acquireInstancedArraysExtension: function(ctx) {
      // Extension available in WebGL 1 from Firefox 26 and Google Chrome 30 onwards. Core feature in WebGL 2.
      var ext = ctx.getExtension('ANGLE_instanced_arrays');
      if (ext) {
        ctx['vertexAttribDivisor'] = function(index, divisor) { ext['vertexAttribDivisorANGLE'](index, divisor); };
        ctx['drawArraysInstanced'] = function(mode, first, count, primcount) { ext['drawArraysInstancedANGLE'](mode, first, count, primcount); };
        ctx['drawElementsInstanced'] = function(mode, count, type, indices, primcount) { ext['drawElementsInstancedANGLE'](mode, count, type, indices, primcount); };
      }
    },

    acquireVertexArrayObjectExtension: function(ctx) {
      // Extension available in WebGL 1 from Firefox 25 and WebKit 536.28/desktop Safari 6.0.3 onwards. Core feature in WebGL 2.
      var ext = ctx.getExtension('OES_vertex_array_object');
      if (ext) {
        ctx['createVertexArray'] = function() { return ext['createVertexArrayOES'](); };
        ctx['deleteVertexArray'] = function(vao) { ext['deleteVertexArrayOES'](vao); };
        ctx['bindVertexArray'] = function(vao) { ext['bindVertexArrayOES'](vao); };
        ctx['isVertexArray'] = function(vao) { return ext['isVertexArrayOES'](vao); };
      }
    },

    acquireDrawBuffersExtension: function(ctx) {
      // Extension available in WebGL 1 from Firefox 28 onwards. Core feature in WebGL 2.
      var ext = ctx.getExtension('WEBGL_draw_buffers');
      if (ext) {
        ctx['drawBuffers'] = function(n, bufs) { ext['drawBuffersWEBGL'](n, bufs); };
      }
    },

    // In WebGL, extensions must be explicitly enabled to be active, see http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.14.14
    // In GLES2, all extensions are enabled by default without additional operations. Init all extensions we need to give to GLES2 user
    // code here, so that GLES2 code can operate without changing behavior.
    initExtensions: function(context) {
      // If this function is called without a specific context object, init the extensions of the currently active context.
      if (!context) context = GL.currentContext;

      if (context.initExtensionsDone) return;
      context.initExtensionsDone = true;

      var GLctx = context.GLctx;

      // Detect the presence of a few extensions manually, this GL interop layer itself will need to know if they exist.
#if LEGACY_GL_EMULATION
      context.compressionExt = GLctx.getExtension('WEBGL_compressed_texture_s3tc');
      context.anisotropicExt = GLctx.getExtension('EXT_texture_filter_anisotropic');
#endif

      if (context.version < 2) {
        GL.acquireInstancedArraysExtension(GLctx);
        GL.acquireVertexArrayObjectExtension(GLctx);
        GL.acquireDrawBuffersExtension(GLctx);
      }

      GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query");

      // These are the 'safe' feature-enabling extensions that don't add any performance impact related to e.g. debugging, and
      // should be enabled by default so that client GLES2/GL code will not need to go through extra hoops to get its stuff working.
      // As new extensions are ratified at http://www.khronos.org/registry/webgl/extensions/ , feel free to add your new extensions
      // here, as long as they don't produce a performance impact for users that might not be using those extensions.
      // E.g. debugging-related extensions should probably be off by default.
      var automaticallyEnabledExtensions = [ // Khronos ratified WebGL extensions ordered by number (no debug extensions):
                                             "OES_texture_float", "OES_texture_half_float", "OES_standard_derivatives",
                                             "OES_vertex_array_object", "WEBGL_compressed_texture_s3tc", "WEBGL_depth_texture",
                                             "OES_element_index_uint", "EXT_texture_filter_anisotropic", "EXT_frag_depth",
                                             "WEBGL_draw_buffers", "ANGLE_instanced_arrays", "OES_texture_float_linear",
                                             "OES_texture_half_float_linear", "EXT_blend_minmax", "EXT_shader_texture_lod",
                                             // Community approved WebGL extensions ordered by number:
                                             "WEBGL_compressed_texture_pvrtc", "EXT_color_buffer_half_float", "WEBGL_color_buffer_float",
                                             "EXT_sRGB", "WEBGL_compressed_texture_etc1", "EXT_disjoint_timer_query",
                                             "WEBGL_compressed_texture_etc", "WEBGL_compressed_texture_astc", "EXT_color_buffer_float",
                                             "WEBGL_compressed_texture_s3tc_srgb", "EXT_disjoint_timer_query_webgl2"];

      function shouldEnableAutomatically(extension) {
        var ret = false;
        automaticallyEnabledExtensions.forEach(function(include) {
          if (extension.indexOf(include) != -1) {
            ret = true;
          }
        });
        return ret;
      }

      var exts = GLctx.getSupportedExtensions() || []; // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
      exts.forEach(function(ext) {
        if (automaticallyEnabledExtensions.indexOf(ext) != -1) {
          GLctx.getExtension(ext); // Calling .getExtension enables that extension permanently, no need to store the return value to be enabled.
        }
      });
    },

    // In WebGL, uniforms in a shader program are accessed through an opaque object type 'WebGLUniformLocation'.
    // In GLES2, uniforms are accessed via indices. Therefore we must generate a mapping of indices -> WebGLUniformLocations
    // to provide the client code the API that uses indices.
    // This function takes a linked GL program and generates a mapping table for the program.
    // NOTE: Populating the uniform table is performed eagerly at glLinkProgram time, so glLinkProgram should be considered
    //       to be a slow/costly function call. Calling glGetUniformLocation is relatively fast, since it is always a read-only
    //       lookup to the table populated in this function call.
    populateUniformTable: function(program) {
#if GL_ASSERTIONS
      GL.validateGLObjectID(GL.programs, program, 'populateUniformTable', 'program');
#endif
      var p = GL.programs[program];
      var ptable = GL.programInfos[program] = {
        uniforms: {},
        maxUniformLength: 0, // This is eagerly computed below, since we already enumerate all uniforms anyway.
        maxAttributeLength: -1, // This is lazily computed and cached, computed when/if first asked, "-1" meaning not computed yet.
        maxUniformBlockNameLength: -1 // Lazily computed as well
      };

      var utable = ptable.uniforms;
      // A program's uniform table maps the string name of an uniform to an integer location of that uniform.
      // The global GL.uniforms map maps integer locations to WebGLUniformLocations.
      var numUniforms = GLctx.getProgramParameter(p, 0x8B86/*GL_ACTIVE_UNIFORMS*/);
      for (var i = 0; i < numUniforms; ++i) {
        var u = GLctx.getActiveUniform(p, i);

        var name = u.name;
        ptable.maxUniformLength = Math.max(ptable.maxUniformLength, name.length+1);

        // If we are dealing with an array, e.g. vec4 foo[3], strip off the array index part to canonicalize that "foo", "foo[]",
        // and "foo[0]" will mean the same. Loop below will populate foo[1] and foo[2].
        if (name.slice(-1) == ']') {
          name = name.slice(0, name.lastIndexOf('['));
        }

        // Optimize memory usage slightly: If we have an array of uniforms, e.g. 'vec3 colors[3];', then
        // only store the string 'colors' in utable, and 'colors[0]', 'colors[1]' and 'colors[2]' will be parsed as 'colors'+i.
        // Note that for the GL.uniforms table, we still need to fetch the all WebGLUniformLocations for all the indices.
        var loc = GLctx.getUniformLocation(p, name);
        if (loc) {
          var id = GL.getNewId(GL.uniforms);
          utable[name] = [u.size, id];
          GL.uniforms[id] = loc;

          for (var j = 1; j < u.size; ++j) {
            var n = name + '['+j+']';
            loc = GLctx.getUniformLocation(p, n);
            id = GL.getNewId(GL.uniforms);

            GL.uniforms[id] = loc;
          }
        }
      }
    }
  },

  glPixelStorei__sig: 'vii',
  glPixelStorei: function(pname, param) {
    if (pname == 0x0cf5 /* GL_UNPACK_ALIGNMENT */) {
      GL.unpackAlignment = param;
    }
    GLctx.pixelStorei(pname, param);
  },

  glGetString__sig: 'ii',
  glGetString__deps: ['$stringToNewUTF8'],
  glGetString: function(name_) {
    if (GL.stringCache[name_]) return GL.stringCache[name_];
    var ret;
    switch(name_) {
      case 0x1F03 /* GL_EXTENSIONS */:
        var exts = GLctx.getSupportedExtensions() || []; // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
#if GL_EXTENSIONS_IN_PREFIXED_FORMAT
        exts = exts.concat(exts.map(function(e) { return "GL_" + e; }));
#endif
        ret = stringToNewUTF8(exts.join(' '));
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
        if (!s) {
          GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
          err('GL_INVALID_ENUM in glGetString: Received empty parameter for query name ' + name_ + '!'); // This occurs e.g. if one attempts GL_UNMASKED_VENDOR_WEBGL when it is not supported.
#endif
        }
        ret = stringToNewUTF8(s);
        break;

#if GL_EMULATE_GLES_VERSION_STRING_FORMAT
      case 0x1F02 /* GL_VERSION */:
        var glVersion = GLctx.getParameter(GLctx.VERSION);
        // return GLES version string corresponding to the version of the WebGL context
#if USE_WEBGL2
        if (GL.currentContext.version >= 2) glVersion = 'OpenGL ES 3.0 (' + glVersion + ')';
        else
#endif
        {
          glVersion = 'OpenGL ES 2.0 (' + glVersion + ')';
        }
        ret = stringToNewUTF8(glVersion);
        break;
      case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
        var glslVersion = GLctx.getParameter(GLctx.SHADING_LANGUAGE_VERSION);
        // extract the version number 'N.M' from the string 'WebGL GLSL ES N.M ...'
        var ver_re = /^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;
        var ver_num = glslVersion.match(ver_re);
        if (ver_num !== null) {
          if (ver_num[1].length == 3) ver_num[1] = ver_num[1] + '0'; // ensure minor version has 2 digits
          glslVersion = 'OpenGL ES GLSL ES ' + ver_num[1] + ' (' + glslVersion + ')';
        }
        ret = stringToNewUTF8(glslVersion);
        break;
#endif
      default:
        GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
        err('GL_INVALID_ENUM in glGetString: Unknown parameter ' + name_ + '!');
#endif
        return 0;
    }
    GL.stringCache[name_] = ret;
    return ret;
  },

  $emscriptenWebGLGet: function(name_, p, type) {
    // Guard against user passing a null pointer.
    // Note that GLES2 spec does not say anything about how passing a null pointer should be treated.
    // Testing on desktop core GL 3, the application crashes on glGetIntegerv to a null pointer, but
    // better to report an error instead of doing anything random.
    if (!p) {
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGet' + type + 'v(name=' + name_ + ': Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    var ret = undefined;
    switch(name_) { // Handle a few trivial GLES values
      case 0x8DFA: // GL_SHADER_COMPILER
        ret = 1;
        break;
      case 0x8DF8: // GL_SHADER_BINARY_FORMATS
#if GL_TRACK_ERRORS
        if (type != {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}} && type != {{{ cDefine('EM_FUNC_SIG_PARAM_I64') }}}) {
          GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
          err('GL_INVALID_ENUM in glGet' + type + 'v(GL_SHADER_BINARY_FORMATS): Invalid parameter type!');
#endif
        }
#endif
        return; // Do not write anything to the out pointer, since no binary formats are supported.
#if USE_WEBGL2
      case 0x87FE: // GL_NUM_PROGRAM_BINARY_FORMATS
#endif
      case 0x8DF9: // GL_NUM_SHADER_BINARY_FORMATS
        ret = 0;
        break;
      case 0x86A2: // GL_NUM_COMPRESSED_TEXTURE_FORMATS
        // WebGL doesn't have GL_NUM_COMPRESSED_TEXTURE_FORMATS (it's obsolete since GL_COMPRESSED_TEXTURE_FORMATS returns a JS array that can be queried for length),
        // so implement it ourselves to allow C++ GLES2 code get the length.
        var formats = GLctx.getParameter(0x86A3 /*GL_COMPRESSED_TEXTURE_FORMATS*/);
        ret = formats ? formats.length : 0;
        break;
#if USE_WEBGL2
      case 0x821D: // GL_NUM_EXTENSIONS
#if GL_TRACK_ERRORS
        if (GL.currentContext.version < 2) {
          GL.recordError(0x0502 /* GL_INVALID_OPERATION */); // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
          return;
        }
#endif
        // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
        var exts = GLctx.getSupportedExtensions() || [];
#if GL_EXTENSIONS_IN_PREFIXED_FORMAT
        ret = 2 * exts.length; // each extension is duplicated, first in unprefixed WebGL form, and then a second time with "GL_" prefix.
#else
        ret = exts.length;
#endif
        break;
      case 0x821B: // GL_MAJOR_VERSION
      case 0x821C: // GL_MINOR_VERSION
#if GL_TRACK_ERRORS
        if (GL.currentContext.version < 2) {
          GL.recordError(0x0500); // GL_INVALID_ENUM
          return;
        }
#endif
        ret = name_ == 0x821B ? 3 : 0; // return version 3.0
        break;
#endif
    }

    if (ret === undefined) {
      var result = GLctx.getParameter(name_);
      switch (typeof(result)) {
        case "number":
          ret = result;
          break;
        case "boolean":
          ret = result ? 1 : 0;
          break;
        case "string":
          GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
          err('GL_INVALID_ENUM in glGet' + type + 'v(' + name_ + ') on a name which returns a string!');
#endif
          return;
        case "object":
          if (result === null) {
            // null is a valid result for some (e.g., which buffer is bound - perhaps nothing is bound), but otherwise
            // can mean an invalid name_, which we need to report as an error
            switch(name_) {
              case 0x8894: // ARRAY_BUFFER_BINDING
              case 0x8B8D: // CURRENT_PROGRAM
              case 0x8895: // ELEMENT_ARRAY_BUFFER_BINDING
              case 0x8CA6: // FRAMEBUFFER_BINDING
              case 0x8CA7: // RENDERBUFFER_BINDING
              case 0x8069: // TEXTURE_BINDING_2D
              case 0x85B5: // WebGL 2 GL_VERTEX_ARRAY_BINDING, or WebGL 1 extension OES_vertex_array_object GL_VERTEX_ARRAY_BINDING_OES
#if USE_WEBGL2
              case 0x8919: // GL_SAMPLER_BINDING
              case 0x8E25: // GL_TRANSFORM_FEEDBACK_BINDING
#endif
              case 0x8514: { // TEXTURE_BINDING_CUBE_MAP
                ret = 0;
                break;
              }
              default: {
                GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
                err('GL_INVALID_ENUM in glGet' + type + 'v(' + name_ + ') and it returns null!');
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
                case {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}}: {{{ makeSetValue('p', 'i*4', 'result[i]', 'i32') }}}; break;
                case {{{ cDefine('EM_FUNC_SIG_PARAM_F') }}}: {{{ makeSetValue('p', 'i*4', 'result[i]', 'float') }}}; break;
                case {{{ cDefine('EM_FUNC_SIG_PARAM_B') }}}: {{{ makeSetValue('p', 'i',   'result[i] ? 1 : 0', 'i8') }}}; break;
#if GL_ASSERTIONS
                default: throw 'internal glGet error, bad type: ' + type;
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
              GL.recordError(0x0500); // GL_INVALID_ENUM
              err('GL_INVALID_ENUM in glGet' + type + 'v: Unknown object returned from WebGL getParameter(' + name_ + ')! (error: ' + e + ')');
              return;
            }
#endif
          }
          break;
#if GL_TRACK_ERRORS
        default:
          GL.recordError(0x0500); // GL_INVALID_ENUM
          err('GL_INVALID_ENUM in glGet' + type + 'v: Native code calling glGet' + type + 'v(' + name_ + ') and it returns ' + result + ' of type ' + typeof(result) + '!');
          return;
#endif
      }
    }

    switch (type) {
      case {{{ cDefine('EM_FUNC_SIG_PARAM_I64') }}}: {{{ makeSetValue('p', '0', 'ret', 'i64') }}};    break;
      case {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}}: {{{ makeSetValue('p', '0', 'ret', 'i32') }}};    break;
      case {{{ cDefine('EM_FUNC_SIG_PARAM_F') }}}:   {{{ makeSetValue('p', '0', 'ret', 'float') }}};  break;
      case {{{ cDefine('EM_FUNC_SIG_PARAM_B') }}}: {{{ makeSetValue('p', '0', 'ret ? 1 : 0', 'i8') }}}; break;
#if GL_ASSERTIONS
      default: throw 'internal glGet error, bad type: ' + type;
#endif
    }
  },

  glGetIntegerv__sig: 'vii',
  glGetIntegerv__deps: ['$emscriptenWebGLGet'],
  glGetIntegerv: function(name_, p) {
    emscriptenWebGLGet(name_, p, {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}});
  },

  glGetFloatv__sig: 'vii',
  glGetFloatv__deps: ['$emscriptenWebGLGet'],
  glGetFloatv: function(name_, p) {
    emscriptenWebGLGet(name_, p, {{{ cDefine('EM_FUNC_SIG_PARAM_F') }}});
  },

  glGetBooleanv__sig: 'vii',
  glGetBooleanv__deps: ['$emscriptenWebGLGet'],
  glGetBooleanv: function(name_, p) {
    emscriptenWebGLGet(name_, p, {{{ cDefine('EM_FUNC_SIG_PARAM_B') }}});
  },

  glDeleteTextures__sig: 'vii',
  glDeleteTextures: function(n, textures) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('textures', 'i*4', 'i32') }}};
      var texture = GL.textures[id];
      if (!texture) continue; // GL spec: "glDeleteTextures silently ignores 0s and names that do not correspond to existing textures".
      GLctx.deleteTexture(texture);
      texture.name = 0;
      GL.textures[id] = null;
    }
  },

  glCompressedTexImage2D__sig: 'viiiiiiii',
  glCompressedTexImage2D: function(target, level, internalFormat, width, height, border, imageSize, data) {
#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, imageSize, data);
      } else {
        GLctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, HEAPU8, data, imageSize);
      }
      return;
    }
#endif
    GLctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, data ? {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}} : null);
  },


  glCompressedTexSubImage2D__sig: 'viiiiiiiii',
  glCompressedTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, imageSize, data) {
#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx['compressedTexSubImage2D'](target, level, xoffset, yoffset, width, height, format, imageSize, data);
      } else {
        GLctx['compressedTexSubImage2D'](target, level, xoffset, yoffset, width, height, format, HEAPU8, data, imageSize);
      }
      return;
    }
#endif
    GLctx['compressedTexSubImage2D'](target, level, xoffset, yoffset, width, height, format, data ? {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}} : null);
  },

  _computeUnpackAlignedImageSize: function(width, height, sizePerPixel, alignment) {
    function roundedToNextMultipleOf(x, y) {
#if GL_ASSERTIONS
      assert((y & (y-1)) === 0, 'Unpack alignment must be a power of 2! (Allowed values per WebGL spec are 1, 2, 4 or 8)');
#endif
      return (x + y - 1) & -y;
    }
    var plainRowSize = width * sizePerPixel;
    var alignedRowSize = roundedToNextMultipleOf(plainRowSize, alignment);
    return height * alignedRowSize;
  },

  _colorChannelsInGlTextureFormat: {
    0x1906 /* GL_ALPHA */: 1,
    0x1909 /* GL_LUMINANCE */: 1,
    0x1902 /* GL_DEPTH_COMPONENT */: 1,
    0x190A /* GL_LUMINANCE_ALPHA */: 2,
    0x1907 /* GL_RGB */: 3,
    0x8C40 /* GL_SRGB_EXT */: 3,
    0x1908 /* GL_RGBA */: 4,
    0x8C42 /* GL_SRGB_ALPHA_EXT */: 4,
#if USE_WEBGL2
    0x1903 /* GL_RED */: 1,
    0x8D94 /* GL_RED_INTEGER */: 1,
    0x8227 /* GL_RG */: 2,
    0x8228 /* GL_RG_INTEGER*/: 2,
    0x8D98 /* GL_RGB_INTEGER */: 3,
    0x8D99 /* GL_RGBA_INTEGER */: 4
#endif
  },

  _sizeOfGlTextureElementType: {
    0x1401 /* GL_UNSIGNED_BYTE */: 1,
    0x1403 /* GL_UNSIGNED_SHORT */: 2,
    0x8D61 /* GL_HALF_FLOAT_OES */: 2,
    0x1405 /* GL_UNSIGNED_INT */: 4,
    0x1406 /* GL_FLOAT */: 4,
    0x84FA /* GL_UNSIGNED_INT_24_8_WEBGL/GL_UNSIGNED_INT_24_8 */: 4,
    0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */: 2,
    0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */: 2,
    0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */: 2,
#if USE_WEBGL2
    0x1400 /* GL_BYTE */: 1,
    0x140B /* GL_HALF_FLOAT */: 2,
    0x1402 /* GL_SHORT */: 2,
    0x1404 /* GL_INT */: 4,
    0x8C3E /* GL_UNSIGNED_INT_5_9_9_9_REV */: 4,
    0x8368 /* GL_UNSIGNED_INT_2_10_10_10_REV */: 4,
    0x8C3B /* GL_UNSIGNED_INT_10F_11F_11F_REV */: 4,
#endif
  },

  $emscriptenWebGLGetTexPixelData__deps: ['_computeUnpackAlignedImageSize', '_colorChannelsInGlTextureFormat', '_sizeOfGlTextureElementType'],
  $emscriptenWebGLGetTexPixelData: function(type, format, width, height, pixels, internalFormat) {
    var sizePerPixel = __colorChannelsInGlTextureFormat[format] * __sizeOfGlTextureElementType[type];
    if (!sizePerPixel) {
      GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
      if (!__colorChannelsInGlTextureFormat[format]) err('GL_INVALID_ENUM due to unknown format in glTex[Sub]Image/glReadPixels, format: ' + format);
      else err('GL_INVALID_ENUM in glTex[Sub]Image/glReadPixels, type: ' + type + ', format: ' + format);
#endif
      return;
    }
    var bytes = __computeUnpackAlignedImageSize(width, height, sizePerPixel, GL.unpackAlignment);
    var end = pixels + bytes;
    switch(type) {
#if USE_WEBGL2
      case 0x1400 /* GL_BYTE */:
        return HEAP8.subarray(pixels, end);
#endif
      case 0x1401 /* GL_UNSIGNED_BYTE */:
        return HEAPU8.subarray(pixels, end);
#if USE_WEBGL2
      case 0x1402 /* GL_SHORT */:
#if GL_ASSERTIONS
        assert((pixels & 1) == 0, 'Pointer to int16 data passed to texture get function must be aligned to two bytes!');
#endif
        return HEAP16.subarray(pixels>>1, end>>1);
      case 0x1404 /* GL_INT */:
#if GL_ASSERTIONS
        assert((pixels & 3) == 0, 'Pointer to integer data passed to texture get function must be aligned to four bytes!');
#endif
        return HEAP32.subarray(pixels>>2, end>>2);
#endif
      case 0x1406 /* GL_FLOAT */:
#if GL_ASSERTIONS
        assert((pixels & 3) == 0, 'Pointer to float data passed to texture get function must be aligned to four bytes!');
#endif
        return HEAPF32.subarray(pixels>>2, end>>2);
      case 0x1405 /* GL_UNSIGNED_INT */:
      case 0x84FA /* GL_UNSIGNED_INT_24_8_WEBGL/GL_UNSIGNED_INT_24_8 */:
#if USE_WEBGL2
      case 0x8C3E /* GL_UNSIGNED_INT_5_9_9_9_REV */:
      case 0x8368 /* GL_UNSIGNED_INT_2_10_10_10_REV */:
      case 0x8C3B /* GL_UNSIGNED_INT_10F_11F_11F_REV */:
#endif
#if GL_ASSERTIONS
        assert((pixels & 3) == 0, 'Pointer to integer data passed to texture get function must be aligned to four bytes!');
#endif
        return HEAPU32.subarray(pixels>>2, end>>2);
      case 0x1403 /* GL_UNSIGNED_SHORT */:
      case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
      case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
      case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
      case 0x8D61 /* GL_HALF_FLOAT_OES */:
#if USE_WEBGL2
      case 0x140B /* GL_HALF_FLOAT */:
#endif
#if GL_ASSERTIONS
        assert((pixels & 1) == 0, 'Pointer to int16 data passed to texture get function must be aligned to two bytes!');
#endif
        return HEAPU16.subarray(pixels>>1, end>>1);
      default:
        GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
        err('GL_INVALID_ENUM in glTex[Sub]Image/glReadPixels, type: ' + type);
#endif
    }
  },

  glTexImage2D__sig: 'viiiiiiiii',
  glTexImage2D__deps: ['$emscriptenWebGLGetTexPixelData'
#if USE_WEBGL2
                       , '_heapObjectForWebGLType', '_heapAccessShiftForWebGLType'
#endif
  ],
  glTexImage2D: function(target, level, internalFormat, width, height, border, format, type, pixels) {
#if USE_WEBGL2
#if WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION
    if (GL.currentContext.version >= 2) {
      // WebGL 1 unsized texture internalFormats are no longer supported in WebGL 2, so patch those format
      // enums to the ones that are present in WebGL 2.
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
    }
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) {
      // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
      } else if (pixels != 0) {
        GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, __heapObjectForWebGLType(type), pixels >> (__heapAccessShiftForWebGLType[type]|0));
      } else {
        GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, null);
      }
      return;
    }
#endif
    GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat) : null);
  },

  glTexSubImage2D__sig: 'viiiiiiiii',
  glTexSubImage2D__deps: ['$emscriptenWebGLGetTexPixelData'
#if USE_WEBGL2
                          , '_heapObjectForWebGLType', '_heapAccessShiftForWebGLType'
#endif
  ],
  glTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, type, pixels) {
#if USE_WEBGL2
#if WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION
    if (GL.currentContext.version >= 2) {
      // In WebGL 1 to do half float textures, one uses the type enum GL_HALF_FLOAT_OES, but in
      // WebGL 2 when half float textures were adopted to the core spec, the enum changed value
      // which breaks backwards compatibility. Route old enum number to the new one.
      if (type == 0x8d61/*GL_HALF_FLOAT_OES*/) type = 0x140B /*GL_HALF_FLOAT*/;
    }
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) {
      // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
      } else if (pixels != 0) {
        GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, __heapObjectForWebGLType(type), pixels >> (__heapAccessShiftForWebGLType[type]|0));
      } else {
        GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, null);
      }
      return;
    }
#endif
    var pixelData = null;
    if (pixels) pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, 0);
    GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixelData);
  },

  glReadPixels__sig: 'viiiiiii',
  glReadPixels__deps: ['$emscriptenWebGLGetTexPixelData'
#if USE_WEBGL2
                       , '_heapObjectForWebGLType', '_heapAccessShiftForWebGLType'
#endif
  ],
  glReadPixels: function(x, y, width, height, format, type, pixels) {
#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      if (GLctx.currentPixelPackBufferBinding) {
        GLctx.readPixels(x, y, width, height, format, type, pixels);
      } else {
        GLctx.readPixels(x, y, width, height, format, type, __heapObjectForWebGLType(type), pixels >> (__heapAccessShiftForWebGLType[type]|0));
      }
      return;
    }
#endif
    var pixelData = emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, format);
    if (!pixelData) {
      GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
      err('GL_INVALID_ENUM in glReadPixels: Unrecognized combination of type=' + type + ' and format=' + format + '!');
#endif
      return;
    }
    GLctx.readPixels(x, y, width, height, format, type, pixelData);
  },

  glBindTexture__sig: 'vii',
  glBindTexture: function(target, texture) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.textures, texture, 'glBindTexture', 'texture');
#endif
    GLctx.bindTexture(target, GL.textures[texture]);
  },

  glGetTexParameterfv__sig: 'viii',
  glGetTexParameterfv: function(target, pname, params) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetTexParameterfv(target=' + target +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('params', '0', 'GLctx.getTexParameter(target, pname)', 'float') }}};
  },

  glGetTexParameteriv__sig: 'viii',
  glGetTexParameteriv: function(target, pname, params) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetTexParameteriv(target=' + target +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('params', '0', 'GLctx.getTexParameter(target, pname)', 'i32') }}};
  },

  glTexParameterfv__sig: 'viii',
  glTexParameterfv: function(target, pname, params) {
    var param = {{{ makeGetValue('params', '0', 'float') }}};
    GLctx.texParameterf(target, pname, param);
  },

  glTexParameteriv__sig: 'viii',
  glTexParameteriv: function(target, pname, params) {
    var param = {{{ makeGetValue('params', '0', 'i32') }}};
    GLctx.texParameteri(target, pname, param);
  },

  glIsTexture__sig: 'ii',
  glIsTexture: function(id) {
    var texture = GL.textures[id];
    if (!texture) return 0;
    return GLctx.isTexture(texture);
  },

  // The code path for creating textures, buffers, framebuffers and other objects is so identical to each other (and not in fast path), that
  // merge the functions together to only have one generated copy of this. 'createFunction' refers to the WebGL context function name to do
  // the actual creation, 'objectTable' points to the GL object table where to populate the created objects, and 'functionName' carries
  // the name of the caller for debug information.
  _glGenObject__sig: 'vii',
  _glGenObject: function(n, buffers, createFunction, objectTable
#if GL_ASSERTIONS
    , functionName
#endif
    ) {
    for (var i = 0; i < n; i++) {
      var buffer = GLctx[createFunction]();
      var id = buffer && GL.getNewId(objectTable);
      if (buffer) {
        buffer.name = id;
        objectTable[id] = buffer;
      } else {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
        err('GL_INVALID_OPERATION in ' + functionName + ': GLctx.' + createFunction + ' returned null - most likely GL context is lost!');
#endif
      }
      {{{ makeSetValue('buffers', 'i*4', 'id', 'i32') }}};
    }
  },

  glGenBuffers__deps: ['_glGenObject'],
  glGenBuffers__sig: 'vii',
  glGenBuffers: function(n, buffers) {
    __glGenObject(n, buffers, 'createBuffer', GL.buffers
#if GL_ASSERTIONS
    , 'glGenBuffers'
#endif
      );
  },

  glGenTextures__deps: ['_glGenObject'],
  glGenTextures__sig: 'vii',
  glGenTextures: function(n, textures) {
    __glGenObject(n, textures, 'createTexture', GL.textures
#if GL_ASSERTIONS
    , 'glGenTextures'
#endif
      );
  },

  glDeleteBuffers__sig: 'vii',
  glDeleteBuffers: function(n, buffers) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('buffers', 'i*4', 'i32') }}};
      var buffer = GL.buffers[id];

      // From spec: "glDeleteBuffers silently ignores 0's and names that do not
      // correspond to existing buffer objects."
      if (!buffer) continue;

      GLctx.deleteBuffer(buffer);
      buffer.name = 0;
      GL.buffers[id] = null;

      if (id == GL.currArrayBuffer) GL.currArrayBuffer = 0;
      if (id == GL.currElementArrayBuffer) GL.currElementArrayBuffer = 0;
#if USE_WEBGL2
      if (id == GLctx.currentPixelPackBufferBinding) GLctx.currentPixelPackBufferBinding = 0;
      if (id == GLctx.currentPixelUnpackBufferBinding) GLctx.currentPixelUnpackBufferBinding = 0;
#endif
    }
  },

  glGetBufferParameteriv__sig: 'viii',
  glGetBufferParameteriv: function(target, value, data) {
    if (!data) {
      // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
      // if data == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetBufferParameteriv(target=' + target + ', value=' + value + ', data=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('data', '0', 'GLctx.getBufferParameter(target, value)', 'i32') }}};
  },

  glBufferData__sig: 'viiii',
  glBufferData: function(target, size, data, usage) {
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
#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      if (data) {
        GLctx.bufferData(target, HEAPU8, usage, data, size);
      } else {
        GLctx.bufferData(target, size, usage);
      }
    } else {
#endif
      // N.b. here first form specifies a heap subarray, second form an integer size, so the ?: code here is polymorphic. It is advised to avoid
      // randomly mixing both uses in calling code, to avoid any potential JS engine JIT issues.
      GLctx.bufferData(target, data ? HEAPU8.subarray(data, data+size) : size, usage);
#if USE_WEBGL2
    }
#endif
  },

  glBufferSubData__sig: 'viiii',
  glBufferSubData: function(target, offset, size, data) {
#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.bufferSubData(target, offset, HEAPU8, data, size);
      return;
    }
#endif
    GLctx.bufferSubData(target, offset, HEAPU8.subarray(data, data+size));
  },

  // Queries EXT
  glGenQueriesEXT__sig: 'vii',
  glGenQueriesEXT: function(n, ids) {
    for (var i = 0; i < n; i++) {
      var query = GLctx.disjointTimerQueryExt['createQueryEXT']();
      if (!query) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
        err('GL_INVALID_OPERATION in glGenQueriesEXT: GLctx.disjointTimerQueryExt.createQueryEXT returned null - most likely GL context is lost!');
#endif
        while(i < n) {{{ makeSetValue('ids', 'i++*4', 0, 'i32') }}};
        return;
      }
      var id = GL.getNewId(GL.timerQueriesEXT);
      query.name = id;
      GL.timerQueriesEXT[id] = query;
      {{{ makeSetValue('ids', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteQueriesEXT__sig: 'vii',
  glDeleteQueriesEXT: function(n, ids) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('ids', 'i*4', 'i32') }}};
      var query = GL.timerQueriesEXT[id];
      if (!query) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
      GLctx.disjointTimerQueryExt['deleteQueryEXT'](query);
      GL.timerQueriesEXT[id] = null;
    }
  },

  glIsQueryEXT__sig: 'ii',
  glIsQueryEXT: function(id) {
    var query = GL.timerQueriesEXT[id];
    if (!query) return 0;
    return GLctx.disjointTimerQueryExt['isQueryEXT'](query);
  },

  glBeginQueryEXT__sig: 'vii',
  glBeginQueryEXT: function(target, id) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.timerQueriesEXT, id, 'glBeginQueryEXT', 'id');
#endif
    GLctx.disjointTimerQueryExt['beginQueryEXT'](target, GL.timerQueriesEXT[id]);
  },

  glEndQueryEXT__sig: 'vi',
  glEndQueryEXT: function(target) {
    GLctx.disjointTimerQueryExt['endQueryEXT'](target);
  },

  glQueryCounterEXT__sig: 'vii',
  glQueryCounterEXT: function(id, target) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.timerQueriesEXT, id, 'glQueryCounterEXT', 'id');
#endif
    GLctx.disjointTimerQueryExt['queryCounterEXT'](GL.timerQueriesEXT[id], target);
  },

  glGetQueryivEXT__sig: 'viii',
  glGetQueryivEXT: function(target, pname, params) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetQueryivEXT(target=' + target +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('params', '0', 'GLctx.disjointTimerQueryExt[\'getQueryEXT\'](target, pname)', 'i32') }}};
  },

  glGetQueryObjectivEXT__sig: 'viii',
  glGetQueryObjectivEXT: function(id, pname, params) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetQueryObject(u)ivEXT(id=' + id +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.timerQueriesEXT, id, 'glGetQueryObjectivEXT', 'id');
#endif
    var query = GL.timerQueriesEXT[id];
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

  glGetQueryObjecti64vEXT__sig: 'viii',
  glGetQueryObjecti64vEXT: function(id, pname, params) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetQueryObject(u)i64vEXT(id=' + id +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.timerQueriesEXT, id, 'glGetQueryObjecti64vEXT', 'id');
#endif
    var query = GL.timerQueriesEXT[id];
    var param = GLctx.disjointTimerQueryExt['getQueryObjectEXT'](query, pname);
    var ret;
    if (typeof param == 'boolean') {
      ret = param ? 1 : 0;
    } else {
      ret = param;
    }
    {{{ makeSetValue('params', '0', 'ret', 'i64') }}};
  },
  glGetQueryObjectui64vEXT: 'glGetQueryObjecti64vEXT',

  glIsBuffer__sig: 'ii',
  glIsBuffer: function(buffer) {
    var b = GL.buffers[buffer];
    if (!b) return 0;
    return GLctx.isBuffer(b);
  },

  glGenRenderbuffers__sig: 'vii',
  glGenRenderbuffers__deps: ['_glGenObject'],
  glGenRenderbuffers: function(n, renderbuffers) {
    __glGenObject(n, renderbuffers, 'createRenderbuffer', GL.renderbuffers
#if GL_ASSERTIONS
    , 'glGenRenderbuffers'
#endif
      );
  },

  glDeleteRenderbuffers__sig: 'vii',
  glDeleteRenderbuffers: function(n, renderbuffers) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('renderbuffers', 'i*4', 'i32') }}};
      var renderbuffer = GL.renderbuffers[id];
      if (!renderbuffer) continue; // GL spec: "glDeleteRenderbuffers silently ignores 0s and names that do not correspond to existing renderbuffer objects".
      GLctx.deleteRenderbuffer(renderbuffer);
      renderbuffer.name = 0;
      GL.renderbuffers[id] = null;
    }
  },

  glBindRenderbuffer__sig: 'vii',
  glBindRenderbuffer: function(target, renderbuffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.renderbuffers, renderbuffer, 'glBindRenderbuffer', 'renderbuffer');
#endif
    GLctx.bindRenderbuffer(target, GL.renderbuffers[renderbuffer]);
  },

  glGetRenderbufferParameteriv__sig: 'viii',
  glGetRenderbufferParameteriv: function(target, pname, params) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetRenderbufferParameteriv(target=' + target + ', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('params', '0', 'GLctx.getRenderbufferParameter(target, pname)', 'i32') }}};
  },

  glIsRenderbuffer__sig: 'ii',
  glIsRenderbuffer: function(renderbuffer) {
    var rb = GL.renderbuffers[renderbuffer];
    if (!rb) return 0;
    return GLctx.isRenderbuffer(rb);
  },

  $emscriptenWebGLGetUniform: function(program, location, params, type) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetUniform*v(program=' + program + ', location=' + location + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniform*v', 'program');
    GL.validateGLObjectID(GL.uniforms, location, 'glGetUniform*v', 'location');
#endif
    var data = GLctx.getUniform(GL.programs[program], GL.uniforms[location]);
    if (typeof data == 'number' || typeof data == 'boolean') {
      switch (type) {
        case {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}}: {{{ makeSetValue('params', '0', 'data', 'i32') }}}; break;
        case {{{ cDefine('EM_FUNC_SIG_PARAM_F') }}}: {{{ makeSetValue('params', '0', 'data', 'float') }}}; break;
        default: throw 'internal emscriptenWebGLGetUniform() error, bad type: ' + type;
      }
    } else {
      for (var i = 0; i < data.length; i++) {
        switch (type) {
          case {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}}: {{{ makeSetValue('params', 'i*4', 'data[i]', 'i32') }}}; break;
          case {{{ cDefine('EM_FUNC_SIG_PARAM_F') }}}: {{{ makeSetValue('params', 'i*4', 'data[i]', 'float') }}}; break;
          default: throw 'internal emscriptenWebGLGetUniform() error, bad type: ' + type;
        }
      }
    }
  },

  glGetUniformfv__sig: 'viii',
  glGetUniformfv__deps: ['$emscriptenWebGLGetUniform'],
  glGetUniformfv: function(program, location, params) {
    emscriptenWebGLGetUniform(program, location, params, {{{ cDefine('EM_FUNC_SIG_PARAM_F') }}});
  },

  glGetUniformiv__sig: 'viii',
  glGetUniformiv__deps: ['$emscriptenWebGLGetUniform'],
  glGetUniformiv: function(program, location, params) {
    emscriptenWebGLGetUniform(program, location, params, {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}});
  },

  glGetUniformLocation__sig: 'iii',
  glGetUniformLocation: function(program, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformLocation', 'program');
#endif
    name = UTF8ToString(name);

    var arrayIndex = 0;
    // If user passed an array accessor "[index]", parse the array index off the accessor.
    if (name[name.length - 1] == ']') {
      var leftBrace = name.lastIndexOf('[');
      arrayIndex = name[leftBrace+1] != ']' ? parseInt(name.slice(leftBrace + 1)) : 0; // "index]", parseInt will ignore the ']' at the end; but treat "foo[]" as "foo[0]"
      name = name.slice(0, leftBrace);
    }

    var uniformInfo = GL.programInfos[program] && GL.programInfos[program].uniforms[name]; // returns pair [ dimension_of_uniform_array, uniform_location ]
    if (uniformInfo && arrayIndex >= 0 && arrayIndex < uniformInfo[0]) { // Check if user asked for an out-of-bounds element, i.e. for 'vec4 colors[3];' user could ask for 'colors[10]' which should return -1.
      return uniformInfo[1] + arrayIndex;
    } else {
      return -1;
    }
  },

  $emscriptenWebGLGetVertexAttrib: function(index, pname, params, type) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetVertexAttrib*v(index=' + index + ', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#if FULL_ES2
    if (GL.currentContext.clientBuffers[index].enabled) {
      err("glGetVertexAttrib*v on client-side array: not supported, bad data returned");
    }
#endif
    var data = GLctx.getVertexAttrib(index, pname);
    if (pname == 0x889F/*VERTEX_ATTRIB_ARRAY_BUFFER_BINDING*/) {
      {{{ makeSetValue('params', '0', 'data["name"]', 'i32') }}};
    } else if (typeof data == 'number' || typeof data == 'boolean') {
      switch (type) {
        case {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}}: {{{ makeSetValue('params', '0', 'data', 'i32') }}}; break;
        case {{{ cDefine('EM_FUNC_SIG_PARAM_F') }}}: {{{ makeSetValue('params', '0', 'data', 'float') }}}; break;
        case {{{ cDefine('EM_FUNC_SIG_PARAM_F2I') }}}: {{{ makeSetValue('params', '0', 'Math.fround(data)', 'i32') }}}; break;
        default: throw 'internal emscriptenWebGLGetVertexAttrib() error, bad type: ' + type;
      }
    } else {
      for (var i = 0; i < data.length; i++) {
        switch (type) {
          case {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}}: {{{ makeSetValue('params', 'i*4', 'data[i]', 'i32') }}}; break;
          case {{{ cDefine('EM_FUNC_SIG_PARAM_F') }}}: {{{ makeSetValue('params', 'i*4', 'data[i]', 'float') }}}; break;
          case {{{ cDefine('EM_FUNC_SIG_PARAM_F2I') }}}: {{{ makeSetValue('params', 'i*4', 'Math.fround(data[i])', 'i32') }}}; break;
          default: throw 'internal emscriptenWebGLGetVertexAttrib() error, bad type: ' + type;
        }
      }
    }
  },

  glGetVertexAttribfv__sig: 'viii',
  glGetVertexAttribfv__deps: ['$emscriptenWebGLGetVertexAttrib'],
  glGetVertexAttribfv: function(index, pname, params) {
    // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttrib*f(),
    // otherwise the results are undefined. (GLES3 spec 6.1.12)
    emscriptenWebGLGetVertexAttrib(index, pname, params, {{{ cDefine('EM_FUNC_SIG_PARAM_F') }}});
  },

  glGetVertexAttribiv__sig: 'viii',
  glGetVertexAttribiv__deps: ['$emscriptenWebGLGetVertexAttrib'],
  glGetVertexAttribiv: function(index, pname, params) {
    // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttrib*f(),
    // otherwise the results are undefined. (GLES3 spec 6.1.12)
    emscriptenWebGLGetVertexAttrib(index, pname, params, {{{ cDefine('EM_FUNC_SIG_PARAM_F2I') }}});
  },

  glGetVertexAttribPointerv__sig: 'viii',
  glGetVertexAttribPointerv: function(index, pname, pointer) {
    if (!pointer) {
      // GLES2 specification does not specify how to behave if pointer is a null pointer. Since calling this function does not make sense
      // if pointer == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetVertexAttribPointerv(index=' + index + ', pname=' + pname + ', pointer=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#if FULL_ES2
    if (GL.currentContext.clientBuffers[index].enabled) {
      err("glGetVertexAttribPointer on client-side array: not supported, bad data returned");
    }
#endif
    {{{ makeSetValue('pointer', '0', 'GLctx.getVertexAttribOffset(index, pname)', 'i32') }}};
  },

  glGetActiveUniform__sig: 'viiiiiii',
  glGetActiveUniform: function(program, index, bufSize, length, size, type, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveUniform', 'program');
#endif
    program = GL.programs[program];
    var info = GLctx.getActiveUniform(program, index);
    if (!info) return; // If an error occurs, nothing will be written to length, size, type and name.

    var numBytesWrittenExclNull = (bufSize > 0 && name) ? stringToUTF8(info.name, name, bufSize) : 0;
    if (length) {{{ makeSetValue('length', '0', 'numBytesWrittenExclNull', 'i32') }}};
    if (size) {{{ makeSetValue('size', '0', 'info.size', 'i32') }}};
    if (type) {{{ makeSetValue('type', '0', 'info.type', 'i32') }}};
  },

  glUniform1f__sig: 'vif',
  glUniform1f: function(location, v0) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1f', 'location');
#endif
    GLctx.uniform1f(GL.uniforms[location], v0);
  },

  glUniform2f__sig: 'viff',
  glUniform2f: function(location, v0, v1) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2f', 'location');
#endif
    GLctx.uniform2f(GL.uniforms[location], v0, v1);
  },

  glUniform3f__sig: 'vifff',
  glUniform3f: function(location, v0, v1, v2) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3f', 'location');
#endif
    GLctx.uniform3f(GL.uniforms[location], v0, v1, v2);
  },

  glUniform4f__sig: 'viffff',
  glUniform4f: function(location, v0, v1, v2, v3) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4f', 'location');
#endif
    GLctx.uniform4f(GL.uniforms[location], v0, v1, v2, v3);
  },

  glUniform1i__sig: 'vii',
  glUniform1i: function(location, v0) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1i', 'location');
#endif
    GLctx.uniform1i(GL.uniforms[location], v0);
  },

  glUniform2i__sig: 'viii',
  glUniform2i: function(location, v0, v1) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2i', 'location');
#endif
    GLctx.uniform2i(GL.uniforms[location], v0, v1);
  },

  glUniform3i__sig: 'viiii',
  glUniform3i: function(location, v0, v1, v2) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3i', 'location');
#endif
    GLctx.uniform3i(GL.uniforms[location], v0, v1, v2);
  },

  glUniform4i__sig: 'viiiii',
  glUniform4i: function(location, v0, v1, v2, v3) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4i', 'location');
#endif
    GLctx.uniform4i(GL.uniforms[location], v0, v1, v2, v3);
  },

  glUniform1iv__sig: 'viii',
  glUniform1iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1iv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform1iv must be aligned to four bytes!');
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform1iv(GL.uniforms[location], HEAP32, value>>2, count);
      return;
    }
#endif

    GLctx.uniform1iv(GL.uniforms[location], {{{ makeHEAPView('32', 'value', 'value+count*4') }}});
  },

  glUniform2iv__sig: 'viii',
  glUniform2iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2iv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform2iv must be aligned to four bytes!');
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform2iv(GL.uniforms[location], HEAP32, value>>2, count*2);
      return;
    }
#endif

    GLctx.uniform2iv(GL.uniforms[location], {{{ makeHEAPView('32', 'value', 'value+count*8') }}});
  },

  glUniform3iv__sig: 'viii',
  glUniform3iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3iv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform3iv must be aligned to four bytes!');
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform3iv(GL.uniforms[location], HEAP32, value>>2, count*3);
      return;
    }
#endif

    GLctx.uniform3iv(GL.uniforms[location], {{{ makeHEAPView('32', 'value', 'value+count*12') }}});
  },

  glUniform4iv__sig: 'viii',
  glUniform4iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4iv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform4iv must be aligned to four bytes!');
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform4iv(GL.uniforms[location], HEAP32, value>>2, count*4);
      return;
    }
#endif

    GLctx.uniform4iv(GL.uniforms[location], {{{ makeHEAPView('32', 'value', 'value+count*16') }}});
  },

  glUniform1fv__sig: 'viii',
  glUniform1fv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniform1fv must be aligned to four bytes!');
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform1fv(GL.uniforms[location], HEAPF32, value>>2, count);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (count <= GL.MINI_TEMP_BUFFER_SIZE) {
      // avoid allocation when uploading few enough uniforms
      var view = GL.miniTempBufferViews[count-1];
      for (var i = 0; i < count; ++i) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
#if WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG
      if (GL.currentContext.cannotHandleOffsetsInUniformArrayViews) view = new Float32Array(view);
#endif
    }
    GLctx.uniform1fv(GL.uniforms[location], view);
  },

  glUniform2fv__sig: 'viii',
  glUniform2fv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniform2fv must be aligned to four bytes!');
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform2fv(GL.uniforms[location], HEAPF32, value>>2, count*2);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (2*count <= GL.MINI_TEMP_BUFFER_SIZE) {
      // avoid allocation when uploading few enough uniforms
      var view = GL.miniTempBufferViews[2*count-1];
      for (var i = 0; i < 2*count; i += 2) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*8') }}};
#if WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG
      if (GL.currentContext.cannotHandleOffsetsInUniformArrayViews) view = new Float32Array(view);
#endif
    }
    GLctx.uniform2fv(GL.uniforms[location], view);
  },

  glUniform3fv__sig: 'viii',
  glUniform3fv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniform3fv must be aligned to four bytes!' + value);
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform3fv(GL.uniforms[location], HEAPF32, value>>2, count*3);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (3*count <= GL.MINI_TEMP_BUFFER_SIZE) {
      // avoid allocation when uploading few enough uniforms
      var view = GL.miniTempBufferViews[3*count-1];
      for (var i = 0; i < 3*count; i += 3) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'float') }}};
        view[i+2] = {{{ makeGetValue('value', '4*i+8', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*12') }}};
#if WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG
      if (GL.currentContext.cannotHandleOffsetsInUniformArrayViews) view = new Float32Array(view);
#endif
    }
    GLctx.uniform3fv(GL.uniforms[location], view);
  },

  glUniform4fv__sig: 'viii',
  glUniform4fv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniform4fv must be aligned to four bytes!');
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform4fv(GL.uniforms[location], HEAPF32, value>>2, count*4);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (4*count <= GL.MINI_TEMP_BUFFER_SIZE) {
      // avoid allocation when uploading few enough uniforms
      var view = GL.miniTempBufferViews[4*count-1];
      for (var i = 0; i < 4*count; i += 4) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'float') }}};
        view[i+2] = {{{ makeGetValue('value', '4*i+8', 'float') }}};
        view[i+3] = {{{ makeGetValue('value', '4*i+12', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*16') }}};
#if WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG
      if (GL.currentContext.cannotHandleOffsetsInUniformArrayViews) view = new Float32Array(view);
#endif
    }
    GLctx.uniform4fv(GL.uniforms[location], view);
  },

  glUniformMatrix2fv__sig: 'viiii',
  glUniformMatrix2fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix2fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix2fv must be aligned to four bytes!');
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniformMatrix2fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*4);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (4*count <= GL.MINI_TEMP_BUFFER_SIZE) {
      // avoid allocation when uploading few enough uniforms
      var view = GL.miniTempBufferViews[4*count-1];
      for (var i = 0; i < 4*count; i += 4) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'float') }}};
        view[i+2] = {{{ makeGetValue('value', '4*i+8', 'float') }}};
        view[i+3] = {{{ makeGetValue('value', '4*i+12', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*16') }}};
#if WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG
      if (GL.currentContext.cannotHandleOffsetsInUniformArrayViews) view = new Float32Array(view);
#endif
    }
    GLctx.uniformMatrix2fv(GL.uniforms[location], !!transpose, view);
  },

  glUniformMatrix3fv__sig: 'viiii',
  glUniformMatrix3fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix3fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix3fv must be aligned to four bytes!');
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniformMatrix3fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*9);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (9*count <= GL.MINI_TEMP_BUFFER_SIZE) {
      // avoid allocation when uploading few enough uniforms
      var view = GL.miniTempBufferViews[9*count-1];
      for (var i = 0; i < 9*count; i += 9) {
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
#if WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG
      if (GL.currentContext.cannotHandleOffsetsInUniformArrayViews) view = new Float32Array(view);
#endif
    }
    GLctx.uniformMatrix3fv(GL.uniforms[location], !!transpose, view);
  },

  glUniformMatrix4fv__sig: 'viiii',
  glUniformMatrix4fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix4fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix4fv must be aligned to four bytes!');
#endif

#if USE_WEBGL2
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniformMatrix4fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*16);
      return;
    }
#endif

#if GL_POOL_TEMP_BUFFERS
    if (16*count <= GL.MINI_TEMP_BUFFER_SIZE) {
      // avoid allocation when uploading few enough uniforms
      var view = GL.miniTempBufferViews[16*count-1];
      for (var i = 0; i < 16*count; i += 16) {
        view[i] = {{{ makeGetValue('value', '4*i', 'float') }}};
        view[i+1] = {{{ makeGetValue('value', '4*i+4', 'float') }}};
        view[i+2] = {{{ makeGetValue('value', '4*i+8', 'float') }}};
        view[i+3] = {{{ makeGetValue('value', '4*i+12', 'float') }}};
        view[i+4] = {{{ makeGetValue('value', '4*i+16', 'float') }}};
        view[i+5] = {{{ makeGetValue('value', '4*i+20', 'float') }}};
        view[i+6] = {{{ makeGetValue('value', '4*i+24', 'float') }}};
        view[i+7] = {{{ makeGetValue('value', '4*i+28', 'float') }}};
        view[i+8] = {{{ makeGetValue('value', '4*i+32', 'float') }}};
        view[i+9] = {{{ makeGetValue('value', '4*i+36', 'float') }}};
        view[i+10] = {{{ makeGetValue('value', '4*i+40', 'float') }}};
        view[i+11] = {{{ makeGetValue('value', '4*i+44', 'float') }}};
        view[i+12] = {{{ makeGetValue('value', '4*i+48', 'float') }}};
        view[i+13] = {{{ makeGetValue('value', '4*i+52', 'float') }}};
        view[i+14] = {{{ makeGetValue('value', '4*i+56', 'float') }}};
        view[i+15] = {{{ makeGetValue('value', '4*i+60', 'float') }}};
      }
    } else
#endif
    {
      var view = {{{ makeHEAPView('F32', 'value', 'value+count*64') }}};
#if WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG
      if (GL.currentContext.cannotHandleOffsetsInUniformArrayViews) view = new Float32Array(view);
#endif
    }
    GLctx.uniformMatrix4fv(GL.uniforms[location], !!transpose, view);
  },

  glBindBuffer__sig: 'vii',
  glBindBuffer: function(target, buffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.buffers, buffer, 'glBindBuffer', 'buffer');
#endif
#if FULL_ES2 || LEGACY_GL_EMULATION
    if (target == GLctx.ARRAY_BUFFER) {
      GL.currArrayBuffer = buffer;
#if LEGACY_GL_EMULATION
      GLImmediate.lastArrayBuffer = buffer;
#endif
    } else if (target == GLctx.ELEMENT_ARRAY_BUFFER) {
      GL.currElementArrayBuffer = buffer;
    }
#endif

#if USE_WEBGL2
    if (target == 0x88EB /*GL_PIXEL_PACK_BUFFER*/) {
      // In WebGL 2 glReadPixels entry point, we need to use a different WebGL 2 API function call when a buffer is bound to
      // GL_PIXEL_PACK_BUFFER_BINDING point, so must keep track whether that binding point is non-null to know what is
      // the proper API function to call.
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

  glVertexAttrib1fv__sig: 'vii',
  glVertexAttrib1fv: function(index, v) {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib1fv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttrib1fv!');
#endif

    GLctx.vertexAttrib1f(index, HEAPF32[v>>2]);
  },

  glVertexAttrib2fv__sig: 'vii',
  glVertexAttrib2fv: function(index, v) {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib2fv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttrib2fv!');
#endif

    GLctx.vertexAttrib2f(index, HEAPF32[v>>2], HEAPF32[v+4>>2]);
  },

  glVertexAttrib3fv__sig: 'vii',
  glVertexAttrib3fv: function(index, v) {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib3fv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttrib3fv!');
#endif

    GLctx.vertexAttrib3f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2]);
  },

  glVertexAttrib4fv__sig: 'vii',
  glVertexAttrib4fv: function(index, v) {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to float data passed to glVertexAttrib4fv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttrib4fv!');
#endif

    GLctx.vertexAttrib4f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2], HEAPF32[v+12>>2]);
  },

  glGetAttribLocation__sig: 'iii',
  glGetAttribLocation: function(program, name) {
    return GLctx.getAttribLocation(GL.programs[program], UTF8ToString(name));
  },

  glGetActiveAttrib__sig: 'viiiiiii',
  glGetActiveAttrib: function(program, index, bufSize, length, size, type, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveAttrib', 'program');
#endif
    program = GL.programs[program];
    var info = GLctx.getActiveAttrib(program, index);
    if (!info) return; // If an error occurs, nothing will be written to length, size and type and name.

    var numBytesWrittenExclNull = (bufSize > 0 && name) ? stringToUTF8(info.name, name, bufSize) : 0;
    if (length) {{{ makeSetValue('length', '0', 'numBytesWrittenExclNull', 'i32') }}};
    if (size) {{{ makeSetValue('size', '0', 'info.size', 'i32') }}};
    if (type) {{{ makeSetValue('type', '0', 'info.type', 'i32') }}};
  },

  glCreateShader__sig: 'ii',
  glCreateShader: function(shaderType) {
    var id = GL.getNewId(GL.shaders);
    GL.shaders[id] = GLctx.createShader(shaderType);
    return id;
  },

  glDeleteShader__sig: 'vi',
  glDeleteShader: function(id) {
    if (!id) return;
    var shader = GL.shaders[id];
    if (!shader) { // glDeleteShader actually signals an error when deleting a nonexisting object, unlike some other GL delete functions.
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    GLctx.deleteShader(shader);
    GL.shaders[id] = null;
  },

  glGetAttachedShaders__sig: 'viiii',
  glGetAttachedShaders: function(program, maxCount, count, shaders) {
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

  glShaderSource__sig: 'viiii',
  glShaderSource: function(shader, count, string, length) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glShaderSource', 'shader');
#endif
    var source = GL.getSource(shader, count, string, length);

#if WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION
    if (GL.currentContext.version >= 2) {
      // If a WebGL 1 shader happens to use GL_EXT_shader_texture_lod extension,
      // it will not compile on WebGL 2, because WebGL 2 no longer supports that
      // extension for WebGL 1 shaders. Therefore upgrade shaders to WebGL 2
      // by doing a bunch of dirty hacks. Not guaranteed to work on all shaders.
      // One might consider doing this for only the shaders that actually use
      // the GL_EXT_shader_texture_lod extension, but the problem is that
      // vertex and fragment shader versions need to match, and when compiling
      // the corresponding vertex shader, we would not know if that needed to
      // be compiled with or without the patch, so we must patch all shaders.
      if (source.indexOf('#version 100') != -1) {
        source = source.replace(/#extension GL_OES_standard_derivatives : enable/g, "");
        source = source.replace(/#extension GL_EXT_shader_texture_lod : enable/g, '');
        var prelude = '';
        if (source.indexOf('gl_FragColor') != -1) {
          prelude += 'out mediump vec4 GL_FragColor;\n';
          source = source.replace(/gl_FragColor/g, 'GL_FragColor');
        }
        if (source.indexOf('attribute') != -1) {
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

    GLctx.shaderSource(GL.shaders[shader], source);
  },

  glGetShaderSource__sig: 'viiii',
  glGetShaderSource: function(shader, bufSize, length, source) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderSource', 'shader');
#endif
    var result = GLctx.getShaderSource(GL.shaders[shader]);
    if (!result) return; // If an error occurs, nothing will be written to length or source.
    var numBytesWrittenExclNull = (bufSize > 0 && source) ? stringToUTF8(result, source, bufSize) : 0;
    if (length) {{{ makeSetValue('length', '0', 'numBytesWrittenExclNull', 'i32') }}};
  },

  glCompileShader__sig: 'vi',
  glCompileShader: function(shader) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glCompileShader', 'shader');
#endif
    GLctx.compileShader(GL.shaders[shader]);
#if GL_DEBUG
    var log = (GLctx.getShaderInfoLog(GL.shaders[shader]) || '').trim();
    if (log) console.error('glCompileShader: ' + log);
#endif
  },

  glGetShaderInfoLog__sig: 'viiii',
  glGetShaderInfoLog: function(shader, maxLength, length, infoLog) {
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

  glGetShaderiv__sig: 'viii',
  glGetShaderiv : function(shader, pname, p) {
    if (!p) {
      // GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetShaderiv(shader=' + shader + ', pname=' + pname + ', p=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
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
      {{{ makeSetValue('p', '0', 'log.length + 1', 'i32') }}};
    } else if (pname == 0x8B88) { // GL_SHADER_SOURCE_LENGTH
      var source = GLctx.getShaderSource(GL.shaders[shader]);
      var sourceLength = (source === null || source.length == 0) ? 0 : source.length + 1;
      {{{ makeSetValue('p', '0', 'sourceLength', 'i32') }}};
    } else {
      {{{ makeSetValue('p', '0', 'GLctx.getShaderParameter(GL.shaders[shader], pname)', 'i32') }}};
    }
  },

  glGetProgramiv__sig: 'viii',
  glGetProgramiv : function(program, pname, p) {
    if (!p) {
      // GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetProgramiv(program=' + program + ', pname=' + pname + ', p=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramiv', 'program');
#endif

    if (program >= GL.counter) {
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetProgramiv(program=' + program + ', pname=' + pname + ', p=0x' + p.toString(16) + '): The specified program object name was not generated by GL!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }

    var ptable = GL.programInfos[program];
    if (!ptable) {
#if GL_ASSERTIONS
      err('GL_INVALID_OPERATION in glGetProgramiv(program=' + program + ', pname=' + pname + ', p=0x' + p.toString(16) + '): The specified GL object name does not refer to a program object!');
#endif
      GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
      return;
    }

    if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
      var log = GLctx.getProgramInfoLog(GL.programs[program]);
#if GL_ASSERTIONS || GL_TRACK_ERRORS
      if (log === null) log = '(unknown error)';
#endif
      {{{ makeSetValue('p', '0', 'log.length + 1', 'i32') }}};
    } else if (pname == 0x8B87 /* GL_ACTIVE_UNIFORM_MAX_LENGTH */) {
      {{{ makeSetValue('p', '0', 'ptable.maxUniformLength', 'i32') }}};
    } else if (pname == 0x8B8A /* GL_ACTIVE_ATTRIBUTE_MAX_LENGTH */) {
      if (ptable.maxAttributeLength == -1) {
        program = GL.programs[program];
        var numAttribs = GLctx.getProgramParameter(program, 0x8B89/*GL_ACTIVE_ATTRIBUTES*/);
        ptable.maxAttributeLength = 0; // Spec says if there are no active attribs, 0 must be returned.
        for (var i = 0; i < numAttribs; ++i) {
          var activeAttrib = GLctx.getActiveAttrib(program, i);
          ptable.maxAttributeLength = Math.max(ptable.maxAttributeLength, activeAttrib.name.length+1);
        }
      }
      {{{ makeSetValue('p', '0', 'ptable.maxAttributeLength', 'i32') }}};
    } else if (pname == 0x8A35 /* GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH */) {
      if (ptable.maxUniformBlockNameLength == -1) {
        program = GL.programs[program];
        var numBlocks = GLctx.getProgramParameter(program, 0x8A36/*GL_ACTIVE_UNIFORM_BLOCKS*/);
        ptable.maxUniformBlockNameLength = 0;
        for (var i = 0; i < numBlocks; ++i) {
          var activeBlockName = GLctx.getActiveUniformBlockName(program, i);
          ptable.maxUniformBlockNameLength = Math.max(ptable.maxUniformBlockNameLength, activeBlockName.length+1);
        }
      }
      {{{ makeSetValue('p', '0', 'ptable.maxUniformBlockNameLength', 'i32') }}};
    } else {
      {{{ makeSetValue('p', '0', 'GLctx.getProgramParameter(GL.programs[program], pname)', 'i32') }}};
    }
  },

  glIsShader__sig: 'ii',
  glIsShader: function(shader) {
    var s = GL.shaders[shader];
    if (!s) return 0;
    return GLctx.isShader(s);
  },

  glCreateProgram__sig: 'i',
  glCreateProgram: function() {
    var id = GL.getNewId(GL.programs);
    var program = GLctx.createProgram();
    program.name = id;
    GL.programs[id] = program;
    return id;
  },

  glDeleteProgram__sig: 'vi',
  glDeleteProgram: function(id) {
    if (!id) return;
    var program = GL.programs[id];
    if (!program) { // glDeleteProgram actually signals an error when deleting a nonexisting object, unlike some other GL delete functions.
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    GLctx.deleteProgram(program);
    program.name = 0;
    GL.programs[id] = null;
    GL.programInfos[id] = null;
  },

  glAttachShader__sig: 'vii',
  glAttachShader: function(program, shader) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glAttachShader', 'program');
    GL.validateGLObjectID(GL.shaders, shader, 'glAttachShader', 'shader');
#endif
    GLctx.attachShader(GL.programs[program],
                            GL.shaders[shader]);
  },

  glDetachShader__sig: 'vii',
  glDetachShader: function(program, shader) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glDetachShader', 'program');
    GL.validateGLObjectID(GL.shaders, shader, 'glDetachShader', 'shader');
#endif
    GLctx.detachShader(GL.programs[program],
                            GL.shaders[shader]);
  },

  glGetShaderPrecisionFormat__sig: 'viiii',
  glGetShaderPrecisionFormat: function(shaderType, precisionType, range, precision) {
    var result = GLctx.getShaderPrecisionFormat(shaderType, precisionType);
    {{{ makeSetValue('range', '0', 'result.rangeMin', 'i32') }}};
    {{{ makeSetValue('range', '4', 'result.rangeMax', 'i32') }}};
    {{{ makeSetValue('precision', '0', 'result.precision', 'i32') }}};
  },

  glLinkProgram__sig: 'vi',
  glLinkProgram: function(program) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glLinkProgram', 'program');
#endif
    GLctx.linkProgram(GL.programs[program]);
#if GL_DEBUG
    var log = (GLctx.getProgramInfoLog(GL.programs[program]) || '').trim();
    if (log) console.error('glLinkProgram: ' + log);
#endif
    GL.populateUniformTable(program);
  },

  glGetProgramInfoLog__sig: 'viiii',
  glGetProgramInfoLog: function(program, maxLength, length, infoLog) {
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

  glUseProgram__sig: 'vi',
  glUseProgram: function(program) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glUseProgram', 'program');
#endif
    GLctx.useProgram(GL.programs[program]);
  },

  glValidateProgram__sig: 'vi',
  glValidateProgram: function(program) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glValidateProgram', 'program');
#endif
    GLctx.validateProgram(GL.programs[program]);
  },

  glIsProgram__sig: 'ii',
  glIsProgram: function(program) {
    program = GL.programs[program];
    if (!program) return 0;
    return GLctx.isProgram(program);
  },

  glBindAttribLocation__sig: 'viii',
  glBindAttribLocation: function(program, index, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glBindAttribLocation', 'program');
#endif
    GLctx.bindAttribLocation(GL.programs[program], index, UTF8ToString(name));
  },

  glBindFramebuffer__sig: 'vii',
  glBindFramebuffer: function(target, framebuffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.framebuffers, framebuffer, 'glBindFramebuffer', 'framebuffer');
#endif

#if OFFSCREEN_FRAMEBUFFER
    // defaultFbo may not be present if 'renderViaOffscreenBackBuffer' was not enabled during context creation time,
    // i.e. setting -s OFFSCREEN_FRAMEBUFFER=1 at compilation time does not yet mandate that offscreen back buffer
    // is being used, but that is ultimately decided at context creation time.
    GLctx.bindFramebuffer(target, framebuffer ? GL.framebuffers[framebuffer] : GL.currentContext.defaultFbo);
#else
    GLctx.bindFramebuffer(target, GL.framebuffers[framebuffer]);
#endif

  },

  glGenFramebuffers__sig: 'vii',
  glGenFramebuffers__deps: ['_glGenObject'],
  glGenFramebuffers: function(n, ids) {
    __glGenObject(n, ids, 'createFramebuffer', GL.framebuffers
#if GL_ASSERTIONS
    , 'glGenFramebuffers'
#endif
      );
  },

  glDeleteFramebuffers__sig: 'vii',
  glDeleteFramebuffers: function(n, framebuffers) {
    for (var i = 0; i < n; ++i) {
      var id = {{{ makeGetValue('framebuffers', 'i*4', 'i32') }}};
      var framebuffer = GL.framebuffers[id];
      if (!framebuffer) continue; // GL spec: "glDeleteFramebuffers silently ignores 0s and names that do not correspond to existing framebuffer objects".
      GLctx.deleteFramebuffer(framebuffer);
      framebuffer.name = 0;
      GL.framebuffers[id] = null;
    }
  },

  glFramebufferRenderbuffer__sig: 'viiii',
  glFramebufferRenderbuffer: function(target, attachment, renderbuffertarget, renderbuffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.renderbuffers, renderbuffer, 'glFramebufferRenderbuffer', 'renderbuffer');
#endif
    GLctx.framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                       GL.renderbuffers[renderbuffer]);
  },

  glFramebufferTexture2D__sig: 'viiiii',
  glFramebufferTexture2D: function(target, attachment, textarget, texture, level) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.textures, texture, 'glFramebufferTexture2D', 'texture');
#endif
    GLctx.framebufferTexture2D(target, attachment, textarget,
                                    GL.textures[texture], level);
  },

  glGetFramebufferAttachmentParameteriv__sig: 'viiii',
  glGetFramebufferAttachmentParameteriv: function(target, attachment, pname, params) {
    var result = GLctx.getFramebufferAttachmentParameter(target, attachment, pname);
    if (result instanceof WebGLRenderbuffer ||
        result instanceof WebGLTexture) {
      result = result.name | 0;
    }
    {{{ makeSetValue('params', '0', 'result', 'i32') }}};
  },

  glIsFramebuffer__sig: 'ii',
  glIsFramebuffer: function(framebuffer) {
    var fb = GL.framebuffers[framebuffer];
    if (!fb) return 0;
    return GLctx.isFramebuffer(fb);
  },

  glGenVertexArrays__deps: ['_glGenObject'
#if LEGACY_GL_EMULATION
  , 'emulGlGenVertexArrays'
#endif
  ],
  glGenVertexArrays__sig: 'vii',
  glGenVertexArrays: function (n, arrays) {
#if LEGACY_GL_EMULATION
    _emulGlGenVertexArrays(n, arrays);
#else
#if GL_ASSERTIONS
    assert(GLctx['createVertexArray'], 'Must have WebGL2 or OES_vertex_array_object to use vao');
#endif
    __glGenObject(n, arrays, 'createVertexArray', GL.vaos
#if GL_ASSERTIONS
    , 'glGenVertexArrays'
#endif
      );
#endif
  },

#if LEGACY_GL_EMULATION
  glDeleteVertexArrays__deps: ['emulGlDeleteVertexArrays'],
#endif
  glDeleteVertexArrays__sig: 'vii',
  glDeleteVertexArrays: function(n, vaos) {
#if LEGACY_GL_EMULATION
    _emulGlDeleteVertexArrays(n, vaos);
#else
#if GL_ASSERTIONS
    assert(GLctx['deleteVertexArray'], 'Must have WebGL2 or OES_vertex_array_object to use vao');
#endif
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('vaos', 'i*4', 'i32') }}};
      GLctx['deleteVertexArray'](GL.vaos[id]);
      GL.vaos[id] = null;
    }
#endif
  },

#if LEGACY_GL_EMULATION
  glBindVertexArray__deps: ['emulGlBindVertexArray'],
#endif
  glBindVertexArray__sig: 'vi',
  glBindVertexArray: function(vao) {
#if LEGACY_GL_EMULATION
    _emulGlBindVertexArray(vao);
#else
#if GL_ASSERTIONS
    assert(GLctx['bindVertexArray'], 'Must have WebGL2 or OES_vertex_array_object to use vao');
#endif
    GLctx['bindVertexArray'](GL.vaos[vao]);
#endif
#if FULL_ES2 || LEGACY_GL_EMULATION
    var ibo = GLctx.getParameter(GLctx.ELEMENT_ARRAY_BUFFER_BINDING);
    GL.currElementArrayBuffer = ibo ? (ibo.name | 0) : 0;
#endif
  },

#if LEGACY_GL_EMULATION
  glIsVertexArray__deps: ['emulGlIsVertexArray'],
#endif
  glIsVertexArray__sig: 'ii',
  glIsVertexArray: function(array) {
#if LEGACY_GL_EMULATION
    return _emulGlIsVertexArray(array);
#else
#if GL_ASSERTIONS
    assert(GLctx['isVertexArray'], 'Must have WebGL2 or OES_vertex_array_object to use vao');
#endif

    var vao = GL.vaos[array];
    if (!vao) return 0;
    return GLctx['isVertexArray'](vao);
#endif
  },

#if !LEGACY_GL_EMULATION

  glVertexPointer: function(){ throw 'Legacy GL function (glVertexPointer) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },
  glMatrixMode: function(){ throw 'Legacy GL function (glMatrixMode) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },
  glBegin: function(){ throw 'Legacy GL function (glBegin) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },
  glLoadIdentity: function(){ throw 'Legacy GL function (glLoadIdentity) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },

#endif // LEGACY_GL_EMULATION

  // Open GLES1.1 vao compatibility (Could work w/o -s LEGACY_GL_EMULATION=1)

  glGenVertexArraysOES: 'glGenVertexArrays',
  glDeleteVertexArraysOES: 'glDeleteVertexArrays',
  glBindVertexArrayOES: 'glBindVertexArray',
  glIsVertexArrayOES: 'glIsVertexArray',

  // GLU

  gluPerspective: function(fov, aspect, near, far) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrix[GLImmediate.currentMatrix] =
      GLImmediate.matrixLib.mat4.perspective(fov, aspect, near, far,
                                               GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  gluLookAt: function(ex, ey, ez, cx, cy, cz, ux, uy, uz) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.lookAt(GLImmediate.matrix[GLImmediate.currentMatrix], [ex, ey, ez],
        [cx, cy, cz], [ux, uy, uz]);
  },

  gluProject: function(objX, objY, objZ, model, proj, view, winX, winY, winZ) {
    // The algorithm for this functions comes from Mesa

    var inVec = new Float32Array(4);
    var outVec = new Float32Array(4);
    GLImmediate.matrixLib.mat4.multiplyVec4({{{ makeHEAPView('F64', 'model', 'model+' + (16*8)) }}},
        [objX, objY, objZ, 1.0], outVec);
    GLImmediate.matrixLib.mat4.multiplyVec4({{{ makeHEAPView('F64', 'proj', 'proj+' + (16*8)) }}},
        outVec, inVec);
    if (inVec[3] == 0.0) {
      return 0 /* GL_FALSE */;
    }
    inVec[0] /= inVec[3];
    inVec[1] /= inVec[3];
    inVec[2] /= inVec[3];
    // Map x, y and z to range 0-1 */
    inVec[0] = inVec[0] * 0.5 + 0.5;
    inVec[1] = inVec[1] * 0.5 + 0.5;
    inVec[2] = inVec[2] * 0.5 + 0.5;
    // Map x, y to viewport
    inVec[0] = inVec[0] * {{{ makeGetValue('view', 2*4, 'i32') }}} + {{{ makeGetValue('view', 0*4, 'i32') }}};
    inVec[1] = inVec[1] * {{{ makeGetValue('view', 3*4, 'i32') }}} + {{{ makeGetValue('view', 1*4, 'i32') }}};

    {{{ makeSetValue('winX', '0', 'inVec[0]', 'double') }}};
    {{{ makeSetValue('winY', '0', 'inVec[1]', 'double') }}};
    {{{ makeSetValue('winZ', '0', 'inVec[2]', 'double') }}};

    return 1 /* GL_TRUE */;
  },

  gluUnProject: function(winX, winY, winZ, model, proj, view, objX, objY, objZ) {
    var result = GLImmediate.matrixLib.mat4.unproject([winX, winY, winZ],
        {{{ makeHEAPView('F64', 'model', 'model+' + (16*8)) }}},
        {{{ makeHEAPView('F64', 'proj', 'proj+' + (16*8)) }}},
        {{{ makeHEAPView('32', 'view', 'view+' + (4*4)) }}});

    if (result === null) {
      return 0 /* GL_FALSE */;
    }

    {{{ makeSetValue('objX', '0', 'result[0]', 'double') }}};
    {{{ makeSetValue('objY', '0', 'result[1]', 'double') }}};
    {{{ makeSetValue('objZ', '0', 'result[2]', 'double') }}};

    return 1 /* GL_TRUE */;
  },

  gluOrtho2D__deps: ['glOrtho'],
  gluOrtho2D: function(left, right, bottom, top) {
    _glOrtho(left, right, bottom, top, -1, 1);
  },

  // GLES2 emulation

  glVertexAttribPointer__sig: 'viiiiii',
  glVertexAttribPointer: function(index, size, type, normalized, stride, ptr) {
#if FULL_ES2
    var cb = GL.currentContext.clientBuffers[index];
#if GL_ASSERTIONS
    assert(cb, index);
#endif
    if (!GL.currArrayBuffer) {
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

  glEnableVertexAttribArray__sig: 'vi',
  glEnableVertexAttribArray: function(index) {
#if FULL_ES2
    var cb = GL.currentContext.clientBuffers[index];
#if GL_ASSERTIONS
    assert(cb, index);
#endif
    cb.enabled = true;
#endif
    GLctx.enableVertexAttribArray(index);
  },

  glDisableVertexAttribArray__sig: 'vi',
  glDisableVertexAttribArray: function(index) {
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
  glDrawArrays__sig: 'viii',
  glDrawArrays: function(mode, first, count) {
#if FULL_ES2
    // bind any client-side buffers
    GL.preDrawHandleClientVertexAttribBindings(first + count);
#endif

    GLctx.drawArrays(mode, first, count);

#if FULL_ES2
    GL.postDrawHandleClientVertexAttribBindings();
#endif
  },

  glDrawElements__sig: 'viiii',
  glDrawElements: function(mode, count, type, indices) {
#if FULL_ES2
    var buf;
    if (!GL.currElementArrayBuffer) {
      var size = GL.calcBufLength(1, type, 0, count);
      buf = GL.getTempIndexBuffer(size);
      GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, buf);
      GLctx.bufferSubData(GLctx.ELEMENT_ARRAY_BUFFER,
                               0,
                               HEAPU8.subarray(indices, indices + size));
      // the index is now 0
      indices = 0;
    }

    // bind any client-side buffers
    GL.preDrawHandleClientVertexAttribBindings(count);
#endif

    GLctx.drawElements(mode, count, type, indices);

#if FULL_ES2
    GL.postDrawHandleClientVertexAttribBindings(count);

    if (!GL.currElementArrayBuffer) {
      GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, null);
    }
#endif
  },
#endif // ~#if !LEGACY_GL_EMULATION

  glShaderBinary__sig: 'v',
  glShaderBinary: function() {
    GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
    err("GL_INVALID_ENUM in glShaderBinary: WebGL does not support binary shader formats! Calls to glShaderBinary always fail.");
#endif
  },

  glReleaseShaderCompiler__sig: 'v',
  glReleaseShaderCompiler: function() {
    // NOP (as allowed by GLES 2.0 spec)
  },

  glGetError__sig: 'i',
  glGetError: function() {
#if GL_TRACK_ERRORS
    var error = GLctx.getError() || GL.lastError;
    GL.lastError = 0/*GL_NO_ERROR*/;
    return error;
#else
    return GLctx.getError();
#endif
  },

  // ANGLE_instanced_arrays WebGL extension related functions (in core in WebGL 2)

  glVertexAttribDivisor__sig: 'vii',
  glVertexAttribDivisor: function(index, divisor) {
#if GL_ASSERTIONS
    assert(GLctx['vertexAttribDivisor'], 'Must have ANGLE_instanced_arrays extension or WebGL 2 to use WebGL instancing');
#endif
    GLctx['vertexAttribDivisor'](index, divisor);
  },

  glDrawArraysInstanced__sig: 'viiii',
  glDrawArraysInstanced: function(mode, first, count, primcount) {
#if GL_ASSERTIONS
    assert(GLctx['drawArraysInstanced'], 'Must have ANGLE_instanced_arrays extension or WebGL 2 to use WebGL instancing');
#endif
    GLctx['drawArraysInstanced'](mode, first, count, primcount);
  },

  glDrawElementsInstanced__sig: 'viiiii',
  glDrawElementsInstanced: function(mode, count, type, indices, primcount) {
#if GL_ASSERTIONS
    assert(GLctx['drawElementsInstanced'], 'Must have ANGLE_instanced_arrays extension or WebGL 2 to use WebGL instancing');
#endif
    GLctx['drawElementsInstanced'](mode, count, type, indices, primcount);
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


  glDrawBuffers__deps: ['_tempFixedLengthArray'],
  glDrawBuffers__sig: 'vii',
  glDrawBuffers: function(n, bufs) {
#if GL_ASSERTIONS
    assert(GLctx['drawBuffers'], 'Must have WebGL2 or WEBGL_draw_buffers extension to use drawBuffers');
#endif
#if GL_ASSERTIONS
    assert(n < __tempFixedLengthArray.length, 'Invalid count of numBuffers=' + n + ' passed to glDrawBuffers (that many draw buffer points do not exist in GL)');
#endif

    var bufArray = __tempFixedLengthArray[n];
    for (var i = 0; i < n; i++) {
      bufArray[i] = {{{ makeGetValue('bufs', 'i*4', 'i32') }}};
    }

    GLctx['drawBuffers'](bufArray);
  },

  // OpenGL ES 2.0 draw buffer extensions compatibility

  glDrawBuffersEXT: 'glDrawBuffers',
  glDrawBuffersWEBGL: 'glDrawBuffers',

  // passthrough functions with GLboolean parameters

  glColorMask__sig: 'viiii',
  glColorMask: function(red, green, blue, alpha) {
    GLctx.colorMask(!!red, !!green, !!blue, !!alpha);
  },

  glDepthMask__sig: 'vi',
  glDepthMask: function(flag) {
    GLctx.depthMask(!!flag);
  },

  glSampleCoverage__sig: 'vii',
  glSampleCoverage: function(value, invert) {
    GLctx.sampleCoverage(value, !!invert);
  },

  // As a small peculiarity, we currently allow building with -s FULL_ES3=1 to emulate client side arrays,
  // but without targeting WebGL 2, so this FULL_ES3 block is in library_webgl.js instead of library_webgl2.js
#if FULL_ES3
  $emscriptenWebGLGetBufferBinding: function(target) {
    switch(target) {
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

  $emscriptenWebGLValidateMapBufferTarget: function(target) {
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

  glMapBufferRange__sig: 'iiiii',
  glMapBufferRange__deps: ['$emscriptenWebGLGetBufferBinding', '$emscriptenWebGLValidateMapBufferTarget'],
  glMapBufferRange: function(target, offset, length, access) {
    if (access != 0x1A && access != 0xA) {
      err("glMapBufferRange is only supported when access is MAP_WRITE|INVALIDATE_BUFFER");
      return 0;
    }

    if (!emscriptenWebGLValidateMapBufferTarget(target)) {
      GL.recordError(0x0500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glMapBufferRange');
      return 0;
    }

    var mem = _malloc(length);
    if (!mem) return 0;

    GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)] = {
      offset: offset,
      length: length,
      mem: mem,
      access: access,
    };
    return mem;
  },

  glGetBufferPointerv__sig: 'viii',
  glGetBufferPointerv__deps: ['$emscriptenWebGLGetBufferBinding'],
  glGetBufferPointerv: function(target, pname, params) {
    if (pname == 0x88BD/*GL_BUFFER_MAP_POINTER*/) {
      var ptr = 0;
      var mappedBuffer = GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];
      if (mappedBuffer) {
        ptr = mappedBuffer.mem;
      }
      {{{ makeSetValue('params', '0', 'ptr', 'i32') }}};
    } else {
      GL.recordError(0x0500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glGetBufferPointerv');
    }
  },

  glFlushMappedBufferRange__sig: 'viii',
  glFlushMappedBufferRange__deps: ['$emscriptenWebGLGetBufferBinding', '$emscriptenWebGLValidateMapBufferTarget'],
  glFlushMappedBufferRange: function(target, offset, length) {
    if (!emscriptenWebGLValidateMapBufferTarget(target)) {
      GL.recordError(0x0500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glFlushMappedBufferRange');
      return;
    }

    var mapping = GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];
    if (!mapping) {
      GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
      Module.printError('buffer was never mapped in glFlushMappedBufferRange');
      return;
    }

    if (!(mapping.access & 0x10)) {
      GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
      Module.printError('buffer was not mapped with GL_MAP_FLUSH_EXPLICIT_BIT in glFlushMappedBufferRange');
      return;
    }
    if (offset < 0 || length < 0 || offset + length > mapping.length) {
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      Module.printError('invalid range in glFlushMappedBufferRange');
      return;
    }

    GLctx.bufferSubData(
      target,
      mapping.offset,
      HEAPU8.subarray(mapping.mem + offset, mapping.mem + offset + length));
  },

  glUnmapBuffer__sig: 'ii',
  glUnmapBuffer__deps: ['$emscriptenWebGLGetBufferBinding', '$emscriptenWebGLValidateMapBufferTarget'],
  glUnmapBuffer: function(target) {
    if (!emscriptenWebGLValidateMapBufferTarget(target)) {
      GL.recordError(0x0500/*GL_INVALID_ENUM*/);
      err('GL_INVALID_ENUM in glUnmapBuffer');
      return 0;
    }

    var buffer = emscriptenWebGLGetBufferBinding(target);
    var mapping = GL.mappedBuffers[buffer];
    if (!mapping) {
      GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
      Module.printError('buffer was never mapped in glUnmapBuffer');
      return 0;
    }
    GL.mappedBuffers[buffer] = null;

    if (!(mapping.access & 0x10)) /* GL_MAP_FLUSH_EXPLICIT_BIT */
      if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
        GLctx.bufferSubData(target, mapping.offset, HEAPU8, mapping.mem, mapping.length);
      } else {
        GLctx.bufferSubData(target, mapping.offset, HEAPU8.subarray(mapping.mem, mapping.mem+mapping.length));
      }
    _free(mapping.mem);
    return 1;
  },
#endif

  // signatures of simple pass-through functions, see later

  glActiveTexture__sig: 'vi',
  glCheckFramebufferStatus__sig: 'ii',
  glRenderbufferStorage__sig: 'viiii',
  glClearStencil__sig: 'vi',
  glStencilFunc__sig: 'viii',
  glLineWidth__sig: 'vi',
  glBlendEquation__sig: 'vi',
  glBlendEquationSeparate__sig: 'vii',
  glVertexAttrib1f__sig: 'vii',
  glVertexAttrib2f__sig: 'viii',
  glVertexAttrib3f__sig: 'viiii',
  glVertexAttrib4f__sig: 'viiiii',
  glCullFace__sig: 'vi',
  glBlendFunc__sig: 'vii',
  glBlendFuncSeparate__sig: 'viiii',
  glBlendColor__sig: 'vffff',
  glPolygonOffset__sig: 'vii',
  glStencilOp__sig: 'viii',
  glStencilOpSeparate__sig: 'viiii',
  glGenerateMipmap__sig: 'vi',
  glHint__sig: 'vii',
  glViewport__sig: 'viiii',
  glDepthFunc__sig: 'vi',
  glStencilMask__sig: 'vi',
  glStencilMaskSeparate__sig: 'vii',
  glClearDepthf__sig: 'vi',
  glFinish__sig: 'v',
  glFlush__sig: 'v',
  glClearColor__sig: 'viiii',
  glIsEnabled__sig: 'ii',
  glFrontFace__sig: 'vi',
};

// Simple pass-through functions. Starred ones have return values. [X] ones have X in the C name but not in the JS name
var glFuncs = [[0, 'finish flush'],
 [1, 'clearDepth clearDepth[f] depthFunc enable disable frontFace cullFace clear lineWidth clearStencil stencilMask checkFramebufferStatus* generateMipmap activeTexture blendEquation isEnabled*'],
 [2, 'blendFunc blendEquationSeparate depthRange depthRange[f] stencilMaskSeparate hint polygonOffset vertexAttrib1f'],
 [3, 'texParameteri texParameterf vertexAttrib2f stencilFunc stencilOp'],
 [4, 'viewport clearColor scissor vertexAttrib3f renderbufferStorage blendFuncSeparate blendColor stencilFuncSeparate stencilOpSeparate'],
 [5, 'vertexAttrib4f'],
 [6, ''],
 [7, ''],
 [8, 'copyTexImage2D copyTexSubImage2D'],
 [9, ''],
 [10, '']];

function createGLPassthroughFunctions(lib, funcs) {
  funcs.forEach(function(data) {
    var num = data[0];
    var names = data[1];
    var args = range(num).map(function(i) { return 'x' + i }).join(', ');
    var plainStub = '(function(' + args + ') { GLctx[\'NAME\'](' + args + ') })';
    var returnStub = '(function(' + args + ') { return GLctx[\'NAME\'](' + args + ') })';
    var sigEnd = range(num).map(function() { return 'i' }).join('');
    names.split(' ').forEach(function(name) {
      if (name.length == 0) return;
      var stub = plainStub;
      var sig;
      if (name[name.length-1] == '*') {
        name = name.substr(0, name.length-1);
        stub = returnStub;
        sig = 'i' + sigEnd;
      } else {
        sig = 'v' + sigEnd;
      }
      var cName = name;
      if (name.indexOf('[') >= 0) {
        cName = name.replace('[', '').replace(']', '');
        name = cName.substr(0, cName.length-1);
      }
      var cName = 'gl' + cName[0].toUpperCase() + cName.substr(1);
      assert(!(cName in lib), "Cannot reimplement the existing function " + cName);
      lib[cName] = eval(stub.replace('NAME', name));
      if (!lib[cName + '__sig']) lib[cName + '__sig'] = sig;
    });
  });
}

createGLPassthroughFunctions(LibraryGL, glFuncs);

autoAddDeps(LibraryGL, '$GL');

function copyLibEntry(lib, a, b) {
  lib[a] = lib[b];
  lib[a + '__postset'] = lib[b + '__postset'];
  lib[a + '__proxy'] = lib[b + '__proxy'];
  lib[a + '__sig'] = lib[b + '__sig'];
  lib[a + '__asm'] = lib[b + '__asm'];
  lib[a + '__deps'] = (lib[b + '__deps'] || []).slice(0);
}

function recordGLProcAddressGet(lib) {
  // GL proc address retrieval - allow access through glX and emscripten_glX, to allow name collisions with user-implemented things having the same name (see gl.c)
  keys(lib).forEach(function(x) {
    if (x.substr(-7) == '__proxy' || x.substr(-6) == '__deps' || x.substr(-9) == '__postset' || x.substr(-5) == '__sig' || x.substr(-5) == '__asm' || x.substr(0, 2) != 'gl') return;
    while (typeof lib[x] === 'string') {
      // resolve aliases right here, simpler for fastcomp
      copyLibEntry(lib, x, lib[x]);
    }
    var y = 'emscripten_' + x;
    lib[x + '__deps'] = (lib[x + '__deps'] || []).map(function(dep) {
      // prefix dependencies as well
      if (typeof dep === 'string' && dep[0] == 'g' && dep[1] == 'l' && lib[dep]) {
        var orig = dep;
        dep = 'emscripten_' + dep;
        var fixed = lib[x].toString().replace(new RegExp('_' + orig + '\\(', 'g'), '_' + dep + '(');
        // `function` is 8 characters, add space and an explicit name after if there isn't one already
        if (fixed.startsWith('function(') || fixed.startsWith('function (')) {
          fixed = fixed.substr(0, 8) + ' _' + y + fixed.substr(8);
        }
        lib[x] = eval('(function() { return ' + fixed + ' })()');
      }
      return dep;
    });
    // copy it
    copyLibEntry(lib, y, x);
  });
}

recordGLProcAddressGet(LibraryGL);

// Final merge
mergeInto(LibraryManager.library, LibraryGL);
