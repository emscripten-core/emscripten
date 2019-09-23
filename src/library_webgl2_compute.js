// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

var LibraryWebGL2Compute = {
  glDrawArraysIndirect__sig: 'vii',
  glDrawArraysIndirect: function(mode, indirect) {
    throw 'glDrawArraysIndirect: TODO';
  },

  glDrawElementsIndirect__sig: 'viii',
  glDrawElementsIndirect: function(mode, type, indirect) {
    throw 'glDrawElementsIndirect: TODO';
  },

  glGetFramebufferParameteriv__sig: 'viii',
  glGetFramebufferParameteriv: function(target, pname, params) {
    throw 'glGetFramebufferParameteriv: TODO';
  },

  glGetProgramInterfaceiv__sig: 'viiii',
  glGetProgramInterfaceiv: function(program, programInterface, pname, params) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramInterfaceiv', 'program');
#endif
    switch (pname) {
      case 0x92F5: // GL_ACTIVE_RESOURCES
      case 0x92F6: // GL_MAX_NAME_LENGTH
      case 0x92F7: // GL_MAX_NUM_ACTIVE_VARIABLES
        break;
      default:
        // in case pname is valid for getProgramInterfaceParameter() but the return value is incompatible with an integer
        GL.recordError(0x0500); // GL_INVALID_ENUM
        return;
    }
    var ret = GLctx.getProgramInterfaceParameter(GL.programs[program], programInterface, pname);
    if (ret === null) return;
    {{{ makeSetValue('params', '0', 'ret', 'i32') }}};
  },

  glGetProgramResourceiv__sig: 'viiiiiiii',
  glGetProgramResourceiv: function(program, programInterface, index, propCount, props, bufSize, length, params) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramResourceiv', 'program');
#endif
    var i, j;
    if (propCount <= 0 || bufSize < 0) {
      GL.recordError(0x0501); // GL_INVALID_VALUE
      return;
    }
    var properties = new Array(propCount);
    for (i = 0; i < propCount; i++) {
      var property = {{{ makeGetValue('props', 'i*4', 'i32') }}};
      switch (property) {
        case 0x92F9: // GL_NAME_LENGTH
        case 0x92FA: // GL_TYPE
        case 0x92FB: // GL_ARRAY_SIZE
        case 0x92FC: // GL_OFFSET
        case 0x92FD: // GL_BLOCK_INDEX
        case 0x92FE: // GL_ARRAY_STRIDE
        case 0x92FF: // GL_MATRIX_STRIDE
        case 0x9300: // GL_IS_ROW_MAJOR
        case 0x9301: // GL_ATOMIC_COUNTER_BUFFER_INDEX
        case 0x9302: // GL_BUFFER_BINDING
        case 0x9303: // GL_BUFFER_DATA_SIZE
        case 0x9304: // GL_NUM_ACTIVE_VARIABLES
        case 0x9305: // GL_ACTIVE_VARIABLES
        case 0x9306: // GL_REFERENCED_BY_VERTEX_SHADER
        case 0x930A: // GL_REFERENCED_BY_FRAGMENT_SHADER
        case 0x930B: // GL_REFERENCED_BY_COMPUTE_SHADER
        case 0x930C: // GL_TOP_LEVEL_ARRAY_SIZE
        case 0x930D: // GL_TOP_LEVEL_ARRAY_STRIDE
        case 0x930E: // GL_LOCATION
          break;
        default:
          // in case any value in props is valid for getProgramResource() but the return value is incompatible with an integer
          GL.recordError(0x0500); // GL_INVALID_ENUM
          return;
      }
      properties[i] = property;
    }
    var ret = GLctx.getProgramResource(GL.programs[program], programInterface, index, properties);
    if (ret === null) return;
    var len = 0;
    for (i = 0; i < propCount; i++) {
      switch (properties[i]) {
        case 0x92F9: // GL_NAME_LENGTH
        case 0x92FA: // GL_TYPE
        case 0x92FB: // GL_ARRAY_SIZE
        case 0x92FC: // GL_OFFSET
        case 0x92FD: // GL_BLOCK_INDEX
        case 0x92FE: // GL_ARRAY_STRIDE
        case 0x92FF: // GL_MATRIX_STRIDE
        case 0x9300: // GL_IS_ROW_MAJOR
        case 0x9301: // GL_ATOMIC_COUNTER_BUFFER_INDEX
        case 0x9302: // GL_BUFFER_BINDING
        case 0x9303: // GL_BUFFER_DATA_SIZE
        case 0x9304: // GL_NUM_ACTIVE_VARIABLES
        case 0x9306: // GL_REFERENCED_BY_VERTEX_SHADER
        case 0x930A: // GL_REFERENCED_BY_FRAGMENT_SHADER
        case 0x930B: // GL_REFERENCED_BY_COMPUTE_SHADER
        case 0x930C: // GL_TOP_LEVEL_ARRAY_SIZE
        case 0x930D: // GL_TOP_LEVEL_ARRAY_STRIDE
          if (len >= bufSize) break;
          {{{ makeSetValue('params', 'len*4', 'ret[i]', 'i32') }}};
          len++;
          break;
        case 0x930E: // GL_LOCATION
          var loc = -1;
          switch (programInterface) {
            case 0x92E3: // GL_PROGRAM_INPUT
            case 0x92E4: // GL_PROGRAM_OUTPUT
              loc = ret[i];
              break;
            case 0x92E1: // GL_UNIFORM
              var str = GLctx.getProgramResourceName(GL.programs[program], programInterface, index);
              loc = GL.lookupUniformTable(program, str);
              break;
            default:
#if GL_ASSERTIONS
              err('internal bug: unhandled programInterface in glGetProgramResourceiv');
#endif
          }
          if (len >= bufSize) break;
          {{{ makeSetValue('params', 'len*4', 'loc', 'i32') }}};
          len++;
          break;
        case 0x9305: // GL_ACTIVE_VARIABLES
          for (j = 0; j < ret[i].length; j++) {
            if (len >= bufSize) break;
            {{{ makeSetValue('params', 'len*4', 'ret[i][j]', 'i32') }}};
            len++;
          }
          break;
        default:
#if GL_ASSERTIONS
          err('internal bug: unhandled value in props in glGetProgramResourceiv');
#endif
      }
    }
    if (length) {
      {{{ makeSetValue('length', '0', 'len', 'i32') }}};
    }
  },

  glGetProgramResourceIndex__sig: 'iiii',
  glGetProgramResourceIndex: function(program, programInterface, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramResourceIndex', 'program');
#endif
    return GLctx.getProgramResourceIndex(GL.programs[program], programInterface, UTF8ToString(name));
  },

  glGetProgramResourceName__sig: 'viiiiii',
  glGetProgramResourceName: function(program, programInterface, index, bufSize, length, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramResourceName', 'program');
#endif
    if (bufSize < 0) {
      GL.recordError(0x0501); // GL_INVALID_VALUE
      return;
    }
    var str = GLctx.getProgramResourceName(GL.programs[program], programInterface, index);
    if (str === null) return;
    var len = stringToUTF8(str, name, bufSize);
    if (length) {
      {{{ makeSetValue('length', '0', 'len', 'i32') }}};
    }
  },

  glGetProgramResourceLocation__sig: 'iiii',
  glGetProgramResourceLocation: function(program, programInterface, name) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.programs, program, 'glGetProgramResourceLocation', 'program');
#endif
    var str = UTF8ToString(name);
    var loc = GLctx.getProgramResourceLocation(GL.programs[program], programInterface, str);
    switch (programInterface) {
      case 0x92E3: // GL_PROGRAM_INPUT
      case 0x92E4: // GL_PROGRAM_OUTPUT
        break;
      case 0x92E1: // GL_UNIFORM
        loc = GL.lookupUniformTable(program, str);
        break;
      default:
#if GL_ASSERTIONS
        err('internal bug: unhandled programInterface in glGetProgramResourceLocation');
#endif
    }
    return loc;
  },

  glProgramUniform1i__sig: 'viii',
  glProgramUniform1i: function(program, location, v0) {
    throw 'glProgramUniform1i: TODO';
  },

  glProgramUniform2i__sig: 'viiii',
  glProgramUniform2i: function(program, location, v0, v1) {
    throw 'glProgramUniform2i: TODO';
  },

  glProgramUniform3i__sig: 'viiiii',
  glProgramUniform3i: function(program, location, v0, v1, v2) {
    throw 'glProgramUniform3i: TODO';
  },

  glProgramUniform4i__sig: 'viiiiii',
  glProgramUniform4i: function(program, location, v0, v1, v2, v3) {
    throw 'glProgramUniform4i: TODO';
  },

  glProgramUniform1ui__sig: 'viii',
  glProgramUniform1ui: function(program, location, v0) {
    throw 'glProgramUniform1ui: TODO';
  },

  glProgramUniform2ui__sig: 'viiii',
  glProgramUniform2ui: function(program, location, v0, v1) {
    throw 'glProgramUniform2ui: TODO';
  },

  glProgramUniform3ui__sig: 'viiiii',
  glProgramUniform3ui: function(program, location, v0, v1, v2) {
    throw 'glProgramUniform3ui: TODO';
  },

  glProgramUniform4ui__sig: 'viiiiii',
  glProgramUniform4ui: function(program, location, v0, v1, v2, v3) {
    throw 'glProgramUniform4ui: TODO';
  },

  glProgramUniform1f__sig: 'viif',
  glProgramUniform1f: function(program, location, v0) {
    throw 'glProgramUniform1f: TODO';
  },

  glProgramUniform2f__sig: 'viiff',
  glProgramUniform2f: function(program, location, v0, v1) {
    throw 'glProgramUniform2f: TODO';
  },

  glProgramUniform3f__sig: 'viifff',
  glProgramUniform3f: function(program, location, v0, v1, v2) {
    throw 'glProgramUniform3f: TODO';
  },

  glProgramUniform4f__sig: 'viiffff',
  glProgramUniform4f: function(program, location, v0, v1, v2, v3) {
    throw 'glProgramUniform4f: TODO';
  },

  glProgramUniform1iv__sig: 'viiii',
  glProgramUniform1iv: function(program, location, count, value) {
    throw 'glProgramUniform1iv: TODO';
  },

  glProgramUniform2iv__sig: 'viiii',
  glProgramUniform2iv: function(program, location, count, value) {
    throw 'glProgramUniform2iv: TODO';
  },

  glProgramUniform3iv__sig: 'viiii',
  glProgramUniform3iv: function(program, location, count, value) {
    throw 'glProgramUniform3iv: TODO';
  },

  glProgramUniform4iv__sig: 'viiii',
  glProgramUniform4iv: function(program, location, count, value) {
    throw 'glProgramUniform4iv: TODO';
  },

  glProgramUniform1uiv__sig: 'viiii',
  glProgramUniform1uiv: function(program, location, count, value) {
    throw 'glProgramUniform1uiv: TODO';
  },

  glProgramUniform2uiv__sig: 'viiii',
  glProgramUniform2uiv: function(program, location, count, value) {
    throw 'glProgramUniform2uiv: TODO';
  },

  glProgramUniform3uiv__sig: 'viiii',
  glProgramUniform3uiv: function(program, location, count, value) {
    throw 'glProgramUniform3uiv: TODO';
  },

  glProgramUniform4uiv__sig: 'viiii',
  glProgramUniform4uiv: function(program, location, count, value) {
    throw 'glProgramUniform4uiv: TODO';
  },

  glProgramUniform1fv__sig: 'viiii',
  glProgramUniform1fv: function(program, location, count, value) {
    throw 'glProgramUniform1fv: TODO';
  },

  glProgramUniform2fv__sig: 'viiii',
  glProgramUniform2fv: function(program, location, count, value) {
    throw 'glProgramUniform2fv: TODO';
  },

  glProgramUniform3fv__sig: 'viiii',
  glProgramUniform3fv: function(program, location, count, value) {
    throw 'glProgramUniform3fv: TODO';
  },

  glProgramUniform4fv__sig: 'viiii',
  glProgramUniform4fv: function(program, location, count, value) {
    throw 'glProgramUniform4fv: TODO';
  },

  glProgramUniformMatrix2fv__sig: 'viiiii',
  glProgramUniformMatrix2fv: function(program, location, count, transpose, value) {
    throw 'glProgramUniformMatrix2fv: TODO';
  },

  glProgramUniformMatrix3fv__sig: 'viiiii',
  glProgramUniformMatrix3fv: function(program, location, count, transpose, value) {
    throw 'glProgramUniformMatrix3fv: TODO';
  },

  glProgramUniformMatrix4fv__sig: 'viiiii',
  glProgramUniformMatrix4fv: function(program, location, count, transpose, value) {
    throw 'glProgramUniformMatrix4fv: TODO';
  },

  glProgramUniformMatrix2x3fv__sig: 'viiiii',
  glProgramUniformMatrix2x3fv: function(program, location, count, transpose, value) {
    throw 'glProgramUniformMatrix2x3fv: TODO';
  },

  glProgramUniformMatrix3x2fv__sig: 'viiiii',
  glProgramUniformMatrix3x2fv: function(program, location, count, transpose, value) {
    throw 'glProgramUniformMatrix3x2fv: TODO';
  },

  glProgramUniformMatrix2x4fv__sig: 'viiiii',
  glProgramUniformMatrix2x4fv: function(program, location, count, transpose, value) {
    throw 'glProgramUniformMatrix2x4fv: TODO';
  },

  glProgramUniformMatrix4x2fv__sig: 'viiiii',
  glProgramUniformMatrix4x2fv: function(program, location, count, transpose, value) {
    throw 'glProgramUniformMatrix4x2fv: TODO';
  },

  glProgramUniformMatrix3x4fv__sig: 'viiiii',
  glProgramUniformMatrix3x4fv: function(program, location, count, transpose, value) {
    throw 'glProgramUniformMatrix3x4fv: TODO';
  },

  glProgramUniformMatrix4x3fv__sig: 'viiiii',
  glProgramUniformMatrix4x3fv: function(program, location, count, transpose, value) {
    throw 'glProgramUniformMatrix4x3fv: TODO';
  },

  glBindImageTexture__sig: 'viiiiiii',
  glBindImageTexture: function(unit, texture, level, layered, layer, access, format) {
#if GL_ASSERTIONS
    GL.validateGLObjectID(GL.textures, texture, 'glBindImageTexture', 'texture');
#endif
    GLctx.bindImageTexture(unit, GL.textures[texture], level, !!layered, layer, access, format);
  },

  glGetBooleani_v__sig: 'viii',
  glGetBooleani_v: function(target, index, data) {
    throw 'glGetBooleani_v: TODO';
  },

  glGetTexLevelParameteriv__sig: 'viiii',
  glGetTexLevelParameteriv: function(target, level, pname, params) {
    throw 'glGetTexLevelParameteriv: TODO';
  },

  glGetTexLevelParameterfv__sig: 'viiii',
  glGetTexLevelParameterfv: function(target, level, pname, params) {
    throw 'glGetTexLevelParameterfv: TODO';
  },

  glGetMultisamplefv__sig: 'viii',
  glGetMultisamplefv: function(pname, index, val) {
    throw 'glGetMultisamplefv: TODO';
  },

  glBindVertexBuffer__sig: 'viiii',
  glBindVertexBuffer: function(bindingindex, buffer, offset, stride) {
    throw 'glBindVertexBuffer: TODO';
  },

  glDispatchCompute__sig: 'viii',
  glDispatchComputeIndirect__sig: 'vi',
  glFramebufferParameteri__sig: 'viii',
  glMemoryBarrier__sig: 'vi',
  glMemoryBarrierByRegion__sig: 'vi',
  glTexStorage2DMultisample__sig: 'viiiiii',
  glSampleMaski__sig: 'vii',
  glVertexAttribFormat__sig: 'viiiii',
  glVertexAttribIFormat__sig: 'viiii',
  glVertexAttribBinding__sig: 'vii',
  glVertexBindingDivisor__sig: 'vii',
};

// Simple pass-through functions. Starred ones have return values. [X] ones have X in the C name but not in the JS name
var webgl2computeFuncs = [[0, ''],
 [1, 'dispatchComputeIndirect memoryBarrier memoryBarrierByRegion'],
 [2, 'sampleMask[i] vertexAttribBinding vertexBindingDivisor'],
 [3, 'dispatchCompute framebufferParameter[i]'],
 [4, 'vertexAttribIFormat'],
 [5, 'vertexAttribFormat'],
 [6, 'texStorage2DMultisample'],
 [7, ''],
 [8, ''],
 [9, ''],
 [10, '']];

#if GL_MAX_FEATURE_LEVEL >= 30

createGLPassthroughFunctions(LibraryWebGL2Compute, webgl2computeFuncs);

recordGLProcAddressGet(LibraryWebGL2Compute);

mergeInto(LibraryManager.library, LibraryWebGL2Compute);

#endif

#if USE_WEBGL2_COMPUTE
assert(GL_MAX_FEATURE_LEVEL == 30, 'cannot resolve conflicting USE_WEBGL* settings');
#endif
