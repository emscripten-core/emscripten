/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

{{{
  globalThis.copySigs = (func) => {
    if (!RELOCATABLE) return '';
    return ` _${func}.sig = _emscripten_${func}.sig = orig_${func}.sig;`;
  };
  globalThis.fromPtr = (arg) => {
    if (CAN_ADDRESS_2GB) {
      return `${arg} >>>= 0`;
    } else if (MEMORY64) {
      return `${arg} = Number(${arg})`;
    }
    return '';
  };
  null;
}}}

var LibraryGLEmulation = {
  // GL emulation: provides misc. functionality not present in OpenGL ES 2.0 or WebGL
  $GLEmulation__deps: ['$GLImmediateSetup', 'glEnable', 'glDisable',
    'glIsEnabled', 'glGetBooleanv', 'glGetIntegerv', 'glGetString',
    'glCreateShader', 'glShaderSource', 'glCompileShader', 'glAttachShader',
    'glDetachShader', 'glUseProgram', 'glDeleteProgram', 'glBindAttribLocation',
    'glLinkProgram', 'glBindBuffer', 'glGetFloatv', 'glHint',
    'glEnableVertexAttribArray', 'glDisableVertexAttribArray',
    'glVertexAttribPointer', 'glActiveTexture', '$stringToNewUTF8',
    '$ptrToString', '$getEmscriptenSupportedExtensions',
  ],
  $GLEmulation__postset:
#if MAYBE_CLOSURE_COMPILER
    // Forward declare GL functions that are overridden by GLEmulation here to appease Closure compiler.
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glDrawArrays;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glDrawElements;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glActiveTexture;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glEnable;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glDisable;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glTexEnvf;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glTexEnvi;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glTexEnvfv;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glGetIntegerv;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glIsEnabled;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glGetBooleanv;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glGetString;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glCreateShader;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glShaderSource;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glCompileShader;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glAttachShader;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glDetachShader;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glUseProgram;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glDeleteProgram;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glBindAttribLocation;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glLinkProgram;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glBindBuffer;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glGetFloatv;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glHint;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glEnableVertexAttribArray;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glDisableVertexAttribArray;' +
    '/**@suppress {duplicate, undefinedVars}*/var _emscripten_glVertexAttribPointer;' +
    '/**@suppress {duplicate, undefinedVars}*/var _glTexEnvf;' +
    '/**@suppress {duplicate, undefinedVars}*/var _glTexEnvi;' +
    '/**@suppress {duplicate, undefinedVars}*/var _glTexEnvfv;' +
    '/**@suppress {duplicate, undefinedVars}*/var _glGetTexEnviv;' +
    '/**@suppress {duplicate, undefinedVars}*/var _glGetTexEnvfv;' +
#endif
    'GLEmulation.init();',
  $GLEmulation: {
    // Fog support. Partial, we assume shaders are used that implement fog. We just pass them uniforms
    fogStart: 0,
    fogEnd: 1,
    fogDensity: 1.0,
    fogColor: null,
    fogMode: 0x800, // GL_EXP
    fogEnabled: false,

    // GL_CLIP_PLANE support
    MAX_CLIP_PLANES: 6,
    clipPlaneEnabled: [false, false, false, false, false, false],
    clipPlaneEquation: [],

    // GL_LIGHTING support
    lightingEnabled: false,

    lightModelAmbient: null,
    lightModelLocalViewer: false,
    lightModelTwoSide: false,

    materialAmbient: null,
    materialDiffuse: null,
    materialSpecular: null,
    materialShininess: null,
    materialEmission: null,

    MAX_LIGHTS: 8,
    lightEnabled: [false, false, false, false, false, false, false, false],
    lightAmbient: [],
    lightDiffuse: [],
    lightSpecular: [],
    lightPosition: [],
    // TODO attenuation modes of lights

    // GL_ALPHA_TEST support
    alphaTestEnabled: false,
    alphaTestFunc: 0x207, // GL_ALWAYS
    alphaTestRef: 0.0,

    // GL_POINTS support.
    pointSize: 1.0,

    // VAO support
    vaos: [],
    currentVao: null,
    enabledVertexAttribArrays: {}, // helps with vao cleanups

    hasRunInit: false,

    // Find a token in a shader source string
    findToken(source, token) {
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

    init() {
      // Do not activate immediate/emulation code (e.g. replace glDrawElements)
      // when in FULL_ES2 mode.  We do not need full emulation, we instead
      // emulate client-side arrays etc. in FULL_ES2 code in a straightforward
      // manner, and avoid not having a bound buffer be ambiguous between es2
      // emulation code and legacy gl emulation code.
#if FULL_ES2
      return;
#endif

      if (GLEmulation.hasRunInit) {
        return;
      }
      GLEmulation.hasRunInit = true;

      GLEmulation.fogColor = new Float32Array(4);

      for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
        GLEmulation.clipPlaneEquation[clipPlaneId] = new Float32Array(4);
      }

      // set defaults for GL_LIGHTING
      GLEmulation.lightModelAmbient = new Float32Array([0.2, 0.2, 0.2, 1.0]);
      GLEmulation.materialAmbient = new Float32Array([0.2, 0.2, 0.2, 1.0]);
      GLEmulation.materialDiffuse = new Float32Array([0.8, 0.8, 0.8, 1.0]);
      GLEmulation.materialSpecular = new Float32Array([0.0, 0.0, 0.0, 1.0]);
      GLEmulation.materialShininess = new Float32Array([0.0]);
      GLEmulation.materialEmission = new Float32Array([0.0, 0.0, 0.0, 1.0]);

      for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
        GLEmulation.lightAmbient[lightId] = new Float32Array([0.0, 0.0, 0.0, 1.0]);
        GLEmulation.lightDiffuse[lightId] = lightId ? new Float32Array([0.0, 0.0, 0.0, 1.0]) : new Float32Array([1.0, 1.0, 1.0, 1.0]);
        GLEmulation.lightSpecular[lightId] = lightId ? new Float32Array([0.0, 0.0, 0.0, 1.0]) : new Float32Array([1.0, 1.0, 1.0, 1.0]);
        GLEmulation.lightPosition[lightId] = new Float32Array([0.0, 0.0, 1.0, 0.0]);
      }


      // Add some emulation workarounds
      err('WARNING: using emscripten GL emulation. This is a collection of limited workarounds, do not expect it to work.');
#if GL_UNSAFE_OPTS == 1
      err('WARNING: using emscripten GL emulation unsafe opts. If weirdness happens, try -sGL_UNSAFE_OPTS=0');
#endif

      // XXX some of the capabilities we don't support may lead to incorrect rendering, if we do not emulate them in shaders
      var validCapabilities = {
        0xB44: 1, // GL_CULL_FACE
        0xBE2: 1, // GL_BLEND
        0xBD0: 1, // GL_DITHER,
        0xB90: 1, // GL_STENCIL_TEST
        0xB71: 1, // GL_DEPTH_TEST
        0xC11: 1, // GL_SCISSOR_TEST
        0x8037: 1, // GL_POLYGON_OFFSET_FILL
        0x809E: 1, // GL_SAMPLE_ALPHA_TO_COVERAGE
        0x80A0: 1  // GL_SAMPLE_COVERAGE
      };

      var orig_glEnable = _glEnable;
      _glEnable = _emscripten_glEnable = (cap) => {
        // Clean up the renderer on any change to the rendering state. The optimization of
        // skipping renderer setup is aimed at the case of multiple glDraw* right after each other
        GLImmediate.lastRenderer?.cleanup();
        if (cap == 0xB60 /* GL_FOG */) {
          if (GLEmulation.fogEnabled != true) {
            GLImmediate.currentRenderer = null; // Fog parameter is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.fogEnabled = true;
          }
          return;
        } else if ((cap >= 0x3000) && (cap < 0x3006)  /* GL_CLIP_PLANE0 to GL_CLIP_PLANE5 */) {
          var clipPlaneId = cap - 0x3000;
          if (GLEmulation.clipPlaneEnabled[clipPlaneId] != true) {
            GLImmediate.currentRenderer = null; // clip plane parameter is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.clipPlaneEnabled[clipPlaneId] = true;
          }
          return;
        } else if ((cap >= 0x4000) && (cap < 0x4008)  /* GL_LIGHT0 to GL_LIGHT7 */) {
          var lightId = cap - 0x4000;
          if (GLEmulation.lightEnabled[lightId] != true) {
            GLImmediate.currentRenderer = null; // light parameter is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.lightEnabled[lightId] = true;
          }
          return;
        } else if (cap == 0xB50 /* GL_LIGHTING */) {
          if (GLEmulation.lightingEnabled != true) {
            GLImmediate.currentRenderer = null; // light parameter is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.lightingEnabled = true;
          }
          return;
        } else if (cap == 0xBC0 /* GL_ALPHA_TEST */) {
          if (GLEmulation.alphaTestEnabled != true) {
            GLImmediate.currentRenderer = null; // alpha testing is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.alphaTestEnabled = true;
          }
          return;
        } else if (cap == 0xDE1 /* GL_TEXTURE_2D */) {
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
        orig_glEnable(cap);
      };
      {{{ copySigs('glEnable') }}}

      var orig_glDisable = _glDisable;
      _glDisable = _emscripten_glDisable = (cap) => {
        GLImmediate.lastRenderer?.cleanup();
        if (cap == 0xB60 /* GL_FOG */) {
          if (GLEmulation.fogEnabled != false) {
            GLImmediate.currentRenderer = null; // Fog parameter is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.fogEnabled = false;
          }
          return;
        } else if ((cap >= 0x3000) && (cap < 0x3006)  /* GL_CLIP_PLANE0 to GL_CLIP_PLANE5 */) {
          var clipPlaneId = cap - 0x3000;
          if (GLEmulation.clipPlaneEnabled[clipPlaneId] != false) {
            GLImmediate.currentRenderer = null; // clip plane parameter is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.clipPlaneEnabled[clipPlaneId] = false;
          }
          return;
        } else if ((cap >= 0x4000) && (cap < 0x4008)  /* GL_LIGHT0 to GL_LIGHT7 */) {
          var lightId = cap - 0x4000;
          if (GLEmulation.lightEnabled[lightId] != false) {
            GLImmediate.currentRenderer = null; // light parameter is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.lightEnabled[lightId] = false;
          }
          return;
        } else if (cap == 0xB50 /* GL_LIGHTING */) {
          if (GLEmulation.lightingEnabled != false) {
            GLImmediate.currentRenderer = null; // light parameter is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.lightingEnabled = false;
          }
          return;
        } else if (cap == 0xBC0 /* GL_ALPHA_TEST */) {
          if (GLEmulation.alphaTestEnabled != false) {
            GLImmediate.currentRenderer = null; // alpha testing is part of the FFP shader state, we must re-lookup the renderer to use.
            GLEmulation.alphaTestEnabled = false;
          }
          return;
        } else if (cap == 0xDE1 /* GL_TEXTURE_2D */) {
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
        orig_glDisable(cap);
      };
      {{{ copySigs('glDisable') }}}

      var orig_glIsEnabled = _glIsEnabled;
      _glIsEnabled = _emscripten_glIsEnabled = (cap) => {
        if (cap == 0xB60 /* GL_FOG */) {
          return GLEmulation.fogEnabled ? 1 : 0;
        } else if ((cap >= 0x3000) && (cap < 0x3006)  /* GL_CLIP_PLANE0 to GL_CLIP_PLANE5 */) {
          var clipPlaneId = cap - 0x3000;
          return GLEmulation.clipPlaneEnabled[clipPlaneId] ? 1 : 0;
        } else if ((cap >= 0x4000) && (cap < 0x4008)  /* GL_LIGHT0 to GL_LIGHT7 */) {
          var lightId = cap - 0x4000;
          return GLEmulation.lightEnabled[lightId] ? 1 : 0;
        } else if (cap == 0xB50 /* GL_LIGHTING */) {
          return GLEmulation.lightingEnabled ? 1 : 0;
        } else if (cap == 0xBC0 /* GL_ALPHA_TEST */) {
          return GLEmulation.alphaTestEnabled ? 1 : 0;
        } else if (!(cap in validCapabilities)) {
          return 0;
        }
        return GLctx.isEnabled(cap);
      };
      {{{ copySigs('glIsEnabled') }}}

      var orig_glGetBooleanv = _glGetBooleanv;
      _glGetBooleanv = _emscripten_glGetBooleanv = (pname, p) => {
        var attrib = GLEmulation.getAttributeFromCapability(pname);
        if (attrib !== null) {
          {{{ fromPtr('p') }}}
          var result = GLImmediate.enabledClientAttributes[attrib];
          {{{ makeSetValue('p', '0', 'result === true ? 1 : 0', 'i8') }}};
          return;
        }
        orig_glGetBooleanv(pname, p);
      };
      {{{ copySigs('glGetBooleanv') }}}

      var orig_glGetIntegerv = _glGetIntegerv;
      _glGetIntegerv = _emscripten_glGetIntegerv = (pname, params) => {
        {{{ fromPtr('params') }}}
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
          case 0x0D32: { // GL_MAX_CLIP_PLANES
            {{{ makeSetValue('params', '0', 'GLEmulation.MAX_CLIP_PLANES', 'i32') }}}; // all implementations need to support atleast 6
            return;
          }
          case 0x0BA0: { // GL_MATRIX_MODE
            {{{ makeSetValue('params', '0', 'GLImmediate.currentMatrix + 0x1700', 'i32') }}};
            return;
          }
          case 0x0BC1: { // GL_ALPHA_TEST_FUNC
            {{{ makeSetValue('params', '0', 'GLEmulation.alphaTestFunc', 'i32') }}};
            return;
          }
        }
        orig_glGetIntegerv(pname, params);
      };
      {{{ copySigs('glGetIntegerv') }}}

      var orig_glGetString = _glGetString;
      _glGetString = _emscripten_glGetString = (name_) => {
        if (GL.stringCache[name_]) return GL.stringCache[name_];
        switch (name_) {
          case 0x1F03 /* GL_EXTENSIONS */: // Add various extensions that we can support
            var ret = stringToNewUTF8(getEmscriptenSupportedExtensions(GLctx).join(' ') +
                   ' GL_EXT_texture_env_combine GL_ARB_texture_env_crossbar GL_ATI_texture_env_combine3 GL_NV_texture_env_combine4 GL_EXT_texture_env_dot3 GL_ARB_multitexture GL_ARB_vertex_buffer_object GL_EXT_framebuffer_object GL_ARB_vertex_program GL_ARB_fragment_program GL_ARB_shading_language_100 GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader GL_ARB_texture_cube_map GL_EXT_draw_range_elements' +
                   (GL.currentContext.compressionExt ? ' GL_ARB_texture_compression GL_EXT_texture_compression_s3tc' : '') +
                   (GL.currentContext.anisotropicExt ? ' GL_EXT_texture_filter_anisotropic' : '')
            );
            return GL.stringCache[name_] = {{{ to64('ret') }}};
        }
        return orig_glGetString(name_);
      };
      {{{ copySigs('glGetString') }}}

      // Do some automatic rewriting to work around GLSL differences. Note that this must be done in
      // tandem with the rest of the program, by itself it cannot suffice.
      // Note that we need to remember shader types for this rewriting, saving sources makes it easier to debug.
      GL.shaderInfos = {};
#if GL_DEBUG
      GL.shaderSources = {};
      GL.shaderOriginalSources = {};
#endif
      var orig_glCreateShader = _glCreateShader;
      _glCreateShader = _emscripten_glCreateShader = (shaderType) => {
        var id = orig_glCreateShader(shaderType);
        GL.shaderInfos[id] = {
          type: shaderType,
          ftransform: false
        };
        return id;
      };
      {{{ copySigs('glCreateShader') }}}

      function ensurePrecision(source) {
        if (!/precision +(low|medium|high)p +float *;/.test(source)) {
          source = '#ifdef GL_FRAGMENT_PRECISION_HIGH\nprecision highp float;\n#else\nprecision mediump float;\n#endif\n' + source;
        }
        return source;
      }

      var orig_glShaderSource = _glShaderSource;
      _glShaderSource = _emscripten_glShaderSource = (shader, count, string, length) => {
        {{{ fromPtr('string') }}}
        {{{ fromPtr('length') }}}
        var source = GL.getSource(shader, count, string, length);
#if GL_DEBUG
        dbg("glShaderSource: Input: \n" + source);
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
            var need_vtc = source.search(`v_texCoord${i}`) == -1;
            source = source.replace(new RegExp(`gl_TexCoord\\[${i}\\]`, 'g'), `v_texCoord${i}`)
                           .replace(new RegExp(`gl_MultiTexCoord${i}`, 'g'), `a_texCoord${i}`);
            if (source != old) {
              source = `attribute vec4 a_texCoord${i}; \n${source}`;
              if (need_vtc) {
                source = `varying vec4 v_texCoord${i};   \n${source}`;
              }
            }

            old = source;
            source = source.replace(new RegExp(`gl_TextureMatrix\\[${i}\\]`, 'g'), `u_textureMatrix${i}`);
            if (source != old) {
              source = `uniform mat4 u_textureMatrix${i}; \n${source}`;
            }
          }
          if (source.includes('gl_FrontColor')) {
            source = 'varying vec4 v_color; \n' +
                     source.replace(/gl_FrontColor/g, 'v_color');
          }
          if (source.includes('gl_Color')) {
            source = 'attribute vec4 a_color; \n' +
                     source.replace(/gl_Color/g, 'a_color');
          }
          if (source.includes('gl_Normal')) {
            source = 'attribute vec3 a_normal; \n' +
                     source.replace(/gl_Normal/g, 'a_normal');
          }
          // fog
          if (source.includes('gl_FogFragCoord')) {
            source = 'varying float v_fogFragCoord;   \n' +
                     source.replace(/gl_FogFragCoord/g, 'v_fogFragCoord');
          }
        } else { // Fragment shader
          for (i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
            old = source;
            source = source.replace(new RegExp(`gl_TexCoord\\[${i}\\]`, 'g'), `v_texCoord${i}`);
            if (source != old) {
              source = 'varying vec4 v_texCoord' + i + ';   \n' + source;
            }
          }
          if (source.includes('gl_Color')) {
            source = 'varying vec4 v_color; \n' + source.replace(/gl_Color/g, 'v_color');
          }
          if (source.includes('gl_Fog.color')) {
            source = 'uniform vec4 u_fogColor;   \n' +
                     source.replace(/gl_Fog.color/g, 'u_fogColor');
          }
          if (source.includes('gl_Fog.end')) {
            source = 'uniform float u_fogEnd;   \n' +
                     source.replace(/gl_Fog.end/g, 'u_fogEnd');
          }
          if (source.includes('gl_Fog.scale')) {
            source = 'uniform float u_fogScale;   \n' +
                     source.replace(/gl_Fog.scale/g, 'u_fogScale');
          }
          if (source.includes('gl_Fog.density')) {
            source = 'uniform float u_fogDensity;   \n' +
                     source.replace(/gl_Fog.density/g, 'u_fogDensity');
          }
          if (source.includes('gl_FogFragCoord')) {
            source = 'varying float v_fogFragCoord;   \n' +
                     source.replace(/gl_FogFragCoord/g, 'v_fogFragCoord');
          }
          source = ensurePrecision(source);
        }
#if GL_DEBUG
        GL.shaderSources[shader] = source;
        dbg("glShaderSource: Output: \n" + source);
#endif
        GLctx.shaderSource(GL.shaders[shader], source);
      };
      {{{ copySigs('glShaderSource') }}}

      var orig_glCompileShader = _glCompileShader;
      _glCompileShader = _emscripten_glCompileShader = (shader) => {
        GLctx.compileShader(GL.shaders[shader]);
#if GL_DEBUG
        if (!GLctx.getShaderParameter(GL.shaders[shader], GLctx.COMPILE_STATUS)) {
          dbg(`Failed to compile shader: ${GLctx.getShaderInfoLog(GL.shaders[shader])}`);
          dbg(`Info: ${JSON.stringify(GL.shaderInfos[shader])}`);
          dbg(`Original source: ${GL.shaderOriginalSources[shader]}`);
          dbg(`Source: ${GL.shaderSources[shader]}`);
          throw 'Shader compilation halt';
        }
#endif
      };
      {{{ copySigs('glCompileShader') }}}

      GL.programShaders = {};
      var orig_glAttachShader = _glAttachShader;
      _glAttachShader = _emscripten_glAttachShader = (program, shader) => {
        GL.programShaders[program] ||= [];
        GL.programShaders[program].push(shader);
        orig_glAttachShader(program, shader);
      };
      {{{ copySigs('glAttachShader') }}}

      var orig_glDetachShader = _glDetachShader;
      _glDetachShader = _emscripten_glDetachShader = (program, shader) => {
        var programShader = GL.programShaders[program];
        if (!programShader) {
          err(`WARNING: _glDetachShader received invalid program: ${program}`);
          return;
        }
        var index = programShader.indexOf(shader);
        programShader.splice(index, 1);
        orig_glDetachShader(program, shader);
      };
      {{{ copySigs('glDetachShader') }}}

      var orig_glUseProgram = _glUseProgram;
      _glUseProgram = _emscripten_glUseProgram = (program) => {
#if GL_DEBUG
        if (GL.debug) {
          dbg('[using program with shaders]');
          if (program) {
            GL.programShaders[program].forEach((shader) => {
              dbg(`  shader ${shader}, original source: ${GL.shaderOriginalSources[shader]}`);
              dbg(`         Source: ${GL.shaderSources[shader]}`);
            });
          }
        }
#endif
        if (GL.currProgram != program) {
          GLImmediate.currentRenderer = null; // This changes the FFP emulation shader program, need to recompute that.
          GL.currProgram = program;
          GLImmediate.fixedFunctionProgram = 0;
          orig_glUseProgram(program);
        }
      }
      {{{ copySigs('glUseProgram') }}}

      var orig_glDeleteProgram = _glDeleteProgram;
      _glDeleteProgram = _emscripten_glDeleteProgram = (program) => {
        orig_glDeleteProgram(program);
        if (program == GL.currProgram) {
          GLImmediate.currentRenderer = null; // This changes the FFP emulation shader program, need to recompute that.
          GL.currProgram = 0;
        }
      };
      {{{ copySigs('glDeleteProgram') }}}

      // If attribute 0 was not bound, bind it to 0 for WebGL performance reasons. Track if 0 is free for that.
      var zeroUsedPrograms = {};
      var orig_glBindAttribLocation = _glBindAttribLocation;
      _glBindAttribLocation = _emscripten_glBindAttribLocation = (program, index, name) => {
        if (index == 0) zeroUsedPrograms[program] = true;
        orig_glBindAttribLocation(program, index, name);
      };
      {{{ copySigs('glBindAttribLocation') }}}

      var orig_glLinkProgram = _glLinkProgram;
      _glLinkProgram = _emscripten_glLinkProgram = (program) => {
        if (!(program in zeroUsedPrograms)) {
          GLctx.bindAttribLocation(GL.programs[program], 0, 'a_position');
        }
        orig_glLinkProgram(program);
      };
      {{{ copySigs('glLinkProgram') }}}

      var orig_glBindBuffer = _glBindBuffer;
      _glBindBuffer = _emscripten_glBindBuffer = (target, buffer) => {
        orig_glBindBuffer(target, buffer);
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
      {{{ copySigs('glBindBuffer') }}}

      var orig_glGetFloatv = _glGetFloatv;
      _glGetFloatv = _emscripten_glGetFloatv = (pname, params) => {
        {{{ fromPtr('params') }}}
        if (pname == 0xBA6) { // GL_MODELVIEW_MATRIX
          HEAPF32.set(GLImmediate.matrix[0/*m*/], {{{ getHeapOffset('params', 'float') }}});
        } else if (pname == 0xBA7) { // GL_PROJECTION_MATRIX
          HEAPF32.set(GLImmediate.matrix[1/*p*/], {{{ getHeapOffset('params', 'float') }}});
        } else if (pname == 0xBA8) { // GL_TEXTURE_MATRIX
          HEAPF32.set(GLImmediate.matrix[2/*t*/ + GLImmediate.clientActiveTexture], {{{ getHeapOffset('params', 'float') }}});
        } else if (pname == 0xB66) { // GL_FOG_COLOR
          HEAPF32.set(GLEmulation.fogColor, {{{ getHeapOffset('params', 'float') }}});
        } else if (pname == 0xB63) { // GL_FOG_START
          {{{ makeSetValue('params', '0', 'GLEmulation.fogStart', 'float') }}};
        } else if (pname == 0xB64) { // GL_FOG_END
          {{{ makeSetValue('params', '0', 'GLEmulation.fogEnd', 'float') }}};
        } else if (pname == 0xB62) { // GL_FOG_DENSITY
          {{{ makeSetValue('params', '0', 'GLEmulation.fogDensity', 'float') }}};
        } else if (pname == 0xB65) { // GL_FOG_MODE
          {{{ makeSetValue('params', '0', 'GLEmulation.fogMode', 'float') }}};
        } else if (pname == 0xB53) { // GL_LIGHT_MODEL_AMBIENT
          {{{ makeSetValue('params', '0', 'GLEmulation.lightModelAmbient[0]', 'float') }}};
          {{{ makeSetValue('params', '4', 'GLEmulation.lightModelAmbient[1]', 'float') }}};
          {{{ makeSetValue('params', '8', 'GLEmulation.lightModelAmbient[2]', 'float') }}};
          {{{ makeSetValue('params', '12', 'GLEmulation.lightModelAmbient[3]', 'float') }}};
        } else if (pname == 0xBC2) { // GL_ALPHA_TEST_REF
          {{{ makeSetValue('params', '0', 'GLEmulation.alphaTestRef', 'float') }}};
        } else {
          orig_glGetFloatv(pname, params);
        }
      };
      {{{ copySigs('glGetFloatv') }}}

      var orig_glHint = _glHint;
      _glHint = _emscripten_glHint = (target, mode) => {
        if (target == 0x84EF) { // GL_TEXTURE_COMPRESSION_HINT
          return;
        }
        orig_glHint(target, mode);
      };
      {{{ copySigs('glHint') }}}

      var orig_glEnableVertexAttribArray = _glEnableVertexAttribArray;
      _glEnableVertexAttribArray = _emscripten_glEnableVertexAttribArray = (index) => {
        orig_glEnableVertexAttribArray(index);
        GLEmulation.enabledVertexAttribArrays[index] = 1;
        if (GLEmulation.currentVao) GLEmulation.currentVao.enabledVertexAttribArrays[index] = 1;
      };
      {{{ copySigs('glEnableVertexAttribArray') }}}

      var orig_glDisableVertexAttribArray = _glDisableVertexAttribArray;
      _glDisableVertexAttribArray = _emscripten_glDisableVertexAttribArray = (index) => {
        orig_glDisableVertexAttribArray(index);
        delete GLEmulation.enabledVertexAttribArrays[index];
        if (GLEmulation.currentVao) delete GLEmulation.currentVao.enabledVertexAttribArrays[index];
      };
      {{{ copySigs('glDisableVertexAttribArray') }}}

      var orig_glVertexAttribPointer = _glVertexAttribPointer;
      _glVertexAttribPointer = _emscripten_glVertexAttribPointer = (index, size, type, normalized, stride, pointer) => {
        orig_glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        if (GLEmulation.currentVao) { // TODO: avoid object creation here? likely not hot though
          GLEmulation.currentVao.vertexAttribPointers[index] = [index, size, type, normalized, stride, pointer];
        }
      };
      {{{ copySigs('glVertexAttribPointer') }}}
    },

    getAttributeFromCapability(cap) {
      var attrib = null;
      switch (cap) {
        case 0xDE1: // GL_TEXTURE_2D - XXX not according to spec, and not in desktop GL, but works in some GLES1.x apparently, so support it
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
  glDeleteObject: (id) => {
    if (GL.programs[id]) {
      _glDeleteProgram(id);
    } else if (GL.shaders[id]) {
      _glDeleteShader(id);
    } else {
      err(`WARNING: deleteObject received invalid id: ${id}`);
    }
  },
  glDeleteObjectARB: 'glDeleteObject',

  glGetObjectParameteriv__deps: ['glGetProgramiv', 'glGetShaderiv'],
  glGetObjectParameteriv: (id, type, result) => {
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
      err(`WARNING: getObjectParameteriv received invalid id: ${id}`);
    }
  },
  glGetObjectParameterivARB: 'glGetObjectParameteriv',

  glGetInfoLog__deps: ['glGetProgramInfoLog', 'glGetShaderInfoLog'],
  glGetInfoLog: (id, maxLength, length, infoLog) => {
    if (GL.programs[id]) {
      _glGetProgramInfoLog(id, maxLength, length, infoLog);
    } else if (GL.shaders[id]) {
      _glGetShaderInfoLog(id, maxLength, length, infoLog);
    } else {
      err(`WARNING: glGetInfoLog received invalid id: ${id}`);
    }
  },
  glGetInfoLogARB: 'glGetInfoLog',

  glBindProgram: (type, id) => {
#if ASSERTIONS
    assert(id == 0);
#endif
  },
  glBindProgramARB: 'glBindProgram',

  glGetPointerv: (name, p) => {
    var attribute;
    switch (name) {
      case 0x808E: // GL_VERTEX_ARRAY_POINTER
        attribute = GLImmediate.clientAttributes[GLImmediate.VERTEX]; break;
      case 0x8090: // GL_COLOR_ARRAY_POINTER
        attribute = GLImmediate.clientAttributes[GLImmediate.COLOR]; break;
      case 0x8092: // GL_TEXTURE_COORD_ARRAY_POINTER
        attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture]; break;
      default:
        GL.recordError(0x500/*GL_INVALID_ENUM*/);
#if GL_ASSERTIONS
        err(`GL_INVALID_ENUM in glGetPointerv: Unsupported name ${name}!`);
#endif
        return;
    }
    {{{ makeSetValue('p', '0', 'attribute ? attribute.pointer : 0', 'i32') }}};
  },

  // GL Immediate mode

  // See comment in GLEmulation.init()
#if !FULL_ES2
  $GLImmediate__postset: 'GLImmediate.setupFuncs(); Browser.moduleContextCreatedCallbacks.push(() => GLImmediate.init());',
#endif
  $GLImmediate__deps: ['$Browser', '$GL', '$GLEmulation'],
  $GLImmediate: {
    MapTreeLib: null,
    spawnMapTreeLib: () => {
      /**
       * A naive implementation of a map backed by an array, and accessed by
       * naive iteration along the array. (hashmap with only one bucket)
       * @constructor
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

      /**
       * A tree of map nodes.
       * Uses `KeyView`s to allow descending the tree without garbage.
       * Example: {
       *   // Create our map object.
       *   var map = new ObjTreeMap();
       *
       *   // Grab the static keyView for the map.
       *   var keyView = map.GetStaticKeyView();
       *
       *   // Let's make a map for:
       *   // root: <undefined>
       *   //   1: <undefined>
       *   //     2: <undefined>
       *   //       5: "Three, sir!"
       *   //       3: "Three!"
       *
       *   // Note how we can chain together `Reset` and `Next` to
       *   // easily descend based on multiple key fragments.
       *   keyView.Reset().Next(1).Next(2).Next(5).Set("Three, sir!");
       *   keyView.Reset().Next(1).Next(2).Next(3).Set("Three!");
       * }
       * @constructor
       */
      function CMapTree() {
        /** @constructor */
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

        /** @constructor */
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
        create: () => new CMapTree(),
      };
    },

    TexEnvJIT: null,
    spawnTexEnvJIT: () => {
      // GL defs:
      var GL_TEXTURE0 = 0x84C0;
      var GL_TEXTURE_1D = 0xDE0;
      var GL_TEXTURE_2D = 0xDE1;
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
      var GL_ALPHA_SCALE = 0xD1C;

      // env.mode
      var GL_ADD      = 0x104;
      var GL_BLEND    = 0xBE2;
      var GL_REPLACE  = 0x1E01;
      var GL_MODULATE = 0x2100;
      var GL_DECAL    = 0x2101;
      var GL_COMBINE  = 0x8570;

      // env.color/alphaCombiner
      //var GL_ADD         = 0x104;
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
      var GL_SRC_COLOR           = 0x300;
      var GL_ONE_MINUS_SRC_COLOR = 0x301;
      var GL_SRC_ALPHA           = 0x302;
      var GL_ONE_MINUS_SRC_ALPHA = 0x303;

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
            return abort_sanity(`Unknown texType: ${ptrToString(texType)}`);
        }

        var texCoordExpr = TEX_COORD_VARYING_PREFIX + texUnitID;
        if (TEX_MATRIX_UNIFORM_PREFIX != null) {
          texCoordExpr = `(${TEX_MATRIX_UNIFORM_PREFIX}${texUnitID} * ${texCoordExpr})`;
        }
        return `${func}(${TEX_UNIT_UNIFORM_PREFIX}${texUnitID}, ${texCoordExpr}.xy)`;
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

        return abort_noSupport("Unsupported combiner op: " + ptrToString(op));
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
              return abort_noSupport("Unsupported combiner src: " + ptrToString(src));
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
            return abort_noSupport("Unsupported combiner op: " + ptrToString(op));
        }

        return expr;
      }

      function valToFloatLiteral(val) {
        if (val == Math.round(val)) return val + '.0';
        return val;
      }


      // Classes:
      /** @constructor */
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
          0x104 /* GL_ADD */: 2,
          0xBE2 /* GL_BLEND */: 3,
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
          0x300 /* GL_SRC_COLOR */: 0,
          0x301 /* GL_ONE_MINUS_SRC_COLOR */: 1,
          0x302 /* GL_SRC_ALPHA */: 2,
          0x303 /* GL_ONE_MINUS_SRC_ALPHA */: 3
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
          var key = k[this.colorOp[0]] * 4096;
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

      /** @constructor */
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
        while (load = texLoadRegex.exec(lines)) {
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

        return abort_noSupport("Unsupported TexEnv mode: " + ptrToString(this.mode));
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
            return abort_noSupport("Unsupported combiner: " + ptrToString(combiner));
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
            lines = [`${outputType} ${outputVar} = ${src0Expr};`]
            break;
          }
          case GL_MODULATE: {
            lines = [`${outputType} ${outputVar} = ${src0Expr} * ${src1Expr};`];
            break;
          }
          case GL_ADD: {
            lines = [`${outputType} ${outputVar} = ${src0Expr} + ${src1Expr};`]
            break;
          }
          case GL_SUBTRACT: {
            lines = [`${outputType} ${outputVar} = ${src0Expr} - ${src1Expr};`]
            break;
          }
          case GL_INTERPOLATE: {
            var prefix = `${TEXENVJIT_NAMESPACE_PREFIX}env${texUnitID}_`;
            var arg2Var = `${prefix}colorSrc2`;
            var arg2Type = getTypeFromCombineOp(this.colorOp[2]);

            lines = [
              `${arg2Type} ${arg2Var} = ${src2Expr};`,
              `${outputType} ${outputVar} = ${src0Expr} * ${arg2Var} + ${src1Expr} * (1.0 - ${arg2Var});`,
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
        init: (gl, specifiedMaxTextureImageUnits) => {
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

        setGLSLVars: (uTexUnitPrefix, vTexCoordPrefix, vPrimColor, uTexMatrixPrefix) => {
          TEX_UNIT_UNIFORM_PREFIX   = uTexUnitPrefix;
          TEX_COORD_VARYING_PREFIX  = vTexCoordPrefix;
          PRIM_COLOR_VARYING        = vPrimColor;
          TEX_MATRIX_UNIFORM_PREFIX = uTexMatrixPrefix;
        },

        genAllPassLines: (resultDest, indentSize = 0) => {
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

        getUsedTexUnitList: () => s_requiredTexUnitsForPass,

        getActiveTexture: () => s_activeTexture,

        traverseState: (keyView) => {
          for (var i = 0; i < s_texUnits.length; i++) {
            s_texUnits[i].traverseState(keyView);
          }
        },

        getTexUnitType: (texUnitID) => {
#if ASSERTIONS
          assert(texUnitID >= 0 &&
                 texUnitID < s_texUnits.length);
#endif
          return s_texUnits[texUnitID].getTexType();
        },

        // Hooks:
        hook_activeTexture: (texture) => {
          s_activeTexture = texture - GL_TEXTURE0;
          // Check if the current matrix mode is GL_TEXTURE.
          if (GLImmediate.currentMatrix >= 2) {
            // Switch to the corresponding texture matrix stack.
            GLImmediate.currentMatrix = 2 + s_activeTexture;
          }
        },

        hook_enable: (cap) => {
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

        hook_disable: (cap) => {
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

        hook_texEnvf(target, pname, param) {
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

        hook_texEnvi(target, pname, param) {
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

        hook_texEnvfv(target, pname, params) {
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

        hook_getTexEnviv(target, pname, param) {
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

        hook_getTexEnvfv: (target, pname, param) => {
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

    setClientAttribute(name, size, type, stride, pointer) {
      var attrib = GLImmediate.clientAttributes[name];
      if (!attrib) {
        for (var i = 0; i <= name; i++) { // keep flat
          GLImmediate.clientAttributes[i] ||= {
            name,
            size,
            type,
            stride,
            pointer,
            offset: 0
          };
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
    addRendererComponent(name, size, type) {
      if (!GLImmediate.rendererComponents[name]) {
        GLImmediate.rendererComponents[name] = 1;
#if ASSERTIONS
        if (GLImmediate.enabledClientAttributes[name]) {
          warnOnce("Warning: glTexCoord used after EnableClientState for TEXTURE_COORD_ARRAY for TEXTURE0. Disabling TEXTURE_COORD_ARRAY...");
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

    disableBeginEndClientAttributes() {
      for (var i = 0; i < GLImmediate.NUM_ATTRIBUTES; i++) {
        if (GLImmediate.rendererComponents[i]) GLImmediate.enabledClientAttributes[i] = false;
      }
    },

    getRenderer() {
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

      // To prevent using more than 31 bits add another level to the maptree
      // and reset the enabledAttributesKey for the next glemulation state bits
      keyView.next(enabledAttributesKey);
      enabledAttributesKey = 0;

      // By fog state:
      var fogParam = 0;
      if (GLEmulation.fogEnabled) {
        switch (GLEmulation.fogMode) {
          case 0x801: // GL_EXP2
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
      enabledAttributesKey = (enabledAttributesKey << 2) | fogParam;

      // By clip plane mode
      for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
        enabledAttributesKey = (enabledAttributesKey << 1) | GLEmulation.clipPlaneEnabled[clipPlaneId];
      }

      // By lighting mode and enabled lights
      enabledAttributesKey = (enabledAttributesKey << 1) | GLEmulation.lightingEnabled;
      for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
        enabledAttributesKey = (enabledAttributesKey << 1) | (GLEmulation.lightingEnabled ? GLEmulation.lightEnabled[lightId] : 0);
      }

      // By alpha testing mode
      enabledAttributesKey = (enabledAttributesKey << 3) | (GLEmulation.alphaTestEnabled ? (GLEmulation.alphaTestFunc - 0x200) : 0x7);

      // By drawing mode:
      enabledAttributesKey = (enabledAttributesKey << 1) | (GLImmediate.mode == GLctx.POINTS ? 1 : 0);

      keyView.next(enabledAttributesKey);

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
        dbg(`generating renderer for ${JSON.stringify(attributes)}`);
#endif
        renderer = GLImmediate.createRenderer();
        GLImmediate.currentRenderer = renderer;
        keyView.set(renderer);
        return renderer;
      }
      GLImmediate.currentRenderer = renderer; // Cache the currently used renderer, so later lookups without state changes can get this fast.
      return renderer;
    },

    createRenderer(renderer) {
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

      /** @constructor */
      function Renderer() {
        this.init = function() {
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
                case 0x801: // GL_EXP2
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

            var vsPointSizeDefs = null;
            var vsPointSizeInit = null;
            if (GLImmediate.mode == GLctx.POINTS) {
              vsPointSizeDefs = 'uniform float u_pointSize;\n';
              vsPointSizeInit = '  gl_PointSize = u_pointSize;\n';
            }

            var vsClipPlaneDefs = '';
            var vsClipPlaneInit = '';
            var fsClipPlaneDefs = '';
            var fsClipPlanePass = '';
            for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
              if (GLEmulation.clipPlaneEnabled[clipPlaneId]) {
                vsClipPlaneDefs += 'uniform vec4 u_clipPlaneEquation' + clipPlaneId + ';';
                vsClipPlaneDefs += 'varying float v_clipDistance' + clipPlaneId + ';';
                vsClipPlaneInit += '  v_clipDistance' + clipPlaneId + ' = dot(ecPosition, u_clipPlaneEquation' + clipPlaneId + ');';
                fsClipPlaneDefs += 'varying float v_clipDistance' + clipPlaneId + ';';
                fsClipPlanePass += '  if (v_clipDistance' + clipPlaneId + ' < 0.0) discard;';
              }
            }

            var vsLightingDefs = '';
            var vsLightingPass = '';
            if (GLEmulation.lightingEnabled) {
              vsLightingDefs += 'attribute vec3 a_normal;';
              vsLightingDefs += 'uniform mat3 u_normalMatrix;';
              vsLightingDefs += 'uniform vec4 u_lightModelAmbient;';
              vsLightingDefs += 'uniform vec4 u_materialAmbient;';
              vsLightingDefs += 'uniform vec4 u_materialDiffuse;';
              vsLightingDefs += 'uniform vec4 u_materialSpecular;';
              vsLightingDefs += 'uniform float u_materialShininess;';
              vsLightingDefs += 'uniform vec4 u_materialEmission;';

              vsLightingPass += '  vec3 ecNormal = normalize(u_normalMatrix * a_normal);';
              vsLightingPass += '  v_color.w = u_materialDiffuse.w;';
              vsLightingPass += '  v_color.xyz = u_materialEmission.xyz;';
              vsLightingPass += '  v_color.xyz += u_lightModelAmbient.xyz * u_materialAmbient.xyz;';

              for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
                if (GLEmulation.lightEnabled[lightId]) {
                  vsLightingDefs += 'uniform vec4 u_lightAmbient' + lightId + ';';
                  vsLightingDefs += 'uniform vec4 u_lightDiffuse' + lightId + ';';
                  vsLightingDefs += 'uniform vec4 u_lightSpecular' + lightId + ';';
                  vsLightingDefs += 'uniform vec4 u_lightPosition' + lightId + ';';

                  vsLightingPass += '  {';
                  vsLightingPass += '    vec3 lightDirection = normalize(u_lightPosition' + lightId + ').xyz;';
                  vsLightingPass += '    vec3 halfVector = normalize(lightDirection + vec3(0,0,1));';
                  vsLightingPass += '    vec3 ambient = u_lightAmbient' + lightId + '.xyz * u_materialAmbient.xyz;';
                  vsLightingPass += '    float diffuseI = max(dot(ecNormal, lightDirection), 0.0);';
                  vsLightingPass += '    float specularI = max(dot(ecNormal, halfVector), 0.0);';
                  vsLightingPass += '    vec3 diffuse = diffuseI * u_lightDiffuse' + lightId + '.xyz * u_materialDiffuse.xyz;';
                  vsLightingPass += '    specularI = (diffuseI > 0.0 && specularI > 0.0) ? exp(u_materialShininess * log(specularI)) : 0.0;';
                  vsLightingPass += '    vec3 specular = specularI * u_lightSpecular' + lightId + '.xyz * u_materialSpecular.xyz;';
                  vsLightingPass += '    v_color.xyz += ambient + diffuse + specular;';
                  vsLightingPass += '  }';
                }
              }
              vsLightingPass += '  v_color = clamp(v_color, 0.0, 1.0);';
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
              vsPointSizeDefs,
              vsClipPlaneDefs,
              vsLightingDefs,
              'void main()',
              '{',
              '  vec4 ecPosition = u_modelView * a_position;', // eye-coordinate position
              '  gl_Position = u_projection * ecPosition;',
              '  v_color = a_color;',
              vsTexCoordInits,
              vsFogVaryingInit,
              vsPointSizeInit,
              vsClipPlaneInit,
              vsLightingPass,
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

            var fsAlphaTestDefs = '';
            var fsAlphaTestPass = '';
            if (GLEmulation.alphaTestEnabled) {
              fsAlphaTestDefs = 'uniform float u_alphaTestRef;';
              switch (GLEmulation.alphaTestFunc) {
                case 0x200: // GL_NEVER
                  fsAlphaTestPass = 'discard;';
                  break;
                case 0x201: // GL_LESS
                  fsAlphaTestPass = 'if (!(gl_FragColor.a < u_alphaTestRef)) { discard; }';
                  break;
                case 0x202: // GL_EQUAL
                  fsAlphaTestPass = 'if (!(gl_FragColor.a == u_alphaTestRef)) { discard; }';
                  break;
                case 0x203: // GL_LEQUAL
                  fsAlphaTestPass = 'if (!(gl_FragColor.a <= u_alphaTestRef)) { discard; }';
                  break;
                case 0x204: // GL_GREATER
                  fsAlphaTestPass = 'if (!(gl_FragColor.a > u_alphaTestRef)) { discard; }';
                  break;
                case 0x205: // GL_NOTEQUAL
                  fsAlphaTestPass = 'if (!(gl_FragColor.a != u_alphaTestRef)) { discard; }';
                  break;
                case 0x206: // GL_GEQUAL
                  fsAlphaTestPass = 'if (!(gl_FragColor.a >= u_alphaTestRef)) { discard; }';
                  break;
                case 0x207: // GL_ALWAYS
                  fsAlphaTestPass = '';
                  break;
              }
            }

            var fsSource = [
              'precision mediump float;',
              texUnitVaryingList,
              texUnitUniformList,
              'varying vec4 v_color;',
              fogHeaderIfNeeded,
              fsClipPlaneDefs,
              fsAlphaTestDefs,
              'void main()',
              '{',
              fsClipPlanePass,
              fsTexEnvPass,
              fogPass,
              fsAlphaTestPass,
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
              this.texCoordLocations[i] = GLctx.getAttribLocation(this.program, `a_texCoord${i}`);
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
            this.textureMatrixLocations[i] = GLctx.getUniformLocation(this.program, `u_textureMatrix${i}`);
          }
          this.normalLocation = GLctx.getAttribLocation(this.program, 'a_normal');

          this.modelViewLocation = GLctx.getUniformLocation(this.program, 'u_modelView');
          this.projectionLocation = GLctx.getUniformLocation(this.program, 'u_projection');
          this.normalMatrixLocation = GLctx.getUniformLocation(this.program, 'u_normalMatrix');

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

          this.pointSizeLocation = GLctx.getUniformLocation(this.program, 'u_pointSize');

          this.hasClipPlane = false;
          this.clipPlaneEquationLocation = [];
          for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
            this.clipPlaneEquationLocation[clipPlaneId] = GLctx.getUniformLocation(this.program, `u_clipPlaneEquation${clipPlaneId}`);
            this.hasClipPlane = (this.hasClipPlane || this.clipPlaneEquationLocation[clipPlaneId]);
          }

          this.hasLighting = GLEmulation.lightingEnabled;
          this.lightModelAmbientLocation = GLctx.getUniformLocation(this.program, 'u_lightModelAmbient');
          this.materialAmbientLocation = GLctx.getUniformLocation(this.program, 'u_materialAmbient');
          this.materialDiffuseLocation = GLctx.getUniformLocation(this.program, 'u_materialDiffuse');
          this.materialSpecularLocation = GLctx.getUniformLocation(this.program, 'u_materialSpecular');
          this.materialShininessLocation = GLctx.getUniformLocation(this.program, 'u_materialShininess');
          this.materialEmissionLocation = GLctx.getUniformLocation(this.program, 'u_materialEmission');
          this.lightAmbientLocation = []
          this.lightDiffuseLocation = []
          this.lightSpecularLocation = []
          this.lightPositionLocation = []
          for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
            this.lightAmbientLocation[lightId] = GLctx.getUniformLocation(this.program, `u_lightAmbient${lightId}`);
            this.lightDiffuseLocation[lightId] = GLctx.getUniformLocation(this.program, `u_lightDiffuse${lightId}`);
            this.lightSpecularLocation[lightId] = GLctx.getUniformLocation(this.program, `u_lightSpecular${lightId}`);
            this.lightPositionLocation[lightId] = GLctx.getUniformLocation(this.program, `u_lightPosition${lightId}`);
          }

          this.hasAlphaTest = GLEmulation.alphaTestEnabled;
          this.alphaTestRefLocation = GLctx.getUniformLocation(this.program, 'u_alphaTestRef');

        };

        this.prepare = function() {
          // Calculate the array buffer
          var arrayBuffer;
          if (!GLctx.currentArrayBufferBinding) {
            var start = GLImmediate.firstVertex*GLImmediate.stride;
            var end = GLImmediate.lastVertex*GLImmediate.stride;
#if ASSERTIONS
            assert(end <= GL.MAX_TEMP_BUFFER_SIZE, 'too much vertex data');
#endif
            arrayBuffer = GL.getTempVertexBuffer(end);
            // TODO: consider using the last buffer we bound, if it was larger. downside is larger buffer, but we might avoid rebinding and preparing
          } else {
            arrayBuffer = GLctx.currentArrayBufferBinding;
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
          if (!GLctx.currentArrayBufferBinding) {
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
          GLImmediate.lastStride = GLImmediate.stride;
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

            // set normal matrix to the upper 3x3 of the inverse transposed current modelview matrix
            if (GLEmulation.lightEnabled) {
              var tmpMVinv = GLImmediate.matrixLib.mat4.create(GLImmediate.matrix[0]);
              GLImmediate.matrixLib.mat4.inverse(tmpMVinv);
              GLImmediate.matrixLib.mat4.transpose(tmpMVinv);
              GLctx.uniformMatrix3fv(this.normalMatrixLocation, false, GLImmediate.matrixLib.mat4.toMat3(tmpMVinv));
            }
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
          if (!GLctx.currentArrayBufferBinding) {
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
              if (!GLctx.currentArrayBufferBinding) {
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
            if (!GLctx.currentArrayBufferBinding) {
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

          if (this.hasClipPlane) {
            for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
              if (this.clipPlaneEquationLocation[clipPlaneId]) GLctx.uniform4fv(this.clipPlaneEquationLocation[clipPlaneId], GLEmulation.clipPlaneEquation[clipPlaneId]);
            }
          }

          if (this.hasLighting) {
            if (this.lightModelAmbientLocation) GLctx.uniform4fv(this.lightModelAmbientLocation, GLEmulation.lightModelAmbient);
            if (this.materialAmbientLocation) GLctx.uniform4fv(this.materialAmbientLocation, GLEmulation.materialAmbient);
            if (this.materialDiffuseLocation) GLctx.uniform4fv(this.materialDiffuseLocation, GLEmulation.materialDiffuse);
            if (this.materialSpecularLocation) GLctx.uniform4fv(this.materialSpecularLocation, GLEmulation.materialSpecular);
            if (this.materialShininessLocation) GLctx.uniform1f(this.materialShininessLocation, GLEmulation.materialShininess[0]);
            if (this.materialEmissionLocation) GLctx.uniform4fv(this.materialEmissionLocation, GLEmulation.materialEmission);
            for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
              if (this.lightAmbientLocation[lightId]) GLctx.uniform4fv(this.lightAmbientLocation[lightId], GLEmulation.lightAmbient[lightId]);
              if (this.lightDiffuseLocation[lightId]) GLctx.uniform4fv(this.lightDiffuseLocation[lightId], GLEmulation.lightDiffuse[lightId]);
              if (this.lightSpecularLocation[lightId]) GLctx.uniform4fv(this.lightSpecularLocation[lightId], GLEmulation.lightSpecular[lightId]);
              if (this.lightPositionLocation[lightId]) GLctx.uniform4fv(this.lightPositionLocation[lightId], GLEmulation.lightPosition[lightId]);
            }
          }

          if (this.hasAlphaTest) {
            if (this.alphaTestRefLocation) GLctx.uniform1f(this.alphaTestRefLocation, GLEmulation.alphaTestRef);
          }

          if (GLImmediate.mode == GLctx.POINTS) {
            if (this.pointSizeLocation) {
              GLctx.uniform1f(this.pointSizeLocation, GLEmulation.pointSize);
            }
          }
        };

        this.cleanup = function() {
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
          if (!GLctx.currentArrayBufferBinding) {
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

        this.init();
      }
      return new Renderer();
    },

    setupFuncs() {
      // TexEnv stuff needs to be prepared early, so do it here.
      // init() is too late for -O2, since it freezes the GL functions
      // by that point.
      GLImmediate.MapTreeLib = GLImmediate.spawnMapTreeLib();
      GLImmediate.spawnMapTreeLib = null;

      GLImmediate.TexEnvJIT = GLImmediate.spawnTexEnvJIT();
      GLImmediate.spawnTexEnvJIT = null;

      GLImmediate.setupHooks();
    },

    setupHooks() {
      if (!GLEmulation.hasRunInit) {
        GLEmulation.init();
      }

      var glActiveTexture = _glActiveTexture;
      _glActiveTexture = _emscripten_glActiveTexture = (texture) => {
        GLImmediate.TexEnvJIT.hook_activeTexture(texture);
        glActiveTexture(texture);
      };

      var glEnable = _glEnable;
      _glEnable = _emscripten_glEnable = (cap) => {
        GLImmediate.TexEnvJIT.hook_enable(cap);
        glEnable(cap);
      };

      var glDisable = _glDisable;
      _glDisable = _emscripten_glDisable = (cap) => {
        GLImmediate.TexEnvJIT.hook_disable(cap);
        glDisable(cap);
      };

      var glTexEnvf = (typeof _glTexEnvf != 'undefined') ? _glTexEnvf : () => {};
      /** @suppress {checkTypes} */
      _glTexEnvf = _emscripten_glTexEnvf = (target, pname, param) => {
        GLImmediate.TexEnvJIT.hook_texEnvf(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvf(target, pname, param);
      };

      var glTexEnvi = (typeof _glTexEnvi != 'undefined') ? _glTexEnvi : () => {};
      /** @suppress {checkTypes} */
      _glTexEnvi = _emscripten_glTexEnvi = (target, pname, param) => {
        {{{ fromPtr('param') }}}
        GLImmediate.TexEnvJIT.hook_texEnvi(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvi(target, pname, param);
      };

      var glTexEnvfv = (typeof _glTexEnvfv != 'undefined') ? _glTexEnvfv : () => {};
      /** @suppress {checkTypes} */
      _glTexEnvfv = _emscripten_glTexEnvfv = (target, pname, param) => {
        {{{ fromPtr('param') }}}
        GLImmediate.TexEnvJIT.hook_texEnvfv(target, pname, param);
        // Don't call old func, since we are the implementor.
        //glTexEnvfv(target, pname, param);
      };

      _glGetTexEnviv = (target, pname, param) => {
        {{{ fromPtr('param') }}}
        GLImmediate.TexEnvJIT.hook_getTexEnviv(target, pname, param);
      };

      _glGetTexEnvfv = (target, pname, param) => {
        {{{ fromPtr('param') }}}
        GLImmediate.TexEnvJIT.hook_getTexEnvfv(target, pname, param);
      };

      var glGetIntegerv = _glGetIntegerv;
      _glGetIntegerv = _emscripten_glGetIntegerv = (pname, params) => {
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
    },

    // Main functions
    initted: false,
    init() {
      err('WARNING: using emscripten GL immediate mode emulation. This is very limited in what it supports');
      GLImmediate.initted = true;

      if (!Browser.useWebGL) return; // a 2D canvas may be currently used TODO: make sure we are actually called in that case

      // User can override the maximum number of texture units that we emulate. Using fewer texture units increases runtime performance
      // slightly, so it is advantageous to choose as small value as needed.
      // Limit to a maximum of 28 to not overflow the state bits used for renderer caching (31 bits = 3 attributes + 28 texture units).
      GLImmediate.MAX_TEXTURES = Math.min(Module['GL_MAX_TEXTURE_IMAGE_UNITS'] || GLctx.getParameter(GLctx.MAX_TEXTURE_IMAGE_UNITS), 28);

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
    prepareClientAttributes(count, beginEnd) {
      // If no client attributes were modified since we were last called, do
      // nothing. Note that this does not work for glBegin/End, where we
      // generate renderer components dynamically and then disable them
      // ourselves, but it does help with glDrawElements/Arrays.
      if (!GLImmediate.modifiedClientAttributes) {
#if GL_ASSERTIONS
        if ((GLImmediate.stride & 3) != 0) {
          warnOnce(`Warning: Rendering from client side vertex arrays where stride (${GLImmediate.stride}) is not a multiple of four! This is not currently supported!`);
        }
#endif
        GLImmediate.vertexCounter = (GLImmediate.stride * count) / 4; // XXX assuming float
        return;
      }
      GLImmediate.modifiedClientAttributes = false;

      // The role of prepareClientAttributes is to examine the set of
      // client-side vertex attribute buffers that user code has submitted, and
      // to prepare them to be uploaded to a VBO in GPU memory (since WebGL does
      // not support client-side rendering, i.e. rendering from vertex data in
      // CPU memory). User can submit vertex data generally in three different
      // configurations:
      // 1. Fully planar: all attributes are in their own separate
      //                  tightly-packed arrays in CPU memory.
      // 2. Fully interleaved: all attributes share a single array where data is
      //                       interleaved something like (pos,uv,normal),
      //                       (pos,uv,normal), ...
      // 3. Complex hybrid: Multiple separate arrays that either are sparsely
      //                    strided, and/or partially interleaves vertex
      //                    attributes.

      // For simplicity, we support the case (2) as the fast case. For (1) and
      // (3), we do a memory copy of the vertex data here to prepare a
      // relayouted buffer that is of the structure in case (2). The reason
      // for this is that it allows the emulation code to get away with using
      // just one VBO buffer for rendering, and not have to maintain multiple
      // ones. Therefore cases (1) and (3) will be very slow, and case (2) is
      // fast.

      // Detect which case we are in by using a quick heuristic by examining the
      // strides of the buffers. If all the buffers have identical stride, we
      // assume we have case (2), otherwise we have something more complex.
      var clientStartPointer = {{{ POINTER_MAX }}};
      var bytes = 0; // Total number of bytes taken up by a single vertex.
      var minStride = {{{ POINTER_MAX }}};
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
        // We are in cases (1) or (3): slow path, shuffle the data around into a
        // single interleaved vertex buffer.
        // The immediate-mode glBegin()/glEnd() vertex submission gets
        // automatically generated in appropriate layout, so never need to come
        // down this path if that was used.
#if GL_ASSERTIONS
        warnOnce('Rendering from planar client-side vertex arrays. This is a very slow emulation path! Use interleaved vertex arrays for best performance.');
#endif
        GLImmediate.restrideBuffer ||= _malloc(GL.MAX_TEMP_BUFFER_SIZE);
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
            for (var j = 0; j < count; j++) {
              for (var k = 0; k < attr.sizeBytes; k+=4) { // copy in chunks of 4 bytes, our alignment makes this possible
                var val = {{{ makeGetValue('attr.pointer', 'j*srcStride + k', 'i32') }}};
                {{{ makeSetValue('start + attr.offset', 'bytes*j + k', 'val', 'i32') }}};
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
        if (GLctx.currentArrayBufferBinding) {
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
          warnOnce(`Warning: Rendering from client side vertex arrays where stride (${GLImmediate.stride}) is not a multiple of four! This is not currently supported!`);
        }
#endif
        GLImmediate.vertexCounter = (GLImmediate.stride * count) / 4; // XXX assuming float
      }
    },

    flush(numProvidedIndexes, startIndex = 0, ptr = 0) {
#if ASSERTIONS
      assert(numProvidedIndexes >= 0 || !numProvidedIndexes);
#endif
      var renderer = GLImmediate.getRenderer();

      // Generate index data in a format suitable for GLES 2.0/WebGL
      var numVertices = 4 * GLImmediate.vertexCounter / GLImmediate.stride;
      if (!numVertices) return;
#if ASSERTIONS
      assert(numVertices % 1 == 0, "`numVertices` must be an integer.");
#endif
      var emulatedElementArrayBuffer = false;
      var numIndexes = 0;
      if (numProvidedIndexes) {
        numIndexes = numProvidedIndexes;
        if (!GLctx.currentArrayBufferBinding && GLImmediate.firstVertex > GLImmediate.lastVertex) {
          // Figure out the first and last vertex from the index data
#if ASSERTIONS
          // If we are going to upload array buffer data, we need to find which range to
          // upload based on the indices. If they are in a buffer on the GPU, that is very
          // inconvenient! So if you do not have an array buffer, you should also not have
          // an element array buffer. But best is to use both buffers!
          assert(!GLctx.currentElementArrayBufferBinding);
#endif
          for (var i = 0; i < numProvidedIndexes; i++) {
            var currIndex = {{{ makeGetValue('ptr', 'i*2', 'u16') }}};
            GLImmediate.firstVertex = Math.min(GLImmediate.firstVertex, currIndex);
            GLImmediate.lastVertex = Math.max(GLImmediate.lastVertex, currIndex+1);
          }
        }
        if (!GLctx.currentElementArrayBufferBinding) {
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
        // GLImmediate.firstVertex is the first vertex we want. Quad indexes are
        // in the pattern 0 1 2, 0 2 3, 4 5 6, 4 6 7, so we need to look at
        // index firstVertex * 1.5 to see it.  Then since indexes are 2 bytes
        // each, that means 3
#if ASSERTIONS
        assert(GLImmediate.firstVertex % 4 == 0);
#endif
        ptr = GLImmediate.firstVertex * 3;
        var numQuads = numVertices / 4;
        numIndexes = numQuads * 6; // 0 1 2, 0 2 3 pattern
#if ASSERTIONS
        assert(ptr + (numIndexes << 1) <= GL.MAX_TEMP_BUFFER_SIZE, 'too many immediate mode indexes (b)');
#endif
        GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, GL.currentContext.tempQuadIndexBuffer);
        emulatedElementArrayBuffer = true;
        GLImmediate.mode = GLctx.TRIANGLES;
      }

      renderer.prepare();

      if (numIndexes) {
        GLctx.drawElements(GLImmediate.mode, numIndexes, GLctx.UNSIGNED_SHORT, ptr);
      } else {
        GLctx.drawArrays(GLImmediate.mode, startIndex, numVertices);
      }

      if (emulatedElementArrayBuffer) {
        GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, GL.buffers[GLctx.currentElementArrayBufferBinding] || null);
      }

#if !GL_UNSAFE_OPTS
#if !GL_FFP_ONLY
      renderer.cleanup();
#endif
#endif
    }
  },

  $GLImmediateSetup__deps: ['$GLImmediate', () => 'GLImmediate.matrixLib = ' + read('gl-matrix.js') + ';\n'],
  $GLImmediateSetup: {},

  glBegin__deps: ['$GLImmediateSetup'],
  glBegin: (mode) => {
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

  glEnd: () => {
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

  glVertex2f: (x, y) => {
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

  glVertex3f: (x, y, z) => {
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

  glVertex4f: (x, y, z, w) => {
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
  glVertex2fv: (p) => _glVertex2f({{{ makeGetValue('p', '0', 'float') }}},
                                  {{{ makeGetValue('p', '4', 'float') }}}),

  glVertex3fv__deps: ['glVertex3f'],
  glVertex3fv: (p) => _glVertex3f({{{ makeGetValue('p', '0', 'float') }}},
                                  {{{ makeGetValue('p', '4', 'float') }}},
                                  {{{ makeGetValue('p', '8', 'float') }}}),

  glVertex4fv__deps: ['glVertex4f'],
  glVertex4fv: (p) => _glVertex4f({{{ makeGetValue('p', '0', 'float') }}},
                                  {{{ makeGetValue('p', '4', 'float') }}},
                                  {{{ makeGetValue('p', '8', 'float') }}},
                                  {{{ makeGetValue('p', '12', 'float') }}}),

  glVertex2i: 'glVertex2f',

  glVertex3i: 'glVertex3f',

  glVertex4i: 'glVertex4f',

  glTexCoord2i: (u, v) => {
#if ASSERTIONS
    assert(GLImmediate.mode >= 0); // must be in begin/end
#endif
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = u;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = v;
    GLImmediate.addRendererComponent(GLImmediate.TEXTURE0, 2, GLctx.FLOAT);
  },
  glTexCoord2f: 'glTexCoord2i',

  glTexCoord2fv__deps: ['glTexCoord2i'],
  glTexCoord2fv: (v) =>
    _glTexCoord2i({{{ makeGetValue('v', '0', 'float') }}}, {{{ makeGetValue('v', '4', 'float') }}}),

  glTexCoord4f: () => { throw 'glTexCoord4f: TODO' },

  glColor4f: (r, g, b, a) => {
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
  glColor4ub: (r, g, b, a) => _glColor4f((r&255)/255, (g&255)/255, (b&255)/255, (a&255)/255),

  glColor4us__deps: ['glColor4f'],
  glColor4us: (r, g, b, a) => _glColor4f((r&65535)/65535, (g&65535)/65535, (b&65535)/65535, (a&65535)/65535),

  glColor4ui__deps: ['glColor4f'],
  glColor4ui: (r, g, b, a) => _glColor4f((r>>>0)/4294967295, (g>>>0)/4294967295, (b>>>0)/4294967295, (a>>>0)/4294967295),

  glColor3f__deps: ['glColor4f'],
  glColor3f: (r, g, b) => _glColor4f(r, g, b, 1),

  glColor3d: 'glColor3f',

  glColor3ub__deps: ['glColor4ub'],
  glColor3ub: (r, g, b) => _glColor4ub(r, g, b, 255),

  glColor3us__deps: ['glColor4us'],
  glColor3us: (r, g, b) => _glColor4us(r, g, b, 65535),

  glColor3ui__deps: ['glColor4ui'],
  glColor3ui: (r, g, b) => _glColor4ui(r, g, b, 4294967295),

  glColor3ubv__deps: ['glColor3ub'],
  glColor3ubv: (p) => _glColor3ub({{{ makeGetValue('p', '0', 'i8') }}},
                                  {{{ makeGetValue('p', '1', 'i8') }}},
                                  {{{ makeGetValue('p', '2', 'i8') }}}),

  glColor3usv__deps: ['glColor3us'],
  glColor3usv: (p) => _glColor3us({{{ makeGetValue('p', '0', 'i16') }}},
                                  {{{ makeGetValue('p', '2', 'i16') }}},
                                  {{{ makeGetValue('p', '4', 'i16') }}}),

  glColor3uiv__deps: ['glColor3ui'],
  glColor3uiv: (p) => _glColor3ui({{{ makeGetValue('p', '0', 'i32') }}},
                                  {{{ makeGetValue('p', '4', 'i32') }}},
                                  {{{ makeGetValue('p', '8', 'i32') }}}),

  glColor3fv__deps: ['glColor3f'],
  glColor3fv: (p) => _glColor3f({{{ makeGetValue('p', '0', 'float') }}},
                                {{{ makeGetValue('p', '4', 'float') }}},
                                {{{ makeGetValue('p', '8', 'float') }}}),

  glColor4fv__deps: ['glColor4f'],
  glColor4fv: (p) => _glColor4f({{{ makeGetValue('p', '0', 'float') }}},
                                {{{ makeGetValue('p', '4', 'float') }}},
                                {{{ makeGetValue('p', '8', 'float') }}},
                                {{{ makeGetValue('p', '12', 'float') }}}),

  glColor4ubv__deps: ['glColor4ub'],
  glColor4ubv: (p) => _glColor4ub({{{ makeGetValue('p', '0', 'i8') }}},
                                  {{{ makeGetValue('p', '1', 'i8') }}},
                                  {{{ makeGetValue('p', '2', 'i8') }}},
                                  {{{ makeGetValue('p', '3', 'i8') }}}),

  glFogf: (pname, param) => { // partial support, TODO
    switch (pname) {
      case 0xB63: // GL_FOG_START
        GLEmulation.fogStart = param; break;
      case 0xB64: // GL_FOG_END
        GLEmulation.fogEnd = param; break;
      case 0xB62: // GL_FOG_DENSITY
        GLEmulation.fogDensity = param; break;
      case 0xB65: // GL_FOG_MODE
        switch (param) {
          case 0x801: // GL_EXP2
          case 0x2601: // GL_LINEAR
            if (GLEmulation.fogMode != param) {
              GLImmediate.currentRenderer = null; // Fog mode is part of the FFP shader state, we must re-lookup the renderer to use.
              GLEmulation.fogMode = param;
            }
            break;
          default: // default to GL_EXP
            if (GLEmulation.fogMode != 0x800 /* GL_EXP */) {
              GLImmediate.currentRenderer = null; // Fog mode is part of the FFP shader state, we must re-lookup the renderer to use.
              GLEmulation.fogMode = 0x800 /* GL_EXP */;
            }
            break;
        }
        break;
    }
  },
  glFogi__deps: ['glFogf'],
  glFogi: (pname, param) => {
    return _glFogf(pname, param);
  },
  glFogfv__deps: ['glFogf'],
  glFogfv: (pname, param) => { // partial support, TODO
    switch (pname) {
      case 0xB66: // GL_FOG_COLOR
        GLEmulation.fogColor[0] = {{{ makeGetValue('param', '0', 'float') }}};
        GLEmulation.fogColor[1] = {{{ makeGetValue('param', '4', 'float') }}};
        GLEmulation.fogColor[2] = {{{ makeGetValue('param', '8', 'float') }}};
        GLEmulation.fogColor[3] = {{{ makeGetValue('param', '12', 'float') }}};
        break;
      case 0xB63: // GL_FOG_START
      case 0xB64: // GL_FOG_END
        _glFogf(pname, {{{ makeGetValue('param', '0', 'float') }}}); break;
    }
  },
  glFogiv__deps: ['glFogf'],
  glFogiv: (pname, param) => {
    switch (pname) {
      case 0xB66: // GL_FOG_COLOR
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

  glPointSize: (size) => {
    GLEmulation.pointSize = size;
  },

  glPolygonMode: () => {}, // TODO

  glAlphaFunc: (func, ref) => {
    switch(func) {
      case 0x200: // GL_NEVER
      case 0x201: // GL_LESS
      case 0x202: // GL_EQUAL
      case 0x203: // GL_LEQUAL
      case 0x204: // GL_GREATER
      case 0x205: // GL_NOTEQUAL
      case 0x206: // GL_GEQUAL
      case 0x207: // GL_ALWAYS
        GLEmulation.alphaTestRef = ref;
        if (GLEmulation.alphaTestFunc != func) {
          GLEmulation.alphaTestFunc = func;
          GLImmediate.currentRenderer = null; // alpha test mode is part of the FFP shader state, we must re-lookup the renderer to use.
        }
        break;
      default: // invalid value provided
#if GL_ASSERTIONS
        err(`glAlphaFunc: Invalid alpha comparison function ${ptrToString(func)}!`);
#endif
        break;
    }
  },

  glNormal3f: (x, y, z) => {
#if ASSERTIONS
    assert(GLImmediate.mode >= 0); // must be in begin/end
#endif
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = x;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = y;
    GLImmediate.vertexData[GLImmediate.vertexCounter++] = z;
#if ASSERTIONS
    assert(GLImmediate.vertexCounter << 2 < GL.MAX_TEMP_BUFFER_SIZE);
#endif
    GLImmediate.addRendererComponent(GLImmediate.NORMAL, 3, GLctx.FLOAT);
  },

  glNormal3fv__deps: ['glNormal3f'],
  glNormal3fv: (p) => {
    _glNormal3f({{{ makeGetValue('p', '0', 'float') }}}, {{{ makeGetValue('p', '4', 'float') }}}, {{{ makeGetValue('p', '8', 'float') }}});
  },


  // Additional non-GLES rendering calls

  glDrawRangeElements__deps: ['glDrawElements'],
  glDrawRangeElements: (mode, start, end, count, type, indices) => {
    _glDrawElements(mode, count, type, indices, start, end);
  },

  // ClientState/gl*Pointer

  glEnableClientState: (cap) => {
    var attrib = GLEmulation.getAttributeFromCapability(cap);
    if (attrib === null) {
#if ASSERTIONS
      err(`WARNING: unhandled clientstate: ${cap}`);
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
  glDisableClientState: (cap) => {
    var attrib = GLEmulation.getAttributeFromCapability(cap);
    if (attrib === null) {
#if ASSERTIONS
      err(`WARNING: unhandled clientstate: ${cap}`);
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
  glVertexPointer: (size, type, stride, pointer) => {
    GLImmediate.setClientAttribute(GLImmediate.VERTEX, size, type, stride, pointer);
#if GL_FFP_ONLY
    if (GLctx.currentArrayBufferBinding) {
      GLctx.vertexAttribPointer(GLImmediate.VERTEX, size, type, false, stride, pointer);
    }
#endif
  },
  glTexCoordPointer: (size, type, stride, pointer) => {
    GLImmediate.setClientAttribute(GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture, size, type, stride, pointer);
#if GL_FFP_ONLY
    if (GLctx.currentArrayBufferBinding) {
      var loc = GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture;
      GLctx.vertexAttribPointer(loc, size, type, false, stride, pointer);
    }
#endif
  },
  glNormalPointer: (type, stride, pointer) => {
    GLImmediate.setClientAttribute(GLImmediate.NORMAL, 3, type, stride, pointer);
#if GL_FFP_ONLY
    if (GLctx.currentArrayBufferBinding) {
      GLctx.vertexAttribPointer(GLImmediate.NORMAL, 3, type, true, stride, pointer);
    }
#endif
  },
  glColorPointer: (size, type, stride, pointer) => {
    GLImmediate.setClientAttribute(GLImmediate.COLOR, size, type, stride, pointer);
#if GL_FFP_ONLY
    if (GLctx.currentArrayBufferBinding) {
      GLctx.vertexAttribPointer(GLImmediate.COLOR, size, type, true, stride, pointer);
    }
#endif
  },

  glClientActiveTexture: (texture) => {
    GLImmediate.clientActiveTexture = texture - 0x84C0; // GL_TEXTURE0
  },

  // Replace some functions with immediate-mode aware versions. If there are no
  // client attributes enabled, and we use webgl-friendly modes (no GL_QUADS),
  // then no need for emulation
  glDrawArrays: (mode, first, count) => {
    if (GLImmediate.totalEnabledClientAttributes == 0 && mode <= 6) {
      GLctx.drawArrays(mode, first, count);
      return;
    }
    GLImmediate.prepareClientAttributes(count, false);
    GLImmediate.mode = mode;
    if (!GLctx.currentArrayBufferBinding) {
      GLImmediate.vertexData = {{{ makeHEAPView('F32', 'GLImmediate.vertexPointer', 'GLImmediate.vertexPointer + (first+count)*GLImmediate.stride') }}}; // XXX assuming float
      GLImmediate.firstVertex = first;
      GLImmediate.lastVertex = first + count;
    }
    GLImmediate.flush(null, first);
    GLImmediate.mode = -1;
  },

  // start, end are given if we come from glDrawRangeElements
  glDrawElements: (mode, count, type, indices, start, end) => {
    if (GLImmediate.totalEnabledClientAttributes == 0 && mode <= 6 && GLctx.currentElementArrayBufferBinding) {
      GLctx.drawElements(mode, count, type, indices);
      return;
    }
#if ASSERTIONS
    if (!GLctx.currentElementArrayBufferBinding) {
      assert(type == GLctx.UNSIGNED_SHORT); // We can only emulate buffers of this kind, for now
    }
    warnOnce("DrawElements doesn't actually prepareClientAttributes properly.");
#endif
    GLImmediate.prepareClientAttributes(count, false);
    GLImmediate.mode = mode;
    if (!GLctx.currentArrayBufferBinding) {
      GLImmediate.firstVertex = end ? start : HEAP8.length; // if we don't know the start, set an invalid value and we will calculate it later from the indices
      GLImmediate.lastVertex = end ? end + 1 : 0;
      start = GLImmediate.vertexPointer;
      // TODO(sbc): Combine these two subarray calls back into a single one if
      // we ever fix https://github.com/emscripten-core/emscripten/issues/21250.
      if (end) {
        end = GLImmediate.vertexPointer + (end +1 ) * GLImmediate.stride;
        GLImmediate.vertexData = HEAPF32.subarray({{{ getHeapOffset('start', 'float') }}}, {{{ getHeapOffset('end', 'float') }}});
      } else {
        GLImmediate.vertexData = HEAPF32.subarray({{{ getHeapOffset('start', 'float') }}});
      }
    }
    GLImmediate.flush(count, 0, indices);
    GLImmediate.mode = -1;
  },

  // Vertex array object (VAO) support. TODO: when the WebGL extension is
  // popular, use that and remove this code and GL.vaos
  $emulGlGenVertexArrays__deps: ['$GLEmulation'],
  $emulGlGenVertexArrays: (n, vaos) => {
    for (var i = 0; i < n; i++) {
      var id = GL.getNewId(GLEmulation.vaos);
      GLEmulation.vaos[id] = {
        id,
        arrayBuffer: 0,
        elementArrayBuffer: 0,
        enabledVertexAttribArrays: {},
        vertexAttribPointers: {},
        enabledClientStates: {},
      };
      {{{ makeSetValue('vaos', 'i*4', 'id', 'i32') }}};
    }
  },
  $emulGlDeleteVertexArrays: (n, vaos) => {
    for (var i = 0; i < n; i++) {
      var id = {{{ makeGetValue('vaos', 'i*4', 'i32') }}};
      GLEmulation.vaos[id] = null;
      if (GLEmulation.currentVao && GLEmulation.currentVao.id == id) GLEmulation.currentVao = null;
    }
  },
  $emulGlIsVertexArray: (array) => {
    var vao = GLEmulation.vaos[array];
    if (!vao) return 0;
    return 1;
  },
  $emulGlBindVertexArray__deps: ['glBindBuffer', 'glEnableVertexAttribArray', 'glVertexAttribPointer', 'glEnableClientState'],
  $emulGlBindVertexArray: (vao) => {
    // undo vao-related things, wipe the slate clean, both for vao of 0 or an actual vao
    GLEmulation.currentVao = null; // make sure the commands we run here are not recorded
    GLImmediate.lastRenderer?.cleanup();
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
        _glVertexAttribPointer(...info.vertexAttribPointers[vaa]);
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
  glMatrixMode: (mode) => {
    if (mode == 0x1700 /* GL_MODELVIEW */) {
      GLImmediate.currentMatrix = 0/*m*/;
    } else if (mode == 0x1701 /* GL_PROJECTION */) {
      GLImmediate.currentMatrix = 1/*p*/;
    } else if (mode == 0x1702) { // GL_TEXTURE
      GLImmediate.useTextureMatrix = true;
      GLImmediate.currentMatrix = 2/*t*/ + GLImmediate.TexEnvJIT.getActiveTexture();
    } else {
      throw `Wrong mode ${mode} passed to glMatrixMode`;
    }
  },

  glPushMatrix: () => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixStack[GLImmediate.currentMatrix].push(
        Array.prototype.slice.call(GLImmediate.matrix[GLImmediate.currentMatrix]));
  },

  glPopMatrix: () => {
    if (GLImmediate.matrixStack[GLImmediate.currentMatrix].length == 0) {
      GL.recordError(0x504/*GL_STACK_UNDERFLOW*/);
      return;
    }
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrix[GLImmediate.currentMatrix] = GLImmediate.matrixStack[GLImmediate.currentMatrix].pop();
  },

  glLoadIdentity__deps: ['$GL', '$GLImmediateSetup'],
  glLoadIdentity: () => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.identity(GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  glLoadMatrixd: (matrix) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}}, GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  glLoadMatrixf: (matrix) => {
#if GL_DEBUG
    if (GL.debug) dbg('glLoadMatrixf receiving: ' + Array.prototype.slice.call(HEAPF32.subarray(matrix >> 2, (matrix >> 2) + 16)));
#endif
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}}, GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  glLoadTransposeMatrixd: (matrix) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}}, GLImmediate.matrix[GLImmediate.currentMatrix]);
    GLImmediate.matrixLib.mat4.transpose(GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  glLoadTransposeMatrixf: (matrix) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}}, GLImmediate.matrix[GLImmediate.currentMatrix]);
    GLImmediate.matrixLib.mat4.transpose(GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  glMultMatrixd: (matrix) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix],
        {{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}});
  },

  glMultMatrixf: (matrix) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix],
        {{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}});
  },

  glMultTransposeMatrixd: (matrix) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    var colMajor = GLImmediate.matrixLib.mat4.create();
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F64', 'matrix', 'matrix+' + (16*8)) }}}, colMajor);
    GLImmediate.matrixLib.mat4.transpose(colMajor);
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix], colMajor);
  },

  glMultTransposeMatrixf: (matrix) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    var colMajor = GLImmediate.matrixLib.mat4.create();
    GLImmediate.matrixLib.mat4.set({{{ makeHEAPView('F32', 'matrix', 'matrix+' + (16*4)) }}}, colMajor);
    GLImmediate.matrixLib.mat4.transpose(colMajor);
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix], colMajor);
  },

  glFrustum: (left, right, bottom, top_, nearVal, farVal) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix],
        GLImmediate.matrixLib.mat4.frustum(left, right, bottom, top_, nearVal, farVal));
  },
  glFrustumf: 'glFrustum',

  glOrtho: (left, right, bottom, top_, nearVal, farVal) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix],
        GLImmediate.matrixLib.mat4.ortho(left, right, bottom, top_, nearVal, farVal));
  },
  glOrthof: 'glOrtho',

  glScaled: (x, y, z) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.scale(GLImmediate.matrix[GLImmediate.currentMatrix], [x, y, z]);
  },
  glScalef: 'glScaled',

  glTranslated: (x, y, z) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.translate(GLImmediate.matrix[GLImmediate.currentMatrix], [x, y, z]);
  },
  glTranslatef: 'glTranslated',

  glRotated: (angle, x, y, z) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.rotate(GLImmediate.matrix[GLImmediate.currentMatrix], angle*Math.PI/180, [x, y, z]);
  },
  glRotatef: 'glRotated',

  glDrawBuffer: () => { throw 'glDrawBuffer: TODO' },
#if MAX_WEBGL_VERSION < 2
  glReadBuffer: () => { throw 'glReadBuffer: TODO' },
#endif

  glClipPlane: (pname, param) => {
    if ((pname >= 0x3000) && (pname < 0x3006)  /* GL_CLIP_PLANE0 to GL_CLIP_PLANE5 */) {
      var clipPlaneId = pname - 0x3000;

      GLEmulation.clipPlaneEquation[clipPlaneId][0] = {{{ makeGetValue('param', '0', 'double') }}};
      GLEmulation.clipPlaneEquation[clipPlaneId][1] = {{{ makeGetValue('param', '8', 'double') }}};
      GLEmulation.clipPlaneEquation[clipPlaneId][2] = {{{ makeGetValue('param', '16', 'double') }}};
      GLEmulation.clipPlaneEquation[clipPlaneId][3] = {{{ makeGetValue('param', '24', 'double') }}};

      // apply inverse transposed current modelview matrix when setting clip plane
      var tmpMV = GLImmediate.matrixLib.mat4.create(GLImmediate.matrix[0]);
      GLImmediate.matrixLib.mat4.inverse(tmpMV);
      GLImmediate.matrixLib.mat4.transpose(tmpMV);
      GLImmediate.matrixLib.mat4.multiplyVec4(tmpMV, GLEmulation.clipPlaneEquation[clipPlaneId]);
    }
  },

  glLightfv: (light, pname, param) => {
    if ((light >= 0x4000) && (light < 0x4008)  /* GL_LIGHT0 to GL_LIGHT7 */) {
      var lightId = light - 0x4000;

      if (pname == 0x1200) { // GL_AMBIENT
        GLEmulation.lightAmbient[lightId][0] = {{{ makeGetValue('param', '0', 'float') }}};
        GLEmulation.lightAmbient[lightId][1] = {{{ makeGetValue('param', '4', 'float') }}};
        GLEmulation.lightAmbient[lightId][2] = {{{ makeGetValue('param', '8', 'float') }}};
        GLEmulation.lightAmbient[lightId][3] = {{{ makeGetValue('param', '12', 'float') }}};
      } else if (pname == 0x1201) { // GL_DIFFUSE
        GLEmulation.lightDiffuse[lightId][0] = {{{ makeGetValue('param', '0', 'float') }}};
        GLEmulation.lightDiffuse[lightId][1] = {{{ makeGetValue('param', '4', 'float') }}};
        GLEmulation.lightDiffuse[lightId][2] = {{{ makeGetValue('param', '8', 'float') }}};
        GLEmulation.lightDiffuse[lightId][3] = {{{ makeGetValue('param', '12', 'float') }}};
      } else if (pname == 0x1202) { // GL_SPECULAR
        GLEmulation.lightSpecular[lightId][0] = {{{ makeGetValue('param', '0', 'float') }}};
        GLEmulation.lightSpecular[lightId][1] = {{{ makeGetValue('param', '4', 'float') }}};
        GLEmulation.lightSpecular[lightId][2] = {{{ makeGetValue('param', '8', 'float') }}};
        GLEmulation.lightSpecular[lightId][3] = {{{ makeGetValue('param', '12', 'float') }}};
      } else if (pname == 0x1203) { // GL_POSITION
        GLEmulation.lightPosition[lightId][0] = {{{ makeGetValue('param', '0', 'float') }}};
        GLEmulation.lightPosition[lightId][1] = {{{ makeGetValue('param', '4', 'float') }}};
        GLEmulation.lightPosition[lightId][2] = {{{ makeGetValue('param', '8', 'float') }}};
        GLEmulation.lightPosition[lightId][3] = {{{ makeGetValue('param', '12', 'float') }}};

        // multiply position with current modelviewmatrix
        GLImmediate.matrixLib.mat4.multiplyVec4(GLImmediate.matrix[0], GLEmulation.lightPosition[lightId]);
      } else {
        throw 'glLightfv: TODO: ' + pname;
      }
    }
  },

  glLightModelf: (pname, param) => {
    if (pname == 0x0B52) { // GL_LIGHT_MODEL_TWO_SIDE
      GLEmulation.lightModelTwoSide = (param != 0) ? true : false;
    } else {
      throw 'glLightModelf: TODO: ' + pname;
    }
  },

  glLightModelfv: (pname, param) => { // TODO: GL_LIGHT_MODEL_LOCAL_VIEWER
    if (pname == 0x0B53) { // GL_LIGHT_MODEL_AMBIENT
      GLEmulation.lightModelAmbient[0] = {{{ makeGetValue('param', '0', 'float') }}};
      GLEmulation.lightModelAmbient[1] = {{{ makeGetValue('param', '4', 'float') }}};
      GLEmulation.lightModelAmbient[2] = {{{ makeGetValue('param', '8', 'float') }}};
      GLEmulation.lightModelAmbient[3] = {{{ makeGetValue('param', '12', 'float') }}};
    } else {
      throw 'glLightModelfv: TODO: ' + pname;
    }
  },

  glMaterialfv: (face, pname, param) => {
    if ((face != 0x0404) && (face != 0x0408)) { throw 'glMaterialfv: TODO' + face; } // only GL_FRONT and GL_FRONT_AND_BACK supported

    if (pname == 0x1200) { // GL_AMBIENT
      GLEmulation.materialAmbient[0] = {{{ makeGetValue('param', '0', 'float') }}};
      GLEmulation.materialAmbient[1] = {{{ makeGetValue('param', '4', 'float') }}};
      GLEmulation.materialAmbient[2] = {{{ makeGetValue('param', '8', 'float') }}};
      GLEmulation.materialAmbient[3] = {{{ makeGetValue('param', '12', 'float') }}};
    } else if (pname == 0x1201) { // GL_DIFFUSE
      GLEmulation.materialDiffuse[0] = {{{ makeGetValue('param', '0', 'float') }}};
      GLEmulation.materialDiffuse[1] = {{{ makeGetValue('param', '4', 'float') }}};
      GLEmulation.materialDiffuse[2] = {{{ makeGetValue('param', '8', 'float') }}};
      GLEmulation.materialDiffuse[3] = {{{ makeGetValue('param', '12', 'float') }}};
    } else if (pname == 0x1202) { // GL_SPECULAR
      GLEmulation.materialSpecular[0] = {{{ makeGetValue('param', '0', 'float') }}};
      GLEmulation.materialSpecular[1] = {{{ makeGetValue('param', '4', 'float') }}};
      GLEmulation.materialSpecular[2] = {{{ makeGetValue('param', '8', 'float') }}};
      GLEmulation.materialSpecular[3] = {{{ makeGetValue('param', '12', 'float') }}};
    } else if (pname == 0x1601) { // GL_SHININESS
      GLEmulation.materialShininess[0] = {{{ makeGetValue('param', '0', 'float') }}};
    } else {
      throw 'glMaterialfv: TODO: ' + pname;
    }
  },

  glTexGeni: (coord, pname, param) => { throw 'glTexGeni: TODO' },
  glTexGenfv: (coord, pname, param) => { throw 'glTexGenfv: TODO' },
  glTexEnvi: (target, pname, params) => warnOnce('glTexEnvi: TODO'),
  glTexEnvf: (target, pname, params) => warnOnce('glTexEnvf: TODO'),
  glTexEnvfv: (target, pname, params) => warnOnce('glTexEnvfv: TODO'),

  glGetTexEnviv: (target, pname, param) => { throw 'GL emulation not initialized!'; },
  glGetTexEnvfv: (target, pname, param) => { throw 'GL emulation not initialized!'; },

  glTexImage1D: (target, level, internalformat, width, border, format, type, data) => { throw 'glTexImage1D: TODO' },
  glTexCoord3f: (target, level, internalformat, width, border, format, type, data) => { throw 'glTexCoord3f: TODO' },
  glGetTexLevelParameteriv: (target, level, pname, params) => { throw 'glGetTexLevelParameteriv: TODO' },

  glShadeModel: () => warnOnce('TODO: glShadeModel'),

  // Open GLES1.1 compatibility

  glGenFramebuffersOES: 'glGenFramebuffers',
  glGenRenderbuffersOES: 'glGenRenderbuffers',
  glBindFramebufferOES: 'glBindFramebuffer',
  glBindRenderbufferOES: 'glBindRenderbuffer',
  glGetRenderbufferParameterivOES: 'glGetRenderbufferParameteriv',
  glFramebufferRenderbufferOES: 'glFramebufferRenderbuffer',
  glRenderbufferStorageOES: 'glRenderbufferStorage',
  glCheckFramebufferStatusOES: 'glCheckFramebufferStatus',
  glDeleteFramebuffersOES: 'glDeleteFramebuffers',
  glDeleteRenderbuffersOES: 'glDeleteRenderbuffers',
  glFramebufferTexture2DOES: 'glFramebufferTexture2D',

  // GLU

  gluPerspective: (fov, aspect, near, far) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrix[GLImmediate.currentMatrix] =
      GLImmediate.matrixLib.mat4.perspective(fov, aspect, near, far,
                                               GLImmediate.matrix[GLImmediate.currentMatrix]);
  },

  gluLookAt: (ex, ey, ez, cx, cy, cz, ux, uy, uz) => {
    GLImmediate.matricesModified = true;
    GLImmediate.matrixVersion[GLImmediate.currentMatrix] = (GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1)|0;
    GLImmediate.matrixLib.mat4.lookAt(GLImmediate.matrix[GLImmediate.currentMatrix], [ex, ey, ez],
        [cx, cy, cz], [ux, uy, uz]);
  },

  gluProject: (objX, objY, objZ, model, proj, view, winX, winY, winZ) => {
    // The algorithm for this functions comes from Mesa

    var inVec = new Float32Array(4);
    var outVec = new Float32Array(4);
    GLImmediate.matrixLib.mat4.multiplyVec4({{{ makeHEAPView('F64', 'model', 'model+' + (16*8)) }}},
        [objX, objY, objZ, 1.0], outVec);
    GLImmediate.matrixLib.mat4.multiplyVec4({{{ makeHEAPView('F64', 'proj', 'proj+' + (16*8)) }}},
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

  gluUnProject: (winX, winY, winZ, model, proj, view, objX, objY, objZ) => {
    var result = GLImmediate.matrixLib.vec3.unproject([winX, winY, winZ],
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
  gluOrtho2D: (left, right, bottom, top) => _glOrtho(left, right, bottom, top, -1, 1),
};

// Legacy GL emulation
if (LEGACY_GL_EMULATION) {
  extraLibraryFuncs.push('$GLEmulation');
}

recordGLProcAddressGet(LibraryGLEmulation);

addToLibrary(LibraryGLEmulation);

assert(!(FULL_ES2 && LEGACY_GL_EMULATION), 'cannot emulate both ES2 and legacy GL');
assert(!(FULL_ES3 && LEGACY_GL_EMULATION), 'cannot emulate both ES3 and legacy GL');
