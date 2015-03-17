/*
 * GL support. See https://github.com/kripken/emscripten/wiki/OpenGL-support
 * for current status.
 */

var LibraryGL = {
  $GL__postset: 'var GLctx; GL.init()',
  $GL: {
#if GL_DEBUG
    debug: true,
#endif

    counter: 1, // 0 is reserved as 'null' in gl
    lastError: 0,
    buffers: [],
    mappedBuffers: {},
    programs: [],
    framebuffers: [],
    renderbuffers: [],
    textures: [],
    uniforms: [],
    shaders: [],
    vaos: [],
    contexts: [],
    currentContext: null,
#if USE_WEBGL2
    queries: [],
    samplers: [],
    transformFeedbacks: [],
#endif

#if USES_GL_EMULATION
    currArrayBuffer: 0,
    currElementArrayBuffer: 0,
#endif

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

    programInfos: {}, // Stores additional information needed for each shader program. Each entry is of form:
    /* { uniforms: {}, // Maps ints back to the opaque WebGLUniformLocation objects.
         maxUniformLength: int, // Cached in order to implement glGetProgramiv(GL_ACTIVE_UNIFORM_MAX_LENGTH)
         maxAttributeLength: int // Cached in order to implement glGetProgramiv(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH)
       } */

    stringCache: {},

    packAlignment: 4,   // default alignment is 4 bytes
    unpackAlignment: 4, // default alignment is 4 bytes

    init: function() {
#if USES_GL_EMULATION
      GL.createLog2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);
#endif
      GL.miniTempBuffer = new Float32Array(GL.MINI_TEMP_BUFFER_SIZE);
      for (var i = 0; i < GL.MINI_TEMP_BUFFER_SIZE; i++) {
        GL.miniTempBufferViews[i] = GL.miniTempBuffer.subarray(0, i+1);
      }
    },

    // Records a GL error condition that occurred, stored until user calls glGetError() to fetch it. As per GLES2 spec, only the first error 
    // is remembered, and subsequent errors are discarded until the user has cleared the stored error by a call to glGetError().
    recordError: function recordError(errorCode) {
      if (!GL.lastError) {
        GL.lastError = errorCode;
      }
    },
    // Get a new ID for a texture/buffer/etc., while keeping the table dense and fast. Creation is fairly rare so it is worth optimizing lookups later.
    getNewId: function(table) {
      var ret = GL.counter++;
      for (var i = table.length; i < ret; i++) {
        table[i] = null;
      }
      return ret;
    },

    // Mini temp buffer
    MINI_TEMP_BUFFER_SIZE: 16,
    miniTempBuffer: null,
    miniTempBufferViews: [0], // index i has the view of size i+1

#if USES_GL_EMULATION
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
      for(var i = 1; i <= maxValue; ++i) {
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
      for(var i = 0; i <= largestIndex; ++i) {
        context.tempIndexBuffers[i] = null; // Created on-demand
        context.tempVertexBufferCounters1[i] = context.tempVertexBufferCounters2[i] = 0;
        var ringbufferLength = GL.numTempVertexBuffersPerSize;
        context.tempVertexBuffers1[i] = [];
        context.tempVertexBuffers2[i] = [];
        var ringbuffer1 = context.tempVertexBuffers1[i];
        var ringbuffer2 = context.tempVertexBuffers2[i];
        ringbuffer1.length = ringbuffer2.length = ringbufferLength;
        for(var j = 0; j < ringbufferLength; ++j) {
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
      for(var i = 0; i <= largestIndex; ++i) {
        GL.currentContext.tempVertexBufferCounters1[i] = 0;
      }
    },
#endif

#if LEGACY_GL_EMULATION
    // Find a token in a shader source string
    findToken: function(source, token) {
      function isIdentChar(ch) {
        if (ch >= 48 && ch <= 57) // 0-9
          return true;
        if (ch >= 65 && ch <= 90) // A-Z
          return true;
        if (ch >= 97 && ch <= 122) // a-z
          return true;
        return false;
      }
      var i = -1;
      do {
        i = source.indexOf(token, i + 1);
        if (i < 0) {
          break;
        }
        if (i > 0 && isIdentChar(source[i - 1])) {
          continue;
        }
        i += token.length;
        if (i < source.length - 1 && isIdentChar(source[i + 1])) {
          continue;
        }
        return true;
      } while (true);
      return false;
    },
#endif

    getSource: function(shader, count, string, length) {
      var source = '';
      for (var i = 0; i < count; ++i) {
        var frag;
        if (length) {
          var len = {{{ makeGetValue('length', 'i*4', 'i32') }}};
          if (len < 0) {
            frag = Pointer_stringify({{{ makeGetValue('string', 'i*4', 'i32') }}});
          } else {
            frag = Pointer_stringify({{{ makeGetValue('string', 'i*4', 'i32') }}}, len);
          }
        } else {
          frag = Pointer_stringify({{{ makeGetValue('string', 'i*4', 'i32') }}});
        }
        source += frag;
      }
#if LEGACY_GL_EMULATION
      // Let's see if we need to enable the standard derivatives extension
      type = GLctx.getShaderParameter(GL.shaders[shader], 0x8B4F /* GL_SHADER_TYPE */);
      if (type == 0x8B30 /* GL_FRAGMENT_SHADER */) {
        if (GL.findToken(source, "dFdx") ||
            GL.findToken(source, "dFdy") ||
            GL.findToken(source, "fwidth")) {
          source = "#extension GL_OES_standard_derivatives : enable\n" + source;
          var extension = GLctx.getExtension("OES_standard_derivatives");
#if GL_DEBUG
          if (!extension) {
            Module.printErr("Shader attempts to use the standard derivatives extension which is not available.");
          }
#endif
        }
      }
#endif
      return source;
    },

    computeImageSize: function(width, height, sizePerPixel, alignment) {
      function roundedToNextMultipleOf(x, y) {
        return Math.floor((x + y - 1) / y) * y
      }
      var plainRowSize = width * sizePerPixel;
      var alignedRowSize = roundedToNextMultipleOf(plainRowSize, alignment);
      return (height <= 0) ? 0 :
               ((height - 1) * alignedRowSize + plainRowSize);
    },

    get: function(name_, p, type) {
      // Guard against user passing a null pointer.
      // Note that GLES2 spec does not say anything about how passing a null pointer should be treated.
      // Testing on desktop core GL 3, the application crashes on glGetIntegerv to a null pointer, but
      // better to report an error instead of doing anything random.
      if (!p) {
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_VALUE in glGet' + type + 'v(name=' + name_ + ': Function called with null out pointer!');
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
          if (type !== 'Integer') {
            GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
            Module.printErr('GL_INVALID_ENUM in glGet' + type + 'v(GL_SHADER_BINARY_FORMATS): Invalid parameter type!');
#endif
          }
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
          ret = formats.length;
          break;
        case 0x8B9A: // GL_IMPLEMENTATION_COLOR_READ_TYPE
          ret = 0x1401; // GL_UNSIGNED_BYTE
          break;
        case 0x8B9B: // GL_IMPLEMENTATION_COLOR_READ_FORMAT
          ret = 0x1908; // GL_RGBA
          break;
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
            Module.printErr('GL_INVALID_ENUM in glGet' + type + 'v(' + name_ + ') on a name which returns a string!');
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
                case 0x8514: { // TEXTURE_BINDING_CUBE_MAP
                  ret = 0;
                  break;
                }
                default: {
                  GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
                  Module.printErr('GL_INVALID_ENUM in glGet' + type + 'v(' + name_ + ') and it returns null!');
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
                  case 'Integer': {{{ makeSetValue('p', 'i*4', 'result[i]',     'i32') }}};   break;
                  case 'Float':   {{{ makeSetValue('p', 'i*4', 'result[i]',     'float') }}}; break;
                  case 'Boolean': {{{ makeSetValue('p', 'i',   'result[i] ? 1 : 0', 'i8') }}};    break;
                  default: throw 'internal glGet error, bad type: ' + type;
                }
              }
              return;
            } else if (result instanceof WebGLBuffer ||
                       result instanceof WebGLProgram ||
                       result instanceof WebGLFramebuffer ||
                       result instanceof WebGLRenderbuffer ||
                       result instanceof WebGLTexture) {
              ret = result.name | 0;
            } else {
              GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
              Module.printErr('GL_INVALID_ENUM in glGet' + type + 'v: Unknown object returned from WebGL getParameter(' + name_ + ')!');
#endif
              return;
            }
            break;
          default:
            GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
            Module.printErr('GL_INVALID_ENUM in glGetIntegerv: Native code calling glGet' + type + 'v(' + name_ + ') and it returns ' + result + ' of type ' + typeof(result) + '!');
#endif
            return;
        }
      }

      switch (type) {
        case 'Integer': {{{ makeSetValue('p', '0', 'ret', 'i32') }}};    break;
        case 'Float':   {{{ makeSetValue('p', '0', 'ret', 'float') }}};  break;
        case 'Boolean': {{{ makeSetValue('p', '0', 'ret ? 1 : 0', 'i8') }}}; break;
        default: throw 'internal glGet error, bad type: ' + type;
      }
    },  

    getTexPixelData: function(type, format, width, height, pixels, internalFormat) {
      var sizePerPixel;
      var numChannels;
      switch(format) {
        case 0x1906 /* GL_ALPHA */:
        case 0x1909 /* GL_LUMINANCE */:
        case 0x1902 /* GL_DEPTH_COMPONENT */:
        case 0x1903 /* GL_RED */:
          numChannels = 1;
          break;
        case 0x190A /* GL_LUMINANCE_ALPHA */:
        case 0x8227 /* GL_RG */:
          numChannels = 2;
          break;
        case 0x1907 /* GL_RGB */:
          numChannels = 3;
          break;
        case 0x1908 /* GL_RGBA */:
          numChannels = 4;
          break;
        default:
          GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
          Module.printErr('GL_INVALID_ENUM due to unknown format in getTexPixelData, type: ' + type + ', format: ' + format);
#endif
          return {
            pixels: null,
            internalFormat: 0x0
          };
      }
      switch (type) {
        case 0x1401 /* GL_UNSIGNED_BYTE */:
          sizePerPixel = numChannels*1;
          break;
        case 0x1403 /* GL_UNSIGNED_SHORT */:
        case 0x8D61 /* GL_HALF_FLOAT_OES */:
          sizePerPixel = numChannels*2;
          break;
        case 0x1405 /* GL_UNSIGNED_INT */:
        case 0x1406 /* GL_FLOAT */:
          sizePerPixel = numChannels*4;
          break;
        case 0x84FA /* UNSIGNED_INT_24_8_WEBGL */:
          sizePerPixel = 4;
          break;
        case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
        case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
        case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
          sizePerPixel = 2;
          break;
        default:
          GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
          Module.printErr('GL_INVALID_ENUM in glTex[Sub]Image/glReadPixels, type: ' + type + ', format: ' + format);
#endif
          return {
            pixels: null,
            internalFormat: 0x0
          };
      }
      var bytes = GL.computeImageSize(width, height, sizePerPixel, GL.unpackAlignment);
      if (type == 0x1401 /* GL_UNSIGNED_BYTE */) {
        pixels = {{{ makeHEAPView('U8', 'pixels', 'pixels+bytes') }}};
      } else if (type == 0x1406 /* GL_FLOAT */) {
        pixels = {{{ makeHEAPView('F32', 'pixels', 'pixels+bytes') }}};
      } else if (type == 0x1405 /* GL_UNSIGNED_INT */ || type == 0x84FA /* UNSIGNED_INT_24_8_WEBGL */) {
        pixels = {{{ makeHEAPView('U32', 'pixels', 'pixels+bytes') }}};
      } else {
        pixels = {{{ makeHEAPView('U16', 'pixels', 'pixels+bytes') }}};
      }
      return {
        pixels: pixels,
        internalFormat: internalFormat
      };
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
        GLctx.vertexAttribPointer(i, cb.size, cb.type, cb.normalized, cb.stride, 0);
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

    validateBufferTarget: function(target) {
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
    
    // Returns the context handle to the new context.
    createContext: function(canvas, webGLContextAttributes) {
#if !USE_TYPED_ARRAYS
      Module.print('(USE_TYPED_ARRAYS needs to be enabled for WebGL)');
      return null;
#endif
      if (typeof webGLContextAttributes.majorVersion === 'undefined' && typeof webGLContextAttributes.minorVersion === 'undefined') {
#if USE_WEBGL2
        webGLContextAttributes.majorVersion = 2;
#else
        webGLContextAttributes.majorVersion = 1;
#endif
        webGLContextAttributes.minorVersion = 0;
      }
      var ctx;
      var errorInfo = '?';
      function onContextCreationError(event) {
        errorInfo = event.statusMessage || errorInfo;
      }
      try {
        canvas.addEventListener('webglcontextcreationerror', onContextCreationError, false);
        try {
          if (webGLContextAttributes.majorVersion == 1 && webGLContextAttributes.minorVersion == 0) {
            ctx = canvas.getContext("webgl", webGLContextAttributes) || canvas.getContext("experimental-webgl", webGLContextAttributes);
          } else if (webGLContextAttributes.majorVersion == 2 && webGLContextAttributes.minorVersion == 0) {
            ctx = canvas.getContext("webgl2", webGLContextAttributes) || canvas.getContext("experimental-webgl2", webGLContextAttributes);
          } else {
            throw 'Unsupported WebGL context version ' + majorVersion + '.' + minorVersion + '!'
          }
        } finally {
          canvas.removeEventListener('webglcontextcreationerror', onContextCreationError, false);
        }
        if (!ctx) throw ':(';
      } catch (e) {
        Module.print('Could not create canvas: ' + [errorInfo, e, JSON.stringify(webGLContextAttributes)]);
        return 0;
      }
#if GL_DEBUG
      function wrapDebugGL(ctx) {

        var printObjectList = [];

        function prettyPrint(arg) {
          if (typeof arg == 'undefined') return '!UNDEFINED!';
          if (typeof arg == 'boolean') arg = arg + 0;
          if (!arg) return arg;
          var index = printObjectList.indexOf(arg);
          if (index >= 0) return '<' + arg + '|'; // + index + '>';
          if (arg.toString() == '[object HTMLImageElement]') {
            return arg + '\n\n';
          }
          if (arg.byteLength) {
            return '{' + Array.prototype.slice.call(arg, 0, Math.min(arg.length, 400)) + '}'; // Useful for correct arrays, less so for compiled arrays, see the code below for that
            var buf = new ArrayBuffer(32);
            var i8buf = new Int8Array(buf);
            var i16buf = new Int16Array(buf);
            var f32buf = new Float32Array(buf);
            switch(arg.toString()) {
              case '[object Uint8Array]':
                i8buf.set(arg.subarray(0, 32));
                break;
              case '[object Float32Array]':
                f32buf.set(arg.subarray(0, 5));
                break;
              case '[object Uint16Array]':
                i16buf.set(arg.subarray(0, 16));
                break;
              default:
                alert('unknown array for debugging: ' + arg);
                throw 'see alert';
            }
            var ret = '{' + arg.byteLength + ':\n';
            var arr = Array.prototype.slice.call(i8buf);
            ret += 'i8:' + arr.toString().replace(/,/g, ',') + '\n';
            arr = Array.prototype.slice.call(f32buf, 0, 8);
            ret += 'f32:' + arr.toString().replace(/,/g, ',') + '}';
            return ret;
          }
          if (typeof arg == 'object') {
            printObjectList.push(arg);
            return '<' + arg + '|'; // + (printObjectList.length-1) + '>';
          }
          if (typeof arg == 'number') {
            if (arg > 0) return '0x' + arg.toString(16) + ' (' + arg + ')';
          }
          return arg;
        }

        var wrapper = {};
        for (var prop in ctx) {
          (function(prop) {
            switch (typeof ctx[prop]) {
              case 'function': {
                wrapper[prop] = function gl_wrapper() {
                  var printArgs = Array.prototype.slice.call(arguments).map(prettyPrint);
                  dump('[gl_f:' + prop + ':' + printArgs + ']\n');
                  var ret = ctx[prop].apply(ctx, arguments);
                  if (typeof ret != 'undefined') {
                    dump('[     gl:' + prop + ':return:' + prettyPrint(ret) + ']\n');
                  }
                  return ret;
                }
                break;
              }
              case 'number': case 'string': {
                wrapper.__defineGetter__(prop, function() {
                  //dump('[gl_g:' + prop + ':' + ctx[prop] + ']\n');
                  return ctx[prop];
                });
                wrapper.__defineSetter__(prop, function(value) {
                  dump('[gl_s:' + prop + ':' + value + ']\n');
                  ctx[prop] = value;
                });
                break;
              }
            }
          })(prop);
        }
        return wrapper;
      }
#endif
      // possible GL_DEBUG entry point: ctx = wrapDebugGL(ctx);

      if (!ctx) return 0;
      return GL.registerContext(ctx, webGLContextAttributes);
    },

    registerContext: function(ctx, webGLContextAttributes) {
      var handle = GL.getNewId(GL.contexts);
      var context = {
        handle: handle,
        version: webGLContextAttributes.majorVersion,
        GLctx: ctx
      };
      // Store the created context object so that we can access the context given a canvas without having to pass the parameters again.
      if (ctx.canvas) ctx.canvas.GLctxObject = context;
      GL.contexts[handle] = context;
      if (typeof webGLContextAttributes['enableExtensionsByDefault'] === 'undefined' || webGLContextAttributes.enableExtensionsByDefault) {
        GL.initExtensions(context);
      }
      return handle;
    },

    makeContextCurrent: function(contextHandle) {
      var context = GL.contexts[contextHandle];
      if (!context) return false;
      GLctx = Module.ctx = context.GLctx; // Active WebGL context object.
      GL.currentContext = context; // Active Emscripten GL layer context object.
      return true;
    },

    getContext: function(contextHandle) {
      return GL.contexts[contextHandle];
    },

    deleteContext: function(contextHandle) {
      if (GL.currentContext === GL.contexts[contextHandle]) GL.currentContext = null;
      if (typeof JSEvents === 'object') JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas); // Release all JS event handlers on the DOM element that the GL context is associated with since the context is now deleted.
      if (GL.contexts[contextHandle] && GL.contexts[contextHandle].GLctx.canvas) GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined; // Make sure the canvas object no longer refers to the context object so there are no GC surprises.
      GL.contexts[contextHandle] = null;
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

      context.maxVertexAttribs = GLctx.getParameter(GLctx.MAX_VERTEX_ATTRIBS);
#if FULL_ES2
      context.clientBuffers = [];
      for (var i = 0; i < context.maxVertexAttribs; i++) {
        context.clientBuffers[i] = { enabled: false, clientside: false, size: 0, type: 0, normalized: 0, stride: 0, ptr: 0 };
      }

      GL.generateTempBuffers(false, context);
#endif

      // Detect the presence of a few extensions manually, this GL interop layer itself will need to know if they exist. 
      context.compressionExt = GLctx.getExtension('WEBGL_compressed_texture_s3tc') ||
                          GLctx.getExtension('MOZ_WEBGL_compressed_texture_s3tc') ||
                          GLctx.getExtension('WEBKIT_WEBGL_compressed_texture_s3tc');

      context.anisotropicExt = GLctx.getExtension('EXT_texture_filter_anisotropic') ||
                          GLctx.getExtension('MOZ_EXT_texture_filter_anisotropic') ||
                          GLctx.getExtension('WEBKIT_EXT_texture_filter_anisotropic');

      context.floatExt = GLctx.getExtension('OES_texture_float');

      // Extension available from Firefox 26 and Google Chrome 30
      context.instancedArraysExt = GLctx.getExtension('ANGLE_instanced_arrays');
      
      // Extension available from Firefox 25 and WebKit
      context.vaoExt = GLctx.getExtension('OES_vertex_array_object');

      if (context.version === 2) {
        // drawBuffers is available in WebGL2 by default.
        context.drawBuffersExt = function(n, bufs) {
          GLctx.drawBuffers(n, bufs);
        };
      } else {
        var ext = GLctx.getExtension('WEBGL_draw_buffers');
        if (ext) {
          context.drawBuffersExt = function(n, bufs) {
            ext.drawBuffersWEBGL(n, bufs);
          };
        }
      }

      // These are the 'safe' feature-enabling extensions that don't add any performance impact related to e.g. debugging, and
      // should be enabled by default so that client GLES2/GL code will not need to go through extra hoops to get its stuff working.
      // As new extensions are ratified at http://www.khronos.org/registry/webgl/extensions/ , feel free to add your new extensions
      // here, as long as they don't produce a performance impact for users that might not be using those extensions.
      // E.g. debugging-related extensions should probably be off by default.
      var automaticallyEnabledExtensions = [ "OES_texture_float", "OES_texture_half_float", "OES_standard_derivatives",
                                             "OES_vertex_array_object", "WEBGL_compressed_texture_s3tc", "WEBGL_depth_texture",
                                             "OES_element_index_uint", "EXT_texture_filter_anisotropic", "ANGLE_instanced_arrays",
                                             "OES_texture_float_linear", "OES_texture_half_float_linear", "WEBGL_compressed_texture_atc",
                                             "WEBGL_compressed_texture_pvrtc", "EXT_color_buffer_half_float", "WEBGL_color_buffer_float",
                                             "EXT_frag_depth", "EXT_sRGB", "WEBGL_draw_buffers", "WEBGL_shared_resources",
                                             "EXT_shader_texture_lod" ];

      function shouldEnableAutomatically(extension) {
        var ret = false;
        automaticallyEnabledExtensions.forEach(function(include) {
          if (ext.indexOf(include) != -1) {
            ret = true;
          }
        });
        return ret;
      }

      var exts = GLctx.getSupportedExtensions();
      if (exts && exts.length > 0) {
        GLctx.getSupportedExtensions().forEach(function(ext) {
          ext = ext.replace('MOZ_', '').replace('WEBKIT_', '');
          if (automaticallyEnabledExtensions.indexOf(ext) != -1) {
            GLctx.getExtension(ext); // Calling .getExtension enables that extension permanently, no need to store the return value to be enabled.
          }
        });
      }
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
      GL.programInfos[program] = {
        uniforms: {},
        maxUniformLength: 0, // This is eagerly computed below, since we already enumerate all uniforms anyway.
        maxAttributeLength: -1 // This is lazily computed and cached, computed when/if first asked, "-1" meaning not computed yet.
      };

      var ptable = GL.programInfos[program];
      var utable = ptable.uniforms;
      // A program's uniform table maps the string name of an uniform to an integer location of that uniform.
      // The global GL.uniforms map maps integer locations to WebGLUniformLocations.
      var numUniforms = GLctx.getProgramParameter(p, GLctx.ACTIVE_UNIFORMS);
      for (var i = 0; i < numUniforms; ++i) {
        var u = GLctx.getActiveUniform(p, i);

        var name = u.name;
        ptable.maxUniformLength = Math.max(ptable.maxUniformLength, name.length+1);

        // Strip off any trailing array specifier we might have got, e.g. "[0]".
        if (name.indexOf(']', name.length-1) !== -1) {
          var ls = name.lastIndexOf('[');
          name = name.slice(0, ls);
        }

        // Optimize memory usage slightly: If we have an array of uniforms, e.g. 'vec3 colors[3];', then 
        // only store the string 'colors' in utable, and 'colors[0]', 'colors[1]' and 'colors[2]' will be parsed as 'colors'+i.
        // Note that for the GL.uniforms table, we still need to fetch the all WebGLUniformLocations for all the indices.
        var loc = GLctx.getUniformLocation(p, name);
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
  },

  glPixelStorei__sig: 'vii',
  glPixelStorei: function(pname, param) {
    if (pname == 0x0D05 /* GL_PACK_ALIGNMENT */) {
      GL.packAlignment = param;
    } else if (pname == 0x0cf5 /* GL_UNPACK_ALIGNMENT */) {
      GL.unpackAlignment = param;
    }
    GLctx.pixelStorei(pname, param);
  },

  glGetString__sig: 'ii',
  glGetString: function(name_) {
    if (GL.stringCache[name_]) return GL.stringCache[name_];
    var ret; 
    switch(name_) {
      case 0x1F00 /* GL_VENDOR */:
      case 0x1F01 /* GL_RENDERER */:
      case 0x1F02 /* GL_VERSION */:
        ret = allocate(intArrayFromString(GLctx.getParameter(name_)), 'i8', ALLOC_NORMAL);
        break;
      case 0x1F03 /* GL_EXTENSIONS */:
        var exts = GLctx.getSupportedExtensions();
        var gl_exts = [];
        for (var i in exts) {
          gl_exts.push(exts[i]);
          gl_exts.push("GL_" + exts[i]);
        }
        ret = allocate(intArrayFromString(gl_exts.join(' ')), 'i8', ALLOC_NORMAL);
        break;
      case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
        ret = allocate(intArrayFromString('OpenGL ES GLSL 1.00 (WebGL)'), 'i8', ALLOC_NORMAL);
        break;
      default:
        GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_ENUM in glGetString: Unknown parameter ' + name_ + '!');
#endif
        return 0;
    }
    GL.stringCache[name_] = ret;
    return ret;
  },

  glGetIntegerv__sig: 'vii',
  glGetIntegerv: function(name_, p) {
    return GL.get(name_, p, 'Integer');
  },

  glGetFloatv__sig: 'vii',
  glGetFloatv: function(name_, p) {
    return GL.get(name_, p, 'Float');
  },

  glGetBooleanv__sig: 'vii',
  glGetBooleanv: function(name_, p) {
    return GL.get(name_, p, 'Boolean');
  },

  glGenTextures__sig: 'vii',
  glGenTextures: function(n, textures) {
    for (var i = 0; i < n; i++) {
      var texture = GLctx.createTexture();
      if (!texture) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */); // GLES + EGL specs don't specify what should happen here, so best to issue an error and create IDs with 0.
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_OPERATION in glGenTextures: GLctx.createTexture returned null - most likely GL context is lost!');
#endif
        while(i < n) {{{ makeSetValue('textures', 'i++*4', 0, 'i32') }}};
        return;
      }
      var id = GL.getNewId(GL.textures);
      texture.name = id;
      GL.textures[id] = texture;
      {{{ makeSetValue('textures', 'i*4', 'id', 'i32') }}};
    }
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
#if ASSERTIONS
    assert(GL.currentContext.compressionExt);
#endif
    if (data) {
      data = {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}};
    } else {
      data = null;
    }
    // N.b. using array notation explicitly to not confuse Closure minification.
    GLctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, data);
  },

  glCompressedTexSubImage2D__sig: 'viiiiiiiii',
  glCompressedTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, imageSize, data) {
#if ASSERTIONS
    assert(GL.currentContext.compressionExt);
#endif
    if (data) {
      data = {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}};
    } else {
      data = null;
    }
    GLctx['compressedTexSubImage2D'](target, level, xoffset, yoffset, width, height, format, data);
  },

  glTexImage2D__sig: 'viiiiiiiii',
  glTexImage2D: function(target, level, internalFormat, width, height, border, format, type, pixels) {
    if (pixels) {
      var data = GL.getTexPixelData(type, format, width, height, pixels, internalFormat);
      pixels = data.pixels;
      internalFormat = data.internalFormat;
    } else {
      pixels = null;
    }
    GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
  },

  glTexSubImage2D__sig: 'viiiiiiiii',
  glTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, type, pixels) {
    if (pixels) {
      var data = GL.getTexPixelData(type, format, width, height, pixels, -1);
      pixels = data.pixels;
    } else {
      pixels = null;
    }
    GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
  },

  glReadPixels__sig: 'viiiiiii',
  glReadPixels: function(x, y, width, height, format, type, pixels) {
    var data = GL.getTexPixelData(type, format, width, height, pixels, format);
    if (!data.pixels) {
      GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
      Module.printErr('GL_INVALID_ENUM in glReadPixels: Unrecognized combination of type=' + type + ' and format=' + format + '!');
#endif
      return;
    }
    GLctx.readPixels(x, y, width, height, format, type, data.pixels);
  },

  glBindTexture__sig: 'vii',
  glBindTexture: function(target, texture) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.textures, texture, 'glBindTexture', 'texture');
#endif
    GLctx.bindTexture(target, texture ? GL.textures[texture] : null);
  },

  glGetTexParameterfv__sig: 'viii',
  glGetTexParameterfv: function(target, pname, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetTexParameterfv(target=' + target +', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    {{{ makeSetValue('params', '0', 'GLctx.getTexParameter(target, pname)', 'float') }}};
  },

  glGetTexParameteriv__sig: 'viii',
  glGetTexParameteriv: function(target, pname, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetTexParameteriv(target=' + target +', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
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
  glIsTexture: function(texture) {
    var texture = GL.textures[texture];
    if (!texture) return 0;
    return GLctx.isTexture(texture);
  },

  glGenBuffers__sig: 'vii',
  glGenBuffers: function(n, buffers) {
    for (var i = 0; i < n; i++) {
      var buffer = GLctx.createBuffer();
      if (!buffer) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_OPERATION in glGenBuffers: GLctx.createBuffer returned null - most likely GL context is lost!');
#endif
        while(i < n) {{{ makeSetValue('buffers', 'i++*4', 0, 'i32') }}};
        return;
      }
      var id = GL.getNewId(GL.buffers);
      buffer.name = id;
      GL.buffers[id] = buffer;
      {{{ makeSetValue('buffers', 'i*4', 'id', 'i32') }}};
    }
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
    }
  },

  glGetBufferParameteriv__sig: 'viii',
  glGetBufferParameteriv: function(target, value, data) {
#if GL_ASSERTIONS
    if (!data) {
      // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
      // if data == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetBufferParameteriv(target=' + target + ', value=' + value + ', data=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    {{{ makeSetValue('data', '0', 'GLctx.getBufferParameter(target, value)', 'i32') }}};
  },

  glBufferData__sig: 'viiii',
  glBufferData: function(target, size, data, usage) {
    switch (usage) { // fix usages, WebGL only has *_DRAW
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
    if (!data) {
      GLctx.bufferData(target, size, usage);
    } else {
      GLctx.bufferData(target, HEAPU8.subarray(data, data+size), usage);
    }
  },

  glBufferSubData__sig: 'viiii',
  glBufferSubData: function(target, offset, size, data) {
    GLctx.bufferSubData(target, offset, HEAPU8.subarray(data, data+size));
  },

#if FULL_ES3
  glMapBufferRange__sig: 'iiiii',
  glMapBufferRange: function(target, offset, length, access) {
    if (access != 0x1A && access != 0xA) {
      Module.printErr("glMapBufferRange is only supported when access is MAP_WRITE|INVALIDATE_BUFFER");
      return 0;
    }

    if (!GL.validateBufferTarget(target)) {
      GL.recordError(0x0500/*GL_INVALID_ENUM*/);
      Module.printErr('GL_INVALID_ENUM in glMapBufferRange');
      return 0;
    }

    var mem = _malloc(length);
    if (!mem) return 0;

    GL.mappedBuffers[target] = {
      offset: offset,
      length: length,
      mem: mem,
      access: access,
    };
    return mem;
  },

  glFlushMappedBufferRange__sig: 'viii',
  glFlushMappedBufferRange: function(target, offset, length) {
    if (!GL.validateBufferTarget(target)) {
      GL.recordError(0x0500/*GL_INVALID_ENUM*/);
      Module.printErr('GL_INVALID_ENUM in glUnmapBuffer');
      return 0;
    }

    var mapping = GL.mappedBuffers[target];
    if (!mapping) {
      GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
      Module.printError('buffer was never mapped in glFlushMappedBufferRange');
      return 0;
    }

    if (!(mapping.access & 0x10)) {
      GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
      Module.printError('buffer was not mapped with GL_MAP_FLUSH_EXPLICIT_BIT in glFlushMappedBufferRange');
      return 0;
    }
    if (offset < 0 || length < 0 || offset + length > mapping.length) {
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      Module.printError('invalid range in glFlushMappedBufferRange');
      return 0;
    }

    GLctx.bufferSubData(
      target,
      mapping.offset,
      HEAPU8.subarray(mapping.mem + offset, mapping.mem + offset + length));
  },

  glUnmapBuffer__sig: 'ii',
  glUnmapBuffer: function(target) {
    if (!GL.validateBufferTarget(target)) {
      GL.recordError(0x0500/*GL_INVALID_ENUM*/);
      Module.printErr('GL_INVALID_ENUM in glUnmapBuffer');
      return 0;
    }

    var mapping = GL.mappedBuffers[target];
    if (!mapping) {
      GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
      Module.printError('buffer was never mapped in glUnmapBuffer');
      return 0;
    }
    GL.mappedBuffers[target] = null;

    if (!(mapping.access & 0x10)) /* GL_MAP_FLUSH_EXPLICIT_BIT */
      GLctx.bufferSubData(target, mapping.offset, HEAPU8.subarray(mapping.mem, mapping.mem+mapping.length));
    _free(mapping.mem);
    return 1;
  },
#endif

#if USE_WEBGL2
  glInvalidateFramebuffer__sig: 'viii',
  glInvalidateFramebuffer: function(target, numAttachments, attachments) {
    var list = [];
    for (var i = 0; i < numAttachments; i++)
      list.push({{{ makeGetValue('attachments', 'i*4', 'i32') }}});

    GLctx.invalidateFramebuffer(target, list);
  },

  glInvalidateSubFramebuffer__sig: 'viiiiiii',
  glInvalidateSubFramebuffer: function(target, numAttachments, attachments, x, y, width, height) {
    var list = [];
    for (var i = 0; i < numAttachments; i++)
      list.push({{{ makeGetValue('attachments', 'i*4', 'i32') }}});

    GLctx.invalidateSubFramebuffer(target, list, x, y, width, height);
  },

  glTexStorage2D__sig: 'viiiii',
  glTexStorage2D: function(target, levels, internalformat, width, height) {
    GLctx.texStorage2D(target, levels, internalformat, width, height);
  },

  glTexStorage3D__sig: 'viiiiii',
  glTexStorage3D: function(target, levels, internalformat, width, height, depth) {
    GLctx.texStorage3D(target, levels, internalformat, width, height, depth);
  },

  glTexImage3D__sig: 'viiiiiiiiii',
  glTexImage3D: function(target, level, internalFormat, width, height, depth, border, format, type, data) {
    GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type,
                     HEAPU8.subarray(data));
  },

  glTexSubImage3D__sig: 'viiiiiiiiiii',
  glTexSubImage3D: function(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data) {
    GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type,
                        HEAPU8.subarray(data));
  },

  // Framebuffer objects
  glBlitFramebuffer__sig: 'viiiiiiiiii',
  glBlitFramebuffer: function(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter) {
    GLctx.blitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
  },

  glReadBuffer__sig: 'vi',
  glReadBuffer: function(src) {
    GLctx.readBuffer(src);
  },

  // Queries
  glGenQueries__sig: 'vii',
  glGenQueries: function(n, ids) {
    for (var i = 0; i < n; i++) {
      var query = GLctx.createQuery();
      if (!query) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_OPERATION in glGenQueries: GLctx.createQuery returned null - most likely GL context is lost!');
#endif
        while(i < n) {{{ makeSetValue('ids', 'i++*4', 0, 'i32') }}};
        return;
      }
      var id = GL.getNewId(GL.queries);
      query.name = id;
      GL.queries[id] = query;
      {{{ makeSetValue('ids', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteQueries__sig: 'vii',
  glDeleteQueries: function(n, ids) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('ids', 'i*4', 'i32') }}};
      var query = GL.queries[id];
      if (!query) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
      GLctx.deleteQuery(query);
      query.name = 0;
      GL.queries[id] = null;
    }
  },

  glIsQuery__sig: 'ii',
  glIsQuery: function(id) {
    var query = GL.queries[query];
    if (!query) return 0;
    return GLctx.isQuery(query);
  },

  glBeginQuery__sig: 'vii',
  glBeginQuery: function(target, id) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.queries, id, 'glBeginQuery', 'id');
#endif
    GLctx.beginQuery(target, id ? GL.queries[id] : null);
  },

  glEndQuery__sig: 'vi',
  glEndQuery: function(target) {
    GLctx.endQuery(target);
  },

  glGetQueryiv__sig: 'viii',
  glGetQueryiv: function(target, pname, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetQueryiv(target=' + target +', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    {{{ makeSetValue('params', '0', 'GLctx.getQuery(target, pname)', 'i32') }}};
  },

  glGetQueryObjectuiv__sig: 'viii',
  glGetQueryObjectuiv: function(id, pname, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetQueryObjectuiv(id=' + id +', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    GL.validateGLObjectID(GL.queries, id, 'glGetQueryObjectuiv', 'id');
#endif
    var query = GL.queries[id];
    var param = GLctx.getQueryParameter(query, pname);
    var ret;
    if (typeof param == 'boolean') {
      ret = param ? 1 : 0;
    } else {
      ret = param;
    }
    {{{ makeSetValue('params', '0', 'ret', 'i32') }}};
  },

  // Renderbuffer objects
  glRenderbufferStorageMultisample__sig: 'viiiii',
  glRenderbufferStorageMultisample: function(target, samples, internalformat, width, height) {
    GLctx.renderbufferStorageMultisample(target, samples, internalformat, width, height);
  },

  // Sampler objects
  glGenSamplers__sig: 'vii',
  glGenSamplers: function(n, samplers) {
    for (var i = 0; i < n; i++) {
      var sampler = GLctx.createSampler();
      if (!sampler) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_OPERATION in glGenSamplers: GLctx.createSampler returned null - most likely GL context is lost!');
#endif
        while(i < n) {{{ makeSetValue('samplers', 'i++*4', 0, 'i32') }}};
        return;
      }
      var id = GL.getNewId(GL.samplers);
      sampler.name = id;
      GL.samplers[id] = sampler;
      {{{ makeSetValue('samplers', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteSamplers__sig: 'vii',
  glDeleteSamplers: function(n, samplers) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('samplers', 'i*4', 'i32') }}};
      var sampler = GL.samplers[id];
      if (!sampler) continue;
      GLctx.deleteSampler(sampler);
      sampler.name = 0;
      GL.samplers[id] = null;
    }
  },

  glIsSampler__sig: 'ii',
  glIsSampler: function(id) {
    var sampler = GL.samplers[id];
    if (!sampler) return 0;
    return GLctx.isSampler(sampler);
  },

  glBindSampler__sig: 'vii',
  glBindSampler: function(unit, sampler) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    GLctx.bindSampler(unit, sampler ? GL.samplers[sampler] : null);
  },

  glSamplerParameterf__sig: 'viif',
  glSamplerParameterf: function(sampler, pname, param) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    GLctx.samplerParameterf(sampler ? GL.samplers[sampler] : null, pname, param);
  },

  glSamplerParameteri__sig: 'viii',
  glSamplerParameteri: function(sampler, pname, param) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    GLctx.samplerParameteri(sampler ? GL.samplers[sampler] : null, pname, param);
  },

  glSamplerParameterfv__sig: 'viii',
  glSamplerParameterfv: function(sampler, pname, params) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    var param = {{{ makeGetValue('params', '0', 'float') }}};
    GLctx.samplerParameterf(sampler ? GL.samplers[sampler] : null, pname, param);
  },

  glSamplerParameteriv__sig: 'viii',
  glSamplerParameteriv: function(sampler, pname, params) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    var param = {{{ makeGetValue('params', '0', 'i32') }}};
    GLctx.samplerParameteri(sampler ? GL.samplers[sampler] : null, pname, param);
  },

  glGetSamplerParameterfv__sig: 'viii',
  glGetSamplerParameterfv: function(sampler, pname, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
      Module.printErr('GL_INVALID_VALUE in glGetSamplerParameterfv(sampler=' + sampler +', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    sampler = GL.samplers[sampler];
    {{{ makeSetValue('params', '0', 'GLctx.getSamplerParameter(sampler, pname)', 'float') }}};
  },

  glGetSamplerParameteriv__sig: 'viii',
  glGetSamplerParameteriv: function(sampler, pname, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
      Module.printErr('GL_INVALID_VALUE in glGetSamplerParameteriv(sampler=' + sampler +', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    sampler = GL.samplers[sampler];
    {{{ makeSetValue('params', '0', 'GLctx.getSamplerParameter(sampler, pname)', 'i32') }}};
  },

  // Transform Feedback
  glGenTransformFeedbacks__sig: 'vii',
  glGenTransformFeedbacks: function(n, ids) {
    for (var i = 0; i < n; i++) {
      var transformFeedback = GLctx.createTransformFeedback();
      if (!transformFeedback) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_OPERATION in glGenTransformFeedbacks: GLctx.createTransformFeedback returned null - most likely GL context is lost!');
#endif
        while(i < n) {{{ makeSetValue('ids', 'i++*4', 0, 'i32') }}};
        return;
      }
      var id = GL.getNewId(GL.transformFeedbacks);
      transformFeedback.name = id;
      GL.transformFeedbacks[id] = transformFeedback;
      {{{ makeSetValue('ids', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteTransformFeedbacks__sig: 'vii',
  glDeleteTransformFeedbacks: function(n, ids) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('ids', 'i*4', 'i32') }}};
      var transformFeedback = GL.transformFeedbacks[id];
      if (!transformFeedback) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
      GLctx.deleteTransformFeedback(transformFeedback);
      transformFeedback.name = 0;
      GL.transformFeedbacks[id] = null;
    }
  },

  glIsTransformFeedback__sig: 'ii',
  glIsTransformFeedback: function(transformFeedback) {
    var transformFeedback = GL.transformFeedbacks[transformFeedback];
    if (!transformFeedback) return 0;
    return GLctx.isTransformFeedback(transformFeedback);
  },

  glBindTransformFeedback__sig: 'vii',
  glBindTransformFeedback: function(target, id) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.transformFeedbacks, id, 'glBindTransformFeedback', 'id');
#endif
    var transformFeedback = id ? GL.transformFeedbacks[id] : null;
    if (id && !transformFeedback) { // Passing an nonexisting or an already deleted id is an error.
      GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
      return;
    }
    GLctx.bindTransformFeedback(target, transformFeedback);
  },

  glBeginTransformFeedback__sig: 'vi',
  glBeginTransformFeedback: function(primitiveMode) {
    GLctx.beginTransformFeedback(primitiveMode);
  },

  glEndTransformFeedback__sig: 'v',
  glEndTransformFeedback: function() {
    GLctx.endTransformFeedback();
  },

  glPauseTransformFeedback__sig: 'v',
  glPauseTransformFeedback: function() {
    GLctx.pauseTransformFeedback();
  },

  glResumeTransformFeedback__sig: 'v',
  glResumeTransformFeedback: function() {
    GLctx.resumeTransformFeedback();
  },

  glTransformFeedbackVaryings__sig: 'viiii',
  glTransformFeedbackVaryings: function(program, count, varyings, bufferMode) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glTransformFeedbackVaryings', 'program');
#endif
    program = GL.programs[program];
    var vars = [];
    for (var i = 0; i < count; i++)
      vars.push(Pointer_stringify({{{ makeGetValue('varyings', 'i*4', 'i32') }}}));

    GLctx.transformFeedbackVaryings(program, vars, bufferMode);
  },

  glGetTransformFeedbackVarying__sig: 'viiiiiii',
  glGetTransformFeedbackVarying: function(program, index, bufSize, length, size, type, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetTransformFeedbackVarying', 'program');
#endif
    program = GL.programs[program];
    var info = GLctx.getTransformFeedbackVarying(program, index);
    if (!info) return; // If an error occurred, the return parameters length, size, type and name will be unmodified.

    var infoname = info.name.slice(0, Math.max(0, bufSize - 1));
    if (name && bufSize > 0) {
      writeStringToMemory(infoname, name);
      if (length) {{{ makeSetValue('length', '0', 'infoname.length', 'i32') }}};
    } else {
      if (length) {{{ makeSetValue('length', '0', 0, 'i32') }}};
    }

    if (size) {{{ makeSetValue('size', '0', 'info.size', 'i32') }}};
    if (type) {{{ makeSetValue('type', '0', 'info.type', 'i32') }}};
  },

  glGetIntegeri_v__sig: 'viii',
  glGetIntegeri_v: function(target, index, data) {
#if GL_ASSERTIONS
    if (!data) {
      // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
      // if data == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetIntegeri_v(target=' + target + ', index=' + index + ', data=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    var result = GLctx.getIndexedParameter(target, index);
    var ret;
    switch (typeof result) {
      case 'boolean':
        ret = result ? 1 : 0;
        break;
      case 'number':
        ret = result;
        break;
      case 'object':
        if (result === null) {
          switch (target) {
            case 0x8C8F: // TRANSFORM_FEEDBACK_BUFFER_BINDING
            case 0x8A28: // UNIFORM_BUFFER_BINDING
              ret = 0;
              break;
            default: {
              GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
              Module.printErr('GL_INVALID_ENUM in glGetIntegeri_v(' + target + ') and it returns null!');
#endif
              return;
            }
          }
        } else if (result instanceof WebGLBuffer) {
          ret = result.name | 0;
        } else {
          GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
          Module.printErr('GL_INVALID_ENUM in glGetIntegeri_v: Unknown object returned from WebGL getIndexedParameter(' + target + ')!');
#endif
          return;
        }
        break;
      default:
        GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_ENUM in glGetIntegeri_v: Native code calling glGetIntegeri_v(' + target + ') and it returns ' + result + ' of type ' + typeof(result) + '!');
#endif
        return;
    }

    {{{ makeSetValue('data', '0', 'ret', 'i32') }}};
  },

  // Uniform Buffer objects
  glBindBufferBase__sig: 'viii',
  glBindBufferBase: function(target, index, buffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.buffers, buffer, 'glBindBufferBase', 'buffer');
#endif
    var bufferObj = buffer ? GL.buffers[buffer] : null;
    GLctx.bindBufferBase(target, index, bufferObj);
  },

  glBindBufferRange__sig: 'viiiii',
  glBindBufferRange: function(target, index, buffer, offset, ptrsize) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.buffers, buffer, 'glBindBufferRange', 'buffer');
#endif
    var bufferObj = buffer ? GL.buffers[buffer] : null;
    GLctx.bindBufferRange(target, index, bufferObj, offset, ptrsize);
  },

  glGetUniformIndices__sig: 'viiii',
  glGetUniformIndices: function(program, uniformCount, uniformNames, uniformIndices) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformIndices', 'program');
    if (!uniformIndices) {
      // GLES2 specification does not specify how to behave if uniformIndices is a null pointer. Since calling this function does not make sense
      // if uniformIndices == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetUniformIndices(program=' + program + ', uniformCount=' + uniformCount + ', uniformNames=' + uniformNames + ', uniformIndices=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    if (uniformCount > 0 && (uniformNames == 0 || uniformIndices == 0)) {
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    program = GL.programs[program];
    var names = [];
    for (var i = 0; i < uniformCount; i++)
      names.push(Pointer_stringify({{{ makeGetValue('uniformNames', 'i*4', 'i32') }}}));

    var result = GLctx.getUniformIndices(program, names);
    if (!result) return; // GL spec: If an error is generated, nothing is written out to uniformIndices.

    var len = result.length;
    for (var i = 0; i < len; i++) {
      {{{ makeSetValue('uniformIndices', 'i*4', 'result[i]', 'i32') }}};
    }
  },

  glGetActiveUniformsiv__sig: 'viiiii',
  glGetActiveUniformsiv: function(program, uniformCount, uniformIndices, pname, params) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveUniformsiv', 'program');
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetActiveUniformsiv(program=' + program + ', uniformCount=' + uniformCount + ', uniformIndices=' + uniformIndices + ', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    if (uniformCount > 0 && uniformIndices == 0) {
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    program = GL.programs[program];
    var ids = [];
    for (var i = 0; i < uniformCount; i++) {
      ids.push({{{ makeGetValue('uniformIndices', 'i*4', 'i32') }}});
    }

    var result = GLctx.getActiveUniforms(program, ids, pname);
    if (!result) return; // GL spec: If an error is generated, nothing is written out to params.

    var len = result.length;
    for (var i = 0; i < len; i++) {
      {{{ makeSetValue('params', 'i*4', 'result[i]', 'i32') }}};
    }
  },

  glGetUniformBlockIndex__sig: 'iii',
  glGetUniformBlockIndex: function(program, uniformBlockName) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformBlockIndex', 'program');
#endif
    program = GL.programs[program];
    uniformBlockName = Pointer_stringify(uniformBlockName);
    return GLctx.getUniformBlockIndex(program, uniformBlockName);
  },

  glGetActiveUniformBlockiv__sig: 'viiii',
  glGetActiveUniformBlockiv: function(program, uniformBlockIndex, pname, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetActiveUniformBlockiv(program=' + program + ', uniformBlockIndex=' + uniformBlockIndex + ', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveUniformBlockiv', 'program');
#endif
    program = GL.programs[program];

    var result = GLctx.getActiveUniformBlockParameter(program, uniformBlockIndex, pname);
    if (!result) return; // If an error occurs, nothing will be written to params.
    if (typeof result == 'number') {
      {{{ makeSetValue('params', '0', 'result', 'i32') }}};
    } else {
      for (var i = 0; i < result.length; i++) {
        {{{ makeSetValue('params', 'i*4', 'result[i]', 'i32') }}};
      }
    }
  },

  glGetActiveUniformBlockName__sig: 'viiiii',
  glGetActiveUniformBlockName: function(program, uniformBlockIndex, bufSize, length, uniformBlockName) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveUniformBlockName', 'program');
#endif
    program = GL.programs[program];

    var result = GLctx.getActiveUniformBlockName(program, uniformBlockIndex);
    if (!result) return; // If an error occurs, nothing will be written to uniformBlockName or length.
    var name = result.slice(0, Math.max(0, bufSize - 1));
    if (uniformBlockName && bufSize > 0) {
      writeStringToMemory(name, uniformBlockName);
      if (length) {{{ makeSetValue('length', '0', 'name.length', 'i32') }}};
    } else {
      if (length) {{{ makeSetValue('length', '0', 0, 'i32') }}};
    }
  },

  glUniformBlockBinding__sig: 'viii',
  glUniformBlockBinding: function(program, uniformBlockIndex, uniformBlockBinding) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glUniformBlockBinding', 'program');
#endif
    program = GL.programs[program];

    GLctx.uniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
  },

// ~USE_WEBGL2
#endif

  glIsBuffer__sig: 'ii',
  glIsBuffer: function(buffer) {
    var b = GL.buffers[buffer];
    if (!b) return 0;
    return GLctx.isBuffer(b);
  },

  glGenRenderbuffers__sig: 'vii',
  glGenRenderbuffers: function(n, renderbuffers) {
    for (var i = 0; i < n; i++) {
      var renderbuffer = GLctx.createRenderbuffer();
      if (!renderbuffer) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_OPERATION in glGenRenderbuffers: GLctx.createRenderbuffer returned null - most likely GL context is lost!');
#endif
        while(i < n) {{{ makeSetValue('renderbuffers', 'i++*4', 0, 'i32') }}};
        return;
      }
      var id = GL.getNewId(GL.renderbuffers);
      renderbuffer.name = id;
      GL.renderbuffers[id] = renderbuffer;
      {{{ makeSetValue('renderbuffers', 'i*4', 'id', 'i32') }}};
    }
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
    GLctx.bindRenderbuffer(target, renderbuffer ? GL.renderbuffers[renderbuffer] : null);
  },

  glGetRenderbufferParameteriv__sig: 'viii',
  glGetRenderbufferParameteriv: function(target, pname, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetRenderbufferParameteriv(target=' + target + ', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    {{{ makeSetValue('params', '0', 'GLctx.getRenderbufferParameter(target, pname)', 'i32') }}};
  },

  glIsRenderbuffer__sig: 'ii',
  glIsRenderbuffer: function(renderbuffer) {
    var rb = GL.renderbuffers[renderbuffer];
    if (!rb) return 0;
    return GLctx.isRenderbuffer(rb);
  },

  glGetUniformfv__sig: 'viii',
  glGetUniformfv: function(program, location, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetUniformfv(program=' + program + ', location=' + location + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformfv', 'program');
    GL.validateGLObjectID(GL.uniforms, location, 'glGetUniformfv', 'location');
#endif
    var data = GLctx.getUniform(GL.programs[program], GL.uniforms[location]);
    if (typeof data == 'number') {
      {{{ makeSetValue('params', '0', 'data', 'float') }}};
    } else {
      for (var i = 0; i < data.length; i++) {
        {{{ makeSetValue('params', 'i', 'data[i]', 'float') }}};
      }
    }
  },

  glGetUniformiv__sig: 'viii',
  glGetUniformiv: function(program, location, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetUniformiv(program=' + program + ', location=' + location + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformiv', 'program');
    GL.validateGLObjectID(GL.uniforms, location, 'glGetUniformiv', 'location');
#endif
    var data = GLctx.getUniform(GL.programs[program], GL.uniforms[location]);
    if (typeof data == 'number' || typeof data == 'boolean') {
      {{{ makeSetValue('params', '0', 'data', 'i32') }}};
    } else {
      for (var i = 0; i < data.length; i++) {
        {{{ makeSetValue('params', 'i', 'data[i]', 'i32') }}};
      }
    }
  },

  glGetUniformLocation__sig: 'iii',
  glGetUniformLocation: function(program, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformLocation', 'program');
#endif
    name = Pointer_stringify(name);

    var arrayOffset = 0;
    // If user passed an array accessor "[index]", parse the array index off the accessor.
    if (name.indexOf(']', name.length-1) !== -1) {
      var ls = name.lastIndexOf('[');
      var arrayIndex = name.slice(ls+1, -1);
      if (arrayIndex.length > 0) {
        arrayOffset = parseInt(arrayIndex);
        if (arrayOffset < 0) {
          return -1;
        }
      }
      name = name.slice(0, ls);
    }

    var ptable = GL.programInfos[program];
    if (!ptable) {
      return -1;
    }
    var utable = ptable.uniforms;
    var uniformInfo = utable[name]; // returns pair [ dimension_of_uniform_array, uniform_location ]
    if (uniformInfo && arrayOffset < uniformInfo[0]) { // Check if user asked for an out-of-bounds element, i.e. for 'vec4 colors[3];' user could ask for 'colors[10]' which should return -1.
      return uniformInfo[1]+arrayOffset;
    } else {
      return -1;
    }
  },

  glGetVertexAttribfv__sig: 'viii',
  glGetVertexAttribfv: function(index, pname, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetVertexAttribfv(index=' + index + ', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
#if FULL_ES2
    if (GL.currentContext.clientBuffers[index].enabled) {
      Module.printErr("glGetVertexAttribfv on client-side array: not supported, bad data returned");
    }
#endif
    var data = GLctx.getVertexAttrib(index, pname);
    if (typeof data == 'number') {
      {{{ makeSetValue('params', '0', 'data', 'float') }}};
    } else {
      for (var i = 0; i < data.length; i++) {
        {{{ makeSetValue('params', 'i', 'data[i]', 'float') }}};
      }
    }
  },

  glGetVertexAttribiv__sig: 'viii',
  glGetVertexAttribiv: function(index, pname, params) {
#if GL_ASSERTIONS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetVertexAttribiv(index=' + index + ', pname=' + pname + ', params=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
#if FULL_ES2
    if (GL.currentContext.clientBuffers[index].enabled) {
      Module.printErr("glGetVertexAttribiv on client-side array: not supported, bad data returned");
    }
#endif
    var data = GLctx.getVertexAttrib(index, pname);
    if (typeof data == 'number' || typeof data == 'boolean') {
      {{{ makeSetValue('params', '0', 'data', 'i32') }}};
    } else {
      for (var i = 0; i < data.length; i++) {
        {{{ makeSetValue('params', 'i', 'data[i]', 'i32') }}};
      }
    }
  },

  glGetVertexAttribPointerv__sig: 'viii',
  glGetVertexAttribPointerv: function(index, pname, pointer) {
#if GL_ASSERTIONS
    if (!pointer) {
      // GLES2 specification does not specify how to behave if pointer is a null pointer. Since calling this function does not make sense
      // if pointer == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetVertexAttribPointerv(index=' + index + ', pname=' + pname + ', pointer=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
#if FULL_ES2
    if (GL.currentContext.clientBuffers[index].enabled) {
      Module.printErr("glGetVertexAttribPointer on client-side array: not supported, bad data returned");
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

    var infoname = info.name.slice(0, Math.max(0, bufSize - 1));
    if (bufSize > 0 && name) {
      writeStringToMemory(infoname, name);
      if (length) {{{ makeSetValue('length', '0', 'infoname.length', 'i32') }}};
    } else {
      if (length) {{{ makeSetValue('length', '0', 0, 'i32') }}};
    }

    if (size) {{{ makeSetValue('size', '0', 'info.size', 'i32') }}};
    if (type) {{{ makeSetValue('type', '0', 'info.type', 'i32') }}};
  },

  glUniform1f__sig: 'vif',
  glUniform1f: function(location, v0) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1f', 'location');
#endif
    location = GL.uniforms[location];
    GLctx.uniform1f(location, v0);
  },

  glUniform2f__sig: 'viff',
  glUniform2f: function(location, v0, v1) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2f', 'location');
#endif
    location = GL.uniforms[location];
    GLctx.uniform2f(location, v0, v1);
  },

  glUniform3f__sig: 'vifff',
  glUniform3f: function(location, v0, v1, v2) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3f', 'location');
#endif
    location = GL.uniforms[location];
    GLctx.uniform3f(location, v0, v1, v2);
  },

  glUniform4f__sig: 'viffff',
  glUniform4f: function(location, v0, v1, v2, v3) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4f', 'location');
#endif
    location = GL.uniforms[location];
    GLctx.uniform4f(location, v0, v1, v2, v3);
  },

  glUniform1i__sig: 'vii',
  glUniform1i: function(location, v0) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1i', 'location');
#endif
    location = GL.uniforms[location];
    GLctx.uniform1i(location, v0);
  },

  glUniform2i__sig: 'viii',
  glUniform2i: function(location, v0, v1) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2i', 'location');
#endif
    location = GL.uniforms[location];
    GLctx.uniform2i(location, v0, v1);
  },

  glUniform3i__sig: 'viiii',
  glUniform3i: function(location, v0, v1, v2) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3i', 'location');
#endif
    location = GL.uniforms[location];
    GLctx.uniform3i(location, v0, v1, v2);
  },

  glUniform4i__sig: 'viiiii',
  glUniform4i: function(location, v0, v1, v2, v3) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4i', 'location');
#endif
    location = GL.uniforms[location];
    GLctx.uniform4i(location, v0, v1, v2, v3);
  },

  glUniform1iv__sig: 'viii',
  glUniform1iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1iv', 'location');
#endif
    location = GL.uniforms[location];
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    GLctx.uniform1iv(location, value);
  },

  glUniform2iv__sig: 'viii',
  glUniform2iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2iv', 'location');
#endif
    location = GL.uniforms[location];
    count *= 2;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    GLctx.uniform2iv(location, value);
  },

  glUniform3iv__sig: 'viii',
  glUniform3iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3iv', 'location');
#endif
    location = GL.uniforms[location];
    count *= 3;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    GLctx.uniform3iv(location, value);
  },

  glUniform4iv__sig: 'viii',
  glUniform4iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4iv', 'location');
#endif
    location = GL.uniforms[location];
    count *= 4;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    GLctx.uniform4iv(location, value);
  },

  glUniform1fv__sig: 'viii',
  glUniform1fv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1fv', 'location');
#endif
    location = GL.uniforms[location];
    var view;
    if (count === 1) {
      // avoid allocation for the common case of uploading one uniform
      view = GL.miniTempBufferViews[0];
      view[0] = {{{ makeGetValue('value', '0', 'float') }}};
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
    }
    GLctx.uniform1fv(location, view);
  },

  glUniform2fv__sig: 'viii',
  glUniform2fv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2fv', 'location');
#endif
    location = GL.uniforms[location];
    var view;
    if (count === 1) {
      // avoid allocation for the common case of uploading one uniform
      view = GL.miniTempBufferViews[1];
      view[0] = {{{ makeGetValue('value', '0', 'float') }}};
      view[1] = {{{ makeGetValue('value', '4', 'float') }}};
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*8') }}};
    }
    GLctx.uniform2fv(location, view);
  },

  glUniform3fv__sig: 'viii',
  glUniform3fv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3fv', 'location');
#endif
    location = GL.uniforms[location];
    var view;
    if (count === 1) {
      // avoid allocation for the common case of uploading one uniform
      view = GL.miniTempBufferViews[2];
      view[0] = {{{ makeGetValue('value', '0', 'float') }}};
      view[1] = {{{ makeGetValue('value', '4', 'float') }}};
      view[2] = {{{ makeGetValue('value', '8', 'float') }}};
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*12') }}};
    }
    GLctx.uniform3fv(location, view);
  },

  glUniform4fv__sig: 'viii',
  glUniform4fv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4fv', 'location');
#endif
    location = GL.uniforms[location];
    var view;
    if (count === 1) {
      // avoid allocation for the common case of uploading one uniform
      view = GL.miniTempBufferViews[3];
      view[0] = {{{ makeGetValue('value', '0', 'float') }}};
      view[1] = {{{ makeGetValue('value', '4', 'float') }}};
      view[2] = {{{ makeGetValue('value', '8', 'float') }}};
      view[3] = {{{ makeGetValue('value', '12', 'float') }}};
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*16') }}};
    }
    GLctx.uniform4fv(location, view);
  },

  glUniformMatrix2fv__sig: 'viiii',
  glUniformMatrix2fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix2fv', 'location');
#endif
    location = GL.uniforms[location];
    var view;
    if (count === 1) {
      // avoid allocation for the common case of uploading one uniform matrix
      view = GL.miniTempBufferViews[3];
      for (var i = 0; i < 4; i++) {
        view[i] = {{{ makeGetValue('value', 'i*4', 'float') }}};
      }
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*16') }}};
    }
    GLctx.uniformMatrix2fv(location, transpose, view);
  },

  glUniformMatrix3fv__sig: 'viiii',
  glUniformMatrix3fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix3fv', 'location');
#endif
    location = GL.uniforms[location];
    var view;
    if (count === 1) {
      // avoid allocation for the common case of uploading one uniform matrix
      view = GL.miniTempBufferViews[8];
      for (var i = 0; i < 9; i++) {
        view[i] = {{{ makeGetValue('value', 'i*4', 'float') }}};
      }
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*36') }}};
    }
    GLctx.uniformMatrix3fv(location, transpose, view);
  },

  glUniformMatrix4fv__sig: 'viiii',
  glUniformMatrix4fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix4fv', 'location');
#endif
    location = GL.uniforms[location];
    var view;
    if (count === 1) {
      // avoid allocation for the common case of uploading one uniform matrix
      view = GL.miniTempBufferViews[15];
      for (var i = 0; i < 16; i++) {
        view[i] = {{{ makeGetValue('value', 'i*4', 'float') }}};
      }
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*64') }}};
    }
    GLctx.uniformMatrix4fv(location, transpose, view);
  },

  glBindBuffer__sig: 'vii',
  glBindBuffer: function(target, buffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.buffers, buffer, 'glBindBuffer', 'buffer');
#endif
    var bufferObj = buffer ? GL.buffers[buffer] : null;

#if USES_GL_EMULATION
    if (target == GLctx.ARRAY_BUFFER) {
      GL.currArrayBuffer = buffer;
#if LEGACY_GL_EMULATION
      GLImmediate.lastArrayBuffer = buffer;
#endif
    } else if (target == GLctx.ELEMENT_ARRAY_BUFFER) {
      GL.currElementArrayBuffer = buffer;
    }
#endif

    GLctx.bindBuffer(target, bufferObj);
  },

  glVertexAttrib1fv__sig: 'vii',
  glVertexAttrib1fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+' + (1*4)) }}};
    GLctx.vertexAttrib1fv(index, v);
  },

  glVertexAttrib2fv__sig: 'vii',
  glVertexAttrib2fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+' + (2*4)) }}};
    GLctx.vertexAttrib2fv(index, v);
  },

  glVertexAttrib3fv__sig: 'vii',
  glVertexAttrib3fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+' + (3*4)) }}};
    GLctx.vertexAttrib3fv(index, v);
  },

  glVertexAttrib4fv__sig: 'vii',
  glVertexAttrib4fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+' + (4*4)) }}};
    GLctx.vertexAttrib4fv(index, v);
  },

  glGetAttribLocation__sig: 'vii',
  glGetAttribLocation: function(program, name) {
    program = GL.programs[program];
    name = Pointer_stringify(name);
    return GLctx.getAttribLocation(program, name);
  },

  glGetActiveAttrib__sig: 'viiiiiii',
  glGetActiveAttrib: function(program, index, bufSize, length, size, type, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveAttrib', 'program');
#endif
    program = GL.programs[program];
    var info = GLctx.getActiveAttrib(program, index);
    if (!info) return; // If an error occurs, nothing will be written to length, size and type and name.

    var infoname = info.name.slice(0, Math.max(0, bufSize - 1));
    if (bufSize > 0 && name) {
      writeStringToMemory(infoname, name);
      if (length) {{{ makeSetValue('length', '0', 'infoname.length', 'i32') }}};
    } else {
      if (length) {{{ makeSetValue('length', '0', 0, 'i32') }}};
    }

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
#if ASSERTIONS
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
    GLctx.shaderSource(GL.shaders[shader], source);
  },

  glGetShaderSource__sig: 'viiii',
  glGetShaderSource: function(shader, bufSize, length, source) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderSource', 'shader');
#endif
    var result = GLctx.getShaderSource(GL.shaders[shader]);
    if (!result) return; // If an error occurs, nothing will be written to length or source.
    result = result.slice(0, Math.max(0, bufSize - 1));
    if (bufSize > 0 && source) {
      writeStringToMemory(result, source);
      if (length) {{{ makeSetValue('length', '0', 'result.length', 'i32') }}};
    } else {
      if (length) {{{ makeSetValue('length', '0', 0, 'i32') }}};
    }
  },

  glCompileShader__sig: 'vi',
  glCompileShader: function(shader) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glCompileShader', 'shader');
#endif
    GLctx.compileShader(GL.shaders[shader]);
  },

  glGetShaderInfoLog__sig: 'viiii',
  glGetShaderInfoLog: function(shader, maxLength, length, infoLog) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderInfoLog', 'shader');
#endif
    var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
    // Work around a bug in Chromium which causes getShaderInfoLog to return null
    if (!log) log = '(unknown error)';
    log = log.substr(0, maxLength - 1);
    if (maxLength > 0 && infoLog) {
      writeStringToMemory(log, infoLog);
      if (length) {{{ makeSetValue('length', '0', 'log.length', 'i32') }}};
    } else {
      if (length) {{{ makeSetValue('length', '0', 0, 'i32') }}};
    }
  },

  glGetShaderiv__sig: 'viii',
  glGetShaderiv : function(shader, pname, p) {
#if GL_ASSERTIONS
    if (!p) {
      // GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetShaderiv(shader=' + shader + ', pname=' + pname + ', p=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderiv', 'shader');
#endif
    if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
      var log = GLctx.getShaderInfoLog(GL.shaders[shader]);
      // Work around a bug in Chromium which causes getShaderInfoLog to return null: https://code.google.com/p/chromium/issues/detail?id=111337
      if (!log) log = '(unknown error)';
      {{{ makeSetValue('p', '0', 'log.length + 1', 'i32') }}};
    } else {
      {{{ makeSetValue('p', '0', 'GLctx.getShaderParameter(GL.shaders[shader], pname)', 'i32') }}};
    }
  },

  glGetProgramiv__sig: 'viii',
  glGetProgramiv : function(program, pname, p) {
#if GL_ASSERTIONS
    if (!p) {
      // GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it. 
      Module.printErr('GL_INVALID_VALUE in glGetProgramiv(program=' + program + ', pname=' + pname + ', p=0): Function called with null out pointer!');
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramiv', 'program');
#endif
    if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
      {{{ makeSetValue('p', '0', 'GLctx.getProgramInfoLog(GL.programs[program]).length + 1', 'i32') }}};
    } else if (pname == 0x8B87 /* GL_ACTIVE_UNIFORM_MAX_LENGTH */) {
      var ptable = GL.programInfos[program];
      if (ptable) {
        {{{ makeSetValue('p', '0', 'ptable.maxUniformLength', 'i32') }}};
        return;
      } else if (program < GL.counter) {
#if GL_ASSERTIONS
        Module.printErr("A GL object " + program + " that is not a program object was passed to glGetProgramiv!");
#endif
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
      } else {
#if GL_ASSERTIONS
        Module.printErr("A GL object " + program + " that did not come from GL was passed to glGetProgramiv!");
#endif
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      }
    } else if (pname == 0x8B8A /* GL_ACTIVE_ATTRIBUTE_MAX_LENGTH */) {
      var ptable = GL.programInfos[program];
      if (ptable) {
        if (ptable.maxAttributeLength == -1) {
          var program = GL.programs[program];
          var numAttribs = GLctx.getProgramParameter(program, GLctx.ACTIVE_ATTRIBUTES);
          ptable.maxAttributeLength = 0; // Spec says if there are no active attribs, 0 must be returned.
          for(var i = 0; i < numAttribs; ++i) {
            var activeAttrib = GLctx.getActiveAttrib(program, i);
            ptable.maxAttributeLength = Math.max(ptable.maxAttributeLength, activeAttrib.name.length+1);
          }
        }
        {{{ makeSetValue('p', '0', 'ptable.maxAttributeLength', 'i32') }}};
        return;
      } else if (program < GL.counter) {
#if GL_ASSERTIONS
        Module.printErr("A GL object " + program + " that is not a program object was passed to glGetProgramiv!");
#endif
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
      } else {
#if GL_ASSERTIONS
        Module.printErr("A GL object " + program + " that did not come from GL was passed to glGetProgramiv!");
#endif
        GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      }
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
    GL.programInfos[program] = null; // uniforms no longer keep the same names after linking
    GL.populateUniformTable(program);
  },

  glGetProgramInfoLog__sig: 'viiii',
  glGetProgramInfoLog: function(program, maxLength, length, infoLog) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramInfoLog', 'program');
#endif
    var log = GLctx.getProgramInfoLog(GL.programs[program]);
    // Work around a bug in Chromium which causes getProgramInfoLog to return null: https://code.google.com/p/chromium/issues/detail?id=111337
    // Note that this makes glGetProgramInfoLog behavior to be inconsistent. If an error occurs, GL functions should not write anything
    // to the output parameters, however with this workaround in place, we will always write an empty string out to 'infoLog', even if an
    // error did occur.
    if (!log) log = "";

    log = log.substr(0, maxLength - 1);
    if (maxLength > 0 && infoLog) {
      writeStringToMemory(log, infoLog);
      if (length) {{{ makeSetValue('length', '0', 'log.length', 'i32') }}};
    } else {
      if (length) {{{ makeSetValue('length', '0', 0, 'i32') }}};
    }
  },

  glUseProgram__sig: 'vi',
  glUseProgram: function(program) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glUseProgram', 'program');
#endif
    GLctx.useProgram(program ? GL.programs[program] : null);
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
    var program = GL.programs[program];
    if (!program) return 0;
    return GLctx.isProgram(program);
  },

  glBindAttribLocation__sig: 'viii',
  glBindAttribLocation: function(program, index, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glBindAttribLocation', 'program');
#endif
    name = Pointer_stringify(name);
    GLctx.bindAttribLocation(GL.programs[program], index, name);
  },

  glBindFramebuffer__sig: 'vii',
  glBindFramebuffer: function(target, framebuffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.framebuffers, framebuffer, 'glBindFramebuffer', 'framebuffer');
#endif
    GLctx.bindFramebuffer(target, framebuffer ? GL.framebuffers[framebuffer] : null);
  },

  glGenFramebuffers__sig: 'vii',
  glGenFramebuffers: function(n, ids) {
    for (var i = 0; i < n; ++i) {
      var framebuffer = GLctx.createFramebuffer();
      if (!framebuffer) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_OPERATION in glGenFramebuffers: GLctx.createFramebuffer returned null - most likely GL context is lost!');
#endif
        while(i < n) {{{ makeSetValue('ids', 'i++*4', 0, 'i32') }}};
        return;
      }
      var id = GL.getNewId(GL.framebuffers);
      framebuffer.name = id;
      GL.framebuffers[id] = framebuffer;
      {{{ makeSetValue('ids', 'i*4', 'id', 'i32') }}};
    }
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
    {{{ makeSetValue('params', '0', 'result', 'i32') }}};
  },

  glIsFramebuffer__sig: 'ii',
  glIsFramebuffer: function(framebuffer) {
    var fb = GL.framebuffers[framebuffer];
    if (!fb) return 0;
    return GLctx.isFramebuffer(fb);
  },

#if LEGACY_GL_EMULATION
  glGenVertexArrays__deps: ['emulGlGenVertexArrays'],
#endif
  glGenVertexArrays__sig: 'vii',
  glGenVertexArrays: function (n, arrays) {
#if LEGACY_GL_EMULATION
    _emulGlGenVertexArrays(n, arrays);
#else
#if GL_ASSERTIONS
    assert(GL.currentContext.vaoExt, 'Must have OES_vertex_array_object to use vao');
#endif

    for(var i = 0; i < n; i++) {
      var vao = GL.currentContext.vaoExt.createVertexArrayOES();
      if (!vao) {
        GL.recordError(0x0502 /* GL_INVALID_OPERATION */);
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_OPERATION in glGenVertexArrays: GLctx.vao.createVertexArrayOES returned null - most likely GL context is lost!');
#endif
        while(i < n) {{{ makeSetValue('arrays', 'i++*4', 0, 'i32') }}};
        return;
      }
      var id = GL.getNewId(GL.vaos);
      vao.name = id;
      GL.vaos[id] = vao;
      {{{ makeSetValue('arrays', 'i*4', 'id', 'i32') }}};
    }
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
    assert(GL.currentContext.vaoExt, 'Must have OES_vertex_array_object to use vao');
#endif
    for(var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('vaos', 'i*4', 'i32') }}};
      GL.currentContext.vaoExt.deleteVertexArrayOES(GL.vaos[id]);
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
    assert(GL.currentContext.vaoExt, 'Must have OES_vertex_array_object to use vao');
#endif

    GL.currentContext.vaoExt.bindVertexArrayOES(GL.vaos[vao]);
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
    assert(GL.currentContext.vaoExt, 'Must have OES_vertex_array_object to use vao');
#endif  

    var vao = GL.vaos[array];
    if (!vao) return 0;
    return GL.currentContext.vaoExt.isVertexArrayOES(vao);
#endif
  },

#if LEGACY_GL_EMULATION

  // GL emulation: provides misc. functionality not present in OpenGL ES 2.0 or WebGL

  $GLEmulation__deps: ['$GLImmediateSetup', 'glEnable', 'glDisable', 'glIsEnabled', 'glGetBooleanv', 'glGetIntegerv', 'glGetString', 'glCreateShader', 'glShaderSource', 'glCompileShader', 'glAttachShader', 'glDetachShader', 'glUseProgram', 'glDeleteProgram', 'glBindAttribLocation', 'glLinkProgram', 'glBindBuffer', 'glGetFloatv', 'glHint', 'glEnableVertexAttribArray', 'glDisableVertexAttribArray', 'glVertexAttribPointer', 'glActiveTexture'],
  $GLEmulation__postset: 'GLEmulation.init();',
  $GLEmulation: {
    // Fog support. Partial, we assume shaders are used that implement fog. We just pass them uniforms
    fogStart: 0,
    fogEnd: 1,
    fogDensity: 1.0,
    fogColor: null,
    fogMode: 0x0800, // GL_EXP
    fogEnabled: false,

    // VAO support
    vaos: [],
    currentVao: null,
    enabledVertexAttribArrays: {}, // helps with vao cleanups

    hasRunInit: false,

    init: function() {
      // Do not activate immediate/emulation code (e.g. replace glDrawElements) when in FULL_ES2 mode.
      // We do not need full emulation, we instead emulate client-side arrays etc. in FULL_ES2 code in
      // a straightforward manner, and avoid not having a bound buffer be ambiguous between es2 emulation
      // code and legacy gl emulation code.
#if FULL_ES2
      return;
#endif

      if (GLEmulation.hasRunInit) {
        return;
      }
      GLEmulation.hasRunInit = true;

      GLEmulation.fogColor = new Float32Array(4);

      // Add some emulation workarounds
      Module.printErr('WARNING: using emscripten GL emulation. This is a collection of limited workarounds, do not expect it to work.');
#if GL_UNSAFE_OPTS == 1
      Module.printErr('WARNING: using emscripten GL emulation unsafe opts. If weirdness happens, try -s GL_UNSAFE_OPTS=0');
#endif

      // XXX some of the capabilities we don't support may lead to incorrect rendering, if we do not emulate them in shaders
      var validCapabilities = {
        0x0B44: 1, // GL_CULL_FACE
        0x0BE2: 1, // GL_BLEND
        0x0BD0: 1, // GL_DITHER,
        0x0B90: 1, // GL_STENCIL_TEST
        0x0B71: 1, // GL_DEPTH_TEST
        0x0C11: 1, // GL_SCISSOR_TEST
        0x8037: 1, // GL_POLYGON_OFFSET_FILL
        0x809E: 1, // GL_SAMPLE_ALPHA_TO_COVERAGE
        0x80A0: 1  // GL_SAMPLE_COVERAGE
      };

      var glEnable = _glEnable;
      _glEnable = _emscripten_glEnable = function _glEnable(cap) {
        // Clean up the renderer on any change to the rendering state. The optimization of
        // skipping renderer setup is aimed at the case of multiple glDraw* right after each other
        if (GLImmediate.lastRenderer) GLImmediate.lastRenderer.cleanup();
        if (cap == 0x0B60 /* GL_FOG */) {
          if (GLEmulation.fogEnabled != true) {
            GLImmediate.currentRenderer = null; // Fog parameter is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.fogEnabled = true;
          }
          return;
        } else if (cap == 0x0de1 /* GL_TEXTURE_2D */) {
          // XXX not according to spec, and not in desktop GL, but works in some GLES1.x apparently, so support
          // it by forwarding to glEnableClientState
          /* Actually, let's not, for now. (This sounds exceedingly broken)
           * This is in gl_ps_workaround2.c.
          _glEnableClientState(cap);
          */
          return;
        } else if (!(cap in validCapabilities)) {
          return;
        }
        glEnable(cap);
      };

      var glDisable = _glDisable;
      _glDisable = _emscripten_glDisable = function _glDisable(cap) {
        if (GLImmediate.lastRenderer) GLImmediate.lastRenderer.cleanup();
        if (cap == 0x0B60 /* GL_FOG */) {
          if (GLEmulation.fogEnabled != false) {
            GLImmediate.currentRenderer = null; // Fog parameter is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.fogEnabled = false;
          }
          return;
        } else if (cap == 0x0de1 /* GL_TEXTURE_2D */) {
          // XXX not according to spec, and not in desktop GL, but works in some GLES1.x apparently, so support
          // it by forwarding to glDisableClientState
          /* Actually, let's not, for now. (This sounds exceedingly broken)
           * This is in gl_ps_workaround2.c.
          _glDisableClientState(cap);
          */
          return;
        } else if (!(cap in validCapabilities)) {
          return;
        }
        glDisable(cap);
      };
      _glIsEnabled = _emscripten_glIsEnabled = function _glIsEnabled(cap) {
        if (cap == 0x0B60 /* GL_FOG */) {
          return GLEmulation.fogEnabled ? 1 : 0;
        } else if (!(cap in validCapabilities)) {
          return 0;
        }
        return GLctx.isEnabled(cap);
      };

      var glGetBooleanv = _glGetBooleanv;
      _glGetBooleanv = _emscripten_glGetBooleanv = function _glGetBooleanv(pname, p) {
        var attrib = GLEmulation.getAttributeFromCapability(pname);
        if (attrib !== null) {
          var result = GLImmediate.enabledClientAttributes[attrib];
          {{{ makeSetValue('p', '0', 'result === true ? 1 : 0', 'i8') }}};
          return;
        }
        glGetBooleanv(pname, p);
      };

      var glGetIntegerv = _glGetIntegerv;
      _glGetIntegerv = _emscripten_glGetIntegerv = function _glGetIntegerv(pname, params) {
        switch (pname) {
          case 0x84E2: pname = GLctx.MAX_TEXTURE_IMAGE_UNITS /* fake it */; break; // GL_MAX_TEXTURE_UNITS
          case 0x8B4A: { // GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB
            var result = GLctx.getParameter(GLctx.MAX_VERTEX_UNIFORM_VECTORS);
            {{{ makeSetValue('params', '0', 'result*4', 'i32') }}}; // GLES gives num of 4-element vectors, GL wants individual components, so multiply
            return;
          }
          case 0x8B49: { // GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB
            var result = GLctx.getParameter(GLctx.MAX_FRAGMENT_UNIFORM_VECTORS);
            {{{ makeSetValue('params', '0', 'result*4', 'i32') }}}; // GLES gives num of 4-element vectors, GL wants individual components, so multiply
            return;
          }
          case 0x8B4B: { // GL_MAX_VARYING_FLOATS_ARB
            var result = GLctx.getParameter(GLctx.MAX_VARYING_VECTORS);
            {{{ makeSetValue('params', '0', 'result*4', 'i32') }}}; // GLES gives num of 4-element vectors, GL wants individual components, so multiply
            return;
          }
          case 0x8871: pname = GLctx.MAX_COMBINED_TEXTURE_IMAGE_UNITS /* close enough */; break; // GL_MAX_TEXTURE_COORDS
          case 0x807A: { // GL_VERTEX_ARRAY_SIZE
            var attribute = GLImmediate.clientAttributes[GLImmediate.VERTEX];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.size : 0', 'i32') }}};
            return;
          }
          case 0x807B: { // GL_VERTEX_ARRAY_TYPE
            var attribute = GLImmediate.clientAttributes[GLImmediate.VERTEX];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.type : 0', 'i32') }}};
            return;
          }
          case 0x807C: { // GL_VERTEX_ARRAY_STRIDE
            var attribute = GLImmediate.clientAttributes[GLImmediate.VERTEX];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.stride : 0', 'i32') }}};
            return;
          }
          case 0x8081: { // GL_COLOR_ARRAY_SIZE
            var attribute = GLImmediate.clientAttributes[GLImmediate.COLOR];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.size : 0', 'i32') }}};
            return;
          }
          case 0x8082: { // GL_COLOR_ARRAY_TYPE
            var attribute = GLImmediate.clientAttributes[GLImmediate.COLOR];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.type : 0', 'i32') }}};
            return;
          }
          case 0x8083: { // GL_COLOR_ARRAY_STRIDE
            var attribute = GLImmediate.clientAttributes[GLImmediate.COLOR];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.stride : 0', 'i32') }}};
            return;
          }
          case 0x8088: { // GL_TEXTURE_COORD_ARRAY_SIZE
            var attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.size : 0', 'i32') }}};
            return;
          }
          case 0x8089: { // GL_TEXTURE_COORD_ARRAY_TYPE
            var attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.type : 0', 'i32') }}};
            return;
          }
          case 0x808A: { // GL_TEXTURE_COORD_ARRAY_STRIDE
            var attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.stride : 0', 'i32') }}};
            return;
          }
        }
        glGetIntegerv(pname, params);
      };

      var glGetString = _glGetString;
      _glGetString = _emscripten_glGetString = function _glGetString(name_) {
        if (GL.stringCache[name_]) return GL.stringCache[name_];
        switch(name_) {
          case 0x1F03 /* GL_EXTENSIONS */: // Add various extensions that we can support
            var ret = allocate(intArrayFromString(GLctx.getSupportedExtensions().join(' ') +
                   ' GL_EXT_texture_env_combine GL_ARB_texture_env_crossbar GL_ATI_texture_env_combine3 GL_NV_texture_env_combine4 GL_EXT_texture_env_dot3 GL_ARB_multitexture GL_ARB_vertex_buffer_object GL_EXT_framebuffer_object GL_ARB_vertex_program GL_ARB_fragment_program GL_ARB_shading_language_100 GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader GL_ARB_texture_cube_map GL_EXT_draw_range_elements' +
                   (GL.currentContext.compressionExt ? ' GL_ARB_texture_compression GL_EXT_texture_compression_s3tc' : '') +
                   (GL.currentContext.anisotropicExt ? ' GL_EXT_texture_filter_anisotropic' : '')
            ), 'i8', ALLOC_NORMAL);
            GL.stringCache[name_] = ret;
            return ret;
        }
        return glGetString(name_);
      };

      // Do some automatic rewriting to work around GLSL differences. Note that this must be done in
      // tandem with the rest of the program, by itself it cannot suffice.
      // Note that we need to remember shader types for this rewriting, saving sources makes it easier to debug.
      GL.shaderInfos = {};
#if GL_DEBUG
      GL.shaderSources = {};
      GL.shaderOriginalSources = {};
#endif
      var glCreateShader = _glCreateShader;
      _glCreateShader = _emscripten_glCreateShader = function _glCreateShader(shaderType) {
        var id = glCreateShader(shaderType);
        GL.shaderInfos[id] = {
          type: shaderType,
          ftransform: false
        };
        return id;
      };

      function ensurePrecision(source) {
        if (!/precision +(low|medium|high)p +float *;/.test(source)) {
          source = 'precision mediump float;\n' + source;
        }
        return source;
      }

      var glShaderSource = _glShaderSource;
      _glShaderSource = _emscripten_glShaderSource = function _glShaderSource(shader, count, string, length) {
        var source = GL.getSource(shader, count, string, length);
#if GL_DEBUG
        console.log("glShaderSource: Input: \n" + source);
        GL.shaderOriginalSources[shader] = source;
#endif
        // XXX We add attributes and uniforms to shaders. The program can ask for the # of them, and see the
        // ones we generated, potentially confusing it? Perhaps we should hide them.
        if (GL.shaderInfos[shader].type == GLctx.VERTEX_SHADER) {
          // Replace ftransform() with explicit project/modelview transforms, and add position and matrix info.
          var has_pm = source.search(/u_projection/) >= 0;
          var has_mm = source.search(/u_modelView/) >= 0;
          var has_pv = source.search(/a_position/) >= 0;
          var need_pm = 0, need_mm = 0, need_pv = 0;
          var old = source;
          source = source.replace(/ftransform\(\)/g, '(u_projection * u_modelView * a_position)');
          if (old != source) need_pm = need_mm = need_pv = 1;
          old = source;
          source = source.replace(/gl_ProjectionMatrix/g, 'u_projection');
          if (old != source) need_pm = 1;
          old = source;
          source = source.replace(/gl_ModelViewMatrixTranspose\[2\]/g, 'vec4(u_modelView[0][2], u_modelView[1][2], u_modelView[2][2], u_modelView[3][2])'); // XXX extremely inefficient
          if (old != source) need_mm = 1;
          old = source;
          source = source.replace(/gl_ModelViewMatrix/g, 'u_modelView');
          if (old != source) need_mm = 1;
          old = source;
          source = source.replace(/gl_Vertex/g, 'a_position');
          if (old != source) need_pv = 1;
          old = source;
          source = source.replace(/gl_ModelViewProjectionMatrix/g, '(u_projection * u_modelView)');
          if (old != source) need_pm = need_mm = 1;
          if (need_pv && !has_pv) source = 'attribute vec4 a_position; \n' + source;
          if (need_mm && !has_mm) source = 'uniform mat4 u_modelView; \n' + source;
          if (need_pm && !has_pm) source = 'uniform mat4 u_projection; \n' + source;
          GL.shaderInfos[shader].ftransform = need_pm || need_mm || need_pv; // we will need to provide the fixed function stuff as attributes and uniforms
          for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
            // XXX To handle both regular texture mapping and cube mapping, we use vec4 for tex coordinates.
            var old = source;
            var need_vtc = source.search('v_texCoord' + i) == -1;
            source = source.replace(new RegExp('gl_TexCoord\\[' + i + '\\]', 'g'), 'v_texCoord' + i)
                           .replace(new RegExp('gl_MultiTexCoord' + i, 'g'), 'a_texCoord' + i);
            if (source != old) {
              source = 'attribute vec4 a_texCoord' + i + '; \n' + source;
              if (need_vtc) {
                source = 'varying vec4 v_texCoord' + i + ';   \n' + source;
              }
            }

            old = source;
            source = source.replace(new RegExp('gl_TextureMatrix\\[' + i + '\\]', 'g'), 'u_textureMatrix' + i);
            if (source != old) {
              source = 'uniform mat4 u_textureMatrix' + i + '; \n' + source;
            }
          }
          if (source.indexOf('gl_FrontColor') >= 0) {
            source = 'varying vec4 v_color; \n' +
                     source.replace(/gl_FrontColor/g, 'v_color');
          }
          if (source.indexOf('gl_Color') >= 0) {
            source = 'attribute vec4 a_color; \n' +
                     source.replace(/gl_Color/g, 'a_color');
          }
          if (source.indexOf('gl_Normal') >= 0) {
            source = 'attribute vec3 a_normal; \n' +
                     source.replace(/gl_Normal/g, 'a_normal');
          }
          // fog
          if (source.indexOf('gl_FogFragCoord') >= 0) {
            source = 'varying float v_fogFragCoord;   \n' +
                     source.replace(/gl_FogFragCoord/g, 'v_fogFragCoord');
          }
          source = ensurePrecision(source);
        } else { // Fragment shader
          for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
            var old = source;
            source = source.replace(new RegExp('gl_TexCoord\\[' + i + '\\]', 'g'), 'v_texCoord' + i);
            if (source != old) {
              source = 'varying vec4 v_texCoord' + i + ';   \n' + source;
            }
          }
          if (source.indexOf('gl_Color') >= 0) {
            source = 'varying vec4 v_color; \n' + source.replace(/gl_Color/g, 'v_color');
          }
          if (source.indexOf('gl_Fog.color') >= 0) {
            source = 'uniform vec4 u_fogColor;   \n' +
                     source.replace(/gl_Fog.color/g, 'u_fogColor');
          }
          if (source.indexOf('gl_Fog.end') >= 0) {
            source = 'uniform float u_fogEnd;   \n' +
                     source.replace(/gl_Fog.end/g, 'u_fogEnd');
          }
          if (source.indexOf('gl_Fog.scale') >= 0) {
            source = 'uniform float u_fogScale;   \n' +
                     source.replace(/gl_Fog.scale/g, 'u_fogScale');
          }
          if (source.indexOf('gl_Fog.density') >= 0) {
            source = 'uniform float u_fogDensity;   \n' +
                     source.replace(/gl_Fog.density/g, 'u_fogDensity');
          }
          if (source.indexOf('gl_FogFragCoord') >= 0) {
            source = 'varying float v_fogFragCoord;   \n' +
                     source.replace(/gl_FogFragCoord/g, 'v_fogFragCoord');
          }
          source = ensurePrecision(source);
        }
#if GL_DEBUG
        GL.shaderSources[shader] = source;
        console.log("glShaderSource: Output: \n" + source);
#endif
        GLctx.shaderSource(GL.shaders[shader], source);
      };

      var glCompileShader = _glCompileShader;
      _glCompileShader = _emscripten_glCompileShader = function _glCompileShader(shader) {
        GLctx.compileShader(GL.shaders[shader]);
#if GL_DEBUG
        if (!GLctx.getShaderParameter(GL.shaders[shader], GLctx.COMPILE_STATUS)) {
          Module.printErr('Failed to compile shader: ' + GLctx.getShaderInfoLog(GL.shaders[shader]));
          Module.printErr('Info: ' + JSON.stringify(GL.shaderInfos[shader]));
          Module.printErr('Original source: ' + GL.shaderOriginalSources[shader]);
          Module.printErr('Source: ' + GL.shaderSources[shader]);
          throw 'Shader compilation halt';
        }
#endif
      };

      GL.programShaders = {};
      var glAttachShader = _glAttachShader;
      _glAttachShader = _emscripten_glAttachShader = function _glAttachShader(program, shader) {
        if (!GL.programShaders[program]) GL.programShaders[program] = [];
        GL.programShaders[program].push(shader);
        glAttachShader(program, shader);
      };

      var glDetachShader = _glDetachShader;
      _glDetachShader = _emscripten_glDetachShader = function _glDetachShader(program, shader) {
        var programShader = GL.programShaders[program];
        if (!programShader) {
          Module.printErr('WARNING: _glDetachShader received invalid program: ' + program);
          return;
        }
        var index = programShader.indexOf(shader);
        programShader.splice(index, 1);
        glDetachShader(program, shader);
      };

      var glUseProgram = _glUseProgram;
      _glUseProgram = _emscripten_glUseProgram = function _glUseProgram(program) {
#if GL_DEBUG
        if (GL.debug) {
          Module.printErr('[using program with shaders]');
          if (program) {
            GL.programShaders[program].forEach(function(shader) {
              Module.printErr('  shader ' + shader + ', original source: ' + GL.shaderOriginalSources[shader]);
              Module.printErr('         Source: ' + GL.shaderSources[shader]);
            });
          }
        }
#endif
        if (GL.currProgram != program) {
          GLImmediate.currentRenderer = null; // This changes the FFP emulation shader program, need to recompute that.
          GL.currProgram = program;
          GLImmediate.fixedFunctionProgram = 0;
          glUseProgram(program);
        }
      }

      var glDeleteProgram = _glDeleteProgram;
      _glDeleteProgram = _emscripten_glDeleteProgram = function _glDeleteProgram(program) {
        glDeleteProgram(program);
        if (program == GL.currProgram) {
          GLImmediate.currentRenderer = null; // This changes the FFP emulation shader program, need to recompute that.
          GL.currProgram = 0;
        }
      };

      // If attribute 0 was not bound, bind it to 0 for WebGL performance reasons. Track if 0 is free for that.
      var zeroUsedPrograms = {};
      var glBindAttribLocation = _glBindAttribLocation;
      _glBindAttribLocation = _emscripten_glBindAttribLocation = function _glBindAttribLocation(program, index, name) {
        if (index == 0) zeroUsedPrograms[program] = true;
        glBindAttribLocation(program, index, name);
      };
      var glLinkProgram = _glLinkProgram;
      _glLinkProgram = _emscripten_glLinkProgram = function _glLinkProgram(program) {
        if (!(program in zeroUsedPrograms)) {
          GLctx.bindAttribLocation(GL.programs[program], 0, 'a_position');
        }
        glLinkProgram(program);
      };

      var glBindBuffer = _glBindBuffer;
      _glBindBuffer = _emscripten_glBindBuffer = function _glBindBuffer(target, buffer) {
        glBindBuffer(target, buffer);
        if (target == GLctx.ARRAY_BUFFER) {
          if (GLEmulation.currentVao) {
#if ASSERTIONS
            assert(GLEmulation.currentVao.arrayBuffer == buffer || GLEmulation.currentVao.arrayBuffer == 0 || buffer == 0, 'TODO: support for multiple array buffers in vao');
#endif
            GLEmulation.currentVao.arrayBuffer = buffer;
          }
        } else if (target == GLctx.ELEMENT_ARRAY_BUFFER) {
          if (GLEmulation.currentVao) GLEmulation.currentVao.elementArrayBuffer = buffer;
        }
      };

      var glGetFloatv = _glGetFloatv;
      _glGetFloatv = _emscripten_glGetFloatv = function _glGetFloatv(pname, params) {
        if (pname == 0x0BA6) { // GL_MODELVIEW_MATRIX
          HEAPF32.set(GLImmediate.matrix[0/*m*/], params >> 2);
        } else if (pname == 0x0BA7) { // GL_PROJECTION_MATRIX
          HEAPF32.set(GLImmediate.matrix[1/*p*/], params >> 2);
        } else if (pname == 0x0BA8) { // GL_TEXTURE_MATRIX
          HEAPF32.set(GLImmediate.matrix[2/*t*/ + GLImmediate.clientActiveTexture], params >> 2);
        } else if (pname == 0x0B66) { // GL_FOG_COLOR
          HEAPF32.set(GLEmulation.fogColor, params >> 2);
        } else if (pname == 0x0B63) { // GL_FOG_START
          {{{ makeSetValue('params', '0', 'GLEmulation.fogStart', 'float') }}};
        } else if (pname == 0x0B64) { // GL_FOG_END
          {{{ makeSetValue('params', '0', 'GLEmulation.fogEnd', 'float') }}};
        } else if (pname == 0x0B62) { // GL_FOG_DENSITY
          {{{ makeSetValue('params', '0', 'GLEmulation.fogDensity', 'float') }}};
        } else if (pname == 0x0B65) { // GL_FOG_MODE
          {{{ makeSetValue('params', '0', 'GLEmulation.fogMode', 'float') }}};
        } else {
          glGetFloatv(pname, params);
        }
      };

      var glHint = _glHint;
      _glHint = _emscripten_glHint = function _glHint(target, mode) {
        if (target == 0x84EF) { // GL_TEXTURE_COMPRESSION_HINT
          return;
        }
        glHint(target, mode);
      };

      var glEnableVertexAttribArray = _glEnableVertexAttribArray;
      _glEnableVertexAttribArray = _emscripten_glEnableVertexAttribArray = function _glEnableVertexAttribArray(index) {
        glEnableVertexAttribArray(index);
        GLEmulation.enabledVertexAttribArrays[index] = 1;
        if (GLEmulation.currentVao) GLEmulation.currentVao.enabledVertexAttribArrays[index] = 1;
      };

      var glDisableVertexAttribArray = _glDisableVertexAttribArray;
      _glDisableVertexAttribArray = _emscripten_glDisableVertexAttribArray = function _glDisableVertexAttribArray(index) {
        glDisableVertexAttribArray(index);
        delete GLEmulation.enabledVertexAttribArrays[index];
        if (GLEmulation.currentVao) delete GLEmulation.currentVao.enabledVertexAttribArrays[index];
      };

      var glVertexAttribPointer = _glVertexAttribPointer;
      _glVertexAttribPointer = _emscripten_glVertexAttribPointer = function _glVertexAttribPointer(index, size, type, normalized, stride, pointer) {
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        if (GLEmulation.currentVao) { // TODO: avoid object creation here? likely not hot though
          GLEmulation.currentVao.vertexAttribPointers[index] = [index, size, type, normalized, stride, pointer];
        }
      };
    },

    getAttributeFromCapability: function(cap) {
      var attrib = null;
      switch (cap) {
        case 0x0de1: // GL_TEXTURE_2D - XXX not according to spec, and not in desktop GL, but works in some GLES1.x apparently, so support it
#if ASSERTIONS
          abort("GL_TEXTURE_2D is not a spec-defined capability for gl{Enable,Disable}ClientState.");
#endif
          // Fall through:
        case 0x8078: // GL_TEXTURE_COORD_ARRAY
          attrib = GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture; break;
        case 0x8074: // GL_VERTEX_ARRAY
          attrib = GLImmediate.VERTEX; break;
        case 0x8075: // GL_NORMAL_ARRAY
          attrib = GLImmediate.NORMAL; break;
        case 0x8076: // GL_COLOR_ARRAY
          attrib = GLImmediate.COLOR; break;
      }
      return attrib;
    },
  },

  glGetShaderPrecisionFormat__sig: 'v',
  glGetShaderPrecisionFormat: function() { throw 'glGetShaderPrecisionFormat: TODO' },

  glDeleteObject__deps: ['glDeleteProgram', 'glDeleteShader'],
  glDeleteObject__sig: 'vi',
  glDeleteObject: function(id) {
    if (GL.programs[id]) {
      _glDeleteProgram(id);
    } else if (GL.shaders[id]) {
      _glDeleteShader(id);
    } else {
      Module.printErr('WARNING: deleteObject received invalid id: ' + id);
    }
  },
  glDeleteObjectARB: 'glDeleteObject',

  glGetObjectParameteriv__sig: 'viii',
  glGetObjectParameteriv__deps: ['glGetProgramiv', 'glGetShaderiv'],
  glGetObjectParameteriv: function(id, type, result) {
    if (GL.programs[id]) {
      if (type == 0x8B84) { // GL_OBJECT_INFO_LOG_LENGTH_ARB
        {{{ makeSetValue('result', '0', 'GLctx.getProgramInfoLog(GL.programs[id]).length', 'i32') }}};
        return;
      }
      _glGetProgramiv(id, type, result);
    } else if (GL.shaders[id]) {
      if (type == 0x8B84) { // GL_OBJECT_INFO_LOG_LENGTH_ARB
        {{{ makeSetValue('result', '0', 'GLctx.getShaderInfoLog(GL.shaders[id]).length', 'i32') }}};
        return;
      } else if (type == 0x8B88) { // GL_OBJECT_SHADER_SOURCE_LENGTH_ARB
        {{{ makeSetValue('result', '0', 'GLctx.getShaderSource(GL.shaders[id]).length', 'i32') }}};
        return;
      }
      _glGetShaderiv(id, type, result);
    } else {
      Module.printErr('WARNING: getObjectParameteriv received invalid id: ' + id);
    }
  },
  glGetObjectParameterivARB: 'glGetObjectParameteriv',

  glGetInfoLog__deps: ['glGetProgramInfoLog', 'glGetShaderInfoLog'],
  glGetInfoLog__sig: 'viiii',
  glGetInfoLog: function(id, maxLength, length, infoLog) {
    if (GL.programs[id]) {
      _glGetProgramInfoLog(id, maxLength, length, infoLog);
    } else if (GL.shaders[id]) {
      _glGetShaderInfoLog(id, maxLength, length, infoLog);
    } else {
      Module.printErr('WARNING: getObjectParameteriv received invalid id: ' + id);
    }
  },
  glGetInfoLogARB: 'glGetInfoLog',

  glBindProgram__sig: 'vii',
  glBindProgram: function(type, id) {
#if ASSERTIONS
    assert(id == 0);
#endif
  },
  glBindProgramARB: 'glBindProgram',

  glGetPointerv: function(name, p) {
    var attribute;
    switch(name) {
      case 0x808E: // GL_VERTEX_ARRAY_POINTER
        attribute = GLImmediate.clientAttributes[GLImmediate.VERTEX]; break;
      case 0x8090: // GL_COLOR_ARRAY_POINTER
        attribute = GLImmediate.clientAttributes[GLImmediate.COLOR]; break;
      case 0x8092: // GL_TEXTURE_COORD_ARRAY_POINTER
        attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture]; break;
      default:
        GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
        Module.printErr('GL_INVALID_ENUM in glGetPointerv: Unsupported name ' + name + '!');
#endif
        return;
    }
    {{{ makeSetValue('p', '0', 'attribute ? attribute.pointer : 0', 'i32') }}};
  },

  // GL Immediate mode

  // See comment in GLEmulation.init()
#if !FULL_ES2
  $GLImmediate__postset: 'GLImmediate.setupFuncs(); Browser.moduleContextCreatedCallbacks.push(function() { GLImmediate.init() });',
#endif
  $GLImmediate__deps: ['$Browser', '$GL', '$GLEmulation'],
  $GLImmediate: {
    MapTreeLib: null,
    spawnMapTreeLib: function() {
      /* A naive implementation of a map backed by an array, and accessed by
       * naive iteration along the array. (hashmap with only one bucket)
       */
      function CNaiveListMap() {
        var list = [];

        this.insert = function CNaiveListMap_insert(key, val) {
          if (this.contains(key|0)) return false;
          list.push([key, val]);
          return true;
        };

        var __contains_i;
        this.contains = function CNaiveListMap_contains(key) {
          for (__contains_i = 0; __contains_i < list.length; ++__contains_i) {
            if (list[__contains_i][0] === key) return true;
          }
          return false;
        };

        var __get_i;
        this.get = function CNaiveListMap_get(key) {
          for (__get_i = 0; __get_i < list.length; ++__get_i) {
            if (list[__get_i][0] === key) return list[__get_i][1];
          }
          return undefined;
        };
      };

      /* A tree of map nodes.
        Uses `KeyView`s to allow descending the tree without garbage.
        Example: {
          // Create our map object.
          var map = new ObjTreeMap();

          // Grab the static keyView for the map.
          var keyView = map.GetStaticKeyView();

          // Let's make a map for:
          // root: <undefined>
          //   1: <undefined>
          //     2: <undefined>
          //       5: "Three, sir!"
          //       3: "Three!"

          // Note how we can chain together `Reset` and `Next` to
          // easily descend based on multiple key fragments.
          keyView.Reset().Next(1).Next(2).Next(5).Set("Three, sir!");
          keyView.Reset().Next(1).Next(2).Next(3).Set("Three!");
        }
      */
      function CMapTree() {
        function CNLNode() {
          var map = new CNaiveListMap();

          this.child = function CNLNode_child(keyFrag) {
            if (!map.contains(keyFrag|0)) {
              map.insert(keyFrag|0, new CNLNode());
            }
            return map.get(keyFrag|0);
          };

          this.value = undefined;
          this.get = function CNLNode_get() {
            return this.value;
          };

          this.set = function CNLNode_set(val) {
            this.value = val;
          };
        }

        function CKeyView(root) {
          var cur;

          this.reset = function CKeyView_reset() {
            cur = root;
            return this;
          };
          this.reset();

          this.next = function CKeyView_next(keyFrag) {
            cur = cur.child(keyFrag);
            return this;
          };

          this.get = function CKeyView_get() {
            return cur.get();
          };

          this.set = function CKeyView_set(val) {
            cur.set(val);
          };
        };

        var root;
        var staticKeyView;

        this.createKeyView = function CNLNode_createKeyView() {
          return new CKeyView(root);
        }

        this.clear = function CNLNode_clear() {
          root = new CNLNode();
          staticKeyView = this.createKeyView();
        };
        this.clear();

        this.getStaticKeyView = function CNLNode_getStaticKeyView() {
          staticKeyView.reset();
          return staticKeyView;
        };
      };

      // Exports:
      return {
        create: function() {
          return new CMapTree();
        },
      };
    },

    TexEnvJIT: null,
    spawnTexEnvJIT: function() {
      // GL defs:
      var GL_TEXTURE0 = 0x84C0;
      var GL_TEXTURE_1D = 0x0DE0;
      var GL_TEXTURE_2D = 0x0DE1;
      var GL_TEXTURE_3D = 0x806f;
      var GL_TEXTURE_CUBE_MAP = 0x8513;
      var GL_TEXTURE_ENV = 0x2300;
      var GL_TEXTURE_ENV_MODE = 0x2200;
      var GL_TEXTURE_ENV_COLOR = 0x2201;
      var GL_TEXTURE_CUBE_MAP_POSITIVE_X = 0x8515;
      var GL_TEXTURE_CUBE_MAP_NEGATIVE_X = 0x8516;
      var GL_TEXTURE_CUBE_MAP_POSITIVE_Y = 0x8517;
      var GL_TEXTURE_CUBE_MAP_NEGATIVE_Y = 0x8518;
      var GL_TEXTURE_CUBE_MAP_POSITIVE_Z = 0x8519;
      var GL_TEXTURE_CUBE_MAP_NEGATIVE_Z = 0x851A;

      var GL_SRC0_RGB = 0x8580;
      var GL_SRC1_RGB = 0x8581;
      var GL_SRC2_RGB = 0x8582;

      var GL_SRC0_ALPHA = 0x8588;
      var GL_SRC1_ALPHA = 0x8589;
      var GL_SRC2_ALPHA = 0x858A;

      var GL_OPERAND0_RGB = 0x8590;
      var GL_OPERAND1_RGB = 0x8591;
      var GL_OPERAND2_RGB = 0x8592;

      var GL_OPERAND0_ALPHA = 0x8598;
      var GL_OPERAND1_ALPHA = 0x8599;
      var GL_OPERAND2_ALPHA = 0x859A;

      var GL_COMBINE_RGB = 0x8571;
      var GL_COMBINE_ALPHA = 0x8572;

      var GL_RGB_SCALE = 0x8573;
      var GL_ALPHA_SCALE = 0x0D1C;

      // env.mode
      var GL_ADD      = 0x0104;
      var GL_BLEND    = 0x0BE2;
      var GL_REPLACE  = 0x1E01;
      var GL_MODULATE = 0x2100;
      var GL_DECAL    = 0x2101;
      var GL_COMBINE  = 0x8570;

      // env.color/alphaCombiner
      //var GL_ADD         = 0x0104;
      //var GL_REPLACE     = 0x1E01;
      //var GL_MODULATE    = 0x2100;
      var GL_SUBTRACT    = 0x84E7;
      var GL_INTERPOLATE = 0x8575;

      // env.color/alphaSrc
      var GL_TEXTURE       = 0x1702;
      var GL_CONSTANT      = 0x8576;
      var GL_PRIMARY_COLOR = 0x8577;
      var GL_PREVIOUS      = 0x8578;

      // env.color/alphaOp
      var GL_SRC_COLOR           = 0x0300;
      var GL_ONE_MINUS_SRC_COLOR = 0x0301;
      var GL_SRC_ALPHA           = 0x0302;
      var GL_ONE_MINUS_SRC_ALPHA = 0x0303;

      var GL_RGB  = 0x1907;
      var GL_RGBA = 0x1908;

      // Our defs:
      var TEXENVJIT_NAMESPACE_PREFIX = "tej_";
      // Not actually constant, as they can be changed between JIT passes:
      var TEX_UNIT_UNIFORM_PREFIX = "uTexUnit";
      var TEX_COORD_VARYING_PREFIX = "vTexCoord";
      var PRIM_COLOR_VARYING = "vPrimColor";
      var TEX_MATRIX_UNIFORM_PREFIX = "uTexMatrix";

      // Static vars:
      var s_texUnits = null; //[];
      var s_activeTexture = 0;

      var s_requiredTexUnitsForPass = [];

      // Static funcs:
      function abort(info) {
        assert(false, "[TexEnvJIT] ABORT: " + info);
      }

      function abort_noSupport(info) {
        abort("No support: " + info);
      }

      function abort_sanity(info) {
        abort("Sanity failure: " + info);
      }

      function genTexUnitSampleExpr(texUnitID) {
        var texUnit = s_texUnits[texUnitID];
        var texType = texUnit.getTexType();

        var func = null;
        switch (texType) {
          case GL_TEXTURE_1D:
            func = "texture2D";
            break;
          case GL_TEXTURE_2D:
            func = "texture2D";
            break;
          case GL_TEXTURE_3D:
            return abort_noSupport("No support for 3D textures.");
          case GL_TEXTURE_CUBE_MAP:
            func = "textureCube";
            break;
          default:
            return abort_sanity("Unknown texType: 0x" + texType.toString(16));
        }

        var texCoordExpr = TEX_COORD_VARYING_PREFIX + texUnitID;
        if (TEX_MATRIX_UNIFORM_PREFIX != null) {
          texCoordExpr = "(" + TEX_MATRIX_UNIFORM_PREFIX + texUnitID + " * " + texCoordExpr + ")";
        }
        return func + "(" + TEX_UNIT_UNIFORM_PREFIX + texUnitID + ", " + texCoordExpr + ".xy)";
      }

      function getTypeFromCombineOp(op) {
        switch (op) {
          case GL_SRC_COLOR:
          case GL_ONE_MINUS_SRC_COLOR:
            return "vec3";
          case GL_SRC_ALPHA:
          case GL_ONE_MINUS_SRC_ALPHA:
            return "float";
        }

        return abort_noSupport("Unsupported combiner op: 0x" + op.toString(16));
      }

      function getCurTexUnit() {
        return s_texUnits[s_activeTexture];
      }

      function genCombinerSourceExpr(texUnitID, constantExpr, previousVar,
                                     src, op)
      {
        var srcExpr = null;
        switch (src) {
          case GL_TEXTURE:
            srcExpr = genTexUnitSampleExpr(texUnitID);
            break;
          case GL_CONSTANT:
            srcExpr = constantExpr;
            break;
          case GL_PRIMARY_COLOR:
            srcExpr = PRIM_COLOR_VARYING;
            break;
          case GL_PREVIOUS:
            srcExpr = previousVar;
            break;
          default:
              return abort_noSupport("Unsupported combiner src: 0x" + src.toString(16));
        }

        var expr = null;
        switch (op) {
          case GL_SRC_COLOR:
            expr = srcExpr + ".rgb";
            break;
          case GL_ONE_MINUS_SRC_COLOR:
            expr = "(vec3(1.0) - " + srcExpr + ".rgb)";
            break;
          case GL_SRC_ALPHA:
            expr = srcExpr + ".a";
            break;
          case GL_ONE_MINUS_SRC_ALPHA:
            expr = "(1.0 - " + srcExpr + ".a)";
            break;
          default:
            return abort_noSupport("Unsupported combiner op: 0x" + op.toString(16));
        }

        return expr;
      }

      function valToFloatLiteral(val) {
        if (val == Math.round(val)) return val + '.0';
        return val;
      }


      // Classes:
      function CTexEnv() {
        this.mode = GL_MODULATE;
        this.colorCombiner = GL_MODULATE;
        this.alphaCombiner = GL_MODULATE;
        this.colorScale = 1;
        this.alphaScale = 1;
        this.envColor = [0, 0, 0, 0];

        this.colorSrc = [
          GL_TEXTURE,
          GL_PREVIOUS,
          GL_CONSTANT
        ];
        this.alphaSrc = [
          GL_TEXTURE,
          GL_PREVIOUS,
          GL_CONSTANT
        ];
        this.colorOp = [
          GL_SRC_COLOR,
          GL_SRC_COLOR,
          GL_SRC_ALPHA
        ];
        this.alphaOp = [
          GL_SRC_ALPHA,
          GL_SRC_ALPHA,
          GL_SRC_ALPHA
        ];

        // Map GLenums to small values to efficiently pack the enums to bits for tighter access.
        this.traverseKey = {
          // mode
          0x1E01 /* GL_REPLACE */: 0,
          0x2100 /* GL_MODULATE */: 1,
          0x0104 /* GL_ADD */: 2,
          0x0BE2 /* GL_BLEND */: 3,
          0x2101 /* GL_DECAL */: 4,
          0x8570 /* GL_COMBINE */: 5,

          // additional color and alpha combiners
          0x84E7 /* GL_SUBTRACT */: 3,
          0x8575 /* GL_INTERPOLATE */: 4,

          // color and alpha src
          0x1702 /* GL_TEXTURE */: 0,
          0x8576 /* GL_CONSTANT */: 1,
          0x8577 /* GL_PRIMARY_COLOR */: 2,
          0x8578 /* GL_PREVIOUS */: 3,

          // color and alpha op
          0x0300 /* GL_SRC_COLOR */: 0,
          0x0301 /* GL_ONE_MINUS_SRC_COLOR */: 1,
          0x0302 /* GL_SRC_ALPHA */: 2,
          0x0300 /* GL_ONE_MINUS_SRC_ALPHA */: 3
        };

        // The tuple (key0,key1,key2) uniquely identifies the state of the variables in CTexEnv.
        // -1 on key0 denotes 'the whole cached key is dirty'
        this.key0 = -1;
        this.key1 = 0;
        this.key2 = 0;

        this.computeKey0 = function() {
          var k = this.traverseKey;
          var key = k[this.mode] * 1638400; // 6 distinct values.
          key += k[this.colorCombiner] * 327680; // 5 distinct values.
          key += k[this.alphaCombiner] * 65536; // 5 distinct values.
          // The above three fields have 6*5*5=150 distinct values -> 8 bits.
          key += (this.colorScale-1) * 16384; // 10 bits used.
          key += (this.alphaScale-1) * 4096; // 12 bits used.
          key += k[this.colorSrc[0]] * 1024; // 14
          key += k[this.colorSrc[1]] * 256; // 16
          key += k[this.colorSrc[2]] * 64; // 18
          key += k[this.alphaSrc[0]] * 16; // 20
          key += k[this.alphaSrc[1]] * 4; // 22
          key += k[this.alphaSrc[2]]; // 24 bits used total.
          return key;
        }
        this.computeKey1 = function() {
          var k = this.traverseKey;
          key = k[this.colorOp[0]] * 4096;
          key += k[this.colorOp[1]] * 1024;             
          key += k[this.colorOp[2]] * 256;
          key += k[this.alphaOp[0]] * 16;
          key += k[this.alphaOp[1]] * 4;
          key += k[this.alphaOp[2]];
          return key;            
        }
        // TODO: remove this. The color should not be part of the key!
        this.computeKey2 = function() {
          return this.envColor[0] * 16777216 + this.envColor[1] * 65536 + this.envColor[2] * 256 + 1 + this.envColor[3];
        }
        this.recomputeKey = function() {
          this.key0 = this.computeKey0();
          this.key1 = this.computeKey1();
          this.key2 = this.computeKey2();
        }
        this.invalidateKey = function() {
          this.key0 = -1; // The key of this texture unit must be recomputed when rendering the next time.
          GLImmediate.currentRenderer = null; // The currently used renderer must be re-evaluated at next render.
        }
      }

      function CTexUnit() {
        this.env = new CTexEnv();
        this.enabled_tex1D   = false;
        this.enabled_tex2D   = false;
        this.enabled_tex3D   = false;
        this.enabled_texCube = false;
        this.texTypesEnabled = 0; // A bitfield combination of the four flags above, used for fast access to operations.

        this.traverseState = function CTexUnit_traverseState(keyView) {
          if (this.texTypesEnabled) {
            if (this.env.key0 == -1) {
              this.env.recomputeKey();
            }
            keyView.next(this.texTypesEnabled | (this.env.key0 << 4));
            keyView.next(this.env.key1);
            keyView.next(this.env.key2);
          } else {
            // For correctness, must traverse a zero value, theoretically a subsequent integer key could collide with this value otherwise.
            keyView.next(0);
          }
        };
      };

      // Class impls:
      CTexUnit.prototype.enabled = function CTexUnit_enabled() {
        return this.texTypesEnabled;
      }

      CTexUnit.prototype.genPassLines = function CTexUnit_genPassLines(passOutputVar, passInputVar, texUnitID) {
        if (!this.enabled()) {
          return ["vec4 " + passOutputVar + " = " + passInputVar + ";"];
        }
        var lines = this.env.genPassLines(passOutputVar, passInputVar, texUnitID).join('\n');

        var texLoadLines = '';
        var texLoadRegex = /(texture.*?\(.*?\))/g;
        var loadCounter = 0;
        var load;

        // As an optimization, merge duplicate identical texture loads to one var.
        while(load = texLoadRegex.exec(lines)) {
          var texLoadExpr = load[1];
          var secondOccurrence = lines.slice(load.index+1).indexOf(texLoadExpr);
          if (secondOccurrence != -1) { // And also has a second occurrence of same load expression..
            // Create new var to store the common load.
            var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
            var texLoadVar = prefix + 'texload' + loadCounter++;
            var texLoadLine = 'vec4 ' + texLoadVar + ' = ' + texLoadExpr + ';\n';
            texLoadLines += texLoadLine + '\n'; // Store the generated texture load statements in a temp string to not confuse regex search in progress.
            lines = lines.split(texLoadExpr).join(texLoadVar);
            // Reset regex search, since we modified the string.
            texLoadRegex = /(texture.*\(.*\))/g;
          }
        }
        return [texLoadLines + lines];
      }

      CTexUnit.prototype.getTexType = function CTexUnit_getTexType() {
        if (this.enabled_texCube) {
          return GL_TEXTURE_CUBE_MAP;
        } else if (this.enabled_tex3D) {
          return GL_TEXTURE_3D;
        } else if (this.enabled_tex2D) {
          return GL_TEXTURE_2D;
        } else if (this.enabled_tex1D) {
          return GL_TEXTURE_1D;
        }
        return 0;
      }

      CTexEnv.prototype.genPassLines = function CTexEnv_genPassLines(passOutputVar, passInputVar, texUnitID) {
        switch (this.mode) {
          case GL_REPLACE: {
            /* RGB:
             * Cv = Cs
             * Av = Ap // Note how this is different, and that we'll
             *            need to track the bound texture internalFormat
             *            to get this right.
             *
             * RGBA:
             * Cv = Cs
             * Av = As
             */
            return [
              "vec4 " + passOutputVar + " = " + genTexUnitSampleExpr(texUnitID) + ";",
            ];
          }
          case GL_ADD: {
            /* RGBA:
             * Cv = Cp + Cs
             * Av = ApAs
             */
            var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
            var texVar = prefix + "tex";
            var colorVar = prefix + "color";
            var alphaVar = prefix + "alpha";

            return [
              "vec4 " + texVar + " = " + genTexUnitSampleExpr(texUnitID) + ";",
              "vec3 " + colorVar + " = " + passInputVar + ".rgb + " + texVar + ".rgb;",
              "float " + alphaVar + " = " + passInputVar + ".a * " + texVar + ".a;",
              "vec4 " + passOutputVar + " = vec4(" + colorVar + ", " + alphaVar + ");",
            ];
          }
          case GL_MODULATE: {
            /* RGBA:
             * Cv = CpCs
             * Av = ApAs
             */
            var line = [
              "vec4 " + passOutputVar,
              " = ",
                passInputVar,
                " * ",
                genTexUnitSampleExpr(texUnitID),
              ";",
            ];
            return [line.join("")];
          }
          case GL_DECAL: {
            /* RGBA:
             * Cv = Cp(1 - As) + CsAs
             * Av = Ap
             */
            var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
            var texVar = prefix + "tex";
            var colorVar = prefix + "color";
            var alphaVar = prefix + "alpha";

            return [
              "vec4 " + texVar + " = " + genTexUnitSampleExpr(texUnitID) + ";",
              [
                "vec3 " + colorVar + " = ",
                  passInputVar + ".rgb * (1.0 - " + texVar + ".a)",
                    " + ",
                  texVar + ".rgb * " + texVar + ".a",
                ";"
              ].join(""),
              "float " + alphaVar + " = " + passInputVar + ".a;",
              "vec4 " + passOutputVar + " = vec4(" + colorVar + ", " + alphaVar + ");",
            ];
          }
          case GL_BLEND: {
            /* RGBA:
             * Cv = Cp(1 - Cs) + CcCs
             * Av = As
             */
            var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
            var texVar = prefix + "tex";
            var colorVar = prefix + "color";
            var alphaVar = prefix + "alpha";

            return [
              "vec4 " + texVar + " = " + genTexUnitSampleExpr(texUnitID) + ";",
              [
                "vec3 " + colorVar + " = ",
                  passInputVar + ".rgb * (1.0 - " + texVar + ".rgb)",
                    " + ",
                  PRIM_COLOR_VARYING + ".rgb * " + texVar + ".rgb",
                ";"
              ].join(""),
              "float " + alphaVar + " = " + texVar + ".a;",
              "vec4 " + passOutputVar + " = vec4(" + colorVar + ", " + alphaVar + ");",
            ];
          }
          case GL_COMBINE: {
            var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
            var colorVar = prefix + "color";
            var alphaVar = prefix + "alpha";
            var colorLines = this.genCombinerLines(true, colorVar,
                                                   passInputVar, texUnitID,
                                                   this.colorCombiner, this.colorSrc, this.colorOp);
            var alphaLines = this.genCombinerLines(false, alphaVar,
                                                   passInputVar, texUnitID,
                                                   this.alphaCombiner, this.alphaSrc, this.alphaOp);

            // Generate scale, but avoid generating an identity op that multiplies by one.
            var scaledColor = (this.colorScale == 1) ? colorVar : (colorVar + " * " + valToFloatLiteral(this.colorScale));
            var scaledAlpha = (this.alphaScale == 1) ? alphaVar : (alphaVar + " * " + valToFloatLiteral(this.alphaScale));

            var line = [
              "vec4 " + passOutputVar,
              " = ",
                "vec4(",
                    scaledColor,
                    ", ",
                    scaledAlpha,
                ")",
              ";",
            ].join("");
            return [].concat(colorLines, alphaLines, [line]);
          }
        }

        return abort_noSupport("Unsupported TexEnv mode: 0x" + this.mode.toString(16));
      }

      CTexEnv.prototype.genCombinerLines = function CTexEnv_getCombinerLines(isColor, outputVar,
                                                                             passInputVar, texUnitID,
                                                                             combiner, srcArr, opArr)
      {
        var argsNeeded = null;
        switch (combiner) {
          case GL_REPLACE:
            argsNeeded = 1;
            break;

          case GL_MODULATE:
          case GL_ADD:
          case GL_SUBTRACT:
            argsNeeded = 2;
            break;

          case GL_INTERPOLATE:
            argsNeeded = 3;
            break;

          default:
            return abort_noSupport("Unsupported combiner: 0x" + combiner.toString(16));
        }

        var constantExpr = [
          "vec4(",
            valToFloatLiteral(this.envColor[0]),
            ", ",
            valToFloatLiteral(this.envColor[1]),
            ", ",
            valToFloatLiteral(this.envColor[2]),
            ", ",
            valToFloatLiteral(this.envColor[3]),
          ")",
        ].join("");
        var src0Expr = (argsNeeded >= 1) ? genCombinerSourceExpr(texUnitID, constantExpr, passInputVar, srcArr[0], opArr[0])
                                         : null;
        var src1Expr = (argsNeeded >= 2) ? genCombinerSourceExpr(texUnitID, constantExpr, passInputVar, srcArr[1], opArr[1])
                                         : null;
        var src2Expr = (argsNeeded >= 3) ? genCombinerSourceExpr(texUnitID, constantExpr, passInputVar, srcArr[2], opArr[2])
                                         : null;

        var outputType = isColor ? "vec3" : "float";
        var lines = null;
        switch (combiner) {
          case GL_REPLACE: {
            var line = [
              outputType + " " + outputVar,
              " = ",
                src0Expr,
              ";",
            ];
            lines = [line.join("")];
            break;
          }
          case GL_MODULATE: {
            var line = [
              outputType + " " + outputVar + " = ",
                src0Expr + " * " + src1Expr,
              ";",
            ];
            lines = [line.join("")];
            break;
          }
          case GL_ADD: {
            var line = [
              outputType + " " + outputVar + " = ",
                src0Expr + " + " + src1Expr,
              ";",
            ];
            lines = [line.join("")];
            break;
          }
          case GL_SUBTRACT: {
            var line = [
              outputType + " " + outputVar + " = ",
                src0Expr + " - " + src1Expr,
              ";",
            ];
            lines = [line.join("")];
            break;
          }
          case GL_INTERPOLATE: {
            var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + texUnitID + "_";
            var arg2Var = prefix + "colorSrc2";
            var arg2Line = getTypeFromCombineOp(this.colorOp[2]) + " " + arg2Var + " = " + src2Expr + ";";

            var line = [
              outputType + " " + outputVar,
              " = ",
                src0Expr + " * " + arg2Var,
                " + ",
                src1Expr + " * (1.0 - " + arg2Var + ")",
              ";",
            ];
            lines = [
              arg2Line,
              line.join(""),
            ];
            break;
          }

          default:
            return abort_sanity("Unmatched TexEnv.colorCombiner?");
        }

        return lines;
      }

      return {
        // Exports:
        init: function(gl, specifiedMaxTextureImageUnits) {
          var maxTexUnits = 0;
          if (specifiedMaxTextureImageUnits) {
            maxTexUnits = specifiedMaxTextureImageUnits;
          } else if (gl) {
            maxTexUnits = gl.getParameter(gl.MAX_TEXTURE_IMAGE_UNITS);
          }
#if ASSERTIONS
          assert(maxTexUnits > 0);
#endif
          s_texUnits = [];
          for (var i = 0; i < maxTexUnits; i++) {
            s_texUnits.push(new CTexUnit());
          }
        },

        setGLSLVars: function(uTexUnitPrefix, vTexCoordPrefix, vPrimColor, uTexMatrixPrefix) {
          TEX_UNIT_UNIFORM_PREFIX   = uTexUnitPrefix;
          TEX_COORD_VARYING_PREFIX  = vTexCoordPrefix;
          PRIM_COLOR_VARYING        = vPrimColor;
          TEX_MATRIX_UNIFORM_PREFIX = uTexMatrixPrefix;
        },

        genAllPassLines: function(resultDest, indentSize) {
          indentSize = indentSize || 0;

          s_requiredTexUnitsForPass.length = 0; // Clear the list.
          var lines = [];
          var lastPassVar = PRIM_COLOR_VARYING;
          for (var i = 0; i < s_texUnits.length; i++) {
            if (!s_texUnits[i].enabled()) continue;

            s_requiredTexUnitsForPass.push(i);

            var prefix = TEXENVJIT_NAMESPACE_PREFIX + 'env' + i + "_";
            var passOutputVar = prefix + "result";

            var newLines = s_texUnits[i].genPassLines(passOutputVar, lastPassVar, i);
            lines = lines.concat(newLines, [""]);

            lastPassVar = passOutputVar;
          }
          lines.push(resultDest + " = " + lastPassVar + ";");

          var indent = "";
          for (var i = 0; i < indentSize; i++) indent += " ";

          var output = indent + lines.join("\n" + indent);

          return output;
        },

        getUsedTexUnitList: function() {
          return s_requiredTexUnitsForPass;
        },

        traverseState: function(keyView) {
          for (var i = 0; i < s_texUnits.length; i++) {
            s_texUnits[i].traverseState(keyView);
          }
        },

        getTexUnitType: function(texUnitID) {
#if ASSERTIONS
          assert(texUnitID >= 0 &&
                 texUnitID < s_texUnits.length);
#endif
          return s_texUnits[texUnitID].getTexType();
        },

        // Hooks:
        hook_activeTexture: function(texture) {
          s_activeTexture = texture - GL_TEXTURE0;
        },

        hook_enable: function(cap) {
          var cur = getCurTexUnit();
          switch (cap) {
            case GL_TEXTURE_1D:
              if (!cur.enabled_tex1D) {
                GLImmediate.currentRenderer = null; // Renderer state changed, and must be recreated or looked up again.
                cur.enabled_tex1D = true;
                cur.texTypesEnabled |= 1;
              }
              break;
            case GL_TEXTURE_2D:
              if (!cur.enabled_tex2D) {
                GLImmediate.currentRenderer = null;
                cur.enabled_tex2D = true;
                cur.texTypesEnabled |= 2;
              }
              break;
            case GL_TEXTURE_3D:
              if (!cur.enabled_tex3D) {
                GLImmediate.currentRenderer = null;
                cur.enabled_tex3D = true;
                cur.texTypesEnabled |= 4;
              }
              break;
            case GL_TEXTURE_CUBE_MAP:
              if (!cur.enabled_texCube) {
                GLImmediate.currentRenderer = null;
                cur.enabled_texCube = true;
                cur.texTypesEnabled |= 8;
              }
              break;
          }
        },

        hook_disable: function(cap) {
          var cur = getCurTexUnit();
          switch (cap) {
            case GL_TEXTURE_1D:
              if (cur.enabled_tex1D) {
                GLImmediate.currentRenderer = null; // Renderer state changed, and must be recreated or looked up again.
                cur.enabled_tex1D = false;
                cur.texTypesEnabled &= ~1;
              }
              break;
            case GL_TEXTURE_2D:
              if (cur.enabled_tex2D) {
                GLImmediate.currentRenderer = null;
                cur.enabled_tex2D = false;
                cur.texTypesEnabled &= ~2;
              }
              break;
            case GL_TEXTURE_3D:
              if (cur.enabled_tex3D) {
                GLImmediate.currentRenderer = null;
                cur.enabled_tex3D = false;
                cur.texTypesEnabled &= ~4;
              }
              break;
            case GL_TEXTURE_CUBE_MAP:
              if (cur.enabled_texCube) {
                GLImmediate.currentRenderer = null;
                cur.enabled_texCube = false;
                cur.texTypesEnabled &= ~8;
              }
              break;
          }
        },

        hook_texEnvf: function(target, pname, param) {
          if (target != GL_TEXTURE_ENV)
            return;

          var env = getCurTexUnit().env;
          switch (pname) {
            case GL_RGB_SCALE:
              if (env.colorScale != param) {
                env.invalidateKey(); // We changed FFP emulation renderer state.
                env.colorScale = param;
              }
              break;
            case GL_ALPHA_SCALE:
              if (env.alphaScale != param) {
                env.invalidateKey();
                env.alphaScale = param;
              }
              break;

            default:
              Module.printErr('WARNING: Unhandled `pname` in call to `glTexEnvf`.');
          }
        },

        hook_texEnvi: function(target, pname, param) {
          if (target != GL_TEXTURE_ENV)
            return;

          var env = getCurTexUnit().env;
          switch (pname) {
            case GL_TEXTURE_ENV_MODE:
              if (env.mode != param) {
                env.invalidateKey(); // We changed FFP emulation renderer state.
                env.mode = param;
              }
              break;

            case GL_COMBINE_RGB:
              if (env.colorCombiner != param) {
                env.invalidateKey();
                env.colorCombiner = param;
              }
              break;
            case GL_COMBINE_ALPHA:
              if (env.alphaCombiner != param) {
                env.invalidateKey();
                env.alphaCombiner = param;
              }
              break;

            case GL_SRC0_RGB:
              if (env.colorSrc[0] != param) {
                env.invalidateKey();
                env.colorSrc[0] = param;
              }
              break;
            case GL_SRC1_RGB:
              if (env.colorSrc[1] != param) {
                env.invalidateKey();
                env.colorSrc[1] = param;
              }
              break;
            case GL_SRC2_RGB:
              if (env.colorSrc[2] != param) {
                env.invalidateKey();
                env.colorSrc[2] = param;
              }
              break;

            case GL_SRC0_ALPHA:
              if (env.alphaSrc[0] != param) {
                env.invalidateKey();
                env.alphaSrc[0] = param;
              }
              break;
            case GL_SRC1_ALPHA:
              if (env.alphaSrc[1] != param) {
                env.invalidateKey();
                env.alphaSrc[1] = param;
              }
              break;
            case GL_SRC2_ALPHA:
              if (env.alphaSrc[2] != param) {
                env.invalidateKey();
                env.alphaSrc[2] = param;
              }
              break;

            case GL_OPERAND0_RGB:
              if (env.colorOp[0] != param) {
                env.invalidateKey();
                env.colorOp[0] = param;
              }
              break;
            case GL_OPERAND1_RGB:
              if (env.colorOp[1] != param) {
                env.invalidateKey();
                env.colorOp[1] = param;
              }
              break;
            case GL_OPERAND2_RGB:
              if (env.colorOp[2] != param) {
                env.invalidateKey();
                env.colorOp[2] = param;
              }
              break;

            case GL_OPERAND0_ALPHA:
              if (env.alphaOp[0] != param) {
                env.invalidateKey();
                env.alphaOp[0] = param;
              }
              break;
            case GL_OPERAND1_ALPHA:
              if (env.alphaOp[1] != param) {
                env.invalidateKey();
                env.alphaOp[1] = param;
              }
              break;
            case GL_OPERAND2_ALPHA:
              if (env.alphaOp[2] != param) {
                env.invalidateKey();
                env.alphaOp[2] = param;
              }
              break;

            case GL_RGB_SCALE:
              if (env.colorScale != param) {
                env.invalidateKey();
                env.colorScale = param;
              }
              break;
            case GL_ALPHA_SCALE:
              if (env.alphaScale != param) {
                env.invalidateKey();
                env.alphaScale = param;
              }
              break;

            default:
              Module.printErr('WARNING: Unhandled `pname` in call to `glTexEnvi`.');
          }
        },

        hook_texEnvfv: function(target, pname, params) {
          if (target != GL_TEXTURE_ENV) return;

          var env = getCurTexUnit().env;
          switch (pname) {
            case GL_TEXTURE_ENV_COLOR: {
              for (var i = 0; i < 4; i++) {
                var param = {{{ makeGetValue('params', 'i*4', 'float') }}};
                if (env.envColor[i] != param) {
                  env.invalidateKey(); // We changed FFP emulation renderer state.
                  env.envColor[i] = param;
                }
              }
              break
            }
            default:
              Module.printErr('WARNING: Unhandled `pname` in call to `glTexEnvfv`.');
          }
        },

        hook_getTexEnviv: function(target, pname, param) {
          if (target != GL_TEXTURE_ENV)
            return;

          var env = getCurTexUnit().env;
          switch (pname) {
            case GL_TEXTURE_ENV_MODE:
              {{{ makeSetValue('param', '0', 'env.mode', 'i32') }}};
              return;

            case GL_TEXTURE_ENV_COLOR:
              {{{ makeSetValue('param', '0', 'Math.max(Math.min(env.envColor[0]*255, 255, -255))', 'i32') }}};
              {{{ makeSetValue('param', '1', 'Math.max(Math.min(env.envColor[1]*255, 255, -255))', 'i32') }}};
              {{{ makeSetValue('param', '2', 'Math.max(Math.min(env.envColor[2]*255, 255, -255))', 'i32') }}};
              {{{ makeSetValue('param', '3', 'Math.max(Math.min(env.envColor[3]*255, 255, -255))', 'i32') }}};
              return;

            case GL_COMBINE_RGB:
              {{{ makeSetValue('param', '0', 'env.colorCombiner', 'i32') }}};
              return;

            case GL_COMBINE_ALPHA:
              {{{ makeSetValue('param', '0', 'env.alphaCombiner', 'i32') }}};
              return;

            case GL_SRC0_RGB:
              {{{ makeSetValue('param', '0', 'env.colorSrc[0]', 'i32') }}};
              return;

            case GL_SRC1_RGB:
              {{{ makeSetValue('param', '0', 'env.colorSrc[1]', 'i32') }}};
              return;

            case GL_SRC2_RGB:
              {{{ makeSetValue('param', '0', 'env.colorSrc[2]', 'i32') }}};
              return;

            case GL_SRC0_ALPHA:
              {{{ makeSetValue('param', '0', 'env.alphaSrc[0]', 'i32') }}};
              return;

            case GL_SRC1_ALPHA:
              {{{ makeSetValue('param', '0', 'env.alphaSrc[1]', 'i32') }}};
              return;

            case GL_SRC2_ALPHA:
              {{{ makeSetValue('param', '0', 'env.alphaSrc[2]', 'i32') }}};
              return;

            case GL_OPERAND0_RGB:
              {{{ makeSetValue('param', '0', 'env.colorOp[0]', 'i32') }}};
              return;

            case GL_OPERAND1_RGB:
              {{{ makeSetValue('param', '0', 'env.colorOp[1]', 'i32') }}};
              return;

            case GL_OPERAND2_RGB:
              {{{ makeSetValue('param', '0', 'env.colorOp[2]', 'i32') }}};
              return;

            case GL_OPERAND0_ALPHA:
              {{{ makeSetValue('param', '0', 'env.alphaOp[0]', 'i32') }}};
              return;

            case GL_OPERAND1_ALPHA:
              {{{ makeSetValue('param', '0', 'env.alphaOp[1]', 'i32') }}};
              return;

            case GL_OPERAND2_ALPHA:
              {{{ makeSetValue('param', '0', 'env.alphaOp[2]', 'i32') }}};
              return;

            case GL_RGB_SCALE:
              {{{ makeSetValue('param', '0', 'env.colorScale', 'i32') }}};
              return;

            case GL_ALPHA_SCALE:
              {{{ makeSetValue('param', '0', 'env.alphaScale', 'i32') }}};
              return;

            default:
              Module.printErr('WARNING: Unhandled `pname` in call to `glGetTexEnvi`.');
          }
        },

        hook_getTexEnvfv: function(target, pname, param) {
          if (target != GL_TEXTURE_ENV)
            return;

          var env = getCurTexUnit().env;
          switch (pname) {
            case GL_TEXTURE_ENV_COLOR:
              {{{ makeSetValue('param', '0', 'env.envColor[0]', 'float') }}};
              {{{ makeSetValue('param', '4', 'env.envColor[1]', 'float') }}};
              {{{ makeSetValue('param', '8', 'env.envColor[2]', 'float') }}};
              {{{ makeSetValue('param', '12', 'env.envColor[3]', 'float') }}};
              return;
          }
        }
      };
    },

    // Vertex and index data
    vertexData: null, // current vertex data. either tempData (glBegin etc.) or a view into the heap (gl*Pointer). Default view is F32
    vertexDataU8: null, // U8 view
    tempData: null,
    indexData: null,
    vertexCounter: 0,
    mode: -1,

    rendererCache: null,
    rendererComponents: [], // small cache for calls inside glBegin/end. counts how many times the element was seen
    rendererComponentPointer: 0, // next place to start a glBegin/end component
    lastRenderer: null, // used to avoid cleaning up and re-preparing the same renderer
    lastArrayBuffer: null, // used in conjunction with lastRenderer
    lastProgram: null, // ""
    lastStride: -1, // ""

    // The following data structures are used for OpenGL Immediate Mode matrix routines.
    matrix: [],
    matrixStack: [],
    currentMatrix: 0, // 0: modelview, 1: projection, 2+i, texture matrix i.
    tempMatrix: null,
    matricesModified: false,
    useTextureMatrix: false,

    // Clientside attributes
    VERTEX: 0,
    NORMAL: 1,
    COLOR: 2,
    TEXTURE0: 3,
    NUM_ATTRIBUTES: -1, // Initialized in GL emulation init().
    MAX_TEXTURES: -1,   // Initialized in GL emulation init().

    totalEnabledClientAttributes: 0,
    enabledClientAttributes: [0, 0],
    clientAttributes: [], // raw data, including possible unneeded ones
    liveClientAttributes: [], // the ones actually alive in the current computation, sorted
    currentRenderer: null, // Caches the currently active FFP emulation renderer, so that it does not have to be re-looked up unless relevant state changes.
    modifiedClientAttributes: false,
    clientActiveTexture: 0,
    clientColor: null,
    usedTexUnitList: [],
    fixedFunctionProgram: null,

    setClientAttribute: function setClientAttribute(name, size, type, stride, pointer) {
      var attrib = GLImmediate.clientAttributes[name];
      if (!attrib) {
        for (var i = 0; i <= name; i++) { // keep flat
          if (!GLImmediate.clientAttributes[i]) {
            GLImmediate.clientAttributes[i] = {
              name: name,
              size: size,
              type: type,
              stride: stride,
              pointer: pointer,
              offset: 0
            };
          }
        }
      } else {
        attrib.name = name;
        attrib.size = size;
        attrib.type = type;
        attrib.stride = stride;
        attrib.pointer = pointer;
        attrib.offset = 0;
      }
      GLImmediate.modifiedClientAttributes = true;
    },

    // Renderers
    addRendererComponent: function addRendererComponent(name, size, type) {
      if (!GLImmediate.rendererComponents[name]) {
        GLImmediate.rendererComponents[name] = 1;
#if ASSERTIONS
        if (GLImmediate.enabledClientAttributes[name]) {
          console.log("Warning: glTexCoord used after EnableClientState for TEXTURE_COORD_ARRAY for TEXTURE0. Disabling TEXTURE_COORD_ARRAY...");
        }
#endif
        GLImmediate.enabledClientAttributes[name] = true;
        GLImmediate.setClientAttribute(name, size, type, 0, GLImmediate.rendererComponentPointer);
        GLImmediate.rendererComponentPointer += size * GL.byteSizeByType[type - GL.byteSizeByTypeRoot];
#if GL_FFP_ONLY
        // We can enable the correct attribute stream index immediately here, since the same attribute in each shader
        // will be bound to this same index.
        GL.enableVertexAttribArray(name);
#endif
      } else {
        GLImmediate.rendererComponents[name]++;
      }
    },

    disableBeginEndClientAttributes: function disableBeginEndClientAttributes() {
      for (var i = 0; i < GLImmediate.NUM_ATTRIBUTES; i++) {
        if (GLImmediate.rendererComponents[i]) GLImmediate.enabledClientAttributes[i] = false;
      }
    },

    getRenderer: function getRenderer() {
      // If no FFP state has changed that would have forced to re-evaluate which FFP emulation shader to use,
      // we have the currently used renderer in cache, and can immediately return that.
      if (GLImmediate.currentRenderer) {
        return GLImmediate.currentRenderer;
      }
      // return a renderer object given the liveClientAttributes
      // we maintain a cache of renderers, optimized to not generate garbage
      var attributes = GLImmediate.liveClientAttributes;
      var cacheMap = GLImmediate.rendererCache;
      var keyView = cacheMap.getStaticKeyView().reset();

      // By attrib state:
      var enabledAttributesKey = 0;
      for (var i = 0; i < attributes.length; i++) {
        enabledAttributesKey |= 1 << attributes[i].name;
      }

      // By fog state:
      var fogParam = 0;
      if (GLEmulation.fogEnabled) {
        switch (GLEmulation.fogMode) {
          case 0x0801: // GL_EXP2
            fogParam = 1;
            break;
          case 0x2601: // GL_LINEAR
            fogParam = 2;
            break;
          default: // default to GL_EXP
            fogParam = 3;
            break;
        }
      }
      keyView.next((enabledAttributesKey << 2) | fogParam);

#if !GL_FFP_ONLY
      // By cur program:
      keyView.next(GL.currProgram);
      if (!GL.currProgram) {
#endif
        GLImmediate.TexEnvJIT.traverseState(keyView);
#if !GL_FFP_ONLY
      }
#endif

      // If we don't already have it, create it.
      var renderer = keyView.get();
      if (!renderer) {
#if GL_DEBUG
        Module.printErr('generating renderer for ' + JSON.stringify(attributes));
#endif
        renderer = GLImmediate.createRenderer();
        GLImmediate.currentRenderer = renderer;
        keyView.set(renderer);
        return renderer;
      }
      GLImmediate.currentRenderer = renderer; // Cache the currently used renderer, so later lookups without state changes can get this fast.
      return renderer;
    },

    createRenderer: function createRenderer(renderer) {
      var useCurrProgram = !!GL.currProgram;
      var hasTextures = false;
      for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
        var texAttribName = GLImmediate.TEXTURE0 + i;
        if (!GLImmediate.enabledClientAttributes[texAttribName])
          continue;

#if ASSERTIONS
        if (!useCurrProgram) {
          if (GLImmediate.TexEnvJIT.getTexUnitType(i) == 0) {
             Runtime.warnOnce("GL_TEXTURE" + i + " coords are supplied, but that texture unit is disabled in the fixed-function pipeline.");
          }
        }
#endif

        hasTextures = true;
      }

      var ret = {
        init: function init() {
          // For fixed-function shader generation.
          var uTexUnitPrefix = 'u_texUnit';
          var aTexCoordPrefix = 'a_texCoord';
          var vTexCoordPrefix = 'v_texCoord';
          var vPrimColor = 'v_color';
          var uTexMatrixPrefix = GLImmediate.useTextureMatrix ? 'u_textureMatrix' : null;

          if (useCurrProgram) {
            if (GL.shaderInfos[GL.programShaders[GL.currProgram][0]].type == GLctx.VERTEX_SHADER) {
              this.vertexShader = GL.shaders[GL.programShaders[GL.currProgram][0]];
              this.fragmentShader = GL.shaders[GL.programShaders[GL.currProgram][1]];
            } else {
              this.vertexShader = GL.shaders[GL.programShaders[GL.currProgram][1]];
              this.fragmentShader = GL.shaders[GL.programShaders[GL.currProgram][0]];
            }
            this.program = GL.programs[GL.currProgram];
            this.usedTexUnitList = [];
          } else {
            // IMPORTANT NOTE: If you parameterize the shader source based on any runtime values
            // in order to create the least expensive shader possible based on the features being
            // used, you should also update the code in the beginning of getRenderer to make sure
            // that you cache the renderer based on the said parameters.
            if (GLEmulation.fogEnabled) {
              switch (GLEmulation.fogMode) {
                case 0x0801: // GL_EXP2
                  // fog = exp(-(gl_Fog.density * gl_FogFragCoord)^2)
                  var fogFormula = '  float fog = exp(-u_fogDensity * u_fogDensity * ecDistance * ecDistance); \n';
                  break;
                case 0x2601: // GL_LINEAR
                  // fog = (gl_Fog.end - gl_FogFragCoord) * gl_fog.scale
                  var fogFormula = '  float fog = (u_fogEnd - ecDistance) * u_fogScale; \n';
                  break;
                default: // default to GL_EXP
                  // fog = exp(-gl_Fog.density * gl_FogFragCoord)
                  var fogFormula = '  float fog = exp(-u_fogDensity * ecDistance); \n';
                  break;
              }
            }

            GLImmediate.TexEnvJIT.setGLSLVars(uTexUnitPrefix, vTexCoordPrefix, vPrimColor, uTexMatrixPrefix);
            var fsTexEnvPass = GLImmediate.TexEnvJIT.genAllPassLines('gl_FragColor', 2);

            var texUnitAttribList = '';
            var texUnitVaryingList = '';
            var texUnitUniformList = '';
            var vsTexCoordInits = '';
            this.usedTexUnitList = GLImmediate.TexEnvJIT.getUsedTexUnitList();
            for (var i = 0; i < this.usedTexUnitList.length; i++) {
              var texUnit = this.usedTexUnitList[i];
              texUnitAttribList += 'attribute vec4 ' + aTexCoordPrefix + texUnit + ';\n';
              texUnitVaryingList += 'varying vec4 ' + vTexCoordPrefix + texUnit + ';\n';
              texUnitUniformList += 'uniform sampler2D ' + uTexUnitPrefix + texUnit + ';\n';
              vsTexCoordInits += '  ' + vTexCoordPrefix + texUnit + ' = ' + aTexCoordPrefix + texUnit + ';\n';

              if (GLImmediate.useTextureMatrix) {
                texUnitUniformList += 'uniform mat4 ' + uTexMatrixPrefix + texUnit + ';\n';
              }
            }

            var vsFogVaryingInit = null;
            if (GLEmulation.fogEnabled) {
              vsFogVaryingInit = '  v_fogFragCoord = abs(ecPosition.z);\n';
            }

            var vsSource = [
              'attribute vec4 a_position;',
              'attribute vec4 a_color;',
              'varying vec4 v_color;',
              texUnitAttribList,
              texUnitVaryingList,
              (GLEmulation.fogEnabled ? 'varying float v_fogFragCoord;' : null),
              'uniform mat4 u_modelView;',
              'uniform mat4 u_projection;',
              'void main()',
              '{',
              '  vec4 ecPosition = u_modelView * a_position;', // eye-coordinate position
              '  gl_Position = u_projection * ecPosition;',
              '  v_color = a_color;',
              vsTexCoordInits,
              vsFogVaryingInit,
              '}',
              ''
            ].join('\n').replace(/\n\n+/g, '\n');

            this.vertexShader = GLctx.createShader(GLctx.VERTEX_SHADER);
            GLctx.shaderSource(this.vertexShader, vsSource);
            GLctx.compileShader(this.vertexShader);

            var fogHeaderIfNeeded = null;
            if (GLEmulation.fogEnabled) {
              fogHeaderIfNeeded = [
                '',
                'varying float v_fogFragCoord; ',
                'uniform vec4 u_fogColor;      ',
                'uniform float u_fogEnd;       ',
                'uniform float u_fogScale;     ',
                'uniform float u_fogDensity;   ',
                'float ffog(in float ecDistance) { ',
                fogFormula,
                '  fog = clamp(fog, 0.0, 1.0); ',
                '  return fog;                 ',
                '}',
                '',
              ].join("\n");
            }

            var fogPass = null;
            if (GLEmulation.fogEnabled) {
              fogPass = 'gl_FragColor = vec4(mix(u_fogColor.rgb, gl_FragColor.rgb, ffog(v_fogFragCoord)), gl_FragColor.a);\n';
            }

            var fsSource = [
              'precision mediump float;',
              texUnitVaryingList,
              texUnitUniformList,
              'varying vec4 v_color;',
              fogHeaderIfNeeded,
              'void main()',
              '{',
              fsTexEnvPass,
              fogPass,
              '}',
              ''
            ].join("\n").replace(/\n\n+/g, '\n');

            this.fragmentShader = GLctx.createShader(GLctx.FRAGMENT_SHADER);
            GLctx.shaderSource(this.fragmentShader, fsSource);
            GLctx.compileShader(this.fragmentShader);

            this.program = GLctx.createProgram();
            GLctx.attachShader(this.program, this.vertexShader);
            GLctx.attachShader(this.program, this.fragmentShader);

            // As optimization, bind all attributes to prespecified locations, so that the FFP emulation
            // code can submit attributes to any generated FFP shader without having to examine each shader in turn.
            // These prespecified locations are only assumed if GL_FFP_ONLY is specified, since user could also create their
            // own shaders that didn't have attributes in the same locations.
            GLctx.bindAttribLocation(this.program, GLImmediate.VERTEX, 'a_position');
            GLctx.bindAttribLocation(this.program, GLImmediate.COLOR, 'a_color');
            GLctx.bindAttribLocation(this.program, GLImmediate.NORMAL, 'a_normal');
            var maxVertexAttribs = GLctx.getParameter(GLctx.MAX_VERTEX_ATTRIBS);
            for (var i = 0; i < GLImmediate.MAX_TEXTURES && GLImmediate.TEXTURE0 + i < maxVertexAttribs; i++) {
              GLctx.bindAttribLocation(this.program, GLImmediate.TEXTURE0 + i, 'a_texCoord'+i);
              GLctx.bindAttribLocation(this.program, GLImmediate.TEXTURE0 + i, aTexCoordPrefix+i);
            }
            GLctx.linkProgram(this.program);
          }

          // Stores an array that remembers which matrix uniforms are up-to-date in this FFP renderer, so they don't need to be resubmitted
          // each time we render with this program.
          this.textureMatrixVersion = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ];

          this.positionLocation = GLctx.getAttribLocation(this.program, 'a_position');

          this.texCoordLocations = [];

          for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
            if (!GLImmediate.enabledClientAttributes[GLImmediate.TEXTURE0 + i]) {
              this.texCoordLocations[i] = -1;
              continue;
            }

            if (useCurrProgram) {
              this.texCoordLocations[i] = GLctx.getAttribLocation(this.program, 'a_texCoord' + i);
            } else {
              this.texCoordLocations[i] = GLctx.getAttribLocation(this.program, aTexCoordPrefix + i);
            }
          }
          this.colorLocation = GLctx.getAttribLocation(this.program, 'a_color');
          if (!useCurrProgram) {
            // Temporarily switch to the program so we can set our sampler uniforms early.
            var prevBoundProg = GLctx.getParameter(GLctx.CURRENT_PROGRAM);
            GLctx.useProgram(this.program);
            {
              for (var i = 0; i < this.usedTexUnitList.length; i++) {
                var texUnitID = this.usedTexUnitList[i];
                var texSamplerLoc = GLctx.getUniformLocation(this.program, uTexUnitPrefix + texUnitID);
                GLctx.uniform1i(texSamplerLoc, texUnitID);
              }
            }
            // The default color attribute value is not the same as the default for all other attribute streams (0,0,0,1) but (1,1,1,1),
            // so explicitly set it right at start.
            GLctx.vertexAttrib4fv(this.colorLocation, [1,1,1,1]);
            GLctx.useProgram(prevBoundProg);
          }

          this.textureMatrixLocations = [];
          for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
            this.textureMatrixLocations[i] = GLctx.getUniformLocation(this.program, 'u_textureMatrix' + i);
          }
          this.normalLocation = GLctx.getAttribLocation(this.program, 'a_normal');

          this.modelViewLocation = GLctx.getUniformLocation(this.program, 'u_modelView');
          this.projectionLocation = GLctx.getUniformLocation(this.program, 'u_projection');

          this.hasTextures = hasTextures;
          this.hasNormal = GLImmediate.enabledClientAttributes[GLImmediate.NORMAL] &&
                           GLImmediate.clientAttributes[GLImmediate.NORMAL].size > 0 &&
                           this.normalLocation >= 0;
          this.hasColor = (this.colorLocation === 0) || this.colorLocation > 0;

          this.floatType = GLctx.FLOAT; // minor optimization

          this.fogColorLocation = GLctx.getUniformLocation(this.program, 'u_fogColor');
          this.fogEndLocation = GLctx.getUniformLocation(this.program, 'u_fogEnd');
          this.fogScaleLocation = GLctx.getUniformLocation(this.program, 'u_fogScale');
          this.fogDensityLocation = GLctx.getUniformLocation(this.program, 'u_fogDensity');
          this.hasFog = !!(this.fogColorLocation || this.fogEndLocation ||
                           this.fogScaleLocation || this.fogDensityLocation);
        },

        prepare: function prepare() {
          // Calculate the array buffer
          var arrayBuffer;
          if (!GL.currArrayBuffer) {
            var start = GLImmediate.firstVertex*GLImmediate.stride;
            var end = GLImmediate.lastVertex*GLImmediate.stride;
#if ASSERTIONS
            assert(end <= GL.MAX_TEMP_BUFFER_SIZE, 'too much vertex data');
#endif
            arrayBuffer = GL.getTempVertexBuffer(end);
            // TODO: consider using the last buffer we bound, if it was larger. downside is larger buffer, but we might avoid rebinding and preparing
          } else {
            arrayBuffer = GL.currArrayBuffer;
          }

#if GL_UNSAFE_OPTS
          // If the array buffer is unchanged and the renderer as well, then we can avoid all the work here
          // XXX We use some heuristics here, and this may not work in all cases. Try disabling GL_UNSAFE_OPTS if you
          // have odd glitches
          var lastRenderer = GLImmediate.lastRenderer;
          var canSkip = this == lastRenderer &&
                        arrayBuffer == GLImmediate.lastArrayBuffer &&
                        (GL.currProgram || this.program) == GLImmediate.lastProgram &&
                        GLImmediate.stride == GLImmediate.lastStride &&
                        !GLImmediate.matricesModified;
          if (!canSkip && lastRenderer) lastRenderer.cleanup();
#endif
          if (!GL.currArrayBuffer) {
            // Bind the array buffer and upload data after cleaning up the previous renderer

            if (arrayBuffer != GLImmediate.lastArrayBuffer) {
              GLctx.bindBuffer(GLctx.ARRAY_BUFFER, arrayBuffer);
              GLImmediate.lastArrayBuffer = arrayBuffer;
            }

            GLctx.bufferSubData(GLctx.ARRAY_BUFFER, start, GLImmediate.vertexData.subarray(start >> 2, end >> 2));
          }
#if GL_UNSAFE_OPTS
          if (canSkip) return;
          GLImmediate.lastRenderer = this;
          GLImmediate.lastProgram = GL.currProgram || this.program;
          GLImmediate.lastStride == GLImmediate.stride;
          GLImmediate.matricesModified = false;
#endif

          if (!GL.currProgram) {
            if (GLImmediate.fixedFunctionProgram != this.program) {
              GLctx.useProgram(this.program);
              GLImmediate.fixedFunctionProgram = this.program;
            }
          }

          if (this.modelViewLocation && this.modelViewMatrixVersion != GLImmediate.matrixVersion[0/*m*/]) {
            this.modelViewMatrixVersion = GLImmediate.matrixVersion[0/*m*/];
            GLctx.uniformMatrix4fv(this.modelViewLocation, false, GLImmediate.matrix[0/*m*/]);
          }
          if (this.projectionLocation && this.projectionMatrixVersion != GLImmediate.matrixVersion[1/*p*/]) {
            this.projectionMatrixVersion = GLImmediate.matrixVersion[1/*p*/];
            GLctx.uniformMatrix4fv(this.projectionLocation, false, GLImmediate.matrix[1/*p*/]);
          }

          var clientAttributes = GLImmediate.clientAttributes;
          var posAttr = clientAttributes[GLImmediate.VERTEX];

#if GL_ASSERTIONS
          GL.validateVertexAttribPointer(posAttr.size, posAttr.type, GLImmediate.stride, clientAttributes[GLImmediate.VERTEX].offset);
#endif

#if GL_FFP_ONLY
          if (!GL.currArrayBuffer) {
            GLctx.vertexAttribPointer(GLImmediate.VERTEX, posAttr.size, posAttr.type, false, GLImmediate.stride, posAttr.offset);
            if (this.hasNormal) {
              var normalAttr = clientAttributes[GLImmediate.NORMAL];
              GLctx.vertexAttribPointer(GLImmediate.NORMAL, normalAttr.size, normalAttr.type, true, GLImmediate.stride, normalAttr.offset);
            }
          }
#else
          GLctx.vertexAttribPointer(this.positionLocation, posAttr.size, posAttr.type, false, GLImmediate.stride, posAttr.offset);
          GLctx.enableVertexAttribArray(this.positionLocation);
          if (this.hasNormal) {
            var normalAttr = clientAttributes[GLImmediate.NORMAL];
#if GL_ASSERTIONS
            GL.validateVertexAttribPointer(normalAttr.size, normalAttr.type, GLImmediate.stride, normalAttr.offset);
#endif
            GLctx.vertexAttribPointer(this.normalLocation, normalAttr.size, normalAttr.type, true, GLImmediate.stride, normalAttr.offset);
            GLctx.enableVertexAttribArray(this.normalLocation);
          }
#endif
          if (this.hasTextures) {
            for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
#if GL_FFP_ONLY
              if (!GL.currArrayBuffer) {
                var attribLoc = GLImmediate.TEXTURE0+i;
                var texAttr = clientAttributes[attribLoc];
                if (texAttr.size) {
                  GLctx.vertexAttribPointer(attribLoc, texAttr.size, texAttr.type, false, GLImmediate.stride, texAttr.offset);
                } else {
                  // These two might be dangerous, but let's try them.
                  GLctx.vertexAttrib4f(attribLoc, 0, 0, 0, 1);
                }
              }
#else
              var attribLoc = this.texCoordLocations[i];
              if (attribLoc === undefined || attribLoc < 0) continue;
              var texAttr = clientAttributes[GLImmediate.TEXTURE0+i];

              if (texAttr.size) {
#if GL_ASSERTIONS
                GL.validateVertexAttribPointer(texAttr.size, texAttr.type, GLImmediate.stride, texAttr.offset);
#endif
                GLctx.vertexAttribPointer(attribLoc, texAttr.size, texAttr.type, false, GLImmediate.stride, texAttr.offset);
                GLctx.enableVertexAttribArray(attribLoc);
              } else {
                // These two might be dangerous, but let's try them.
                GLctx.vertexAttrib4f(attribLoc, 0, 0, 0, 1);
                GLctx.disableVertexAttribArray(attribLoc);
              }
#endif
              var t = 2/*t*/+i;
              if (this.textureMatrixLocations[i] && this.textureMatrixVersion[t] != GLImmediate.matrixVersion[t]) { // XXX might we need this even without the condition we are currently in?
                this.textureMatrixVersion[t] = GLImmediate.matrixVersion[t];
                GLctx.uniformMatrix4fv(this.textureMatrixLocations[i], false, GLImmediate.matrix[t]);
              }
            }
          }
          if (GLImmediate.enabledClientAttributes[GLImmediate.COLOR]) {
            var colorAttr = clientAttributes[GLImmediate.COLOR];
#if GL_ASSERTIONS
            GL.validateVertexAttribPointer(colorAttr.size, colorAttr.type, GLImmediate.stride, colorAttr.offset);
#endif
#if GL_FFP_ONLY
            if (!GL.currArrayBuffer) {
              GLctx.vertexAttribPointer(GLImmediate.COLOR, colorAttr.size, colorAttr.type, true, GLImmediate.stride, colorAttr.offset);
            }
#else
            GLctx.vertexAttribPointer(this.colorLocation, colorAttr.size, colorAttr.type, true, GLImmediate.stride, colorAttr.offset);
            GLctx.enableVertexAttribArray(this.colorLocation);
#endif
          }
#if !GL_FFP_ONLY
          else if (this.hasColor) {
            GLctx.disableVertexAttribArray(this.colorLocation);
            GLctx.vertexAttrib4fv(this.colorLocation, GLImmediate.clientColor);
          }
#endif
          if (this.hasFog) {
            if (this.fogColorLocation) GLctx.uniform4fv(this.fogColorLocation, GLEmulation.fogColor);
            if (this.fogEndLocation) GLctx.uniform1f(this.fogEndLocation, GLEmulation.fogEnd);
            if (this.fogScaleLocation) GLctx.uniform1f(this.fogScaleLocation, 1/(GLEmulation.fogEnd - GLEmulation.fogStart));
            if (this.fogDensityLocation) GLctx.uniform1f(this.fogDensityLocation, GLEmulation.fogDensity);
          }
        },

        cleanup: function cleanup() {
#if !GL_FFP_ONLY
          GLctx.disableVertexAttribArray(this.positionLocation);
          if (this.hasTextures) {
            for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
              if (GLImmediate.enabledClientAttributes[GLImmediate.TEXTURE0+i] && this.texCoordLocations[i] >= 0) {
                GLctx.disableVertexAttribArray(this.texCoordLocations[i]);
              }
            }
          }
          if (this.hasColor) {
            GLctx.disableVertexAttribArray(this.colorLocation);
          }
          if (this.hasNormal) {
            GLctx.disableVertexAttribArray(this.normalLocation);
          }
          if (!GL.currProgram) {
            GLctx.useProgram(null);
            GLImmediate.fixedFunctionProgram = 0;
          }
          if (!GL.currArrayBuffer) {
            GLctx.bindBuffer(GLctx.ARRAY_BUFFER, null);
            GLImmediate.lastArrayBuffer = null;
          }

#if GL_UNSAFE_OPTS
          GLImmediate.lastRenderer = null;
          GLImmediate.lastProgram = null;
#endif
          GLImmediate.matricesModified = true;
#endif
        }
      };
      ret.init();
      return ret;
    },

    setupFuncs: function() {
      // Replace some functions with immediate-mode aware versions. If there are no client
      // attributes enabled, and we use webgl-friendly modes (no GL_QUADS), then no need
      // for emulation
      _glDrawArrays = _emscripten_glDrawArrays = function _glDrawArrays(mode, first, count) {
        if (GLImmediate.totalEnabledClientAttributes == 0 && mode <= 6) {
          GLctx.drawArrays(mode, first, count);
          return;
        }
        GLImmediate.prepareClientAttributes(count, false);
        GLImmediate.mode = mode;
        if (!GL.currArrayBuffer) {
          GLImmediate.vertexData = {{{ makeHEAPView('F32', 'GLImmediate.vertexPointer', 'GLImmediate.vertexPointer + (first+count)*GLImmediate.stride') }}}; // XXX assuming float
          GLImmediate.firstVertex = first;
          GLImmediate.lastVertex = first + count;
        }
        GLImmediate.flush(null, first);
        GLImmediate.mode = -1;
      };

      _glDrawElements = _emscripten_glDrawElements = function _glDrawElements(mode, count, type, indices, start, end) { // start, end are given if we come from glDrawRangeElements
        if (GLImmediate.totalEnabledClientAttributes == 0 && mode <= 6 && GL.currElementArrayBuffer) {
          GLctx.drawElements(mode, count, type, indices);
          return;
        }
#if ASSERTIONS
        if (!GL.currElementArrayBuffer) {
          assert(type == GLctx.UNSIGNED_SHORT); // We can only emulate buffers of this kind, for now
        }
        console.log("DrawElements doesn't actually prepareClientAttributes properly.");
#endif
        GLImmediate.prepareClientAttributes(count, false);
        GLImmediate.mode = mode;
        if (!GL.currArrayBuffer) {
          GLImmediate.firstVertex = end ? start : TOTAL_MEMORY; // if we don't know the start, set an invalid value and we will calculate it later from the indices
          GLImmediate.lastVertex = end ? end+1 : 0;
          GLImmediate.vertexData = {{{ makeHEAPView('F32', 'GLImmediate.vertexPointer', '(end ? GLImmediate.vertexPointer + (end+1)*GLImmediate.stride : TOTAL_MEMORY)') }}}; // XXX assuming float
        }
        GLImmediate.flush(count, 0, indices);
        GLImmediate.mode = -1;
      };

      // TexEnv stuff needs to be prepared early, so do it here.
      // init() is too late for -O2, since it freezes the GL functions
      // by that point.
      GLImmediate.MapTreeLib = GLImmediate.spawnMapTreeLib();
      GLImmediate.spawnMapTreeLib = null;

      GLImmediate.TexEnvJIT = GLImmediate.spawnTexEnvJIT();
      GLImmediate.spawnTexEnvJIT = null;

      GLImmediate.setupHooks();
    },

    setupHooks: function() {
      if (!GLEmulation.hasRunInit) {
        GLEmulation.init();
      }

      var glActiveTexture = _glActiveTexture;
      _glActiveTexture = _emscripten_glActiveTexture = function _glActiveTexture(texture) {
        GLImmediate.TexEnvJIT.hook_activeTexture(texture);
        glActiveTexture(texture);
      };

      var glEnable = _glEnable;
      _glEnable = _emscripten_glEnable = function _glEnable(cap) {
        GLImmediate.TexEnvJIT.hook_enable(cap);
        glEnable(cap);
      };
      var glDisable = _glDisable;
      _glDisable = _emscripten_glDisable = function _glDisable(cap) {
        GLImmediate.TexEnvJIT.hook_disable(cap);
        glDisable(cap);
      };

      var glTexEnvf = (typeof(_glTexEnvf) != 'undefined') ? _glTexEnvf : function(){};
      _glTexEnvf = _emscripten_glTexEnvf = function _glTexEnvf(target, pname, param) {
        GLImmediate.TexEnvJIT.hook_texEnvf(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvf(target, pname, param);
      };
      var glTexEnvi = (typeof(_glTexEnvi) != 'undefined') ? _glTexEnvi : function(){};
      _glTexEnvi = _emscripten_glTexEnvi = function _glTexEnvi(target, pname, param) {
        GLImmediate.TexEnvJIT.hook_texEnvi(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvi(target, pname, param);
      };
      var glTexEnvfv = (typeof(_glTexEnvfv) != 'undefined') ? _glTexEnvfv : function(){};
      _glTexEnvfv = _emscripten_glTexEnvfv = function _glTexEnvfv(target, pname, param) {
        GLImmediate.TexEnvJIT.hook_texEnvfv(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvfv(target, pname, param);
      };

      _glGetTexEnviv = function _glGetTexEnviv(target, pname, param) {
        GLImmediate.TexEnvJIT.hook_getTexEnviv(target, pname, param);
      };

      _glGetTexEnvfv = function _glGetTexEnvfv(target, pname, param) {
        GLImmediate.TexEnvJIT.hook_getTexEnvfv(target, pname, param);
      };

      var glGetIntegerv = _glGetIntegerv;
      _glGetIntegerv = _emscripten_glGetIntegerv = function _glGetIntegerv(pname, params) {
        switch (pname) {
          case 0x8B8D: { // GL_CURRENT_PROGRAM
            // Just query directly so we're working with WebGL objects.
            var cur = GLctx.getParameter(GLctx.CURRENT_PROGRAM);
            if (cur == GLImmediate.fixedFunctionProgram) {
              // Pretend we're not using a program.
              {{{ makeSetValue('params', '0', '0', 'i32') }}};
              return;
            }
            break;
          }
        }
        glGetIntegerv(pname, params);
      };
    },

    // Main functions
    initted: false,
    init: function() {
      Module.printErr('WARNING: using emscripten GL immediate mode emulation. This is very limited in what it supports');
      GLImmediate.initted = true;

      if (!Module.useWebGL) return; // a 2D canvas may be currently used TODO: make sure we are actually called in that case

      // User can override the maximum number of texture units that we emulate. Using fewer texture units increases runtime performance
      // slightly, so it is advantageous to choose as small value as needed.
      GLImmediate.MAX_TEXTURES = Module['GL_MAX_TEXTURE_IMAGE_UNITS'] || GLctx.getParameter(GLctx.MAX_TEXTURE_IMAGE_UNITS);

      GLImmediate.TexEnvJIT.init(GLctx, GLImmediate.MAX_TEXTURES);

      GLImmediate.NUM_ATTRIBUTES = 3 /*pos+normal+color attributes*/ + GLImmediate.MAX_TEXTURES;
      GLImmediate.clientAttributes = [];
      GLEmulation.enabledClientAttribIndices = [];
      for (var i = 0; i < GLImmediate.NUM_ATTRIBUTES; i++) {
        GLImmediate.clientAttributes.push({});
        GLEmulation.enabledClientAttribIndices.push(false);
      }

      // Initialize matrix library
      // When user sets a matrix, increment a 'version number' on the new data, and when rendering, submit
      // the matrices to the shader program only if they have an old version of the data.
      GLImmediate.matrix = [];
      GLImmediate.matrixStack = [];
      GLImmediate.matrixVersion = [];
      for (var i = 0; i < 2 + GLImmediate.MAX_TEXTURES; i++) { // Modelview, Projection, plus one matrix for each texture coordinate.
        GLImmediate.matrixStack.push([]);
        GLImmediate.matrixVersion.push(0);
        GLImmediate.matrix.push(GLImmediate.matrixLib.mat4.create());
        GLImmediate.matrixLib.mat4.identity(GLImmediate.matrix[i]);
      }

      // Renderer cache
      GLImmediate.rendererCache = GLImmediate.MapTreeLib.create();

      // Buffers for data
      GLImmediate.tempData = new Float32Array(GL.MAX_TEMP_BUFFER_SIZE >> 2);
      GLImmediate.indexData = new Uint16Array(GL.MAX_TEMP_BUFFER_SIZE >> 1);

      GLImmediate.vertexDataU8 = new Uint8Array(GLImmediate.tempData.buffer);

      GL.generateTempBuffers(true, GL.currentContext);

      GLImmediate.clientColor = new Float32Array([1, 1, 1, 1]);
    },

    // Prepares and analyzes client attributes.
    // Modifies liveClientAttributes, stride, vertexPointer, vertexCounter
    //   count: number of elements we will draw
    //   beginEnd: whether we are drawing the results of a begin/end block
    prepareClientAttributes: function prepareClientAttributes(count, beginEnd) {
      // If no client attributes were modified since we were last called, do nothing. Note that this
      // does not work for glBegin/End, where we generate renderer components dynamically and then
      // disable them ourselves, but it does help with glDrawElements/Arrays.
      if (!GLImmediate.modifiedClientAttributes) {
#if GL_ASSERTIONS
        if ((GLImmediate.stride & 3) != 0) {
          Runtime.warnOnce('Warning: Rendering from client side vertex arrays where stride (' + GLImmediate.stride + ') is not a multiple of four! This is not currently supported!');
        }
#endif
        GLImmediate.vertexCounter = (GLImmediate.stride * count) / 4; // XXX assuming float
        return;
      }
      GLImmediate.modifiedClientAttributes = false;

      // The role of prepareClientAttributes is to examine the set of client-side vertex attribute buffers
      // that user code has submitted, and to prepare them to be uploaded to a VBO in GPU memory
      // (since WebGL does not support client-side rendering, i.e. rendering from vertex data in CPU memory)
      // User can submit vertex data generally in three different configurations:
      // 1. Fully planar: all attributes are in their own separate tightly-packed arrays in CPU memory.
      // 2. Fully interleaved: all attributes share a single array where data is interleaved something like (pos,uv,normal), (pos,uv,normal), ...
      // 3. Complex hybrid: Multiple separate arrays that either are sparsely strided, and/or partially interleave vertex attributes.

      // For simplicity, we support the case (2) as the fast case. For (1) and (3), we do a memory copy of the
      // vertex data here to prepare a relayouted buffer that is of the structure in case (2). The reason
      // for this is that it allows the emulation code to get away with using just one VBO buffer for rendering,
      // and not have to maintain multiple ones. Therefore cases (1) and (3) will be very slow, and case (2) is fast.

      // Detect which case we are in by using a quick heuristic by examining the strides of the buffers. If all the buffers have identical 
      // stride, we assume we have case (2), otherwise we have something more complex.
      var clientStartPointer = 0x7FFFFFFF;
      var bytes = 0; // Total number of bytes taken up by a single vertex.
      var minStride = 0x7FFFFFFF;
      var maxStride = 0;
      var attributes = GLImmediate.liveClientAttributes;
      attributes.length = 0;
      for (var i = 0; i < 3+GLImmediate.MAX_TEXTURES; i++) {
        if (GLImmediate.enabledClientAttributes[i]) {
          var attr = GLImmediate.clientAttributes[i];
          attributes.push(attr);
          clientStartPointer = Math.min(clientStartPointer, attr.pointer);
          attr.sizeBytes = attr.size * GL.byteSizeByType[attr.type - GL.byteSizeByTypeRoot];
          bytes += attr.sizeBytes;
          minStride = Math.min(minStride, attr.stride);
          maxStride = Math.max(maxStride, attr.stride);
        }
      }

      if ((minStride != maxStride || maxStride < bytes) && !beginEnd) {
        // We are in cases (1) or (3): slow path, shuffle the data around into a single interleaved vertex buffer.
        // The immediate-mode glBegin()/glEnd() vertex submission gets automatically generated in appropriate layout,
        // so never need to come down this path if that was used.
#if GL_ASSERTIONS
        Runtime.warnOnce('Rendering from planar client-side vertex arrays. This is a very slow emulation path! Use interleaved vertex arrays for best performance.');
#endif
        if (!GLImmediate.restrideBuffer) GLImmediate.restrideBuffer = _malloc(GL.MAX_TEMP_BUFFER_SIZE);
        var start = GLImmediate.restrideBuffer;
        bytes = 0;
        // calculate restrided offsets and total size
        for (var i = 0; i < attributes.length; i++) {
          var attr = attributes[i];
          var size = attr.sizeBytes;
          if (size % 4 != 0) size += 4 - (size % 4); // align everything
          attr.offset = bytes;
          bytes += size;
        }
        // copy out the data (we need to know the stride for that, and define attr.pointer)
        for (var i = 0; i < attributes.length; i++) {
          var attr = attributes[i];
          var srcStride = Math.max(attr.sizeBytes, attr.stride);
          if ((srcStride & 3) == 0 && (attr.sizeBytes & 3) == 0) {
            var size4 = attr.sizeBytes>>2;
            var srcStride4 = Math.max(attr.sizeBytes, attr.stride)>>2;
            for (var j = 0; j < count; j++) {
              for (var k = 0; k < size4; k++) { // copy in chunks of 4 bytes, our alignment makes this possible
                HEAP32[((start + attr.offset + bytes*j)>>2) + k] = HEAP32[(attr.pointer>>2) + j*srcStride4 + k];
              }
            }
          } else {
            for (var j = 0; j < count; j++) {
              for (var k = 0; k < attr.sizeBytes; k++) { // source data was not aligned to multiples of 4, must copy byte by byte.
                HEAP8[start + attr.offset + bytes*j + k] = HEAP8[attr.pointer + j*srcStride + k];
              }
            }
          }
          attr.pointer = start + attr.offset;
        }
        GLImmediate.stride = bytes;
        GLImmediate.vertexPointer = start;
      } else {
        // case (2): fast path, all data is interleaved to a single vertex array so we can get away with a single VBO upload.
        if (GL.currArrayBuffer) {
          GLImmediate.vertexPointer = 0;
        } else {
          GLImmediate.vertexPointer = clientStartPointer;
        }
        for (var i = 0; i < attributes.length; i++) {
          var attr = attributes[i];
          attr.offset = attr.pointer - GLImmediate.vertexPointer; // Compute what will be the offset of this attribute in the VBO after we upload.
        }
        GLImmediate.stride = Math.max(maxStride, bytes);
      }
      if (!beginEnd) {
#if GL_ASSERTIONS
        if ((GLImmediate.stride & 3) != 0) {
          Runtime.warnOnce('Warning: Rendering from client side vertex arrays where stride (' + GLImmediate.stride + ') is not a multiple of four! This is not currently supported!');
        }
#endif
        GLImmediate.vertexCounter = (GLImmediate.stride * count) / 4; // XXX assuming float
      }
    },

    flush: function flush(numProvidedIndexes, startIndex, ptr) {
#if ASSERTIONS
      assert(numProvidedIndexes >= 0 || !numProvidedIndexes);
#endif
      startIndex = startIndex || 0;
      ptr = ptr || 0;

      var renderer = GLImmediate.getRenderer();

      // Generate index data in a format suitable for GLES 2.0/WebGL
      var numVertexes = 4 * GLImmediate.vertexCounter / GLImmediate.stride;
#if ASSERTIONS
      assert(numVertexes % 1 == 0, "`numVertexes` must be an integer.");
#endif
      var emulatedElementArrayBuffer = false;
      var numIndexes = 0;
      if (numProvidedIndexes) {
        numIndexes = numProvidedIndexes;
        if (!GL.currArrayBuffer && GLImmediate.firstVertex > GLImmediate.lastVertex) {
          // Figure out the first and last vertex from the index data
#if ASSERTIONS
          assert(!GL.currElementArrayBuffer); // If we are going to upload array buffer data, we need to find which range to
                                              // upload based on the indices. If they are in a buffer on the GPU, that is very
                                              // inconvenient! So if you do not have an array buffer, you should also not have
                                              // an element array buffer. But best is to use both buffers!
#endif
          for (var i = 0; i < numProvidedIndexes; i++) {
            var currIndex = {{{ makeGetValue('ptr', 'i*2', 'i16', null, 1) }}};
            GLImmediate.firstVertex = Math.min(GLImmediate.firstVertex, currIndex);
            GLImmediate.lastVertex = Math.max(GLImmediate.lastVertex, currIndex+1);
          }
        }
        if (!GL.currElementArrayBuffer) {
          // If no element array buffer is bound, then indices is a literal pointer to clientside data
#if ASSERTIONS
          assert(numProvidedIndexes << 1 <= GL.MAX_TEMP_BUFFER_SIZE, 'too many immediate mode indexes (a)');
#endif
          var indexBuffer = GL.getTempIndexBuffer(numProvidedIndexes << 1);
          GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, indexBuffer);
          GLctx.bufferSubData(GLctx.ELEMENT_ARRAY_BUFFER, 0, {{{ makeHEAPView('U16', 'ptr', 'ptr + (numProvidedIndexes << 1)') }}});
          ptr = 0;
          emulatedElementArrayBuffer = true;
        }
      } else if (GLImmediate.mode > 6) { // above GL_TRIANGLE_FAN are the non-GL ES modes
        if (GLImmediate.mode != 7) throw 'unsupported immediate mode ' + GLImmediate.mode; // GL_QUADS
        // GLImmediate.firstVertex is the first vertex we want. Quad indexes are in the pattern
        // 0 1 2, 0 2 3, 4 5 6, 4 6 7, so we need to look at index firstVertex * 1.5 to see it.
        // Then since indexes are 2 bytes each, that means 3
#if ASSERTIONS
        assert(GLImmediate.firstVertex % 4 == 0);
#endif
        ptr = GLImmediate.firstVertex*3;
        var numQuads = numVertexes / 4;
        numIndexes = numQuads * 6; // 0 1 2, 0 2 3 pattern
#if ASSERTIONS
        assert(ptr + (numIndexes << 1) <= GL.MAX_TEMP_BUFFER_SIZE, 'too many immediate mode indexes (b)');
#endif
        GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, GL.currentContext.tempQuadIndexBuffer);
        emulatedElementArrayBuffer = true;
      }

      renderer.prepare();

      if (numIndexes) {
        GLctx.drawElements(GLctx.TRIANGLES, numIndexes, GLctx.UNSIGNED_SHORT, ptr);
      } else {
        GLctx.drawArrays(GLImmediate.mode, startIndex, numVertexes);
      }

      if (emulatedElementArrayBuffer) {
        GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, GL.buffers[GL.currElementArrayBuffer] || null);
      }

#if !GL_UNSAFE_OPTS
#if !GL_FFP_ONLY
      renderer.cleanup();
#endif
#endif
    }
  },

  $GLImmediateSetup: {},
  $GLImmediateSetup__deps: ['$GLImmediate', function() { return 'GLImmediate.matrixLib = ' + read('gl-matrix.js') + ';\n' }],
  $GLImmediateSetup: {},

  glBegin__deps: ['$GLImmediateSetup'],
  glBegin: function(mode) {
    // Push the old state:
    GLImmediate.enabledClientAttributes_preBegin = GLImmediate.enabledClientAttributes;
    GLImmediate.enabledClientAttributes = [];

    GLImmediate.clientAttributes_preBegin = GLImmediate.clientAttributes;
    GLImmediate.clientAttributes = []
    for (var i = 0; i < GLImmediate.clientAttributes_preBegin.length; i++) {
      GLImmediate.clientAttributes.push({});
    }

    GLImmediate.mode = mode;
    GLImmediate.vertexCounter = 0;
    var components = GLImmediate.rendererComponents = [];
    for (var i = 0; i < GLImmediate.NUM_ATTRIBUTES; i++) {
      components[i] = 0;
    }
    GLImmediate.rendererComponentPointer = 0;
    GLImmediate.vertexData = GLImmediate.tempData;
  },

  glEnd: function() {
    GLImmediate.prepareClientAttributes(GLImmediate.rendererComponents[GLImmediate.VERTEX], true);
    GLImmediate.firstVertex = 0;
    GLImmediate.lastVertex = GLImmediate.vertexCounter / (GLImmediate.stride >> 2);
    GLImmediate.flush();
    GLImmediate.disableBeginEndClientAttributes();
    GLImmediate.mode = -1;

    // Pop the old state:
    GLImmediate.enabledClientAttributes = GLImmediate.enabledClientAttributes_preBegin;
    GLImmediate.clientAttributes = GLImmediate.clientAttributes_preBegin;
    GLImmediate.currentRenderer = null; // The set of active client attributes changed, we must re-lookup the renderer to use.
    GLImmediate.modifiedClientAttributes = true;
  },

  glVertex3f: function(x, y, z) {
#if ASSERTIONS
    assert(GLImmediate.mode >= 0); // must be in begin/end
#endif
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = x;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = y;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = z || 0;
#if ASSERTIONS
    assert(GLImmediate.vertexCounter << 2 < GL.MAX_TEMP_BUFFER_SIZE);
#endif
    GLImmediate.addRendererComponent(GLImmediate.VERTEX, 3, GLctx.FLOAT);
  },
  glVertex2f: 'glVertex3f',

  glVertex3fv__deps: ['glVertex3f'],
  glVertex3fv: function(p) {
    _glVertex3f({{{ makeGetValue('p', '0', 'float') }}}, {{{ makeGetValue('p', '4', 'float') }}}, {{{ makeGetValue('p', '8', 'float') }}});
  },
  glVertex2fv__deps: ['glVertex3f'],
  glVertex2fv: function(p) {
    _glVertex3f({{{ makeGetValue('p', '0', 'float') }}}, {{{ makeGetValue('p', '4', 'float') }}}, 0);
  },

  glVertex3i: 'glVertex3f',

  glVertex2i: 'glVertex3f',

  glTexCoord2i: function(u, v) {
#if ASSERTIONS
    assert(GLImmediate.mode >= 0); // must be in begin/end
#endif
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = u;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = v;
    GLImmediate.addRendererComponent(GLImmediate.TEXTURE0, 2, GLctx.FLOAT);
  },
  glTexCoord2f: 'glTexCoord2i',

  glTexCoord2fv__deps: ['glTexCoord2i'],
  glTexCoord2fv: function(v) {
    _glTexCoord2i({{{ makeGetValue('v', '0', 'float') }}}, {{{ makeGetValue('v', '4', 'float') }}});
  },

  glTexCoord4f: function() { throw 'glTexCoord4f: TODO' },

  glColor4f: function(r, g, b, a) {
    r = Math.max(Math.min(r, 1), 0);
    g = Math.max(Math.min(g, 1), 0);
    b = Math.max(Math.min(b, 1), 0);
    a = Math.max(Math.min(a, 1), 0);

    // TODO: make ub the default, not f, save a few mathops
    if (GLImmediate.mode >= 0) {
      var start = GLImmediate.vertexCounter << 2;
      GLImmediate.vertexDataU8[start + 0] = r * 255;
      GLImmediate.vertexDataU8[start + 1] = g * 255;
      GLImmediate.vertexDataU8[start + 2] = b * 255;
      GLImmediate.vertexDataU8[start + 3] = a * 255;
      GLImmediate.vertexCounter++;
      GLImmediate.addRendererComponent(GLImmediate.COLOR, 4, GLctx.UNSIGNED_BYTE);
    } else {
      GLImmediate.clientColor[0] = r;
      GLImmediate.clientColor[1] = g;
      GLImmediate.clientColor[2] = b;
      GLImmediate.clientColor[3] = a;
#if GL_FFP_ONLY
      GLctx.vertexAttrib4fv(GLImmediate.COLOR, GLImmediate.clientColor);
#endif
    }
  },
  glColor4d: 'glColor4f',
  glColor4ub__deps: ['glColor4f'],
  glColor4ub: function(r, g, b, a) {
    _glColor4f((r&255)/255, (g&255)/255, (b&255)/255, (a&255)/255);
  },
  glColor4us__deps: ['glColor4f'],
  glColor4us: function(r, g, b, a) {
    _glColor4f((r&65535)/65535, (g&65535)/65535, (b&65535)/65535, (a&65535)/65535);
  },
  glColor4ui__deps: ['glColor4f'],
  glColor4ui: function(r, g, b, a) {
    _glColor4f((r>>>0)/4294967295, (g>>>0)/4294967295, (b>>>0)/4294967295, (a>>>0)/4294967295);
  },
  glColor3f__deps: ['glColor4f'],
  glColor3f: function(r, g, b) {
    _glColor4f(r, g, b, 1);
  },
  glColor3d: 'glColor3f',
  glColor3ub__deps: ['glColor4ub'],
  glColor3ub: function(r, g, b) {
    _glColor4ub(r, g, b, 255);
  },
  glColor3us__deps: ['glColor4us'],
  glColor3us: function(r, g, b) {
    _glColor4us(r, g, b, 65535);
  },
  glColor3ui__deps: ['glColor4ui'],
  glColor3ui: function(r, g, b) {
    _glColor4ui(r, g, b, 4294967295);
  },

  glColor3ubv__deps: ['glColor3ub'],
  glColor3ubv: function(p) {
    _glColor3ub({{{ makeGetValue('p', '0', 'i8') }}}, {{{ makeGetValue('p', '1', 'i8') }}}, {{{ makeGetValue('p', '2', 'i8') }}});
  },
  glColor3usv__deps: ['glColor3us'],
  glColor3usv: function(p) {
    _glColor3us({{{ makeGetValue('p', '0', 'i16') }}}, {{{ makeGetValue('p', '2', 'i16') }}}, {{{ makeGetValue('p', '4', 'i16') }}});
  },
  glColor3uiv__deps: ['glColor3ui'],
  glColor3uiv: function(p) {
    _glColor3ui({{{ makeGetValue('p', '0', 'i32') }}}, {{{ makeGetValue('p', '4', 'i32') }}}, {{{ makeGetValue('p', '8', 'i32') }}});
  },
  glColor3fv__deps: ['glColor3f'],
  glColor3fv: function(p) {
    _glColor3f({{{ makeGetValue('p', '0', 'float') }}}, {{{ makeGetValue('p', '4', 'float') }}}, {{{ makeGetValue('p', '8', 'float') }}});
  },
  glColor4fv__deps: ['glColor4f'],
  glColor4fv: function(p) {
    _glColor4f({{{ makeGetValue('p', '0', 'float') }}}, {{{ makeGetValue('p', '4', 'float') }}}, {{{ makeGetValue('p', '8', 'float') }}}, {{{ makeGetValue('p', '12', 'float') }}});
  },

  glColor4ubv__deps: ['glColor4ub'],
  glColor4ubv: function(p) {
    _glColor4ub({{{ makeGetValue('p', '0', 'i8') }}}, {{{ makeGetValue('p', '1', 'i8') }}}, {{{ makeGetValue('p', '2', 'i8') }}}, {{{ makeGetValue('p', '3', 'i8') }}});
  },

  glFogf: function(pname, param) { // partial support, TODO
    switch(pname) {
      case 0x0B63: // GL_FOG_START
        GLEmulation.fogStart = param; break;
      case 0x0B64: // GL_FOG_END
        GLEmulation.fogEnd = param; break;
      case 0x0B62: // GL_FOG_DENSITY
        GLEmulation.fogDensity = param; break;
      case 0x0B65: // GL_FOG_MODE
        switch (param) {
          case 0x0801: // GL_EXP2
          case 0x2601: // GL_LINEAR
            if (GLEmulation.fogMode != param) {
              GLImmediate.currentRenderer = null; // Fog mode is part of the FFP shader state, we must re-lookup the renderer to use.
              GLEmulation.fogMode = param;
            }
            break;
          default: // default to GL_EXP
            if (GLEmulation.fogMode != 0x0800 /* GL_EXP */) {
              GLImmediate.currentRenderer = null; // Fog mode is part of the FFP shader state, we must re-lookup the renderer to use.
              GLEmulation.fogMode = 0x0800 /* GL_EXP */;
            }
            break;
        }
        break;
    }
  },
  glFogi__deps: ['glFogf'],
  glFogi: function(pname, param) {
    return _glFogf(pname, param);
  },
  glFogfv__deps: ['glFogf'],
  glFogfv: function(pname, param) { // partial support, TODO
    switch(pname) {
      case 0x0B66: // GL_FOG_COLOR
        GLEmulation.fogColor[0] = {{{ makeGetValue('param', '0', 'float') }}};
        GLEmulation.fogColor[1] = {{{ makeGetValue('param', '4', 'float') }}};
        GLEmulation.fogColor[2] = {{{ makeGetValue('param', '8', 'float') }}};
        GLEmulation.fogColor[3] = {{{ makeGetValue('param', '12', 'float') }}};
        break;
      case 0x0B63: // GL_FOG_START
      case 0x0B64: // GL_FOG_END
        _glFogf(pname, {{{ makeGetValue('param', '0', 'float') }}}); break;
    }
  },
  glFogiv__deps: ['glFogf'],
  glFogiv: function(pname, param) {
    switch(pname) {
      case 0x0B66: // GL_FOG_COLOR
        GLEmulation.fogColor[0] = ({{{ makeGetValue('param', '0', 'i32') }}}/2147483647)/2.0+0.5;
        GLEmulation.fogColor[1] = ({{{ makeGetValue('param', '4', 'i32') }}}/2147483647)/2.0+0.5;
        GLEmulation.fogColor[2] = ({{{ makeGetValue('param', '8', 'i32') }}}/2147483647)/2.0+0.5;
        GLEmulation.fogColor[3] = ({{{ makeGetValue('param', '12', 'i32') }}}/2147483647)/2.0+0.5;
        break;
      default:
        _glFogf(pname, {{{ makeGetValue('param', '0', 'i32') }}}); break;
    }
  },
  glFogx: 'glFogi',
  glFogxv: 'glFogiv',

  glPolygonMode: function(){}, // TODO

  glAlphaFunc: function(){}, // TODO

  glNormal3f: function(){}, // TODO

  // Additional non-GLES rendering calls

  glDrawRangeElements__deps: ['glDrawElements'],
  glDrawRangeElements__sig: 'viiiiii',
  glDrawRangeElements: function(mode, start, end, count, type, indices) {
    _glDrawElements(mode, count, type, indices, start, end);
  },

  // ClientState/gl*Pointer

  glEnableClientState: function(cap) {
    var attrib = GLEmulation.getAttributeFromCapability(cap);
    if (attrib === null) {
#if ASSERTIONS
      Module.printErr('WARNING: unhandled clientstate: ' + cap);
#endif
      return;
    }
    if (!GLImmediate.enabledClientAttributes[attrib]) {
      GLImmediate.enabledClientAttributes[attrib] = true;
      GLImmediate.totalEnabledClientAttributes++;
      GLImmediate.currentRenderer = null; // Will need to change current renderer, since the set of active vertex pointers changed.
#if GL_FFP_ONLY
      // In GL_FFP_ONLY mode, attributes are bound to the same index in each FFP emulation shader, so we can immediately apply the change here.
      GL.enableVertexAttribArray(attrib);
#endif
      if (GLEmulation.currentVao) GLEmulation.currentVao.enabledClientStates[cap] = 1;
      GLImmediate.modifiedClientAttributes = true;
    }
  },
  glDisableClientState: function(cap) {
    var attrib = GLEmulation.getAttributeFromCapability(cap);
    if (attrib === null) {
#if ASSERTIONS
      Module.printErr('WARNING: unhandled clientstate: ' + cap);
#endif
      return;
    }
    if (GLImmediate.enabledClientAttributes[attrib]) {
      GLImmediate.enabledClientAttributes[attrib] = false;
      GLImmediate.totalEnabledClientAttributes--;
      GLImmediate.currentRenderer = null; // Will need to change current renderer, since the set of active vertex pointers changed.
#if GL_FFP_ONLY
      // In GL_FFP_ONLY mode, attributes are bound to the same index in each FFP emulation shader, so we can immediately apply the change here.
      GL.disableVertexAttribArray(attrib);
#endif
      if (GLEmulation.currentVao) delete GLEmulation.currentVao.enabledClientStates[cap];
      GLImmediate.modifiedClientAttributes = true;
    }
  },

  glVertexPointer__deps: ['$GLEmulation'], // if any pointers are used, glVertexPointer must be, and if it is, then we need emulation
  glVertexPointer: function(size, type, stride, pointer) {
    GLImmediate.setClientAttribute(GLImmediate.VERTEX, size, type, stride, pointer);
#if GL_FFP_ONLY
    if (GL.currArrayBuffer) {
      GLctx.vertexAttribPointer(GLImmediate.VERTEX, size, type, false, stride, pointer);
    }
#endif
  },
  glTexCoordPointer: function(size, type, stride, pointer) {
    GLImmediate.setClientAttribute(GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture, size, type, stride, pointer);
#if GL_FFP_ONLY
    if (GL.currArrayBuffer) {
      var loc = GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture;
      GLctx.vertexAttribPointer(loc, size, type, false, stride, pointer);
    }
#endif
  },
  glNormalPointer: function(type, stride, pointer) {
    GLImmediate.setClientAttribute(GLImmediate.NORMAL, 3, type, stride, pointer);
#if GL_FFP_ONLY
    if (GL.currArrayBuffer) {
      GLctx.vertexAttribPointer(GLImmediate.NORMAL, size, type, true, stride, pointer);
    }
#endif
  },
  glColorPointer: function(size, type, stride, pointer) {
    GLImmediate.setClientAttribute(GLImmediate.COLOR, size, type, stride, pointer);
#if GL_FFP_ONLY
    if (GL.currArrayBuffer) {
      GLctx.vertexAttribPointer(GLImmediate.COLOR, size, type, true, stride, pointer);
    }
#endif
  },

  glClientActiveTexture__sig: 'vi',
  glClientActiveTexture: function(texture) {
    GLImmediate.clientActiveTexture = texture - 0x84C0; // GL_TEXTURE0
  },

  // Vertex array object (VAO) support. TODO: when the WebGL extension is popular, use that and remove this code and GL.vaos
  emulGlGenVertexArrays__deps: ['$GLEmulation'],
  emulGlGenVertexArrays__sig: 'vii',
  emulGlGenVertexArrays: function(n, vaos) {
    for (var i = 0; i < n; i++) {
      var id = GL.getNewId(GLEmulation.vaos);
      GLEmulation.vaos[id] = {
        id: id,
        arrayBuffer: 0,
        elementArrayBuffer: 0,
        enabledVertexAttribArrays: {},
        vertexAttribPointers: {},
        enabledClientStates: {},
      };
      {{{ makeSetValue('vaos', 'i*4', 'id', 'i32') }}};
    }
  },
  emulGlDeleteVertexArrays__sig: 'vii',
  emulGlDeleteVertexArrays: function(n, vaos) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('vaos', 'i*4', 'i32') }}};
      GLEmulation.vaos[id] = null;
      if (GLEmulation.currentVao && GLEmulation.currentVao.id == id) GLEmulation.currentVao = null;
    }
  },
  emulGlIsVertexArray__sig: 'vi',
  emulGlIsVertexArray: function(array) {
    var vao = GLEmulation.vaos[array];
    if (!vao) return 0;
    return 1;
  },
  emulGlBindVertexArray__deps: ['glBindBuffer', 'glEnableVertexAttribArray', 'glVertexAttribPointer', 'glEnableClientState'],
  emulGlBindVertexArray__sig: 'vi',
  emulGlBindVertexArray: function(vao) {
    // undo vao-related things, wipe the slate clean, both for vao of 0 or an actual vao
    GLEmulation.currentVao = null; // make sure the commands we run here are not recorded
    if (GLImmediate.lastRenderer) GLImmediate.lastRenderer.cleanup();
    _glBindBuffer(GLctx.ARRAY_BUFFER, 0); // XXX if one was there before we were bound?
    _glBindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, 0);
    for (var vaa in GLEmulation.enabledVertexAttribArrays) {
      GLctx.disableVertexAttribArray(vaa);
    }
    GLEmulation.enabledVertexAttribArrays = {};
    GLImmediate.enabledClientAttributes = [0, 0];
    GLImmediate.totalEnabledClientAttributes = 0;
    GLImmediate.modifiedClientAttributes = true;
    if (vao) {
      // replay vao
      var info = GLEmulation.vaos[vao];
      _glBindBuffer(GLctx.ARRAY_BUFFER, info.arrayBuffer); // XXX overwrite current binding?
      _glBindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, info.elementArrayBuffer);
      for (var vaa in info.enabledVertexAttribArrays) {
        _glEnableVertexAttribArray(vaa);
      }
      for (var vaa in info.vertexAttribPointers) {
        _glVertexAttribPointer.apply(null, info.vertexAttribPointers[vaa]);
      }
      for (var attrib in info.enabledClientStates) {
        _glEnableClientState(attrib|0);
      }
      GLEmulation.currentVao = info; // set currentVao last, so the commands we ran here were not recorded
    }
  },

  // OpenGL Immediate Mode matrix routines.
  // Note that in the future we might make these available only in certain modes.
  glMatrixMode__deps: ['$GL', '$GLImmediateSetup', '$GLEmulation'], // emulation is not strictly needed, this is a workaround
  glMatrixMode: function(mode) {
    if (mode == 0x1700 /* GL_MODELVIEW */) {
      GLImmediate.currentMatrix = 0/*m*/;
    } else if (mode == 0x1701 /* GL_PROJECTION */) {
      GLImmediate.currentMatrix = 1/*p*/;
    } else if (mode == 0x1702) { // GL_TEXTURE
      GLImmediate.useTextureMatrix = true;
      GLImmediate.currentMatrix = 2/*t*/ + GLImmediate.clientActiveTexture;
    } else {
      throw "Wrong mode " + mode + " passed to glMatrixMode";
    }
  },

  glPushMatrix: function() {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixStack[GLImmediate.currentMatrix].push(
        Array.prototype.slice.call(GLImmediate.matrix[GLImmediate.currentMatrix]));
  },

  glPopMatrix: function() {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrix[GLImmediate.currentMatrix] = GLImmediate.matrixStack[GLImmediate.currentMatrix].pop();
  },

  glLoadIdentity__deps: ['$GL', '$GLImmediateSetup'],
  glLoadIdentity: function() {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.identity(GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  glLoadMatrixd: function(matrix) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}}, GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  glLoadMatrixf: function(matrix) {
#if GL_DEBUG
    if (GL.debug) Module.printErr('glLoadMatrixf receiving: ' + Array.prototype.slice.call(HEAPF32.subarray(matrix >> 2, (matrix >> 2) + 16)));
#endif
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}}, GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  glLoadTransposeMatrixd: function(matrix) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}}, GLImmediate.matrix[GLImmediate.currentMatrix]);
    GLImmediate.matrixLib.mat4.transpose(GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  glLoadTransposeMatrixf: function(matrix) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}}, GLImmediate.matrix[GLImmediate.currentMatrix]);
    GLImmediate.matrixLib.mat4.transpose(GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  glMultMatrixd: function(matrix) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix],
        {{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}});
  },

  glMultMatrixf: function(matrix) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix],
        {{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}});
  },

  glMultTransposeMatrixd: function(matrix) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    var colMajor = GLImmediate.matrixLib.mat4.create();
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}}, colMajor);
    GLImmediate.matrixLib.mat4.transpose(colMajor);
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix], colMajor);
  },

  glMultTransposeMatrixf: function(matrix) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    var colMajor = GLImmediate.matrixLib.mat4.create();
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}}, colMajor);
    GLImmediate.matrixLib.mat4.transpose(colMajor);
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix], colMajor);
  },

  glFrustum: function(left, right, bottom, top_, nearVal, farVal) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix],
        GLImmediate.matrixLib.mat4.frustum(left, right, bottom, top_, nearVal, farVal));
  },
  glFrustumf: 'glFrustum',

  glOrtho: function(left, right, bottom, top_, nearVal, farVal) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix],
        GLImmediate.matrixLib.mat4.ortho(left, right, bottom, top_, nearVal, farVal));
  },
  glOrthof: 'glOrtho',

  glScaled: function(x, y, z) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.scale(GLImmediate.matrix[GLImmediate.currentMatrix], [x, y, z]);
  },
  glScalef: 'glScaled',

  glTranslated: function(x, y, z) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.translate(GLImmediate.matrix[GLImmediate.currentMatrix], [x, y, z]);
  },
  glTranslatef: 'glTranslated',

  glRotated: function(angle, x, y, z) {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.rotate(GLImmediate.matrix[GLImmediate.currentMatrix], angle*Math.PI/180, [x, y, z]);
  },
  glRotatef: 'glRotated',

  glDrawBuffer: function() { throw 'glDrawBuffer: TODO' },
#if !USE_WEBGL2
  glReadBuffer: function() { throw 'glReadBuffer: TODO' },
#endif

  glLightfv: function() { throw 'glLightfv: TODO' },
  glLightModelfv: function() { throw 'glLightModelfv: TODO' },
  glMaterialfv: function() { throw 'glMaterialfv: TODO' },

  glTexGeni: function() { throw 'glTexGeni: TODO' },
  glTexGenfv: function() { throw 'glTexGenfv: TODO' },
  glTexEnvi: function() { Runtime.warnOnce('glTexEnvi: TODO') },
  glTexEnvf: function() { Runtime.warnOnce('glTexEnvf: TODO') },
  glTexEnvfv: function() { Runtime.warnOnce('glTexEnvfv: TODO') },

  glGetTexEnviv: function(target, pname, param) { throw 'GL emulation not initialized!'; },
  glGetTexEnvfv: function(target, pname, param) { throw 'GL emulation not initialized!'; },

  glTexImage1D: function() { throw 'glTexImage1D: TODO' },
  glTexCoord3f: function() { throw 'glTexCoord3f: TODO' },
  glGetTexLevelParameteriv: function() { throw 'glGetTexLevelParameteriv: TODO' },

  glShadeModel: function() { Runtime.warnOnce('TODO: glShadeModel') },

  // Open GLES1.1 compatibility

  glGenFramebuffersOES : 'glGenFramebuffers',
  glGenRenderbuffersOES : 'glGenRenderbuffers',
  glBindFramebufferOES : 'glBindFramebuffer',
  glBindRenderbufferOES : 'glBindRenderbuffer',
  glGetRenderbufferParameterivOES : 'glGetRenderbufferParameteriv',
  glFramebufferRenderbufferOES : 'glFramebufferRenderbuffer',
  glRenderbufferStorageOES : 'glRenderbufferStorage',
  glCheckFramebufferStatusOES : 'glCheckFramebufferStatus',
  glDeleteFramebuffersOES : 'glDeleteFramebuffers',
  glDeleteRenderbuffersOES : 'glDeleteRenderbuffers',
  glFramebufferTexture2DOES: 'glFramebufferTexture2D',

#else // LEGACY_GL_EMULATION

  glVertexPointer: function(){ throw 'Legacy GL function (glVertexPointer) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },
  glMatrixMode: function(){ throw 'Legacy GL function (glMatrixMode) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },
  glBegin: function(){ throw 'Legacy GL function (glBegin) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },
  glLoadIdentity: function(){ throw 'Legacy GL function (glLoadIdentity) called. If you want legacy GL emulation, you need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },

#endif // LEGACY_GL_EMULATION

  // Open GLES1.1 vao compatibility (Could work w/o -s LEGACY_GL_EMULATION=1)

  glGenVertexArraysOES: 'glGenVertexArrays',
  glDeleteVertexArraysOES: 'glDeleteVertexArrays',
  glBindVertexArrayOES: 'glBindVertexArray',

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
#if ASSERTIONS
    assert(cb, index);
#endif
    if (!GL.currArrayBuffer) {
      cb.size = size;
      cb.type = type;
      cb.normalized = normalized;
      cb.stride = stride;
      cb.ptr = ptr;
      cb.clientside = true;
      return;
    }
    cb.clientside = false;
#endif
#if GL_ASSERTIONS
    GL.validateVertexAttribPointer(size, type, stride, ptr);
#endif
    GLctx.vertexAttribPointer(index, size, type, normalized, stride, ptr);
  },

#if USE_WEBGL2
  glVertexAttribIPointer__sig: 'viiiii',
  glVertexAttribIPointer: function(index, size, type, stride, ptr) {
#if FULL_ES3
    var cb = GL.currentContext.clientBuffers[index];
#if ASSERTIONS
    assert(cb, index);
#endif
    if (!GL.currArrayBuffer) {
      cb.size = size;
      cb.type = type;
      cb.normalized = false;
      cb.stride = stride;
      cb.ptr = ptr;
      cb.clientside = true;
      return;
    }
    cb.clientside = false;
#endif
#if GL_ASSERTIONS
    GL.validateVertexAttribPointer(size, type, stride, ptr);
#endif
    GLctx.vertexAttribIPointer(index, size, type, stride, ptr);
  },
// ~USE_WEBGL2
#endif

  glEnableVertexAttribArray__sig: 'vi',
  glEnableVertexAttribArray: function(index) {
#if FULL_ES2
    var cb = GL.currentContext.clientBuffers[index];
#if ASSERTIONS
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
#if ASSERTIONS
    assert(cb, index);
#endif
    cb.enabled = false;
#endif
    GLctx.disableVertexAttribArray(index);
  },

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

  glShaderBinary__sig: 'v',
  glShaderBinary: function() {
    GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
    Module.printErr("GL_INVALID_ENUM in glShaderBinary: WebGL does not support binary shader formats! Calls to glShaderBinary always fail.");
#endif    
  },

  glReleaseShaderCompiler__sig: 'v',
  glReleaseShaderCompiler: function() {
    // NOP (as allowed by GLES 2.0 spec)
  },

  glGetError__sig: 'i',
  glGetError: function() {
    // First return any GL error generated by the emscripten library_gl.js interop layer.
    if (GL.lastError) {
      var error = GL.lastError;
      GL.lastError = 0/*GL_NO_ERROR*/;
      return error;
    } else { // If there were none, return the GL error from the browser GL context.
      return GLctx.getError();
    }
  },
  
  // ANGLE_instanced_arrays WebGL extension related functions
  
  glVertexAttribDivisor__sig: 'vii',
  glVertexAttribDivisor: function(index, divisor) {
#if GL_ASSERTIONS    
    assert(GL.currentContext.instancedArraysExt, 'Must have ANGLE_instanced_arrays extension to use WebGL instancing');
#endif
    GL.currentContext.instancedArraysExt.vertexAttribDivisorANGLE(index, divisor);    
  },

  glDrawArraysInstanced__sig: 'viiii',
  glDrawArraysInstanced: function(mode, first, count, primcount) {
#if GL_ASSERTIONS    
    assert(GL.currentContext.instancedArraysExt, 'Must have ANGLE_instanced_arrays extension to use WebGL instancing');
#endif
    GL.currentContext.instancedArraysExt.drawArraysInstancedANGLE(mode, first, count, primcount);
  },
  
  glDrawElementsInstanced__sig: 'viiiii',
  glDrawElementsInstanced: function(mode, count, type, indices, primcount) {
#if GL_ASSERTIONS    
    assert(GL.currentContext.instancedArraysExt, 'Must have ANGLE_instanced_arrays extension to use WebGL instancing');
#endif
    GL.currentContext.instancedArraysExt.drawElementsInstancedANGLE(mode, count, type, indices, primcount);
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


  glDrawBuffers__sig: 'vii',
  glDrawBuffers: function(n, bufs) {
#if GL_ASSERTIONS
    assert(GL.currentContext.drawBuffersExt, 'Must have WebGL2 or WEBGL_draw_buffers extension to use drawBuffers');
#endif
    var bufArray = [];
    for (var i = 0; i < n; i++)
      bufArray.push({{{ makeGetValue('bufs', 'i*4', 'i32') }}});

    GL.currentContext.drawBuffersExt(bufArray);
  },
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
  glPolygonOffset__sig: 'vii',
  glColorMask__sig: 'viiii',
  glStencilOp__sig: 'viii',
  glStencilOpSeparate__sig: 'viiii',
  glGenerateMipmap__sig: 'vi',
  glHint__sig: 'vii',
  glDepthMask__sig: 'vi',
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
  glSampleCoverage__sig: 'vii',
};


// Simple pass-through functions. Starred ones have return values. [X] ones have X in the C name but not in the JS name
[[0, 'finish flush'],
 [1, 'clearDepth clearDepth[f] depthFunc enable disable frontFace cullFace clear lineWidth clearStencil depthMask stencilMask checkFramebufferStatus* generateMipmap activeTexture blendEquation isEnabled*'],
 [2, 'blendFunc blendEquationSeparate depthRange depthRange[f] stencilMaskSeparate hint polygonOffset vertexAttrib1f sampleCoverage'],
 [3, 'texParameteri texParameterf vertexAttrib2f stencilFunc stencilOp'],
 [4, 'viewport clearColor scissor vertexAttrib3f colorMask renderbufferStorage blendFuncSeparate blendColor stencilFuncSeparate stencilOpSeparate'],
 [5, 'vertexAttrib4f'],
 [8, 'copyTexImage2D copyTexSubImage2D']].forEach(function(data) {
  var num = data[0];
  var names = data[1];
  var args = range(num).map(function(i) { return 'x' + i }).join(', ');
  var plainStub = '(function(' + args + ') { GLctx.NAME(' + args + ') })';
  var returnStub = '(function(' + args + ') { return GLctx.NAME(' + args + ') })';
  var sigEnd = range(num).map(function() { return 'i' }).join('');
  names.split(' ').forEach(function(name) {
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
    assert(!(cName in LibraryGL), "Cannot reimplement the existing function " + cName);
    LibraryGL[cName] = eval(stub.replace('NAME', name));
    if (!LibraryGL[cName + '__sig']) LibraryGL[cName + '__sig'] = sig;
  });
});

autoAddDeps(LibraryGL, '$GL');

// Legacy GL emulation 
if (LEGACY_GL_EMULATION) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$GLEmulation');
}

function copyLibEntry(a, b) {
  LibraryGL[a] = LibraryGL[b];
  LibraryGL[a + '__postset'] = LibraryGL[b + '__postset'];
  LibraryGL[a + '__sig'] = LibraryGL[b + '__sig'];
  LibraryGL[a + '__asm'] = LibraryGL[b + '__asm'];
  LibraryGL[a + '__deps'] = LibraryGL[b + '__deps'].slice(0);
}

// GL proc address retrieval - allow access through glX and emscripten_glX, to allow name collisions with user-implemented things having the same name (see gl.c)
keys(LibraryGL).forEach(function(x) {
  if (x.substr(-6) == '__deps' || x.substr(-9) == '__postset' || x.substr(-5) == '__sig' || x.substr(-5) == '__asm' || x.substr(0, 2) != 'gl') return;
  while (typeof LibraryGL[x] === 'string') {
    // resolve aliases right here, simpler for fastcomp
    copyLibEntry(x, LibraryGL[x]);
  }
  var y = 'emscripten_' + x;
  LibraryGL[x + '__deps'] = LibraryGL[x + '__deps'].map(function(dep) {
    // prefix dependencies as well
    if (typeof dep === 'string' && dep[0] == 'g' && dep[1] == 'l' && LibraryGL[dep]) {
      var orig = dep;
      dep = 'emscripten_' + dep;
      var fixed = LibraryGL[x].toString().replace(new RegExp('_' + orig + '\\(', 'g'), '_' + dep + '(');
      fixed = fixed.substr(0, 9) + '_' + y + fixed.substr(9);
      LibraryGL[x] = eval('(function() { return ' + fixed + ' })()');
    }
    return dep;
  });
  // copy it
  copyLibEntry(y, x);
});

// Final merge
mergeInto(LibraryManager.library, LibraryGL);

assert(!(FULL_ES2 && LEGACY_GL_EMULATION), 'cannot emulate both ES2 and legacy GL');

