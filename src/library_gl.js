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
        case 0x1403 /* GL_UNSIGNED_SHORT */:
          if (format == 0x1902 /* GL_DEPTH_COMPONENT */) {
            sizePerPixel = 2;
          } else {
            throw 'Invalid format (' + format + ')';
          }
          break;
        case 0x1405 /* GL_UNSIGNED_INT */:
          if (format == 0x1902 /* GL_DEPTH_COMPONENT */) {
            sizePerPixel = 4;
          } else {
            throw 'Invalid format (' + format + ')';
          }
          break;
        case 0x84FA /* UNSIGNED_INT_24_8_WEBGL */:
          sizePerPixel = 4;
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
      } else if (type == 0x1405 /* GL_UNSIGNED_INT */ || type == 0x84FA /* UNSIGNED_INT_24_8_WEBGL */) {
        pixels = {{{ makeHEAPView('U32', 'pixels', 'pixels+bytes') }}};
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
#if GL_ASSERTIONS
        GL.validateVertexAttribPointer(cb.size, cb.type, cb.stride, 0);
#endif
        Module.ctx.vertexAttribPointer(i, cb.size, cb.type, cb.normalized, cb.stride, 0);
      }
    },

    postDrawHandleClientVertexAttribBindings: function() {
      if (GL.resetBufferBinding) {
        Module.ctx.bindBuffer(Module.ctx.ARRAY_BUFFER, GL.buffers[GL.currArrayBuffer]);
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

    // In WebGL, extensions must be explicitly enabled to be active, see http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.14.14
    // In GLES2, all extensions are enabled by default without additional operations. Init all extensions we need to give to GLES2 user
    // code here, so that GLES2 code can operate without changing behavior.
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

      // Detect the presence of a few extensions manually, this GL interop layer itself will need to know if they exist. 
      GL.compressionExt = Module.ctx.getExtension('WEBGL_compressed_texture_s3tc') ||
                          Module.ctx.getExtension('MOZ_WEBGL_compressed_texture_s3tc') ||
                          Module.ctx.getExtension('WEBKIT_WEBGL_compressed_texture_s3tc');

      GL.anisotropicExt = Module.ctx.getExtension('EXT_texture_filter_anisotropic') ||
                          Module.ctx.getExtension('MOZ_EXT_texture_filter_anisotropic') ||
                          Module.ctx.getExtension('WEBKIT_EXT_texture_filter_anisotropic');

      GL.floatExt = Module.ctx.getExtension('OES_texture_float');

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
                                             "EXT_frag_depth", "EXT_sRGB", "WEBGL_draw_buffers", "WEBGL_shared_resources" ];

      function shouldEnableAutomatically(extension) {
        for(var i in automaticallyEnabledExtensions) {
          var include = automaticallyEnabledExtensions[i];
          if (ext.indexOf(include) != -1) {
            return true;
          }
        }
        return false;
      }

      var extensions = Module.ctx.getSupportedExtensions();
      for(var e in extensions) {
        var ext = extensions[e].replace('MOZ_', '').replace('WEBKIT_', '');
        if (automaticallyEnabledExtensions.indexOf(ext) != -1) {
          Module.ctx.getExtension(ext); // Calling .getExtension enables that extension permanently, no need to store the return value to be enabled.
        }
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
      GL.uniformTable[program] = {};
      var ptable = GL.uniformTable[program];
      // A program's uniformTable maps the string name of an uniform to an integer location of that uniform.
      // The global GL.uniforms map maps integer locations to WebGLUniformLocations.
      var numUniforms = Module.ctx.getProgramParameter(p, Module.ctx.ACTIVE_UNIFORMS);
      for (var i = 0; i < numUniforms; ++i) {
        var u = Module.ctx.getActiveUniform(p, i);

        var name = u.name;
        // Strip off any trailing array specifier we might have got, e.g. "[0]".
        if (name.indexOf(']', name.length-1) !== -1) {
          var ls = name.lastIndexOf('[');
          name = name.slice(0, ls);
        }

        // Optimize memory usage slightly: If we have an array of uniforms, e.g. 'vec3 colors[3];', then 
        // only store the string 'colors' in ptable, and 'colors[0]', 'colors[1]' and 'colors[2]' will be parsed as 'colors'+i.
        // Note that for the GL.uniforms table, we still need to fetch the all WebGLUniformLocations for all the indices.
        var loc = Module.ctx.getUniformLocation(p, name);
        var id = GL.getNewId(GL.uniforms);
        ptable[name] = [u.size, id];
        GL.uniforms[id] = loc;

        for (var j = 1; j < u.size; ++j) {
          var n = name + '['+j+']';
          loc = Module.ctx.getUniformLocation(p, n);
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
        var exts = Module.ctx.getSupportedExtensions();
        var gl_exts = [];
        for (i in exts) {
          gl_exts.push(exts[i]);
          gl_exts.push("GL_" + exts[i]);
        }
        return allocate(intArrayFromString(gl_exts.join(' ')), 'i8', ALLOC_NORMAL); // XXX this leaks! TODO: Cache all results like this in library_gl.js to be clean and nice and avoid leaking.
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
      case 0x86A2: // GL_NUM_COMPRESSED_TEXTURE_FORMATS
        // WebGL doesn't have GL_NUM_COMPRESSED_TEXTURE_FORMATS (it's obsolete since GL_COMPRESSED_TEXTURE_FORMATS returns a JS array that can be queried for length),
        // so implement it ourselves to allow C++ GLES2 code get the length.
        var formats = Module.ctx.getParameter(0x86A3 /*GL_COMPRESSED_TEXTURE_FORMATS*/);
        {{{ makeSetValue('p', '0', 'formats.length', 'i32') }}};
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
          {{{ makeSetValue('p', '0', 'result.name | 0', 'i32') }}};
        } else if (result instanceof WebGLProgram) {
          {{{ makeSetValue('p', '0', 'result.name | 0', 'i32') }}};
        } else if (result instanceof WebGLFramebuffer) {
          {{{ makeSetValue('p', '0', 'result.name | 0', 'i32') }}};
        } else if (result instanceof WebGLRenderbuffer) {
          {{{ makeSetValue('p', '0', 'result.name | 0', 'i32') }}};
        } else if (result instanceof WebGLTexture) {
          {{{ makeSetValue('p', '0', 'result.name | 0', 'i32') }}};
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
          {{{ makeSetValue('p', '0', 'result.name | 0', 'float') }}};
        } else if (result instanceof WebGLProgram) {
          {{{ makeSetValue('p', '0', 'result.name | 0', 'float') }}};
        } else if (result instanceof WebGLFramebuffer) {
          {{{ makeSetValue('p', '0', 'result.name | 0', 'float') }}};
        } else if (result instanceof WebGLRenderbuffer) {
          {{{ makeSetValue('p', '0', 'result.name | 0', 'float') }}};
        } else if (result instanceof WebGLTexture) {
          {{{ makeSetValue('p', '0', 'result.name | 0', 'float') }}};
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
      var texture = Module.ctx.createTexture();
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
      Module.ctx.deleteTexture(texture);
      texture.name = 0;
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.textures, texture, 'glBindTexture', 'texture');
#endif
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
      var buffer = Module.ctx.createBuffer();
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

      Module.ctx.deleteBuffer(buffer);
      buffer.name = 0;
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
      var renderbuffer = Module.ctx.createRenderbuffer();
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
      Module.ctx.deleteRenderbuffer(renderbuffer);
      renderbuffer.name = 0;
      GL.renderbuffers[id] = null;
    }
  },

  glBindRenderbuffer__sig: 'vii',
  glBindRenderbuffer: function(target, renderbuffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.renderbuffers, renderbuffer, 'glBindRenderbuffer', 'renderbuffer');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformfv', 'program');
    GL.validateGLObjectID(GL.uniforms, location, 'glGetUniformfv', 'location');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformiv', 'program');
    GL.validateGLObjectID(GL.uniforms, location, 'glGetUniformiv', 'location');
#endif
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

    var ptable = GL.uniformTable[program];
    if (!ptable) {
      return -1;
    }
    var uniformInfo = ptable[name]; // returns pair [ dimension_of_uniform_array, uniform_location ]
    if (uniformInfo && arrayOffset < uniformInfo[0]) { // Check if user asked for an out-of-bounds element, i.e. for 'vec4 colors[3];' user could ask for 'colors[10]' which should return -1.
      return uniformInfo[1]+arrayOffset;
    } else {
      return -1;
    }
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveUniform', 'program');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1f', 'location');
#endif
    location = GL.uniforms[location];
    Module.ctx.uniform1f(location, v0);
  },

  glUniform2f__sig: 'viff',
  glUniform2f: function(location, v0, v1) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2f', 'location');
#endif
    location = GL.uniforms[location];
    Module.ctx.uniform2f(location, v0, v1);
  },

  glUniform3f__sig: 'vifff',
  glUniform3f: function(location, v0, v1, v2) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3f', 'location');
#endif
    location = GL.uniforms[location];
    Module.ctx.uniform3f(location, v0, v1, v2);
  },

  glUniform4f__sig: 'viffff',
  glUniform4f: function(location, v0, v1, v2, v3) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4f', 'location');
#endif
    location = GL.uniforms[location];
    Module.ctx.uniform4f(location, v0, v1, v2, v3);
  },

  glUniform1i__sig: 'vii',
  glUniform1i: function(location, v0) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1i', 'location');
#endif
    location = GL.uniforms[location];
    Module.ctx.uniform1i(location, v0);
  },

  glUniform2i__sig: 'viii',
  glUniform2i: function(location, v0, v1) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2i', 'location');
#endif
    location = GL.uniforms[location];
    Module.ctx.uniform2i(location, v0, v1);
  },

  glUniform3i__sig: 'viiii',
  glUniform3i: function(location, v0, v1, v2) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3i', 'location');
#endif
    location = GL.uniforms[location];
    Module.ctx.uniform3i(location, v0, v1, v2);
  },

  glUniform4i__sig: 'viiiii',
  glUniform4i: function(location, v0, v1, v2, v3) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4i', 'location');
#endif
    location = GL.uniforms[location];
    Module.ctx.uniform4i(location, v0, v1, v2, v3);
  },

  glUniform1iv__sig: 'viii',
  glUniform1iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1iv', 'location');
#endif
    location = GL.uniforms[location];
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform1iv(location, value);
  },

  glUniform2iv__sig: 'viii',
  glUniform2iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2iv', 'location');
#endif
    location = GL.uniforms[location];
    count *= 2;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform2iv(location, value);
  },

  glUniform3iv__sig: 'viii',
  glUniform3iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3iv', 'location');
#endif
    location = GL.uniforms[location];
    count *= 3;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform3iv(location, value);
  },

  glUniform4iv__sig: 'viii',
  glUniform4iv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4iv', 'location');
#endif
    location = GL.uniforms[location];
    count *= 4;
    value = {{{ makeHEAPView('32', 'value', 'value+count*4') }}};
    Module.ctx.uniform4iv(location, value);
  },

  glUniform1fv__sig: 'viii',
  glUniform1fv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1fv', 'location');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2fv', 'location');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3fv', 'location');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4fv', 'location');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix2fv', 'location');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix3fv', 'location');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix4fv', 'location');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.buffers, buffer, 'glBindBuffer', 'buffer');
#endif
    var bufferObj = buffer ? GL.buffers[buffer] : null;

    if (target == Module.ctx.ARRAY_BUFFER) {
      GL.currArrayBuffer = buffer;
    } else if (target == Module.ctx.ELEMENT_ARRAY_BUFFER) {
      GL.currElementArrayBuffer = buffer;
    }

    Module.ctx.bindBuffer(target, bufferObj);
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveAttrib', 'program');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetAttachedShaders', 'program');
#endif
    var result = Module.ctx.getAttachedShaders(GL.programs[program]);
    var len = result.length;
    if (len > maxCount) {
      len = maxCount;
    }
    {{{ makeSetValue('count', '0', 'len', 'i32') }}};
    for (var i = 0; i < len; ++i) {
      var id = GL.shaders.indexOf(result[i]);
      assert(id !== -1, 'shader not bound to local id');
      {{{ makeSetValue('shaders', 'i*4', 'id', 'i32') }}};
    }
  },

  glShaderSource__sig: 'viiii',
  glShaderSource: function(shader, count, string, length) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glShaderSource', 'shader');
#endif
    var source = GL.getSource(shader, count, string, length);
    Module.ctx.shaderSource(GL.shaders[shader], source);
  },

  glGetShaderSource__sig: 'viiii',
  glGetShaderSource: function(shader, bufSize, length, source) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderSource', 'shader');
#endif
    var result = Module.ctx.getShaderSource(GL.shaders[shader]);
    result = result.slice(0, Math.max(0, bufSize - 1));
    writeStringToMemory(result, source);
    if (length) {
      {{{ makeSetValue('length', '0', 'result.length', 'i32') }}};
    }
  },

  glCompileShader__sig: 'vi',
  glCompileShader: function(shader) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glCompileShader', 'shader');
#endif
    Module.ctx.compileShader(GL.shaders[shader]);
  },

  glGetShaderInfoLog__sig: 'viiii',
  glGetShaderInfoLog: function(shader, maxLength, length, infoLog) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderInfoLog', 'shader');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.shaders, shader, 'glGetShaderiv', 'shader');
#endif
    if (pname == 0x8B84) { // GL_INFO_LOG_LENGTH
      {{{ makeSetValue('p', '0', 'Module.ctx.getShaderInfoLog(GL.shaders[shader]).length + 1', 'i32') }}};
    } else {
      {{{ makeSetValue('p', '0', 'Module.ctx.getShaderParameter(GL.shaders[shader], pname)', 'i32') }}};
    }
  },

  glGetProgramiv__sig: 'viii',
  glGetProgramiv : function(program, pname, p) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramiv', 'program');
#endif
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
    var program = Module.ctx.createProgram();
    program.name = id;
    GL.programs[id] = program;
    return id;
  },

  glDeleteProgram__sig: 'vi',
  glDeleteProgram: function(program) {
    var program = GL.programs[program];
    Module.ctx.deleteProgram(program);
    program.name = 0;
    GL.programs[program] = null;
    GL.uniformTable[program] = null;
  },

  glAttachShader__sig: 'vii',
  glAttachShader: function(program, shader) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glAttachShader', 'program');
    GL.validateGLObjectID(GL.shaders, shader, 'glAttachShader', 'shader');
#endif
    Module.ctx.attachShader(GL.programs[program],
                            GL.shaders[shader]);
  },

  glDetachShader__sig: 'vii',
  glDetachShader: function(program, shader) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glDetachShader', 'program');
    GL.validateGLObjectID(GL.shaders, shader, 'glDetachShader', 'shader');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glLinkProgram', 'program');
#endif
    Module.ctx.linkProgram(GL.programs[program]);
    GL.uniformTable[program] = {}; // uniforms no longer keep the same names after linking
    GL.populateUniformTable(program);
  },

  glGetProgramInfoLog__sig: 'viiii',
  glGetProgramInfoLog: function(program, maxLength, length, infoLog) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramInfoLog', 'program');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glUseProgram', 'program');
#endif
    Module.ctx.useProgram(program ? GL.programs[program] : null);
  },

  glValidateProgram__sig: 'vi',
  glValidateProgram: function(program) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glValidateProgram', 'program');
#endif
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
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glBindAttribLocation', 'program');
#endif
    name = Pointer_stringify(name);
    Module.ctx.bindAttribLocation(GL.programs[program], index, name);
  },

  glBindFramebuffer__sig: 'vii',
  glBindFramebuffer: function(target, framebuffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.framebuffers, framebuffer, 'glBindFramebuffer', 'framebuffer');
#endif
    Module.ctx.bindFramebuffer(target, framebuffer ? GL.framebuffers[framebuffer] : null);
  },

  glGenFramebuffers__sig: 'vii',
  glGenFramebuffers: function(n, ids) {
    for (var i = 0; i < n; ++i) {
      var id = GL.getNewId(GL.framebuffers);
      var framebuffer = Module.ctx.createFramebuffer();
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
      Module.ctx.deleteFramebuffer(framebuffer);
      framebuffer.name = 0;
      GL.framebuffers[id] = null;
    }
  },

  glFramebufferRenderbuffer__sig: 'viiii',
  glFramebufferRenderbuffer: function(target, attachment, renderbuffertarget, renderbuffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.renderbuffers, renderbuffer, 'glFramebufferRenderbuffer', 'renderbuffer');
#endif
    Module.ctx.framebufferRenderbuffer(target, attachment, renderbuffertarget,
                                       GL.renderbuffers[renderbuffer]);
  },

  glFramebufferTexture2D__sig: 'viiiii',
  glFramebufferTexture2D: function(target, attachment, textarget, texture, level) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.textures, texture, 'glFramebufferTexture2D', 'texture');
#endif
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

      var glEnable = _glEnable;
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
      _glDisable = function(cap) {
        if (GL.immediate.lastRenderer) GL.immediate.lastRenderer.cleanup();
        if (cap == 0x0B60 /* GL_FOG */) {
          GLEmulation.fogEnabled = false;
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
        console.log("glShaderSource: Input: \n" + source);
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
        console.log("glShaderSource: Output: \n" + source);
#endif
        Module.ctx.shaderSource(GL.shaders[shader], source);
      };

      var glCompileShader = _glCompileShader;
      _glCompileShader = function(shader) {
        Module.ctx.compileShader(GL.shaders[shader]);
#if GL_DEBUG
        if (!Module.ctx.getShaderParameter(GL.shaders[shader], Module.ctx.COMPILE_STATUS)) {
          Module.printErr('Failed to compile shader: ' + Module.ctx.getShaderInfoLog(GL.shaders[shader]));
          Module.printErr('Info: ' + JSON.stringify(GL.shaderInfos[shader]));
          Module.printErr('Original source: ' + GL.shaderOriginalSources[shader]);
          Module.printErr('Source: ' + GL.shaderSources[shader]);
          throw 'Shader compilation halt';
        }
#endif
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
        case 0x0de1: // GL_TEXTURE_2D - XXX not according to spec, and not in desktop GL, but works in some GLES1.x apparently, so support it
#if ASSERTIONS
          abort("GL_TEXTURE_2D is not a spec-defined capability for gl{Enable,Disable}ClientState.");
#endif
          // Fall through:
        case 0x8078: // GL_TEXTURE_COORD_ARRAY
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

  // See comment in GLEmulation.init()
#if FULL_ES2 == 0
  $GLImmediate__postset: 'GL.immediate.setupFuncs(); Browser.moduleContextCreatedCallbacks.push(function() { GL.immediate.init() });',
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

        this.insert = function(key, val) {
          if (this.contains(key|0)) return false;
          list.push([key, val]);
          return true;
        };

        var __contains_i;
        this.contains = function(key) {
          for (__contains_i = 0; __contains_i < list.length; ++__contains_i) {
            if (list[__contains_i][0] === key) return true;
          }
          return false;
        };

        var __get_i;
        this.get = function(key) {
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

          this.child = function(keyFrag) {
            if (!map.contains(keyFrag|0)) {
              map.insert(keyFrag|0, new CNLNode());
            }
            return map.get(keyFrag|0);
          };

          this.value = undefined;
          this.get = function() {
            return this.value;
          };

          this.set = function(val) {
            this.value = val;
          };
        }

        function CKeyView(root) {
          var cur;

          this.reset = function() {
            cur = root;
            return this;
          };
          this.reset();

          this.next = function(keyFrag) {
            cur = cur.child(keyFrag);
            return this;
          };

          this.get = function() {
            return cur.get();
          };

          this.set = function(val) {
            cur.set(val);
          };
        };

        var root;
        var staticKeyView;

        this.createKeyView = function() {
          return new CKeyView(root);
        }

        this.clear = function() {
          root = new CNLNode();
          staticKeyView = this.createKeyView();
        };
        this.clear();

        this.getStaticKeyView = function() {
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

        return Abort_NoSupport("Unsupported combiner op: 0x" + op.toString(16));
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

        this.traverseState = function(keyView) {
          keyView.next(this.mode);
          keyView.next(this.colorCombiner);
          keyView.next(this.alphaCombiner);
          keyView.next(this.colorCombiner);
          keyView.next(this.alphaScale);
          keyView.next(this.envColor[0]);
          keyView.next(this.envColor[1]);
          keyView.next(this.envColor[2]);
          keyView.next(this.envColor[3]);

          keyView.next(this.colorSrc[0]);
          keyView.next(this.colorSrc[1]);
          keyView.next(this.colorSrc[2]);

          keyView.next(this.alphaSrc[0]);
          keyView.next(this.alphaSrc[1]);
          keyView.next(this.alphaSrc[2]);

          keyView.next(this.colorOp[0]);
          keyView.next(this.colorOp[1]);
          keyView.next(this.colorOp[2]);

          keyView.next(this.alphaOp[0]);
          keyView.next(this.alphaOp[1]);
          keyView.next(this.alphaOp[2]);
        };
      }

      function CTexUnit() {
        this.env = new CTexEnv();
        this.enabled_tex1D   = false;
        this.enabled_tex2D   = false;
        this.enabled_tex3D   = false;
        this.enabled_texCube = false;

        this.traverseState = function(keyView) {
          var texUnitType = this.getTexType();
          keyView.next(texUnitType);
          if (!texUnitType) return;
          this.env.traverseState(keyView);
        };
      };

      // Class impls:
      CTexUnit.prototype.enabled = function() {
        return this.getTexType() != 0;
      }

      CTexUnit.prototype.genPassLines = function(passOutputVar, passInputVar, texUnitID) {
        if (!this.enabled()) {
          return ["vec4 " + passOutputVar + " = " + passInputVar + ";"];
        }

        return this.env.genPassLines(passOutputVar, passInputVar, texUnitID);
      }

      CTexUnit.prototype.getTexType = function() {
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

      CTexEnv.prototype.genPassLines = function(passOutputVar, passInputVar, texUnitID) {
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
            var line = [
              "vec4 " + passOutputVar,
              " = ",
                "vec4(",
                    colorVar + " * " + valToFloatLiteral(this.colorScale),
                    ", ",
                    alphaVar + " * " + valToFloatLiteral(this.alphaScale),
                ")",
              ";",
            ].join("");
            return [].concat(colorLines, alphaLines, [line]);
          }
        }

        return Abort_NoSupport("Unsupported TexEnv mode: 0x" + this.mode.toString(16));
      }

      CTexEnv.prototype.genCombinerLines = function(isColor, outputVar,
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

          assert(maxTexUnits > 0);

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
            var texUnit = s_texUnits[i];
            var enabled = texUnit.enabled();
            keyView.next(enabled);
            if (enabled) {
              texUnit.traverseState(keyView);
            }
          }
        },

        getTexUnitType: function(texUnitID) {
          assert(texUnitID >= 0 &&
                 texUnitID < s_texUnits.length);

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
              cur.enabled_tex1D = true;
              break;
            case GL_TEXTURE_2D:
              cur.enabled_tex2D = true;
              break;
            case GL_TEXTURE_3D:
              cur.enabled_tex3D = true;
              break;
            case GL_TEXTURE_CUBE_MAP:
              cur.enabled_texCube = true;
              break;
          }
        },

        hook_disable: function(cap) {
          var cur = getCurTexUnit();
          switch (cap) {
            case GL_TEXTURE_1D:
              cur.enabled_tex1D = false;
              break;
            case GL_TEXTURE_2D:
              cur.enabled_tex2D = false;
              break;
            case GL_TEXTURE_3D:
              cur.enabled_tex3D = false;
              break;
            case GL_TEXTURE_CUBE_MAP:
              cur.enabled_texCube = false;
              break;
          }
        },

        hook_texEnvf: function(target, pname, param) {
          if (target != GL_TEXTURE_ENV)
            return;

          var env = getCurTexUnit().env;
          switch (pname) {
            case GL_RGB_SCALE:
              env.colorScale = param;
              break;
            case GL_ALPHA_SCALE:
              env.alphaScale = param;
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
              env.mode = param;
              break;

            case GL_COMBINE_RGB:
              env.colorCombiner = param;
              break;
            case GL_COMBINE_ALPHA:
              env.alphaCombiner = param;
              break;

            case GL_SRC0_RGB:
              env.colorSrc[0] = param;
              break;
            case GL_SRC1_RGB:
              env.colorSrc[1] = param;
              break;
            case GL_SRC2_RGB:
              env.colorSrc[2] = param;
              break;

            case GL_SRC0_ALPHA:
              env.alphaSrc[0] = param;
              break;
            case GL_SRC1_ALPHA:
              env.alphaSrc[1] = param;
              break;
            case GL_SRC2_ALPHA:
              env.alphaSrc[2] = param;
              break;

            case GL_OPERAND0_RGB:
              env.colorOp[0] = param;
              break;
            case GL_OPERAND1_RGB:
              env.colorOp[1] = param;
              break;
            case GL_OPERAND2_RGB:
              env.colorOp[2] = param;
              break;

            case GL_OPERAND0_ALPHA:
              env.alphaOp[0] = param;
              break;
            case GL_OPERAND1_ALPHA:
              env.alphaOp[1] = param;
              break;
            case GL_OPERAND2_ALPHA:
              env.alphaOp[2] = param;
              break;

            case GL_RGB_SCALE:
              env.colorScale = param;
              break;
            case GL_ALPHA_SCALE:
              env.alphaScale = param;
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
                env.envColor[i] = param;
              }
              break
            }
            default:
              Module.printErr('WARNING: Unhandled `pname` in call to `glTexEnvfv`.');
          }
        },
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
    matrix: {},
    matrixStack: {},
    currentMatrix: 'm', // default is modelview
    tempMatrix: null,
    matricesModified: false,
    useTextureMatrix: false,

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
    NUM_ATTRIBUTES: 10, // Overwritten in init().
    MAX_TEXTURES: 7,    // Overwritten in init().

    totalEnabledClientAttributes: 0,
    enabledClientAttributes: [0, 0],
    clientAttributes: [], // raw data, including possible unneeded ones
    liveClientAttributes: [], // the ones actually alive in the current computation, sorted
    modifiedClientAttributes: false,
    clientActiveTexture: 0,
    clientColor: null,
    usedTexUnitList: [],
    fixedFunctionProgram: null,

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
        if (this.enabledClientAttributes[name]) {
          console.log("Warning: glTexCoord used after EnableClientState for TEXTURE_COORD_ARRAY for TEXTURE0. Disabling TEXTURE_COORD_ARRAY...");
        }
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
      var cacheMap = GL.immediate.rendererCache;
      var temp;
      var keyView = cacheMap.getStaticKeyView().reset();

      // By attrib state:
      for (var i = 0; i < attributes.length; i++) {
        var attribute = attributes[i];
        keyView.next(attribute.name).next(attribute.size).next(attribute.type);
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
      keyView.next(fogParam);

      // By cur program:
      keyView.next(GL.currProgram);
      if (!GL.currProgram) {
        GL.immediate.TexEnvJIT.traverseState(keyView);
      }

      // If we don't already have it, create it.
      if (!keyView.get()) {
#if GL_DEBUG
        Module.printErr('generating renderer for ' + JSON.stringify(attributes));
#endif
        keyView.set(this.createRenderer());
      }
      return keyView.get();
    },

    createRenderer: function(renderer) {
      var useCurrProgram = !!GL.currProgram;
      var hasTextures = false, textureSizes = [], textureTypes = [];
      for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
        var texAttribName = GL.immediate.TEXTURE0 + i;
        if (!GL.immediate.enabledClientAttributes[texAttribName])
          continue;

        if (!useCurrProgram) {
          assert(GL.immediate.TexEnvJIT.getTexUnitType(i) != 0, "GL_TEXTURE" + i + " coords are supplied, but that texture unit is disabled in the fixed-function pipeline.");
        }

        textureSizes[i] = GL.immediate.clientAttributes[texAttribName].size;
        textureTypes[i] = GL.immediate.clientAttributes[texAttribName].type;
        hasTextures = true;
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
          // For fixed-function shader generation.
          var uTexUnitPrefix = 'u_texUnit';
          var aTexCoordPrefix = 'a_texCoord';
          var vTexCoordPrefix = 'v_texCoord';
          var vPrimColor = 'v_color';
          var uTexMatrixPrefix = GL.immediate.useTextureMatrix ? 'u_textureMatrix' : null;

          if (useCurrProgram) {
            if (GL.shaderInfos[GL.programShaders[GL.currProgram][0]].type == Module.ctx.VERTEX_SHADER) {
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

            GL.immediate.TexEnvJIT.setGLSLVars(uTexUnitPrefix, vTexCoordPrefix, vPrimColor, uTexMatrixPrefix);
            var fsTexEnvPass = GL.immediate.TexEnvJIT.genAllPassLines('gl_FragColor', 2);

            var texUnitAttribList = '';
            var texUnitVaryingList = '';
            var texUnitUniformList = '';
            var vsTexCoordInits = '';
            this.usedTexUnitList = GL.immediate.TexEnvJIT.getUsedTexUnitList();
            for (var i = 0; i < this.usedTexUnitList.length; i++) {
              var texUnit = this.usedTexUnitList[i];
              texUnitAttribList += 'attribute vec4 ' + aTexCoordPrefix + texUnit + ';\n';
              texUnitVaryingList += 'varying vec4 ' + vTexCoordPrefix + texUnit + ';\n';
              texUnitUniformList += 'uniform sampler2D ' + uTexUnitPrefix + texUnit + ';\n';
              vsTexCoordInits += '  ' + vTexCoordPrefix + texUnit + ' = ' + aTexCoordPrefix + texUnit + ';\n';

              if (GL.immediate.useTextureMatrix) {
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

            this.vertexShader = Module.ctx.createShader(Module.ctx.VERTEX_SHADER);
            Module.ctx.shaderSource(this.vertexShader, vsSource);
            Module.ctx.compileShader(this.vertexShader);

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

            this.fragmentShader = Module.ctx.createShader(Module.ctx.FRAGMENT_SHADER);
            Module.ctx.shaderSource(this.fragmentShader, fsSource);
            Module.ctx.compileShader(this.fragmentShader);

            this.program = Module.ctx.createProgram();
            Module.ctx.attachShader(this.program, this.vertexShader);
            Module.ctx.attachShader(this.program, this.fragmentShader);
            Module.ctx.bindAttribLocation(this.program, 0, 'a_position');
            Module.ctx.linkProgram(this.program);
          }

          this.positionLocation = Module.ctx.getAttribLocation(this.program, 'a_position');

          this.texCoordLocations = [];

          for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
            if (!GL.immediate.enabledClientAttributes[GL.immediate.TEXTURE0 + i]) {
              this.texCoordLocations[i] = -1;
              continue;
            }

            if (useCurrProgram) {
              this.texCoordLocations[i] = Module.ctx.getAttribLocation(this.program, 'a_texCoord' + i);
            } else {
              this.texCoordLocations[i] = Module.ctx.getAttribLocation(this.program, aTexCoordPrefix + i);
            }
          }

          if (!useCurrProgram) {
            // Temporarily switch to the program so we can set our sampler uniforms early.
            var prevBoundProg = Module.ctx.getParameter(Module.ctx.CURRENT_PROGRAM);
            Module.ctx.useProgram(this.program);
            {
              for (var i = 0; i < this.usedTexUnitList.length; i++) {
                var texUnitID = this.usedTexUnitList[i];
                var texSamplerLoc = Module.ctx.getUniformLocation(this.program, uTexUnitPrefix + texUnitID);
                Module.ctx.uniform1i(texSamplerLoc, texUnitID);
              }
            }
            Module.ctx.useProgram(prevBoundProg);
          }

          this.textureMatrixLocations = [];
          for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
            this.textureMatrixLocations[i] = Module.ctx.getUniformLocation(this.program, 'u_textureMatrix' + i);
          }
          this.colorLocation = Module.ctx.getAttribLocation(this.program, 'a_color');
          this.normalLocation = Module.ctx.getAttribLocation(this.program, 'a_normal');

          this.modelViewLocation = Module.ctx.getUniformLocation(this.program, 'u_modelView');
          this.projectionLocation = Module.ctx.getUniformLocation(this.program, 'u_projection');

          this.hasTextures = hasTextures;
          this.hasNormal = normalSize > 0 && this.normalLocation >= 0;
          this.hasColor = (this.colorLocation === 0) || this.colorLocation > 0;

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
            GL.immediate.fixedFunctionProgram = this.program;
          }

          if (this.modelViewLocation) Module.ctx.uniformMatrix4fv(this.modelViewLocation, false, GL.immediate.matrix['m']);
          if (this.projectionLocation) Module.ctx.uniformMatrix4fv(this.projectionLocation, false, GL.immediate.matrix['p']);

          var clientAttributes = GL.immediate.clientAttributes;

#if GL_ASSERTIONS
          GL.validateVertexAttribPointer(positionSize, positionType, GL.immediate.stride, clientAttributes[GL.immediate.VERTEX].offset);
#endif
          Module.ctx.vertexAttribPointer(this.positionLocation, positionSize, positionType, false,
                                         GL.immediate.stride, clientAttributes[GL.immediate.VERTEX].offset);
          Module.ctx.enableVertexAttribArray(this.positionLocation);
          if (this.hasTextures) {
            //for (var i = 0; i < this.usedTexUnitList.length; i++) {
            //  var texUnitID = this.usedTexUnitList[i];
            for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
              var texUnitID = i;
              var attribLoc = this.texCoordLocations[texUnitID];
              if (attribLoc === undefined || attribLoc < 0) continue;

              if (texUnitID < textureSizes.length && textureSizes[texUnitID]) {
#if GL_ASSERTIONS
                GL.validateVertexAttribPointer(textureSizes[texUnitID], textureTypes[texUnitID], GL.immediate.stride, GL.immediate.clientAttributes[GL.immediate.TEXTURE0 + texUnitID].offset);
#endif
                Module.ctx.vertexAttribPointer(attribLoc, textureSizes[texUnitID], textureTypes[texUnitID], false,
                                               GL.immediate.stride, GL.immediate.clientAttributes[GL.immediate.TEXTURE0 + texUnitID].offset);
                Module.ctx.enableVertexAttribArray(attribLoc);
              } else {
                // These two might be dangerous, but let's try them.
                Module.ctx.vertexAttrib4f(attribLoc, 0, 0, 0, 1);
                Module.ctx.disableVertexAttribArray(attribLoc);
              }
            }
            for (var i = 0; i < GL.immediate.MAX_TEXTURES; i++) {
              if (this.textureMatrixLocations[i]) { // XXX might we need this even without the condition we are currently in?
                Module.ctx.uniformMatrix4fv(this.textureMatrixLocations[i], false, GL.immediate.matrix['t' + i]);
              }
            }
          }
          if (colorSize) {
#if GL_ASSERTIONS
            GL.validateVertexAttribPointer(colorSize, colorType, GL.immediate.stride, clientAttributes[GL.immediate.COLOR].offset);
#endif
            Module.ctx.vertexAttribPointer(this.colorLocation, colorSize, colorType, true,
                                           GL.immediate.stride, clientAttributes[GL.immediate.COLOR].offset);
            Module.ctx.enableVertexAttribArray(this.colorLocation);
          } else if (this.hasColor) {
            Module.ctx.disableVertexAttribArray(this.colorLocation);
            Module.ctx.vertexAttrib4fv(this.colorLocation, GL.immediate.clientColor);
          }
          if (this.hasNormal) {
#if GL_ASSERTIONS
            GL.validateVertexAttribPointer(normalSize, normalType, GL.immediate.stride, clientAttributes[GL.immediate.NORMAL].offset);
#endif
            Module.ctx.vertexAttribPointer(this.normalLocation, normalSize, normalType, true,
                                           GL.immediate.stride, clientAttributes[GL.immediate.NORMAL].offset);
            Module.ctx.enableVertexAttribArray(this.normalLocation);
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
          if (this.hasColor) {
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
#if ASSERTIONS
        console.log("DrawElements doesn't actually prepareClientAttributes properly.");
#endif
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

      // TexEnv stuff needs to be prepared early, so do it here.
      // init() is too late for -O2, since it freezes the GL functions
      // by that point.
      GL.immediate.MapTreeLib = GL.immediate.spawnMapTreeLib();
      GL.immediate.spawnMapTreeLib = null;

      GL.immediate.TexEnvJIT = GL.immediate.spawnTexEnvJIT();
      GL.immediate.spawnTexEnvJIT = null;

      GL.immediate.setupHooks();
    },

    setupHooks: function() {
      if (!GLEmulation.hasRunInit) {
        GLEmulation.init();
      }

      var glActiveTexture = _glActiveTexture;
      _glActiveTexture = function(texture) {
        GL.immediate.TexEnvJIT.hook_activeTexture(texture);
        glActiveTexture(texture);
      };

      var glEnable = _glEnable;
      _glEnable = function(cap) {
        GL.immediate.TexEnvJIT.hook_enable(cap);
        glEnable(cap);
      };
      var glDisable = _glDisable;
      _glDisable = function(cap) {
        GL.immediate.TexEnvJIT.hook_disable(cap);
        glDisable(cap);
      };

      var glTexEnvf = (typeof(_glTexEnvf) != 'undefined') ? _glTexEnvf : function(){};
      _glTexEnvf = function(target, pname, param) {
        GL.immediate.TexEnvJIT.hook_texEnvf(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvf(target, pname, param);
      };
      var glTexEnvi = (typeof(_glTexEnvi) != 'undefined') ? _glTexEnvi : function(){};
      _glTexEnvi = function(target, pname, param) {
        GL.immediate.TexEnvJIT.hook_texEnvi(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvi(target, pname, param);
      };
      var glTexEnvfv = (typeof(_glTexEnvfv) != 'undefined') ? _glTexEnvfv : function(){};
      _glTexEnvfv = function(target, pname, param) {
        GL.immediate.TexEnvJIT.hook_texEnvfv(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvfv(target, pname, param);
      };

      var glGetIntegerv = _glGetIntegerv;
      _glGetIntegerv = function(pname, params) {
        switch (pname) {
          case 0x8B8D: { // GL_CURRENT_PROGRAM
            // Just query directly so we're working with WebGL objects.
            var cur = Module.ctx.getParameter(Module.ctx.CURRENT_PROGRAM);
            if (cur == GL.immediate.fixedFunctionProgram) {
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
      GL.immediate.initted = true;

      if (!Module.useWebGL) return; // a 2D canvas may be currently used TODO: make sure we are actually called in that case

      this.TexEnvJIT.init(Module.ctx);

      GL.immediate.MAX_TEXTURES = Module.ctx.getParameter(Module.ctx.MAX_TEXTURE_IMAGE_UNITS);
      GL.immediate.NUM_ATTRIBUTES = GL.immediate.TEXTURE0 + GL.immediate.MAX_TEXTURES;
      GL.immediate.clientAttributes = [];
      for (var i = 0; i < GL.immediate.NUM_ATTRIBUTES; i++) {
        GL.immediate.clientAttributes.push({});
      }

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
      this.rendererCache = this.MapTreeLib.create();

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
        GL.immediate.vertexCounter = (GL.immediate.stride * count) / 4; // XXX assuming float
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
      var numVertexes = 4 * this.vertexCounter / GL.immediate.stride;
      assert(numVertexes % 1 == 0, "`numVertexes` must be an integer.");

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
    // Push the old state:
    GL.immediate.enabledClientAttributes_preBegin = GL.immediate.enabledClientAttributes;
    GL.immediate.enabledClientAttributes = [];

    GL.immediate.clientAttributes_preBegin = GL.immediate.clientAttributes;
    GL.immediate.clientAttributes = []
    for (var i = 0; i < GL.immediate.clientAttributes_preBegin.length; i++) {
      GL.immediate.clientAttributes.push({});
    }

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

    // Pop the old state:
    GL.immediate.enabledClientAttributes = GL.immediate.enabledClientAttributes_preBegin;
    GL.immediate.clientAttributes = GL.immediate.clientAttributes_preBegin;

    GL.immediate.modifiedClientAttributes = true;
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
  glGenVertexArrays__deps: ['$GLEmulation'],
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
      GL.immediate.useTextureMatrix = true;
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
  glFramebufferTexture2DOES: 'glFramebufferTexture2D',

#else // LEGACY_GL_EMULATION

  // Warn if code tries to use various emulation stuff, when emulation is disabled
  // (do not warn if INCLUDE_FULL_LIBRARY is one, because then likely the gl code will
  // not be called anyhow, leave only the runtime aborts)
  glVertexPointer__deps: [function() {
#if INCLUDE_FULL_LIBRARY == 0
    warn('Legacy GL function (glVertexPointer) called. You need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.');
#endif
  }],
  glVertexPointer: function(){ throw 'Legacy GL function (glVertexPointer) called. You need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },
  glGenVertexArrays__deps: [function() {
#if INCLUDE_FULL_LIBRARY == 0
    warn('Legacy GL function (glGenVertexArrays) called. You need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.');
#endif
  }],
  glGenVertexArrays: function(){ throw 'Legacy GL function (glGenVertexArrays) called. You need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },
  glMatrixMode__deps: [function() {
#if INCLUDE_FULL_LIBRARY == 0
    warn('Legacy GL function (glMatrixMode) called. You need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.');
#endif
  }],
  glMatrixMode: function(){ throw 'Legacy GL function (glMatrixMode) called. You need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },
  glBegin__deps: [function() {
#if INCLUDE_FULL_LIBRARY == 0
    warn('Legacy GL function (glBegin) called. You need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.');
#endif
  }],
  glBegin: function(){ throw 'Legacy GL function (glBegin) called. You need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },
  glLoadIdentity__deps: [function() {
#if INCLUDE_FULL_LIBRARY == 0
    warn('Legacy GL function (glLoadIdentity) called. You need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.');
#endif
  }],
  glLoadIdentity: function(){ throw 'Legacy GL function (glLoadIdentity) called. You need to compile with -s LEGACY_GL_EMULATION=1 to enable legacy GL emulation.'; },

#endif // LEGACY_GL_EMULATION

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

  gluOrtho2D__deps: ['glOrtho'],
  gluOrtho2D: function(left, right, bottom, top) {
    _glOrtho(left, right, bottom, top, -1, 1);
  },

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
#if GL_ASSERTIONS
    GL.validateVertexAttribPointer(size, type, stride, ptr);
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

// Legacy GL emulation 
if (LEGACY_GL_EMULATION) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$GLEmulation');
}

// GL proc address retrieval
LibraryGL.emscripten_GetProcAddress__deps = [function() {
  // ProcAddress is used, so include everything in GL. This runs before we go to the $ProcAddressTable object,
  // and we fill its deps just in time, and create the lookup table
  var table = {};
  LibraryManager.library.emscripten_procAddressTable__deps = keys(LibraryGL).map(function(x) {
    if (x.substr(-6) == '__deps' || x.substr(-9) == '__postset' || x.substr(-5) == '__sig' || x.substr(-5) == '__asm' || x.substr(0, 2) != 'gl') return null;
    var original = x;
    if (('_' + x) in Functions.implementedFunctions) {
      // a user-implemented function aliases this one, but we still want it to be accessible by name, so rename it
      var y = x + '__procTable';
      LibraryManager.library[y] = LibraryManager.library[x];
      LibraryManager.library[y + '__deps'] = LibraryManager.library[x + '__deps'];
      LibraryManager.library[y + '__postset'] = LibraryManager.library[x + '__postset'];
      LibraryManager.library[y + '__sig'] = LibraryManager.library[x + '__sig'];//|| Functions.implementedFunctions['_' + x];
      LibraryManager.library[y + '__asm'] = LibraryManager.library[x + '__asm'];
      x = y;
      assert(!(y in Functions.implementedFunctions) && !Functions.unimplementedFunctions['_' + y]);
    }
    var longX = '_' + x;
    var sig = LibraryManager.library[x + '__sig'] || functionStubSigs[longX];
    if (sig) {
      table[original] = Functions.getIndex(longX, sig);
      if (!(longX in Functions.implementedFunctions)) Functions.unimplementedFunctions[longX] = sig;
    }
    return x;
  }).filter(function(x) { return x !== null });
  // convert table into function with switch, to not confuse closure compiler
  var tableImpl = 'switch(name) {\n';
  for (var x in table) tableImpl += 'case "' + x + '": return ' + table[x] + '; break;\n';
  tableImpl += '}\nreturn 0;';
  LibraryManager.library.emscripten_procAddressTable = new Function('name', tableImpl);
}, 'emscripten_procAddressTable'];
LibraryGL.emscripten_GetProcAddress = function(name) {
  name = name.replace('EXT', '').replace('ARB', '');
  switch(name) { // misc renamings
    case 'glCreateProgramObject': name = 'glCreateProgram'; break;
    case 'glUseProgramObject': name = 'glUseProgram'; break;
    case 'glCreateShaderObject': name = 'glCreateShader'; break;
    case 'glAttachObject': name = 'glAttachShader'; break;
    case 'glDetachObject': name = 'glDetachShader'; break;
  }
  var ret = _emscripten_procAddressTable(name);
  if (!ret) Module.printErr('WARNING: getProcAddress failed for ' + name);
  return ret;
}

// Final merge
mergeInto(LibraryManager.library, LibraryGL);

assert(!(FULL_ES2 && LEGACY_GL_EMULATION), 'cannot emulate both ES2 and legacy GL');

