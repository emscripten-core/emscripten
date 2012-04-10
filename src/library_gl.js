/*
 * GL support. See https://github.com/kripken/emscripten/wiki/OpenGL-support
 * for current status.
 */

var LibraryGL = {
  $GL: {
    counter: 1,
    packAlignment: 4,   // default alignment is 4 bytes
    unpackAlignment: 4, // default alignment is 4 bytes
    arrayBufferBound: false,
    elementArrayBufferBound: false,
    vertexAttribPointers: {},
    buffers: {},
    programs: {},
    framebuffers: {},
    renderbuffer: {},
    textures: {},
    uniforms: {},
    shaders: {},

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

    computeImageSize: function(width, height, sizePerPixel, alignment) {
      function roundedToNextMultipleOf(x, y) {
        return Math.floor((x + y - 1) / y) * y
      }
      var plainRowSize = width * sizePerPixel;
      var alignedRowSize = roundedToNextMultipleOf(plainRowSize, alignment);
      return (height <= 0) ? 0 :
               ((height - 1) * alignedRowSize + plainRowSize);
    },

    recordVertexAttribPointer: function(index, size, type, normalized,
                                        stride, pointer) {
      this.vertexAttribPointers[index] = {
        size: size,
        type: type,
        normalized: normalized,
        stride: stride,
        pointer: pointer
      };
    },

    setVertexAttribPointers: function(indices, indexSize, count) {
      function computeArraySize(elem, indices, indexSize, count) {
        var maxIndex = 0;
        for (var i = 0; i < count; ++i) {
          var index;
          if (indexSize == 1) {
            index = {{{ makeGetValue('indices', 'i*indexSize' , 'i8') }}};
          } else if (indexSize == 2) {
            index = {{{ makeGetValue('indices', 'i*indexSize' , 'i16') }}};
          } else if (indexSize == 4) {
            index = {{{ makeGetValue('indices', 'i*indexSize' , 'i32') }}};
          }
          if (index > maxIndex) {
            maxIndex = index;
          }
        }
        // The byte size of elements in the array equals the number of vertices (maxIndex + 1)
        // times the size of each element times the stride, but since stride is a byte offset,
        // we need to devide again by the size of each element, hence the below formula.
        // This is why we can get away here without computing the size of each element.
        return (maxIndex + 1) * elem.stride;
      }

      for (var i in GL.vertexAttribPointers) {
        var elem = GL.vertexAttribPointers[i];
        var buffer = Module.ctx.createBuffer();
        Module.ctx.bindBuffer(0x8892 /* GL_ARRAY_BUFFER */, buffer);
        Module.ctx.bufferData(0x8892,
            new Uint8Array(Array_copy(elem.pointer,
                computeArraySize(elem, indices, indexSize, count))),
            0x88E0 /* GL_STREAM_DRAW */);
        Module.ctx.vertexAttribPointer(i, elem.size, elem.type,
            elem.normalized, elem.stride, 0);
      }
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
          break;
        case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
        case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
        case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
          sizePerPixel = 2;
          break;
        default:
          throw 'Invalid type (' + type + ') passed to glTexImage2D';
      }
      var bytes = GL.computeImageSize(width, height, sizePerPixel, GL.unpackAlignment);
      pixels = new Uint8Array(Array_copy(pixels, bytes));
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
          break;
        case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
        case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
        case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
          sizePerPixel = 2;
          break;
        default:
          throw 'Invalid type (' + type + ') passed to glTexSubImage2D';
      }
      var bytes = GL.computeImageSize(width, height, sizePerPixel, GL.unpackAlignment);
      pixels = new Uint8Array(Array_copy(pixels, bytes));
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
    var floatArray = new Float32Array(TypedArray_copy(data, size, offset));
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
    value = new Int32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform1iv(location, value);
  },

  glUniform2iv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 2;
    value = new Int32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform2iv(location, value);
  },

  glUniform3iv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 3;
    value = new Int32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform3iv(location, value);
  },

  glUniform4iv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 4;
    value = new Int32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform4iv(location, value);
  },

  glUniform1fv: function(location, count, value) {
    location = GL.uniforms[location];
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform1fv(location, value);
  },

  glUniform2fv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 2;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform2fv(location, value);
  },

  glUniform3fv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 3;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform3fv(location, value);
  },

  glUniform4fv: function(location, count, value) {
    location = GL.uniforms[location];
    count *= 4;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniform4fv(location, value);
  },

  glUniformMatrix2fv: function(location, count, transpose, value) {
    location = GL.uniforms[location];
    count *= 4;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniformMatrix2fv(location, transpose, value);
  },

  glUniformMatrix3fv: function(location, count, transpose, value) {
    location = GL.uniforms[location];
    count *= 9;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniformMatrix3fv(location, transpose, value);
  },

  glUniformMatrix4fv: function(location, count, transpose, value) {
    location = GL.uniforms[location];
    count *= 16;
    value = new Float32Array(TypedArray_copy(value, count*4)); // TODO: optimize
    Module.ctx.uniformMatrix4fv(location, transpose, value);
  },

  glBindBuffer: function(target, buffer) {
    Module.ctx.bindBuffer(target, GL.buffers[buffer]);
    if (target == 0x8892 /* GL_ARRAY_BUFFER */) {
      GL.arrayBufferBound = true;
    } else if (target == 0x8893 /* GL_ELEMENT_ARRAY_BUFFER */) {
      GL.elementArrayBufferBound = true;
    }
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

  glVertexAttribPointer: function(index, size, type, normalized, stride, pointer) {
    if (GL.arrayBufferBound) {
      Module.ctx.vertexAttribPointer(index, size, type, normalized, stride, pointer);
      return;
    }

    // If there is no buffer bound, we should create and bind our own buffer
    GL.recordVertexAttribPointer(index, size, type, normalized, stride, pointer);
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

  glDrawElements: function(mode, count, type, indices) {
    if (count > 0) {
      var sizePerElem;
      switch (type) {
      case 0x1401 /* GL_UNSIGNED_BYTE */:
        sizePerElem = 1;
        break;
      case 0x1403 /* GL_UNSIGNED_SHORT */:
        sizePerElem = 2;
        break;
      case 0x1405 /* GL_UNSIGNED_INT */:
        sizePerElem = 4;
        break;
      default:
        throw "Invalid type (" + type + ") passed to glDrawElements";
      }
      if (GL.elementArrayBufferBound) {
        // A buffer has already been bound for the elements, and indices is an
        // index into that buffer.
        GL.setVertexAttribPointers(indices, sizePerElem, count);
        Module.ctx.drawElements(mode, count, type, indices);
      } else {
        var buffer = Module.ctx.createBuffer();
        Module.ctx.bindBuffer(0x8893 /* GL_ELEMENT_ARRAY_BUFFER */, buffer);
        Module.ctx.bufferData(0x8893,
            new Uint8Array(Array_copy(indices, count * sizePerElem)),
            0x88E0 /* GL_STREAM_DRAW */);
        GL.setVertexAttribPointers(indices, sizePerElem, count);
        Module.ctx.drawElements(mode, count, type, 0);
      }
    }
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
  }
};

// Simple pass-through functions
[[0, 'shadeModel fogi fogfv getError finish flush'],
 [1, 'clearDepth depthFunc enable disable frontFace cullFace clear enableVertexAttribArray disableVertexAttribArray lineWidth clearStencil depthMask stencilMask stencilMaskSeparate checkFramebufferStatus generateMipmap activeTexture blendEquation'],
 [2, 'blendFunc blendEquationSeparate'],
 [3, 'texParameteri texParameterf drawArrays vertexAttrib2f'],
 [4, 'viewport clearColor scissor vertexAttrib3f colorMask renderbufferStorage blendFuncSeparate'],
 [5, 'vertexAttrib4f'],
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

