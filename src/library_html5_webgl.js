/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryHtml5WebGL = {
  // Writes a JS typed array containing 32-bit floats or ints to memory
  $writeGLArray: (arr, dst, dstLength, heapType) => {
#if ASSERTIONS
    assert(arr);
    assert(typeof arr.length != 'undefined');
#endif
    var len = arr.length;
    var writeLength = dstLength < len ? dstLength : len;
    var heap = heapType ? HEAPF32 : HEAP32;
    // Works because HEAPF32 and HEAP32 have the same bytes-per-element
    dst = {{{ getHeapOffset('dst', 'float') }}};
    for (var i = 0; i < writeLength; ++i) {
      heap[dst + i] = arr[i];
    }
    return len;
  },

  $webglPowerPreferences__internal: true,
  $webglPowerPreferences: ['default', 'low-power', 'high-performance'],

#if PTHREADS && OFFSCREEN_FRAMEBUFFER
  // In offscreen framebuffer mode, we implement a proxied version of the
  // emscripten_webgl_create_context() function in JS.
  emscripten_webgl_create_context_proxied__proxy: 'sync',
  emscripten_webgl_create_context_proxied__deps: ['emscripten_webgl_do_create_context'],
  emscripten_webgl_create_context_proxied: (target, attributes) =>
    _emscripten_webgl_do_create_context(target, attributes),

  // The other proxied GL commands are defined in C (guarded by the
  // __EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__ definition).
#else
  // When not in offscreen framebuffer mode, these functions are implemented
  // in JS and forwarded without any proxying.
  emscripten_webgl_create_context: 'emscripten_webgl_do_create_context',

  emscripten_webgl_get_current_context: 'emscripten_webgl_do_get_current_context',

  emscripten_webgl_commit_frame: 'emscripten_webgl_do_commit_frame',
#endif

#if OFFSCREENCANVAS_SUPPORT
  emscripten_webgl_do_create_context__postset: `
  registerPreMainLoop(() => {
    // If the current GL context is an OffscreenCanvas, but it was initialized
    // with implicit swap mode, perform the swap on behalf of the user.
    if (GL.currentContext && !GL.currentContextIsProxied && !GL.currentContext.attributes.explicitSwapControl && GL.currentContext.GLctx.commit) {
      GL.currentContext.GLctx.commit();
    }
  });`,
#endif

  emscripten_webgl_do_create_context__deps: [
#if OFFSCREENCANVAS_SUPPORT
  '$registerPreMainLoop',
  'malloc',
  'emscripten_supports_offscreencanvas',
#endif
#if PTHREADS && OFFSCREEN_FRAMEBUFFER
  'emscripten_webgl_create_context_proxied',
#endif
  '$JSEvents', '$webglPowerPreferences', '$findEventTarget', '$findCanvasEventTarget'],
  // This function performs proxying manually, depending on the style of context that is to be created.
  emscripten_webgl_do_create_context: (target, attributes) => {
#if ASSERTIONS
    assert(attributes);
#endif
    var attr32 = {{{ getHeapOffset('attributes', 'i32') }}};
    var powerPreference = HEAP32[attr32 + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.powerPreference }}}>>2)];
    var contextAttributes = {
      'alpha': !!HEAP8[attributes + {{{ C_STRUCTS.EmscriptenWebGLContextAttributes.alpha }}}],
      'depth': !!HEAP8[attributes + {{{ C_STRUCTS.EmscriptenWebGLContextAttributes.depth }}}],
      'stencil': !!HEAP8[attributes + {{{ C_STRUCTS.EmscriptenWebGLContextAttributes.stencil }}}],
      'antialias': !!HEAP8[attributes + {{{ C_STRUCTS.EmscriptenWebGLContextAttributes.antialias }}}],
      'premultipliedAlpha': !!HEAP8[attributes + {{{ C_STRUCTS.EmscriptenWebGLContextAttributes.premultipliedAlpha }}}],
      'preserveDrawingBuffer': !!HEAP8[attributes + {{{ C_STRUCTS.EmscriptenWebGLContextAttributes.preserveDrawingBuffer }}}],
      'powerPreference': webglPowerPreferences[powerPreference],
      'failIfMajorPerformanceCaveat': !!HEAP8[attributes + {{{ C_STRUCTS.EmscriptenWebGLContextAttributes.failIfMajorPerformanceCaveat }}}],
      // The following are not predefined WebGL context attributes in the WebGL specification, so the property names can be minified by Closure.
      majorVersion: HEAP32[attr32 + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.majorVersion }}}>>2)],
      minorVersion: HEAP32[attr32 + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.minorVersion }}}>>2)],
      enableExtensionsByDefault: HEAP8[attributes + {{{ C_STRUCTS.EmscriptenWebGLContextAttributes.enableExtensionsByDefault }}}],
      explicitSwapControl: HEAP8[attributes + {{{ C_STRUCTS.EmscriptenWebGLContextAttributes.explicitSwapControl }}}],
      proxyContextToMainThread: HEAP32[attr32 + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.proxyContextToMainThread }}}>>2)],
      renderViaOffscreenBackBuffer: HEAP8[attributes + {{{ C_STRUCTS.EmscriptenWebGLContextAttributes.renderViaOffscreenBackBuffer }}}]
    };

    var canvas = findCanvasEventTarget(target);

#if GL_DEBUG
    var targetStr = UTF8ToString(target);
#endif

#if PTHREADS && OFFSCREEN_FRAMEBUFFER
    // Create a WebGL context that is proxied to main thread if canvas was not found on worker, or if explicitly requested to do so.
    if (ENVIRONMENT_IS_PTHREAD) {
      if (contextAttributes.proxyContextToMainThread === {{{ cDefs.EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS }}} ||
         (!canvas && contextAttributes.proxyContextToMainThread === {{{ cDefs.EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK }}})) {
        // When WebGL context is being proxied via the main thread, we must render using an offscreen FBO render target to avoid WebGL's
        // "implicit swap when callback exits" behavior. TODO: If OffscreenCanvas is supported, explicitSwapControl=true and still proxying,
        // then this can be avoided, since OffscreenCanvas enables explicit swap control.
#if GL_DEBUG
        if (contextAttributes.proxyContextToMainThread === {{{ cDefs.EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS }}}) dbg('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS enabled, proxying WebGL rendering from pthread to main thread.');
        if (!canvas && contextAttributes.proxyContextToMainThread === {{{ cDefs.EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK }}}) dbg(`Specified canvas target "${targetStr}" is not an OffscreenCanvas in the current pthread, but EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK is set. Proxying WebGL rendering from pthread to main thread.`);
        dbg('Performance warning: forcing renderViaOffscreenBackBuffer=true and preserveDrawingBuffer=true since proxying WebGL rendering.');
#endif
        // We will be proxying - if OffscreenCanvas is supported, we can proxy a bit more efficiently by avoiding having to create an Offscreen FBO.
        if (!_emscripten_supports_offscreencanvas()) {
          {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.renderViaOffscreenBackBuffer, '1', 'i8') }}};
          {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.preserveDrawingBuffer, '1', 'i8') }}};
        }
        return _emscripten_webgl_create_context_proxied(target, attributes);
      }
    }
#endif

    if (!canvas) {
#if GL_DEBUG
      dbg(`emscripten_webgl_create_context failed: Unknown canvas target "${targetStr}"!`);
#endif
      return 0;
    }

#if OFFSCREENCANVAS_SUPPORT
    if (canvas.offscreenCanvas) canvas = canvas.offscreenCanvas;

#if GL_DEBUG
    if (_emscripten_supports_offscreencanvas() && canvas instanceof OffscreenCanvas) dbg(`emscripten_webgl_create_context: Creating an OffscreenCanvas-based WebGL context on target "${targetStr}"`);
    else if (typeof HTMLCanvasElement != 'undefined' && canvas instanceof HTMLCanvasElement) dbg(`emscripten_webgl_create_context: Creating an HTMLCanvasElement-based WebGL context on target "${targetStr}"`);
#endif

    if (contextAttributes.explicitSwapControl) {
      var supportsOffscreenCanvas = canvas.transferControlToOffscreen || (_emscripten_supports_offscreencanvas() && canvas instanceof OffscreenCanvas);

      if (!supportsOffscreenCanvas) {
#if OFFSCREEN_FRAMEBUFFER
        if (!contextAttributes.renderViaOffscreenBackBuffer) {
          contextAttributes.renderViaOffscreenBackBuffer = true;
#if GL_DEBUG
          dbg('emscripten_webgl_create_context: Performance warning, OffscreenCanvas is not supported but explicitSwapControl was requested, so force-enabling renderViaOffscreenBackBuffer=true to allow explicit swapping!');
#endif
        }
#else
#if GL_DEBUG
        dbg('emscripten_webgl_create_context failed: OffscreenCanvas is not supported but explicitSwapControl was requested!');
#endif
        return 0;
#endif
      }

      if (canvas.transferControlToOffscreen) {
#if GL_DEBUG
        dbg(`explicitSwapControl requested: canvas.transferControlToOffscreen() on canvas "${targetStr}" to get .commit() function and not rely on implicit WebGL swap`);
#endif
        if (!canvas.controlTransferredOffscreen) {
          GL.offscreenCanvases[canvas.id] = {
            canvas: canvas.transferControlToOffscreen(),
            canvasSharedPtr: _malloc(12),
            id: canvas.id
          };
          canvas.controlTransferredOffscreen = true;
        } else if (!GL.offscreenCanvases[canvas.id]) {
#if GL_DEBUG
          dbg(`OffscreenCanvas is supported, and canvas "${canvas.id}" has already before been transferred offscreen, but there is no known OffscreenCanvas with that name!`);
#endif
          return 0;
        }
        canvas = GL.offscreenCanvases[canvas.id];
      }
    }
#else // !OFFSCREENCANVAS_SUPPORT
#if OFFSCREEN_FRAMEBUFFER
    if (contextAttributes.explicitSwapControl && !contextAttributes.renderViaOffscreenBackBuffer) {
      contextAttributes.renderViaOffscreenBackBuffer = true;
#if GL_DEBUG
      dbg('emscripten_webgl_create_context: Performance warning, not building with OffscreenCanvas support enabled but explicitSwapControl was requested, so force-enabling renderViaOffscreenBackBuffer=true to allow explicit swapping!');
#endif
    }
#else
    if (contextAttributes.explicitSwapControl) {
#if GL_DEBUG
      dbg('emscripten_webgl_create_context failed: explicitSwapControl is not supported, please rebuild with -sOFFSCREENCANVAS_SUPPORT to enable targeting the experimental OffscreenCanvas specification, or rebuild with -sOFFSCREEN_FRAMEBUFFER to emulate explicitSwapControl in the absence of OffscreenCanvas support!');
#endif
      return 0;
    }
#endif // ~!OFFSCREEN_FRAMEBUFFER

#endif // ~!OFFSCREENCANVAS_SUPPORT

    var contextHandle = GL.createContext(canvas, contextAttributes);
    return contextHandle;
  },

#if PTHREADS && OFFSCREEN_FRAMEBUFFER
  // Runs on the calling thread, proxies if needed.
  emscripten_webgl_make_context_current_calling_thread__sig: 'ip',
  emscripten_webgl_make_context_current_calling_thread: (contextHandle) => {
    var success = GL.makeContextCurrent(contextHandle);
    if (success) GL.currentContextIsProxied = false; // If succeeded above, we will have a local GL context from this thread (worker or main).
    return success ? {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}} : {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
  },
  // This function gets called in a pthread, after it has successfully activated (with make_current()) a proxied GL context to itself from the main thread.
  // In this scenario, the pthread does not hold a high-level JS object to the GL context, because it lives on the main thread, in which case we record
  // an integer pointer as a token value to represent the GL context activation from another thread. (when this function is called, the main browser thread
  // has already accepted the GL context activation for our pthread, so that side is good)
#if GL_SUPPORT_EXPLICIT_SWAP_CONTROL
  _emscripten_proxied_gl_context_activated_from_main_browser_thread__deps: ['$registerPreMainLoop'],
  _emscripten_proxied_gl_context_activated_from_main_browser_thread__postjs: `
    // If the current GL context is a proxied regular WebGL context, and was
    // initialized with implicit swap mode on the main thread, and we are on the
    // parent thread, perform the swap on behalf of the user.
    registerPreMainLoop(() => {
      if (GL.currentContext && GL.currentContextIsProxied) {
        var explicitSwapControl = {{{ makeGetValue('GL.currentContext', 0, 'i32') }}};
        if (!explicitSwapControl) _emscripten_webgl_commit_frame();
      }
    });`,
#endif
  _emscripten_proxied_gl_context_activated_from_main_browser_thread: (contextHandle) => {
    GLctx = Module.ctx = GL.currentContext = contextHandle;
    GL.currentContextIsProxied = true;
  },
#else
  emscripten_webgl_make_context_current: (contextHandle) => {
    var success = GL.makeContextCurrent(contextHandle);
    return success ? {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}} : {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
  },
#endif

  emscripten_webgl_do_get_current_context: () => GL.currentContext ? GL.currentContext.handle : 0,

  emscripten_webgl_get_drawing_buffer_size__proxy: 'sync_on_webgl_context_handle_thread',
  emscripten_webgl_get_drawing_buffer_size: (contextHandle, width, height) => {
    var GLContext = GL.getContext(contextHandle);

    if (!GLContext || !GLContext.GLctx || !width || !height) {
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    }
    {{{ makeSetValue('width', '0', 'GLContext.GLctx.drawingBufferWidth', 'i32') }}};
    {{{ makeSetValue('height', '0', 'GLContext.GLctx.drawingBufferHeight', 'i32') }}};
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_webgl_do_commit_frame: () => {
#if TRACE_WEBGL_CALLS
    var threadId = (typeof _pthread_self != 'undefined') ? _pthread_self : () => 1;
    err(`[Thread ${threadId()}, GL ctx: ${GL.currentContext.handle}]: emscripten_webgl_do_commit_frame()`);
#endif
    if (!GL.currentContext || !GL.currentContext.GLctx) {
#if GL_DEBUG
      dbg('emscripten_webgl_commit_frame() failed: no GL context set current via emscripten_webgl_make_context_current()!');
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }

#if OFFSCREEN_FRAMEBUFFER
    if (GL.currentContext.defaultFbo) {
      GL.blitOffscreenFramebuffer(GL.currentContext);
#if GL_DEBUG && OFFSCREENCANVAS_SUPPORT
      if (GL.currentContext.GLctx.commit) dbg('emscripten_webgl_commit_frame(): Offscreen framebuffer should never have gotten created when canvas is in OffscreenCanvas mode, since it is redundant and not necessary');
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
    }
#endif
    if (!GL.currentContext.attributes.explicitSwapControl) {
#if GL_DEBUG
      dbg('emscripten_webgl_commit_frame() cannot be called for canvases with implicit swap control mode!');
#endif
      return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    }
    // We would do GL.currentContext.GLctx.commit(); here, but the current implementation
    // in browsers has removed it - swap is implicit, so this function is a no-op for now
    // (until/unless the spec changes).
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_webgl_get_context_attributes__proxy: 'sync_on_webgl_context_handle_thread',
  emscripten_webgl_get_context_attributes__deps: ['$webglPowerPreferences'],
  emscripten_webgl_get_context_attributes: (c, a) => {
    if (!a) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_PARAM }}};
    c = GL.contexts[c];
    if (!c) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    var t = c.GLctx;
    if (!t) return {{{ cDefs.EMSCRIPTEN_RESULT_INVALID_TARGET }}};
    t = t.getContextAttributes();

    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.alpha, 't.alpha', 'i8') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.depth, 't.depth', 'i8') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.stencil, 't.stencil', 'i8') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.antialias, 't.antialias', 'i8') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.premultipliedAlpha, 't.premultipliedAlpha', 'i8') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.preserveDrawingBuffer, 't.preserveDrawingBuffer', 'i8') }}};
    var power = t['powerPreference'] && webglPowerPreferences.indexOf(t['powerPreference']);
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.powerPreference, 'power', 'i32') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.failIfMajorPerformanceCaveat, 't.failIfMajorPerformanceCaveat', 'i8') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.majorVersion, 'c.version', 'i32') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.minorVersion, 0, 'i32') }}};
#if GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.enableExtensionsByDefault, 'c.attributes.enableExtensionsByDefault', 'i8') }}};
#endif
#if GL_SUPPORT_EXPLICIT_SWAP_CONTROL
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.explicitSwapControl, 'c.attributes.explicitSwapControl', 'i8') }}};
#endif
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_webgl_destroy_context__proxy: 'sync_on_webgl_context_handle_thread',
  emscripten_webgl_destroy_context__deps: ['free'],
  emscripten_webgl_destroy_context: (contextHandle) => {
    if (GL.currentContext == contextHandle) GL.currentContext = 0;
    GL.deleteContext(contextHandle);
  },

#if PTHREADS
  // Special function that will be invoked on the thread calling emscripten_webgl_destroy_context(), before routing
  // the call over to the target thread.
  $emscripten_webgl_destroy_context_before_on_calling_thread__deps: ['emscripten_webgl_get_current_context', 'emscripten_webgl_make_context_current'],
  $emscripten_webgl_destroy_context_before_on_calling_thread: (contextHandle) => {
    if (_emscripten_webgl_get_current_context() == contextHandle) _emscripten_webgl_make_context_current(0);
  },
#endif

  emscripten_webgl_enable_extension__deps: [
#if GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS
#if MIN_WEBGL_VERSION == 1
    '$webgl_enable_ANGLE_instanced_arrays',
    '$webgl_enable_OES_vertex_array_object',
    '$webgl_enable_WEBGL_draw_buffers',
#endif
#if MAX_WEBGL_VERSION >= 2
    '$webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance',
    '$webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance',
#endif
    '$webgl_enable_EXT_polygon_offset_clamp',
    '$webgl_enable_EXT_clip_control',
    '$webgl_enable_WEBGL_polygon_mode',
    '$webgl_enable_WEBGL_multi_draw',
#endif
  ],
  emscripten_webgl_enable_extension__proxy: 'sync_on_webgl_context_handle_thread',
  emscripten_webgl_enable_extension: (contextHandle, extension) => {
    var context = GL.getContext(contextHandle);
    var extString = UTF8ToString(extension);
#if GL_EXTENSIONS_IN_PREFIXED_FORMAT
    if (extString.startsWith('GL_')) extString = extString.substr(3); // Allow enabling extensions both with "GL_" prefix and without.
#endif

#if GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS
    // Switch-board that pulls in code for all GL extensions, even if those are not used :/
    // Build with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0 to avoid this.

#if MIN_WEBGL_VERSION == 1
    // Obtain function entry points to WebGL 1 extension related functions.
    if (extString == 'ANGLE_instanced_arrays') webgl_enable_ANGLE_instanced_arrays(GLctx);
    if (extString == 'OES_vertex_array_object') webgl_enable_OES_vertex_array_object(GLctx);
    if (extString == 'WEBGL_draw_buffers') webgl_enable_WEBGL_draw_buffers(GLctx);
#endif

#if MAX_WEBGL_VERSION >= 2
    if (extString == 'WEBGL_draw_instanced_base_vertex_base_instance') webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);
    if (extString == 'WEBGL_multi_draw_instanced_base_vertex_base_instance') webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GLctx);
#endif

    if (extString == 'WEBGL_multi_draw') webgl_enable_WEBGL_multi_draw(GLctx);
    if (extString == 'EXT_polygon_offset_clamp') webgl_enable_EXT_polygon_offset_clamp(GLctx);
    if (extString == 'EXT_clip_control') webgl_enable_EXT_clip_control(GLctx);
    if (extString == 'WEBGL_polygon_mode') webgl_enable_WEBGL_polygon_mode(GLctx);

#elif ASSERTIONS || GL_ASSERTIONS
    if (['ANGLE_instanced_arrays',
         'OES_vertex_array_object',
         'WEBGL_draw_buffers',
         'WEBGL_multi_draw',
         'EXT_polygon_offset_clamp',
         'EXT_clip_control',
         'WEBGL_polygon_mode',
         'WEBGL_draw_instanced_base_vertex_base_instance',
         'WEBGL_multi_draw_instanced_base_vertex_base_instance'].includes(extString)) {
      err('When building with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0, function emscripten_webgl_enable_extension() cannot be used to enable extension '
                    + extString + '! Use one of the functions emscripten_webgl_enable_*() to enable it!');
    }
#endif

    var ext = context.GLctx.getExtension(extString);
    return !!ext;
  },

  emscripten_supports_offscreencanvas: () =>
    // TODO: Add a new build mode, e.g. OFFSCREENCANVAS_SUPPORT=2, which
    // necessitates OffscreenCanvas support at build time, and "return 1;" here in that build mode.
#if OFFSCREENCANVAS_SUPPORT
    typeof OffscreenCanvas != 'undefined'
#else
    0
#endif
  ,

  $registerWebGlEventCallback__deps: ['$JSEvents', '$findEventTarget'],
  $registerWebGlEventCallback: (target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) => {
#if PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif

#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    target ||= Module['canvas'];
#endif

    var webGlEventHandlerFunc = (e = event) => {
#if PTHREADS
      if (targetThread) __emscripten_run_callback_on_thread(targetThread, callbackfunc, eventTypeId, 0, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, 0, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString,
      callbackfunc,
      handlerFunc: webGlEventHandlerFunc,
      useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_webglcontextlost_callback_on_thread__proxy: 'sync',
  emscripten_set_webglcontextlost_callback_on_thread__deps: ['$registerWebGlEventCallback'],
  emscripten_set_webglcontextlost_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) => {
    registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST }}}, "webglcontextlost", targetThread);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_set_webglcontextrestored_callback_on_thread__proxy: 'sync',
  emscripten_set_webglcontextrestored_callback_on_thread__deps: ['$registerWebGlEventCallback'],
  emscripten_set_webglcontextrestored_callback_on_thread: (target, userData, useCapture, callbackfunc, targetThread) => {
    registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefs.EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED }}}, "webglcontextrestored", targetThread);
    return {{{ cDefs.EMSCRIPTEN_RESULT_SUCCESS }}};
  },

  emscripten_is_webgl_context_lost__proxy: 'sync_on_webgl_context_handle_thread',
  emscripten_is_webgl_context_lost: (contextHandle) => {
    return !GL.contexts[contextHandle] || GL.contexts[contextHandle].GLctx.isContextLost(); // No context ~> lost context.
  },

  emscripten_webgl_get_supported_extensions__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_supported_extensions__deps: ['$stringToNewUTF8'],
  // Here we report the full list of extensions supported by WebGL rather than
  // using getEmscriptenSupportedExtensions which filters the list based on
  // what is has explicit support in.
  emscripten_webgl_get_supported_extensions: () =>
    stringToNewUTF8(GLctx.getSupportedExtensions().join(' ')),

  emscripten_webgl_get_program_parameter_d__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_program_parameter_d: (program, param) =>
    GLctx.getProgramParameter(GL.programs[program], param),

  emscripten_webgl_get_program_info_log_utf8__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_program_info_log_utf8__deps: ['$stringToNewUTF8'],
  emscripten_webgl_get_program_info_log_utf8: (program) =>
    stringToNewUTF8(GLctx.getProgramInfoLog(GL.programs[program])),

  emscripten_webgl_get_shader_parameter_d__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_shader_parameter_d: (shader, param) =>
    GLctx.getShaderParameter(GL.shaders[shader], param),

  emscripten_webgl_get_shader_info_log_utf8__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_shader_info_log_utf8__deps: ['$stringToNewUTF8'],
  emscripten_webgl_get_shader_info_log_utf8: (shader) =>
    stringToNewUTF8(GLctx.getShaderInfoLog(GL.shaders[shader])),

  emscripten_webgl_get_shader_source_utf8__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_shader_source_utf8__deps: ['$stringToNewUTF8'],
  emscripten_webgl_get_shader_source_utf8: (shader) =>
    stringToNewUTF8(GLctx.getShaderSource(GL.shaders[shader])),

  emscripten_webgl_get_vertex_attrib_d__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_vertex_attrib_d: (index, param) =>
    GLctx.getVertexAttrib(index, param),

  emscripten_webgl_get_vertex_attrib_o__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_vertex_attrib_o: (index, param) => {
    var obj = GLctx.getVertexAttrib(index, param);
    return obj?.name;
  },

  emscripten_webgl_get_vertex_attrib_v__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_vertex_attrib_v__deps: ['$writeGLArray'],
  emscripten_webgl_get_vertex_attrib_v: (index, param, dst, dstLength, dstType) =>
    writeGLArray(GLctx.getVertexAttrib(index, param), dst, dstLength, dstType),

  emscripten_webgl_get_uniform_d__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_uniform_d__deps: ['$webglGetUniformLocation'],
  emscripten_webgl_get_uniform_d: (program, location) =>
    GLctx.getUniform(GL.programs[program], webglGetUniformLocation(location)),

  emscripten_webgl_get_uniform_v__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_uniform_v__deps: ['$writeGLArray', '$webglGetUniformLocation'],
  emscripten_webgl_get_uniform_v: (program, location, dst, dstLength, dstType) =>
    writeGLArray(GLctx.getUniform(GL.programs[program], webglGetUniformLocation(location)), dst, dstLength, dstType),

  emscripten_webgl_get_parameter_v__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_parameter_v__deps: ['$writeGLArray'],
  emscripten_webgl_get_parameter_v: (param, dst, dstLength, dstType) =>
    writeGLArray(GLctx.getParameter(param), dst, dstLength, dstType),

  emscripten_webgl_get_parameter_d__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_parameter_d: (param) => GLctx.getParameter(param),

  emscripten_webgl_get_parameter_o__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_parameter_o: (param) => {
    var obj = GLctx.getParameter(param);
    return obj?.name;
  },

  emscripten_webgl_get_parameter_utf8__deps: ['$stringToNewUTF8'],
  emscripten_webgl_get_parameter_utf8__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_parameter_utf8: (param) => stringToNewUTF8(GLctx.getParameter(param)),

  emscripten_webgl_get_parameter_i64v__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_parameter_i64v__deps: ['$writeI53ToI64'],
  emscripten_webgl_get_parameter_i64v: (param, dst) => writeI53ToI64(dst, GLctx.getParameter(param)),
};

function handleWebGLProxying(funcs) {
#if SHARED_MEMORY
// Process 'sync_on_webgl_context_handle_thread' and 'sync_on_current_webgl_context_thread' pseudo-proxying modes
// to appropriate proxying mechanism, either proxying on-demand, unconditionally, or never, depending on build modes.
// 'sync_on_webgl_context_handle_thread' is used for function signatures that take a HTML5 WebGL context handle
// object as the first argument. 'sync_on_current_webgl_context_thread' is used for functions that operate on the
// implicit "current WebGL context" as activated via emscripten_webgl_make_current() function.

  function listOfNFunctionArgs(func) {
    var args = [];
    for (var i = 0; i < func.length; ++i) {
      args.push('p' + i);
    }
    return args;
  }

  var targetingOffscreenCanvas, targetingOffscreenFramebuffer;
#if OFFSCREENCANVAS_SUPPORT
  targetingOffscreenCanvas = true;
#endif
#if OFFSCREEN_FRAMEBUFFER
  targetingOffscreenFramebuffer = true;
#endif

  for (var i in funcs) {
    // Is this a function that takes GL context handle as first argument?
    var proxyContextHandle = funcs[i + '__proxy'] == 'sync_on_webgl_context_handle_thread';

    // Is this a function that operates on the implicit current GL context object?
    var proxyCurrentContext = funcs[i + '__proxy'] == 'sync_on_current_webgl_context_thread';

    if (!proxyContextHandle && !proxyCurrentContext) {
      continue; // no resolving of pseudo-proxying needed for this function.
    }

    if (targetingOffscreenCanvas && (targetingOffscreenFramebuffer || proxyContextHandle)) {
      // Dynamically check at runtime whether the current thread owns the GL context handle/current GL context
      // object. If not, proxy the call to main thread.
      // TODO: this handles the calling pthread and main thread cases, but not yet the case from pthread->pthread.
      const sig = funcs[i + '__sig'] || LibraryManager.library[i + '__sig']
      assert(sig);
      funcs[i + '_calling_thread'] = funcs[i];
      funcs[i + '_main_thread'] = i + '_calling_thread';
      funcs[i + '_main_thread__proxy'] = 'sync';
      funcs[i + '_main_thread__sig'] = sig;
      funcs[i + '__deps'] ??= [];
      funcs[i + '__deps'].push(i + '_calling_thread');
      funcs[i + '__deps'].push(i + '_main_thread');
      delete funcs[i + '__proxy'];
      var funcArgs = listOfNFunctionArgs(funcs[i]);
      var funcArgsString = funcArgs.join(',');
      var retStatement = sig[0] != 'v' ? 'return' : '';
      var contextCheck = proxyContextHandle ? 'GL.contexts[p0]' : 'GLctx';
      var funcBody = `${retStatement} ${contextCheck} ? _${i}_calling_thread(${funcArgsString}) : _${i}_main_thread(${funcArgsString});`;
      if (funcs[i + '_before_on_calling_thread']) {
        funcs[i + '__deps'].push('$' + i + '_before_on_calling_thread');
        funcBody = `${i}_before_on_calling_thread(${funcArgsString}); ` + funcBody;
      }
      funcArgs.push(funcBody);
      funcs[i] = new (Function.prototype.bind.call(Function, Function, ...funcArgs));
    } else if (targetingOffscreenFramebuffer) {
      // When targeting only OFFSCREEN_FRAMEBUFFER, unconditionally proxy all GL calls to
      // main thread.
      funcs[i + '__proxy'] = 'sync';
    } else {
      // Building without OFFSCREENCANVAS_SUPPORT or OFFSCREEN_FRAMEBUFFER; or building
      // with OFFSCREENCANVAS_SUPPORT and no OFFSCREEN_FRAMEBUFFER: the application
      // will only utilize WebGL in the main browser thread, and in the calling thread.
      // Remove the WebGL proxying directives.
      delete funcs[i + '__proxy'];
    }
  }
#else
  // In single threaded mode just delete our custom __proxy addributes, otherwise
  // they will causes errors in the JS compiler.
  for (var i in funcs) {
    delete funcs[i + '__proxy'];
  }
#endif // SHARED_MEMORY
}

handleWebGLProxying(LibraryHtml5WebGL);

#if LibraryManager.has('library_webgl.js')
autoAddDeps(LibraryHtml5WebGL, '$GL');
#endif

addToLibrary(LibraryHtml5WebGL);
