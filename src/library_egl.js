/*
 The EGL implementation supports only one EGLNativeDisplayType, the EGL_DEFAULT_DISPLAY.
 This native display type returns the only supported EGLDisplay handle with the magic value 62000.
 There is only a single EGLConfig configuration supported, that has the magic value 62002.
 The implementation only allows a single EGLContext to be created, that has the magic value of 62004. (multiple creations silently return this same context)
 The implementation only creates a single EGLSurface, a handle with the magic value of 62006. (multiple creations silently return the same surface)
*/ 
 
var LibraryEGL = {
  $EGL: {
    // This variable tracks the success status of the most recently invoked EGL function call.
    errorCode: 0x3000 /* EGL_SUCCESS */,
    defaultDisplayInitialized: false,
    currentContext: 0 /* EGL_NO_CONTEXT */,
    currentReadSurface: 0 /* EGL_NO_SURFACE */,
    currentDrawSurface: 0 /* EGL_NO_SURFACE */,

    stringCache: {},
    
    setErrorCode: function(code) {
      EGL.errorCode = code;
    },
    
    chooseConfig: function(display, attribList, config, config_size, numConfigs) { 
      if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
        EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
        return 0;
      }
      // TODO: read attribList.
      if ((!config || !config_size) && !numConfigs) {
        EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
        return 0;
      }
      if (numConfigs) {
        {{{ makeSetValue('numConfigs', '0', '1', 'i32') }}}; // Total number of supported configs: 1.
      }
      if (config && config_size > 0) {
        {{{ makeSetValue('config', '0', '62002' /* Magic ID for the only EGLConfig supported by Emscripten */, 'i32') }}}; 
      }
      
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    },
  },

  // EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id);
  eglGetDisplay: function(nativeDisplayType) {
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    // Note: As a 'conformant' implementation of EGL, we would prefer to init here only if the user
    //       calls this function with EGL_DEFAULT_DISPLAY. Other display IDs would be preferred to be unsupported
    //       and EGL_NO_DISPLAY returned. Uncomment the following code lines to do this.
    // Instead, an alternative route has been preferred, namely that the Emscripten EGL implementation
    // "emulates" X11, and eglGetDisplay is expected to accept/receive a pointer to an X11 Display object.
    // Therefore, be lax and allow anything to be passed in, and return the magic handle to our default EGLDisplay object.

//    if (nativeDisplayType == 0 /* EGL_DEFAULT_DISPLAY */) {
        return 62000; // Magic ID for Emscripten 'default display'
//    }
//    else
//      return 0; // EGL_NO_DISPLAY
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
  eglInitialize: function(display, majorVersion, minorVersion) {
    if (display == 62000 /* Magic ID for Emscripten 'default display' */) {
      if (majorVersion) {
        {{{ makeSetValue('majorVersion', '0', '1', 'i32') }}}; // Advertise EGL Major version: '1'
      }
      if (minorVersion) {
        {{{ makeSetValue('minorVersion', '0', '4', 'i32') }}}; // Advertise EGL Minor version: '4'
      }
      EGL.defaultDisplayInitialized = true;
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    } 
    else {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy);
  eglTerminate: function(display) {
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }
    EGL.currentContext = 0;
    EGL.currentReadSurface = 0;
    EGL.currentDrawSurface = 0;
    EGL.defaultDisplayInitialized = false;
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    return 1;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
  eglGetConfigs: function(display, configs, config_size, numConfigs) { 
    return EGL.chooseConfig(display, 0, configs, config_size, numConfigs);
  },
  
  // EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
  eglChooseConfig: function(display, attrib_list, configs, config_size, numConfigs) { 
    return EGL.chooseConfig(display, attrib_list, configs, config_size, numConfigs);
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
  eglGetConfigAttrib: function(display, config, attribute, value) {
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }
    if (config != 62002 /* Magic ID for the only EGLConfig supported by Emscripten */) {
      EGL.setErrorCode(0x3005 /* EGL_BAD_CONFIG */);
      return 0;
    }
    if (!value) {
      EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
      return 0;
    }
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    switch(attribute) {
    case 0x3020: // EGL_BUFFER_SIZE
      {{{ makeSetValue('value', '0', '32' /* 8 bits for each A,R,G,B. */, 'i32') }}};
      return 1;
    case 0x3021: // EGL_ALPHA_SIZE
      {{{ makeSetValue('value', '0', '8' /* 8 bits for alpha channel. */, 'i32') }}};
      return 1;
    case 0x3022: // EGL_BLUE_SIZE
      {{{ makeSetValue('value', '0', '8' /* 8 bits for blue channel. */, 'i32') }}};
      return 1;
    case 0x3023: // EGL_GREEN_SIZE
      {{{ makeSetValue('value', '0', '8' /* 8 bits for green channel. */, 'i32') }}};
      return 1;
    case 0x3024: // EGL_RED_SIZE
      {{{ makeSetValue('value', '0', '8' /* 8 bits for red channel. */, 'i32') }}};
      return 1;
    case 0x3025: // EGL_DEPTH_SIZE
      {{{ makeSetValue('value', '0', '24' /* 24 bits for depth buffer. TODO: This is hardcoded, add support for this! */, 'i32') }}};
      return 1;
    case 0x3026: // EGL_STENCIL_SIZE
      {{{ makeSetValue('value', '0', '8' /* 8 bits for stencil buffer. TODO: This is hardcoded, add support for this! */, 'i32') }}};
      return 1;
    case 0x3027: // EGL_CONFIG_CAVEAT
      // We can return here one of EGL_NONE (0x3038), EGL_SLOW_CONFIG (0x3050) or EGL_NON_CONFORMANT_CONFIG (0x3051).
      {{{ makeSetValue('value', '0', '0x3038' /* EGL_NONE */, 'i32') }}};
      return 1;
    case 0x3028: // EGL_CONFIG_ID
      {{{ makeSetValue('value', '0', '62002' /* Magic ID for the only EGLConfig supported by Emscripten */, 'i32') }}};
      return 1;
    case 0x3029: // EGL_LEVEL
      {{{ makeSetValue('value', '0', '0' /* Z order/depth layer for this level. Not applicable for Emscripten. */, 'i32') }}};
      return 1;
    case 0x302A: // EGL_MAX_PBUFFER_HEIGHT
      {{{ makeSetValue('value', '0', '4096', 'i32') }}};
      return 1;
    case 0x302B: // EGL_MAX_PBUFFER_PIXELS
      {{{ makeSetValue('value', '0', '16777216' /* 4096 * 4096 */, 'i32') }}};
      return 1;
    case 0x302C: // EGL_MAX_PBUFFER_WIDTH
      {{{ makeSetValue('value', '0', '4096', 'i32') }}};
      return 1;
    case 0x302D: // EGL_NATIVE_RENDERABLE
      {{{ makeSetValue('value', '0', '0' /* This config does not allow co-rendering with other 'native' rendering APIs. */, 'i32') }}};
      return 1;
    case 0x302E: // EGL_NATIVE_VISUAL_ID
      {{{ makeSetValue('value', '0', '0' /* N/A for Emscripten. */, 'i32') }}};
      return 1;
    case 0x302F: // EGL_NATIVE_VISUAL_TYPE
      {{{ makeSetValue('value', '0', '0x3038' /* EGL_NONE */, 'i32') }}};
      return 1;
    case 0x3031: // EGL_SAMPLES
      {{{ makeSetValue('value', '0', '4' /* 2x2 Multisampling */, 'i32') }}};
      return 1;
    case 0x3032: // EGL_SAMPLE_BUFFERS
      {{{ makeSetValue('value', '0', '1' /* Multisampling enabled */, 'i32') }}};
      return 1;
    case 0x3033: // EGL_SURFACE_TYPE
      {{{ makeSetValue('value', '0', '0x0004' /* EGL_WINDOW_BIT */, 'i32') }}};
      return 1;
    case 0x3034: // EGL_TRANSPARENT_TYPE
      // If this returns EGL_TRANSPARENT_RGB (0x3052), transparency is used through color-keying. No such thing applies to Emscripten canvas.
      {{{ makeSetValue('value', '0', '0x3038' /* EGL_NONE */, 'i32') }}};
      return 1;
    case 0x3035: // EGL_TRANSPARENT_BLUE_VALUE
    case 0x3036: // EGL_TRANSPARENT_GREEN_VALUE
    case 0x3037: // EGL_TRANSPARENT_RED_VALUE
      // "If EGL_TRANSPARENT_TYPE is EGL_NONE, then the values for EGL_TRANSPARENT_RED_VALUE, EGL_TRANSPARENT_GREEN_VALUE, and EGL_TRANSPARENT_BLUE_VALUE are undefined."
      {{{ makeSetValue('value', '0', '-1' /* Report a "does not apply" value. */, 'i32') }}};
      return 1;
    case 0x3039: // EGL_BIND_TO_TEXTURE_RGB
    case 0x303A: // EGL_BIND_TO_TEXTURE_RGBA
      {{{ makeSetValue('value', '0', '0' /* Only pbuffers would be bindable, but these are not supported. */, 'i32') }}};
      return 1;
    case 0x303B: // EGL_MIN_SWAP_INTERVAL
    case 0x303C: // EGL_MAX_SWAP_INTERVAL
      {{{ makeSetValue('value', '0', '1' /* TODO: Currently this is not strictly true, since user can specify custom presentation interval in JS requestAnimationFrame/emscripten_set_main_loop. */, 'i32') }}};
      return 1;
    case 0x303D: // EGL_LUMINANCE_SIZE
    case 0x303E: // EGL_ALPHA_MASK_SIZE
      {{{ makeSetValue('value', '0', '0' /* N/A in this config. */, 'i32') }}};
      return 1;
    case 0x303F: // EGL_COLOR_BUFFER_TYPE
      // EGL has two types of buffers: EGL_RGB_BUFFER and EGL_LUMINANCE_BUFFER.
      {{{ makeSetValue('value', '0', '0x308E' /* EGL_RGB_BUFFER */, 'i32') }}};
      return 1;
    case 0x3040: // EGL_RENDERABLE_TYPE
      // A bit combination of EGL_OPENGL_ES_BIT,EGL_OPENVG_BIT,EGL_OPENGL_ES2_BIT and EGL_OPENGL_BIT.
      {{{ makeSetValue('value', '0', '0x0004' /* EGL_OPENGL_ES2_BIT */, 'i32') }}};
      return 1;
    case 0x3042: // EGL_CONFORMANT
      // "EGL_CONFORMANT is a mask indicating if a client API context created with respect to the corresponding EGLConfig will pass the required conformance tests for that API."
      {{{ makeSetValue('value', '0', '0' /* EGL_OPENGL_ES2_BIT */, 'i32') }}};
      return 1;
    default:
      EGL.setErrorCode(0x3004 /* EGL_BAD_ATTRIBUTE */);
      return 0;
    }
  },

  // EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
  eglCreateWindowSurface: function(display, config, win, attrib_list) { 
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }
    if (config != 62002 /* Magic ID for the only EGLConfig supported by Emscripten */) {
      EGL.setErrorCode(0x3005 /* EGL_BAD_CONFIG */);
      return 0;
    }
    // TODO: Examine attrib_list! Parameters that can be present there are:
    // - EGL_RENDER_BUFFER (must be EGL_BACK_BUFFER)
    // - EGL_VG_COLORSPACE (can't be set)
    // - EGL_VG_ALPHA_FORMAT (can't be set)
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    return 62006; /* Magic ID for Emscripten 'default surface' */
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay display, EGLSurface surface);
  eglDestroySurface: function(display, surface) { 
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0; 
    }
    if (surface != 62006 /* Magic ID for the only EGLSurface supported by Emscripten */) {
      EGL.setErrorCode(0x300D /* EGL_BAD_SURFACE */);
      return 1;
    }
    if (EGL.currentReadSurface == surface) {
      EGL.currentReadSurface = 0;
    }
    if (EGL.currentDrawSurface == surface) {
      EGL.currentDrawSurface = 0;
    }
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    return 1; /* Magic ID for Emscripten 'default surface' */
  },

  eglCreateContext__deps: ['glutInitDisplayMode', 'glutCreateWindow', '$GL'],
  
  // EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
  eglCreateContext: function(display, config, hmm, contextAttribs) {
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }

    // EGL 1.4 spec says default EGL_CONTEXT_CLIENT_VERSION is GLES1, but this is not supported by Emscripten.
    // So user must pass EGL_CONTEXT_CLIENT_VERSION == 2 to initialize EGL.
    var glesContextVersion = 1;
    for(;;) {
      var param = {{{ makeGetValue('contextAttribs', '0', 'i32') }}};
      if (param == 0x3098 /*EGL_CONTEXT_CLIENT_VERSION*/) {
        glesContextVersion = {{{ makeGetValue('contextAttribs', '4', 'i32') }}};
      } else if (param == 0x3038 /*EGL_NONE*/) {
        break;
      } else {
        /* EGL1.4 specifies only EGL_CONTEXT_CLIENT_VERSION as supported attribute */
        EGL.setErrorCode(0x3004 /*EGL_BAD_ATTRIBUTE*/);
        return 0;
      }
      contextAttribs += 8;
    }
    if (glesContextVersion != 2) {
#if GL_ASSERTIONS
      Module.printErr('When initializing GLES2/WebGL1 via EGL, one must pass EGL_CONTEXT_CLIENT_VERSION = 2 to GL context attributes! GLES version ' + glesContextVersion + ' is not supported!');
#endif
      EGL.setErrorCode(0x3005 /* EGL_BAD_CONFIG */);
      return 0; /* EGL_NO_CONTEXT */
    }

    _glutInitDisplayMode(0xB2 /* GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_STENCIL */);
    EGL.windowID = _glutCreateWindow();
    if (EGL.windowID != 0) {
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      // Note: This function only creates a context, but it shall not make it active.
      return 62004; // Magic ID for Emscripten EGLContext
    } else {
      EGL.setErrorCode(0x3009 /* EGL_BAD_MATCH */); // By the EGL 1.4 spec, an implementation that does not support GLES2 (WebGL in this case), this error code is set.
      return 0; /* EGL_NO_CONTEXT */
    }
  },

  eglDestroyContext__deps: ['glutDestroyWindow', '$GL'],
  
  // EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext context);
  eglDestroyContext: function(display, context) {
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }
    if (context != 62004 /* Magic ID for Emscripten EGLContext */) {
      EGL.setErrorCode(0x3006 /* EGL_BAD_CONTEXT */);
      return 0;
    }

    _glutDestroyWindow(EGL.windowID);
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    if (EGL.currentContext == context) {
      EGL.currentContext = 0;
    }
    return 1 /* EGL_TRUE */;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
  eglDestroyContext: function(display, context) {
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }

    if (context != 62004 /* Magic ID for Emscripten EGLContext */) {
      EGL.setErrorCode(0x3006 /* EGL_BAD_CONTEXT */);
      return 0;
    }

    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    return 1;
  }, 

  // EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
  eglQuerySurface: function(display, surface, attribute, value) { 
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }
    if (surface != 62006 /* Magic ID for Emscripten 'default surface' */) {
      EGL.setErrorCode(0x300D /* EGL_BAD_SURFACE */);
      return 0;
    }
    if (!value) {
      EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
      return 0;
    }
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    switch(attribute) {
    case 0x3028: // EGL_CONFIG_ID
      {{{ makeSetValue('value', '0', '62002' /* A magic value for the only EGLConfig configuration ID supported by Emscripten. */, 'i32') }}};
        return 1;
    case 0x3058: // EGL_LARGEST_PBUFFER
      // Odd EGL API: If surface is not a pbuffer surface, 'value' should not be written to. It's not specified as an error, so true should(?) be returned.
      // Existing Android implementation seems to do so at least.
      return 1;
    case 0x3057: // EGL_WIDTH
      {{{ makeSetValue('value', '0', 'Module.canvas.width', 'i32') }}};
      return 1;
    case 0x3056: // EGL_HEIGHT
      {{{ makeSetValue('value', '0', 'Module.canvas.height', 'i32') }}};
      return 1;
    case 0x3090: // EGL_HORIZONTAL_RESOLUTION
      {{{ makeSetValue('value', '0', '-1' /* EGL_UNKNOWN */, 'i32') }}};
      return 1;
    case 0x3091: // EGL_VERTICAL_RESOLUTION
      {{{ makeSetValue('value', '0', '-1' /* EGL_UNKNOWN */, 'i32') }}};
      return 1;
    case 0x3092: // EGL_PIXEL_ASPECT_RATIO
      {{{ makeSetValue('value', '0', '-1' /* EGL_UNKNOWN */, 'i32') }}};
      return 1;
    case 0x3086: // EGL_RENDER_BUFFER
      // The main surface is bound to the visible canvas window - it's always backbuffered. 
      // Alternative to EGL_BACK_BUFFER would be EGL_SINGLE_BUFFER.
      {{{ makeSetValue('value', '0', '0x3084' /* EGL_BACK_BUFFER */, 'i32') }}}; 
      return 1;
    case 0x3099: // EGL_MULTISAMPLE_RESOLVE
      {{{ makeSetValue('value', '0', '0x309A' /* EGL_MULTISAMPLE_RESOLVE_DEFAULT */, 'i32') }}}; 
      return 1;
    case 0x3093: // EGL_SWAP_BEHAVIOR
      // The two possibilities are EGL_BUFFER_PRESERVED and EGL_BUFFER_DESTROYED. Slightly unsure which is the
      // case for browser environment, but advertise the 'weaker' behavior to be sure.
      {{{ makeSetValue('value', '0', '0x3095' /* EGL_BUFFER_DESTROYED */, 'i32') }}};
      return 1;
    case 0x3080: // EGL_TEXTURE_FORMAT
    case 0x3081: // EGL_TEXTURE_TARGET
    case 0x3082: // EGL_MIPMAP_TEXTURE
    case 0x3083: // EGL_MIPMAP_LEVEL
      // This is a window surface, not a pbuffer surface. Spec:
      // "Querying EGL_TEXTURE_FORMAT, EGL_TEXTURE_TARGET, EGL_MIPMAP_TEXTURE, or EGL_MIPMAP_LEVEL for a non-pbuffer surface is not an error, but value is not modified."
      // So pass-through.
      return 1;
    default:
      EGL.setErrorCode(0x3004 /* EGL_BAD_ATTRIBUTE */);
      return 0;
    }
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
  eglQueryContext: function(display, context, attribute, value) {
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }
    //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy. 
    if (context != 62004 /* Magic ID for Emscripten EGLContext */) {
      EGL.setErrorCode(0x3006 /* EGL_BAD_CONTEXT */);
      return 0;
    }
    if (!value) {
      EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
      return 0;
    }
  
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    switch(attribute) {
      case 0x3028: // EGL_CONFIG_ID
        {{{ makeSetValue('value', '0', '62002' /* A magic value for the only EGLConfig configuration ID supported by Emscripten. */, 'i32') }}};
        return 1;
      case 0x3097: // EGL_CONTEXT_CLIENT_TYPE
        {{{ makeSetValue('value', '0', '0x30A0' /* EGL_OPENGL_ES_API */, 'i32') }}};
        return 1;
      case 0x3098: // EGL_CONTEXT_CLIENT_VERSION
        {{{ makeSetValue('value', '0', '2' /* GLES2 context */, 'i32') }}}; // We always report the context to be a GLES2 context (and not a GLES1 context)
        return 1;
      case 0x3086: // EGL_RENDER_BUFFER
        // The context is bound to the visible canvas window - it's always backbuffered. 
        // Alternative to EGL_BACK_BUFFER would be EGL_SINGLE_BUFFER.
        {{{ makeSetValue('value', '0', '0x3084' /* EGL_BACK_BUFFER */, 'i32') }}}; 
        return 1;
      default:
        EGL.setErrorCode(0x3004 /* EGL_BAD_ATTRIBUTE */);
        return 0;
    }
  },
  
  // EGLAPI EGLint EGLAPIENTRY eglGetError(void);
  eglGetError: function() { 
    return EGL.errorCode;
  },

  // EGLAPI const char * EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name);
  eglQueryString: function(display, name) {
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }
    //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy. 
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    if (EGL.stringCache[name]) return EGL.stringCache[name];
    var ret;
    switch(name) {
      case 0x3053 /* EGL_VENDOR */: ret = allocate(intArrayFromString("Emscripten"), 'i8', ALLOC_NORMAL); break;
      case 0x3054 /* EGL_VERSION */: ret = allocate(intArrayFromString("1.4 Emscripten EGL"), 'i8', ALLOC_NORMAL); break;
      case 0x3055 /* EGL_EXTENSIONS */:  ret = allocate(intArrayFromString(""), 'i8', ALLOC_NORMAL); break; // Currently not supporting any EGL extensions.
      case 0x308D /* EGL_CLIENT_APIS */: ret = allocate(intArrayFromString("OpenGL_ES"), 'i8', ALLOC_NORMAL); break;
      default:
        EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
        return 0;
    }
    EGL.stringCache[name] = ret;
    return ret;
  },
  
  // EGLAPI EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api);
  eglBindAPI: function(api) {
    if (api == 0x30A0 /* EGL_OPENGL_ES_API */) {
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1;
    } else { // if (api == 0x30A1 /* EGL_OPENVG_API */ || api == 0x30A2 /* EGL_OPENGL_API */) {
      EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
      return 0;
    }
  },

  // EGLAPI EGLenum EGLAPIENTRY eglQueryAPI(void);
  eglQueryAPI: function() {
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    return 0x30A0; // EGL_OPENGL_ES_API
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglWaitClient(void);
  eglWaitClient: function() {
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    return 1;
  },

  // EGLAPI EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine);
  eglWaitNative: function(nativeEngineId) {
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    return 1;
  },


  // EGLAPI EGLBoolean EGLAPIENTRY eglWaitGL(void);
  eglWaitGL: 'eglWaitClient',

  // EGLAPI EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval);
  eglSwapInterval__deps: ['emscripten_set_main_loop_timing'],
  eglSwapInterval: function(display, interval) {
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0;
    }
    if (interval == 0) _emscripten_set_main_loop_timing(0/*EM_TIMING_SETTIMEOUT*/, 0);
    else _emscripten_set_main_loop_timing(1/*EM_TIMING_RAF*/, interval);

    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    return 1;
  },
  
  // EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
  eglMakeCurrent: function(display, draw, read, context) { 
    if (display != 62000 /* Magic ID for Emscripten 'default display' */) {
      EGL.setErrorCode(0x3008 /* EGL_BAD_DISPLAY */);
      return 0 /* EGL_FALSE */;
    }
    //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy. 
    if (context != 0 && context != 62004 /* Magic ID for Emscripten EGLContext */) {
      EGL.setErrorCode(0x3006 /* EGL_BAD_CONTEXT */);
      return 0;
    }
    if ((read != 0 && read != 62006) || (draw != 0 && draw != 62006 /* Magic ID for Emscripten 'default surface' */)) {
      EGL.setErrorCode(0x300D /* EGL_BAD_SURFACE */);
      return 0;
    }
    EGL.currentContext = context;
    EGL.currentDrawSurface = draw;
    EGL.currentReadSurface = read;
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    return 1 /* EGL_TRUE */;
  },

  // EGLAPI EGLContext EGLAPIENTRY eglGetCurrentContext(void);
  eglGetCurrentContext: function() {
    return EGL.currentContext;
  },

  // EGLAPI EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw);
  eglGetCurrentSurface: function(readdraw) {
    if (readdraw == 0x305A /* EGL_READ */) {
      return EGL.currentReadSurface;
    } else if (readdraw == 0x3059 /* EGL_DRAW */) {
      return EGL.currentDrawSurface;
    } else {
      EGL.setErrorCode(0x300C /* EGL_BAD_PARAMETER */);
      return 0 /* EGL_NO_SURFACE */;
    }
  },

  // EGLAPI EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void);
  eglGetCurrentDisplay: function() {
    return EGL.currentContext ? 62000 /* Magic ID for Emscripten 'default display' */ : 0;
  },
  
  // EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
  eglSwapBuffers: function() {
#if PROXY_TO_WORKER
    if (Browser.doSwapBuffers) Browser.doSwapBuffers();
#endif

    if (!EGL.defaultDisplayInitialized) {
      EGL.setErrorCode(0x3001 /* EGL_NOT_INITIALIZED */);
    } else if (!Module.ctx) {
      EGL.setErrorCode(0x3002 /* EGL_BAD_ACCESS */);
    } else if (Module.ctx.isContextLost()) {
      EGL.setErrorCode(0x300E /* EGL_CONTEXT_LOST */);
    } else {
      // According to documentation this does an implicit flush.
      // Due to discussion at https://github.com/kripken/emscripten/pull/1871
      // the flush was removed since this _may_ result in slowing code down.
      //_glFlush();
      EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
      return 1 /* EGL_TRUE */;
    }
    return 0 /* EGL_FALSE */;
  },

  eglGetProcAddress__deps: ['emscripten_GetProcAddress'],
  eglGetProcAddress: function(name_) {
    return _emscripten_GetProcAddress(name_);
  },

  eglReleaseThread: function() {
    // Equivalent to eglMakeCurrent with EGL_NO_CONTEXT and EGL_NO_SURFACE.
    EGL.currentContext = 0;
    EGL.currentReadSurface = 0;
    EGL.currentDrawSurface = 0;
    // EGL spec v1.4 p.55:
    // "calling eglGetError immediately following a successful call to eglReleaseThread should not be done.
    //  Such a call will return EGL_SUCCESS - but will also result in reallocating per-thread state."                     
    EGL.setErrorCode(0x3000 /* EGL_SUCCESS */);
    return 1 /* EGL_TRUE */;
  }
};

autoAddDeps(LibraryEGL, '$EGL');

mergeInto(LibraryManager.library, LibraryEGL);
