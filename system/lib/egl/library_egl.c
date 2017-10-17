#include <EGL/egl.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

// TODO: These should be thread local
static EGLint eglError = EGL_SUCCESS;
static EGLSurface eglCurrentReadSurface = 0;
static EGLSurface eglCurrentDrawSurface = 0;
static EGLConfig eglCurrentInitializedConfig = (EGLConfig)0; // TODO: This should not be a singleton

// Process wide:
static EGLint eglDefaultDisplayInitialized = 0;
static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE windowID = 0;

#define EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY ((EGLDisplay)62000)
#define EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_SURFACE ((EGLSurface)62006)

// Currently there are few enough configuration options for creating a WebGL context
// that all the permutations can fit in an uint32. Therefore use bit packing to represent
// these combinations to avoid having to enumerate a list fo all combos, to save memory.
// This is an internal implementation detail, and can be changed in the future if the number of options
// increases to become too large to fit in an uint32/uint64.
#define EM_EGL_ALPHA_BIT 0x1
#define EM_EGL_DEPTH_BIT 0x2
#define EM_EGL_STENCIL_BIT 0x4
#define EM_EGL_ANTIALIAS_BIT 0x8
#define EM_EGL_PREMULTIPLIED_ALPHA_BIT 0x10
#define EM_EGL_PRESERVE_DRAWING_BUFFER_BIT 0x20
#define EM_EGL_PREFER_LOW_POWER_TO_HIGH_PERFORMANCE_BIT 0x40
#define EM_EGL_FAIL_IF_MAJOR_PERFORMANCE_CAVEAT_BIT 0x80
#define EM_EGL_WEBGL2_BIT 0x100
#define EM_EGL_ENABLE_EXTENSIONS_BY_DEFAULT_BIT 0x200
#define EM_EGL_EXPLICIT_SWAP_CONTROL_BIT 0x400
#define EM_EGL_PROXY_TO_MAIN_THREAD_BIT 0x800
#define EM_EGL_RENDER_VIA_OFFSCREEN_BACK_BUFFER_BIT 0x1000

EGLAPI EGLint EGLAPIENTRY eglGetError(void)
{
  return eglError;
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id)
{
  eglError = EGL_SUCCESS;

    // Note: As a 'conformant' implementation of EGL, we would prefer to init here only if the user
    //       calls this function with EGL_DEFAULT_DISPLAY. Other display IDs would be preferred to be unsupported
    //       and EGL_NO_DISPLAY returned. Uncomment the following code lines to do this.
    // Instead, an alternative route has been preferred, namely that the Emscripten EGL implementation
    // "emulates" X11, and eglGetDisplay is expected to accept/receive a pointer to an X11 Display object.
    // Therefore, be lax and allow anything to be passed in, and return the magic handle to our default EGLDisplay object.

//    if (nativeDisplayType == 0 /* EGL_DEFAULT_DISPLAY */) {
        return EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY; // Magic ID for Emscripten 'default display'
//    }
//    else
//      return EGL_FALSE; // EGL_NO_DISPLAY
}

EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }

  if (major) *major = 1;
  if (minor) *minor = 4;
  eglDefaultDisplayInitialized = 1;
  eglError = EGL_SUCCESS;
  return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }
  emscripten_webgl_make_context_current(0);
  eglCurrentReadSurface = 0;
  eglCurrentDrawSurface = 0;
  eglDefaultDisplayInitialized = 0;
  eglError = EGL_SUCCESS;
  return EGL_TRUE;
}

EGLAPI const char * EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return 0;
  }
  //\todo An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy. 
  eglError = EGL_SUCCESS;  
  switch(name)
  {
    case EGL_VENDOR: return "Emscripten";
    case EGL_VERSION: return "1.4 Emscripten EGL";
    case EGL_EXTENSIONS: return "";
    case EGL_CLIENT_APIS: return "OpenGL_ES";
    default:
      eglError = EGL_BAD_PARAMETER;
      return 0;
  }
}

static EGLBoolean ConfigPassesFilter(EGLConfig config, const EGLint *attrib_list)
{
  if (!attrib_list) return EGL_TRUE;


  return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }
  if ((!configs || !config_size) && !num_config)
  {
    eglError = EGL_BAD_PARAMETER;
    return EGL_FALSE;
  }

  const int baseConfig = EM_EGL_ENABLE_EXTENSIONS_BY_DEFAULT_BIT;
  int numMatchingConfigs = 0;
  if (!configs) config_size = 0x7FFFFFFF;

  for(int a = 0; a <= EM_EGL_ALPHA_BIT; a += EM_EGL_ALPHA_BIT)
    for(int d = 0; d <= EM_EGL_DEPTH_BIT; d += EM_EGL_DEPTH_BIT)
      for(int s = 0; s <= EM_EGL_STENCIL_BIT; s += EM_EGL_STENCIL_BIT)
        for(int aa = 0; aa <= EM_EGL_ANTIALIAS_BIT; aa += EM_EGL_ANTIALIAS_BIT)
          for(int es3 = 0; es3 <= EM_EGL_WEBGL2_BIT; es3 += EM_EGL_WEBGL2_BIT)
          {
            if (numMatchingConfigs >= config_size)
              break;

            EGLConfig config = (EGLConfig)(baseConfig | a | d | s | aa | es3);
            if (ConfigPassesFilter(config, attrib_list))
            {
              if (configs) configs[numMatchingConfigs] = config;
              ++numMatchingConfigs;
            }
          }

  *num_config = numMatchingConfigs;
  eglError = EGL_SUCCESS;
  return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
  return eglChooseConfig(dpy, 0, configs, config_size, num_config);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }

  if (!value)
  {
    eglError = EGL_BAD_PARAMETER;
    return EGL_FALSE;
  }

  eglError = EGL_SUCCESS;

  switch(attribute)
  {
    case EGL_BUFFER_SIZE: *value = 32; return EGL_TRUE;
    case EGL_ALPHA_SIZE: *value = ((EGLint)config & EM_EGL_ALPHA_BIT) ? 8 : 0; return EGL_TRUE;
    case EGL_BLUE_SIZE:
    case EGL_GREEN_SIZE:
    case EGL_RED_SIZE:
      *value = 8; return EGL_TRUE;
    case EGL_DEPTH_SIZE: *value = ((EGLint)config & EM_EGL_DEPTH_BIT) ? 24 : 0; return EGL_TRUE;
    case EGL_STENCIL_SIZE: *value = ((EGLint)config & EM_EGL_STENCIL_BIT) ? 8 : 0; return EGL_TRUE;
    // We can return here one of EGL_NONE (0x3038), EGL_SLOW_CONFIG (0x3050) or EGL_NON_CONFORMANT_CONFIG (0x3051).
    case EGL_CONFIG_CAVEAT: *value = EGL_NONE; return EGL_TRUE;
    case EGL_CONFIG_ID: *value = (EGLint)config; return EGL_TRUE;
    case EGL_LEVEL: *value = 0; return EGL_TRUE;
    case EGL_MAX_PBUFFER_HEIGHT: *value = 4096; return EGL_TRUE;
    case EGL_MAX_PBUFFER_PIXELS: *value = 4096 * 4096; return EGL_TRUE;
    case EGL_MAX_PBUFFER_WIDTH: *value = 4096; return EGL_TRUE;
    case EGL_NATIVE_RENDERABLE: *value = 0; return EGL_TRUE;
    case EGL_NATIVE_VISUAL_ID: *value = 0; return EGL_TRUE;
    case EGL_NATIVE_VISUAL_TYPE: *value = EGL_NONE; return EGL_TRUE;
    case EGL_SAMPLES: *value = ((EGLint)config & EM_EGL_ANTIALIAS_BIT) ? 4 : 0; return EGL_TRUE;
    case EGL_SAMPLE_BUFFERS: *value = ((EGLint)config & EM_EGL_ANTIALIAS_BIT) ? 1 : 0; return EGL_TRUE;
    case EGL_SURFACE_TYPE: *value = EGL_WINDOW_BIT; return EGL_TRUE;
    case EGL_TRANSPARENT_TYPE: *value = EGL_NONE; return EGL_TRUE;
    case EGL_TRANSPARENT_BLUE_VALUE:
    case EGL_TRANSPARENT_GREEN_VALUE:
    case EGL_TRANSPARENT_RED_VALUE:
      *value = -1; return EGL_TRUE;
    case EGL_BIND_TO_TEXTURE_RGB:
    case EGL_BIND_TO_TEXTURE_RGBA:
      *value = 0; return EGL_TRUE;
    case EGL_MIN_SWAP_INTERVAL: *value = 0; return EGL_TRUE;
    case EGL_MAX_SWAP_INTERVAL: *value = 32; return EGL_TRUE; // This is arbitrary, we can allow unlimited slow swap intervals, but cap to something semi-reasonable
    case EGL_LUMINANCE_SIZE:
    case EGL_ALPHA_MASK_SIZE:
      *value = 0;
      return EGL_TRUE;
    case EGL_COLOR_BUFFER_TYPE: *value = EGL_RGB_BUFFER; return EGL_TRUE;
    case EGL_RENDERABLE_TYPE: *value = EGL_OPENGL_ES2_BIT; return EGL_TRUE;
    case EGL_CONFORMANT: *value = 0; return EGL_TRUE;
    default:
      eglError = EGL_BAD_ATTRIBUTE;
      return EGL_FALSE;
  }
}

EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return 0;
  }

  eglError = EGL_SUCCESS;
  return EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_SURFACE;
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)
{
  eglError = EGL_BAD_ACCESS; // Use in place of a "Not implemented" flag
  return 0;
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
  eglError = EGL_BAD_ACCESS; // Use in place of a "Not implemented" flag
  return 0;
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }

  if (surface != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_SURFACE)
  {
    eglError = EGL_BAD_SURFACE;
    return EGL_FALSE;
  }

  if (eglCurrentReadSurface == surface) eglCurrentReadSurface = 0;
  if (eglCurrentDrawSurface == surface) eglCurrentDrawSurface = 0;
  eglError = EGL_SUCCESS;
  return EGL_TRUE;

}

EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }

  if (surface != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_SURFACE)
  {
    eglError = EGL_BAD_SURFACE;
    return EGL_FALSE;
  }

  if (!value)
  {
    eglError = EGL_BAD_PARAMETER;
    return EGL_FALSE;
  }

  eglError = EGL_SUCCESS;

  switch(attribute)
  {
    case EGL_CONFIG_ID: *value = (EGLint)eglCurrentInitializedConfig; return EGL_TRUE;
    case EGL_LARGEST_PBUFFER: 
      // Odd EGL API: If surface is not a pbuffer surface, 'value' should not be written to. It's not specified as an error, so true should(?) be returned.
      // Existing Android implementation seems to do so at least.
      return EGL_TRUE;
    case EGL_WIDTH:
    case EGL_HEIGHT:
    {
      int w, h;
      emscripten_get_canvas_element_size(NULL, &w, &h); // TODO: Figure out which canvas to query here.
      *value = (attribute == EGL_WIDTH) ? w : h;
      return EGL_TRUE;
    }
    case EGL_HORIZONTAL_RESOLUTION:
    case EGL_VERTICAL_RESOLUTION:
    case EGL_PIXEL_ASPECT_RATIO:
      *value = EGL_UNKNOWN;
      return EGL_TRUE;
    case EGL_RENDER_BUFFER: *value = EGL_BACK_BUFFER; return EGL_TRUE;
    case EGL_MULTISAMPLE_RESOLVE: *value = EGL_MULTISAMPLE_RESOLVE_DEFAULT; return EGL_TRUE;
    case EGL_SWAP_BEHAVIOR: *value = EGL_BUFFER_DESTROYED; return EGL_TRUE; // TODO: Add support for EGL_BUFFER_PRESERVED and pass it to preserveDrawingBuffer of the context creation API
    case EGL_TEXTURE_FORMAT:
    case EGL_TEXTURE_TARGET:
    case EGL_MIPMAP_TEXTURE:
    case EGL_MIPMAP_LEVEL:
      // This is a window surface, not a pbuffer surface. Spec:
      // "Querying EGL_TEXTURE_FORMAT, EGL_TEXTURE_TARGET, EGL_MIPMAP_TEXTURE, or EGL_MIPMAP_LEVEL for a non-pbuffer surface is not an error, but value is not modified."
      // So pass-through.
      return EGL_TRUE;
    default:
      eglError = EGL_BAD_ATTRIBUTE;
      return EGL_FALSE;
  }
}

EGLAPI EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api)
{
  if (api == EGL_OPENGL_ES_API)
  {
    eglError = EGL_SUCCESS;
    return EGL_TRUE;
  }
  eglError = EGL_BAD_PARAMETER;
  return EGL_FALSE;
}

EGLAPI EGLenum EGLAPIENTRY eglQueryAPI(void)
{
  eglError = EGL_SUCCESS;
  return EGL_OPENGL_ES_API;
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitClient(void)
{
  eglError = EGL_SUCCESS;
  return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglReleaseThread(void)
{
  // Equivalent to eglMakeCurrent with EGL_NO_CONTEXT and EGL_NO_SURFACE.
  emscripten_webgl_make_context_current(0);
  eglCurrentReadSurface = 0;
  eglCurrentDrawSurface = 0;

  // EGL spec v1.4 p.55:
  // "calling eglGetError immediately following a successful call to eglReleaseThread should not be done.
  //  Such a call will return EGL_SUCCESS - but will also result in reallocating per-thread state."                     
  eglError = EGL_SUCCESS;
  return EGL_TRUE;
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)
{
  eglError = EGL_BAD_ACCESS; // Use in place of a "Not implemented" flag
  return 0;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
  eglError = EGL_BAD_ACCESS; // Use in place of a "Not implemented" flag
  return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
  eglError = EGL_BAD_ACCESS; // Use in place of a "Not implemented" flag
  return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
  eglError = EGL_BAD_ACCESS; // Use in place of a "Not implemented" flag
  return EGL_FALSE;
}


EGLAPI EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }

  if (interval == 0) emscripten_set_main_loop_timing(EM_TIMING_SETIMMEDIATE, 0);
  else emscripten_set_main_loop_timing(EM_TIMING_RAF, interval);
  eglError = EGL_SUCCESS;
  return EGL_TRUE;
}


EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }

  if (!attrib_list)
  {
    eglError = EGL_BAD_PARAMETER;
    return EGL_FALSE;
  }

  // EGL 1.4 spec says default EGL_CONTEXT_CLIENT_VERSION is GLES1, but this is not supported by Emscripten.
  // So user must pass EGL_CONTEXT_CLIENT_VERSION == 2 to initialize EGL.
  int glesContextVersion = 1;
  while(*attrib_list != EGL_NONE)
  {
    switch(*attrib_list)
    {
      case EGL_CONTEXT_CLIENT_VERSION: glesContextVersion = attrib_list[1]; break;
      default:
        eglError = EGL_BAD_ATTRIBUTE;
        return EGL_NO_CONTEXT;
    }
    attrib_list += 2;
  }
  if (glesContextVersion != 2)
  {
    fprintf(stderr, "When initializing GLES2/WebGL 1 via EGL, one must pass EGL_CONTEXT_CLIENT_VERSION = 2 to GL context attributes! GLES version %d is not supported!\n", glesContextVersion);
    eglError = EGL_BAD_CONFIG;
    return EGL_NO_CONTEXT;
  }

  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.alpha = ((EGLint)config & EM_EGL_ALPHA_BIT) ? 1 : 0;
  attr.depth = ((EGLint)config & EM_EGL_DEPTH_BIT) ? 1 : 0;
  attr.stencil = ((EGLint)config & EM_EGL_STENCIL_BIT) ? 1 : 0;
  attr.antialias = ((EGLint)config & EM_EGL_ANTIALIAS_BIT) ? 1 : 0;
  attr.premultipliedAlpha = ((EGLint)config & EM_EGL_PREMULTIPLIED_ALPHA_BIT) ? 1 : 0;
  attr.preserveDrawingBuffer = ((EGLint)config & EM_EGL_PRESERVE_DRAWING_BUFFER_BIT) ? 1 : 0;
  attr.preferLowPowerToHighPerformance = ((EGLint)config & EM_EGL_PREFER_LOW_POWER_TO_HIGH_PERFORMANCE_BIT) ? 1 : 0;
  attr.failIfMajorPerformanceCaveat = ((EGLint)config & EM_EGL_FAIL_IF_MAJOR_PERFORMANCE_CAVEAT_BIT) ? 1 : 0;
  attr.majorVersion = ((EGLint)config & EM_EGL_WEBGL2_BIT) ? 2 : 1;
  attr.minorVersion = 0;
  attr.enableExtensionsByDefault = ((EGLint)config & EM_EGL_ENABLE_EXTENSIONS_BY_DEFAULT_BIT) ? 1 : 0;
  attr.explicitSwapControl = ((EGLint)config & EM_EGL_EXPLICIT_SWAP_CONTROL_BIT) ? 1 : 0;
  attr.proxyContextToMainThread = ((EGLint)config & EM_EGL_PROXY_TO_MAIN_THREAD_BIT) ? 1 : 0;
  attr.renderViaOffscreenBackBuffer = ((EGLint)config & EM_EGL_RENDER_VIA_OFFSCREEN_BACK_BUFFER_BIT) ? 1 : 0;
  windowID = emscripten_webgl_create_context(0, &attr);
  eglError = windowID ? EGL_SUCCESS : EGL_BAD_MATCH;
  if (EGL_SUCCESS)
  {
    eglCurrentInitializedConfig = config;
  }
  return (EGLContext)windowID;
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }

  if (emscripten_webgl_get_current_context() == (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)ctx) emscripten_webgl_make_context_current(0);

  EMSCRIPTEN_RESULT res = emscripten_webgl_destroy_context((EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)ctx);
  if (res >= 0)
  {
    eglError = EGL_SUCCESS;
    return EGL_TRUE;
  }

  eglCurrentInitializedConfig = (EGLConfig)0;
  return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }

  // TODO: Test if context is valid 

  if ((read != EGL_NO_SURFACE && read != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_SURFACE) || (draw != EGL_NO_SURFACE && draw != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_SURFACE))
  {
    eglError = EGL_BAD_SURFACE;
    return EGL_FALSE;
  }

  EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current((EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)ctx);
  if (res >= 0)
  {
    eglCurrentReadSurface = read;
    eglCurrentDrawSurface = draw;
    eglError = EGL_SUCCESS;
    return EGL_TRUE;
  }
  return EGL_FALSE;
}

EGLAPI EGLContext EGLAPIENTRY eglGetCurrentContext(void)
{
  return (EGLContext)emscripten_webgl_get_current_context();
}

EGLAPI EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw)
{
  if (readdraw == EGL_READ) return eglCurrentReadSurface;
  else if (readdraw == EGL_DRAW) return eglCurrentDrawSurface;
  else
  {
    eglError = EGL_BAD_PARAMETER;
    return EGL_NO_SURFACE;
  }
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void)
{
  return (emscripten_webgl_get_current_context() != 0) ? EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY : 0;
}

EGLAPI EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }

  // TODO An EGL_NOT_INITIALIZED error is generated if EGL is not initialized for dpy.

  // TODO Test if ctx is valid context

  if (!value)
  {
    eglError = EGL_BAD_PARAMETER;
    return EGL_FALSE;
  }

  eglError = EGL_SUCCESS;
  switch(attribute)
  {
    case EGL_CONFIG_ID: *value = (EGLint)eglCurrentInitializedConfig; return EGL_TRUE;
    case EGL_CONTEXT_CLIENT_TYPE: *value = EGL_OPENGL_ES_API; return EGL_TRUE;
    case EGL_CONTEXT_CLIENT_VERSION: *value = 2; return EGL_TRUE;
    case EGL_RENDER_BUFFER: *value = EGL_BACK_BUFFER; return EGL_TRUE;
    default:
      eglError = EGL_BAD_ATTRIBUTE;
      return EGL_FALSE;
  }
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitGL(void)
{
  eglError = EGL_SUCCESS;
  return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine)
{
  eglError = EGL_SUCCESS;
  return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
  if (dpy != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_DISPLAY)
  {
    eglError = EGL_BAD_DISPLAY;
    return EGL_FALSE;
  }

  if (surface != EMSCRIPTEN_EGL_MAGIC_ID_FOR_DEFAULT_SURFACE)
  {
    eglError = EGL_BAD_SURFACE;
    return EGL_FALSE;
  }

  if (!eglDefaultDisplayInitialized)
  {
    eglError = EGL_NOT_INITIALIZED;
    return EGL_FALSE;
  }

  EMSCRIPTEN_RESULT res = emscripten_webgl_commit_frame();
#if 0
  if (res >= 0)
  {
    eglError = EGL_SUCCESS;
    return EGL_TRUE;
  }
  eglError = EGL_BAD_ACCESS;
  return EGL_FALSE;
#else
  eglError = EGL_SUCCESS;
  return EGL_TRUE;
#endif
}

EGLAPI EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
  eglError = EGL_BAD_ACCESS; // Use in place of a "Not implemented" flag
  return 0;  
}

void* emscripten_GetProcAddress(const char *name_);

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY eglGetProcAddress(const char *procname)
{
  return emscripten_GetProcAddress(procname);
}
