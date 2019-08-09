/*
 * Copyright 2010 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * GL support. See http://kripken.github.io/emscripten-site/docs/porting/multimedia_and_graphics/OpenGL-support.html
 * for current status.
 */

var LibraryWebGL2 = {
  glGetStringi__deps: ['$stringToNewUTF8'],
  glGetStringi__sig: 'iii',
  glGetStringi: function(name, index) {
    if (GL.currentContext.version < 2) {
      GL.recordError(0x0502 /* GL_INVALID_OPERATION */); // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
      return 0;
    }
    var stringiCache = GL.stringiCache[name];
    if (stringiCache) {
      if (index < 0 || index >= stringiCache.length) {
        GL.recordError(0x0501/*GL_INVALID_VALUE*/);
#if GL_ASSERTIONS
        err('GL_INVALID_VALUE in glGetStringi: index out of range (' + index + ')!');
#endif
        return 0;
      }
      return stringiCache[index];
    }
    switch(name) {
      case 0x1F03 /* GL_EXTENSIONS */:
        var exts = GLctx.getSupportedExtensions() || []; // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
#if GL_EXTENSIONS_IN_PREFIXED_FORMAT
        exts = exts.concat(exts.map(function(e) { return "GL_" + e; }));
#endif
        exts = exts.map(function(e) { return stringToNewUTF8(e); });

        stringiCache = GL.stringiCache[name] = exts;
        if (index < 0 || index >= stringiCache.length) {
          GL.recordError(0x0501/*GL_INVALID_VALUE*/);
#if GL_ASSERTIONS
          err('GL_INVALID_VALUE in glGetStringi: index out of range (' + index + ') in a call to GL_EXTENSIONS!');
#endif
          return 0;
        }
        return stringiCache[index];
      default:
        GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
        err('GL_INVALID_ENUM in glGetStringi: Unknown parameter ' + name + '!');
#endif
        return 0;
    }
  },

  glGetInteger64v__sig: 'vii',
  glGetInteger64v__deps: ['$emscriptenWebGLGet'],
  glGetInteger64v: function(name_, p) {
    emscriptenWebGLGet(name_, p, {{{ cDefine('EM_FUNC_SIG_PARAM_I64') }}});
  },

  glGetInternalformativ__sig: 'viiiii',
  glGetInternalformativ: function(target, internalformat, pname, bufSize, params) {
    if (bufSize < 0) {
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetInternalformativ(target=' + target + ', internalformat=' + internalformat + ', pname=' + pname + ', bufSize=' + bufSize + ', params=' + params + '): Function called with bufSize < 0!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    if (!params) {
      // GLES3 specification does not specify how to behave if values is a null pointer. Since calling this function does not make sense
      // if values == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetInternalformativ(target=' + target + ', internalformat=' + internalformat + ', pname=' + pname + ', bufSize=' + bufSize + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    var ret = GLctx['getInternalformatParameter'](target, internalformat, pname);
    if (ret === null) return;
    for (var i = 0; i < ret.length && i < bufSize; ++i) {
      {{{ makeSetValue('params', 'i', 'ret[i]', 'i32') }}};
    }
  },

  glCompressedTexImage3D__sig: 'viiiiiiiii',
  glCompressedTexImage3D: function(target, level, internalFormat, width, height, depth, border, imageSize, data) {
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx['compressedTexImage3D'](target, level, internalFormat, width, height, depth, border, imageSize, data);
      } else {
        GLctx['compressedTexImage3D'](target, level, internalFormat, width, height, depth, border, HEAPU8, data, imageSize);
      }
    } else {
      GLctx['compressedTexImage3D'](target, level, internalFormat, width, height, depth, border, data ? {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}} : null);
    }
  },

  glCompressedTexSubImage3D__sig: 'viiiiiiiiiii',
  glCompressedTexSubImage3D: function(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) {
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      if (GLctx.currentPixelUnpackBufferBinding) {
        GLctx['compressedTexSubImage3D'](target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
      } else {
        GLctx['compressedTexSubImage3D'](target, level, xoffset, yoffset, zoffset, width, height, depth, format, HEAPU8, data, imageSize);
      }
    } else {
      GLctx['compressedTexSubImage3D'](target, level, xoffset, yoffset, zoffset, width, height, depth, format, data ? {{{ makeHEAPView('U8', 'data', 'data+imageSize') }}} : null);
    }
  },

  _heapObjectForWebGLType: function(type) {
    switch(type) {
      case 0x1400 /* GL_BYTE */:
        return HEAP8;
      case 0x1401 /* GL_UNSIGNED_BYTE */:
        return HEAPU8;
      case 0x1402 /* GL_SHORT */:
        return HEAP16;
      case 0x1403 /* GL_UNSIGNED_SHORT */:
      case 0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */:
      case 0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */:
      case 0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */:
      case 0x8D61 /* GL_HALF_FLOAT_OES */:
      case 0x140B /* GL_HALF_FLOAT */:
        return HEAPU16;
      case 0x1404 /* GL_INT */:
        return HEAP32;
      case 0x1405 /* GL_UNSIGNED_INT */:
      case 0x84FA /* GL_UNSIGNED_INT_24_8_WEBGL/GL_UNSIGNED_INT_24_8 */:
      case 0x8C3E /* GL_UNSIGNED_INT_5_9_9_9_REV */:
      case 0x8368 /* GL_UNSIGNED_INT_2_10_10_10_REV */:
      case 0x8C3B /* GL_UNSIGNED_INT_10F_11F_11F_REV */:
      case 0x84FA /* GL_UNSIGNED_INT_24_8 */:
        return HEAPU32;
      case 0x1406 /* GL_FLOAT */:
        return HEAPF32;
    }
  },

  _heapAccessShiftForWebGLType: {
      0x1402 /* GL_SHORT */: 1,
      0x1403 /* GL_UNSIGNED_SHORT */: 1,
      0x8363 /* GL_UNSIGNED_SHORT_5_6_5 */: 1,
      0x8033 /* GL_UNSIGNED_SHORT_4_4_4_4 */: 1,
      0x8034 /* GL_UNSIGNED_SHORT_5_5_5_1 */: 1,
      0x8D61 /* GL_HALF_FLOAT_OES */: 1,
      0x140B /* GL_HALF_FLOAT */: 1,
      0x1404 /* GL_INT */: 2,
      0x1406 /* GL_FLOAT */: 2,
      0x1405 /* GL_UNSIGNED_INT */: 2,
      0x84FA /* GL_UNSIGNED_INT_24_8_WEBGL/GL_UNSIGNED_INT_24_8 */: 2,
      0x8C3E /* GL_UNSIGNED_INT_5_9_9_9_REV */: 2,
      0x8368 /* GL_UNSIGNED_INT_2_10_10_10_REV */: 2,
      0x8C3B /* GL_UNSIGNED_INT_10F_11F_11F_REV */: 2,
      0x84FA /* GL_UNSIGNED_INT_24_8 */: 2
  },

  glGetBufferParameteri64v__sig: 'viii',
  glGetBufferParameteri64v: function(target, value, data) {
    if (!data) {
      // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
      // if data == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetBufferParameteri64v(target=' + target + ', value=' + value + ', data=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('data', '0', 'GLctx.getBufferParameter(target, value)', 'i64') }}};
  },

  glInvalidateFramebuffer__deps: ['_tempFixedLengthArray'],
  glInvalidateFramebuffer__sig: 'viii',
  glInvalidateFramebuffer: function(target, numAttachments, attachments) {
#if GL_ASSERTIONS
    assert(numAttachments < __tempFixedLengthArray.length, 'Invalid count of numAttachments=' + numAttachments + ' passed to glInvalidateFramebuffer (that many attachment points do not exist in GL)');
#endif
    var list = __tempFixedLengthArray[numAttachments];
    for (var i = 0; i < numAttachments; i++) {
      list[i] = {{{ makeGetValue('attachments', 'i*4', 'i32') }}};
    }

    GLctx['invalidateFramebuffer'](target, list);
  },

  glInvalidateSubFramebuffer__deps: ['_tempFixedLengthArray'],
  glInvalidateSubFramebuffer__sig: 'viiiiiii',
  glInvalidateSubFramebuffer: function(target, numAttachments, attachments, x, y, width, height) {
#if GL_ASSERTIONS
    assert(numAttachments < __tempFixedLengthArray.length, 'Invalid count of numAttachments=' + numAttachments + ' passed to glInvalidateSubFramebuffer (that many attachment points do not exist in GL)');
#endif
    var list = __tempFixedLengthArray[numAttachments];
    for (var i = 0; i < numAttachments; i++) {
      list[i] = {{{ makeGetValue('attachments', 'i*4', 'i32') }}};
    }

    GLctx['invalidateSubFramebuffer'](target, list, x, y, width, height);
  },

  glTexImage3D__sig: 'viiiiiiiiii',
  glTexImage3D__deps: ['_heapObjectForWebGLType', '_heapAccessShiftForWebGLType'],
  glTexImage3D: function(target, level, internalFormat, width, height, depth, border, format, type, pixels) {
    if (GLctx.currentPixelUnpackBufferBinding) {
      GLctx['texImage3D'](target, level, internalFormat, width, height, depth, border, format, type, pixels);
    } else if (pixels != 0) {
      GLctx['texImage3D'](target, level, internalFormat, width, height, depth, border, format, type, __heapObjectForWebGLType(type), pixels >> (__heapAccessShiftForWebGLType[type]|0));
    } else {
      GLctx['texImage3D'](target, level, internalFormat, width, height, depth, border, format, type, null);
    }
  },

  glTexSubImage3D__sig: 'viiiiiiiiiii',
  glTexSubImage3D__deps: ['_heapObjectForWebGLType', '_heapAccessShiftForWebGLType'],
  glTexSubImage3D: function(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) {
    if (GLctx.currentPixelUnpackBufferBinding) {
      GLctx['texSubImage3D'](target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    } else if (pixels != 0) {
      GLctx['texSubImage3D'](target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, __heapObjectForWebGLType(type), pixels >> (__heapAccessShiftForWebGLType[type]|0));
    } else {
      GLctx['texSubImage3D'](target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, null);
    }
  },

  // Queries
  glGenQueries__sig: 'vii',
  glGenQueries__deps: ['_glGenObject'],
  glGenQueries: function(n, ids) {
    __glGenObject(n, ids, 'createQuery', GL.queries
#if GL_ASSERTIONS
    , 'glGenQueries'
#endif
      );
  },

  glDeleteQueries__sig: 'vii',
  glDeleteQueries: function(n, ids) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('ids', 'i*4', 'i32') }}};
      var query = GL.queries[id];
      if (!query) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
      GLctx['deleteQuery'](query);
      GL.queries[id] = null;
    }
  },

  glIsQuery__sig: 'ii',
  glIsQuery: function(id) {
    var query = GL.queries[id];
    if (!query) return 0;
    return GLctx['isQuery'](query);
  },

  glBeginQuery__sig: 'vii',
  glBeginQuery: function(target, id) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.queries, id, 'glBeginQuery', 'id');
#endif
    GLctx['beginQuery'](target, GL.queries[id]);
  },

  glGetQueryiv__sig: 'viii',
  glGetQueryiv: function(target, pname, params) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetQueryiv(target=' + target +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    {{{ makeSetValue('params', '0', 'GLctx[\'getQuery\'](target, pname)', 'i32') }}};
  },

  glGetQueryObjectuiv__sig: 'viii',
  glGetQueryObjectuiv: function(id, pname, params) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetQueryObjectuiv(id=' + id +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.queries, id, 'glGetQueryObjectuiv', 'id');
#endif
    var query = GL.queries[id];
    var param = GLctx['getQueryParameter'](query, pname);
    var ret;
    if (typeof param == 'boolean') {
      ret = param ? 1 : 0;
    } else {
      ret = param;
    }
    {{{ makeSetValue('params', '0', 'ret', 'i32') }}};
  },

  // Sampler objects
  glGenSamplers__sig: 'vii',
  glGenSamplers__deps: ['_glGenObject'],
  glGenSamplers: function(n, samplers) {
    __glGenObject(n, samplers, 'createSampler', GL.samplers
#if GL_ASSERTIONS
    , 'glGenSamplers'
#endif
      );
  },

  glDeleteSamplers__sig: 'vii',
  glDeleteSamplers: function(n, samplers) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('samplers', 'i*4', 'i32') }}};
      var sampler = GL.samplers[id];
      if (!sampler) continue;
      GLctx['deleteSampler'](sampler);
      sampler.name = 0;
      GL.samplers[id] = null;
    }
  },

  glIsSampler__sig: 'ii',
  glIsSampler: function(id) {
    var sampler = GL.samplers[id];
    if (!sampler) return 0;
    return GLctx['isSampler'](sampler);
  },

  glBindSampler__sig: 'vii',
  glBindSampler: function(unit, sampler) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    GLctx['bindSampler'](unit, GL.samplers[sampler]);
  },

  glSamplerParameterf__sig: 'viif',
  glSamplerParameterf: function(sampler, pname, param) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    GLctx['samplerParameterf'](GL.samplers[sampler], pname, param);
  },

  glSamplerParameteri__sig: 'viii',
  glSamplerParameteri: function(sampler, pname, param) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    GLctx['samplerParameteri'](GL.samplers[sampler], pname, param);
  },

  glSamplerParameterfv__sig: 'viii',
  glSamplerParameterfv: function(sampler, pname, params) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    var param = {{{ makeGetValue('params', '0', 'float') }}};
    GLctx['samplerParameterf'](GL.samplers[sampler], pname, param);
  },

  glSamplerParameteriv__sig: 'viii',
  glSamplerParameteriv: function(sampler, pname, params) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    var param = {{{ makeGetValue('params', '0', 'i32') }}};
    GLctx['samplerParameteri'](GL.samplers[sampler], pname, param);
  },

  glGetSamplerParameterfv__sig: 'viii',
  glGetSamplerParameterfv: function(sampler, pname, params) {
    if (!params) {
      // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetSamplerParameterfv(sampler=' + sampler +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    sampler = GL.samplers[sampler];
    {{{ makeSetValue('params', '0', 'GLctx[\'getSamplerParameter\'](sampler, pname)', 'float') }}};
  },

  glGetSamplerParameteriv__sig: 'viii',
  glGetSamplerParameteriv: function(sampler, pname, params) {
    if (!params) {
      // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetSamplerParameteriv(sampler=' + sampler +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    sampler = GL.samplers[sampler];
    {{{ makeSetValue('params', '0', 'GLctx[\'getSamplerParameter\'](sampler, pname)', 'i32') }}};
  },

  // Transform Feedback
  glGenTransformFeedbacks__sig: 'vii',
  glGenTransformFeedbacks__deps: ['_glGenObject'],
  glGenTransformFeedbacks: function(n, ids) {
    __glGenObject(n, ids, 'createTransformFeedback', GL.transformFeedbacks
#if GL_ASSERTIONS
    , 'glGenTransformFeedbacks'
#endif
      );
  },

  glDeleteTransformFeedbacks__sig: 'vii',
  glDeleteTransformFeedbacks: function(n, ids) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('ids', 'i*4', 'i32') }}};
      var transformFeedback = GL.transformFeedbacks[id];
      if (!transformFeedback) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
      GLctx['deleteTransformFeedback'](transformFeedback);
      transformFeedback.name = 0;
      GL.transformFeedbacks[id] = null;
    }
  },

  glIsTransformFeedback__sig: 'ii',
  glIsTransformFeedback: function(id) {
    return GLctx['isTransformFeedback'](GL.transformFeedbacks[id]);
  },

  glBindTransformFeedback__sig: 'vii',
  glBindTransformFeedback: function(target, id) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.transformFeedbacks, id, 'glBindTransformFeedback', 'id');
#endif
    GLctx['bindTransformFeedback'](target, GL.transformFeedbacks[id]);
  },

  glTransformFeedbackVaryings__sig: 'viiii',
  glTransformFeedbackVaryings: function(program, count, varyings, bufferMode) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glTransformFeedbackVaryings', 'program');
#endif
    program = GL.programs[program];
    var vars = [];
    for (var i = 0; i < count; i++)
      vars.push(UTF8ToString({{{ makeGetValue('varyings', 'i*4', 'i32') }}}));

    GLctx['transformFeedbackVaryings'](program, vars, bufferMode);
  },

  glGetTransformFeedbackVarying__sig: 'viiiiiii',
  glGetTransformFeedbackVarying: function(program, index, bufSize, length, size, type, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetTransformFeedbackVarying', 'program');
#endif
    program = GL.programs[program];
    var info = GLctx['getTransformFeedbackVarying'](program, index);
    if (!info) return; // If an error occurred, the return parameters length, size, type and name will be unmodified.

    if (name && bufSize > 0) {
      var numBytesWrittenExclNull = stringToUTF8(info.name, name, bufSize);
      if (length) {{{ makeSetValue('length', '0', 'numBytesWrittenExclNull', 'i32') }}};
    } else {
      if (length) {{{ makeSetValue('length', '0', 0, 'i32') }}};
    }

    if (size) {{{ makeSetValue('size', '0', 'info.size', 'i32') }}};
    if (type) {{{ makeSetValue('type', '0', 'info.type', 'i32') }}};
  },

  $emscriptenWebGLGetIndexed: function(target, index, data, type) {
    if (!data) {
      // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
      // if data == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetInteger(64)i_v(target=' + target + ', index=' + index + ', data=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    var result = GLctx['getIndexedParameter'](target, index);
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
              err('GL_INVALID_ENUM in glGetInteger(64)i_v(' + target + ') and it returns null!');
#endif
              return;
            }
          }
        } else if (result instanceof WebGLBuffer) {
          ret = result.name | 0;
        } else {
          GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
          err('GL_INVALID_ENUM in glGetInteger(64)i_v: Unknown object returned from WebGL getIndexedParameter(' + target + ')!');
#endif
          return;
        }
        break;
      default:
        GL.recordError(0x0500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
        err('GL_INVALID_ENUM in glGetInteger(64)i_v: Native code calling glGetInteger(64)i_v(' + target + ') and it returns ' + result + ' of type ' + typeof(result) + '!');
#endif
        return;
    }

    switch (type) {
      case {{{ cDefine('EM_FUNC_SIG_PARAM_I64') }}}: {{{ makeSetValue('data', '0', 'ret', 'i64') }}}; break;
      case {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}}: {{{ makeSetValue('data', '0', 'ret', 'i32') }}}; break;
      case {{{ cDefine('EM_FUNC_SIG_PARAM_F') }}}: {{{ makeSetValue('data', '0', 'ret', 'float') }}}; break;
      case {{{ cDefine('EM_FUNC_SIG_PARAM_B') }}}: {{{ makeSetValue('data', '0', 'ret ? 1 : 0', 'i8') }}}; break;
      default: throw 'internal emscriptenWebGLGetIndexed() error, bad type: ' + type;
    }
  },

  glGetIntegeri_v__sig: 'viii',
  glGetIntegeri_v__deps: ['$emscriptenWebGLGetIndexed'],
  glGetIntegeri_v: function(target, index, data) {
    emscriptenWebGLGetIndexed(target, index, data, {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}});
  },

  glGetInteger64i_v__sig: 'viii',
  glGetInteger64i_v__deps: ['$emscriptenWebGLGetIndexed'],
  glGetInteger64i_v: function(target, index, data) {
    emscriptenWebGLGetIndexed(target, index, data, {{{ cDefine('EM_FUNC_SIG_PARAM_I64') }}});
  },

  // Uniform Buffer objects
  glBindBufferBase__sig: 'viii',
  glBindBufferBase: function(target, index, buffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.buffers, buffer, 'glBindBufferBase', 'buffer');
#endif
    GLctx['bindBufferBase'](target, index, GL.buffers[buffer]);
  },

  glBindBufferRange__sig: 'viiiii',
  glBindBufferRange: function(target, index, buffer, offset, ptrsize) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.buffers, buffer, 'glBindBufferRange', 'buffer');
#endif
    GLctx['bindBufferRange'](target, index, GL.buffers[buffer], offset, ptrsize);
  },

  glGetUniformIndices__sig: 'viiii',
  glGetUniformIndices: function(program, uniformCount, uniformNames, uniformIndices) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformIndices', 'program');
#endif
    if (!uniformIndices) {
      // GLES2 specification does not specify how to behave if uniformIndices is a null pointer. Since calling this function does not make sense
      // if uniformIndices == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetUniformIndices(program=' + program + ', uniformCount=' + uniformCount + ', uniformNames=' + uniformNames + ', uniformIndices=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    if (uniformCount > 0 && (uniformNames == 0 || uniformIndices == 0)) {
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    program = GL.programs[program];
    var names = [];
    for (var i = 0; i < uniformCount; i++)
      names.push(UTF8ToString({{{ makeGetValue('uniformNames', 'i*4', 'i32') }}}));

    var result = GLctx['getUniformIndices'](program, names);
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
#endif
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetActiveUniformsiv(program=' + program + ', uniformCount=' + uniformCount + ', uniformIndices=' + uniformIndices + ', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    if (uniformCount > 0 && uniformIndices == 0) {
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    program = GL.programs[program];
    var ids = [];
    for (var i = 0; i < uniformCount; i++) {
      ids.push({{{ makeGetValue('uniformIndices', 'i*4', 'i32') }}});
    }

    var result = GLctx['getActiveUniforms'](program, ids, pname);
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
    return GLctx['getUniformBlockIndex'](GL.programs[program], UTF8ToString(uniformBlockName));
  },

  glGetActiveUniformBlockiv__sig: 'viiii',
  glGetActiveUniformBlockiv: function(program, uniformBlockIndex, pname, params) {
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetActiveUniformBlockiv(program=' + program + ', uniformBlockIndex=' + uniformBlockIndex + ', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveUniformBlockiv', 'program');
#endif
    program = GL.programs[program];

    switch(pname) {
      case 0x8A41: /* GL_UNIFORM_BLOCK_NAME_LENGTH */
        var name = GLctx['getActiveUniformBlockName'](program, uniformBlockIndex);
        {{{ makeSetValue('params', 0, 'name.length+1', 'i32') }}};
        return;
      default:
        var result = GLctx['getActiveUniformBlockParameter'](program, uniformBlockIndex, pname);
        if (!result) return; // If an error occurs, nothing will be written to params.
        if (typeof result == 'number') {
          {{{ makeSetValue('params', '0', 'result', 'i32') }}};
        } else {
          for (var i = 0; i < result.length; i++) {
            {{{ makeSetValue('params', 'i*4', 'result[i]', 'i32') }}};
          }
        }
    }
  },

  glGetActiveUniformBlockName__sig: 'viiiii',
  glGetActiveUniformBlockName: function(program, uniformBlockIndex, bufSize, length, uniformBlockName) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveUniformBlockName', 'program');
#endif
    program = GL.programs[program];

    var result = GLctx['getActiveUniformBlockName'](program, uniformBlockIndex);
    if (!result) return; // If an error occurs, nothing will be written to uniformBlockName or length.
    if (uniformBlockName && bufSize > 0) {
      var numBytesWrittenExclNull = stringToUTF8(result, uniformBlockName, bufSize);
      if (length) {{{ makeSetValue('length', '0', 'numBytesWrittenExclNull', 'i32') }}};
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

    GLctx['uniformBlockBinding'](program, uniformBlockIndex, uniformBlockBinding);
  },

  glClearBufferiv__sig: 'viii',
  glClearBufferiv: function(buffer, drawbuffer, value) {
#if GL_ASSERTIONS
    assert((value & 3) == 0, 'Pointer to integer data passed to glClearBufferiv must be aligned to four bytes!');
#endif

    GLctx['clearBufferiv'](buffer, drawbuffer, HEAP32, value>>2);
  },

  glClearBufferuiv__sig: 'viii',
  glClearBufferuiv: function(buffer, drawbuffer, value) {
#if GL_ASSERTIONS
    assert((value & 3) == 0, 'Pointer to integer data passed to glClearBufferuiv must be aligned to four bytes!');
#endif

    GLctx['clearBufferuiv'](buffer, drawbuffer, HEAPU32, value>>2);
  },

  glClearBufferfv__sig: 'viii',
  glClearBufferfv: function(buffer, drawbuffer, value) {
#if GL_ASSERTIONS
    assert((value & 3) == 0, 'Pointer to float data passed to glClearBufferfv must be aligned to four bytes!');
#endif

    GLctx['clearBufferfv'](buffer, drawbuffer, HEAPF32, value>>2);
  },

  glFenceSync__sig: 'iii',
  glFenceSync: function(condition, flags) {
    var sync = GLctx.fenceSync(condition, flags);
    if (sync) {
      var id = GL.getNewId(GL.syncs);
      sync.name = id;
      GL.syncs[id] = sync;
      return id;
    } else {
      return 0; // Failed to create a sync object
    }
  },

  glDeleteSync__sig: 'vi',
  glDeleteSync: function(id) {
    if (!id) return;
    var sync = GL.syncs[id];
    if (!sync) { // glDeleteSync signals an error when deleting a nonexisting object, unlike some other GL delete functions.
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    GLctx.deleteSync(sync);
    sync.name = 0;
    GL.syncs[id] = null;
  },

  glClientWaitSync__sig: 'iiiii',
  glClientWaitSync: function(sync, flags, timeoutLo, timeoutHi) {
    // WebGL2 vs GLES3 differences: in GLES3, the timeout parameter is a uint64, where 0xFFFFFFFFFFFFFFFFULL means GL_TIMEOUT_IGNORED.
    // In JS, there's no 64-bit value types, so instead timeout is taken to be signed, and GL_TIMEOUT_IGNORED is given value -1.
    // Inherently the value accepted in the timeout is lossy, and can't take in arbitrary u64 bit pattern (but most likely doesn't matter)
    // See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15
    timeoutLo = timeoutLo >>> 0;
    timeoutHi = timeoutHi >>> 0;
    var timeout = (timeoutLo == 0xFFFFFFFF && timeoutHi == 0xFFFFFFFF) ? -1 : makeBigInt(timeoutLo, timeoutHi, true);
    return GLctx.clientWaitSync(GL.syncs[sync], flags, timeout);
  },

  glWaitSync__sig: 'viiii',
  glWaitSync: function(sync, flags, timeoutLo, timeoutHi) {
    // See WebGL2 vs GLES3 difference on GL_TIMEOUT_IGNORED above (https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15)
    timeoutLo = timeoutLo >>> 0;
    timeoutHi = timeoutHi >>> 0;
    var timeout = (timeoutLo == 0xFFFFFFFF && timeoutHi == 0xFFFFFFFF) ? -1 : makeBigInt(timeoutLo, timeoutHi, true);
    GLctx.waitSync(GL.syncs[sync], flags, timeout);
  },

  glGetSynciv__sig: 'viiiii',
  glGetSynciv: function(sync, pname, bufSize, length, values) {
    if (bufSize < 0) {
      // GLES3 specification does not specify how to behave if bufSize < 0, however in the spec wording for glGetInternalformativ, it does say that GL_INVALID_VALUE should be raised,
      // so raise GL_INVALID_VALUE here as well.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetSynciv(sync=' + sync + ', pname=' + pname + ', bufSize=' + bufSize + ', length=' + length + ', values='+values+'): Function called with bufSize < 0!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    if (!values) {
      // GLES3 specification does not specify how to behave if values is a null pointer. Since calling this function does not make sense
      // if values == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetSynciv(sync=' + sync + ', pname=' + pname + ', bufSize=' + bufSize + ', length=' + length + ', values=0): Function called with null out pointer!');
#endif
      GL.recordError(0x0501 /* GL_INVALID_VALUE */);
      return;
    }
    var ret = GLctx.getSyncParameter(GL.syncs[sync], pname);
    {{{ makeSetValue('length', '0', 'ret', 'i32') }}};
    if (ret !== null && length) {{{ makeSetValue('length', '0', '1', 'i32') }}}; // Report a single value outputted.
  },

  glIsSync__sig: 'ii',
  glIsSync: function(sync) {
    var sync = GL.syncs[sync];
    if (!sync) return 0;
    return GLctx.isSync(sync);
  },

  glGetUniformuiv__sig: 'viii',
  glGetUniformuiv__deps: ['$emscriptenWebGLGetUniform'],
  glGetUniformuiv: function(program, location, params) {
    emscriptenWebGLGetUniform(program, location, params, {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}});
  },

  glGetFragDataLocation__sig: 'iii',
  glGetFragDataLocation: function(program, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetFragDataLocation', 'program');
#endif
    return GLctx['getFragDataLocation'](GL.programs[program], UTF8ToString(name));
  },

  glGetVertexAttribIiv__sig: 'viii',
  glGetVertexAttribIiv__deps: ['$emscriptenWebGLGetVertexAttrib'],
  glGetVertexAttribIiv: function(index, pname, params) {
    // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttribI4iv(),
    // otherwise the results are undefined. (GLES3 spec 6.1.12)
    emscriptenWebGLGetVertexAttrib(index, pname, params, {{{ cDefine('EM_FUNC_SIG_PARAM_I') }}});
  },

  // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttribI4uiv(),
  // otherwise the results are undefined. (GLES3 spec 6.1.12)
  glGetVertexAttribIuiv__sig: 'viii',
  glGetVertexAttribIuiv__deps: ['$emscriptenWebGLGetVertexAttrib'],
  glGetVertexAttribIuiv: 'glGetVertexAttribIiv',

  glUniform1ui__sig: 'vii',
  glUniform1ui: function(location, v0) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1ui', 'location');
#endif
    GLctx.uniform1ui(GL.uniforms[location], v0);
  },

  glUniform2ui__sig: 'viii',
  glUniform2ui: function(location, v0, v1) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2ui', 'location');
#endif
    GLctx.uniform2ui(GL.uniforms[location], v0, v1);
  },

  glUniform3ui__sig: 'viiii',
  glUniform3ui: function(location, v0, v1, v2) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3ui', 'location');
#endif
    GLctx.uniform3ui(GL.uniforms[location], v0, v1, v2);
  },

  glUniform4ui__sig: 'viiiii',
  glUniform4ui: function(location, v0, v1, v2, v3) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4ui', 'location');
#endif
    GLctx.uniform4ui(GL.uniforms[location], v0, v1, v2, v3);
  },

  glUniform1uiv__sig: 'viii',
  glUniform1uiv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform1uiv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform1uiv must be aligned to four bytes!');
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform1uiv(GL.uniforms[location], HEAPU32, value>>2, count);
    } else {
      GLctx.uniform1uiv(GL.uniforms[location], {{{ makeHEAPView('U32', 'value', 'value+count*4') }}});
    }
  },

  glUniform2uiv__sig: 'viii',
  glUniform2uiv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform2uiv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform2uiv must be aligned to four bytes!');
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform2uiv(GL.uniforms[location], HEAPU32, value>>2, count*2);
    } else {
      GLctx.uniform2uiv(GL.uniforms[location], {{{ makeHEAPView('U32', 'value', 'value+count*8') }}});
    }
  },

  glUniform3uiv__sig: 'viii',
  glUniform3uiv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform3uiv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform3uiv must be aligned to four bytes!');
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform3uiv(GL.uniforms[location], HEAPU32, value>>2, count*3);
    } else {
      GLctx.uniform3uiv(GL.uniforms[location], {{{ makeHEAPView('U32', 'value', 'value+count*12') }}});
    }
  },

  glUniform4uiv__sig: 'viii',
  glUniform4uiv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniform4uiv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform4uiv must be aligned to four bytes!');
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniform4uiv(GL.uniforms[location], HEAPU32, value>>2, count*4);
    } else {
      GLctx.uniform4uiv(GL.uniforms[location], {{{ makeHEAPView('U32', 'value', 'value+count*16') }}});
    }
  },

  glUniformMatrix2x3fv__sig: 'viiii',
  glUniformMatrix2x3fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix2x3fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix2x3fv must be aligned to four bytes!');
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniformMatrix2x3fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*6);
    } else {
      GLctx.uniformMatrix2x3fv(GL.uniforms[location], !!transpose, {{{ makeHEAPView('F32', 'value', 'value+count*24') }}});
    }
  },

  glUniformMatrix3x2fv__sig: 'viiii',
  glUniformMatrix3x2fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix3x2fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix3x2fv must be aligned to four bytes!');
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniformMatrix3x2fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*6);
    } else {
      GLctx.uniformMatrix3x2fv(GL.uniforms[location], !!transpose, {{{ makeHEAPView('F32', 'value', 'value+count*24') }}});
    }
  },

  glUniformMatrix2x4fv__sig: 'viiii',
  glUniformMatrix2x4fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix2x4fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix2x4fv must be aligned to four bytes!');
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniformMatrix2x4fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*8);
    } else {
      GLctx.uniformMatrix2x4fv(GL.uniforms[location], !!transpose, {{{ makeHEAPView('F32', 'value', 'value+count*32') }}});
    }
  },

  glUniformMatrix4x2fv__sig: 'viiii',
  glUniformMatrix4x2fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix4x2fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix4x2fv must be aligned to four bytes!');
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniformMatrix4x2fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*8);
    } else {
      GLctx.uniformMatrix4x2fv(GL.uniforms[location], !!transpose, {{{ makeHEAPView('F32', 'value', 'value+count*32') }}});
    }
  },

  glUniformMatrix3x4fv__sig: 'viiii',
  glUniformMatrix3x4fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix3x4fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix3x4fv must be aligned to four bytes!');
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniformMatrix3x4fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*12);
    } else {
      GLctx.uniformMatrix3x4fv(GL.uniforms[location], !!transpose, {{{ makeHEAPView('F32', 'value', 'value+count*48') }}});
    }
  },

  glUniformMatrix4x3fv__sig: 'viiii',
  glUniformMatrix4x3fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.uniforms, location, 'glUniformMatrix4x3fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix4x3fv must be aligned to four bytes!');
#endif
    if (GL.currentContext.supportsWebGL2EntryPoints) { // WebGL 2 provides new garbage-free entry points to call to WebGL. Use those always when possible.
      GLctx.uniformMatrix4x3fv(GL.uniforms[location], !!transpose, HEAPF32, value>>2, count*12);
    } else {
      GLctx.uniformMatrix4x3fv(GL.uniforms[location], !!transpose, {{{ makeHEAPView('F32', 'value', 'value+count*48') }}});
    }
  },

  glVertexAttribI4iv__sig: 'vii',
  glVertexAttribI4iv: function(index, v) {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to integer data passed to glVertexAttribI4iv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttribI4iv!');
#endif
    GLctx.vertexAttribI4i(index, HEAP32[v>>2], HEAP32[v+4>>2], HEAP32[v+8>>2], HEAP32[v+12>>2]);
  },

  glVertexAttribI4uiv__sig: 'vii',
  glVertexAttribI4uiv: function(index, v) {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to integer data passed to glVertexAttribI4uiv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttribI4uiv!');
#endif
    GLctx.vertexAttribI4ui(index, HEAPU32[v>>2], HEAPU32[v+4>>2], HEAPU32[v+8>>2], HEAPU32[v+12>>2]);
  },

  glProgramParameteri__sig: 'viii',
  glProgramParameteri: function(program, pname, value) {
    GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
    err("GL_INVALID_ENUM in glProgramParameteri: WebGL does not support binary shader formats! Calls to glProgramParameteri always fail. See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.4");
#endif
  },

  glGetProgramBinary__sig: 'viiiii',
  glGetProgramBinary: function(program, bufSize, length, binaryFormat, binary) {
    GL.recordError(0x0502/*GL_INVALID_OPERATION*/);
#if GL_ASSERTIONS
    err("GL_INVALID_OPERATION in glGetProgramBinary: WebGL does not support binary shader formats! Calls to glGetProgramBinary always fail. See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.4");
#endif
  },

  glProgramBinary__sig: 'viiii',
  glProgramBinary: function(program, binaryFormat, binary, length) {
    GL.recordError(0x0500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
    err("GL_INVALID_ENUM in glProgramBinary: WebGL does not support binary shader formats! Calls to glProgramBinary always fail. See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.4");
#endif
  },

  glFramebufferTextureLayer__sig: 'viiiii',
  glFramebufferTextureLayer: function(target, attachment, texture, level, layer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.textures, texture, 'glFramebufferTextureLayer', 'texture');
#endif
    GLctx.framebufferTextureLayer(target, attachment, GL.textures[texture], level, layer);
  },

  glVertexAttribIPointer__sig: 'viiiii',
  glVertexAttribIPointer: function(index, size, type, stride, ptr) {
#if FULL_ES3
    var cb = GL.currentContext.clientBuffers[index];
#if GL_ASSERTIONS
    assert(cb, index);
#endif
    if (!GL.currArrayBuffer) {
      cb.size = size;
      cb.type = type;
      cb.normalized = false;
      cb.stride = stride;
      cb.ptr = ptr;
      cb.clientside = true;
      cb.vertexAttribPointerAdaptor = function(index, size, type, normalized, stride, ptr) {
        this.vertexAttribIPointer(index, size, type, stride, ptr);
      };
      return;
    }
    cb.clientside = false;
#endif
#if GL_ASSERTIONS
    GL.validateVertexAttribPointer(size, type, stride, ptr);
#endif
    GLctx['vertexAttribIPointer'](index, size, type, stride, ptr);
  },

  glDrawRangeElements__sig: 'viiiiii',
  glDrawRangeElements__deps: ['glDrawElements'],
  glDrawRangeElements: function(mode, start, end, count, type, indices) {
    // TODO: This should be a trivial pass-though function registered at the bottom of this page as
    // glFuncs[6][1] += ' drawRangeElements';
    // but due to https://bugzilla.mozilla.org/show_bug.cgi?id=1202427,
    // we work around by ignoring the range.
    _glDrawElements(mode, count, type, indices);
  },

  glVertexAttribI4i__sig: 'viiiii',
  glVertexAttribI4ui__sig: 'viiiii',
  glCopyBufferSubData__sig: 'viiiii',
  glTexStorage2D__sig: 'viiiii',
  glTexStorage3D__sig: 'viiiiii',
  glBeginTransformFeedback__sig: 'vi',
  glEndTransformFeedback__sig: 'v',
  glPauseTransformFeedback__sig: 'v',
  glResumeTransformFeedback__sig: 'v',
  glBlitFramebuffer__sig: 'viiiiiiiiii',
  glReadBuffer__sig: 'vi',
  glEndQuery__sig: 'vi',
  glRenderbufferStorageMultisample__sig: 'viiiii',
  glCopyTexSubImage3D__sig: 'viiiiiiiii',
  glClearBufferfi__sig: 'viifi',
};

// Simple pass-through functions. Starred ones have return values. [X] ones have X in the C name but not in the JS name
var webgl2Funcs = [[0, 'endTransformFeedback pauseTransformFeedback resumeTransformFeedback'],
 [1, 'beginTransformFeedback readBuffer endQuery'],
 [2, ''],
 [3, ''],
 [4, 'clearBufferfi'],
 [5, 'vertexAttribI4i vertexAttribI4ui copyBufferSubData texStorage2D renderbufferStorageMultisample'],
 [6, 'texStorage3D'],
 [7, ''],
 [8, ''],
 [9, 'copyTexSubImage3D'],
 [10, 'blitFramebuffer']];

#if USE_WEBGL2

// If user passes -s USE_WEBGL2=1 -s STRICT=1 but not -lGL (to link in WebGL 1), then WebGL2 library should not
// be linked in as well.
if (typeof createGLPassthroughFunctions === 'undefined') {
  throw 'In order to use WebGL 2 in strict mode with -s USE_WEBGL2=1, you need to link in WebGL support with -lGL!';
}

createGLPassthroughFunctions(LibraryWebGL2, webgl2Funcs);

recordGLProcAddressGet(LibraryWebGL2);

mergeInto(LibraryManager.library, LibraryWebGL2);

#endif
