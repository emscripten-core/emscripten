/*
 * GL support. See https://github.com/kripken/emscripten/wiki/OpenGL-support
 * for current status.
 */

var LibraryGL = {
  $GL__postset: 'GL.init()',
  $GL: {
#if GL_DEBUG
    debug: true,
#endif

    counter: 1, // 0 is reserved as 'null' in gl
    buffers: [],
    programs: [],
    framebuffers: [],
    renderbuffers: [],
    textures: [],
    uniforms: [],
    shaders: [],

#if FULL_ES2
    clientBuffers: [],
#endif
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

    uniformTable: {}, // name => uniform ID. the uID must be identical until relinking, cannot create a new uID each call to glGetUniformLocation

    packAlignment: 4,   // default alignment is 4 bytes
    unpackAlignment: 4, // default alignment is 4 bytes

    init: function() {
      Browser.moduleContextCreatedCallbacks.push(GL.initExtensions);
    },

    // Get a new ID for a texture/buffer/etc., while keeping the table dense and fast. Creation is farely rare so it is worth optimizing lookups later.
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

    // Large temporary buffers
    MAX_TEMP_BUFFER_SIZE: {{{ GL_MAX_TEMP_BUFFER_SIZE }}},
    tempBufferIndexLookup: null,
    tempVertexBuffers: null,
    tempIndexBuffers: null,
    tempQuadIndexBuffer: null,

    generateTempBuffers: function(quads) {
      GL.tempBufferIndexLookup = new Uint8Array(GL.MAX_TEMP_BUFFER_SIZE+1);
      GL.tempVertexBuffers = [];
      GL.tempIndexBuffers = [];
      var last = -1, curr = -1;
      var size = 1;
      for (var i = 0; i <= GL.MAX_TEMP_BUFFER_SIZE; i++) {
        if (i > size) {
          size <<= 1;
        }
        if (size != last) {
          curr++;
          GL.tempVertexBuffers[curr] = Module.ctx.createBuffer();
          Module.ctx.bindBuffer(Module.ctx.ARRAY_BUFFER, GL.tempVertexBuffers[curr]);
          Module.ctx.bufferData(Module.ctx.ARRAY_BUFFER, size, Module.ctx.DYNAMIC_DRAW);
          Module.ctx.bindBuffer(Module.ctx.ARRAY_BUFFER, null);
          GL.tempIndexBuffers[curr] = Module.ctx.createBuffer();
          Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, GL.tempIndexBuffers[curr]);
          Module.ctx.bufferData(Module.ctx.ELEMENT_ARRAY_BUFFER, size, Module.ctx.DYNAMIC_DRAW);
          Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, null);
          last = size;
        }
        GL.tempBufferIndexLookup[i] = curr;
      }

      if (quads) {
        // GL_QUAD indexes can be precalculated
        GL.tempQuadIndexBuffer = Module.ctx.createBuffer();
        Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, GL.tempQuadIndexBuffer);
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
        Module.ctx.bufferData(Module.ctx.ELEMENT_ARRAY_BUFFER, quadIndexes, Module.ctx.STATIC_DRAW);
        Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, null);
      }
    },

    // Linear lookup in one of the tables (buffers, programs, etc.). TODO: consider using a weakmap to make this faster, if it matters
    scan: function(table, object) {
      for (var item in table) {
        if (table[item] == object) return item;
      }
      return 0;
    },

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
      // Let's see if we need to enable the standard derivatives extension
      type = Module.ctx.getShaderParameter(GL.shaders[shader], 0x8B4F /* GL_SHADER_TYPE */);
      if (type == 0x8B30 /* GL_FRAGMENT_SHADER */) {
        if (GL.findToken(source, "dFdx") ||
            GL.findToken(source, "dFdy") ||
            GL.findToken(source, "fwidth")) {
          source = "#extension GL_OES_standard_derivatives : enable\n" + source;
          var extension = Module.ctx.getExtension("OES_standard_derivatives");
#if GL_DEBUG
          if (!extension) {
            Module.printErr("Shader attempts to use the standard derivatives extension which is not available.");
          }
#endif
        }
      }
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

    getTexPixelData: function(type, format, width, height, pixels, internalFormat) {
      var sizePerPixel;
      switch (type) {
        case 0x1401 /* GL_UNSIGNED_BYTE */:
          switch (format) {
            case 0x1906 /* GL_ALPHA */:
            case 0x1909 /* GL_LUMINANCE */:
              sizePerPixel = 1;
              break;
            case 0x1907 /* GL_RGB */:
              sizePerPixel = 3;
              break;
            case 0x1908 /* GL_RGBA */:
              sizePerPixel = 4;
              break;
            case 0x190A /* GL_LUMINANCE_ALPHA */:
              sizePerPixel = 2;
              break;
            default:
              throw 'Invalid format (' + format + ')';
          }
          break;
        case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
        case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
        case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
          sizePerPixel = 2;
          break;
        case 0x1406 /* GL_FLOAT */:
          assert(GL.floatExt, 'Must have OES_texture_float to use float textures');
          switch (format) {
            case 0x1907 /* GL_RGB */:
              sizePerPixel = 3*4;
              break;
            case 0x1908 /* GL_RGBA */:
              sizePerPixel = 4*4;
              break;
            default:
              throw 'Invalid format (' + format + ')';
          }
          internalFormat = Module.ctx.RGBA;
          break;
        default:
          throw 'Invalid type (' + type + ')';
      }
      var bytes = GL.computeImageSize(width, height, sizePerPixel, GL.unpackAlignment);
      if (type == 0x1401 /* GL_UNSIGNED_BYTE */) {
        pixels = {{{ makeHEAPView('U8', 'pixels', 'pixels+bytes') }}};
      } else if (type == 0x1406 /* GL_FLOAT */) {
        pixels = {{{ makeHEAPView('F32', 'pixels', 'pixels+bytes') }}};
      } else {
        pixels = {{{ makeHEAPView('U16', 'pixels', 'pixels+bytes') }}};
      }
      return {
        pixels: pixels,
        internalFormat: internalFormat
      }
    },

#if FULL_ES2
    calcBufLength: function(size, type, stride, count) {
      if (stride > 0) {
        return count * stride;  // XXXvlad this is not exactly correct I don't think
      }
      var typeSize = GL.byteSizeByType[type - GL.byteSizeByTypeRoot];
      return size * typeSize * count;
    },

    usedTempBuffers: [],

    preDrawHandleClientVertexAttribBindings: function(count) {
      GL.resetBufferBinding = false;

      var used = GL.usedTempBuffers;
      used.length = 0;

      // TODO: initial pass to detect ranges we need to upload, might not need an upload per attrib
      for (var i = 0; i < GL.maxVertexAttribs; ++i) {
        var cb = GL.clientBuffers[i];
        if (!cb.clientside || !cb.enabled) continue;

        GL.resetBufferBinding = true;

        var size = GL.calcBufLength(cb.size, cb.type, cb.stride, count);
        var index = GL.tempBufferIndexLookup[size];
        var buf;
        do {
#if ASSERTIONS
          assert(index < GL.tempVertexBuffers.length);
#endif
          buf = GL.tempVertexBuffers[index++];
        } while (used.indexOf(buf) >= 0);
        used.push(buf);
        Module.ctx.bindBuffer(Module.ctx.ARRAY_BUFFER, buf);
        Module.ctx.bufferSubData(Module.ctx.ARRAY_BUFFER,
                                 0,
                                 HEAPU8.subarray(cb.ptr, cb.ptr + size));
        Module.ctx.vertexAttribPointer(i, cb.size, cb.type, cb.normalized, cb.stride, 0);
      }
    },

    postDrawHandleClientVertexAttribBindings: function() {
      if (GL.resetBufferBinding) {
        Module.ctx.bindBuffer(Module.ctx.ARRAY_BUFFER, GL.buffers[GL.currArrayBuffer]);
      }
    },
#endif

    initExtensions: function() {
      if (GL.initExtensions.done) return;
      GL.initExtensions.done = true;

      if (!Module.useWebGL) return; // an app might link both gl and 2d backends

      GL.miniTempBuffer = new Float32Array(GL.MINI_TEMP_BUFFER_SIZE);
      for (var i = 0; i < GL.MINI_TEMP_BUFFER_SIZE; i++) {
        GL.miniTempBufferViews[i] = GL.miniTempBuffer.subarray(0, i+1);
      }

      GL.maxVertexAttribs = Module.ctx.getParameter(Module.ctx.MAX_VERTEX_ATTRIBS);
#if FULL_ES2
      for (var i = 0; i < GL.maxVertexAttribs; i++) {
        GL.clientBuffers[i] = { enabled: false, clientside: false, size: 0, type: 0, normalized: 0, stride: 0, ptr: 0 };
      }

      GL.generateTempBuffers();
#endif

      GL.compressionExt = Module.ctx.getExtension('WEBGL_compressed_texture_s3tc') ||
                          Module.ctx.getExtension('MOZ_WEBGL_compressed_texture_s3tc') ||
                          Module.ctx.getExtension('WEBKIT_WEBGL_compressed_texture_s3tc');

      GL.anisotropicExt = Module.ctx.getExtension('EXT_texture_filter_anisotropic') ||
                          Module.ctx.getExtension('MOZ_EXT_texture_filter_anisotropic') ||
                          Module.ctx.getExtension('WEBKIT_EXT_texture_filter_anisotropic');

      GL.floatExt = Module.ctx.getExtension('OES_texture_float');

      GL.elementIndexUintExt = Module.ctx.getExtension('OES_element_index_uint');
      GL.standardDerivativesExt = Module.ctx.getExtension('OES_standard_derivatives');
    }
  },

  glPixelStorei__sig: 'vii',
  glPixelStorei: function(pname, param) {
    if (pname == 0x0D05 /* GL_PACK_ALIGNMENT */) {
      GL.packAlignment = param;
    } else if (pname == 0x0cf5 /* GL_UNPACK_ALIGNMENT */) {
      GL.unpackAlignment = param;
    }
    Module.ctx.pixelStorei(pname, param);
  },

  glGetString__sig: 'ii',
  glGetString: function(name_) {
    switch(name_) {
      case 0x1F00 /* GL_VENDOR */:
      case 0x1F01 /* GL_RENDERER */:
      case 0x1F02 /* GL_VERSION */:
        return allocate(intArrayFromString(Module.ctx.getParameter(name_)), 'i8', ALLOC_NORMAL);
      case 0x1F03 /* GL_EXTENSIONS */:
        return allocate(intArrayFromString(Module.ctx.getSupportedExtensions().join(' ')), 'i8', ALLOC_NORMAL);
      case 0x8B8C /* GL_SHADING_LANGUAGE_VERSION */:
        return allocate(intArrayFromString('OpenGL ES GLSL 1.00 (WebGL)'), 'i8', ALLOC_NORMAL);
      default:
        throw 'Failure: Invalid glGetString value: ' + name_;
    }
  },

  glGetIntegerv__sig: 'vii',
  glGetIntegerv: function(name_, p) {
    switch(name_) { // Handle a few trivial GLES values 
      case 0x8DFA: // GL_SHADER_COMPILER
        {{{ makeSetValue('p', '0', '1', 'i32') }}};
        return;
      case 0x8DF9: // GL_NUM_SHADER_BINARY_FORMATS
        {{{ makeSetValue('p', '0', '0', 'i32') }}};
        return;
    }
    var result = Module.ctx.getParameter(name_);
    switch (typeof(result)) {
      case "number":
        {{{ makeSetValue('p', '0', 'result', 'i32') }}};
        break;
      case "boolean":
        {{{ makeSetValue('p', '0', 'result ? 1 : 0', 'i8') }}};
        break;
      case "string":
        throw 'Native code calling glGetIntegerv(' + name_ + ') on a name which returns a string!';
      case "object":
        if (result === null) {
          {{{ makeSetValue('p', '0', '0', 'i32') }}};
        } else if (result instanceof Float32Array ||
                   result instanceof Uint32Array ||
                   result instanceof Int32Array ||
                   result instanceof Array) {
          for (var i = 0; i < result.length; ++i) {
            {{{ makeSetValue('p', 'i*4', 'result[i]', 'i32') }}};
          }
        } else if (result instanceof WebGLBuffer) {
          {{{ makeSetValue('p', '0', 'GL.scan(GL.buffers, result)', 'i32') }}};
        } else if (result instanceof WebGLProgram) {
          {{{ makeSetValue('p', '0', 'GL.scan(GL.programs, result)', 'i32') }}};
        } else if (result instanceof WebGLFramebuffer) {
          {{{ makeSetValue('p', '0', 'GL.scan(GL.framebuffers, result)', 'i32') }}};
        } else if (result instanceof WebGLRenderbuffer) {
          {{{ makeSetValue('p', '0', 'GL.scan(GL.renderbuffers, result)', 'i32') }}};
        } else if (result instanceof WebGLTexture) {
          {{{ makeSetValue('p', '0', 'GL.scan(GL.textures, result)', 'i32') }}};
        } else {
          throw 'Unknown object returned from WebGL getParameter';
        }
        break;
      case "undefined":
        throw 'Native code calling glGetIntegerv(' + name_ + ') and it returns undefined';
      default:
        throw 'Why did we hit the default case?';
    }
  },

  glGetFloatv__sig: 'vii',
  glGetFloatv: function(name_, p) {
    var result = Module.ctx.getParameter(name_);
    switch (typeof(result)) {
      case "number":
        {{{ makeSetValue('p', '0', 'result', 'float') }}};
        break;
      case "boolean":
        {{{ makeSetValue('p', '0', 'result ? 1.0 : 0.0', 'float') }}};
        break;
      case "string":
          {{{ makeSetValue('p', '0', '0', 'float') }}};
      case "object":
        if (result === null) {
          throw 'Native code calling glGetFloatv(' + name_ + ') and it returns null';
        } else if (result instanceof Float32Array ||
                   result instanceof Uint32Array ||
                   result instanceof Int32Array ||
                   result instanceof Array) {
          for (var i = 0; i < result.length; ++i) {
            {{{ makeSetValue('p', 'i*4', 'result[i]', 'float') }}};
          }
        } else if (result instanceof WebGLBuffer) {
          {{{ makeSetValue('p', '0', 'GL.scan(GL.buffers, result)', 'float') }}};
        } else if (result instanceof WebGLProgram) {
          {{{ makeSetValue('p', '0', 'GL.scan(GL.programs, result)', 'float') }}};
        } else if (result instanceof WebGLFramebuffer) {
          {{{ makeSetValue('p', '0', 'GL.scan(GL.framebuffers, result)', 'float') }}};
        } else if (result instanceof WebGLRenderbuffer) {
          {{{ makeSetValue('p', '0', 'GL.scan(GL.renderbuffers, result)', 'float') }}};
        } else if (result instanceof WebGLTexture) {
          {{{ makeSetValue('p', '0', 'GL.scan(GL.textures, result)', 'float') }}};
        } else {
          throw 'Unknown object returned from WebGL getParameter';
        }
        break;
      case "undefined":
        throw 'Native code calling glGetFloatv(' + name_ + ') and it returns undefined';
      default:
        throw 'Why did we hit the default case?';
    }
  },

  glGetBooleanv__sig: 'vii',
  glGetBooleanv: function(name_, p) {
    var result = Module.ctx.getParameter(name_);
    switch (typeof(result)) {
      case "number":
        {{{ makeSetValue('p', '0', 'result != 0', 'i8') }}};
        break;
      case "boolean":
        {{{ makeSetValue('p', '0', 'result != 0', 'i8') }}};
        break;
      case "string":
        throw 'Native code calling glGetBooleanv(' + name_ + ') on a name which returns a string!';
      case "object":
        if (result === null) {
          {{{ makeSetValue('p', '0', '0', 'i8') }}};
        } else if (result instanceof Float32Array ||
                   result instanceof Uint32Array ||
                   result instanceof Int32Array ||
                   result instanceof Array) {
          for (var i = 0; i < result.length; ++i) {
            {{{ makeSetValue('p', 'i', 'result[i] != 0', 'i8') }}};
          }
        } else if (result instanceof WebGLBuffer ||
                   result instanceof WebGLProgram ||
                   result instanceof WebGLFramebuffer ||
                   result instanceof WebGLRenderbuffer ||
                   result instanceof WebGLTexture) {
          {{{ makeSetValue('p', '0', '1', 'i8') }}}; // non-zero ID is always 1!
        } else {
          throw 'Unknown object returned from WebGL getParameter';
        }
        break;
      case "undefined":
          throw 'Unknown object returned from WebGL getParameter';
      default:
        throw 'Why did we hit the default case?';
    }
  },

  glGenTextures__sig: 'vii',
  glGenTextures: function(n, textures) {
    for (var i = 0; i < n; i++) {
      var id = GL.getNewId(GL.textures); 
      GL.textures[id] = Module.ctx.createTexture();
      {{{ makeSetValue('textures', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteTextures__sig: 'vii',
  glDeleteTextures: function(n, textures) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('textures', 'i*4', 'i32') }}};
      Module.ctx.deleteTexture(GL.textures[id]);
      GL.textures[id] = null;
    }
  },

  glCompressedTexImage2D__sig: 'viiiiiiii',
  glCompressedTexImage2D: function(target, level, internalFormat, width, height, border, imageSize, data) {
    assert(GL.compressionExt);
    if (data) {
      data = {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}};
    } else {
      data = null;
    }
    Module.ctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, data);
  },

  glCompressedTexSubImage2D__sig: 'viiiiiiiii',
  glCompressedTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, imageSize, data) {
    assert(GL.compressionExt);
    if (data) {
      data = {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}};
    } else {
      data = null;
    }
    Module.ctx['compressedTexSubImage2D'](target, level, xoffset, yoffset, width, height, data);
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
    Module.ctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
  },

  glTexSubImage2D__sig: 'viiiiiiiii',
  glTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, type, pixels) {
    if (pixels) {
      var data = GL.getTexPixelData(type, format, width, height, pixels, -1);
      pixels = data.pixels;
    } else {
      pixels = null;
    }
    Module.ctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
  },

  glReadPixels__sig: 'viiiiiii',
  glReadPixels: function(x, y, width, height, format, type, pixels) {
    assert(type == 0x1401 /* GL_UNSIGNED_BYTE */);
    var sizePerPixel;
    switch (format) {
      case 0x1907 /* GL_RGB */:
        sizePerPixel = 3;
        break;
      case 0x1908 /* GL_RGBA */:
        sizePerPixel = 4;
        break;
      default: throw 'unsupported glReadPixels format';
    }
    var totalSize = width*height*sizePerPixel;
    Module.ctx.readPixels(x, y, width, height, format, type, HEAPU8.subarray(pixels, pixels + totalSize));
  },

  glBindTexture__sig: 'vii',
  glBindTexture: function(target, texture) {
    Module.ctx.bindTexture(target, texture ? GL.textures[texture] : null);
  },

  glGetTexParameterfv__sig: 'viii',
  glGetTexParameterfv: function(target, pname, params) {
    {{{ makeSetValue('params', '0', 'Module.getTexParameter(target, pname)', 'float') }}};
  },

  glGetTexParameteriv__sig: 'viii',
  glGetTexParameteriv: function(target, pname, params) {
    {{{ makeSetValue('params', '0', 'Module.getTexParameter(target, pname)', 'i32') }}};
  },

  glTexParameterfv__sig: 'viii',
  glTexParameterfv: function(target, pname, params) {
    var param = {{{ makeGetValue('params', '0', 'float') }}};
    Module.ctx.texParameterf(target, pname, param);
  },

  glTexParameteriv__sig: 'viii',
  glTexParameteriv: function(target, pname, params) {
    var param = {{{ makeGetValue('params', '0', 'i32') }}};
    Module.ctx.texParameteri(target, pname, param);
  },

  glIsTexture__sig: 'ii',
  glIsTexture: function(texture) {
    var texture = GL.textures[texture];
    if (!texture) return 0;
    return Module.ctx.isTexture(texture);
  },

  glGenBuffers__sig: 'vii',
  glGenBuffers: function(n, buffers) {
    for (var i = 0; i < n; i++) {
      var id = GL.getNewId(GL.buffers);
      GL.buffers[id] = Module.ctx.createBuffer();
      {{{ makeSetValue('buffers', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteBuffers__sig: 'vii',
  glDeleteBuffers: function(n, buffers) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('buffers', 'i*4', 'i32') }}};
      Module.ctx.deleteBuffer(GL.buffers[id]);
      GL.buffers[id] = null;

      if (id == GL.currArrayBuffer) GL.currArrayBuffer = 0;
      if (id == GL.currElementArrayBuffer) GL.currElementArrayBuffer = 0;
    }
  },

  glGetBufferParameteriv__sig: 'viii',
  glGetBufferParameteriv: function(target, value, data) {
    {{{ makeSetValue('data', '0', 'Module.ctx.getBufferParameter(target, value)', 'i32') }}};
  },

  glBufferData__sig: 'viiii',
  glBufferData: function(target, size, data, usage) {
    Module.ctx.bufferData(target, HEAPU8.subarray(data, data+size), usage);
  },

  glBufferSubData__sig: 'viiii',
  glBufferSubData: function(target, offset, size, data) {
    Module.ctx.bufferSubData(target, offset, HEAPU8.subarray(data, data+size));
  },

  glIsBuffer__sig: 'ii',
  glIsBuffer: function(buffer) {
    var b = GL.buffers[buffer];
    if (!b) return 0;
    return Module.ctx.isBuffer(b);
  },

  glGenRenderbuffers__sig: 'vii',
  glGenRenderbuffers: function(n, renderbuffers) {
    for (var i = 0; i < n; i++) {
      var id = GL.getNewId(GL.renderbuffers);
      GL.renderbuffers[id] = Module.ctx.createRenderbuffer();
      {{{ makeSetValue('renderbuffers', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteRenderbuffers__sig: 'vii',
  glDeleteRenderbuffers: function(n, renderbuffers) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('renderbuffers', 'i*4', 'i32') }}};
      Module.ctx.deleteRenderbuffer(GL.renderbuffers[id]);
      GL.renderbuffers[id];
    }
  },

  glBindRenderbuffer__sig: 'vii',
  glBindRenderbuffer: function(target, renderbuffer) {
    Module.ctx.bindRenderbuffer(target, renderbuffer ? GL.renderbuffers[renderbuffer] : null);
  },

  glGetRenderbufferParameteriv__sig: 'viii',
  glGetRenderbufferParameteriv: function(target, pname, params) {
    {{{ makeSetValue('params', '0', 'Module.ctx.getRenderbufferParameter(target, pname)', 'i32') }}};
  },

  glIsRenderbuffer__sig: 'ii',
  glIsRenderbuffer: function(renderbuffer) {
    var rb = GL.renderbuffers[renderbuffer];
    if (!rb) return 0;
    return Module.ctx.isRenderbuffer(rb);
  },

  glGetUniformfv__sig: 'viii',
  glGetUniformfv: function(program, location, params) {
    var data = Module.ctx.getUniform(GL.programs[program], GL.uniforms[location]);
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
    var data = Module.ctx.getUniform(GL.programs[program], GL.uniforms[location]);
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
    name = Pointer_stringify(name);
    var ptable = GL.uniformTable[program];
    if (!ptable) ptable = GL.uniformTable[program] = {};
    var id = ptable[name];
    if (id) return id; 
    var loc = Module.ctx.getUniformLocation(GL.programs[program], name);
    if (!loc) return -1;
    id = GL.getNewId(GL.uniforms);
    GL.uniforms[id] = loc;
    ptable[name] = id;
    return id;
  },

  glGetVertexAttribfv__sig: 'viii',
  glGetVertexAttribfv: function(index, pname, params) {
#if FULL_ES2
    if (GL.clientBuffers[index].enabled) {
      Module.printErr("glGetVertexAttribfv on client-side array: not supported, bad data returned");
    }
#endif
    var data = Module.ctx.getVertexAttrib(index, pname);
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
#if FULL_ES2
    if (GL.clientBuffers[index].enabled) {
      Module.printErr("glGetVertexAttribiv on client-side array: not supported, bad data returned");
    }
#endif
    var data = Module.ctx.getVertexAttrib(index, pname);
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
#if FULL_ES2
    if (GL.clientBuffers[index].enabled) {
      Module.printErr("glGetVertexAttribPointer on client-side array: not supported, bad data returned");
    }
#endif
    {{{ makeSetValue('pointer', '0', 'Module.ctx.getVertexAttribOffset(index, pname)', 'i32') }}};
  },

  glGetActiveUniform__sig: 'viiiiiii',
  glGetActiveUniform: function(program, index, bufSize, length, size, type, name) {
    program = GL.programs[program];
    var info = Module.ctx.getActiveUniform(program, index);

    var infoname = info.name.slice(0, Math.max(0, bufSize - 1));
    writeStringToMemory(infoname, name);

    if (length) {
      {{{ makeSetValue('length', '0', 'infoname.length', 'i32') }}};
    }
    if (size) {
      {{{ makeSetValue('size', '0', 'info.size', 'i32') }}};
    }
    if (type) {
      {{{ makeSetValue('type', '0', 'info.type', 'i32') }}};
    }
  },

  glUniform1f__sig: 'vif',
  glUniform1f: function(location, v0) {
    location = GL.uniforms[location];
    Module.ctx.uniform1f(location, v0);
  },

  glUniform2f__sig: 'viff',
  glUniform2f: function(location, v0, v1) {
    location = GL.uniforms[location];
    Module.ctx.uniform2f(location, v0, v1);
  },

  glUniform3f__sig: 'vifff',
  glUniform3f: function(location, v0, v1, v2) {
    location = GL.uniforms[location];
    Module.ctx.uniform3f(location, v0, v1, v2);
  },

  glUniform4f__sig: 'viffff',
  glUniform4f: function(location, v0, v1, v2, v3) {
    location = GL.uniforms[location];
    Module.ctx.uniform4f(location, v0, v1, v2, v3);
  },

  glUniform1i__sig: 'vii',
  glUniform1i: function(location, v0) {
    location = GL.uniforms[location];
    Module.ctx.uniform1i(location, v0);
  },

  glUniform2i__sig: 'viii',
  glUniform2i: function(location, v0, v1) {
    location = GL.uniforms[location];
    Module.ctx.uniform2i(location, v0, v1);
  },

  glUniform3i__sig: 'viiii',
  glUniform3i: function(location, v0, v1, v2) {
    location = GL.uniforms[location];
    Module.ctx.uniform3i(location, v0, v1, v2);
  },

  glUniform4i__sig: 'viiiii',
  glUniform4i: function(location, v0, v1, v2, v3) {
    location = GL.uniforms[location];
    Module.ctx.uniform4i(location, v0, v1, v2, v3);
  },

  glUniform1iv__sig: 'viii',
  glUniform1iv: function(location, count, value) {
    location = GL.uniforms[location];
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform1iv(location, value);
  },

  glUniform2iv__sig: 'viii',
  glUniform2iv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 2;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform2iv(location, value);
  },

  glUniform3iv__sig: 'viii',
  glUniform3iv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 3;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform3iv(location, value);
  },

  glUniform4iv__sig: 'viii',
  glUniform4iv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 4;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform4iv(location, value);
  },

  glUniform1fv__sig: 'viii',
  glUniform1fv: function(location, count, value) {
    location = GL.uniforms[location];
    var view;
    if (count == 1) {
      // avoid allocation for the common case of uploading one uniform
      view = GL.miniTempBufferViews[0];
      view[0] = {{{ makeGetValue('value', '0', 'float') }}};
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
    }
    Module.ctx.uniform1fv(location, view);
  },

  glUniform2fv__sig: 'viii',
  glUniform2fv: function(location, count, value) {
    location = GL.uniforms[location];
    var view;
    if (count == 1) {
      // avoid allocation for the common case of uploading one uniform
      view = GL.miniTempBufferViews[1];
      view[0] = {{{ makeGetValue('value', '0', 'float') }}};
      view[1] = {{{ makeGetValue('value', '4', 'float') }}};
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*8') }}};
    }
    Module.ctx.uniform2fv(location, view);
  },

  glUniform3fv__sig: 'viii',
  glUniform3fv: function(location, count, value) {
    location = GL.uniforms[location];
    var view;
    if (count == 1) {
      // avoid allocation for the common case of uploading one uniform
      view = GL.miniTempBufferViews[2];
      view[0] = {{{ makeGetValue('value', '0', 'float') }}};
      view[1] = {{{ makeGetValue('value', '4', 'float') }}};
      view[2] = {{{ makeGetValue('value', '8', 'float') }}};
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*12') }}};
    }
    Module.ctx.uniform3fv(location, view);
  },

  glUniform4fv__sig: 'viii',
  glUniform4fv: function(location, count, value) {
    location = GL.uniforms[location];
    var view;
    if (count == 1) {
      // avoid allocation for the common case of uploading one uniform
      view = GL.miniTempBufferViews[3];
      view[0] = {{{ makeGetValue('value', '0', 'float') }}};
      view[1] = {{{ makeGetValue('value', '4', 'float') }}};
      view[2] = {{{ makeGetValue('value', '8', 'float') }}};
      view[3] = {{{ makeGetValue('value', '12', 'float') }}};
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*16') }}};
    }
    Module.ctx.uniform4fv(location, view);
  },

  glUniformMatrix2fv__sig: 'viiii',
  glUniformMatrix2fv: function(location, count, transpose, value) {
    location = GL.uniforms[location];
    var view;
    if (count == 1) {
      // avoid allocation for the common case of uploading one uniform matrix
      view = GL.miniTempBufferViews[3];
      for (var i = 0; i < 4; i++) {
        view[i] = {{{ makeGetValue('value', 'i*4', 'float') }}};
      }
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*16') }}};
    }
    Module.ctx.uniformMatrix2fv(location, transpose, view);
  },

  glUniformMatrix3fv__sig: 'viiii',
  glUniformMatrix3fv: function(location, count, transpose, value) {
    location = GL.uniforms[location];
    var view;
    if (count == 1) {
      // avoid allocation for the common case of uploading one uniform matrix
      view = GL.miniTempBufferViews[8];
      for (var i = 0; i < 9; i++) {
        view[i] = {{{ makeGetValue('value', 'i*4', 'float') }}};
      }
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*36') }}};
    }
    Module.ctx.uniformMatrix3fv(location, transpose, view);
  },

  glUniformMatrix4fv__sig: 'viiii',
  glUniformMatrix4fv: function(location, count, transpose, value) {
    location = GL.uniforms[location];
    var view;
    if (count == 1) {
      // avoid allocation for the common case of uploading one uniform matrix
      view = GL.miniTempBufferViews[15];
      for (var i = 0; i < 16; i++) {
        view[i] = {{{ makeGetValue('value', 'i*4', 'float') }}};
      }
    } else {
      view = {{{ makeHEAPView('F32', 'value', 'value+count*64') }}};
    }
    Module.ctx.uniformMatrix4fv(location, transpose, view);
  },

  glBindBuffer__sig: 'vii',
  glBindBuffer: function(target, buffer) {
    if (target == Module.ctx.ARRAY_BUFFER) {
      GL.currArrayBuffer = buffer;
    } else if (target == Module.ctx.ELEMENT_ARRAY_BUFFER) {
      GL.currElementArrayBuffer = buffer;
    }

    Module.ctx.bindBuffer(target, buffer ? GL.buffers[buffer] : null);
  },

  glVertexAttrib1fv__sig: 'vii',
  glVertexAttrib1fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+' + (1*4)) }}};
    Module.ctx.vertexAttrib1fv(index, v);
  },

  glVertexAttrib2fv__sig: 'vii',
  glVertexAttrib2fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+' + (2*4)) }}};
    Module.ctx.vertexAttrib2fv(index, v);
  },

  glVertexAttrib3fv__sig: 'vii',
  glVertexAttrib3fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+' + (3*4)) }}};
    Module.ctx.vertexAttrib3fv(index, v);
  },

  glVertexAttrib4fv__sig: 'vii',
  glVertexAttrib4fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+' + (4*4)) }}};
    Module.ctx.vertexAttrib4fv(index, v);
  },

  glGetAttribLocation__sig: 'vii',
  glGetAttribLocation: function(program, name) {
    program = GL.programs[program];
    name = Pointer_stringify(name);
    return Module.ctx.getAttribLocation(program, name);
  },

  glGetActiveAttrib__sig: 'viiiiiii',
  glGetActiveAttrib: function(program, index, bufSize, length, size, type, name) {
    program = GL.programs[program];
    var info = Module.ctx.getActiveAttrib(program, index);

    var infoname = info.name.slice(0, Math.max(0, bufSize - 1));
    writeStringToMemory(infoname, name);

    if (length) {
      {{{ makeSetValue('length', '0', 'infoname.length', 'i32') }}};
    }
    if (size) {
      {{{ makeSetValue('size', '0', 'info.size', 'i32') }}};
    }
    if (type) {
      {{{ makeSetValue('type', '0', 'info.type', 'i32') }}};
    }
  },

  glCreateShader__sig: 'ii',
  glCreateShader: function(shaderType) {
    var id = GL.getNewId(GL.shaders);
    GL.shaders[id] = Module.ctx.createShader(shaderType);
    return id;
  },

  glDeleteShader__sig: 'vi',
  glDeleteShader: function(shader) {
    Module.ctx.deleteShader(GL.shaders[shader]);
    GL.shaders[shader] = null;
  },

  glGetAttachedShaders__sig: 'viiii',
  glGetAttachedShaders: function(program, maxCount, count, shaders) {
    var result = Module.ctx.getAttachedShaders(GL.programs[program]);
    var len = result.length;
    if (len > maxCount) {
      len = maxCount;
    }
    {{{ makeSetValue('count', '0', 'len', 'i32') }}};
    for (var i = 0; i < len; ++i) {
      {{{ makeSetValue('shaders', 'i*4', 'GL.shaders[result[i]]', 'i32') }}};
    }
  },

  glShaderSource__sig: 'viiii',
  glShaderSource: function(shader, count, string, length) {
    var source = GL.getSource(shader, count, string, length);
    Module.ctx.shaderSource(GL.shaders[shader], source);
  },

  glGetShaderSource__sig: 'viiii',
  glGetShaderSource: function(shader, bufSize, length, source) {
    var result = Module.ctx.getShaderSource(GL.shaders[shader]);
    result = result.slice(0, Math.max(0, bufSize - 1));
    writeStringToMemory(result, source);
    if (length) {
      {{{ makeSetValue('length', '0', 'result.length', 'i32') }}};
    }
  },

  glCompileShader__sig: 'vi',
  glCompileShader: function(shader) {
    Module.ctx.compileShader(GL.shaders[shader]);
  },

  glGetShaderInfoLog__sig: 'viiii',
  glGetShaderInfoLog: function(shader, maxLength, length, infoLog) {
    var log = Module.ctx.getShaderInfoLog(GL.shaders[shader]);
    // Work around a bug in Chromium which causes getShaderInfoLog to return null
    if (!log) {
      log = "";
    }
    log = log.substr(0, maxLength - 1);
    writeStringToMemory(log, infoLog);
    if (length) {
      {{{ makeSetValue('length', '0', 'log.length', 'i32') }}}
    }
  },

  glGetShaderiv__sig: 'viii',
  glGetShaderiv : function(shader, pname, p) {
    if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
      {{{ makeSetValue('p', '0', 'Module.ctx.getShaderInfoLog(GL.shaders[shader]).length + 1', 'i32') }}};
    } else {
      {{{ makeSetValue('p', '0', 'Module.ctx.getShaderParameter(GL.shaders[shader], pname)', 'i32') }}};
    }
  },

  glGetProgramiv__sig: 'viii',
  glGetProgramiv : function(program, pname, p) {
    if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
      {{{ makeSetValue('p', '0', 'Module.ctx.getProgramInfoLog(GL.programs[program]).length + 1', 'i32') }}};
    } else {
      {{{ makeSetValue('p', '0', 'Module.ctx.getProgramParameter(GL.programs[program], pname)', 'i32') }}};
    }
  },

  glIsShader__sig: 'ii',
  glIsShader: function(shader) {
    var s = GL.shaders[shader];
    if (!s) return 0;
    return Module.ctx.isShader(s);
  },

  glCreateProgram__sig: 'i',
  glCreateProgram: function() {
    var id = GL.getNewId(GL.programs);
    GL.programs[id] = Module.ctx.createProgram();
    return id;
  },

  glDeleteProgram__sig: 'vi',
  glDeleteProgram: function(program) {
    Module.ctx.deleteProgram(GL.programs[program]);
    GL.programs[program] = null;
    GL.uniformTable[program] = null;
  },

  glAttachShader__sig: 'vii',
  glAttachShader: function(program, shader) {
    Module.ctx.attachShader(GL.programs[program],
                            GL.shaders[shader]);
  },

  glDetachShader__sig: 'vii',
  glDetachShader: function(program, shader) {
    Module.ctx.detachShader(GL.programs[program],
                            GL.shaders[shader]);
  },

  glGetShaderPrecisionFormat: function(shaderType, precisionType, range, precision) {
    var result = Module.ctx.getShaderPrecisionFormat(shaderType, precisionType);
    {{{ makeSetValue('range', '0', 'result.rangeMin', 'i32') }}};
    {{{ makeSetValue('range', '4', 'result.rangeMax', 'i32') }}};
    {{{ makeSetValue('precision', '0', 'result.precision', 'i32') }}};
  },

  glLinkProgram__sig: 'vi',
  glLinkProgram: function(program) {
    Module.ctx.linkProgram(GL.programs[program]);
    GL.uniformTable[program] = {}; // uniforms no longer keep the same names after linking
  },

  glGetProgramInfoLog__sig: 'viiii',
  glGetProgramInfoLog: function(program, maxLength, length, infoLog) {
    var log = Module.ctx.getProgramInfoLog(GL.programs[program]);
    // Work around a bug in Chromium which causes getProgramInfoLog to return null
    if (!log) {
      log = "";
    }
    log = log.substr(0, maxLength - 1);
    writeStringToMemory(log, infoLog);
    if (length) {
      {{{ makeSetValue('length', '0', 'log.length', 'i32') }}}
    }
  },

  glUseProgram__sig: 'vi',
  glUseProgram: function(program) {
    Module.ctx.useProgram(program ? GL.programs[program] : null);
  },

  glValidateProgram__sig: 'vi',
  glValidateProgram: function(program) {
    Module.ctx.validateProgram(GL.programs[program]);
  },

  glIsProgram__sig: 'ii',
  glIsProgram: function(program) {
    var program = GL.programs[program];
    if (!program) return 0;
    return Module.ctx.isProgram(program);
  },

  glBindAttribLocation__sig: 'viii',
  glBindAttribLocation: function(program, index, name) {
    name = Pointer_stringify(name);
    Module.ctx.bindAttribLocation(GL.programs[program], index, name);
  },

  glBindFramebuffer__sig: 'vii',
  glBindFramebuffer: function(target, framebuffer) {
    Module.ctx.bindFramebuffer(target, framebuffer ? GL.framebuffers[framebuffer] : null);
  },

  glGenFramebuffers__sig: 'vii',
  glGenFramebuffers: function(n, ids) {
    for (var i = 0; i < n; ++i) {
      var id = GL.getNewId(GL.framebuffers);
      GL.framebuffers[id] = Module.ctx.createFramebuffer();
      {{{ makeSetValue('ids', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteFramebuffers__sig: 'vii',
  glDeleteFramebuffers: function(n, framebuffers) {
    for (var i = 0; i < n; ++i) {
      var id = {{{ makeGetValue('framebuffers', 'i*4', 'i32') }}};
      Module.ctx.deleteFramebuffer(GL.framebuffers[id]);
      GL.framebuffers[id] = null;
    }
  },

  glFramebufferRenderbuffer__sig: 'viiii',
  glFramebufferRenderbuffer: function(target, attachment, renderbuffertarget, renderbuffer) {
    Module.ctx.framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                       GL.renderbuffers[renderbuffer]);
  },

  glFramebufferTexture2D__sig: 'viiiii',
  glFramebufferTexture2D: function(target, attachment, textarget, texture, level) {
    Module.ctx.framebufferTexture2D(target, attachment, textarget,
                                    GL.textures[texture], level);
  },

  glGetFramebufferAttachmentParameteriv__sig: 'viiii',
  glGetFramebufferAttachmentParameteriv: function(target, attachment, pname, params) {
    var result = Module.ctx.getFramebufferAttachmentParameter(target, attachment, pname);
    {{{ makeSetValue('params', '0', 'params', 'i32') }}};
  },

  glIsFramebuffer__sig: 'ii',
  glIsFramebuffer: function(framebuffer) {
    var fb = GL.framebuffers[framebuffer];
    if (!fb) return 0;
    return Module.ctx.isFramebuffer(fb);
  },

  // GL emulation: provides misc. functionality not present in OpenGL ES 2.0 or WebGL

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

    init: function() {
      GLEmulation.fogColor = new Float32Array(4);

      // Add some emulation workarounds
      Module.printErr('WARNING: using emscripten GL emulation. This is a collection of limited workarounds, do not expect it to work');
#if GL_UNSAFE_OPTS == 0
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

      _glEnable = function(cap) {
        // Clean up the renderer on any change to the rendering state. The optimization of
        // skipping renderer setup is aimed at the case of multiple glDraw* right after each other
        if (GL.immediate.lastRenderer) GL.immediate.lastRenderer.cleanup();
        if (cap == 0x0B60 /* GL_FOG */) {
          GLEmulation.fogEnabled = true;
          return;
        } else if (cap == 0x0de1 /* GL_TEXTURE_2D */) {
          // XXX not according to spec, and not in desktop GL, but works in some GLES1.x apparently, so support
          // it by forwarding to glEnableClientState
          _glEnableClientState(cap);
          return;
        } else if (!(cap in validCapabilities)) {
          return;
        }
        Module.ctx.enable(cap);
      };
      _glDisable = function(cap) {
        if (GL.immediate.lastRenderer) GL.immediate.lastRenderer.cleanup();
        if (cap == 0x0B60 /* GL_FOG */) {
          GLEmulation.fogEnabled = false;
          return;
        } else if (cap == 0x0de1 /* GL_TEXTURE_2D */) {
          // XXX not according to spec, and not in desktop GL, but works in some GLES1.x apparently, so support
          // it by forwarding to glDisableClientState
          _glDisableClientState(cap);
          return;
        } else if (!(cap in validCapabilities)) {
          return;
        }
        Module.ctx.disable(cap);
      };
      _glIsEnabled = function(cap) {
        if (cap == 0x0B60 /* GL_FOG */) {
          return GLEmulation.fogEnabled ? 1 : 0;
        } else if (!(cap in validCapabilities)) {
          return 0;
        }
        return Module.ctx.isEnabled(cap);
      };

      var glGetBooleanv = _glGetBooleanv;
      _glGetBooleanv = function(pname, p) {
        var attrib = GLEmulation.getAttributeFromCapability(pname);
        if (attrib !== null) {
          var result = GL.immediate.enabledClientAttributes[attrib];
          {{{ makeSetValue('p', '0', 'result === true ? 1 : 0', 'i8') }}};
          return;
        }
        glGetBooleanv(pname, p);
      };

      var glGetIntegerv = _glGetIntegerv;
      _glGetIntegerv = function(pname, params) {
        switch (pname) {
          case 0x84E2: pname = Module.ctx.MAX_TEXTURE_IMAGE_UNITS /* fake it */; break; // GL_MAX_TEXTURE_UNITS
          case 0x8B4A: { // GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB
            var result = Module.ctx.getParameter(Module.ctx.MAX_VERTEX_UNIFORM_VECTORS);
            {{{ makeSetValue('params', '0', 'result*4', 'i32') }}}; // GLES gives num of 4-element vectors, GL wants individual components, so multiply
            return;
          }
          case 0x8B49: { // GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB
            var result = Module.ctx.getParameter(Module.ctx.MAX_FRAGMENT_UNIFORM_VECTORS);
            {{{ makeSetValue('params', '0', 'result*4', 'i32') }}}; // GLES gives num of 4-element vectors, GL wants individual components, so multiply
            return;
          }
          case 0x8B4B: { // GL_MAX_VARYING_FLOATS_ARB
            var result = Module.ctx.getParameter(Module.ctx.MAX_VARYING_VECTORS);
            {{{ makeSetValue('params', '0', 'result*4', 'i32') }}}; // GLES gives num of 4-element vectors, GL wants individual components, so multiply
            return;
          }
          case 0x8871: pname = Module.ctx.MAX_COMBINED_TEXTURE_IMAGE_UNITS /* close enough */; break; // GL_MAX_TEXTURE_COORDS
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
            var attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.size : 0', 'i32') }}};
            return;
          }
          case 0x8089: { // GL_TEXTURE_COORD_ARRAY_TYPE
            var attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.type : 0', 'i32') }}};
            return;
          }
          case 0x808A: { // GL_TEXTURE_COORD_ARRAY_STRIDE
            var attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0];
            {{{ makeSetValue('params', '0', 'attribute ? attribute.stride : 0', 'i32') }}};
            return;
          }
        }
        glGetIntegerv(pname, params);
      };

      var glGetString = _glGetString;
      _glGetString = function(name_) {
        switch(name_) {
          case 0x1F03 /* GL_EXTENSIONS */: // Add various extensions that we can support
            return allocate(intArrayFromString(Module.ctx.getSupportedExtensions().join(' ') +
                   ' GL_EXT_texture_env_combine GL_ARB_texture_env_crossbar GL_ATI_texture_env_combine3 GL_NV_texture_env_combine4 GL_EXT_texture_env_dot3 GL_ARB_multitexture GL_ARB_vertex_buffer_object GL_EXT_framebuffer_object GL_ARB_vertex_program GL_ARB_fragment_program GL_ARB_shading_language_100 GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader GL_ARB_texture_cube_map GL_EXT_draw_range_elements' +
                   (GL.compressionExt ? ' GL_ARB_texture_compression GL_EXT_texture_compression_s3tc' : '') +
                   (GL.anisotropicExt ? ' GL_EXT_texture_filter_anisotropic' : '')
            ), 'i8', ALLOC_NORMAL);
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
      _glCreateShader = function(shaderType) {
        var id = glCreateShader(shaderType);
        GL.shaderInfos[id] = {
          type: shaderType,
          ftransform: false
        };
        return id;
      };

      var glShaderSource = _glShaderSource;
      _glShaderSource = function(shader, count, string, length) {
        var source = GL.getSource(shader, count, string, length);
#if GL_DEBUG
        GL.shaderOriginalSources[shader] = source;
#endif
        // XXX We add attributes and uniforms to shaders. The program can ask for the # of them, and see the
        // ones we generated, potentially confusing it? Perhaps we should hide them.
        if (GL.shaderInfos[shader].type == Module.ctx.VERTEX_SHADER) {
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
          for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
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
                     'uniform vec4 u_color; \n' +
                     'uniform int u_hasColorAttrib; \n' +
                     source.replace(/gl_Color/g, '(u_hasColorAttrib > 0 ? a_color : u_color)');
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
        } else { // Fragment shader
          for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
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
          source = 'precision mediump float;\n' + source;
        }
#if GL_DEBUG
        GL.shaderSources[shader] = source;
#endif
        Module.ctx.shaderSource(GL.shaders[shader], source);
      };

      var glCompileShader = _glCompileShader;
      _glCompileShader = function(shader) {
        Module.ctx.compileShader(GL.shaders[shader]);
        if (!Module.ctx.getShaderParameter(GL.shaders[shader], Module.ctx.COMPILE_STATUS)) {
          Module.printErr('Failed to compile shader: ' + Module.ctx.getShaderInfoLog(GL.shaders[shader]));
          Module.printErr('Info: ' + JSON.stringify(GL.shaderInfos[shader]));
#if GL_DEBUG
          Module.printErr('Original source: ' + GL.shaderOriginalSources[shader]);
          Module.printErr('Source: ' + GL.shaderSources[shader]);
          throw 'Shader compilation halt';
#else
          Module.printErr('Enable GL_DEBUG to see shader source');
#endif
        }
      };

      GL.programShaders = {};
      var glAttachShader = _glAttachShader;
      _glAttachShader = function(program, shader) {
        if (!GL.programShaders[program]) GL.programShaders[program] = [];
        GL.programShaders[program].push(shader);
        glAttachShader(program, shader);
      };

      var glDetachShader = _glDetachShader;
      _glDetachShader = function(program, shader) {
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
      _glUseProgram = function(program) {
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
        GL.currProgram = program;
        glUseProgram(program);
      }

      var glDeleteProgram = _glDeleteProgram;
      _glDeleteProgram = function(program) {
        glDeleteProgram(program);
        if (program == GL.currProgram) GL.currProgram = 0;
      };

      // If attribute 0 was not bound, bind it to 0 for WebGL performance reasons. Track if 0 is free for that.
      var zeroUsedPrograms = {};
      var glBindAttribLocation = _glBindAttribLocation;
      _glBindAttribLocation = function(program, index, name) {
        if (index == 0) zeroUsedPrograms[program] = true;
        glBindAttribLocation(program, index, name);
      };
      var glLinkProgram = _glLinkProgram;
      _glLinkProgram = function(program) {
        if (!(program in zeroUsedPrograms)) {
          Module.ctx.bindAttribLocation(GL.programs[program], 0, 'a_position');
        }
        glLinkProgram(program);
      };

      var glBindBuffer = _glBindBuffer;
      _glBindBuffer = function(target, buffer) {
        glBindBuffer(target, buffer);
        if (target == Module.ctx.ARRAY_BUFFER) {
          if (GLEmulation.currentVao) {
            assert(GLEmulation.currentVao.arrayBuffer == buffer || GLEmulation.currentVao.arrayBuffer == 0 || buffer == 0, 'TODO: support for multiple array buffers in vao');
            GLEmulation.currentVao.arrayBuffer = buffer;
          }
        } else if (target == Module.ctx.ELEMENT_ARRAY_BUFFER) {
          if (GLEmulation.currentVao) GLEmulation.currentVao.elementArrayBuffer = buffer;
        }
      };

      var glGetFloatv = _glGetFloatv;
      _glGetFloatv = function(pname, params) {
        if (pname == 0x0BA6) { // GL_MODELVIEW_MATRIX
          HEAPF32.set(GL.immediate.matrix['m'], params >> 2);
        } else if (pname == 0x0BA7) { // GL_PROJECTION_MATRIX
          HEAPF32.set(GL.immediate.matrix['p'], params >> 2);
        } else if (pname == 0x0BA8) { // GL_TEXTURE_MATRIX
          HEAPF32.set(GL.immediate.matrix['t' + GL.immediate.clientActiveTexture], params >> 2);
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
      _glHint = function(target, mode) {
        if (target == 0x84EF) { // GL_TEXTURE_COMPRESSION_HINT
          return;
        }
        glHint(target, mode);
      };

      var glEnableVertexAttribArray = _glEnableVertexAttribArray;
      _glEnableVertexAttribArray = function(index) {
        glEnableVertexAttribArray(index);
        GLEmulation.enabledVertexAttribArrays[index] = 1;
        if (GLEmulation.currentVao) GLEmulation.currentVao.enabledVertexAttribArrays[index] = 1;
      };

      var glDisableVertexAttribArray = _glDisableVertexAttribArray;
      _glDisableVertexAttribArray = function(index) {
        glDisableVertexAttribArray(index);
        delete GLEmulation.enabledVertexAttribArrays[index];
        if (GLEmulation.currentVao) delete GLEmulation.currentVao.enabledVertexAttribArrays[index];
      };

      var glVertexAttribPointer = _glVertexAttribPointer;
      _glVertexAttribPointer = function(index, size, type, normalized, stride, pointer) {
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        if (GLEmulation.currentVao) { // TODO: avoid object creation here? likely not hot though
          GLEmulation.currentVao.vertexAttribPointers[index] = [index, size, type, normalized, stride, pointer];
        }
      };
    },

    getAttributeFromCapability: function(cap) {
      var attrib = null;
      switch (cap) {
        case 0x8078: // GL_TEXTURE_COORD_ARRAY
        case 0x0de1: // GL_TEXTURE_2D - XXX not according to spec, and not in desktop GL, but works in some GLES1.x apparently, so support it
          attrib = GL.immediate.TEXTURE0 + GL.immediate.clientActiveTexture; break;
        case 0x8074: // GL_VERTEX_ARRAY
          attrib = GL.immediate.VERTEX; break;
        case 0x8075: // GL_NORMAL_ARRAY
          attrib = GL.immediate.NORMAL; break;
        case 0x8076: // GL_COLOR_ARRAY
          attrib = GL.immediate.COLOR; break;
      }
      return attrib;
    },

    getProcAddress: function(name) {
      name = name.replace('EXT', '').replace('ARB', '');
      // Do the translation carefully because of closure
      var ret = 0;
      switch (name) {
        case 'glCreateShaderObject': case 'glCreateShader': ret = {{{ Functions.getIndex('_glCreateShader', true) }}}; break;
        case 'glCreateProgramObject': case 'glCreateProgram': ret = {{{ Functions.getIndex('_glCreateProgram', true) }}}; break;
        case 'glAttachObject': case 'glAttachShader': ret = {{{ Functions.getIndex('_glAttachShader', true) }}}; break;
        case 'glUseProgramObject': case 'glUseProgram': ret = {{{ Functions.getIndex('_glUseProgram', true) }}}; break;
        case 'glDetachObject': case 'glDetachShader': ret = {{{ Functions.getIndex('_glDetachShader', true) }}}; break;
        case 'glDeleteObject': ret = {{{ Functions.getIndex('_glDeleteObject', true) }}}; break;
        case 'glGetObjectParameteriv': ret = {{{ Functions.getIndex('_glGetObjectParameteriv', true) }}}; break;
        case 'glGetInfoLog': ret = {{{ Functions.getIndex('_glGetInfoLog', true) }}}; break;
        case 'glBindProgram': ret = {{{ Functions.getIndex('_glBindProgram', true) }}}; break;
        case 'glDrawRangeElements': ret = {{{ Functions.getIndex('_glDrawRangeElements', true) }}}; break;
        case 'glShaderSource': ret = {{{ Functions.getIndex('_glShaderSource', true) }}}; break;
        case 'glCompileShader': ret = {{{ Functions.getIndex('_glCompileShader', true) }}}; break;
        case 'glLinkProgram': ret = {{{ Functions.getIndex('_glLinkProgram', true) }}}; break;
        case 'glGetUniformLocation': ret = {{{ Functions.getIndex('_glGetUniformLocation', true) }}}; break;
        case 'glUniform1f': ret = {{{ Functions.getIndex('_glUniform1f', true) }}}; break;
        case 'glUniform2f': ret = {{{ Functions.getIndex('_glUniform2f', true) }}}; break;
        case 'glUniform3f': ret = {{{ Functions.getIndex('_glUniform3f', true) }}}; break;
        case 'glUniform4f': ret = {{{ Functions.getIndex('_glUniform4f', true) }}}; break;
        case 'glUniform1fv': ret = {{{ Functions.getIndex('_glUniform1fv', true) }}}; break;
        case 'glUniform2fv': ret = {{{ Functions.getIndex('_glUniform2fv', true) }}}; break;
        case 'glUniform3fv': ret = {{{ Functions.getIndex('_glUniform3fv', true) }}}; break;
        case 'glUniform4fv': ret = {{{ Functions.getIndex('_glUniform4fv', true) }}}; break;
        case 'glUniform1i': ret = {{{ Functions.getIndex('_glUniform1i', true) }}}; break;
        case 'glUniform2i': ret = {{{ Functions.getIndex('_glUniform2i', true) }}}; break;
        case 'glUniform3i': ret = {{{ Functions.getIndex('_glUniform3i', true) }}}; break;
        case 'glUniform4i': ret = {{{ Functions.getIndex('_glUniform4i', true) }}}; break;
        case 'glUniform1iv': ret = {{{ Functions.getIndex('_glUniform1iv', true) }}}; break;
        case 'glUniform2iv': ret = {{{ Functions.getIndex('_glUniform2iv', true) }}}; break;
        case 'glUniform3iv': ret = {{{ Functions.getIndex('_glUniform3iv', true) }}}; break;
        case 'glUniform4iv': ret = {{{ Functions.getIndex('_glUniform4iv', true) }}}; break;
        case 'glBindAttribLocation': ret = {{{ Functions.getIndex('_glBindAttribLocation', true) }}}; break;
        case 'glGetActiveUniform': ret = {{{ Functions.getIndex('_glGetActiveUniform', true) }}}; break;
        case 'glGenBuffers': ret = {{{ Functions.getIndex('_glGenBuffers', true) }}}; break;
        case 'glBindBuffer': ret = {{{ Functions.getIndex('_glBindBuffer', true) }}}; break;
        case 'glBufferData': ret = {{{ Functions.getIndex('_glBufferData', true) }}}; break;
        case 'glBufferSubData': ret = {{{ Functions.getIndex('_glBufferSubData', true) }}}; break;
        case 'glDeleteBuffers': ret = {{{ Functions.getIndex('_glDeleteBuffers', true) }}}; break;
        case 'glActiveTexture': ret = {{{ Functions.getIndex('_glActiveTexture', true) }}}; break;
        case 'glClientActiveTexture': ret = {{{ Functions.getIndex('_glClientActiveTexture', true) }}}; break;
        case 'glGetProgramiv': ret = {{{ Functions.getIndex('_glGetProgramiv', true) }}}; break;
        case 'glEnableVertexAttribArray': ret = {{{ Functions.getIndex('_glEnableVertexAttribArray', true) }}}; break;
        case 'glDisableVertexAttribArray': ret = {{{ Functions.getIndex('_glDisableVertexAttribArray', true) }}}; break;
        case 'glVertexAttribPointer': ret = {{{ Functions.getIndex('_glVertexAttribPointer', true) }}}; break;
	case 'glVertexAttrib1f': ret = {{{ Functions.getIndex('_glVertexAttrib1f', true) }}}; break;
	case 'glVertexAttrib2f': ret = {{{ Functions.getIndex('_glVertexAttrib2f', true) }}}; break;
	case 'glVertexAttrib3f': ret = {{{ Functions.getIndex('_glVertexAttrib3f', true) }}}; break;
	case 'glVertexAttrib4f': ret = {{{ Functions.getIndex('_glVertexAttrib4f', true) }}}; break;
	case 'glVertexAttrib1fv': ret = {{{ Functions.getIndex('_glVertexAttrib1fv', true) }}}; break;
	case 'glVertexAttrib2fv': ret = {{{ Functions.getIndex('_glVertexAttrib2fv', true) }}}; break;
	case 'glVertexAttrib3fv': ret = {{{ Functions.getIndex('_glVertexAttrib3fv', true) }}}; break;
	case 'glVertexAttrib4fv': ret = {{{ Functions.getIndex('_glVertexAttrib4fv', true) }}}; break;
	case 'glGetVertexAttribfv': ret = {{{ Functions.getIndex('_glGetVertexAttribfv', true) }}}; break;
	case 'glGetVertexAttribiv': ret = {{{ Functions.getIndex('_glGetVertexAttribiv', true) }}}; break;
	case 'glGetVertexAttribPointerv': ret = {{{ Functions.getIndex('_glGetVertexAttribPointerv', true) }}}; break;
	case 'glGetAttribLocation': ret = {{{ Functions.getIndex('_glGetAttribLocation', true) }}}; break;
	case 'glGetActiveAttrib': ret = {{{ Functions.getIndex('_glGetActiveAttrib', true) }}}; break;
        case 'glBindRenderbuffer': ret = {{{ Functions.getIndex('_glBindRenderbuffer', true) }}}; break;
        case 'glDeleteRenderbuffers': ret = {{{ Functions.getIndex('_glDeleteRenderbuffers', true) }}}; break;
        case 'glGenRenderbuffers': ret = {{{ Functions.getIndex('_glGenRenderbuffers', true) }}}; break;
        case 'glCompressedTexImage2D': ret = {{{ Functions.getIndex('_glCompressedTexImage2D', true) }}}; break;
        case 'glCompressedTexSubImage2D': ret = {{{ Functions.getIndex('_glCompressedTexSubImage2D', true) }}}; break;
        case 'glBindFramebuffer': ret = {{{ Functions.getIndex('_glBindFramebuffer', true) }}}; break;
        case 'glGenFramebuffers': ret = {{{ Functions.getIndex('_glGenFramebuffers', true) }}}; break;
        case 'glDeleteFramebuffers': ret = {{{ Functions.getIndex('_glDeleteFramebuffers', true) }}}; break;
        case 'glFramebufferRenderbuffer': ret = {{{ Functions.getIndex('_glFramebufferRenderbuffer', true) }}}; break;
        case 'glFramebufferTexture2D': ret = {{{ Functions.getIndex('_glFramebufferTexture2D', true) }}}; break;
        case 'glGetFramebufferAttachmentParameteriv': ret = {{{ Functions.getIndex('_glGetFramebufferAttachmentParameteriv', true) }}}; break;
        case 'glIsFramebuffer': ret = {{{ Functions.getIndex('_glIsFramebuffer', true) }}}; break;
        case 'glCheckFramebufferStatus': ret = {{{ Functions.getIndex('_glCheckFramebufferStatus', true) }}}; break;
        case 'glRenderbufferStorage': ret = {{{ Functions.getIndex('_glRenderbufferStorage', true) }}}; break;
        case 'glGenVertexArrays': ret = {{{ Functions.getIndex('_glGenVertexArrays', true) }}}; break;
        case 'glDeleteVertexArrays': ret = {{{ Functions.getIndex('_glDeleteVertexArrays', true) }}}; break;
        case 'glBindVertexArray': ret = {{{ Functions.getIndex('_glBindVertexArray', true) }}}; break;
        case 'glGetString': ret = {{{ Functions.getIndex('_glGetString', true) }}}; break;
	case 'glBindTexture': ret = {{{ Functions.getIndex('_glBindTexture', true) }}}; break;
	case 'glGetBufferParameteriv': ret = {{{ Functions.getIndex('_glGetBufferParameteriv', true) }}}; break;
	case 'glIsBuffer': ret = {{{ Functions.getIndex('_glIsBuffer', true) }}}; break;
	case 'glDeleteShader': ret = {{{ Functions.getIndex('_glDeleteShader', true) }}}; break;
        case 'glUniformMatrix2fv': ret = {{{ Functions.getIndex('_glUniformMatrix2fv', true) }}}; break;
        case 'glUniformMatrix3fv': ret = {{{ Functions.getIndex('_glUniformMatrix3fv', true) }}}; break;
        case 'glUniformMatrix4fv': ret = {{{ Functions.getIndex('_glUniformMatrix4fv', true) }}}; break;
	case 'glIsRenderbuffer': ret = {{{ Functions.getIndex('_glIsRenderbuffer', true) }}}; break;
	case 'glBlendEquation': ret = {{{ Functions.getIndex('_glBlendEquation', true) }}}; break;
	case 'glBlendFunc': ret = {{{ Functions.getIndex('_glBlendFunc', true) }}}; break;
	case 'glBlendFuncSeparate': ret = {{{ Functions.getIndex('_glBlendFuncSeparate', true) }}}; break;
	case 'glBlendEquationSeparate': ret = {{{ Functions.getIndex('_glBlendEquationSeparate', true) }}}; break;
	case 'glDepthRangef': ret = {{{ Functions.getIndex('_glDepthRangef', true) }}}; break;
	case 'glClear': ret = {{{ Functions.getIndex('_glClear', true) }}}; break;
	case 'glGenerateMipmap': ret = {{{ Functions.getIndex('_glGenerateMipmap', true) }}}; break;
	case 'glBlendColor': ret = {{{ Functions.getIndex('_glBlendColor', true) }}}; break;
	case 'glClearDepthf': ret = {{{ Functions.getIndex('_glClearDepthf', true) }}}; break;
	case 'glDeleteProgram': ret = {{{ Functions.getIndex('_glDeleteProgram', true) }}}; break;
	case 'glUniformMatrix3fv': ret = {{{ Functions.getIndex('_glUniformMatrix3fv', true) }}}; break;
	case 'glClearColor': ret = {{{ Functions.getIndex('_glClearColor', true) }}}; break;
	case 'glGetRenderbufferParameteriv': ret = {{{ Functions.getIndex('_glGetRenderbufferParameteriv', true) }}}; break;
	case 'glGetShaderInfoLog': ret = {{{ Functions.getIndex('_glGetShaderInfoLog', true) }}}; break;
	case 'glUniformMatrix4fv': ret = {{{ Functions.getIndex('_glUniformMatrix4fv', true) }}}; break;
	case 'glClearStencil': ret = {{{ Functions.getIndex('_glClearStencil', true) }}}; break;
	case 'glGetProgramInfoLog': ret = {{{ Functions.getIndex('_glGetProgramInfoLog', true) }}}; break;
	case 'glGetUniformfv': ret = {{{ Functions.getIndex('_glGetUniformfv', true) }}}; break;
	case 'glStencilFuncSeparate': ret = {{{ Functions.getIndex('_glStencilFuncSeparate', true) }}}; break;
	case 'glSampleCoverage': ret = {{{ Functions.getIndex('_glSampleCoverage', true) }}}; break;
	case 'glColorMask': ret = {{{ Functions.getIndex('_glColorMask', true) }}}; break;
	case 'glGetShaderiv': ret = {{{ Functions.getIndex('_glGetShaderiv', true) }}}; break;
	case 'glGetUniformiv': ret = {{{ Functions.getIndex('_glGetUniformiv', true) }}}; break;
	case 'glCopyTexSubImage2D': ret = {{{ Functions.getIndex('_glCopyTexSubImage2D', true) }}}; break;
	case 'glDetachShader': ret = {{{ Functions.getIndex('_glDetachShader', true) }}}; break;
	case 'glGetShaderSource': ret = {{{ Functions.getIndex('_glGetShaderSource', true) }}}; break;
	case 'glDeleteTextures': ret = {{{ Functions.getIndex('_glDeleteTextures', true) }}}; break;
	case 'glGetAttachedShaders': ret = {{{ Functions.getIndex('_glGetAttachedShaders', true) }}}; break;
	case 'glValidateProgram': ret = {{{ Functions.getIndex('_glValidateProgram', true) }}}; break;
	case 'glDepthFunc': ret = {{{ Functions.getIndex('_glDepthFunc', true) }}}; break;
	case 'glIsShader': ret = {{{ Functions.getIndex('_glIsShader', true) }}}; break;
	case 'glDepthMask': ret = {{{ Functions.getIndex('_glDepthMask', true) }}}; break;
	case 'glStencilMaskSeparate': ret = {{{ Functions.getIndex('_glStencilMaskSeparate', true) }}}; break;
	case 'glIsProgram': ret = {{{ Functions.getIndex('_glIsProgram', true) }}}; break;
	case 'glDisable': ret = {{{ Functions.getIndex('_glDisable', true) }}}; break;
	case 'glStencilOpSeparate': ret = {{{ Functions.getIndex('_glStencilOpSeparate', true) }}}; break;
	case 'glDrawArrays': ret = {{{ Functions.getIndex('_glDrawArrays', true) }}}; break;
	case 'glDrawElements': ret = {{{ Functions.getIndex('_glDrawElements', true) }}}; break;
	case 'glEnable': ret = {{{ Functions.getIndex('_glEnable', true) }}}; break;
	case 'glFinish': ret = {{{ Functions.getIndex('_glFinish', true) }}}; break;
	case 'glFlush': ret = {{{ Functions.getIndex('_glFlush', true) }}}; break;
	case 'glFrontFace': ret = {{{ Functions.getIndex('_glFrontFace', true) }}}; break;
	case 'glCullFace': ret = {{{ Functions.getIndex('_glCullFace', true) }}}; break;
	case 'glGenTextures': ret = {{{ Functions.getIndex('_glGenTextures', true) }}}; break;
	case 'glGetError': ret = {{{ Functions.getIndex('_glGetError', true) }}}; break;
	case 'glGetIntegerv': ret = {{{ Functions.getIndex('_glGetIntegerv', true) }}}; break;
	case 'glGetBooleanv': ret = {{{ Functions.getIndex('_glGetBooleanv', true) }}}; break;
	case 'glGetFloatv': ret = {{{ Functions.getIndex('_glGetFloatv', true) }}}; break;
	case 'glHint': ret = {{{ Functions.getIndex('_glHint', true) }}}; break;
	case 'glIsTexture': ret = {{{ Functions.getIndex('_glIsTexture', true) }}}; break;
	case 'glPixelStorei': ret = {{{ Functions.getIndex('_glPixelStorei', true) }}}; break;
	case 'glReadPixels': ret = {{{ Functions.getIndex('_glReadPixels', true) }}}; break;
	case 'glScissor': ret = {{{ Functions.getIndex('_glScissor', true) }}}; break;
	case 'glStencilFunc': ret = {{{ Functions.getIndex('_glStencilFunc', true) }}}; break;
	case 'glStencilMask': ret = {{{ Functions.getIndex('_glStencilMask', true) }}}; break;
	case 'glStencilOp': ret = {{{ Functions.getIndex('_glStencilOp', true) }}}; break;
	case 'glTexImage2D': ret = {{{ Functions.getIndex('_glTexImage2D', true) }}}; break;
	case 'glTexParameterf': ret = {{{ Functions.getIndex('_glTexParameterf', true) }}}; break;
	case 'glTexParameterfv': ret = {{{ Functions.getIndex('_glTexParameterfv', true) }}}; break;
	case 'glTexParameteri': ret = {{{ Functions.getIndex('_glTexParameteri', true) }}}; break;
	case 'glTexParameteriv': ret = {{{ Functions.getIndex('_glTexParameteriv', true) }}}; break;
	case 'glGetTexParameterfv': ret = {{{ Functions.getIndex('_glGetTexParameterfv', true) }}}; break;
	case 'glGetTexParameteriv': ret = {{{ Functions.getIndex('_glGetTexParameteriv', true) }}}; break;
	case 'glTexSubImage2D': ret = {{{ Functions.getIndex('_glTexSubImage2D', true) }}}; break;
	case 'glCopyTexImage2D': ret = {{{ Functions.getIndex('_glCopyTexImage2D', true) }}}; break;
	case 'glViewport': ret = {{{ Functions.getIndex('_glViewport', true) }}}; break;
	case 'glIsEnabled': ret = {{{ Functions.getIndex('_glIsEnabled', true) }}}; break;
	case 'glLineWidth': ret = {{{ Functions.getIndex('_glLineWidth', true) }}}; break;
	case 'glPolygonOffset': ret = {{{ Functions.getIndex('_glPolygonOffset', true) }}}; break;
	case 'glReleaseShaderCompiler': ret = {{{ Functions.getIndex('_glReleaseShaderCompiler', true) }}}; break;
	case 'glGetShaderPrecisionFormat': ret = {{{ Functions.getIndex('_glGetShaderPrecisionFormat', true) }}}; break;
	case 'glShaderBinary': ret = {{{ Functions.getIndex('_glShaderBinary', true) }}}; break;
      }
      if (!ret) Module.printErr('WARNING: getProcAddress failed for ' + name);
      return ret;
    }
  },

  glGetShaderPrecisionFormat__sig: 'v',
  glGetShaderPrecisionFormat: function() { throw 'glGetShaderPrecisionFormat: TODO' },

  glShaderBinary__sig: 'v',
  glShaderBinary: function() { throw 'glShaderBinary: TODO' },

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

  glReleaseShaderCompiler__sig: 'v',
  glReleaseShaderCompiler: function() {
    // NOP (as allowed by GLES 2.0 spec)
  },

  glGetObjectParameteriv__sig: 'viii',
  glGetObjectParameteriv: function(id, type, result) {
    if (GL.programs[id]) {
      if (type == 0x8B84) { // GL_OBJECT_INFO_LOG_LENGTH_ARB
        {{{ makeSetValue('result', '0', 'Module.ctx.getProgramInfoLog(GL.programs[id]).length', 'i32') }}};
        return;
      }
      _glGetProgramiv(id, type, result);
    } else if (GL.shaders[id]) {
      if (type == 0x8B84) { // GL_OBJECT_INFO_LOG_LENGTH_ARB
        {{{ makeSetValue('result', '0', 'Module.ctx.getShaderInfoLog(GL.shaders[id]).length', 'i32') }}};
        return;
      } else if (type == 0x8B88) { // GL_OBJECT_SHADER_SOURCE_LENGTH_ARB
        {{{ makeSetValue('result', '0', 'Module.ctx.getShaderSource(GL.shaders[id]).length', 'i32') }}};
        return;
      }
      _glGetShaderiv(id, type, result);
    } else {
      Module.printErr('WARNING: getObjectParameteriv received invalid id: ' + id);
    }
  },

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

  glBindProgram__sig: 'vii',
  glBindProgram: function(type, id) {
    assert(id == 0);
  },

  glGetPointerv: function(name, p) {
    var attribute;
    switch(name) {
      case 0x808E: // GL_VERTEX_ARRAY_POINTER
        attribute = GLImmediate.clientAttributes[GLImmediate.VERTEX]; break;
      case 0x8090: // GL_COLOR_ARRAY_POINTER
        attribute = GLImmediate.clientAttributes[GLImmediate.COLOR]; break;
      case 0x8092: // GL_TEXTURE_COORD_ARRAY_POINTER
        attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0]; break;
      default: throw 'TODO: glGetPointerv for ' + name;
    }
    {{{ makeSetValue('p', '0', 'attribute ? attribute.pointer : 0', 'i32') }}};
  },

  // GL Immediate mode

  $GLImmediate__postset: 'GL.immediate.setupFuncs(); Browser.moduleContextCreatedCallbacks.push(function() { GL.immediate.init() });',
  $GLImmediate__deps: ['$Browser', '$GL', '$GLEmulation'],
  $GLImmediate: {
    MAX_TEXTURES: 7,

    // Vertex and index data
    vertexData: null, // current vertex data. either tempData (glBegin etc.) or a view into the heap (gl*Pointer). Default view is F32
    vertexDataU8: null, // U8 view
    tempData: null,
    indexData: null,
    vertexCounter: 0,
    mode: -1,

    rendererCache: null,
    rendererCacheItemTemplate: [null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null], // 16 nulls
    rendererComponents: [], // small cache for calls inside glBegin/end. counts how many times the element was seen
    rendererComponentPointer: 0, // next place to start a glBegin/end component
    lastRenderer: null, // used to avoid cleaning up and re-preparing the same renderer
    lastArrayBuffer: null, // used in conjunction with lastRenderer
    lastProgram: null, // ""
    lastStride: -1, // ""

    // The following data structures are used for OpenGL Immediate Mode matrix routines.
    matrix: {},
    matrixStack: {},
    currentMatrix: 'm', // default is modelview
    tempMatrix: null,
    matricesModified: false,

    // Clientside attributes
    VERTEX: 0,
    NORMAL: 1,
    COLOR: 2,
    TEXTURE0: 3,
    TEXTURE1: 4,
    TEXTURE2: 5,
    TEXTURE3: 6,
    TEXTURE4: 7,
    TEXTURE5: 8,
    TEXTURE6: 9,
    NUM_ATTRIBUTES: 10,
    NUM_TEXTURES: 7,

    totalEnabledClientAttributes: 0,
    enabledClientAttributes: [0, 0],
    clientAttributes: [], // raw data, including possible unneeded ones
    liveClientAttributes: [], // the ones actually alive in the current computation, sorted
    modifiedClientAttributes: false,
    clientActiveTexture: 0,
    clientColor: null,

    setClientAttribute: function(name, size, type, stride, pointer) {
      var attrib = this.clientAttributes[name];
      if (!attrib) {
        for (var i = 0; i <= name; i++) { // keep flat
          if (!this.clientAttributes[i]) {
            this.clientAttributes[i] = {
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
      this.modifiedClientAttributes = true;
    },

    // Renderers
    addRendererComponent: function(name, size, type) {
      if (!this.rendererComponents[name]) {
        this.rendererComponents[name] = 1;
#if ASSERTIONS
        assert(!this.enabledClientAttributes[name]); // cannot get mixed up with this, for example we will disable this later
#endif
        this.enabledClientAttributes[name] = true;
        this.setClientAttribute(name, size, type, 0, this.rendererComponentPointer);
        this.rendererComponentPointer += size * GL.byteSizeByType[type - GL.byteSizeByTypeRoot];
      } else {
        this.rendererComponents[name]++;
      }
    },

    disableBeginEndClientAttributes: function() {
      for (var i = 0; i < this.NUM_ATTRIBUTES; i++) {
        if (this.rendererComponents[i]) this.enabledClientAttributes[i] = false;
      }
    },

    getRenderer: function() {
      // return a renderer object given the liveClientAttributes
      // we maintain a cache of renderers, optimized to not generate garbage
      var attributes = GL.immediate.liveClientAttributes;
      var cacheItem = GL.immediate.rendererCache;
      var temp;
      for (var i = 0; i < attributes.length; i++) {
        var attribute = attributes[i];
        temp = cacheItem[attribute.name];
        cacheItem = temp ? temp : (cacheItem[attribute.name] = GL.immediate.rendererCacheItemTemplate.slice());
        temp = cacheItem[attribute.size];
        cacheItem = temp ? temp : (cacheItem[attribute.size] = GL.immediate.rendererCacheItemTemplate.slice());
        var typeIndex = attribute.type - GL.byteSizeByTypeRoot; // ensure it starts at 0 to keep the cache items dense
        temp = cacheItem[typeIndex];
        cacheItem = temp ? temp : (cacheItem[typeIndex] = GL.immediate.rendererCacheItemTemplate.slice());
      }
      var fogParam;
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
      } else {
        fogParam = 0;
      }
      temp = cacheItem[fogParam];
      cacheItem = temp ? temp : (cacheItem[fogParam] = GL.immediate.rendererCacheItemTemplate.slice());
      if (GL.currProgram) { // Note the order here; this one is last, and optional. Note that we cannot ensure it is dense, sadly
        temp = cacheItem[GL.currProgram];
        cacheItem = temp ? temp : (cacheItem[GL.currProgram] = GL.immediate.rendererCacheItemTemplate.slice());
      }
      if (!cacheItem.renderer) {
#if GL_DEBUG
        Module.printErr('generating renderer for ' + JSON.stringify(attributes));
#endif
        cacheItem.renderer = this.createRenderer();
      }
      return cacheItem.renderer;
    },

    createRenderer: function(renderer) {
      var useCurrProgram = !!GL.currProgram;
      var hasTextures = false, textureSizes = [], textureTypes = [];
      for (var i = 0; i < GL.immediate.NUM_TEXTURES; i++) {
        if (GL.immediate.enabledClientAttributes[GL.immediate.TEXTURE0 + i]) {
          textureSizes[i] = GL.immediate.clientAttributes[GL.immediate.TEXTURE0 + i].size;
          textureTypes[i] = GL.immediate.clientAttributes[GL.immediate.TEXTURE0 + i].type;
          hasTextures = true;
        }
      }
      var positionSize = GL.immediate.clientAttributes[GL.immediate.VERTEX].size;
      var positionType = GL.immediate.clientAttributes[GL.immediate.VERTEX].type;
      var colorSize = 0, colorType;
      if (GL.immediate.enabledClientAttributes[GL.immediate.COLOR]) {
        colorSize = GL.immediate.clientAttributes[GL.immediate.COLOR].size;
        colorType = GL.immediate.clientAttributes[GL.immediate.COLOR].type;
      }
      var normalSize = 0, normalType;
      if (GL.immediate.enabledClientAttributes[GL.immediate.NORMAL]) {
        normalSize = GL.immediate.clientAttributes[GL.immediate.NORMAL].size;
        normalType = GL.immediate.clientAttributes[GL.immediate.NORMAL].type;
      }
      var ret = {
        init: function() {
          if (useCurrProgram) {
            if (GL.shaderInfos[GL.programShaders[GL.currProgram][0]].type == Module.ctx.VERTEX_SHADER) {
              this.vertexShader = GL.shaders[GL.programShaders[GL.currProgram][0]];
              this.fragmentShader = GL.shaders[GL.programShaders[GL.currProgram][1]];
            } else {
              this.vertexShader = GL.shaders[GL.programShaders[GL.currProgram][1]];
              this.fragmentShader = GL.shaders[GL.programShaders[GL.currProgram][0]];
            }
            this.program = GL.programs[GL.currProgram];
          } else {
            // IMPORTANT NOTE: If you parameterize the shader source based on any runtime values
            // in order to create the least expensive shader possible based on the features being
            // used, you should also update the code in the beginning of getRenderer to make sure
            // that you cache the renderer based on the said parameters.
            this.vertexShader = Module.ctx.createShader(Module.ctx.VERTEX_SHADER);
            var zero = positionSize == 2 ? '0, ' : '';
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
            Module.ctx.shaderSource(this.vertexShader, 'attribute vec' + positionSize + ' a_position;  \n' +
                                                       'attribute vec2 a_texCoord0;  \n' +
                                                       (hasTextures ? 'varying vec2 v_texCoord;    \n' : '') +
                                                       'varying vec4 v_color; \n' +
                                                       (colorSize ? 'attribute vec4 a_color; \n': 'uniform vec4 u_color; \n') +
                                                       (GLEmulation.fogEnabled ? 'varying float v_fogFragCoord; \n' : '') +
                                                       'uniform mat4 u_modelView;   \n' +
                                                       'uniform mat4 u_projection;  \n' +
                                                       'void main()                 \n' +
                                                       '{                           \n' +
                                                       '  vec4 ecPosition = (u_modelView * vec4(a_position, ' + zero + '1.0)); \n' + // eye-coordinate position
                                                       '  gl_Position = u_projection * ecPosition; \n' +
                                                       (hasTextures ? 'v_texCoord = a_texCoord0;    \n' : '') +
                                                       (colorSize ? 'v_color = a_color; \n' : 'v_color = u_color; \n') +
                                                       (GLEmulation.fogEnabled ? 'v_fogFragCoord = abs(ecPosition.z);\n' : '') +
                                                       '}                           \n');
            Module.ctx.compileShader(this.vertexShader);

            this.fragmentShader = Module.ctx.createShader(Module.ctx.FRAGMENT_SHADER);
            Module.ctx.shaderSource(this.fragmentShader, 'precision mediump float;                            \n' +
                                                         'varying vec2 v_texCoord;                            \n' +
                                                         'uniform sampler2D u_texture;                        \n' +
                                                         'varying vec4 v_color;                               \n' +
                                                         (GLEmulation.fogEnabled ? (
                                                           'varying float v_fogFragCoord; \n' +
                                                           'uniform vec4 u_fogColor; \n' +
                                                           'uniform float u_fogEnd; \n' +
                                                           'uniform float u_fogScale; \n' +
                                                           'uniform float u_fogDensity; \n' +
                                                           'float ffog(in float ecDistance) { \n' +
                                                           fogFormula +
                                                           '  fog = clamp(fog, 0.0, 1.0); \n' +
                                                           '  return fog; \n' +
                                                           '} \n'
                                                           ) : '') +
                                                         'void main()                                         \n' +
                                                         '{                                                   \n' +
                                                         (hasTextures ? 'gl_FragColor = v_color * texture2D( u_texture, v_texCoord );\n' :
                                                                        'gl_FragColor = v_color;\n') +
                                                         (GLEmulation.fogEnabled ? 'gl_FragColor = vec4(mix(u_fogColor.rgb, gl_FragColor.rgb, ffog(v_fogFragCoord)), gl_FragColor.a); \n' : '') +
                                                         '}                                                   \n');
            Module.ctx.compileShader(this.fragmentShader);

            this.program = Module.ctx.createProgram();
            Module.ctx.attachShader(this.program, this.vertexShader);
            Module.ctx.attachShader(this.program, this.fragmentShader);
            Module.ctx.bindAttribLocation(this.program, 0, 'a_position');
            Module.ctx.linkProgram(this.program);
          }

          this.positionLocation = Module.ctx.getAttribLocation(this.program, 'a_position');
          this.texCoordLocations = [];
          for (var i = 0; i < textureSizes.length; i++) {
            if (textureSizes[i]) {
              this.texCoordLocations[i] = Module.ctx.getAttribLocation(this.program, 'a_texCoord' + i);
            }
          }
          this.textureMatrixLocations = [];
          for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
            this.textureMatrixLocations[i] = Module.ctx.getUniformLocation(this.program, 'u_textureMatrix' + i);
          }
          this.colorLocation = Module.ctx.getAttribLocation(this.program, 'a_color');
          this.normalLocation = Module.ctx.getAttribLocation(this.program, 'a_normal');

          this.textureLocation = Module.ctx.getUniformLocation(this.program, 'u_texture'); // only for immediate mode with no shaders, so only one is enough
          this.modelViewLocation = Module.ctx.getUniformLocation(this.program, 'u_modelView');
          this.projectionLocation = Module.ctx.getUniformLocation(this.program, 'u_projection');
          this.hasColorAttribLocation = Module.ctx.getUniformLocation(this.program, 'u_hasColorAttrib');
          this.colorUniformLocation = Module.ctx.getUniformLocation(this.program, 'u_color');

          this.hasTextures = hasTextures;
          this.hasColorAttrib = colorSize > 0 && this.colorLocation >= 0;
          this.hasColorUniform = !!this.colorUniformLocation;
          this.hasNormal = normalSize > 0 && this.normalLocation >= 0;

          this.floatType = Module.ctx.FLOAT; // minor optimization

          this.fogColorLocation = Module.ctx.getUniformLocation(this.program, 'u_fogColor');
          this.fogEndLocation = Module.ctx.getUniformLocation(this.program, 'u_fogEnd');
          this.fogScaleLocation = Module.ctx.getUniformLocation(this.program, 'u_fogScale');
          this.fogDensityLocation = Module.ctx.getUniformLocation(this.program, 'u_fogDensity');
          this.hasFog = !!(this.fogColorLocation || this.fogEndLocation ||
                           this.fogScaleLocation || this.fogDensityLocation);
        },

        prepare: function() {
          // Calculate the array buffer
          var arrayBuffer;
          if (!GL.currArrayBuffer) {
            var start = GL.immediate.firstVertex*GL.immediate.stride;
            var end = GL.immediate.lastVertex*GL.immediate.stride;
            assert(end <= GL.MAX_TEMP_BUFFER_SIZE, 'too much vertex data');
            arrayBuffer = GL.tempVertexBuffers[GL.tempBufferIndexLookup[end]];
            // TODO: consider using the last buffer we bound, if it was larger. downside is larger buffer, but we might avoid rebinding and preparing
          } else {
            arrayBuffer = GL.currArrayBuffer;
          }

          // If the array buffer is unchanged and the renderer as well, then we can avoid all the work here
          // XXX We use some heuristics here, and this may not work in all cases. Try disabling GL_UNSAFE_OPTS if you
          // have odd glitches
#if GL_UNSAFE_OPTS
          var lastRenderer = GL.immediate.lastRenderer;
          var canSkip = this == lastRenderer &&
                        arrayBuffer == GL.immediate.lastArrayBuffer &&
                        (GL.currProgram || this.program) == GL.immediate.lastProgram &&
                        GL.immediate.stride == GL.immediate.lastStride &&
                        !GL.immediate.matricesModified;
          if (!canSkip && lastRenderer) lastRenderer.cleanup();
#endif
          if (!GL.currArrayBuffer) {
            // Bind the array buffer and upload data after cleaning up the previous renderer
#if GL_UNSAFE_OPTS
            // Potentially unsafe, since lastArrayBuffer might not reflect the true array buffer in code that mixes immediate/non-immediate
            if (arrayBuffer != GL.immediate.lastArrayBuffer) {
#endif
              Module.ctx.bindBuffer(Module.ctx.ARRAY_BUFFER, arrayBuffer);
#if GL_UNSAFE_OPTS
            }
#endif
            Module.ctx.bufferSubData(Module.ctx.ARRAY_BUFFER, start, GL.immediate.vertexData.subarray(start >> 2, end >> 2));
          }
#if GL_UNSAFE_OPTS
          if (canSkip) return;
          GL.immediate.lastRenderer = this;
          GL.immediate.lastArrayBuffer = arrayBuffer;
          GL.immediate.lastProgram = GL.currProgram || this.program;
          GL.immediate.lastStride == GL.immediate.stride;
          GL.immediate.matricesModified = false;
#endif

          if (!GL.currProgram) {
            Module.ctx.useProgram(this.program);
          }

          if (this.modelViewLocation) Module.ctx.uniformMatrix4fv(this.modelViewLocation, false, GL.immediate.matrix['m']);
          if (this.projectionLocation) Module.ctx.uniformMatrix4fv(this.projectionLocation, false, GL.immediate.matrix['p']);

          var clientAttributes = GL.immediate.clientAttributes;

          Module.ctx.vertexAttribPointer(this.positionLocation, positionSize, positionType, false,
                                         GL.immediate.stride, clientAttributes[GL.immediate.VERTEX].offset);
          Module.ctx.enableVertexAttribArray(this.positionLocation);
          if (this.hasTextures) {
            for (var i = 0; i < textureSizes.length; i++) {
              if (textureSizes[i] && this.texCoordLocations[i] >= 0) {
                Module.ctx.vertexAttribPointer(this.texCoordLocations[i], textureSizes[i], textureTypes[i], false,
                                               GL.immediate.stride, clientAttributes[GL.immediate.TEXTURE0 + i].offset);
                Module.ctx.enableVertexAttribArray(this.texCoordLocations[i]);
              }
            }
            for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
              if (this.textureMatrixLocations[i]) { // XXX might we need this even without the condition we are currently in?
                Module.ctx.uniformMatrix4fv(this.textureMatrixLocations[i], false, GL.immediate.matrix['t' + i]);
              }
            }
          }
          if (this.hasColorAttrib) {
            Module.ctx.vertexAttribPointer(this.colorLocation, colorSize, colorType, true,
                                           GL.immediate.stride, clientAttributes[GL.immediate.COLOR].offset);
            Module.ctx.enableVertexAttribArray(this.colorLocation);
            Module.ctx.uniform1i(this.hasColorAttribLocation, 1);
          } else if (this.hasColorUniform) {
            Module.ctx.uniform1i(this.hasColorAttribLocation, 0);
            Module.ctx.uniform4fv(this.colorUniformLocation, GL.immediate.clientColor);
          }
          if (this.hasNormal) {
            Module.ctx.vertexAttribPointer(this.normalLocation, normalSize, normalType, true,
                                           GL.immediate.stride, clientAttributes[GL.immediate.NORMAL].offset);
            Module.ctx.enableVertexAttribArray(this.normalLocation);
          }
          if (!useCurrProgram) { // otherwise, the user program will set the sampler2D binding and uniform itself
            var texture = Module.ctx.getParameter(Module.ctx.TEXTURE_BINDING_2D);
            Module.ctx.activeTexture(Module.ctx.TEXTURE0);
            Module.ctx.bindTexture(Module.ctx.TEXTURE_2D, texture);
            Module.ctx.uniform1i(this.textureLocation, 0);
          }
          if (this.hasFog) {
            if (this.fogColorLocation) Module.ctx.uniform4fv(this.fogColorLocation, GLEmulation.fogColor);
            if (this.fogEndLocation) Module.ctx.uniform1f(this.fogEndLocation, GLEmulation.fogEnd);
            if (this.fogScaleLocation) Module.ctx.uniform1f(this.fogScaleLocation, 1/(GLEmulation.fogEnd - GLEmulation.fogStart));
            if (this.fogDensityLocation) Module.ctx.uniform1f(this.fogDensityLocation, GLEmulation.fogDensity);
          }
        },

        cleanup: function() {
          Module.ctx.disableVertexAttribArray(this.positionLocation);
          if (this.hasTextures) {
            for (var i = 0; i < textureSizes.length; i++) {
              if (textureSizes[i] && this.texCoordLocations[i] >= 0) {
                Module.ctx.disableVertexAttribArray(this.texCoordLocations[i]);
              }
            }
          }
          if (this.hasColorAttrib) {
            Module.ctx.disableVertexAttribArray(this.colorLocation);
          }
          if (this.hasNormal) {
            Module.ctx.disableVertexAttribArray(this.normalLocation);
          }
          if (!GL.currProgram) {
            Module.ctx.useProgram(null);
          }
          if (!GL.currArrayBuffer) {
            Module.ctx.bindBuffer(Module.ctx.ARRAY_BUFFER, null);
          }

#if GL_UNSAFE_OPTS
          GL.immediate.lastRenderer = null;
          GL.immediate.lastArrayBuffer = null;
          GL.immediate.lastProgram = null;
#endif
          GL.immediate.matricesModified = true;
        }
      };
      ret.init();
      return ret;
    },

    setupFuncs: function() {
      // Replace some functions with immediate-mode aware versions. If there are no client
      // attributes enabled, and we use webgl-friendly modes (no GL_QUADS), then no need
      // for emulation
      _glDrawArrays = function(mode, first, count) {
        if (GL.immediate.totalEnabledClientAttributes == 0 && mode <= 6) {
          Module.ctx.drawArrays(mode, first, count);
          return;
        }
        GL.immediate.prepareClientAttributes(count, false);
        GL.immediate.mode = mode;
        if (!GL.currArrayBuffer) {
          GL.immediate.vertexData = {{{ makeHEAPView('F32', 'GL.immediate.vertexPointer', 'GL.immediate.vertexPointer + (first+count)*GL.immediate.stride') }}}; // XXX assuming float
          GL.immediate.firstVertex = first;
          GL.immediate.lastVertex = first + count;
        }
        GL.immediate.flush(null, first);
        GL.immediate.mode = -1;
      };

      _glDrawElements = function(mode, count, type, indices, start, end) { // start, end are given if we come from glDrawRangeElements
        if (GL.immediate.totalEnabledClientAttributes == 0 && mode <= 6 && GL.currElementArrayBuffer) {
          Module.ctx.drawElements(mode, count, type, indices);
          return;
        }
        if (!GL.currElementArrayBuffer) {
          assert(type == Module.ctx.UNSIGNED_SHORT); // We can only emulate buffers of this kind, for now
        }
        GL.immediate.prepareClientAttributes(count, false);
        GL.immediate.mode = mode;
        if (!GL.currArrayBuffer) {
          GL.immediate.firstVertex = end ? start : TOTAL_MEMORY; // if we don't know the start, set an invalid value and we will calculate it later from the indices
          GL.immediate.lastVertex = end ? end+1 : 0;
          GL.immediate.vertexData = {{{ makeHEAPView('F32', 'GL.immediate.vertexPointer', '(end ? GL.immediate.vertexPointer + (end+1)*GL.immediate.stride : TOTAL_MEMORY)') }}}; // XXX assuming float
        }
        GL.immediate.flush(count, 0, indices);
        GL.immediate.mode = -1;
      };
    },

    // Main functions
    initted: false,
    init: function() {
      Module.printErr('WARNING: using emscripten GL immediate mode emulation. This is very limited in what it supports');
      GL.immediate.initted = true;

      if (!Module.useWebGL) return; // a 2D canvas may be currently used TODO: make sure we are actually called in that case

      this.matrixStack['m'] = [];
      this.matrixStack['p'] = [];
      for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
        this.matrixStack['t' + i] = [];
      }

      // Initialize matrix library

      GL.immediate.matrix['m'] = GL.immediate.matrix.lib.mat4.create();
      GL.immediate.matrix.lib.mat4.identity(GL.immediate.matrix['m']);
      GL.immediate.matrix['p'] = GL.immediate.matrix.lib.mat4.create();
      GL.immediate.matrix.lib.mat4.identity(GL.immediate.matrix['p']);
      for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
        GL.immediate.matrix['t' + i] = GL.immediate.matrix.lib.mat4.create();
      }

      // Renderer cache
      this.rendererCache = this.rendererCacheItemTemplate.slice();

      // Buffers for data
      this.tempData = new Float32Array(GL.MAX_TEMP_BUFFER_SIZE >> 2);
      this.indexData = new Uint16Array(GL.MAX_TEMP_BUFFER_SIZE >> 1);

      this.vertexDataU8 = new Uint8Array(this.tempData.buffer);

      GL.generateTempBuffers(true);

      this.clientColor = new Float32Array([1, 1, 1, 1]);
    },

    // Prepares and analyzes client attributes.
    // Modifies liveClientAttributes, stride, vertexPointer, vertexCounter
    //   count: number of elements we will draw
    //   beginEnd: whether we are drawing the results of a begin/end block
    prepareClientAttributes: function(count, beginEnd) {
      // If no client attributes were modified since we were last called, do nothing. Note that this
      // does not work for glBegin/End, where we generate renderer components dynamically and then
      // disable them ourselves, but it does help with glDrawElements/Arrays.
      if (!this.modifiedClientAttributes) {
        return;
      }
      this.modifiedClientAttributes = false;

      var stride = 0, start;
      var attributes = GL.immediate.liveClientAttributes;
      attributes.length = 0;
      for (var i = 0; i < GL.immediate.NUM_ATTRIBUTES; i++) {
        if (GL.immediate.enabledClientAttributes[i]) attributes.push(GL.immediate.clientAttributes[i]);
      }
      attributes.sort(function(x, y) { return !x ? (!y ? 0 : 1) : (!y ? -1 : (x.pointer - y.pointer)) });
      start = GL.currArrayBuffer ? 0 : attributes[0].pointer;
      var multiStrides = false;
      for (var i = 0; i < attributes.length; i++) {
        var attribute = attributes[i];
        if (!attribute) break;
        if (stride != 0 && stride != attribute.stride) multiStrides = true;
        if (attribute.stride) stride = attribute.stride;
      }
      if (multiStrides) stride = 0; // we will need to restride
      var bytes = 0; // total size in bytes
      if (!stride && !beginEnd) {
        // beginEnd can not have stride in the attributes, that is fine. otherwise,
        // no stride means that all attributes are in fact packed. to keep the rest of
        // our emulation code simple, we perform unpacking/restriding here. this adds overhead, so
        // it is a good idea to not hit this!
#if ASSERTIONS
        Runtime.warnOnce('Unpacking/restriding attributes, this is slow and dangerous');
#endif
        if (!GL.immediate.restrideBuffer) GL.immediate.restrideBuffer = _malloc(GL.MAX_TEMP_BUFFER_SIZE);
        start = GL.immediate.restrideBuffer;
#if ASSERTIONS
        assert(start % 4 == 0);
#endif
        // calculate restrided offsets and total size
        for (var i = 0; i < attributes.length; i++) {
          var attribute = attributes[i];
          if (!attribute) break;
          var size = attribute.size * GL.byteSizeByType[attribute.type - GL.byteSizeByTypeRoot];
          if (size % 4 != 0) size += 4 - (size % 4); // align everything
          attribute.offset = bytes;
          bytes += size;
        }
#if ASSERTIONS
        assert(count*bytes <= GL.MAX_TEMP_BUFFER_SIZE);
#endif
        // copy out the data (we need to know the stride for that, and define attribute.pointer
        for (var i = 0; i < attributes.length; i++) {
          var attribute = attributes[i];
          if (!attribute) break;
          var size4 = Math.floor((attribute.size * GL.byteSizeByType[attribute.type - GL.byteSizeByTypeRoot])/4);
          for (var j = 0; j < count; j++) {
            for (var k = 0; k < size4; k++) { // copy in chunks of 4 bytes, our alignment makes this possible
              HEAP32[((start + attribute.offset + bytes*j)>>2) + k] = HEAP32[(attribute.pointer>>2) + j*size4 + k];
            }
          }
          attribute.pointer = start + attribute.offset;
        }
      } else {
        // normal situation, everything is strided and in the same buffer
        for (var i = 0; i < attributes.length; i++) {
          var attribute = attributes[i];
          if (!attribute) break;
          attribute.offset = attribute.pointer - start;
          if (attribute.offset > bytes) { // ensure we start where we should
            assert((attribute.offset - bytes)%4 == 0); // XXX assuming 4-alignment
            bytes += attribute.offset - bytes;
          }
          bytes += attribute.size * GL.byteSizeByType[attribute.type - GL.byteSizeByTypeRoot];
          if (bytes % 4 != 0) bytes += 4 - (bytes % 4); // XXX assuming 4-alignment
        }
        assert(beginEnd || bytes <= stride); // if not begin-end, explicit stride should make sense with total byte size
        if (bytes < stride) { // ensure the size is that of the stride
          bytes = stride;
        }
      }
      GL.immediate.stride = bytes;

      if (!beginEnd) {
        bytes *= count;
        if (!GL.currArrayBuffer) {
          GL.immediate.vertexPointer = start;
        }
        GL.immediate.vertexCounter = bytes / 4; // XXX assuming float
      }
    },

    flush: function(numProvidedIndexes, startIndex, ptr) {
#if ASSERTIONS
      assert(numProvidedIndexes >= 0 || !numProvidedIndexes);
#endif
      startIndex = startIndex || 0;
      ptr = ptr || 0;

      var renderer = this.getRenderer();

      // Generate index data in a format suitable for GLES 2.0/WebGL
      var numVertexes = 4 * this.vertexCounter / GL.immediate.stride; // XXX assuming float
      assert(numVertexes % 1 == 0);

      var emulatedElementArrayBuffer = false;
      var numIndexes = 0;
      if (numProvidedIndexes) {
        numIndexes = numProvidedIndexes;
        if (!GL.currArrayBuffer && GL.immediate.firstVertex > GL.immediate.lastVertex) {
          // Figure out the first and last vertex from the index data
          assert(!GL.currElementArrayBuffer); // If we are going to upload array buffer data, we need to find which range to
                                              // upload based on the indices. If they are in a buffer on the GPU, that is very
                                              // inconvenient! So if you do not have an array buffer, you should also not have
                                              // an element array buffer. But best is to use both buffers!
          for (var i = 0; i < numProvidedIndexes; i++) {
            var currIndex = {{{ makeGetValue('ptr', 'i*2', 'i16', null, 1) }}};
            GL.immediate.firstVertex = Math.min(GL.immediate.firstVertex, currIndex);
            GL.immediate.lastVertex = Math.max(GL.immediate.lastVertex, currIndex+1);
          }
        }
        if (!GL.currElementArrayBuffer) {
          // If no element array buffer is bound, then indices is a literal pointer to clientside data
          assert(numProvidedIndexes << 1 <= GL.MAX_TEMP_BUFFER_SIZE, 'too many immediate mode indexes (a)');
          var indexBuffer = GL.tempIndexBuffers[GL.tempBufferIndexLookup[numProvidedIndexes << 1]];
          Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, indexBuffer);
          Module.ctx.bufferSubData(Module.ctx.ELEMENT_ARRAY_BUFFER, 0, {{{ makeHEAPView('U16', 'ptr', 'ptr + (numProvidedIndexes << 1)') }}});
          ptr = 0;
          emulatedElementArrayBuffer = true;
        }
      } else if (GL.immediate.mode > 6) { // above GL_TRIANGLE_FAN are the non-GL ES modes
        if (GL.immediate.mode != 7) throw 'unsupported immediate mode ' + GL.immediate.mode; // GL_QUADS
        // GL.immediate.firstVertex is the first vertex we want. Quad indexes are in the pattern
        // 0 1 2, 0 2 3, 4 5 6, 4 6 7, so we need to look at index firstVertex * 1.5 to see it.
        // Then since indexes are 2 bytes each, that means 3
        assert(GL.immediate.firstVertex % 4 == 0);
        ptr = GL.immediate.firstVertex*3;
        var numQuads = numVertexes / 4;
        numIndexes = numQuads * 6; // 0 1 2, 0 2 3 pattern
        assert(ptr + (numIndexes << 1) <= GL.MAX_TEMP_BUFFER_SIZE, 'too many immediate mode indexes (b)');
        Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, GL.tempQuadIndexBuffer);
        emulatedElementArrayBuffer = true;
      }

      renderer.prepare();

      if (numIndexes) {
        Module.ctx.drawElements(Module.ctx.TRIANGLES, numIndexes, Module.ctx.UNSIGNED_SHORT, ptr);
      } else {
        Module.ctx.drawArrays(GL.immediate.mode, startIndex, numVertexes);
      }

      if (emulatedElementArrayBuffer) {
        Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, GL.buffers[GL.currElementArrayBuffer] || null);
      }

#if GL_UNSAFE_OPTS == 0
      renderer.cleanup();
#endif
    }
  },

  $GLImmediateSetup__deps: ['$GLImmediate', function() { return 'GL.immediate = GLImmediate; GL.immediate.matrix.lib = ' + read('gl-matrix.js') + ';\n' }],
  $GLImmediateSetup: {},

  glBegin__deps: ['$GLImmediateSetup'],
  glBegin: function(mode) {
    GL.immediate.mode = mode;
    GL.immediate.vertexCounter = 0;
    var components = GL.immediate.rendererComponents = [];
    for (var i = 0; i < GL.immediate.NUM_ATTRIBUTES; i++) {
      components[i] = 0;
    }
    GL.immediate.rendererComponentPointer = 0;
    GL.immediate.vertexData = GL.immediate.tempData;
  },

  glEnd: function() {
    GL.immediate.prepareClientAttributes(GL.immediate.rendererComponents[GL.immediate.VERTEX], true);
    GL.immediate.firstVertex = 0;
    GL.immediate.lastVertex = GL.immediate.vertexCounter / (GL.immediate.stride >> 2);
    GL.immediate.flush();
    GL.immediate.disableBeginEndClientAttributes();
    GL.immediate.mode = -1;
  },

  glVertex3f: function(x, y, z) {
#if ASSERTIONS
    assert(GL.immediate.mode >= 0); // must be in begin/end
#endif
    GL.immediate.vertexData[GL.immediate.vertexCounter++] = x;
    GL.immediate.vertexData[GL.immediate.vertexCounter++] = y;
    GL.immediate.vertexData[GL.immediate.vertexCounter++] = z || 0;
#if ASSERTIONS
    assert(GL.immediate.vertexCounter << 2 < GL.MAX_TEMP_BUFFER_SIZE);
#endif
    GL.immediate.addRendererComponent(GL.immediate.VERTEX, 3, Module.ctx.FLOAT);
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
    assert(GL.immediate.mode >= 0); // must be in begin/end
#endif
    GL.immediate.vertexData[GL.immediate.vertexCounter++] = u;
    GL.immediate.vertexData[GL.immediate.vertexCounter++] = v;
    GL.immediate.addRendererComponent(GL.immediate.TEXTURE0, 2, Module.ctx.FLOAT);
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
    if (GL.immediate.mode >= 0) {
      var start = GL.immediate.vertexCounter << 2;
      GL.immediate.vertexDataU8[start + 0] = r * 255;
      GL.immediate.vertexDataU8[start + 1] = g * 255;
      GL.immediate.vertexDataU8[start + 2] = b * 255;
      GL.immediate.vertexDataU8[start + 3] = a * 255;
      GL.immediate.vertexCounter++;
      GL.immediate.addRendererComponent(GL.immediate.COLOR, 4, Module.ctx.UNSIGNED_BYTE);
    } else {
      GL.immediate.clientColor[0] = r;
      GL.immediate.clientColor[1] = g;
      GL.immediate.clientColor[2] = b;
      GL.immediate.clientColor[3] = a;
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

  glColor4ubv: function() { throw 'glColor4ubv not implemented' },

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
            GLEmulation.fogMode = param; break;
          default: // default to GL_EXP
            GLEmulation.fogMode = 0x0800 /* GL_EXP */; break;
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

  glDrawRangeElements__sig: 'viiiiii',
  glDrawRangeElements: function(mode, start, end, count, type, indices) {
    _glDrawElements(mode, count, type, indices, start, end);
  },

  // ClientState/gl*Pointer

  glEnableClientState: function(cap, disable) {
    var attrib = GLEmulation.getAttributeFromCapability(cap);
    if (attrib === null) {
#if ASSERTIONS
      Module.printErr('WARNING: unhandled clientstate: ' + cap);
#endif
      return;
    }
    if (disable && GL.immediate.enabledClientAttributes[attrib]) {
      GL.immediate.enabledClientAttributes[attrib] = false;
      GL.immediate.totalEnabledClientAttributes--;
      if (GLEmulation.currentVao) delete GLEmulation.currentVao.enabledClientStates[cap];
    } else if (!disable && !GL.immediate.enabledClientAttributes[attrib]) {
      GL.immediate.enabledClientAttributes[attrib] = true;
      GL.immediate.totalEnabledClientAttributes++;
      if (GLEmulation.currentVao) GLEmulation.currentVao.enabledClientStates[cap] = 1;
    }
    GL.immediate.modifiedClientAttributes = true;
  },
  glDisableClientState: function(cap) {
    _glEnableClientState(cap, 1);
  },

  glVertexPointer__deps: ['$GLEmulation'], // if any pointers are used, glVertexPointer must be, and if it is, then we need emulation
  glVertexPointer: function(size, type, stride, pointer) {
    GL.immediate.setClientAttribute(GL.immediate.VERTEX, size, type, stride, pointer);
  },
  glTexCoordPointer: function(size, type, stride, pointer) {
    GL.immediate.setClientAttribute(GL.immediate.TEXTURE0 + GL.immediate.clientActiveTexture, size, type, stride, pointer);
  },
  glNormalPointer: function(type, stride, pointer) {
    GL.immediate.setClientAttribute(GL.immediate.NORMAL, 3, type, stride, pointer);
  },
  glColorPointer: function(size, type, stride, pointer) {
    GL.immediate.setClientAttribute(GL.immediate.COLOR, size, type, stride, pointer);
  },

  glClientActiveTexture__sig: 'vi',
  glClientActiveTexture: function(texture) {
    GL.immediate.clientActiveTexture = texture - 0x84C0; // GL_TEXTURE0
  },

  // Vertex array object (VAO) support. TODO: when the WebGL extension is popular, use that and remove this code and GL.vaos
  glGenVertexArrays__deps: ['$GLEMulation'],
  glGenVertexArrays__sig: 'vii',
  glGenVertexArrays: function(n, vaos) {
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
  glDeleteVertexArrays__sig: 'vii',
  glDeleteVertexArrays: function(n, vaos) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('vaos', 'i*4', 'i32') }}};
      GLEmulation.vaos[id] = null;
      if (GLEmulation.currentVao && GLEmulation.currentVao.id == id) GLEmulation.currentVao = null;
    }
  },
  glBindVertexArray__sig: 'vi',
  glBindVertexArray: function(vao) {
    // undo vao-related things, wipe the slate clean, both for vao of 0 or an actual vao
    GLEmulation.currentVao = null; // make sure the commands we run here are not recorded
    if (GL.immediate.lastRenderer) GL.immediate.lastRenderer.cleanup();
    _glBindBuffer(Module.ctx.ARRAY_BUFFER, 0); // XXX if one was there before we were bound?
    _glBindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, 0);
    for (var vaa in GLEmulation.enabledVertexAttribArrays) {
      Module.ctx.disableVertexAttribArray(vaa);
    }
    GLEmulation.enabledVertexAttribArrays = {};
    GL.immediate.enabledClientAttributes = [0, 0];
    GL.immediate.totalEnabledClientAttributes = 0;
    GL.immediate.modifiedClientAttributes = true;
    if (vao) {
      // replay vao
      var info = GLEmulation.vaos[vao];
      _glBindBuffer(Module.ctx.ARRAY_BUFFER, info.arrayBuffer); // XXX overwrite current binding?
      _glBindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, info.elementArrayBuffer);
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
      GL.immediate.currentMatrix = 'm';
    } else if (mode == 0x1701 /* GL_PROJECTION */) {
      GL.immediate.currentMatrix = 'p';
    } else if (mode == 0x1702) { // GL_TEXTURE
      GL.immediate.currentMatrix = 't' + GL.immediate.clientActiveTexture;
    } else {
      throw "Wrong mode " + mode + " passed to glMatrixMode";
    }
  },

  glPushMatrix: function() {
    GL.immediate.matricesModified = true;
    GL.immediate.matrixStack[GL.immediate.currentMatrix].push(
        Array.prototype.slice.call(GL.immediate.matrix[GL.immediate.currentMatrix]));
  },

  glPopMatrix: function() {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix[GL.immediate.currentMatrix] = GL.immediate.matrixStack[GL.immediate.currentMatrix].pop();
  },

  glLoadIdentity__deps: ['$GL', '$GLImmediateSetup'],
  glLoadIdentity: function() {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.identity(GL.immediate.matrix[GL.immediate.currentMatrix]);
  },

  glLoadMatrixd: function(matrix) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}}, GL.immediate.matrix[GL.immediate.currentMatrix]);
  },

  glLoadMatrixf: function(matrix) {
#if GL_DEBUG
    if (GL.debug) Module.printErr('glLoadMatrixf receiving: ' + Array.prototype.slice.call(HEAPF32.subarray(matrix >> 2, (matrix >> 2) + 16)));
#endif
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}}, GL.immediate.matrix[GL.immediate.currentMatrix]);
  },

  glLoadTransposeMatrixd: function(matrix) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}}, GL.immediate.matrix[GL.immediate.currentMatrix]);
    GL.immediate.matrix.lib.mat4.transpose(GL.immediate.matrix[GL.immediate.currentMatrix]);
  },

  glLoadTransposeMatrixf: function(matrix) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}}, GL.immediate.matrix[GL.immediate.currentMatrix]);
    GL.immediate.matrix.lib.mat4.transpose(GL.immediate.matrix[GL.immediate.currentMatrix]);
  },

  glMultMatrixd: function(matrix) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix],
        {{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}});
  },

  glMultMatrixf: function(matrix) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix],
        {{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}});
  },

  glMultTransposeMatrixd: function(matrix) {
    GL.immediate.matricesModified = true;
    var colMajor = GL.immediate.matrix.lib.mat4.create();
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}}, colMajor);
    GL.immediate.matrix.lib.mat4.transpose(colMajor);
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix], colMajor);
  },

  glMultTransposeMatrixf: function(matrix) {
    GL.immediate.matricesModified = true;
    var colMajor = GL.immediate.matrix.lib.mat4.create();
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}}, colMajor);
    GL.immediate.matrix.lib.mat4.transpose(colMajor);
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix], colMajor);
  },

  glFrustum: function(left, right, bottom, top_, nearVal, farVal) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix],
        GL.immediate.matrix.lib.mat4.frustum(left, right, bottom, top_, nearVal, farVal));
  },
  glFrustumf: 'glFrustum',

  glOrtho: function(left, right, bottom, top_, nearVal, farVal) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix],
        GL.immediate.matrix.lib.mat4.ortho(left, right, bottom, top_, nearVal, farVal));
  },
  glOrthof: 'glOrtho',

  glScaled: function(x, y, z) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.scale(GL.immediate.matrix[GL.immediate.currentMatrix], [x, y, z]);
  },
  glScalef: 'glScaled',

  glTranslated: function(x, y, z) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.translate(GL.immediate.matrix[GL.immediate.currentMatrix], [x, y, z]);
  },
  glTranslatef: 'glTranslated',

  glRotated: function(angle, x, y, z) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.rotate(GL.immediate.matrix[GL.immediate.currentMatrix], angle*Math.PI/180, [x, y, z]);
  },
  glRotatef: 'glRotated',

  // GLU

  gluPerspective: function(fov, aspect, near, far) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix[GL.immediate.currentMatrix] =
      GL.immediate.matrix.lib.mat4.perspective(fov, aspect, near, far,
                                               GL.immediate.matrix[GL.immediate.currentMatrix]);
  },

  gluLookAt: function(ex, ey, ez, cx, cy, cz, ux, uy, uz) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.lookAt(GL.immediate.matrix[GL.immediate.currentMatrix], [ex, ey, ez],
        [cx, cy, cz], [ux, uy, uz]);
  },

  gluProject: function(objX, objY, objZ, model, proj, view, winX, winY, winZ) {
    // The algorithm for this functions comes from Mesa

    var inVec = new Float32Array(4);
    var outVec = new Float32Array(4);
    GL.immediate.matrix.lib.mat4.multiplyVec4({{{ makeHEAPView('F64', 'model', 'model+' + (16*8)) }}},
        [objX, objY, objZ, 1.0], outVec);
    GL.immediate.matrix.lib.mat4.multiplyVec4({{{ makeHEAPView('F64', 'proj', 'proj+' + (16*8)) }}},
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
    var result = GL.immediate.matrix.lib.mat4.unproject([winX, winY, winZ],
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
  
  gluOrtho2D: function(left, right, bottom, top) {
    _glOrtho(left, right, bottom, top, -1, 1);
  },

  glDrawBuffer: function() { throw 'glDrawBuffer: TODO' },
  glReadBuffer: function() { throw 'glReadBuffer: TODO' },

  glLightfv: function() { throw 'glLightfv: TODO' },
  glLightModelfv: function() { throw 'glLightModelfv: TODO' },
  glMaterialfv: function() { throw 'glMaterialfv: TODO' },

  glTexGeni: function() { throw 'glTexGeni: TODO' },
  glTexGenfv: function() { throw 'glTexGenfv: TODO' },
  glTexEnvi: function() { Runtime.warnOnce('glTexEnvi: TODO') },
  glTexEnvf: function() { Runtime.warnOnce('glTexEnvf: TODO') },
  glTexEnvfv: function() { Runtime.warnOnce('glTexEnvfv: TODO') },

  glTexImage1D: function() { throw 'glTexImage1D: TODO' },
  glTexCoord3f: function() { throw 'glTexCoord3f: TODO' },
  glGetTexLevelParameteriv: function() { throw 'glGetTexLevelParameteriv: TODO' },

  glShadeModel: function() { Runtime.warnOnce('TODO: glShadeModel') },

  // GLES2 emulation

  glVertexAttribPointer__sig: 'viiiiii',
  glVertexAttribPointer: function(index, size, type, normalized, stride, ptr) {
#if FULL_ES2
    var cb = GL.clientBuffers[index];
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
    Module.ctx.vertexAttribPointer(index, size, type, normalized, stride, ptr);
  },

  glEnableVertexAttribArray__sig: 'vi',
  glEnableVertexAttribArray: function(index) {
#if FULL_ES2
    var cb = GL.clientBuffers[index];
#if ASSERTIONS
    assert(cb, index);
#endif
    cb.enabled = true;
#endif
    Module.ctx.enableVertexAttribArray(index);
  },

  glDisableVertexAttribArray__sig: 'vi',
  glDisableVertexAttribArray: function(index) {
#if FULL_ES2
    var cb = GL.clientBuffers[index];
#if ASSERTIONS
    assert(cb, index);
#endif
    cb.enabled = false;
#endif
    Module.ctx.disableVertexAttribArray(index);
  },

  glDrawArrays__sig: 'viii',
  glDrawArrays: function(mode, first, count) {
#if FULL_ES2
    // bind any client-side buffers
    GL.preDrawHandleClientVertexAttribBindings(first + count);
#endif

    Module.ctx.drawArrays(mode, first, count);

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
      buf = GL.tempIndexBuffers[GL.tempBufferIndexLookup[size]];
      Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, buf);
      Module.ctx.bufferSubData(Module.ctx.ELEMENT_ARRAY_BUFFER,
                               0,
                               HEAPU8.subarray(indices, indices + size));
      // the index is now 0
      indices = 0;
    }

    // bind any client-side buffers
    GL.preDrawHandleClientVertexAttribBindings(count);
#endif

    Module.ctx.drawElements(mode, count, type, indices);

#if FULL_ES2
    GL.postDrawHandleClientVertexAttribBindings(count);

    if (!GL.currElementArrayBuffer) {
      Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, null);
    }
#endif
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
  glGetError__sig: 'i',
  glFrontFace__sig: 'vi',
  glSampleCoverage__sig: 'vi',

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
  glGenVertexArraysOES: 'glGenVertexArrays',
  glDeleteVertexArraysOES: 'glDeleteVertexArrays',
  glBindVertexArrayOES: 'glBindVertexArray',
  glFramebufferTexture2DOES: 'glFramebufferTexture2D'
};

// Simple pass-through functions. Starred ones have return values. [X] ones have X in the C name but not in the JS name
[[0, 'getError* finish flush'],
 [1, 'clearDepth clearDepth[f] depthFunc enable disable frontFace cullFace clear lineWidth clearStencil depthMask stencilMask checkFramebufferStatus* generateMipmap activeTexture blendEquation sampleCoverage isEnabled*'],
 [2, 'blendFunc blendEquationSeparate depthRange depthRange[f] stencilMaskSeparate hint polygonOffset vertexAttrib1f'],
 [3, 'texParameteri texParameterf vertexAttrib2f stencilFunc stencilOp'],
 [4, 'viewport clearColor scissor vertexAttrib3f colorMask renderbufferStorage blendFuncSeparate blendColor stencilFuncSeparate stencilOpSeparate'],
 [5, 'vertexAttrib4f'],
 [8, 'copyTexImage2D copyTexSubImage2D']].forEach(function(data) {
  var num = data[0];
  var names = data[1];
  var args = range(num).map(function(i) { return 'x' + i }).join(', ');
  var plainStub = '(function(' + args + ') { Module.ctx.NAME(' + args + ') })';
  var returnStub = '(function(' + args + ') { return Module.ctx.NAME(' + args + ') })';
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

// Emulation requires everything else, potentially
LibraryGL.$GLEmulation__deps = LibraryGL.$GLEmulation__deps.slice(0); // the __deps object is shared
var glFuncs = [];
for (var item in LibraryGL) {
  if (item != '$GLEmulation' && item.substr(-6) != '__deps' && item.substr(-9) != '__postset' && item.substr(-5) != '__sig' && item.substr(0, 2) == 'gl') {
    glFuncs.push(item);
  }
}
LibraryGL.$GLEmulation__deps = LibraryGL.$GLEmulation__deps.concat(glFuncs);
LibraryGL.$GLEmulation__deps.push(function() {
  for (var func in Functions.getIndex.tentative) {
    Functions.getIndex(func);
    Functions.unimplementedFunctions[func] = LibraryGL[func.substr(1) + '__sig'];
  }
});

if (FORCE_GL_EMULATION) {
  LibraryGL.glDrawElements__deps = LibraryGL.glDrawElements__deps.concat('$GLEmulation');
  LibraryGL.glDrawArrays__deps = LibraryGL.glDrawArrays__deps.concat('$GLEmulation');
}

mergeInto(LibraryManager.library, LibraryGL);

