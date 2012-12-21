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

    initExtensions: function() {
      if (GL.initExtensions.done) return;
      GL.initExtensions.done = true;

      if (!Module.useWebGL) return; // an app might link both gl and 2d backends

      GL.compressionExt = Module.ctx.getExtension('WEBGL_compressed_texture_s3tc') ||
                          Module.ctx.getExtension('MOZ_WEBGL_compressed_texture_s3tc') ||
                          Module.ctx.getExtension('WEBKIT_WEBGL_compressed_texture_s3tc');

      GL.anisotropicExt = Module.ctx.getExtension('EXT_texture_filter_anisotropic') ||
                          Module.ctx.getExtension('MOZ_EXT_texture_filter_anisotropic') ||
                          Module.ctx.getExtension('WEBKIT_EXT_texture_filter_anisotropic');

      GL.floatExt = Module.ctx.getExtension('OES_texture_float');
    }
  },

  glPixelStorei: function(pname, param) {
    if (pname == 0x0D05 /* GL_PACK_ALIGNMENT */) {
      GL.packAlignment = param;
    } else if (pname == 0x0cf5 /* GL_UNPACK_ALIGNMENT */) {
      GL.unpackAlignment = param;
    }
    Module.ctx.pixelStorei(pname, param);
  },

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

  glGenTextures: function(n, textures) {
    for (var i = 0; i < n; i++) {
      var id = GL.getNewId(GL.textures); 
      GL.textures[id] = Module.ctx.createTexture();
      {{{ makeSetValue('textures', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteTextures: function(n, textures) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('textures', 'i*4', 'i32') }}};
      Module.ctx.deleteTexture(GL.textures[id]);
      GL.textures[id] = null;
    }
  },

  glCompressedTexImage2D: function(target, level, internalFormat, width, height, border, imageSize, data) {
    assert(GL.compressionExt);
    if (data) {
      data = {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}};
    } else {
      data = null;
    }
    Module.ctx['compressedTexImage2D'](target, level, internalFormat, width, height, border, data);
  },

  glCompressedTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, imageSize, data) {
    assert(GL.compressionExt);
    if (data) {
      data = {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}};
    } else {
      data = null;
    }
    Module.ctx['compressedTexSubImage2D'](target, level, xoffset, yoffset, width, height, data);
  },

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

  glTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, type, pixels) {
    if (pixels) {
      var data = GL.getTexPixelData(type, format, width, height, pixels, -1);
      pixels = data.pixels;
    } else {
      pixels = null;
    }
    Module.ctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
  },

  glReadPixels: function(x, y, width, height, format, type, pixels) {
    Module.ctx.readPixels(x, y, width, height, format, type, HEAPU8.subarray(pixels));
  },

  glBindTexture: function(target, texture) {
    Module.ctx.bindTexture(target, texture ? GL.textures[texture] : null);
  },

  glGetTexParameterfv: function(target, pname, params) {
    {{{ makeSetValue('params', '0', 'Module.getTexParameter(target, pname)', 'float') }}};
  },

  glGetTexParameteriv: function(target, pname, params) {
    {{{ makeSetValue('params', '0', 'Module.getTexParameter(target, pname)', 'i32') }}};
  },

  glIsTexture: function(texture) {
    var fb = GL.textures[texture];
    if (typeof(fb) == 'undefined') {
      return 0;
    }
    return Module.ctx.isTexture(fb);
  },

  glGenBuffers: function(n, buffers) {
    for (var i = 0; i < n; i++) {
      var id = GL.getNewId(GL.buffers);
      GL.buffers[id] = Module.ctx.createBuffer();
      {{{ makeSetValue('buffers', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteBuffers: function(n, buffers) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('buffers', 'i*4', 'i32') }}};
      Module.ctx.deleteBuffer(GL.buffers[id]);
      GL.buffers[id] = null;
    }
  },

  glGetBufferParameteriv: function(target, value, data) {
    {{{ makeSetValue('data', '0', 'Module.ctx.getBufferParameter(target, value)', 'i32') }}};
  },

  glBufferData: function(target, size, data, usage) {
    Module.ctx.bufferData(target, HEAPU8.subarray(data, data+size), usage);
  },

  glBufferSubData: function(target, offset, size, data) {
    Module.ctx.bufferSubData(target, offset, HEAPU8.subarray(data, data+size));
  },

  glIsBuffer: function(buffer) {
    var fb = GL.buffers[buffer];
    if (typeof(fb) == 'undefined') {
      return 0;
    }
    return Module.ctx.isBuffer(fb);
  },

  glGenRenderbuffers: function(n, renderbuffers) {
    for (var i = 0; i < n; i++) {
      var id = GL.getNewId(GL.renderbuffers);
      GL.renderbuffers[id] = Module.ctx.createRenderbuffer();
      {{{ makeSetValue('renderbuffers', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteRenderbuffers: function(n, renderbuffers) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('renderbuffers', 'i*4', 'i32') }}};
      Module.ctx.deleteRenderbuffer(GL.renderbuffers[id]);
      GL.renderbuffers[id];
    }
  },

  glBindRenderbuffer: function(target, renderbuffer) {
    Module.ctx.bindRenderbuffer(target, renderbuffer ? GL.renderbuffers[renderbuffer] : null);
  },

  glGetRenderbufferParameteriv: function(target, pname, params) {
    {{{ makeSetValue('params', '0', 'Module.ctx.getRenderbufferParameter(target, pname)', 'i32') }}};
  },

  glIsRenderbuffer: function(renderbuffer) {
    var fb = GL.renderbuffers[renderbuffer];
    if (typeof(fb) == 'undefined') {
      return 0;
    }
    return Module.ctx.isRenderbuffer(fb);
  },

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

  glGetVertexAttribfv: function(index, pname, params) {
    var data = Module.ctx.getVertexAttrib(index, pname);
    if (typeof data == 'number') {
      {{{ makeSetValue('params', '0', 'data', 'float') }}};
    } else {
      for (var i = 0; i < data.length; i++) {
        {{{ makeSetValue('params', 'i', 'data[i]', 'float') }}};
      }
    }
  },

  glGetVertexAttribiv: function(index, pname, params) {
    var data = Module.ctx.getVertexAttrib(index, pname);
    if (typeof data == 'number' || typeof data == 'boolean') {
      {{{ makeSetValue('params', '0', 'data', 'i32') }}};
    } else {
      for (var i = 0; i < data.length; i++) {
        {{{ makeSetValue('params', 'i', 'data[i]', 'i32') }}};
      }
    }
  },

  glGetVertexAttribPointerv: function(index, pname, pointer) {
    {{{ makeSetValue('pointer', '0', 'Module.ctx.getVertexAttribOffset(index, pname)', 'i32') }}};
  },

  glGetActiveUniform: function(program, index, bufSize, length, size, type, name) {
    program = GL.programs[program];
    var info = Module.ctx.getActiveUniform(program, index);

    var infoname = info.name.slice(0, bufSize - 1);
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

  glUniform1f: function(location, v0) {
    location = GL.uniforms[location];
    Module.ctx.uniform1f(location, v0);
  },

  glUniform2f: function(location, v0, v1) {
    location = GL.uniforms[location];
    Module.ctx.uniform2f(location, v0, v1);
  },

  glUniform3f: function(location, v0, v1, v2) {
    location = GL.uniforms[location];
    Module.ctx.uniform3f(location, v0, v1, v2);
  },

  glUniform4f: function(location, v0, v1, v2, v3) {
    location = GL.uniforms[location];
    Module.ctx.uniform4f(location, v0, v1, v2, v3);
  },

  glUniform1i: function(location, v0) {
    location = GL.uniforms[location];
    Module.ctx.uniform1i(location, v0);
  },

  glUniform2i: function(location, v0, v1) {
    location = GL.uniforms[location];
    Module.ctx.uniform2i(location, v0, v1);
  },

  glUniform3i: function(location, v0, v1, v2) {
    location = GL.uniforms[location];
    Module.ctx.uniform3i(location, v0, v1, v2);
  },

  glUniform4i: function(location, v0, v1, v2, v3) {
    location = GL.uniforms[location];
    Module.ctx.uniform4i(location, v0, v1, v2, v3);
  },

  glUniform1iv: function(location, count, value) {
    location = GL.uniforms[location];
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform1iv(location, value);
  },

  glUniform2iv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 2;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform2iv(location, value);
  },

  glUniform3iv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 3;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform3iv(location, value);
  },

  glUniform4iv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 4;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform4iv(location, value);
  },

  glUniform1fv: function(location, count, value) {
    location = GL.uniforms[location];
    value = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
    Module.ctx.uniform1fv(location, value);
  },

  glUniform2fv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 2;
    value = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
    Module.ctx.uniform2fv(location, value);
  },

  glUniform3fv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 3;
    value = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
    Module.ctx.uniform3fv(location, value);
  },

  glUniform4fv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 4;
    value = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
    Module.ctx.uniform4fv(location, value);
  },

  glUniformMatrix2fv: function(location, count, transpose, value) {
    location = GL.uniforms[location];
    count *= 4;
    value = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
    Module.ctx.uniformMatrix2fv(location, transpose, value);
  },

  glUniformMatrix3fv: function(location, count, transpose, value) {
    location = GL.uniforms[location];
    count *= 9;
    value = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
    Module.ctx.uniformMatrix3fv(location, transpose, value);
  },

  glUniformMatrix4fv: function(location, count, transpose, value) {
    location = GL.uniforms[location];
    count *= 16;
    value = {{{ makeHEAPView('F32', 'value', 'value+count*4') }}};
    Module.ctx.uniformMatrix4fv(location, transpose, value);
  },

  glBindBuffer: function(target, buffer) {
    Module.ctx.bindBuffer(target, buffer ? GL.buffers[buffer] : null);
  },

  glVertexAttrib1fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+1*4') }}};
    Module.ctx.vertexAttrib1fv(index, v);
  },

  glVertexAttrib2fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+2*4') }}};
    Module.ctx.vertexAttrib2fv(index, v);
  },

  glVertexAttrib3fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+3*4') }}};
    Module.ctx.vertexAttrib3fv(index, v);
  },

  glVertexAttrib4fv: function(index, v) {
    v = {{{ makeHEAPView('F32', 'v', 'v+4*4') }}};
    Module.ctx.vertexAttrib4fv(index, v);
  },

  glGetAttribLocation: function(program, name) {
    program = GL.programs[program];
    name = Pointer_stringify(name);
    return Module.ctx.getAttribLocation(program, name);
  },

  glGetActiveAttrib: function(program, index, bufSize, length, size, type, name) {
    program = GL.programs[program];
    var info = Module.ctx.getActiveAttrib(program, index);

    var infoname = info.name.slice(0, bufSize - 1);
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

  glCreateShader: function(shaderType) {
    var id = GL.getNewId(GL.shaders);
    GL.shaders[id] = Module.ctx.createShader(shaderType);
    return id;
  },

  glDeleteShader: function(shader) {
    Module.ctx.deleteShader(GL.shaders[shader]);
    GL.shaders[shader] = null;
  },

  glDetachShader: function(program, shader) {
    Module.ctx.detachShader(GL.programs[program],
                            GL.shaders[shader]);
  },

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

  glShaderSource: function(shader, count, string, length) {
    var source = GL.getSource(shader, count, string, length);
    Module.ctx.shaderSource(GL.shaders[shader], source);
  },

  glGetShaderSource: function(shader, bufSize, length, source) {
    var result = Module.ctx.getShaderSource(GL.shaders[shader]);
    result.slice(0, bufSize - 1);
    writeStringToMemory(result, source);
    if (length) {
      {{{ makeSetValue('length', '0', 'result.length', 'i32') }}};
    }
  },

  glCompileShader: function(shader) {
    Module.ctx.compileShader(GL.shaders[shader]);
  },

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

  glGetShaderiv : function(shader, pname, p) {
    if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
      {{{ makeSetValue('p', '0', 'Module.ctx.getShaderInfoLog(GL.shaders[shader]).length + 1', 'i32') }}};
    } else {
      {{{ makeSetValue('p', '0', 'Module.ctx.getShaderParameter(GL.shaders[shader], pname)', 'i32') }}};
    }
  },

  glGetProgramiv : function(program, pname, p) {
    if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
      {{{ makeSetValue('p', '0', 'Module.ctx.getProgramInfoLog(GL.programs[program]).length + 1', 'i32') }}};
    } else {
      {{{ makeSetValue('p', '0', 'Module.ctx.getProgramParameter(GL.programs[program], pname)', 'i32') }}};
    }
  },

  glIsShader: function(shader) {
    var fb = GL.shaders[shader];
    if (typeof(fb) == 'undefined') {
      return 0;
    }
    return Module.ctx.isShader(fb);
  },

  glCreateProgram: function() {
    var id = GL.getNewId(GL.programs);
    GL.programs[id] = Module.ctx.createProgram();
    return id;
  },

  glDeleteProgram: function(program) {
    Module.ctx.deleteProgram(GL.programs[program]);
    GL.programs[program] = null;
    GL.uniformTable[program] = null;
  },

  glAttachShader: function(program, shader) {
    Module.ctx.attachShader(GL.programs[program],
                            GL.shaders[shader]);
  },

  glGetShaderPrecisionFormat: function(shaderType, precisionType, range, precision) {
    var result = Module.ctx.getShaderPrecisionFormat(shaderType, precisionType);
    {{{ makeSetValue('range', '0', 'result.rangeMin', 'i32') }}};
    {{{ makeSetValue('range', '4', 'result.rangeMax', 'i32') }}};
    {{{ makeSetValue('precision', '0', 'result.precision', 'i32') }}};
  },

  glLinkProgram: function(program) {
    Module.ctx.linkProgram(GL.programs[program]);
    GL.uniformTable[program] = {}; // uniforms no longer keep the same names after linking
  },

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

  glUseProgram: function(program) {
    Module.ctx.useProgram(program ? GL.programs[program] : null);
  },

  glValidateProgram: function(program) {
    Module.ctx.validateProgram(GL.programs[program]);
  },

  glIsProgram: function(program) {
    var fb = GL.programs[program];
    if (typeof(fb) == 'undefined') {
      return 0;
    }
    return Module.ctx.isProgram(fb);
  },

  glBindAttribLocation: function(program, index, name) {
    name = Pointer_stringify(name);
    Module.ctx.bindAttribLocation(GL.programs[program], index, name);
  },

  glBindFramebuffer: function(target, framebuffer) {
    Module.ctx.bindFramebuffer(target, framebuffer ? GL.framebuffers[framebuffer] : null);
  },

  glGenFramebuffers: function(n, ids) {
    for (var i = 0; i < n; ++i) {
      var id = GL.getNewId(GL.framebuffers);
      GL.framebuffers[id] = Module.ctx.createFramebuffer();
      {{{ makeSetValue('ids', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteFramebuffers: function(n, framebuffers) {
    for (var i = 0; i < n; ++i) {
      var id = {{{ makeGetValue('framebuffers', 'i*4', 'i32') }}};
      Module.ctx.deleteFramebuffer(GL.framebuffers[id]);
      GL.framebuffers[id] = null;
    }
  },

  glFramebufferRenderbuffer: function(target, attachment, renderbuffertarget, renderbuffer) {
    Module.ctx.framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                       GL.renderbuffers[renderbuffer]);
  },

  glFramebufferTexture2D: function(target, attachment, textarget, texture, level) {
    Module.ctx.framebufferTexture2D(target, attachment, textarget,
                                    GL.textures[texture], level);
  },

  glGetFramebufferAttachmentParameteriv: function(target, attachment, pname, params) {
    var result = Module.ctx.getFramebufferAttachmentParameter(target, attachment, pname);
    {{{ makeSetValue('params', '0', 'params', 'i32') }}};
  },

  glIsFramebuffer: function(framebuffer) {
    var fb = GL.framebuffers[framebuffer];
    if (typeof(fb) == 'undefined') {
      return 0;
    }
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

    init: function() {
      GLEmulation.fogColor = new Float32Array(4);

      // Add some emulation workarounds
      Module.printErr('WARNING: using emscripten GL emulation. This is a collection of limited workarounds, do not expect it to work');

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
          GL.currArrayBuffer = buffer;
        } else if (target == Module.ctx.ELEMENT_ARRAY_BUFFER) {
          GL.currElementArrayBuffer = buffer;
        }
      };

      var glDeleteBuffers = _glDeleteBuffers;
      _glDeleteBuffers = function(n, buffers) {
        glDeleteBuffers(n, buffers);
        for (var i = 0; i < n; i++) {
          var buffer = {{{ makeGetValue('buffers', 'i*4', 'i32') }}};
          if (buffer == GL.currArrayBuffer) GL.currArrayBuffer = 0;
          if (buffer == GL.currElementArrayBuffer) GL.currElementArrayBuffer = 0;
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
    },

    getProcAddress: function(name) {
      name = name.replace('EXT', '').replace('ARB', '');
      // Do the translation carefully because of closure
      switch (name) {
        case 'glCreateShaderObject': case 'glCreateShader': func = _glCreateShader; break;
        case 'glCreateProgramObject': case 'glCreateProgram': func = _glCreateProgram; break;
        case 'glAttachObject': case 'glAttachShader': func = _glAttachShader; break;
        case 'glUseProgramObject': case 'glUseProgram': func = _glUseProgram; break;
        case 'glDeleteObject': func = function(id) {
          if (GL.programs[id]) {
            _glDeleteProgram(id);
          } else if (GL.shaders[id]) {
            _glDeleteShader(id);
          } else {
            Module.printErr('WARNING: deleteObject received invalid id: ' + id);
          }
        }; break;
        case 'glGetObjectParameteriv': func = function(id, type, result) {
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
            }
            _glGetShaderiv(id, type, result);
          } else {
            Module.printErr('WARNING: getObjectParameteriv received invalid id: ' + id);
          }
        }; break;
        case 'glGetInfoLog': func = function(id, maxLength, length, infoLog) {
          if (GL.programs[id]) {
            _glGetProgramInfoLog(id, maxLength, length, infoLog);
          } else if (GL.shaders[id]) {
            _glGetShaderInfoLog(id, maxLength, length, infoLog);
          } else {
            Module.printErr('WARNING: getObjectParameteriv received invalid id: ' + id);
          }
        }; break;
        case 'glBindProgram': func = function(type, id) {
          assert(id == 0);
        }; break;
        case 'glDrawRangeElements': func = _glDrawRangeElements; break;
        case 'glShaderSource': func = _glShaderSource; break;
        case 'glCompileShader': func = _glCompileShader; break;
        case 'glLinkProgram': func = _glLinkProgram; break;
        case 'glGetUniformLocation': func = _glGetUniformLocation; break;
        case 'glUniform1f': func = _glUniform1f; break;
        case 'glUniform2f': func = _glUniform2f; break;
        case 'glUniform3f': func = _glUniform3f; break;
        case 'glUniform4f': func = _glUniform4f; break;
        case 'glUniform1fv': func = _glUniform1fv; break;
        case 'glUniform2fv': func = _glUniform2fv; break;
        case 'glUniform3fv': func = _glUniform3fv; break;
        case 'glUniform4fv': func = _glUniform4fv; break;
        case 'glUniform1i': func = _glUniform1i; break;
        case 'glUniform2i': func = _glUniform2i; break;
        case 'glUniform3i': func = _glUniform3i; break;
        case 'glUniform4i': func = _glUniform4i; break;
        case 'glUniform1iv': func = _glUniform1iv; break;
        case 'glUniform2iv': func = _glUniform2iv; break;
        case 'glUniform3iv': func = _glUniform3iv; break;
        case 'glUniform4iv': func = _glUniform4iv; break;
        case 'glBindAttribLocation': func = _glBindAttribLocation; break;
        case 'glGetActiveUniform': func = _glGetActiveUniform; break;
        case 'glGenBuffers': func = _glGenBuffers; break;
        case 'glBindBuffer': func = _glBindBuffer; break;
        case 'glBufferData': func = _glBufferData; break;
        case 'glBufferSubData': func = _glBufferSubData; break;
        case 'glDeleteBuffers': func = _glDeleteBuffers; break;
        case 'glActiveTexture': func = _glActiveTexture; break;
        case 'glClientActiveTexture': func = _glClientActiveTexture; break;
        case 'glGetProgramiv': func = _glGetProgramiv; break;
        case 'glEnableVertexAttribArray': func = _glEnableVertexAttribArray; break;
        case 'glDisableVertexAttribArray': func = _glDisableVertexAttribArray; break;
        case 'glVertexAttribPointer': func = _glVertexAttribPointer; break;
        case 'glBindRenderbuffer': func = _glBindRenderbuffer; break;
        case 'glDeleteRenderbuffers': func = _glDeleteRenderbuffers; break;
        case 'glGenRenderbuffers': func = _glGenRenderbuffers; break;
        case 'glCompressedTexImage2D': func = _glCompressedTexImage2D; break;
        case 'glCompressedTexSubImage2D': func = _glCompressedTexSubImage2D; break;
        case 'glBindFramebuffer': func = _glBindFramebuffer; break;
        case 'glGenFramebuffers': func = _glGenFramebuffers; break;
        case 'glDeleteFramebuffers': func = _glDeleteFramebuffers; break;
        case 'glFramebufferRenderbuffer': func = _glFramebufferRenderbuffer; break;
        case 'glFramebufferTexture2D': func = _glFramebufferTexture2D; break;
        case 'glGetFramebufferAttachmentParameteriv': func = _glGetFramebufferAttachmentParameteriv; break;
        case 'glIsFramebuffer': func = _glIsFramebuffer; break;
        case 'glCheckFramebufferStatus': func = _glCheckFramebufferStatus; break;
        case 'glRenderbufferStorage': func = _glRenderbufferStorage; break;
        default: {
          Module.printErr('WARNING: getProcAddress failed for ' + name);
          func = function() {
            Module.printErr('WARNING: empty replacement for ' + name + ' called, no-op');
            return 0;
          };
        }
      }
      return Runtime.addFunction(func);
    }
  },

  // GL Immediate mode

  $GLImmediate__postset: 'Browser.moduleContextCreatedCallbacks.push(function() { GL.immediate.init() });',
  $GLImmediate__deps: ['$Browser', '$GL'],
  $GLImmediate: {
    MAX_TEXTURES: 7,

    // Vertex and index data
    vertexData: null, // current vertex data. either tempData (glBegin etc.) or a view into the heap (gl*Pointer). Default view is F32
    vertexDataU8: null, // U8 view
    tempData: null,
    indexData: null,
    vertexCounter: 0,
    mode: 0,

    rendererCache: null,
    rendererCacheItemTemplate: [null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null], // 16 nulls
    rendererComponents: [], // small cache for calls inside glBegin/end. counts how many times the element was seen
    rendererComponentPointer: 0, // next place to start a glBegin/end component
    lastRenderer: null, // used to avoid cleaning up and re-preparing the same renderer
    lastArrayBuffer: null, // used in conjunction with lastRenderer
    lastProgram: null, // ""

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
    clientAttributes: [{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}], // raw data, including possible unneeded ones
    liveClientAttributes: [], // the ones actually alive in the current computation, sorted
    modifiedClientAttributes: false,
    clientActiveTexture: 0,
    clientColor: null,

    byteSizeByTypeRoot: 0x1400, // GL_BYTE
    byteSizeByType: [
      1, // GL_BYTE
      1, // GL_UNSIGNED_BYTE
      2, // GL_SHORT
      2, // GL_UNSIGNED_SHORT
      4, // GL_INT
      4, // GL_UNSIGNED_INT
      4  // GL_FLOAT
    ],

    setClientAttribute: function(name, size, type, stride, pointer) {
      var attrib = this.clientAttributes[name];
      attrib.name = name;
      attrib.size = size;
      attrib.type = type;
      attrib.stride = stride;
      attrib.pointer = pointer;
      this.modifiedClientAttributes = true;
    },

    // Temporary buffers
    MAX_TEMP_BUFFER_SIZE: 2*1024*1024,
    tempBufferIndexLookup: null,
    tempVertexBuffers: null,
    tempIndexBuffers: null,
    tempQuadIndexBuffer: null,

    generateTempBuffers: function() {
      this.tempBufferIndexLookup = new Uint8Array(this.MAX_TEMP_BUFFER_SIZE+1);
      this.tempVertexBuffers = [];
      this.tempIndexBuffers = [];
      var last = -1, curr = -1;
      var size = 1;
      for (var i = 0; i <= this.MAX_TEMP_BUFFER_SIZE; i++) {
        if (i > size) {
          size <<= 1;
        }
        if (size != last) {
          curr++;
          this.tempVertexBuffers[curr] = Module.ctx.createBuffer();
          Module.ctx.bindBuffer(Module.ctx.ARRAY_BUFFER, this.tempVertexBuffers[curr]);
          Module.ctx.bufferData(Module.ctx.ARRAY_BUFFER, size, Module.ctx.DYNAMIC_DRAW);
          Module.ctx.bindBuffer(Module.ctx.ARRAY_BUFFER, null);
          this.tempIndexBuffers[curr] = Module.ctx.createBuffer();
          Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, this.tempIndexBuffers[curr]);
          Module.ctx.bufferData(Module.ctx.ELEMENT_ARRAY_BUFFER, size, Module.ctx.DYNAMIC_DRAW);
          Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, null);
          last = size;
        }
        this.tempBufferIndexLookup[i] = curr;
      }
      // GL_QUAD indexes can be precalculated
      this.tempQuadIndexBuffer = Module.ctx.createBuffer();
      Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, this.tempQuadIndexBuffer);
      var numIndexes = this.MAX_TEMP_BUFFER_SIZE >> 1;
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
        this.rendererComponentPointer += size * this.byteSizeByType[type - this.byteSizeByTypeRoot];
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
        var typeIndex = attribute.type - GL.immediate.byteSizeByTypeRoot; // ensure it starts at 0 to keep the cache items dense
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
      var hasTextures = false, textureSizes = [], textureTypes = [], textureOffsets = [];
      for (var i = 0; i < GL.immediate.NUM_TEXTURES; i++) {
        if (GL.immediate.enabledClientAttributes[GL.immediate.TEXTURE0 + i]) {
          textureSizes[i] = GL.immediate.clientAttributes[GL.immediate.TEXTURE0 + i].size;
          textureTypes[i] = GL.immediate.clientAttributes[GL.immediate.TEXTURE0 + i].type;
          textureOffsets[i] = GL.immediate.clientAttributes[GL.immediate.TEXTURE0 + i].offset;
          hasTextures = true;
        }
      }
      var stride = GL.immediate.stride;
      var positionSize = GL.immediate.clientAttributes[GL.immediate.VERTEX].size;
      var positionType = GL.immediate.clientAttributes[GL.immediate.VERTEX].type;
      var positionOffset = GL.immediate.clientAttributes[GL.immediate.VERTEX].offset;
      var colorSize = 0, colorType, colorOffset;
      if (GL.immediate.enabledClientAttributes[GL.immediate.COLOR]) {
        colorSize = GL.immediate.clientAttributes[GL.immediate.COLOR].size;
        colorType = GL.immediate.clientAttributes[GL.immediate.COLOR].type;
        colorOffset = GL.immediate.clientAttributes[GL.immediate.COLOR].offset;
      }
      var normalSize = 0, normalType, normalOffset;
      if (GL.immediate.enabledClientAttributes[GL.immediate.NORMAL]) {
        normalSize = GL.immediate.clientAttributes[GL.immediate.NORMAL].size;
        normalType = GL.immediate.clientAttributes[GL.immediate.NORMAL].type;
        normalOffset = GL.immediate.clientAttributes[GL.immediate.NORMAL].offset;
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
            assert(end <= GL.immediate.MAX_TEMP_BUFFER_SIZE, 'too much vertex data');
            arrayBuffer = GL.immediate.tempVertexBuffers[GL.immediate.tempBufferIndexLookup[end]];
            // TODO: consider using the last buffer we bound, if it was larger. downside is larger buffer, but we might avoid rebinding and preparing
          } else {
            arrayBuffer = GL.currArrayBuffer;
          }

          // If the array buffer is unchanged and the renderer as well, then we can avoid all the work here
          // XXX We use some heuristics here, and this may not work in all cases. Try disabling this if you
          // have odd glitches (by setting canSkip always to 0, or even cleaning up the renderer right
          // after rendering)
          var lastRenderer = GL.immediate.lastRenderer;
          var canSkip = this == lastRenderer &&
                        arrayBuffer == GL.immediate.lastArrayBuffer &&
                        (GL.currProgram || this.program) == GL.immediate.lastProgram &&
                        !GL.immediate.matricesModified;
          if (!canSkip && lastRenderer) lastRenderer.cleanup();
          if (!GL.currArrayBuffer) {
            // Bind the array buffer and upload data after cleaning up the previous renderer
            if (arrayBuffer != GL.immediate.lastArrayBuffer) {
              Module.ctx.bindBuffer(Module.ctx.ARRAY_BUFFER, arrayBuffer);
            }
            Module.ctx.bufferSubData(Module.ctx.ARRAY_BUFFER, start, GL.immediate.vertexData.subarray(start >> 2, end >> 2));
          }
          if (canSkip) return;
          GL.immediate.lastRenderer = this;
          GL.immediate.lastArrayBuffer = arrayBuffer;
          GL.immediate.lastProgram = GL.currProgram || this.program;
          GL.immediate.matricesModified = false;

          if (!GL.currProgram) {
            Module.ctx.useProgram(this.program);
          }

          if (this.modelViewLocation) Module.ctx.uniformMatrix4fv(this.modelViewLocation, false, GL.immediate.matrix['m']);
          if (this.projectionLocation) Module.ctx.uniformMatrix4fv(this.projectionLocation, false, GL.immediate.matrix['p']);

          Module.ctx.vertexAttribPointer(this.positionLocation, positionSize, positionType, false,
                                         stride, positionOffset);
          Module.ctx.enableVertexAttribArray(this.positionLocation);
          if (this.hasTextures) {
            for (var i = 0; i < textureSizes.length; i++) {
              if (textureSizes[i] && this.texCoordLocations[i] >= 0) {
                Module.ctx.vertexAttribPointer(this.texCoordLocations[i], textureSizes[i], textureTypes[i], false,
                                               stride, textureOffsets[i]);
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
                                           stride, colorOffset);
            Module.ctx.enableVertexAttribArray(this.colorLocation);
            Module.ctx.uniform1i(this.hasColorAttribLocation, 1);
          } else if (this.hasColorUniform) {
            Module.ctx.uniform1i(this.hasColorAttribLocation, 0);
            Module.ctx.uniform4fv(this.colorUniformLocation, GL.immediate.clientColor);
          }
          if (this.hasNormal) {
            Module.ctx.vertexAttribPointer(this.normalLocation, normalSize, normalType, true,
                                           stride, normalOffset);
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

          GL.immediate.lastRenderer = null;
          GL.immediate.lastArrayBuffer = null;
          GL.immediate.lastProgram = null;
          GL.immediate.matricesModified = true;
        }
      };
      ret.init();
      return ret;
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
      this.tempData = new Float32Array(this.MAX_TEMP_BUFFER_SIZE >> 2);
      this.indexData = new Uint16Array(this.MAX_TEMP_BUFFER_SIZE >> 1);

      this.vertexDataU8 = new Uint8Array(this.tempData.buffer);

      this.generateTempBuffers();

      this.clientColor = new Float32Array([1, 1, 1, 1]);

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
        GL.immediate.mode = 0;
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
        GL.immediate.mode = 0;
      };
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
      start = attributes[0].pointer;
      for (var i = 0; i < attributes.length; i++) {
        var attribute = attributes[i];
        if (!attribute) break;
#if ASSERTIONS
        assert(stride == 0 || stride == attribute.stride); // must all be in the same buffer
#endif
        if (attribute.stride) stride = attribute.stride;
      }

      var bytes = 0;
      for (var i = 0; i < attributes.length; i++) {
        var attribute = attributes[i];
        if (!attribute) break;
        attribute.offset = attribute.pointer - start;
        if (attribute.offset > bytes) { // ensure we start where we should
          assert((attribute.offset - bytes)%4 == 0); // XXX assuming 4-alignment
          bytes += attribute.offset - bytes;
        }
        bytes += attribute.size * GL.immediate.byteSizeByType[attribute.type - GL.immediate.byteSizeByTypeRoot];
        if (bytes % 4 != 0) bytes += 4 - (bytes % 4); // XXX assuming 4-alignment
      }
      assert(stride == 0 || bytes <= stride);
      if (bytes < stride) { // ensure the size is that of the stride
        bytes = stride;
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
          assert(numProvidedIndexes << 1 <= GL.immediate.MAX_TEMP_BUFFER_SIZE, 'too many immediate mode indexes (a)');
          var indexBuffer = GL.immediate.tempIndexBuffers[GL.immediate.tempBufferIndexLookup[numProvidedIndexes << 1]];
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
        assert(ptr + (numIndexes << 1) <= GL.immediate.MAX_TEMP_BUFFER_SIZE, 'too many immediate mode indexes (b)');
        Module.ctx.bindBuffer(Module.ctx.ELEMENT_ARRAY_BUFFER, this.tempQuadIndexBuffer);
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
    GL.immediate.mode = 0;
  },

  glVertex3f: function(x, y, z) {
#if ASSERTIONS
    assert(GL.immediate.mode); // must be in begin/end
#endif
    GL.immediate.vertexData[GL.immediate.vertexCounter++] = x;
    GL.immediate.vertexData[GL.immediate.vertexCounter++] = y;
    GL.immediate.vertexData[GL.immediate.vertexCounter++] = z || 0;
#if ASSERTIONS
    assert(GL.immediate.vertexCounter << 2 < GL.immediate.MAX_TEMP_BUFFER_SIZE);
#endif
    GL.immediate.addRendererComponent(GL.immediate.VERTEX, 3, Module.ctx.FLOAT);
  },
  glVertex2f: 'glVertex3f',

  glVertex3fv__deps: ['glVertex3f'],
  glVertex3fv: function(p) {
    _glVertex3f({{{ makeGetValue('p', '0', 'float') }}}, {{{ makeGetValue('p', '4', 'float') }}}, {{{ makeGetValue('p', '8', 'float') }}});
  },
  glVertex2fv: function(p) {
    _glVertex2f({{{ makeGetValue('p', '0', 'float') }}}, {{{ makeGetValue('p', '4', 'float') }}});
  },

  glTexCoord2i: function(u, v) {
#if ASSERTIONS
    assert(GL.immediate.mode); // must be in begin/end
#endif
    GL.immediate.vertexData[GL.immediate.vertexCounter++] = u;
    GL.immediate.vertexData[GL.immediate.vertexCounter++] = v;
    GL.immediate.addRendererComponent(GL.immediate.TEXTURE0, 2, Module.ctx.FLOAT);
  },
  glTexCoord2f: 'glTexCoord2i',

  glTexCoord2fv__deps: ['glTexCoord2f'],
  glTexCoord2fv: function(v) {
    _glTexCoord2f({{{ makeGetValue('v', '0', 'float') }}}, {{{ makeGetValue('v', '4', 'float') }}});
  },

  glColor4f: function(r, g, b, a) {
    r = Math.max(Math.min(r, 1), 0);
    g = Math.max(Math.min(g, 1), 0);
    b = Math.max(Math.min(b, 1), 0);
    a = Math.max(Math.min(a, 1), 0);

    // TODO: make ub the default, not f, save a few mathops
    if (GL.immediate.mode) {
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

  glDrawRangeElements: function(mode, start, end, count, type, indices) {
    _glDrawElements(mode, count, type, indices, start, end);
  },

  // ClientState/gl*Pointer

  glEnableClientState: function(cap, disable) {
    var attrib;
    switch(cap) {
      case 0x8078: // GL_TEXTURE_COORD_ARRAY
        attrib = GL.immediate.TEXTURE0 + GL.immediate.clientActiveTexture; break;
      case 0x8074: // GL_VERTEX_ARRAY
        attrib = GL.immediate.VERTEX; break;
      case 0x8075: // GL_NORMAL_ARRAY
        attrib = GL.immediate.NORMAL; break;
      case 0x8076: // GL_COLOR_ARRAY
        attrib = GL.immediate.COLOR; break;
      default:
        throw 'unhandled clientstate: ' + cap;
    }
    if (disable && GL.immediate.enabledClientAttributes[attrib]) {
      GL.immediate.enabledClientAttributes[attrib] = false;
      GL.immediate.totalEnabledClientAttributes--;
    } else if (!disable && !GL.immediate.enabledClientAttributes[attrib]) {
      GL.immediate.enabledClientAttributes[attrib] = true;
      GL.immediate.totalEnabledClientAttributes++;
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

  glClientActiveTexture: function(texture) {
    GL.immediate.clientActiveTexture = texture - 0x84C0; // GL_TEXTURE0
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
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+16*8') }}}, GL.immediate.matrix[GL.immediate.currentMatrix]);
  },

  glLoadMatrixf: function(matrix) {
#if GL_DEBUG
    if (GL.debug) Module.printErr('glLoadMatrixf receiving: ' + Array.prototype.slice.call(HEAPF32.subarray(matrix >> 2, (matrix >> 2) + 16)));
#endif
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+16*4') }}}, GL.immediate.matrix[GL.immediate.currentMatrix]);
  },

  glLoadTransposeMatrixd: function(matrix) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+16*8') }}}, GL.immediate.matrix[GL.immediate.currentMatrix]);
    GL.immediate.matrix.lib.mat4.transpose(GL.immediate.matrix[GL.immediate.currentMatrix]);
  },

  glLoadTransposeMatrixf: function(matrix) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+16*4') }}}, GL.immediate.matrix[GL.immediate.currentMatrix]);
    GL.immediate.matrix.lib.mat4.transpose(GL.immediate.matrix[GL.immediate.currentMatrix]);
  },

  glMultMatrixd: function(matrix) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix],
        {{{ makeHEAPView('F64', 'matrix', 'matrix+16*8') }}});
  },

  glMultMatrixf: function(matrix) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix],
        {{{ makeHEAPView('F32', 'matrix', 'matrix+16*4') }}});
  },

  glMultTransposeMatrixd: function(matrix) {
    GL.immediate.matricesModified = true;
    var colMajor = GL.immediate.matrix.lib.mat4.create();
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+16*8') }}}, colMajor);
    GL.immediate.matrix.lib.mat4.transpose(colMajor);
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix], colMajor);
  },

  glMultTransposeMatrixf: function(matrix) {
    GL.immediate.matricesModified = true;
    var colMajor = GL.immediate.matrix.lib.mat4.create();
    GL.immediate.matrix.lib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+16*4') }}}, colMajor);
    GL.immediate.matrix.lib.mat4.transpose(colMajor);
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix], colMajor);
  },

  glFrustum: function(left, right, bottom, top_, nearVal, farVal) {
    GL.immediate.matricesModified = true;
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix],
        GL.immediate.matrix.lib.mat4.frustum(left, right, bottom, top_, nearVal, farVal));
  },

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
    GL.immediate.matrix.lib.mat4.multiply(GL.immediate.matrix[GL.immediate.currentMatrix],
        GL.immediate.matrix.lib.mat4.perspective(fov, aspect, near, far, GL.immediate.currentMatrix));
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
    GL.immediate.matrix.lib.mat4.multiplyVec4({{{ makeHEAPView('F64', 'model', 'model+16*8') }}},
        [objX, objY, objZ, 1.0], outVec);
    GL.immediate.matrix.lib.mat4.multiplyVec4({{{ makeHEAPView('F64', 'proj', 'proj+16*8') }}},
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
    inVec[0] = inVec[0] * {{{ makeGetValue('view', '2*4', 'i32') }}} + {{{ makeGetValue('view', '0*4', 'i32') }}};
    inVec[1] = inVec[1] * {{{ makeGetValue('view', '3*4', 'i32') }}} + {{{ makeGetValue('view', '1*4', 'i32') }}};

    {{{ makeSetValue('winX', '0', 'inVec[0]', 'double') }}};
    {{{ makeSetValue('winY', '0', 'inVec[1]', 'double') }}};
    {{{ makeSetValue('winZ', '0', 'inVec[2]', 'double') }}};

    return 1 /* GL_TRUE */;
  },

  gluUnProject: function(winX, winY, winZ, model, proj, view, objX, objY, objZ) {
    var result = GL.immediate.matrix.lib.mat4.unproject([winX, winY, winZ],
        {{{ makeHEAPView('F64', 'model', 'model+16*8') }}},
        {{{ makeHEAPView('F64', 'proj', 'proj+16*8') }}},
        {{{ makeHEAPView('32', 'view', 'view+4*4') }}});

    if (result === null) {
      return 0 /* GL_FALSE */;
    }

    {{{ makeSetValue('objX', '0', 'result[0]', 'double') }}};
    {{{ makeSetValue('objY', '0', 'result[1]', 'double') }}};
    {{{ makeSetValue('objZ', '0', 'result[2]', 'double') }}};

    return 1 /* GL_TRUE */;
  }
};

// Simple pass-through functions. Starred ones have return values. [X] ones have X in the C name but not in the JS name
[[0, 'shadeModel getError* finish flush'],
 [1, 'clearDepth clearDepth[f] depthFunc enable disable frontFace cullFace clear enableVertexAttribArray disableVertexAttribArray lineWidth clearStencil depthMask stencilMask checkFramebufferStatus* generateMipmap activeTexture blendEquation sampleCoverage isEnabled*'],
 [2, 'blendFunc blendEquationSeparate depthRange depthRange[f] stencilMaskSeparate hint polygonOffset'],
 [3, 'texParameteri texParameterf drawArrays vertexAttrib2f stencilFunc stencilOp'],
 [4, 'viewport clearColor scissor vertexAttrib3f colorMask drawElements renderbufferStorage blendFuncSeparate blendColor stencilFuncSeparate stencilOpSeparate'],
 [5, 'vertexAttrib4f'],
 [6, 'vertexAttribPointer'],
 [8, 'copyTexImage2D copyTexSubImage2D']].forEach(function(data) {
  var num = data[0];
  var names = data[1];
  var args = range(num).map(function(i) { return 'x' + i }).join(', ');
  var plainStub = '(function(' + args + ') { ' + (num > 0 ? 'Module.ctx.NAME(' + args + ')' : '') + ' })';
  var returnStub = '(function(' + args + ') { ' + (num > 0 ? 'return Module.ctx.NAME(' + args + ')' : '') + ' })';
  names.split(' ').forEach(function(name) {
    var stub = plainStub;
    if (name[name.length-1] == '*') {
      name = name.substr(0, name.length-1);
      stub = returnStub;
    }
    var cName = name;
    if (name.indexOf('[') >= 0) {
      cName = name.replace('[', '').replace(']', '');
      name = cName.substr(0, cName.length-1);
    }
    var cName = 'gl' + cName[0].toUpperCase() + cName.substr(1);
    assert(!(cName in LibraryGL), "Cannot reimplement the existing function " + cName);
    LibraryGL[cName] = eval(stub.replace('NAME', name));
  });
});

autoAddDeps(LibraryGL, '$GL');

// Emulation requires everything else, potentially
LibraryGL.$GLEmulation__deps = LibraryGL.$GLEmulation__deps.slice(0);
for (var item in LibraryGL) {
  if (item != '$GLEmulation' && item.substr(-6) != '__deps' && item.substr(-9) != '__postset' && item.substr(0, 2) == 'gl') {
    LibraryGL.$GLEmulation__deps.push(item);
  }
}

mergeInto(LibraryManager.library, LibraryGL);

