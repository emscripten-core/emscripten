/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryWebGL2 = {
  glGetStringi__deps: ['$stringToNewUTF8'],
  glGetStringi: function(name, index) {
    if (GL.currentContext.version < 2) {
      GL.recordError(0x502 /* GL_INVALID_OPERATION */); // Calling GLES3/WebGL2 function with a GLES2/WebGL1 context
      return 0;
    }
    var stringiCache = GL.stringiCache[name];
    if (stringiCache) {
      if (index < 0 || index >= stringiCache.length) {
        GL.recordError(0x501/*GL_INVALID_VALUE*/);
#if GL_ASSERTIONS
        err('GL_INVALID_VALUE in glGetStringi: index out of range (' + index + ')!');
#endif
        return 0;
      }
      return stringiCache[index];
    }
    switch (name) {
      case 0x1F03 /* GL_EXTENSIONS */:
        var exts = GLctx.getSupportedExtensions() || []; // .getSupportedExtensions() can return null if context is lost, so coerce to empty array.
#if GL_EXTENSIONS_IN_PREFIXED_FORMAT
        exts = exts.concat(exts.map(function(e) { return "GL_" + e; }));
#endif
        exts = exts.map(function(e) { return stringToNewUTF8(e); });

        stringiCache = GL.stringiCache[name] = exts;
        if (index < 0 || index >= stringiCache.length) {
          GL.recordError(0x501/*GL_INVALID_VALUE*/);
#if GL_ASSERTIONS
          err('GL_INVALID_VALUE in glGetStringi: index out of range (' + index + ') in a call to GL_EXTENSIONS!');
#endif
          return 0;
        }
        return stringiCache[index];
      default:
        GL.recordError(0x500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
        err('GL_INVALID_ENUM in glGetStringi: Unknown parameter ' + name + '!');
#endif
        return 0;
    }
  },

  glGetInteger64v__deps: ['$emscriptenWebGLGet'],
  glGetInteger64v: function(name_, p) {
    emscriptenWebGLGet(name_, p, {{{ cDefs.EM_FUNC_SIG_PARAM_I64 }}});
  },

  glGetInternalformativ: function(target, internalformat, pname, bufSize, params) {
#if GL_TRACK_ERRORS
    if (bufSize < 0) {
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetInternalformativ(target=' + target + ', internalformat=' + internalformat + ', pname=' + pname + ', bufSize=' + bufSize + ', params=' + params + '): Function called with bufSize < 0!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    if (!params) {
      // GLES3 specification does not specify how to behave if values is a null pointer. Since calling this function does not make sense
      // if values == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetInternalformativ(target=' + target + ', internalformat=' + internalformat + ', pname=' + pname + ', bufSize=' + bufSize + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    var ret = GLctx.getInternalformatParameter(target, internalformat, pname);
    if (ret === null) return;
    for (var i = 0; i < ret.length && i < bufSize; ++i) {
      {{{ makeSetValue('params', 'i*4', 'ret[i]', 'i32') }}};
    }
  },

  glCompressedTexImage3D: function(target, level, internalFormat, width, height, depth, border, imageSize, data) {
    if (GLctx.currentPixelUnpackBufferBinding) {
      GLctx.compressedTexImage3D(target, level, internalFormat, width, height, depth, border, imageSize, data);
    } else {
      GLctx.compressedTexImage3D(target, level, internalFormat, width, height, depth, border, HEAPU8, data, imageSize);
    }
  },

  glCompressedTexSubImage3D: function(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) {
    if (GLctx.currentPixelUnpackBufferBinding) {
      GLctx.compressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
    } else {
      GLctx.compressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, HEAPU8, data, imageSize);
    }
  },

  glGetBufferParameteri64v__deps: ['$writeI53ToI64'],
  glGetBufferParameteri64v: function(target, value, data) {
#if GL_TRACK_ERRORS
    if (!data) {
      // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
      // if data == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetBufferParameteri64v(target=' + target + ', value=' + value + ', data=0): Function called with null out data pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    writeI53ToI64(data, GLctx.getBufferParameter(target, value));
  },

  glGetBufferSubData: function(target, offset, size, data) {
#if GL_TRACK_ERRORS
    if (!data) {
      // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
      // if data == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetBufferSubData(target=' + target + ', offset=' + offset + ', size=' + size + ', data=0): Function called with null out data pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    size && GLctx.getBufferSubData(target, offset, HEAPU8, data, size);
  },

  glInvalidateFramebuffer__deps: ['$tempFixedLengthArray'],
  glInvalidateFramebuffer: function(target, numAttachments, attachments) {
#if GL_ASSERTIONS
    assert(numAttachments < tempFixedLengthArray.length, 'Invalid count of numAttachments=' + numAttachments + ' passed to glInvalidateFramebuffer (that many attachment points do not exist in GL)');
#endif
    var list = tempFixedLengthArray[numAttachments];
    for (var i = 0; i < numAttachments; i++) {
      list[i] = {{{ makeGetValue('attachments', 'i*4', 'i32') }}};
    }

    GLctx.invalidateFramebuffer(target, list);
  },

  glInvalidateSubFramebuffer__deps: ['$tempFixedLengthArray'],
  glInvalidateSubFramebuffer: function(target, numAttachments, attachments, x, y, width, height) {
#if GL_ASSERTIONS
    assert(numAttachments < tempFixedLengthArray.length, 'Invalid count of numAttachments=' + numAttachments + ' passed to glInvalidateSubFramebuffer (that many attachment points do not exist in GL)');
#endif
    var list = tempFixedLengthArray[numAttachments];
    for (var i = 0; i < numAttachments; i++) {
      list[i] = {{{ makeGetValue('attachments', 'i*4', 'i32') }}};
    }

    GLctx.invalidateSubFramebuffer(target, list, x, y, width, height);
  },

  glTexImage3D__deps: ['$heapObjectForWebGLType', '$heapAccessShiftForWebGLHeap'],
  glTexImage3D: function(target, level, internalFormat, width, height, depth, border, format, type, pixels) {
    if (GLctx.currentPixelUnpackBufferBinding) {
      GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixels);
    } else if (pixels) {
      var heap = heapObjectForWebGLType(type);
      GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, heap, pixels >> heapAccessShiftForWebGLHeap(heap));
    } else {
      GLctx.texImage3D(target, level, internalFormat, width, height, depth, border, format, type, null);
    }
  },

  glTexSubImage3D__deps: ['$heapObjectForWebGLType', '$heapAccessShiftForWebGLHeap'],
  glTexSubImage3D: function(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) {
    if (GLctx.currentPixelUnpackBufferBinding) {
      GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    } else if (pixels) {
      var heap = heapObjectForWebGLType(type);
      GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, heap, pixels >> heapAccessShiftForWebGLHeap(heap));
    } else {
      GLctx.texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, null);
    }
  },

  // Queries
  glGenQueries__deps: ['$__glGenObject'],
  glGenQueries: function(n, ids) {
    __glGenObject(n, ids, 'createQuery', GL.queries
#if GL_ASSERTIONS
    , 'glGenQueries'
#endif
      );
  },

  glDeleteQueries: function(n, ids) {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('ids', 'i*4', 'i32') }}};
      var query = GL.queries[id];
      if (!query) continue; // GL spec: "unused names in ids are ignored, as is the name zero."
      GLctx.deleteQuery(query);
      GL.queries[id] = null;
    }
  },

  glIsQuery: function(id) {
    var query = GL.queries[id];
    if (!query) return 0;
    return GLctx.isQuery(query);
  },

  glBeginQuery: function(target, id) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.queries, id, 'glBeginQuery', 'id');
#endif
    GLctx.beginQuery(target, GL.queries[id]);
  },

  glGetQueryiv: function(target, pname, params) {
#if GL_TRACK_ERRORS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetQueryiv(target=' + target +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    {{{ makeSetValue('params', '0', 'GLctx.getQuery(target, pname)', 'i32') }}};
  },

  glGetQueryObjectuiv: function(id, pname, params) {
#if GL_TRACK_ERRORS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetQueryObjectuiv(id=' + id +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
#if GL_ASSERTIONS
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

  // Sampler objects
  glGenSamplers__deps: ['$__glGenObject'],
  glGenSamplers: function(n, samplers) {
    __glGenObject(n, samplers, 'createSampler', GL.samplers
#if GL_ASSERTIONS
    , 'glGenSamplers'
#endif
      );
  },

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

  glIsSampler: function(id) {
    var sampler = GL.samplers[id];
    if (!sampler) return 0;
    return GLctx.isSampler(sampler);
  },

  glBindSampler: function(unit, sampler) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    GLctx.bindSampler(unit, GL.samplers[sampler]);
  },

  glSamplerParameterf: function(sampler, pname, param) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    GLctx.samplerParameterf(GL.samplers[sampler], pname, param);
  },

  glSamplerParameteri: function(sampler, pname, param) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    GLctx.samplerParameteri(GL.samplers[sampler], pname, param);
  },

  glSamplerParameterfv: function(sampler, pname, params) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    var param = {{{ makeGetValue('params', '0', 'float') }}};
    GLctx.samplerParameterf(GL.samplers[sampler], pname, param);
  },

  glSamplerParameteriv: function(sampler, pname, params) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.samplers, sampler, 'glBindSampler', 'sampler');
#endif
    var param = {{{ makeGetValue('params', '0', 'i32') }}};
    GLctx.samplerParameteri(GL.samplers[sampler], pname, param);
  },

  glGetSamplerParameterfv: function(sampler, pname, params) {
#if GL_TRACK_ERRORS
    if (!params) {
      // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetSamplerParameterfv(sampler=' + sampler +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    {{{ makeSetValue('params', '0', 'GLctx.getSamplerParameter(GL.samplers[sampler], pname)', 'float') }}};
  },

  glGetSamplerParameteriv: function(sampler, pname, params) {
#if GL_TRACK_ERRORS
    if (!params) {
      // GLES3 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if p == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetSamplerParameteriv(sampler=' + sampler +', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    {{{ makeSetValue('params', '0', 'GLctx.getSamplerParameter(GL.samplers[sampler], pname)', 'i32') }}};
  },

  // Transform Feedback
  glGenTransformFeedbacks__deps: ['$__glGenObject'],
  glGenTransformFeedbacks: function(n, ids) {
    __glGenObject(n, ids, 'createTransformFeedback', GL.transformFeedbacks
#if GL_ASSERTIONS
    , 'glGenTransformFeedbacks'
#endif
      );
  },

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

  glIsTransformFeedback: function(id) {
    return GLctx.isTransformFeedback(GL.transformFeedbacks[id]);
  },

  glBindTransformFeedback: function(target, id) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.transformFeedbacks, id, 'glBindTransformFeedback', 'id');
#endif
    GLctx.bindTransformFeedback(target, GL.transformFeedbacks[id]);
  },

  glTransformFeedbackVaryings: function(program, count, varyings, bufferMode) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glTransformFeedbackVaryings', 'program');
#endif
    program = GL.programs[program];
    var vars = [];
    for (var i = 0; i < count; i++)
      vars.push(UTF8ToString({{{ makeGetValue('varyings', 'i*4', 'i32') }}}));

    GLctx.transformFeedbackVaryings(program, vars, bufferMode);
  },

  glGetTransformFeedbackVarying: function(program, index, bufSize, length, size, type, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetTransformFeedbackVarying', 'program');
#endif
    program = GL.programs[program];
    var info = GLctx.getTransformFeedbackVarying(program, index);
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

  $emscriptenWebGLGetIndexed__deps: ['$writeI53ToI64'],
  $emscriptenWebGLGetIndexed: function(target, index, data, type) {
#if GL_TRACK_ERRORS
    if (!data) {
      // GLES2 specification does not specify how to behave if data is a null pointer. Since calling this function does not make sense
      // if data == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetInteger(64)i_v(target=' + target + ', index=' + index + ', data=0): Function called with null out pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
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
              GL.recordError(0x500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
              err('GL_INVALID_ENUM in glGetInteger(64)i_v(' + target + ') and it returns null!');
#endif
              return;
            }
          }
        } else if (result instanceof WebGLBuffer) {
          ret = result.name | 0;
        } else {
          GL.recordError(0x500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
          err('GL_INVALID_ENUM in glGetInteger(64)i_v: Unknown object returned from WebGL getIndexedParameter(' + target + ')!');
#endif
          return;
        }
        break;
      default:
        GL.recordError(0x500); // GL_INVALID_ENUM
#if GL_ASSERTIONS
        err('GL_INVALID_ENUM in glGetInteger(64)i_v: Native code calling glGetInteger(64)i_v(' + target + ') and it returns ' + result + ' of type ' + typeof(result) + '!');
#endif
        return;
    }

    switch (type) {
      case {{{ cDefs.EM_FUNC_SIG_PARAM_I64 }}}: writeI53ToI64(data, ret); break;
      case {{{ cDefs.EM_FUNC_SIG_PARAM_I }}}: {{{ makeSetValue('data', '0', 'ret', 'i32') }}}; break;
      case {{{ cDefs.EM_FUNC_SIG_PARAM_F }}}: {{{ makeSetValue('data', '0', 'ret', 'float') }}}; break;
      case {{{ cDefs.EM_FUNC_SIG_PARAM_B }}}: {{{ makeSetValue('data', '0', 'ret ? 1 : 0', 'i8') }}}; break;
      default: throw 'internal emscriptenWebGLGetIndexed() error, bad type: ' + type;
    }
  },

  glGetIntegeri_v__deps: ['$emscriptenWebGLGetIndexed'],
  glGetIntegeri_v: function(target, index, data) {
    emscriptenWebGLGetIndexed(target, index, data, {{{ cDefs.EM_FUNC_SIG_PARAM_I }}});
  },

  glGetInteger64i_v__deps: ['$emscriptenWebGLGetIndexed'],
  glGetInteger64i_v: function(target, index, data) {
    emscriptenWebGLGetIndexed(target, index, data, {{{ cDefs.EM_FUNC_SIG_PARAM_I64 }}});
  },

  // Uniform Buffer objects
  glBindBufferBase: function(target, index, buffer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.buffers, buffer, 'glBindBufferBase', 'buffer');
#endif
    GLctx.bindBufferBase(target, index, GL.buffers[buffer]);
  },

  glBindBufferRange: function(target, index, buffer, offset, ptrsize) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.buffers, buffer, 'glBindBufferRange', 'buffer');
#endif
    GLctx.bindBufferRange(target, index, GL.buffers[buffer], offset, ptrsize);
  },

  glGetUniformIndices: function(program, uniformCount, uniformNames, uniformIndices) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformIndices', 'program');
#endif
#if GL_TRACK_ERRORS
    if (!uniformIndices) {
      // GLES2 specification does not specify how to behave if uniformIndices is a null pointer. Since calling this function does not make sense
      // if uniformIndices == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetUniformIndices(program=' + program + ', uniformCount=' + uniformCount + ', uniformNames=' + uniformNames + ', uniformIndices=0): Function called with null out pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    if (uniformCount > 0 && (uniformNames == 0 || uniformIndices == 0)) {
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    program = GL.programs[program];
    var names = [];
    for (var i = 0; i < uniformCount; i++)
      names.push(UTF8ToString({{{ makeGetValue('uniformNames', 'i*4', 'i32') }}}));

    var result = GLctx.getUniformIndices(program, names);
    if (!result) return; // GL spec: If an error is generated, nothing is written out to uniformIndices.

    var len = result.length;
    for (var i = 0; i < len; i++) {
      {{{ makeSetValue('uniformIndices', 'i*4', 'result[i]', 'i32') }}};
    }
  },

  glGetActiveUniformsiv: function(program, uniformCount, uniformIndices, pname, params) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveUniformsiv', 'program');
#endif
#if GL_TRACK_ERRORS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetActiveUniformsiv(program=' + program + ', uniformCount=' + uniformCount + ', uniformIndices=' + uniformIndices + ', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    if (uniformCount > 0 && uniformIndices == 0) {
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
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

  glGetUniformBlockIndex: function(program, uniformBlockName) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetUniformBlockIndex', 'program');
#endif
    return GLctx.getUniformBlockIndex(GL.programs[program], UTF8ToString(uniformBlockName));
  },

  glGetActiveUniformBlockiv: function(program, uniformBlockIndex, pname, params) {
#if GL_TRACK_ERRORS
    if (!params) {
      // GLES2 specification does not specify how to behave if params is a null pointer. Since calling this function does not make sense
      // if params == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetActiveUniformBlockiv(program=' + program + ', uniformBlockIndex=' + uniformBlockIndex + ', pname=' + pname + ', params=0): Function called with null out pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveUniformBlockiv', 'program');
#endif
    program = GL.programs[program];

    if (pname == 0x8A41 /* GL_UNIFORM_BLOCK_NAME_LENGTH */) {
      var name = GLctx.getActiveUniformBlockName(program, uniformBlockIndex);
      {{{ makeSetValue('params', 0, 'name.length+1', 'i32') }}};
      return;
    }

    var result = GLctx.getActiveUniformBlockParameter(program, uniformBlockIndex, pname);
    if (result === null) return; // If an error occurs, nothing should be written to params.
    if (pname == 0x8A43 /*GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES*/) {
      for (var i = 0; i < result.length; i++) {
        {{{ makeSetValue('params', 'i*4', 'result[i]', 'i32') }}};
      }
    } else {
      {{{ makeSetValue('params', '0', 'result', 'i32') }}};
    }
  },

  glGetActiveUniformBlockName: function(program, uniformBlockIndex, bufSize, length, uniformBlockName) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetActiveUniformBlockName', 'program');
#endif
    program = GL.programs[program];

    var result = GLctx.getActiveUniformBlockName(program, uniformBlockIndex);
    if (!result) return; // If an error occurs, nothing will be written to uniformBlockName or length.
    if (uniformBlockName && bufSize > 0) {
      var numBytesWrittenExclNull = stringToUTF8(result, uniformBlockName, bufSize);
      if (length) {{{ makeSetValue('length', '0', 'numBytesWrittenExclNull', 'i32') }}};
    } else {
      if (length) {{{ makeSetValue('length', '0', 0, 'i32') }}};
    }
  },

  glUniformBlockBinding: function(program, uniformBlockIndex, uniformBlockBinding) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glUniformBlockBinding', 'program');
#endif
    program = GL.programs[program];

    GLctx.uniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
  },

  glClearBufferiv: function(buffer, drawbuffer, value) {
#if GL_ASSERTIONS
    assert((value & 3) == 0, 'Pointer to integer data passed to glClearBufferiv must be aligned to four bytes!');
#endif

    GLctx.clearBufferiv(buffer, drawbuffer, HEAP32, value>>2);
  },

  glClearBufferuiv: function(buffer, drawbuffer, value) {
#if GL_ASSERTIONS
    assert((value & 3) == 0, 'Pointer to integer data passed to glClearBufferuiv must be aligned to four bytes!');
#endif

    GLctx.clearBufferuiv(buffer, drawbuffer, HEAPU32, value>>2);
  },

  glClearBufferfv: function(buffer, drawbuffer, value) {
#if GL_ASSERTIONS
    assert((value & 3) == 0, 'Pointer to float data passed to glClearBufferfv must be aligned to four bytes!');
#endif

    GLctx.clearBufferfv(buffer, drawbuffer, HEAPF32, value>>2);
  },

  glFenceSync: function(condition, flags) {
    var sync = GLctx.fenceSync(condition, flags);
    if (sync) {
      var id = GL.getNewId(GL.syncs);
      sync.name = id;
      GL.syncs[id] = sync;
      return id;
    }
    return 0; // Failed to create a sync object
  },

  glDeleteSync: function(id) {
    if (!id) return;
    var sync = GL.syncs[id];
    if (!sync) { // glDeleteSync signals an error when deleting a nonexisting object, unlike some other GL delete functions.
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    GLctx.deleteSync(sync);
    sync.name = 0;
    GL.syncs[id] = null;
  },

#if !WASM_BIGINT
  glClientWaitSync__deps: ['$convertI32PairToI53'],
#endif
  glClientWaitSync: function(sync, flags, {{{ defineI64Param('timeout') }}}) {
    // WebGL2 vs GLES3 differences: in GLES3, the timeout parameter is a uint64, where 0xFFFFFFFFFFFFFFFFULL means GL_TIMEOUT_IGNORED.
    // In JS, there's no 64-bit value types, so instead timeout is taken to be signed, and GL_TIMEOUT_IGNORED is given value -1.
    // Inherently the value accepted in the timeout is lossy, and can't take in arbitrary u64 bit pattern (but most likely doesn't matter)
    // See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15
    {{{ receiveI64ParamAsI53Unchecked('timeout'); }}}
    return GLctx.clientWaitSync(GL.syncs[sync], flags, timeout);
  },

#if !WASM_BIGINT
  glWaitSync__deps: ['$convertI32PairToI53'],
#endif
  glWaitSync: function(sync, flags, {{{ defineI64Param('timeout') }}}) {
    // See WebGL2 vs GLES3 difference on GL_TIMEOUT_IGNORED above (https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.15)
    {{{ receiveI64ParamAsI53Unchecked('timeout'); }}}
    GLctx.waitSync(GL.syncs[sync], flags, timeout);
  },

  glGetSynciv: function(sync, pname, bufSize, length, values) {
#if GL_TRACK_ERRORS
    if (bufSize < 0) {
      // GLES3 specification does not specify how to behave if bufSize < 0, however in the spec wording for glGetInternalformativ, it does say that GL_INVALID_VALUE should be raised,
      // so raise GL_INVALID_VALUE here as well.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetSynciv(sync=' + sync + ', pname=' + pname + ', bufSize=' + bufSize + ', length=' + length + ', values='+values+'): Function called with bufSize < 0!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
    if (!values) {
      // GLES3 specification does not specify how to behave if values is a null pointer. Since calling this function does not make sense
      // if values == null, issue a GL error to notify user about it.
#if GL_ASSERTIONS
      err('GL_INVALID_VALUE in glGetSynciv(sync=' + sync + ', pname=' + pname + ', bufSize=' + bufSize + ', length=' + length + ', values=0): Function called with null out pointer!');
#endif
      GL.recordError(0x501 /* GL_INVALID_VALUE */);
      return;
    }
#endif
    var ret = GLctx.getSyncParameter(GL.syncs[sync], pname);
    if (ret !== null) {
      {{{ makeSetValue('values', '0', 'ret', 'i32') }}};
      if (length) {{{ makeSetValue('length', '0', '1', 'i32') }}}; // Report a single value outputted.
    }
  },

  glIsSync: function(sync) {
    return GLctx.isSync(GL.syncs[sync]);
  },

  glGetUniformuiv__deps: ['$emscriptenWebGLGetUniform'],
  glGetUniformuiv: function(program, location, params) {
    emscriptenWebGLGetUniform(program, location, params, {{{ cDefs.EM_FUNC_SIG_PARAM_I }}});
  },

  glGetFragDataLocation: function(program, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetFragDataLocation', 'program');
#endif
    return GLctx.getFragDataLocation(GL.programs[program], UTF8ToString(name));
  },

  glGetVertexAttribIiv__deps: ['$emscriptenWebGLGetVertexAttrib'],
  glGetVertexAttribIiv: function(index, pname, params) {
    // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttribI4iv(),
    // otherwise the results are undefined. (GLES3 spec 6.1.12)
    emscriptenWebGLGetVertexAttrib(index, pname, params, {{{ cDefs.EM_FUNC_SIG_PARAM_I }}});
  },

  // N.B. This function may only be called if the vertex attribute was specified using the function glVertexAttribI4uiv(),
  // otherwise the results are undefined. (GLES3 spec 6.1.12)
  glGetVertexAttribIuiv__deps: ['$emscriptenWebGLGetVertexAttrib'],
  glGetVertexAttribIuiv: 'glGetVertexAttribIiv',

  glUniform1ui__deps: ['$webglGetUniformLocation'],
  glUniform1ui: function(location, v0) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform1ui', 'location');
#endif
    GLctx.uniform1ui(webglGetUniformLocation(location), v0);
  },

  glUniform2ui__deps: ['$webglGetUniformLocation'],
  glUniform2ui: function(location, v0, v1) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform2ui', 'location');
#endif
    GLctx.uniform2ui(webglGetUniformLocation(location), v0, v1);
  },

  glUniform3ui__deps: ['$webglGetUniformLocation'],
  glUniform3ui: function(location, v0, v1, v2) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform3ui', 'location');
#endif
    GLctx.uniform3ui(webglGetUniformLocation(location), v0, v1, v2);
  },

  glUniform4ui__deps: ['$webglGetUniformLocation'],
  glUniform4ui: function(location, v0, v1, v2, v3) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform4ui', 'location');
#endif
    GLctx.uniform4ui(webglGetUniformLocation(location), v0, v1, v2, v3);
  },

  glUniform1uiv__deps: ['$webglGetUniformLocation'],
  glUniform1uiv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform1uiv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform1uiv must be aligned to four bytes!');
#endif
    count && GLctx.uniform1uiv(webglGetUniformLocation(location), HEAPU32, value>>2, count);
  },

  glUniform2uiv__deps: ['$webglGetUniformLocation'],
  glUniform2uiv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform2uiv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform2uiv must be aligned to four bytes!');
#endif
    count && GLctx.uniform2uiv(webglGetUniformLocation(location), HEAPU32, value>>2, count*2);
  },

  glUniform3uiv__deps: ['$webglGetUniformLocation'],
  glUniform3uiv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform3uiv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform3uiv must be aligned to four bytes!');
#endif
    count && GLctx.uniform3uiv(webglGetUniformLocation(location), HEAPU32, value>>2, count*3);
  },

  glUniform4uiv__deps: ['$webglGetUniformLocation'],
  glUniform4uiv: function(location, count, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniform4uiv', 'location');
    assert((value & 3) == 0, 'Pointer to integer data passed to glUniform4uiv must be aligned to four bytes!');
#endif
    count && GLctx.uniform4uiv(webglGetUniformLocation(location), HEAPU32, value>>2, count*4);
  },

  glUniformMatrix2x3fv__deps: ['$webglGetUniformLocation'],
  glUniformMatrix2x3fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniformMatrix2x3fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix2x3fv must be aligned to four bytes!');
#endif
    count && GLctx.uniformMatrix2x3fv(webglGetUniformLocation(location), !!transpose, HEAPF32, value>>2, count*6);
  },

  glUniformMatrix3x2fv__deps: ['$webglGetUniformLocation'],
  glUniformMatrix3x2fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniformMatrix3x2fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix3x2fv must be aligned to four bytes!');
#endif
    count && GLctx.uniformMatrix3x2fv(webglGetUniformLocation(location), !!transpose, HEAPF32, value>>2, count*6);
  },

  glUniformMatrix2x4fv__deps: ['$webglGetUniformLocation'],
  glUniformMatrix2x4fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniformMatrix2x4fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix2x4fv must be aligned to four bytes!');
#endif
    count && GLctx.uniformMatrix2x4fv(webglGetUniformLocation(location), !!transpose, HEAPF32, value>>2, count*8);
  },

  glUniformMatrix4x2fv__deps: ['$webglGetUniformLocation'],
  glUniformMatrix4x2fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniformMatrix4x2fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix4x2fv must be aligned to four bytes!');
#endif
    count && GLctx.uniformMatrix4x2fv(webglGetUniformLocation(location), !!transpose, HEAPF32, value>>2, count*8);
  },

  glUniformMatrix3x4fv__deps: ['$webglGetUniformLocation'],
  glUniformMatrix3x4fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniformMatrix3x4fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix3x4fv must be aligned to four bytes!');
#endif
    count && GLctx.uniformMatrix3x4fv(webglGetUniformLocation(location), !!transpose, HEAPF32, value>>2, count*12);
  },

  glUniformMatrix4x3fv__deps: ['$webglGetUniformLocation'],
  glUniformMatrix4x3fv: function(location, count, transpose, value) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GLctx.currentProgram.uniformLocsById, location, 'glUniformMatrix4x3fv', 'location');
    assert((value & 3) == 0, 'Pointer to float data passed to glUniformMatrix4x3fv must be aligned to four bytes!');
#endif
    count && GLctx.uniformMatrix4x3fv(webglGetUniformLocation(location), !!transpose, HEAPF32, value>>2, count*12);
  },

  glVertexAttribI4iv: function(index, v) {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to integer data passed to glVertexAttribI4iv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttribI4iv!');
#endif
    GLctx.vertexAttribI4i(index, HEAP32[v>>2], HEAP32[v+4>>2], HEAP32[v+8>>2], HEAP32[v+12>>2]);
  },

  glVertexAttribI4uiv: function(index, v) {
#if GL_ASSERTIONS
    assert((v & 3) == 0, 'Pointer to integer data passed to glVertexAttribI4uiv must be aligned to four bytes!');
    assert(v != 0, 'Null pointer passed to glVertexAttribI4uiv!');
#endif
    GLctx.vertexAttribI4ui(index, HEAPU32[v>>2], HEAPU32[v+4>>2], HEAPU32[v+8>>2], HEAPU32[v+12>>2]);
  },

  glProgramParameteri: function(program, pname, value) {
    GL.recordError(0x500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
    err("GL_INVALID_ENUM in glProgramParameteri: WebGL does not support binary shader formats! Calls to glProgramParameteri always fail. See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.4");
#endif
  },

  glGetProgramBinary: function(program, bufSize, length, binaryFormat, binary) {
    GL.recordError(0x502/*GL_INVALID_OPERATION*/);
#if GL_ASSERTIONS
    err("GL_INVALID_OPERATION in glGetProgramBinary: WebGL does not support binary shader formats! Calls to glGetProgramBinary always fail. See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.4");
#endif
  },

  glProgramBinary: function(program, binaryFormat, binary, length) {
    GL.recordError(0x500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
    err("GL_INVALID_ENUM in glProgramBinary: WebGL does not support binary shader formats! Calls to glProgramBinary always fail. See https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.4");
#endif
  },

  glFramebufferTextureLayer: function(target, attachment, texture, level, layer) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.textures, texture, 'glFramebufferTextureLayer', 'texture');
#endif
    GLctx.framebufferTextureLayer(target, attachment, GL.textures[texture], level, layer);
  },

  glVertexAttribIPointer: function(index, size, type, stride, ptr) {
#if FULL_ES3
    var cb = GL.currentContext.clientBuffers[index];
#if GL_ASSERTIONS
    assert(cb, index);
#endif
    if (!GLctx.currentArrayBufferBinding) {
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
    GLctx.vertexAttribIPointer(index, size, type, stride, ptr);
  },

#if !LEGACY_GL_EMULATION
  // Defined in library_glemu.js when LEGACY_GL_EMULATION is set
  glDrawRangeElements__deps: ['glDrawElements'],
  glDrawRangeElements: function(mode, start, end, count, type, indices) {
    // TODO: This should be a trivial pass-though function registered at the bottom of this page as
    // glFuncs[6][1] += ' drawRangeElements';
    // but due to https://bugzilla.mozilla.org/show_bug.cgi?id=1202427,
    // we work around by ignoring the range.
    _glDrawElements(mode, count, type, indices);
  },
#endif

  glDrawArraysInstancedBaseInstanceWEBGL__sig: 'viiiii',
  glDrawArraysInstancedBaseInstanceWEBGL: function(mode, first, count, instanceCount, baseInstance) {
    GLctx.dibvbi['drawArraysInstancedBaseInstanceWEBGL'](mode, first, count, instanceCount, baseInstance);
  },
  glDrawArraysInstancedBaseInstance: 'glDrawArraysInstancedBaseInstanceWEBGL',
  glDrawArraysInstancedBaseInstanceANGLE: 'glDrawArraysInstancedBaseInstanceWEBGL',

  glDrawElementsInstancedBaseVertexBaseInstanceWEBGL__sig: 'viiiiiii',
  glDrawElementsInstancedBaseVertexBaseInstanceWEBGL: function(mode, count, type, offset, instanceCount, baseVertex, baseinstance) {
    GLctx.dibvbi['drawElementsInstancedBaseVertexBaseInstanceWEBGL'](mode, count, type, offset, instanceCount, baseVertex, baseinstance);
  },
  glDrawElementsInstancedBaseVertexBaseInstanceANGLE: 'glDrawElementsInstancedBaseVertexBaseInstanceWEBGL',

  $webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance: function(ctx) {
    // Closure is expected to be allowed to minify the '.dibvbi' property, so not accessing it quoted.
    return !!(ctx.dibvbi = ctx.getExtension('WEBGL_draw_instanced_base_vertex_base_instance'));
  },

  emscripten_webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance__deps: ['$webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance'],
  emscripten_webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance: function(ctx) {
    return webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GL.contexts[ctx].GLctx);
  },

  glMultiDrawArraysInstancedBaseInstanceWEBGL__sig: 'viiiiii',
  glMultiDrawArraysInstancedBaseInstanceWEBGL: function(mode, firsts, counts, instanceCounts, baseInstances, drawCount) {
    GLctx.mdibvbi['multiDrawArraysInstancedBaseInstanceWEBGL'](
      mode,
      HEAP32,
      firsts >> 2,
      HEAP32,
      counts >> 2,
      HEAP32,
      instanceCounts >> 2,
      HEAPU32,
      baseInstances >> 2,
      drawCount);
  },
  glMultiDrawArraysInstancedBaseInstanceANGLE: 'glMultiDrawArraysInstancedBaseInstanceWEBGL',

  glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL__sig: 'viiiiiiii',
  glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL: function(mode, counts, type, offsets, instanceCounts, baseVertices, baseInstances, drawCount) {
    GLctx.mdibvbi['multiDrawElementsInstancedBaseVertexBaseInstanceWEBGL'](
      mode,
      HEAP32,
      counts >> 2,
      type,
      HEAP32,
      offsets >> 2,
      HEAP32,
      instanceCounts >> 2,
      HEAP32,
      baseVertices >> 2,
      HEAPU32,
      baseInstances >> 2,
      drawCount);
  },
  glMultiDrawElementsInstancedBaseVertexBaseInstanceANGLE: 'glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL',

  $webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance: function(ctx) {
    // Closure is expected to be allowed to minify the '.mdibvbi' property, so not accessing it quoted.
    return !!(ctx.mdibvbi = ctx.getExtension('WEBGL_multi_draw_instanced_base_vertex_base_instance'));
  },

  emscripten_webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance__deps: ['$webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance'],
  emscripten_webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance: function(ctx) {
    return webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GL.contexts[ctx].GLctx);
  },

};

#if MAX_WEBGL_VERSION >= 2

// Simple pass-through functions.
// - Starred ones have return values.
// - [X] ones have X in the C name but not in the JS name
var webgl2PassthroughFuncs = [
  [0, 'endTransformFeedback pauseTransformFeedback resumeTransformFeedback'],
  [1, 'beginTransformFeedback readBuffer endQuery'],
  [4, 'clearBufferfi'],
  [5, 'vertexAttribI4i vertexAttribI4ui copyBufferSubData texStorage2D renderbufferStorageMultisample'],
  [6, 'texStorage3D'],
  [9, 'copyTexSubImage3D'],
  [10, 'blitFramebuffer']
];

// If user passes -sMAX_WEBGL_VERSION >= 2 -sSTRICT but not -lGL (to link in
// WebGL 1), then WebGL2 library should not be linked in as well.
if (typeof createGLPassthroughFunctions == 'undefined') {
  throw 'In order to use WebGL 2 in strict mode with -sMAX_WEBGL_VERSION=2, you need to link in WebGL support with -lGL!';
}

createGLPassthroughFunctions(LibraryWebGL2, webgl2PassthroughFuncs);

recordGLProcAddressGet(LibraryWebGL2);

mergeInto(LibraryManager.library, LibraryWebGL2);

#endif
