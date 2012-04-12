/*
 * GL support. See https://github.com/kripken/emscripten/wiki/OpenGL-support
 * for current status.
 */

var LibraryGL = {
  $GL: {
    counter: 1,
    buffers: {},
    programs: {},
    framebuffers: {},
    renderbuffers: {},
    textures: {},
    uniforms: {},
    shaders: {},

    // The folowing data structures are used for OpenGL Immediate Mode matrix routines.
    matrix: {
      'm': null, // modelview
      'p': null  // projection
    },
    matrixStack: {
      'm': [], // modelview
      'p': []  // projection
    },
    currentMatrix: 'm', // default is modelview
    tempMatrix: null,
    initMatrixLibrary: function() {
      GL.matrix['m'] = GL.matrix.lib.mat4.create();
      GL.matrix['p'] = GL.matrix.lib.mat4.create();
      GL.tempMatrix = GL.matrix.lib.mat4.create();
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
    }
  },

  glGetString: function(name_) {
    switch(name_) {
      case 0x1F00 /* GL_VENDOR */:
      case 0x1F01 /* GL_RENDERER */:
      case 0x1F02 /* GL_VERSION */:
        return allocate(intArrayFromString(Module.ctx.getParameter(name_)), 'i8', ALLOC_NORMAL);
      case 0x1F03 /* GL_EXTENSIONS */:
        return allocate(intArrayFromString(Module.ctx.getSupportedExtensions().join(' ')), 'i8', ALLOC_NORMAL);
      default:
        throw 'Failure: Invalid glGetString value: ' + name_;
    }
  },

  glGetIntegerv: function(name_, p) {
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
      var id = GL.counter++;
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

  glCompressedTexImage2D: function(target, level, internalformat, width, height, border, imageSize, data) {
    if (data) {
      data = {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}};
    } else {
      data = null;
    }
    Module.ctx.compressedTexImage2D(target, level, internalformat, width, height, border, data);
  },

  glCompressedTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, imageSize, data) {
    if (data) {
      data = {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}};
    } else {
      data = null;
    }
    Module.ctx.compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, data);
  },

  glTexImage2D: function(target, level, internalformat, width, height, border, format, type, pixels) {
    if (pixels) {
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
              throw 'Invalid format (' + format + ') passed to glTexImage2D';
          }
          pixels = {{{ makeHEAPView('U8', 'pixels', 'pixels+width*height*sizePerPixel') }}};
          break;
        case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
        case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
        case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
          sizePerPixel = 2;
          pixels = {{{ makeHEAPView('U16', 'pixels', 'pixels+width*height*sizePerPixel') }}};
          break;
        default:
          throw 'Invalid type (' + type + ') passed to glTexImage2D';
      }
    } else {
      pixels = null;
    }
    Module.ctx.texImage2D(target, level, internalformat, width, height, border, format, type, pixels);
  },

  glTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, type, pixels) {
    if (pixels) {
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
              throw 'Invalid format (' + format + ') passed to glTexSubImage2D';
          }
          pixels = {{{ makeHEAPView('U8', 'pixels', 'pixels+width*height*sizePerPixel') }}};
          break;
        case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
        case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
        case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
          sizePerPixel = 2;
          pixels = {{{ makeHEAPView('U16', 'pixels', 'pixels+width*height*sizePerPixel') }}};
          break;
        default:
          throw 'Invalid type (' + type + ') passed to glTexSubImage2D';
      }
    } else {
      pixels = null;
    }
    Module.ctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
  },

  glBindTexture: function(target, texture) {
    Module.ctx.bindTexture(target, GL.textures[texture]);
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
      return false;
    }
    return Module.ctx.isTexture(fb);
  },

  glGenBuffers: function(n, buffers) {
    for (var i = 0; i < n; i++) {
      var id = GL.counter++;
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

  glBufferData: function(target, size, data, usage) {
    Module.ctx.bufferData(target, HEAPU8.subarray(data, data+size), usage);
  },

  glBufferSubData: function(target, offset, size, data) {
    var floatArray = {{{ makeHEAPView('F32', 'data', 'data+size') }}};
    Module.ctx.bufferSubData(target, offset, floatArray);
  },

  glIsBuffer: function(buffer) {
    var fb = GL.buffers[buffer];
    if (typeof(fb) == 'undefined') {
      return false;
    }
    return Module.ctx.isBuffer(fb);
  },

  glGenRenderbuffers: function(n, renderbuffers) {
    for (var i = 0; i < n; i++) {
      var id = GL.counter++;
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
    Module.ctx.bindRenderbuffer(target, GL.renderbuffers[renderbuffer]);
  },

  glGetRenderbufferParameteriv: function(target, pname, params) {
    {{{ makeSetValue('params', '0', 'Module.ctx.getRenderbufferParameter(target, pname)', 'i32') }}};
  },

  glIsRenderbuffer: function(renderbuffer) {
    var fb = GL.renderbuffers[renderbuffer];
    if (typeof(fb) == 'undefined') {
      return false;
    }
    return Module.ctx.isRenderbuffer(fb);
  },

  glGetUniformLocation: function(program, name) {
    name = Pointer_stringify(name);
    var loc = Module.ctx.getUniformLocation(GL.programs[program], name);
    if (!loc) return -1;
    var id = GL.counter++;
    GL.uniforms[id] = loc;
    return id;
  },

  glGetActiveUniform: function(program, index, bufSize, length, size, type, name) {
    program = GL.programs[program];
    var info = Module.ctx.getActiveUniform(program, index);

    var infoname = info.name.slice(0, bufsize - 1);
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
    Module.ctx.bindBuffer(target, GL.buffers[buffer]);
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

    var infoname = info.name.slice(0, bufsize - 1);
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
    var id = GL.counter++;
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
    var source = "";
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
    Module.ctx.shaderSource(GL.shaders[shader], source);
  },

  glGetShaderSource: function(shader, bufsize, length, source) {
    var result = Module.ctx.getShaderSource(GL.shaders[shader]);
    result.slice(0, bufsize - 1);
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
    {{{ makeSetValue('p', '0', 'Module.ctx.getShaderParameter(GL.shaders[shader], pname)', 'i32') }}};
  },

  glGetProgramiv : function(program, pname, p) {
    {{{ makeSetValue('p', '0', 'Module.ctx.getProgramParameter(GL.programs[program], pname)', 'i32') }}};
  },

  glIsShader: function(shader) {
    var fb = GL.shaders[shader];
    if (typeof(fb) == 'undefined') {
      return false;
    }
    return Module.ctx.isShader(fb);
  },

  glCreateProgram: function() {
    var id = GL.counter++;
    GL.programs[id] = Module.ctx.createProgram();
    return id;
  },

  glDeleteProgram: function(program) {
    Module.ctx.deleteProgram(GL.programs[program]);
    GL.programs[program] = null;
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
    Module.ctx.useProgram(GL.programs[program]);
  },

  glValidateProgram: function(program) {
    Module.ctx.validateProgram(GL.programs[program]);
  },

  glIsProgram: function(program) {
    var fb = GL.programs[program];
    if (typeof(fb) == 'undefined') {
      return false;
    }
    return Module.ctx.isProgram(fb);
  },

  glBindAttribLocation: function(program, index, name) {
    name = Pointer_stringify(name);
    Module.ctx.bindAttribLocation(GL.programs[program], index, name);
  },

  glBindFramebuffer: function(target, framebuffer) {
    Module.ctx.bindFramebuffer(target, GL.framebuffers[framebuffer]);
  },

  glGenFramebuffers: function(n, ids) {
    for (var i = 0; i < n; ++i) {
      var id = GL.counter++;
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
      return false;
    }
    return Module.ctx.isFramebuffer(fb);
  },

  // OpenGL Immediate Mode matrix routines.
  // Note that in the future we might make these available only in certain modes.
  glMatrixMode: function(mode) {
    if (mode == 0x1700 /* GL_MODELVIEW */) {
      GL.currentMatrix = 'm';
    } else if (mode == 0x1701 /* GL_PROJECTION */) {
      GL.currentMatrix = 'p';
    } else {
      throw "Wrong mode " + mode + " passed to glMatrixMode";
    }
  },

  glPushMatrix: function() {
    GL.matrixStack[GL.currentMatrix].push(
        Array.prototype.slice.call(GL.matrix[GL.currentMatrix]));
  },

  glPopMatrix: function() {
    GL.matrix[currentMatrix] = GL.matrixStack[GL.currentMatrix].pop();
  },

  glLoadIdentity: function() {
    GL.matrix.lib.mat4.identity(GL.matrix[GL.currentMatrix]);
  },

  glLoadMatrixd: function(matrix) {
    GL.matrix.lib.mat4.set(GL.matrix[GL.currentMatrix],
        {{{ makeHEAPView('F64', 'matrix', 'matrix+16*8') }}});
  },

  glLoadMatrixf: function(matrix) {
    GL.matrix.lib.mat4.set(GL.matrix[GL.currentMatrix],
        {{{ makeHEAPView('F32', 'matrix', 'matrix+16*4') }}});
  },

  glLoadTransposeMatrixd: function(matrix) {
    GL.matrix.lib.mat4.set(GL.matrix[GL.currentMatrix],
        {{{ makeHEAPView('F64', 'matrix', 'matrix+16*8') }}});
    GL.matrix.lib.mat4.transpose(GL.matrix[GL.currentMatrix]);
  },

  glLoadTransposeMatrixf: function(matrix) {
    GL.matrix.lib.mat4.set(GL.matrix[GL.currentMatrix],
        {{{ makeHEAPView('F32', 'matrix', 'matrix+16*4') }}});
    GL.matrix.lib.mat4.transpose(GL.matrix[GL.currentMatrix]);
  },

  glMultMatrixd: function(matrix) {
    GL.matrix.lib.mat4.multiply(GL.matrix[GL.currentMatrix],
        {{{ makeHEAPView('F64', 'matrix', 'matrix+16*8') }}});
  },

  glMultMatrixf: function(matrix) {
    GL.matrix.lib.mat4.multiply(GL.matrix[GL.currentMatrix],
        {{{ makeHEAPView('F32', 'matrix', 'matrix+16*4') }}});
  },

  glMultTransposeMatrixd: function(matrix) {
    var colMajor = GL.matrix.lib.mat4.create();
    GL.matrix.lib.mat4.set(colMajor,
        {{{ makeHEAPView('F64', 'matrix', 'matrix+16*8') }}});
    GL.matrix.lib.mat4.transpose(colMajor);
    GL.matrix.lib.mat4.multiply(GL.matrix[GL.currentMatrix], colMajor);
  },

  glMultTransposeMatrixf: function(matrix) {
    var colMajor = GL.matrix.lib.mat4.create();
    GL.matrix.lib.mat4.set(colMajor,
        {{{ makeHEAPView('F32', 'matrix', 'matrix+16*4') }}});
    GL.matrix.lib.mat4.transpose(colMajor);
    GL.matrix.lib.mat4.multiply(GL.matrix[GL.currentMatrix], colMajor);
  },

  gluPerspective: function(fov, aspect, near, far) {
    GL.matrix.lib.mat4.multiply(GL.matrix[GL.currentMatrix],
        GL.matrix.lib.mat4.perspective(fov, aspect, near, far, GL.tempMatrix));
  },

  glFrustum: function(left, right, bottom, top_, nearVal, farVal) {
    GL.matrix.lib.mat4.multiply(GL.matrix[GL.currentMatrix],
        GL.matrix.lib.mat4.frustum(left, right, bottom, top_, nearVal, farVal));
  },

  glOrtho: function(left, right, bottom, top_, nearVal, farVal) {
    GL.matrix.lib.mat4.multiply(GL.matrix[GL.currentMatrix],
        GL.matrix.lib.mat4.ortho(left, right, bottom, top_, nearVal, farVal));
  },

  glScaled: function(x, y, z) {
    GL.matrix.lib.mat4.scale(GL.matrix[GL.currentMatrix], [x, y, z]);
  },

  glScalef: function(x, y, z) {
    _glScalef(x, y, z);
  },

  glTranslate: function(x, y, z) {
    GL.matrix.lib.mat4.translate(GL.matrix[GL.currentMatrix], [x, y, z]);
  },

  glTranslatef: function(x, y, z) {
    _glTranslated(x, y, z);
  },

  glRotated: function(angle, x, y, z) {
    GL.matrix.lib.mat4.rotate(GL.matrix[GL.currentMatrix], angle, [x, y, z]);
  },

  glRotatef: function(angle, x, y, z) {
    _glRotated(angle, x, y, z);
  },

  gluLookAt: function(ex, ey, ez, cx, cy, cz, ux, uy, uz) {
    GL.matrix.lib.mat4.lookAt(GL.matrix[GL.currentMatrix], [ex, ey, ez],
        [cx, cy, cz], [ux, uy, uz]);
  },

  gluProject: function(objX, objY, objZ, model, proj, view, winX, winY, winZ) {
    // The algorithm for this functions comes from Mesa

    var inVec = new Float32Array(4);
    var outVec = new Float32Array(4);
    GL.matrix.lib.mat4.multiplyVec4({{{ makeHEAPView('F64', 'model', 'model+16*8') }}},
        [objX, objY, objZ, 1.0], outVec);
    GL.matrix.lib.mat4.multiplyVec4({{{ makeHEAPView('F64', 'proj', 'proj+16*8') }}},
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
    var result = GL.matrix.lib.mat4.unproject([winX, winY, winZ],
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
  },

  // GL emulation: provides misc. functionality not present in OpenGL ES 2.0 or WebGL

  $GLEmulation__deps: ['glCreateShader', 'glShaderSource', 'glCompileShader', 'glCreateProgram', 'glDeleteShader', 'glDeleteProgram', 'glAttachShader', 'glActiveTexture', 'glGetShaderiv', 'glGetProgramiv', 'glLinkProgram'],
  $GLEmulation: {
    procReplacements: {
      'glCreateShaderObjectARB': 'glCreateShader',
      'glShaderSourceARB': 'glShaderSource',
      'glCompileShaderARB': 'glCompileShader',
      'glCreateProgramObjectARB': 'glCreateProgram',
      'glAttachObjectARB': 'glAttachShader',
      'glLinkProgramARB': 'glLinkProgram',
      'glActiveTextureARB': 'glActiveTexture'
    },

    procs: {
      glDeleteObjectARB: function(id) {
        if (GL.programs[id]) {
          _glDeleteProgram(id);
        } else if (GL.shaders[id]) {
          _glDeleteShader(id);
        } else {
          console.log('WARNING: deleteObjectARB received invalid id: ' + id);
        }
      },

      glGetObjectParameterivARB: function(id, type, result) {
        if (GL.programs[id]) {
          _glGetProgramiv(id, type, result);
        } else if (GL.shaders[id]) {
          _glGetShaderiv(id, type, result);
        } else {
          console.log('WARNING: getObjectParameterivARB received invalid id: ' + id);
        }
      },
    },

    getProcAddress: function(name_) {
      name_ = GLEmulation.procReplacements[name_] || name_;
      var func = GLEmulation.procs[name_];
      if (!func) {
        try {
          func = eval('_' + name_);
        } catch(e) {
          console.log('WARNING: getProcAddress failed for ' + name_);
          func = function() {
            console.log('WARNING: empty replacement for ' + name_ + ' called, no-op');
            return 0;
          };
        }
      }
      return Runtime.addFunction(func);
    }
  },

  glBegin__deps: ['$GL', function() { return 'GL.matrix.lib = ' + read('gl-matrix.js') + ';\n' +
                                          'GL.initMatrixLibrary();'
                                    }],
  glBegin: function() {
    Module.print('TODO');
  }
};

// Simple pass-through functions. Starred ones have return values.
[[0, 'shadeModel fogi fogfv getError finish flush'],
 [1, 'clearDepth depthFunc enable disable frontFace cullFace clear enableVertexAttribArray disableVertexAttribArray lineWidth clearStencil depthMask stencilMask stencilMaskSeparate checkFramebufferStatus* generateMipmap activeTexture blendEquation'],
 [2, 'pixelStorei blendFunc blendEquationSeparate'],
 [3, 'texParameteri texParameterf drawArrays vertexAttrib2f'],
 [4, 'viewport clearColor scissor vertexAttrib3f colorMask drawElements renderbufferStorage blendFuncSeparate'],
 [5, 'vertexAttrib4f'],
 [6, 'vertexAttribPointer'],
 [8, 'copyTexImage2D copyTexSubImage2D']].forEach(function(data) {
  var num = data[0];
  var names = data[1];
  var args = range(num).map(function(i) { return 'x' + i }).join(', ');
  var plainStub = '(function(' + args + ') { ' + (num > 0 ? 'Module.ctx.NAME(' + args + ')' : '') + ' })';
  var returnStub = '(function(' + args + ') { ' + (num > 0 ? 'return Module.ctx.NAME(' + args + ')' : '') + ' })';
  names.split(' ').forEach(function(name_) {
    var stub = plainStub;
    if (name_[name_.length-1] == '*') {
      name_ = name_.substr(0, name_.length-1);
      stub = returnStub;
    }
    var cName = 'gl' + name_[0].toUpperCase() + name_.substr(1);
    assert(!(cName in LibraryGL), "Cannot reimplement the existing function " + cName);
    LibraryGL[cName] = eval(stub.replace('NAME', name_));
  });
});

autoAddDeps(LibraryGL, '$GL');
mergeInto(LibraryManager.library, LibraryGL);

