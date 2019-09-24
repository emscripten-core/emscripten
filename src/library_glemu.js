/*
 * Copyright 2010 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Desktop OpenGL emulation support. See http://kripken.github.io/emscripten-site/docs/porting/multimedia_and_graphics/OpenGL-support.html
 * for current status.
 */

var LibraryGLEmulation = {
  // GL emulation: provides misc. functionality not present in OpenGL ES 2.0 or WebGL
  $GLEmulation__deps: ['$GLImmediateSetup', 'glEnable', 'glDisable', 'glIsEnabled', 'glGetBooleanv', 'glGetIntegerv', 'glGetString', 'glCreateShader', 'glShaderSource', 'glCompileShader', 'glAttachShader', 'glDetachShader', 'glUseProgram', 'glDeleteProgram', 'glBindAttribLocation', 'glLinkProgram', 'glBindBuffer', 'glGetFloatv', 'glHint', 'glEnableVertexAttribArray', 'glDisableVertexAttribArray', 'glVertexAttribPointer', 'glActiveTexture', '$stringToNewUTF8'],
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
      err('WARNING: using emscripten GL emulation. This is a collection of limited workarounds, do not expect it to work.');
#if GL_UNSAFE_OPTS == 1
      err('WARNING: using emscripten GL emulation unsafe opts. If weirdness happens, try -s GL_UNSAFE_OPTS=0');
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

#if RELOCATABLE
{{{
(updateExport = function(name) {
  var name = '_' + name;
  var exported = 'Module["' + name + '"]';
  // make sure we write to an existing export, and are not repeating ourselves
  return 'assert(' + exported + ' !== ' + name + '); ' + exported + ' = ' + name + ';';
}, '')
}}}
#else
{{{ (updateExport = function(){ return '' }, '') }}}
#endif

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
      {{{ updateExport('glEnable') }}}

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
      {{{ updateExport('glDisable') }}}

      _glIsEnabled = _emscripten_glIsEnabled = function _glIsEnabled(cap) {
        if (cap == 0x0B60 /* GL_FOG */) {
          return GLEmulation.fogEnabled ? 1 : 0;
        } else if (!(cap in validCapabilities)) {
          return 0;
        }
        return GLctx.isEnabled(cap);
      };
      {{{ updateExport('glIsEnabled') }}}

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
      {{{ updateExport('glGetBooleanv') }}}

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
      {{{ updateExport('glGetIntegerv') }}}

      var glGetString = _glGetString;
      _glGetString = _emscripten_glGetString = function _glGetString(name_) {
        if (GL.stringCache[name_]) return GL.stringCache[name_];
        switch(name_) {
          case 0x1F03 /* GL_EXTENSIONS */: // Add various extensions that we can support
            var ret = stringToNewUTF8((GLctx.getSupportedExtensions() || []).join(' ') +
                   ' GL_EXT_texture_env_combine GL_ARB_texture_env_crossbar GL_ATI_texture_env_combine3 GL_NV_texture_env_combine4 GL_EXT_texture_env_dot3 GL_ARB_multitexture GL_ARB_vertex_buffer_object GL_EXT_framebuffer_object GL_ARB_vertex_program GL_ARB_fragment_program GL_ARB_shading_language_100 GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader GL_ARB_texture_cube_map GL_EXT_draw_range_elements' +
                   (GL.currentContext.compressionExt ? ' GL_ARB_texture_compression GL_EXT_texture_compression_s3tc' : '') +
                   (GL.currentContext.anisotropicExt ? ' GL_EXT_texture_filter_anisotropic' : '')
            );
            GL.stringCache[name_] = ret;
            return ret;
        }
        return glGetString(name_);
      };
      {{{ updateExport('glGetString') }}}

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
      {{{ updateExport('glCreateShader') }}}

      function ensurePrecision(source) {
        if (!/precision +(low|medium|high)p +float *;/.test(source)) {
          source = '#ifdef GL_FRAGMENT_PRECISION_HIGH\nprecision highp float;\n#else\nprecision mediump float;\n#endif\n' + source;
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
            old = source;
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
          for (i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
            old = source;
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
      {{{ updateExport('glShaderSource') }}}

      var glCompileShader = _glCompileShader;
      _glCompileShader = _emscripten_glCompileShader = function _glCompileShader(shader) {
        GLctx.compileShader(GL.shaders[shader]);
#if GL_DEBUG
        if (!GLctx.getShaderParameter(GL.shaders[shader], GLctx.COMPILE_STATUS)) {
          err('Failed to compile shader: ' + GLctx.getShaderInfoLog(GL.shaders[shader]));
          err('Info: ' + JSON.stringify(GL.shaderInfos[shader]));
          err('Original source: ' + GL.shaderOriginalSources[shader]);
          err('Source: ' + GL.shaderSources[shader]);
          throw 'Shader compilation halt';
        }
#endif
      };
      {{{ updateExport('glCompileShader') }}}

      GL.programShaders = {};
      var glAttachShader = _glAttachShader;
      _glAttachShader = _emscripten_glAttachShader = function _glAttachShader(program, shader) {
        if (!GL.programShaders[program]) GL.programShaders[program] = [];
        GL.programShaders[program].push(shader);
        glAttachShader(program, shader);
      };
      {{{ updateExport('glAttachShader') }}}

      var glDetachShader = _glDetachShader;
      _glDetachShader = _emscripten_glDetachShader = function _glDetachShader(program, shader) {
        var programShader = GL.programShaders[program];
        if (!programShader) {
          err('WARNING: _glDetachShader received invalid program: ' + program);
          return;
        }
        var index = programShader.indexOf(shader);
        programShader.splice(index, 1);
        glDetachShader(program, shader);
      };
      {{{ updateExport('glDetachShader') }}}

      var glUseProgram = _glUseProgram;
      _glUseProgram = _emscripten_glUseProgram = function _glUseProgram(program) {
#if GL_DEBUG
        if (GL.debug) {
          err('[using program with shaders]');
          if (program) {
            GL.programShaders[program].forEach(function(shader) {
              err('  shader ' + shader + ', original source: ' + GL.shaderOriginalSources[shader]);
              err('         Source: ' + GL.shaderSources[shader]);
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
      {{{ updateExport('glUseProgram') }}}

      var glDeleteProgram = _glDeleteProgram;
      _glDeleteProgram = _emscripten_glDeleteProgram = function _glDeleteProgram(program) {
        glDeleteProgram(program);
        if (program == GL.currProgram) {
          GLImmediate.currentRenderer = null; // This changes the FFP emulation shader program, need to recompute that.
          GL.currProgram = 0;
        }
      };
      {{{ updateExport('glDeleteProgram') }}}

      // If attribute 0 was not bound, bind it to 0 for WebGL performance reasons. Track if 0 is free for that.
      var zeroUsedPrograms = {};
      var glBindAttribLocation = _glBindAttribLocation;
      _glBindAttribLocation = _emscripten_glBindAttribLocation = function _glBindAttribLocation(program, index, name) {
        if (index == 0) zeroUsedPrograms[program] = true;
        glBindAttribLocation(program, index, name);
      };
      {{{ updateExport('glBindAttribLocation') }}}

      var glLinkProgram = _glLinkProgram;
      _glLinkProgram = _emscripten_glLinkProgram = function _glLinkProgram(program) {
        if (!(program in zeroUsedPrograms)) {
          GLctx.bindAttribLocation(GL.programs[program], 0, 'a_position');
        }
        glLinkProgram(program);
      };
      {{{ updateExport('glLinkProgram') }}}

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
      {{{ updateExport('glBindBuffer') }}}

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
      {{{ updateExport('glGetFloatv') }}}

      var glHint = _glHint;
      _glHint = _emscripten_glHint = function _glHint(target, mode) {
        if (target == 0x84EF) { // GL_TEXTURE_COMPRESSION_HINT
          return;
        }
        glHint(target, mode);
      };
      {{{ updateExport('glHint') }}}

      var glEnableVertexAttribArray = _glEnableVertexAttribArray;
      _glEnableVertexAttribArray = _emscripten_glEnableVertexAttribArray = function _glEnableVertexAttribArray(index) {
        glEnableVertexAttribArray(index);
        GLEmulation.enabledVertexAttribArrays[index] = 1;
        if (GLEmulation.currentVao) GLEmulation.currentVao.enabledVertexAttribArrays[index] = 1;
      };
      {{{ updateExport('glEnableVertexAttribArray') }}}

      var glDisableVertexAttribArray = _glDisableVertexAttribArray;
      _glDisableVertexAttribArray = _emscripten_glDisableVertexAttribArray = function _glDisableVertexAttribArray(index) {
        glDisableVertexAttribArray(index);
        delete GLEmulation.enabledVertexAttribArrays[index];
        if (GLEmulation.currentVao) delete GLEmulation.currentVao.enabledVertexAttribArrays[index];
      };
      {{{ updateExport('glDisableVertexAttribArray') }}}

      var glVertexAttribPointer = _glVertexAttribPointer;
      _glVertexAttribPointer = _emscripten_glVertexAttribPointer = function _glVertexAttribPointer(index, size, type, normalized, stride, pointer) {
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        if (GLEmulation.currentVao) { // TODO: avoid object creation here? likely not hot though
          GLEmulation.currentVao.vertexAttribPointers[index] = [index, size, type, normalized, stride, pointer];
        }
      };
      {{{ updateExport('glVertexAttribPointer') }}}
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

  glDeleteObject__deps: ['glDeleteProgram', 'glDeleteShader'],
  glDeleteObject__sig: 'vi',
  glDeleteObject: function(id) {
    if (GL.programs[id]) {
      _glDeleteProgram(id);
    } else if (GL.shaders[id]) {
      _glDeleteShader(id);
    } else {
      err('WARNING: deleteObject received invalid id: ' + id);
    }
  },
  glDeleteObjectARB: 'glDeleteObject',

  glGetObjectParameteriv__sig: 'viii',
  glGetObjectParameteriv__deps: ['glGetProgramiv', 'glGetShaderiv'],
  glGetObjectParameteriv: function(id, type, result) {
    if (GL.programs[id]) {
      if (type == 0x8B84) { // GL_OBJECT_INFO_LOG_LENGTH_ARB
        var log = GLctx.getProgramInfoLog(GL.programs[id]);
        if (log === null) log = '(unknown error)';
        {{{ makeSetValue('result', '0', 'log.length', 'i32') }}};
        return;
      }
      _glGetProgramiv(id, type, result);
    } else if (GL.shaders[id]) {
      if (type == 0x8B84) { // GL_OBJECT_INFO_LOG_LENGTH_ARB
        var log = GLctx.getShaderInfoLog(GL.shaders[id]);
        if (log === null) log = '(unknown error)';
        {{{ makeSetValue('result', '0', 'log.length', 'i32') }}};
        return;
      } else if (type == 0x8B88) { // GL_OBJECT_SHADER_SOURCE_LENGTH_ARB
        var source = GLctx.getShaderSource(GL.shaders[id]);
        if (source === null) return; // If an error occurs, nothing will be written to result
        {{{ makeSetValue('result', '0', 'source.length', 'i32') }}};
        return;
      }
      _glGetShaderiv(id, type, result);
    } else {
      err('WARNING: getObjectParameteriv received invalid id: ' + id);
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
      err('WARNING: glGetInfoLog received invalid id: ' + id);
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
        err('GL_INVALID_ENUM in glGetPointerv: Unsupported name ' + name + '!');
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
          0x0303 /* GL_ONE_MINUS_SRC_ALPHA */: 3
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
              err('WARNING: Unhandled `pname` in call to `glTexEnvf`.');
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
              err('WARNING: Unhandled `pname` in call to `glTexEnvi`.');
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
              err('WARNING: Unhandled `pname` in call to `glTexEnvfv`.');
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
              err('WARNING: Unhandled `pname` in call to `glGetTexEnvi`.');
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
        err('generating renderer for ' + JSON.stringify(attributes));
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
             warnOnce("GL_TEXTURE" + i + " coords are supplied, but that texture unit is disabled in the fixed-function pipeline.");
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
      {{{ updateExport('glActiveTexture') }}}

      var glEnable = _glEnable;
      _glEnable = _emscripten_glEnable = function _glEnable(cap) {
        GLImmediate.TexEnvJIT.hook_enable(cap);
        glEnable(cap);
      };
      {{{ updateExport('glEnable') }}}

      var glDisable = _glDisable;
      _glDisable = _emscripten_glDisable = function _glDisable(cap) {
        GLImmediate.TexEnvJIT.hook_disable(cap);
        glDisable(cap);
      };
      {{{ updateExport('glDisable') }}}

      var glTexEnvf = (typeof(_glTexEnvf) != 'undefined') ? _glTexEnvf : function(){};
      _glTexEnvf = _emscripten_glTexEnvf = function _glTexEnvf(target, pname, param) {
        GLImmediate.TexEnvJIT.hook_texEnvf(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvf(target, pname, param);
      };
      {{{ updateExport('glTexEnvf') }}}

      var glTexEnvi = (typeof(_glTexEnvi) != 'undefined') ? _glTexEnvi : function(){};
      _glTexEnvi = _emscripten_glTexEnvi = function _glTexEnvi(target, pname, param) {
        GLImmediate.TexEnvJIT.hook_texEnvi(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvi(target, pname, param);
      };
      {{{ updateExport('glTexEnvi') }}}

      var glTexEnvfv = (typeof(_glTexEnvfv) != 'undefined') ? _glTexEnvfv : function(){};
      _glTexEnvfv = _emscripten_glTexEnvfv = function _glTexEnvfv(target, pname, param) {
        GLImmediate.TexEnvJIT.hook_texEnvfv(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvfv(target, pname, param);
      };
      {{{ updateExport('glTexEnvfv') }}}

      _glGetTexEnviv = function _glGetTexEnviv(target, pname, param) {
        GLImmediate.TexEnvJIT.hook_getTexEnviv(target, pname, param);
      };
      {{{ updateExport('glGetTexEnviv') }}}

      _glGetTexEnvfv = function _glGetTexEnvfv(target, pname, param) {
        GLImmediate.TexEnvJIT.hook_getTexEnvfv(target, pname, param);
      };
      {{{ updateExport('glGetTexEnvfv') }}}

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
      {{{ updateExport('glGetIntegerv') }}}
    },

    // Main functions
    initted: false,
    init: function() {
      err('WARNING: using emscripten GL immediate mode emulation. This is very limited in what it supports');
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
          warnOnce('Warning: Rendering from client side vertex arrays where stride (' + GLImmediate.stride + ') is not a multiple of four! This is not currently supported!');
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
        warnOnce('Rendering from planar client-side vertex arrays. This is a very slow emulation path! Use interleaved vertex arrays for best performance.');
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
          warnOnce('Warning: Rendering from client side vertex arrays where stride (' + GLImmediate.stride + ') is not a multiple of four! This is not currently supported!');
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
      if (!numVertexes) return;
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

  glVertex2f: function(x, y) {
#if ASSERTIONS
    assert(GLImmediate.mode >= 0); // must be in begin/end
#endif
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = x;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = y;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = 0;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = 1;
#if ASSERTIONS
    assert(GLImmediate.vertexCounter << 2 < GL.MAX_TEMP_BUFFER_SIZE);
#endif
    GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, GLctx.FLOAT);
  },

  glVertex3f: function(x, y, z) {
#if ASSERTIONS
    assert(GLImmediate.mode >= 0); // must be in begin/end
#endif
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = x;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = y;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = z;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = 1;
#if ASSERTIONS
    assert(GLImmediate.vertexCounter << 2 < GL.MAX_TEMP_BUFFER_SIZE);
#endif
    GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, GLctx.FLOAT);
  },

  glVertex4f: function(x, y, z, w) {
#if ASSERTIONS
    assert(GLImmediate.mode >= 0); // must be in begin/end
#endif
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = x;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = y;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = z;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = w;
#if ASSERTIONS
    assert(GLImmediate.vertexCounter << 2 < GL.MAX_TEMP_BUFFER_SIZE);
#endif
    GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, GLctx.FLOAT);
  },

  glVertex2fv__deps: ['glVertex2f'],
  glVertex2fv: function(p) {
    _glVertex2f({{{ makeGetValue('p', '0', 'float') }}}, {{{ makeGetValue('p', '4', 'float') }}});
  },

  glVertex3fv__deps: ['glVertex3f'],
  glVertex3fv: function(p) {
    _glVertex3f({{{ makeGetValue('p', '0', 'float') }}}, {{{ makeGetValue('p', '4', 'float') }}}, {{{ makeGetValue('p', '8', 'float') }}});
  },

  glVertex4fv__deps: ['glVertex4f'],
  glVertex4fv: function(p) {
    _glVertex4f({{{ makeGetValue('p', '0', 'float') }}}, {{{ makeGetValue('p', '4', 'float') }}}, {{{ makeGetValue('p', '8', 'float') }}}, {{{ makeGetValue('p', '12', 'float') }}});
  },

  glVertex2i: 'glVertex2f',

  glVertex3i: 'glVertex3f',

  glVertex4i: 'glVertex4f',

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
      err('WARNING: unhandled clientstate: ' + cap);
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
      err('WARNING: unhandled clientstate: ' + cap);
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
      GLctx.vertexAttribPointer(GLImmediate.NORMAL, 3, type, true, stride, pointer);
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

  // Replace some functions with immediate-mode aware versions. If there are no client
  // attributes enabled, and we use webgl-friendly modes (no GL_QUADS), then no need
  // for emulation
  glDrawArrays: function(mode, first, count) {
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
  },

  glDrawElements: function(mode, count, type, indices, start, end) { // start, end are given if we come from glDrawRangeElements
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
      GLImmediate.firstVertex = end ? start : HEAP8.length; // if we don't know the start, set an invalid value and we will calculate it later from the indices
      GLImmediate.lastVertex = end ? end+1 : 0;
      GLImmediate.vertexData = HEAPF32.subarray(GLImmediate.vertexPointer >> 2, end ? (GLImmediate.vertexPointer + (end+1)*GLImmediate.stride) >> 2 : undefined); // XXX assuming float
    }
    GLImmediate.flush(count, 0, indices);
    GLImmediate.mode = -1;
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
    if (GLImmediate.matrixStack[GLImmediate.currentMatrix].length == 0) {
      GL.recordError(0x0504/*GL_STACK_UNDERFLOW*/);
      return;
    }
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
    if (GL.debug) err('glLoadMatrixf receiving: ' + Array.prototype.slice.call(HEAPF32.subarray(matrix >> 2, (matrix >> 2) + 16)));
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
  glTexEnvi: function() { warnOnce('glTexEnvi: TODO') },
  glTexEnvf: function() { warnOnce('glTexEnvf: TODO') },
  glTexEnvfv: function() { warnOnce('glTexEnvfv: TODO') },

  glGetTexEnviv: function(target, pname, param) { throw 'GL emulation not initialized!'; },
  glGetTexEnvfv: function(target, pname, param) { throw 'GL emulation not initialized!'; },

  glTexImage1D: function() { throw 'glTexImage1D: TODO' },
  glTexCoord3f: function() { throw 'glTexCoord3f: TODO' },
  glGetTexLevelParameteriv: function() { throw 'glGetTexLevelParameteriv: TODO' },

  glShadeModel: function() { warnOnce('TODO: glShadeModel') },

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
};

// Legacy GL emulation
if (LEGACY_GL_EMULATION) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$GLEmulation');
}

recordGLProcAddressGet(LibraryGLEmulation);

mergeInto(LibraryManager.library, LibraryGLEmulation);

assert(!(FULL_ES2 && LEGACY_GL_EMULATION), 'cannot emulate both ES2 and legacy GL');
assert(!(FULL_ES3 && LEGACY_GL_EMULATION), 'cannot emulate both ES3 and legacy GL');
