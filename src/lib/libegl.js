/**
 * @license
 * Copyright 2012 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
 * The EGL implementation supports only one EGLNativeDisplayType, the
 * EGL_DEFAULT_DISPLAY.  This native display type returns the only supported
 * EGLDisplay handle with the magic value 62000. There is only a single
 * EGLConfig configuration supported, that has the magic value 62002.  The
 * implementation only allows a single EGLContext to be created, that has the
 * magic value of 62004. (multiple creations silently return this same context)
 * The implementation only creates a single EGLSurface, a handle with the magic
 * value of 62006. (multiple creations silently return the same surface)
 */

var LibraryEGL = {
  ...defineInternalConsts({
    // Magic ID for Emscripten 'default display'
    $eglDefaultDisplay: 62000,
    // Magic ID for the only EGLConfig supported by Emscripten
    $eglDefaultConfig: 62002,
    // Magic ID for Emscripten EGLContext
    $eglDefaultContext: 62004,
  }),

  $EGL__deps: ['$Browser'],
  $EGL: {
    // This variable tracks the success status of the most recently invoked EGL function call.
    errorCode: cDefs.EGL_SUCCESS,
    defaultDisplayInitialized: false,
    currentContext: cDefs.EGL_NO_CONTEXT,
    currentReadSurface: cDefs.EGL_NO_SURFACE,
    currentDrawSurface: cDefs.EGL_NO_SURFACE,

    contextAttributes: {
      alpha:     false,
      depth:     false,
      stencil:   false,
      antialias: false
    },

    stringCache: {},

    setErrorCode(code) {
      EGL.errorCode = code;
    },

    chooseConfig(display, attribList, config, config_size, numConfigs) {
      if (display != eglDefaultDisplay) {
        EGL.setErrorCode(EGL_BAD_DISPLAY);
        return EGL_FALSE;
      }

      if (attribList) {
        // read attribList if it is non-null
        for (;;) {
          var param = {{{ makeGetValue('attribList', '0', 'i32') }}};
          if (param == EGL_ALPHA_SIZE) {
            var alphaSize = {{{ makeGetValue('attribList', '4', 'i32') }}};
            EGL.contextAttributes.alpha = (alphaSize > 0);
          } else if (param == EGL_DEPTH_SIZE) {
            var depthSize = {{{ makeGetValue('attribList', '4', 'i32') }}};
            EGL.contextAttributes.depth = (depthSize > 0);
          } else if (param == EGL_STENCIL_SIZE) {
            var stencilSize = {{{ makeGetValue('attribList', '4', 'i32') }}};
            EGL.contextAttributes.stencil = (stencilSize > 0);
          } else if (param == EGL_SAMPLES) {
            var samples = {{{ makeGetValue('attribList', '4', 'i32') }}};
            EGL.contextAttributes.antialias = (samples > 0);
          } else if (param == EGL_SAMPLE_BUFFERS) {
            var samples = {{{ makeGetValue('attribList', '4', 'i32') }}};
            EGL.contextAttributes.antialias = (samples == 1);
          } else if (param == EGL_CONTEXT_PRIORITY_LEVEL_IMG) {
            var requestedPriority = {{{ makeGetValue('attribList', '4', 'i32') }}};
            EGL.contextAttributes.lowLatency = (requestedPriority != EGL_CONTEXT_PRIORITY_LOW_IMG);
          } else if (param == EGL_NONE) {
              break;
          }
          attribList += 8;
        }
      }

      if ((!config || !config_size) && !numConfigs) {
        EGL.setErrorCode(EGL_BAD_PARAMETER);
        return EGL_FALSE;
      }
      if (numConfigs) {
        {{{ makeSetValue('numConfigs', '0', '1', 'i32') }}}; // Total number of supported configs: 1.
      }
      if (config && config_size > 0) {
        {{{ makeSetValue('config', '0', 'eglDefaultConfig' /* Magic ID for the only EGLConfig supported by Emscripten */, '*') }}};
      }

      EGL.setErrorCode(EGL_SUCCESS);
      return EGL_TRUE;
    },
  },

  // EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id);
  eglGetDisplay__proxy: 'sync',
  eglGetDisplay: (nativeDisplayType) => {
    EGL.setErrorCode(EGL_SUCCESS);
    // Emscripten EGL implementation "emulates" X11, and eglGetDisplay is
    // expected to accept/receive a pointer to an X11 Display object (or
    // EGL_DEFAULT_DISPLAY).
    if (nativeDisplayType != EGL_DEFAULT_DISPLAY && nativeDisplayType != 1 /* see library_xlib.js */) {
      return EGL_NO_DISPLAY;
    }
    return eglDefaultDisplay;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
  eglInitialize__proxy: 'sync',
  eglInitialize: (display, majorVersion, minorVersion) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_FALSE;
    }
    if (majorVersion) {
      {{{ makeSetValue('majorVersion', '0', '1', 'i32') }}}; // Advertise EGL Major version: '1'
    }
    if (minorVersion) {
      {{{ makeSetValue('minorVersion', '0', '4', 'i32') }}}; // Advertise EGL Minor version: '4'
    }
    EGL.defaultDisplayInitialized = true;
    EGL.setErrorCode(EGL_SUCCESS);
    return EGL_TRUE;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy);
  eglTerminate__proxy: 'sync',
  eglTerminate: (display) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_FALSE;
    }
    EGL.currentContext = EGL_NO_CONTEXT;
    EGL.currentReadSurface = EGL_NO_SURFACE;
    EGL.currentDrawSurface = EGL_NO_SURFACE;
    EGL.defaultDisplayInitialized = false;
    EGL.setErrorCode(EGL_SUCCESS);
    return EGL_TRUE;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
  eglGetConfigs__proxy: 'sync',
  eglGetConfigs: (display, configs, config_size, numConfigs) =>
    EGL.chooseConfig(display, 0, configs, config_size, numConfigs),

  // EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
  eglChooseConfig__proxy: 'sync',
  eglChooseConfig: (display, attrib_list, configs, config_size, numConfigs) =>
    EGL.chooseConfig(display, attrib_list, configs, config_size, numConfigs),

  // EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
  eglGetConfigAttrib__proxy: 'sync',
  eglGetConfigAttrib: (display, config, attribute, value) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_FALSE;
    }
    if (config != eglDefaultConfig) {
      EGL.setErrorCode(EGL_BAD_CONFIG);
      return EGL_FALSE;
    }
    if (!value) {
      EGL.setErrorCode(EGL_BAD_PARAMETER);
      return EGL_FALSE;
    }
    EGL.setErrorCode(EGL_SUCCESS);
    switch (attribute) {
    case EGL_BUFFER_SIZE:
      {{{ makeSetValue('value', '0', 'EGL.contextAttributes.alpha ? 32 : 24' /* 8 bits for each R,G,B. 8 bits for alpha if enabled*/, 'i32') }}};
      return EGL_TRUE;
    case EGL_ALPHA_SIZE:
      {{{ makeSetValue('value', '0', 'EGL.contextAttributes.alpha ? 8 : 0' /* 8 bits for alpha channel if enabled. */, 'i32') }}};
      return EGL_TRUE;
    case EGL_BLUE_SIZE:
      {{{ makeSetValue('value', '0', '8' /* 8 bits for blue channel. */, 'i32') }}};
      return EGL_TRUE;
    case EGL_GREEN_SIZE:
      {{{ makeSetValue('value', '0', '8' /* 8 bits for green channel. */, 'i32') }}};
      return EGL_TRUE;
    case EGL_RED_SIZE:
      {{{ makeSetValue('value', '0', '8' /* 8 bits for red channel. */, 'i32') }}};
      return EGL_TRUE;
    case EGL_DEPTH_SIZE:
      {{{ makeSetValue('value', '0', 'EGL.contextAttributes.depth ? 24 : 0' /* 24 bits for depth buffer if enabled. */, 'i32') }}};
      return EGL_TRUE;
    case EGL_STENCIL_SIZE:
      {{{ makeSetValue('value', '0', 'EGL.contextAttributes.stencil ? 8 : 0' /* 8 bits for stencil buffer if enabled. */, 'i32') }}};
      return EGL_TRUE;
    case EGL_CONFIG_CAVEAT:
      // We can return here one of EGL_NONE (0x3038), EGL_SLOW_CONFIG (0x3050) or EGL_NON_CONFORMANT_CONFIG (0x3051).
      {{{ makeSetValue('value', '0', cDefs.EGL_NONE, 'i32') }}};
      return EGL_TRUE;
    case EGL_CONFIG_ID:
      {{{ makeSetValue('value', '0', 'eglDefaultConfig', 'i32') }}};
      return EGL_TRUE;
    case EGL_LEVEL:
      {{{ makeSetValue('value', '0', '0' /* Z order/depth layer for this level. Not applicable for Emscripten. */, 'i32') }}};
      return EGL_TRUE;
    case EGL_MAX_PBUFFER_HEIGHT:
      {{{ makeSetValue('value', '0', '4096', 'i32') }}};
      return EGL_TRUE;
    case EGL_MAX_PBUFFER_PIXELS:
      {{{ makeSetValue('value', '0', '16777216' /* 4096 * 4096 */, 'i32') }}};
      return EGL_TRUE;
    case EGL_MAX_PBUFFER_WIDTH:
      {{{ makeSetValue('value', '0', '4096', 'i32') }}};
      return EGL_TRUE;
    case EGL_NATIVE_RENDERABLE:
      {{{ makeSetValue('value', '0', cDefs.EGL_FALSE, 'i32') }}}; // This config does not allow co-rendering with other 'native' rendering APIs.
      return EGL_TRUE;
    case EGL_NATIVE_VISUAL_ID:
      {{{ makeSetValue('value', '0', '0' /* N/A for Emscripten. */, 'i32') }}};
      return EGL_TRUE;
    case EGL_NATIVE_VISUAL_TYPE:
      {{{ makeSetValue('value', '0', cDefs.EGL_NONE, 'i32') }}};
      return EGL_TRUE;
    case EGL_SAMPLES:
      {{{ makeSetValue('value', '0', 'EGL.contextAttributes.antialias ? 4 : 0' /* 2x2 Multisampling */, 'i32') }}};
      return EGL_TRUE;
    case EGL_SAMPLE_BUFFERS:
      {{{ makeSetValue('value', '0', 'EGL.contextAttributes.antialias ? 1 : 0' /* Multisampling enabled */, 'i32') }}};
      return EGL_TRUE;
    case EGL_SURFACE_TYPE:
      {{{ makeSetValue('value', '0', cDefs.EGL_WINDOW_BIT, 'i32') }}};
      return EGL_TRUE;
    case EGL_TRANSPARENT_TYPE:
      // If this returns EGL_TRANSPARENT_RGB (0x3052), transparency is used through color-keying. No such thing applies to Emscripten canvas.
      {{{ makeSetValue('value', '0', cDefs.EGL_NONE, 'i32') }}};
      return EGL_TRUE;
    case EGL_TRANSPARENT_BLUE_VALUE:
    case EGL_TRANSPARENT_GREEN_VALUE:
    case EGL_TRANSPARENT_RED_VALUE:
      // "If EGL_TRANSPARENT_TYPE is EGL_NONE, then the values for EGL_TRANSPARENT_RED_VALUE, EGL_TRANSPARENT_GREEN_VALUE, and EGL_TRANSPARENT_BLUE_VALUE are undefined."
      {{{ makeSetValue('value', '0', cDefs.EGL_DONT_CARE, 'i32') }}}; // Report a "does not apply" value.
      return EGL_TRUE;
    case EGL_BIND_TO_TEXTURE_RGB:
    case EGL_BIND_TO_TEXTURE_RGBA:
      {{{ makeSetValue('value', '0', cDefs.EGL_FALSE, 'i32') }}}; // Only pbuffers would be bindable, but these are not supported.
      return EGL_TRUE;
    case EGL_MIN_SWAP_INTERVAL:
      {{{ makeSetValue('value', '0', '0', 'i32') }}};
      return EGL_TRUE;
    case EGL_MAX_SWAP_INTERVAL:
      {{{ makeSetValue('value', '0', '1' /* TODO: Currently this is not strictly true, since user can specify custom presentation interval in JS requestAnimationFrame/emscripten_set_main_loop. */, 'i32') }}};
      return EGL_TRUE;
    case EGL_LUMINANCE_SIZE:
    case EGL_ALPHA_MASK_SIZE:
      {{{ makeSetValue('value', '0', '0' /* N/A in this config. */, 'i32') }}};
      return EGL_TRUE;
    case EGL_COLOR_BUFFER_TYPE:
      // EGL has two types of buffers: EGL_RGB_BUFFER and EGL_LUMINANCE_BUFFER.
      {{{ makeSetValue('value', '0', cDefs.EGL_RGB_BUFFER, 'i32') }}};
      return EGL_TRUE;
    case EGL_RENDERABLE_TYPE:
      // A bit combination of EGL_OPENGL_ES_BIT,EGL_OPENVG_BIT,EGL_OPENGL_ES2_BIT and EGL_OPENGL_BIT.
      {{{ makeSetValue('value', '0', cDefs.EGL_OPENGL_ES2_BIT, 'i32') }}};
      return EGL_TRUE;
    case EGL_CONFORMANT:
      // "EGL_CONFORMANT is a mask indicating if a client API context created with respect to the corresponding EGLConfig will pass the required conformance tests for that API."
      {{{ makeSetValue('value', '0', '0' /* EGL_OPENGL_ES2_BIT */, 'i32') }}};
      return EGL_TRUE;
    default:
      EGL.setErrorCode(EGL_BAD_ATTRIBUTE);
      return EGL_FALSE;
    }
  },

  // EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
  eglCreateWindowSurface__proxy: 'sync',
  eglCreateWindowSurface: (display, config, win, attrib_list) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_NO_SURFACE;
    }
    if (config != eglDefaultConfig) {
      EGL.setErrorCode(EGL_BAD_CONFIG);
      return EGL_NO_SURFACE;
    }
    // TODO: Examine attrib_list! Parameters that can be present there are:
    // - EGL_RENDER_BUFFER (must be EGL_BACK_BUFFER)
    // - EGL_VG_COLORSPACE (can't be set)
    // - EGL_VG_ALPHA_FORMAT (can't be set)
    EGL.setErrorCode(EGL_SUCCESS);
    return 62006; /* Magic ID for Emscripten 'default surface' */
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay display, EGLSurface surface);
  eglDestroySurface__proxy: 'sync',
  eglDestroySurface: (display, surface) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_FALSE;
    }
    if (surface != 62006 /* Magic ID for the only EGLSurface supported by Emscripten */) {
      EGL.setErrorCode(EGL_BAD_SURFACE);
      return EGL_TRUE;
    }
    if (EGL.currentReadSurface == surface) {
      EGL.currentReadSurface = EGL_NO_SURFACE;
    }
    if (EGL.currentDrawSurface == surface) {
      EGL.currentDrawSurface = EGL_NO_SURFACE;
    }
    EGL.setErrorCode(EGL_SUCCESS);
    return EGL_TRUE; /* Magic ID for Emscripten 'default surface' */
  },


  // EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
  eglCreateContext__proxy: 'sync',
  eglCreateContext: (display, config, hmm, contextAttribs) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_NO_CONTEXT;
    }

    // EGL 1.4 spec says default EGL_CONTEXT_CLIENT_VERSION is GLES1, but this is not supported by Emscripten.
    // So user must pass EGL_CONTEXT_CLIENT_VERSION == 2 to initialize EGL.
    var glesContextVersion = 1;
    for (;;) {
      var param = {{{ makeGetValue('contextAttribs', '0', 'i32') }}};
      if (param == EGL_CONTEXT_CLIENT_VERSION) {
        glesContextVersion = {{{ makeGetValue('contextAttribs', '4', 'i32') }}};
      } else if (param == EGL_NONE) {
        break;
      } else {
        /* EGL1.4 specifies only EGL_CONTEXT_CLIENT_VERSION as supported attribute */
        EGL.setErrorCode(EGL_BAD_ATTRIBUTE);
        return EGL_NO_CONTEXT;
      }
      contextAttribs += 8;
    }
#if MAX_WEBGL_VERSION >= 2
    if (glesContextVersion < 2 || glesContextVersion > 3) {
#else
    if (glesContextVersion != 2) {
#endif
#if GL_ASSERTIONS
      if (glesContextVersion == 3) {
        err('When initializing GLES3/WebGL2 via EGL, one must build with -sMAX_WEBGL_VERSION=2!');
      } else {
        err(`When initializing GLES2/WebGL1 via EGL, one must pass EGL_CONTEXT_CLIENT_VERSION = 2 to GL context attributes! GLES version ${glesContextVersion} is not supported!`);
      }
#endif
      EGL.setErrorCode(EGL_BAD_CONFIG);
      return EGL_NO_CONTEXT;
    }

    EGL.contextAttributes.majorVersion = glesContextVersion - 1; // WebGL 1 is GLES 2, WebGL2 is GLES3
    EGL.contextAttributes.minorVersion = 0;

    EGL.context = GL.createContext(Browser.getCanvas(), EGL.contextAttributes);

    if (EGL.context != 0) {
      EGL.setErrorCode(EGL_SUCCESS);

      // Run callbacks so that GL emulation works
      GL.makeContextCurrent(EGL.context);
      Browser.useWebGL = true;
      Browser.moduleContextCreatedCallbacks.forEach((callback) => callback());

      // Note: This function only creates a context, but it shall not make it active.
      GL.makeContextCurrent(null);
      return eglDefaultContext;
    } else {
      EGL.setErrorCode(EGL_BAD_MATCH); // By the EGL 1.4 spec, an implementation that does not support GLES2 (WebGL in this case), this error code is set.
      return EGL_NO_CONTEXT;
    }
  },


  // EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext context);
  eglDestroyContext__proxy: 'sync',
  eglDestroyContext: (display, context) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_FALSE;
    }
    if (context != eglDefaultContext) {
      EGL.setErrorCode(EGL_BAD_CONTEXT);
      return EGL_FALSE;
    }

    GL.deleteContext(EGL.context);
    EGL.setErrorCode(EGL_SUCCESS);
    if (EGL.currentContext == context) {
      EGL.currentContext = EGL_NO_CONTEXT;
    }
    return EGL_TRUE;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
  eglQuerySurface__proxy: 'sync',
  eglQuerySurface: (display, surface, attribute, value) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_FALSE;
    }
    if (surface != 62006 /* Magic ID for Emscripten 'default surface' */) {
      EGL.setErrorCode(EGL_BAD_SURFACE);
      return EGL_FALSE;
    }
    if (!value) {
      EGL.setErrorCode(EGL_BAD_PARAMETER);
      return EGL_FALSE;
    }
    EGL.setErrorCode(EGL_SUCCESS);
    switch (attribute) {
    case EGL_CONFIG_ID:
      {{{ makeSetValue('value', '0', 'eglDefaultConfig', 'i32') }}};
      return EGL_TRUE;
    case EGL_LARGEST_PBUFFER:
      // Odd EGL API: If surface is not a pbuffer surface, 'value' should not be written to. It's not specified as an error, so true should(?) be returned.
      // Existing Android implementation seems to do so at least.
      return EGL_TRUE;
    case EGL_WIDTH:
      {{{ makeSetValue('value', '0', 'Browser.getCanvas().width', 'i32') }}};
      return EGL_TRUE;
    case EGL_HEIGHT:
      {{{ makeSetValue('value', '0', 'Browser.getCanvas().height', 'i32') }}};
      return EGL_TRUE;
    case EGL_HORIZONTAL_RESOLUTION:
      {{{ makeSetValue('value', '0', cDefs.EGL_UNKNOWN, 'i32') }}};
      return EGL_TRUE;
    case EGL_VERTICAL_RESOLUTION:
      {{{ makeSetValue('value', '0', cDefs.EGL_UNKNOWN, 'i32') }}};
      return EGL_TRUE;
    case EGL_PIXEL_ASPECT_RATIO:
      {{{ makeSetValue('value', '0', cDefs.EGL_UNKNOWN, 'i32') }}};
      return EGL_TRUE;
    case EGL_RENDER_BUFFER:
      // The main surface is bound to the visible canvas window - it's always backbuffered.
      // Alternative to EGL_BACK_BUFFER would be EGL_SINGLE_BUFFER.
      {{{ makeSetValue('value', '0', cDefs.EGL_BACK_BUFFER, 'i32') }}};
      return EGL_TRUE;
    case EGL_MULTISAMPLE_RESOLVE:
      {{{ makeSetValue('value', '0', cDefs.EGL_MULTISAMPLE_RESOLVE_DEFAULT, 'i32') }}};
      return EGL_TRUE;
    case EGL_SWAP_BEHAVIOR:
      // The two possibilities are EGL_BUFFER_PRESERVED and EGL_BUFFER_DESTROYED. Slightly unsure which is the
      // case for browser environment, but advertise the 'weaker' behavior to be sure.
      {{{ makeSetValue('value', '0', cDefs.EGL_BUFFER_DESTROYED, 'i32') }}};
      return EGL_TRUE;
    case EGL_TEXTURE_FORMAT:
    case EGL_TEXTURE_TARGET:
    case EGL_MIPMAP_TEXTURE:
    case EGL_MIPMAP_LEVEL:
      // This is a window surface, not a pbuffer surface. Spec:
      // "Querying EGL_TEXTURE_FORMAT, EGL_TEXTURE_TARGET, EGL_MIPMAP_TEXTURE, or EGL_MIPMAP_LEVEL for a non-pbuffer surface is not an error, but value is not modified."
      // So pass-through.
      return EGL_TRUE;
    default:
      EGL.setErrorCode(EGL_BAD_ATTRIBUTE);
      return EGL_FALSE;
    }
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
  eglQueryContext__proxy: 'sync',
  eglQueryContext: (display, context, attribute, value) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_FALSE;
    }
    //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy.
    if (context != eglDefaultContext) {
      EGL.setErrorCode(EGL_BAD_CONTEXT);
      return EGL_FALSE;
    }
    if (!value) {
      EGL.setErrorCode(EGL_BAD_PARAMETER);
      return EGL_FALSE;
    }

    EGL.setErrorCode(EGL_SUCCESS);
    switch (attribute) {
      case EGL_CONFIG_ID:
        {{{ makeSetValue('value', '0', 'eglDefaultConfig', 'i32') }}};
        return EGL_TRUE;
      case EGL_CONTEXT_CLIENT_TYPE:
        {{{ makeSetValue('value', '0', cDefs.EGL_OPENGL_ES_API, 'i32') }}};
        return EGL_TRUE;
      case EGL_CONTEXT_CLIENT_VERSION:
        {{{ makeSetValue('value', '0', 'EGL.contextAttributes.majorVersion + 1', 'i32') }}};
        return EGL_TRUE;
      case EGL_RENDER_BUFFER:
        // The context is bound to the visible canvas window - it's always backbuffered.
        // Alternative to EGL_BACK_BUFFER would be EGL_SINGLE_BUFFER.
        {{{ makeSetValue('value', '0', cDefs.EGL_BACK_BUFFER, 'i32') }}};
        return EGL_TRUE;
      default:
        EGL.setErrorCode(EGL_BAD_ATTRIBUTE);
        return EGL_FALSE;
    }
  },

  // EGLAPI EGLint EGLAPIENTRY eglGetError(void);
  eglGetError__proxy: 'sync',
  eglGetError: () => EGL.errorCode,

  // EGLAPI const char * EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name);
  // The allocated strings are cached and never freed.
  eglQueryString__noleakcheck: true,
  eglQueryString__proxy: 'sync',
  eglQueryString: (display, name) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return 0;
    }
    //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy.
    EGL.setErrorCode(EGL_SUCCESS);
    if (EGL.stringCache[name]) return EGL.stringCache[name];
    var ret;
    switch (name) {
      case EGL_VENDOR: ret = stringToNewUTF8('Emscripten'); break;
      case EGL_VERSION: ret = stringToNewUTF8('1.4 Emscripten EGL'); break;
      case EGL_EXTENSIONS: ret = stringToNewUTF8(''); break; // Currently not supporting any EGL extensions.
      case EGL_CLIENT_APIS: ret = stringToNewUTF8('OpenGL_ES'); break;
      default:
        EGL.setErrorCode(EGL_BAD_PARAMETER);
        return 0;
    }
    EGL.stringCache[name] = ret;
    return ret;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api);
  eglBindAPI__proxy: 'sync',
  eglBindAPI: (api) => {
    if (api == EGL_OPENGL_ES_API) {
      EGL.setErrorCode(EGL_SUCCESS);
      return EGL_TRUE;
    }
    // if (api == 0x30A1 /* EGL_OPENVG_API */ || api == 0x30A2 /* EGL_OPENGL_API */) {
    EGL.setErrorCode(EGL_BAD_PARAMETER);
    return EGL_FALSE;
  },

  // EGLAPI EGLenum EGLAPIENTRY eglQueryAPI(void);
  eglQueryAPI__proxy: 'sync',
  eglQueryAPI: () => {
    EGL.setErrorCode(EGL_SUCCESS);
    return EGL_OPENGL_ES_API;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglWaitClient(void);
  eglWaitClient__proxy: 'sync',
  eglWaitClient: () => {
    EGL.setErrorCode(EGL_SUCCESS);
    return EGL_TRUE;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine);
  eglWaitNative__proxy: 'sync',
  eglWaitNative: (nativeEngineId) => {
    EGL.setErrorCode(EGL_SUCCESS);
    return EGL_TRUE;
  },


  // EGLAPI EGLBoolean EGLAPIENTRY eglWaitGL(void);
  eglWaitGL: 'eglWaitClient',

  // EGLAPI EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval);
  eglSwapInterval__proxy: 'sync',
  eglSwapInterval: (display, interval) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_FALSE;
    }
    if (interval == 0) _emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 0);
    else _emscripten_set_main_loop_timing(EM_TIMING_RAF, interval);

    EGL.setErrorCode(EGL_SUCCESS);
    return EGL_TRUE;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
  eglMakeCurrent__proxy: 'sync',
  eglMakeCurrent: (display, draw, read, context) => {
    if (display != eglDefaultDisplay) {
      EGL.setErrorCode(EGL_BAD_DISPLAY);
      return EGL_FALSE;
    }
    //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy.
    if (context != EGL_NO_CONTEXT && context != eglDefaultContext) {
      EGL.setErrorCode(EGL_BAD_CONTEXT);
      return EGL_FALSE;
    }
    if ((read != EGL_NO_SURFACE && read != 62006) || (draw != EGL_NO_SURFACE && draw != 62006 /* Magic ID for Emscripten 'default surface' */)) {
      EGL.setErrorCode(EGL_BAD_SURFACE);
      return EGL_FALSE;
    }

    GL.makeContextCurrent(context ? EGL.context : null);

    EGL.currentContext = context;
    EGL.currentDrawSurface = draw;
    EGL.currentReadSurface = read;
    EGL.setErrorCode(EGL_SUCCESS);
    return EGL_TRUE;
  },

  // EGLAPI EGLContext EGLAPIENTRY eglGetCurrentContext(void);
  eglGetCurrentContext__proxy: 'sync',
  eglGetCurrentContext: () => EGL.currentContext,

  // EGLAPI EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw);
  eglGetCurrentSurface__proxy: 'sync',
  eglGetCurrentSurface: (readdraw) => {
    if (readdraw == EGL_READ) {
      return EGL.currentReadSurface;
    } else if (readdraw == EGL_DRAW) {
      return EGL.currentDrawSurface;
    } else {
      EGL.setErrorCode(EGL_BAD_PARAMETER);
      return EGL_NO_SURFACE;
    }
  },

  // EGLAPI EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void);
  eglGetCurrentDisplay__proxy: 'sync',
  eglGetCurrentDisplay: () => EGL.currentContext ? eglDefaultDisplay : EGL_NO_DISPLAY,

  // EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
  eglSwapBuffers__proxy: 'sync',
  eglSwapBuffers: (dpy, surface) => {
    if (!EGL.defaultDisplayInitialized) {
      EGL.setErrorCode(EGL_NOT_INITIALIZED);
    } else if (!GLctx) {
      EGL.setErrorCode(EGL_BAD_ACCESS);
    } else if (GLctx.isContextLost()) {
      EGL.setErrorCode(EGL_CONTEXT_LOST);
    } else {
      // According to documentation this does an implicit flush.
      // Due to discussion at https://github.com/emscripten-core/emscripten/pull/1871
      // the flush was removed since this _may_ result in slowing code down.
      //_glFlush();
      EGL.setErrorCode(EGL_SUCCESS);
      return EGL_TRUE;
    }
    return EGL_FALSE;
  },

  eglReleaseThread__proxy: 'sync',
  eglReleaseThread: () => {
    // Equivalent to eglMakeCurrent with EGL_NO_CONTEXT and EGL_NO_SURFACE.
    EGL.currentContext = EGL_NO_CONTEXT;
    EGL.currentReadSurface = EGL_NO_SURFACE;
    EGL.currentDrawSurface = EGL_NO_SURFACE;
    // EGL spec v1.4 p.55:
    // "calling eglGetError immediately following a successful call to eglReleaseThread should not be done.
    //  Such a call will return EGL_SUCCESS - but will also result in reallocating per-thread state."
    EGL.setErrorCode(EGL_SUCCESS);
    return EGL_TRUE;
  }
};

addToLibrary(LibraryEGL);
