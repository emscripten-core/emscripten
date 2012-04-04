//"use strict";

var LibraryGL = {
  $GL: {
    hashtable: function(name) {
      if (!this._hashtables) {
        this._hashtables = {};
      }
      if (!(name in this._hashtables)) {
        this._hashtables[name] = {
          table: {},
          counter: 1,
          add: function(obj) {
            var id = this.counter++;
            this.table[id] = obj;
            return id;
          },
          get: function(id) {
            if( id == 0 ) return null;
#if ASSERTIONS
            assert(id < this.counter, "Invalid id " + id + " for the hashtable " + name);
#endif
            return this.table[id];
          },
          id: function(obj) {
            for (var i = 1; i < this.counter; ++i) {
              if (obj == this.table[i]) {
                return i;
              }
            }
            return 0;
          },
          remove: function(id) {
            if( id == 0 ) return;
#if ASSERTIONS
            assert(id < this.counter, "Invalid id " + id + " for the hashtable " + name);
#endif
            delete this.table[id];
          },
          lookup: function(v) {
            for (var i = 1; i < this.counter; i++)
              if (this.table[i] == v)
                return i;
            return 0;
          },
        };
      }
      return this._hashtables[name];
    },
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
          {{{ makeSetValue('p', '0', 'GL.hashtable("buffer").id(result)', 'i32') }}};
        } else if (result instanceof WebGLProgram) {
          {{{ makeSetValue('p', '0', 'GL.hashtable("program").id(result)', 'i32') }}};
        } else if (result instanceof WebGLFramebuffer) {
          {{{ makeSetValue('p', '0', 'GL.hashtable("framebuffer").id(result)', 'i32') }}};
        } else if (result instanceof WebGLRenderbuffer) {
          {{{ makeSetValue('p', '0', 'gl.hashtable("renderbuffer").id(result)', 'i32') }}};
        } else if (result instanceof WebGLTexture) {
          {{{ makeSetValue('p', '0', 'gl.hashtable("texture").id(result)', 'i32') }}};
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
          {{{ makeSetValue('p', '0', 'GL.hashtable("buffer").id(result)', 'float') }}};
        } else if (result instanceof WebGLProgram) {
          {{{ makeSetValue('p', '0', 'GL.hashtable("program").id(result)', 'float') }}};
        } else if (result instanceof WebGLFramebuffer) {
          {{{ makeSetValue('p', '0', 'GL.hashtable("framebuffer").id(result)', 'float') }}};
        } else if (result instanceof WebGLRenderbuffer) {
          {{{ makeSetValue('p', '0', 'gl.hashtable("renderbuffer").id(result)', 'float') }}};
        } else if (result instanceof WebGLTexture) {
          {{{ makeSetValue('p', '0', 'gl.hashtable("texture").id(result)', 'float') }}};
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
      var id = GL.hashtable("texture").add(Module.ctx.createTexture());
      {{{ makeSetValue('textures', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteTextures: function(n, textures) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('textures', 'i*4', 'i32') }}};
      Module.ctx.deleteTexture(GL.hashtable("texture").get(id));
      GL.hashtable("texture").remove(id);
    }
  },

  glCompressedTexImage2D: function(target, level, internalformat, width, height, border, imageSize, data) {
    if (data) {
      data = new Uint8Array(Array_copy(data, imageSize));
    } else {
      data = null;
    }
    Module.ctx.compressedTexImage2D(target, level, internalformat, width, height, border, data);
  },

  glCompressedTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, imageSize, data) {
    if (data) {
      data = new Uint8Array(Array_copy(data, imageSize));
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
          pixels = new Uint8Array(Array_copy(pixels, width*height*sizePerPixel));
          break;
        case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
        case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
        case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
          sizePerPixel = 2;
          pixels = new Uint16Array(new ArrayBuffer(Array_copy(pixels, width*height*sizePerPixel*2)));
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
          pixels = new Uint8Array(Array_copy(pixels, (width-xoffset+1)*(height-yoffset+1)*sizePerPixel));
          break;
        case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
        case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
        case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
          sizePerPixel = 2;
          pixels = new Uint16Array(new ArrayBuffer(Array_copy(pixels, (width-xoffset+1)*(height-yoffset+1)*sizePerPixel*2)));
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
    Module.ctx.bindTexture(target, GL.hashtable("texture").get(texture));
  },

  glGetTexParameterfv: function(target, pname, params) {
    {{{ makeSetValue('params', '0', 'Module.getTexParameter(target, pname)', 'float') }}};
  },

  glGetTexParameteriv: function(target, pname, params) {
    {{{ makeSetValue('params', '0', 'Module.getTexParameter(target, pname)', 'i32') }}};
  },

  glIsTexture: function(texture) {
    var fb = GL.hashtable("texture").get(texture);
    if (typeof(fb) == 'undefined') {
      return false;
    }
    return Module.ctx.isTexture(fb);
  },

  glGenBuffers: function(n, buffers) {
    for (var i = 0; i < n; i++) {
      var id = GL.hashtable("buffer").add(Module.ctx.createBuffer());
      {{{ makeSetValue('buffers', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteBuffers: function(n, buffers) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('buffers', 'i*4', 'i32') }}};
      Module.ctx.deleteBuffer(GL.hashtable("buffer").get(id));
      GL.hashtable("buffer").remove(id);
    }
  },

  glBufferData: function(target, size, data, usage) {
    Module.ctx.bufferData(target, HEAPU8.subarray(data, data+size), usage);
  },

  glBufferSubData: function(target, offset, size, data) {
    var floatArray = new Float32Array(TypedArray_copy(data, size, offset));
    Module.ctx.bufferSubData(target, offset, floatArray);
  },

  glIsBuffer: function(buffer) {
    var fb = GL.hashtable("buffer").get(buffer);
    if (typeof(fb) == 'undefined') {
      return false;
    }
    return Module.ctx.isBuffer(fb);
  },

  glGenRenderbuffers: function(n, renderbuffers) {
    for (var i = 0; i < n; i++) {
      var id = GL.hashtable("renderbuffer").add(Module.ctx.createRenderbuffer());
      {{{ makeSetValue('renderbuffers', 'i*4', 'id', 'i32') }}};
    }
  },

  glDeleteRenderbuffers: function(n, renderbuffers) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('renderbuffers', 'i*4', 'i32') }}};
      Module.ctx.deleteRenderbuffer(GL.hashtable("renderbuffer").get(id));
      GL.hashtable("renderbuffer").remove(id);
    }
  },

  glBindRenderbuffer: function(target, renderbuffer) {
    Module.ctx.bindRenderbuffer(target, GL.hashtable("renderbuffer").get(renderbuffer));
  },

  glGetRenderbufferParameteriv: function(target, pname, params) {
    {{{ makeSetValue('params', '0', 'Module.ctx.getRenderbufferParameter(target, pname)', 'i32') }}};
  },

  glIsRenderbuffer: function(renderbuffer) {
    var fb = GL.hashtable("renderbuffer").get(renderbuffer);
    if (typeof(fb) == 'undefined') {
      return false;
    }
    return Module.ctx.isRenderbuffer(fb);
  },

  glGetUniformLocation: function(program, name) {
    name = Pointer_stringify(name);
    var loc = Module.ctx.getUniformLocation(GL.hashtable("program").get(program), name);
    if (!loc) return -1;
    return GL.hashtable("uniform").add(loc);
  },

  glUniform1f: function(Location, v0) {
    Location = GL.hashtable("uniform").get(Location);
    Module.ctx.uniform1f(Location, v0);
  },

  glUniform2f: function(Location, v0, v1) {
    Location = GL.hashtable("uniform").get(Location);
    Module.ctx.uniform2f(Location, v0, v1);
  },

  glUniform3f: function(Location, v0, v1, v2) {
    Location = GL.hashtable("uniform").get(Location);
    Module.ctx.uniform3f(Location, v0, v1, v2);
  },

  glUniform4f: function(Location, v0, v1, v2, v3) {
    Location = GL.hashtable("uniform").get(Location);
    Module.ctx.uniform4f(Location, v0, v1, v2, v3);
  },

  glUniform1i: function(Location, v0) {
    Location = GL.hashtable("uniform").get(Location);
    Module.ctx.uniform1i(Location, v0);
  },

  glUniform2i: function(Location, v0, v1) {
    Location = GL.hashtable("uniform").get(Location);
    Module.ctx.uniform2i(Location, v0, v1);
  },

  glUniform3i: function(Location, v0, v1, v2) {
    Location = GL.hashtable("uniform").get(Location);
    Module.ctx.uniform3i(Location, v0, v1, v2);
  },

  glUniform4i: function(Location, v0, v1, v2, v3) {
    Location = GL.hashtable("uniform").get(Location);
    Module.ctx.uniform4i(Location, v0, v1, v2, v3);
  },

  glUniform1fv: function(Location, count, value) {
    Location = GL.hashtable("uniform").get(Location);
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform1fv(Location, value);
  },

  glUniform2fv: function(Location, count, value) {
    Location = GL.hashtable("uniform").get(Location);
    count *= 2;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform2fv(Location, value);
  },

  glUniform3fv: function(Location, count, value) {
    Location = GL.hashtable("uniform").get(Location);
    count *= 3;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform3fv(Location, value);
  },

  glUniform4fv: function(Location, count, value) {
    Location = GL.hashtable("uniform").get(Location);
    count *= 4;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform4fv(Location, value);
  },

  glUniformMatrix2fv: function(Location, count, transpose, value) {
    Location = GL.hashtable("uniform").get(Location);
    count *= 4;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniformMatrix2fv(Location, transpose, value);
  },

  glUniformMatrix3fv: function(Location, count, transpose, value) {
    Location = GL.hashtable("uniform").get(Location);
    count *= 9;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniformMatrix3fv(Location, transpose, value);
  },

  glUniformMatrix4fv: function(Location, count, transpose, value) {
    Location = GL.hashtable("uniform").get(Location);
    count *= 16;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniformMatrix4fv(Location, transpose, value);
  },

  glBindBuffer: function(target, buffer) {
    Module.ctx.bindBuffer(target, GL.hashtable("buffer").get(buffer));
  },

  glVertexAttrib1fv: function(index, v) {
    v = new Float32Array(TypedArray_copy(v, 1*4)); // TODO: optimize
    Module.ctx.vertexAttrib1fv(index, v);
  },

  glVertexAttrib2fv: function(index, v) {
    v = new Float32Array(TypedArray_copy(v, 2*4)); // TODO: optimize
    Module.ctx.vertexAttrib2fv(index, v);
  },

  glVertexAttrib3fv: function(index, v) {
    v = new Float32Array(TypedArray_copy(v, 3*4)); // TODO: optimize
    Module.ctx.vertexAttrib3fv(index, v);
  },

  glVertexAttrib4fv: function(index, v) {
    v = new Float32Array(TypedArray_copy(v, 4*4)); // TODO: optimize
    Module.ctx.vertexAttrib4fv(index, v);
  },

  glGetAttribLocation: function(program, name) {
    program = GL.hashtable("program").get(program);
    name = Pointer_stringify(name);
    return Module.ctx.getAttribLocation(program, name);
  },

  glCreateShader: function(shaderType) {
    var shader = Module.ctx.createShader(shaderType);
    return GL.hashtable("shader").add(shader);
  },

  glDeleteShader: function(shader) {
    Module.ctx.deleteShader(GL.hashtable("shader").get(shader));
  },

  glDetachShader: function(program, shader) {
    Module.ctx.detachShader(GL.hashtable("program").get(program),
                            GL.hashtable("shader").get(shader));
  },

  glGetAttachedShaders: function(program, maxCount, count, shaders) {
    var result = Module.ctx.getAttachedShaders(GL.hashtable("program").get(program));
    var len = result.length;
    if (len > maxCount) {
      len = maxCount;
    }
    {{{ makeSetValue('count', '0', 'len', 'i32') }}};
    for (var i = 0; i < len; ++i) {
      {{{ makeSetValue('shaders', 'i*4', 'GL.hashtable("shader").get(result[i])', 'i32') }}};
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
    Module.ctx.shaderSource(GL.hashtable("shader").get(shader), source);
  },

  glGetShaderSource: function(shader, bufsize, length, source) {
    var result = Module.ctx.getShaderSource(GL.hashtable("shader").get(shader));
    result.slice(0, bufsize - 1);
    writeStringToMemory(result, source);
    if (length) {
      {{{ makeSetValue('length', '0', 'result.length', 'i32') }}};
    }
  },

  glCompileShader: function(shader) {
    Module.ctx.compileShader(GL.hashtable("shader").get(shader));
  },

  glGetShaderInfoLog: function(shader, maxLength, length, infoLog) {
    var log = Module.ctx.getShaderInfoLog(GL.hashtable("shader").get(shader));
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
    {{{ makeSetValue('p', '0', 'Module.ctx.getShaderParameter(GL.hashtable("shader").get(shader), pname)', 'i32') }}};
  },

  glGetProgramiv : function(program, pname, p) {
    {{{ makeSetValue('p', '0', 'Module.ctx.getProgramParameter(GL.hashtable("program").get(program), pname)', 'i32') }}};
  },

  glIsShader: function(shader) {
    var fb = GL.hashtable("shader").get(shader);
    if (typeof(fb) == 'undefined') {
      return false;
    }
    return Module.ctx.isShader(fb);
  },

  glCreateProgram: function() {
    return GL.hashtable("program").add(Module.ctx.createProgram());
  },

  glDeleteProgram: function(program) {
    Module.ctx.deleteProgram(GL.hashtable("program").get(program));
  },

  glAttachShader: function(program, shader) {
    Module.ctx.attachShader(GL.hashtable("program").get(program),
                            GL.hashtable("shader").get(shader));
  },

  glGetShaderPrecisionFormat: function(shaderType, precisionType, range, precision) {
    var result = Module.ctx.getShaderPrecisionFormat(shaderType, precisionType);
    {{{ makeSetValue('range', '0', 'result.rangeMin', 'i32') }}};
    {{{ makeSetValue('range', '4', 'result.rangeMax', 'i32') }}};
    {{{ makeSetValue('precision', '0', 'result.precision', 'i32') }}};
  },

  glLinkProgram: function(program) {
    Module.ctx.linkProgram(GL.hashtable("program").get(program));
  },

  glGetProgramInfoLog: function(program, maxLength, length, infoLog) {
    var log = Module.ctx.getProgramInfoLog(GL.hashtable("program").get(program));
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
    Module.ctx.useProgram(GL.hashtable("program").get(program));
  },

  glValidateProgram: function(program) {
    Module.ctx.validateProgram(GL.hashtable("program").get(program));
  },

  glIsProgram: function(program) {
    var fb = GL.hashtable("program").get(program);
    if (typeof(fb) == 'undefined') {
      return false;
    }
    return Module.ctx.isProgram(fb);
  },

  glBindAttribLocation: function(program, index, name) {
    name = Pointer_stringify(name);
    Module.ctx.bindAttribLocation(GL.hashtable("program").get(program), index, name);
  },

  glBindFramebuffer: function(target, framebuffer) {
    Module.ctx.bindFramebuffer(target, GL.hashtable("framebuffer").get(framebuffer));
  },

  glGenFramebuffers: function(n, ids) {
    for (var i = 0; i < n; ++i) {
      var fb = GL.hashtable("framebuffer").add(Module.ctx.createFramebuffer());
      {{{ makeSetValue('ids', 'i*4', 'fb', 'i32') }}};
    }
  },

  glDeleteFramebuffers: function(n, framebuffers) {
    for (var i = 0; i < n; ++i) {
      var fb = GL.hashtable("framebuffer").get({{{ makeGetValue('framebuffers', 'i*4', 'i32' ) }}});
      Module.ctx.deleteFramebuffer(fb);
    }
  },

  glFramebufferRenderbuffer: function(target, attachment, renderbuffertarget, renderbuffer) {
    Module.ctx.framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                       GL.hashtable("renderbuffer").get(renderbuffer));
  },

  glFramebufferTexture2D: function(target, attachment, textarget, texture, level) {
    Module.ctx.framebufferTexture2D(target, attachment, textarget,
                                    GL.hashtable("texture").get(texture), level);
  },

  glGetFramebufferAttachmentParameteriv: function(target, attachment, pname, params) {
    var result = Module.ctx.getFramebufferAttachmentParameter(target, attachment, pname);
    {{{ makeSetValue('params', '0', 'params', 'i32') }}};
  },

  glIsFramebuffer: function(framebuffer) {
    var fb = GL.hashtable("framebuffer").get(framebuffer);
    if (typeof(fb) == 'undefined') {
      return false;
    }
    return Module.ctx.isFramebuffer(fb);
  },

};

// Simple pass-through functions
[[0, 'shadeModel fogi fogfv getError finish flush'],
 [1, 'clearDepth depthFunc enable disable frontFace cullFace clear enableVertexAttribArray disableVertexAttribArray lineWidth clearStencil depthMask stencilMask stencilMaskSeparate checkFramebufferStatus generateMipmap activeTexture'],
 [2, 'pixelStorei'],
 [3, 'texParameteri texParameterf drawArrays vertexAttrib2f'],
 [4, 'viewport clearColor scissor vertexAttrib3f colorMask drawElements renderbufferStorage'],
 [5, 'vertexAttrib4f'],
 [6, 'vertexAttribPointer'],
 [8, 'copyTexImage2D copyTexSubImage2D']].forEach(function(data) {
  var num = data[0];
  var names = data[1];
  var args = range(num).map(function(i) { return 'x' + i }).join(', ');
  var stub = '(function(' + args + ') { ' + (num > 0 ? 'Module.ctx.NAME(' + args + ')' : '') + ' })';
  names.split(' ').forEach(function(name_) {
    var cName = 'gl' + name_[0].toUpperCase() + name_.substr(1);
    assert(!(cName in LibraryGL), "Cannot reimplement the existing function " + cName);
    LibraryGL[cName] = eval(stub.replace('NAME', name_));
  });
});

autoAddDeps(LibraryGL, '$GL');
mergeInto(LibraryManager.library, LibraryGL);

