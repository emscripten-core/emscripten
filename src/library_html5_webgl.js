/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryHtml5WebGL = {
  // Writes a JS typed array containing 32-bit floats or ints to memory
  $writeGLArray: function(arr, dst, dstLength, heapType) {
#if ASSERTIONS
    assert(arr);
    assert(typeof arr.length !== 'undefined');
#endif
    var len = arr.length;
    var writeLength = dstLength < len ? dstLength : len;
    var heap = heapType ? HEAPF32 : HEAP32;
    for(var i = 0; i < writeLength; ++i) {
      heap[(dst >> 2) + i] = arr[i];
    }
    return len;
  },

  // Execute in calling thread without proxying needed.
  emscripten_webgl_init_context_attributes: function(attributes) {
#if ASSERTIONS
    assert(attributes);
#endif
    var a = attributes >> 2;
    for(var i = 0; i < ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.__size__ }}}>>2); ++i) {
      HEAP32[a+i] = 0;
    }

    HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.alpha }}}>>2)] =
    HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.depth }}}>>2)] = 
    HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.antialias }}}>>2)] = 
    HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.premultipliedAlpha }}}>>2)] = 
    HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.majorVersion }}}>>2)] = 
    HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.enableExtensionsByDefault }}}>>2)] = 1;

#if USE_PTHREADS
    // Default context initialization state (user can override):
    // - if main thread is creating the context, default to the context not being shared between threads - enabling sharing has performance overhead, because it forces the context to be OffscreenCanvas or OffscreenFramebuffer.
    // - if a web worker is creating the context, default to using OffscreenCanvas if available, or proxying via Offscreen Framebuffer if not
    if (ENVIRONMENT_IS_WORKER) {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.proxyContextToMainThread, 1/*EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK*/, 'i32') }}};
#endif
  },

  _emscripten_webgl_power_preferences: "['default', 'low-power', 'high-performance']",

// In offscreen framebuffer mode, we implement these functions in C so that they enable
// the proxying of GL commands. Otherwise, they are implemented here in JS.
#if !(USE_PTHREADS && OFFSCREEN_FRAMEBUFFER)
  emscripten_webgl_create_context__sig: 'iii',
  emscripten_webgl_create_context: 'emscripten_webgl_do_create_context',

  emscripten_webgl_get_current_context__sig: 'i',
  emscripten_webgl_get_current_context: 'emscripten_webgl_do_get_current_context',

  emscripten_webgl_commit_frame__sig: 'i',
  emscripten_webgl_commit_frame: 'emscripten_webgl_do_commit_frame',
#endif

  // This code is called from the main proxying logic, which has a big switch
  // for all the messages, one of which is this GL-using one. This won't be
  // called if GL is not linked in, but also make sure to not add a dep on
  // GL unnecessarily from here, as that would cause a linker error.
  emscripten_webgl_do_create_context__deps: [
#if LibraryManager.has('library_webgl.js')
  '$GL',
#endif
  '$JSEvents', '_emscripten_webgl_power_preferences', '$findEventTarget', '$findCanvasEventTarget'],
  // This function performs proxying manually, depending on the style of context that is to be created.
  emscripten_webgl_do_create_context: function(target, attributes) {
#if ASSERTIONS
    assert(attributes);
#endif
    var contextAttributes = {};
    var a = attributes >> 2;
    contextAttributes['alpha'] = !!HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.alpha }}}>>2)];
    contextAttributes['depth'] = !!HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.depth }}}>>2)];
    contextAttributes['stencil'] = !!HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.stencil }}}>>2)];
    contextAttributes['antialias'] = !!HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.antialias }}}>>2)];
    contextAttributes['premultipliedAlpha'] = !!HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.premultipliedAlpha }}}>>2)];
    contextAttributes['preserveDrawingBuffer'] = !!HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.preserveDrawingBuffer }}}>>2)];
    var powerPreference = HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.powerPreference }}}>>2)];
    contextAttributes['powerPreference'] = __emscripten_webgl_power_preferences[powerPreference];
    contextAttributes['failIfMajorPerformanceCaveat'] = !!HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.failIfMajorPerformanceCaveat }}}>>2)];
    contextAttributes.majorVersion = HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.majorVersion }}}>>2)];
    contextAttributes.minorVersion = HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.minorVersion }}}>>2)];
    contextAttributes.enableExtensionsByDefault = HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.enableExtensionsByDefault }}}>>2)];
    contextAttributes.explicitSwapControl = HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.explicitSwapControl }}}>>2)];
    contextAttributes.proxyContextToMainThread = HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.proxyContextToMainThread }}}>>2)];
    contextAttributes.renderViaOffscreenBackBuffer = HEAP32[a + ({{{ C_STRUCTS.EmscriptenWebGLContextAttributes.renderViaOffscreenBackBuffer }}}>>2)];

    var canvas = findCanvasEventTarget(target);

#if GL_DEBUG
    var targetStr = UTF8ToString(target);
#endif

#if (USE_PTHREADS && OFFSCREEN_FRAMEBUFFER)
    // Create a WebGL context that is proxied to main thread if canvas was not found on worker, or if explicitly requested to do so.
    if (ENVIRONMENT_IS_PTHREAD) {
      if (contextAttributes.proxyContextToMainThread === {{{ cDefine('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS') }}} ||
         (!canvas && contextAttributes.proxyContextToMainThread === {{{ cDefine('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK') }}})) {
        // When WebGL context is being proxied via the main thread, we must render using an offscreen FBO render target to avoid WebGL's
        // "implicit swap when callback exits" behavior. TODO: If OffscreenCanvas is supported, explicitSwapControl=true and still proxying,
        // then this can be avoided, since OffscreenCanvas enables explicit swap control.
#if GL_DEBUG
        if (contextAttributes.proxyContextToMainThread === {{{ cDefine('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS') }}}) console.error('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS enabled, proxying WebGL rendering from pthread to main thread.');
        if (!canvas && contextAttributes.proxyContextToMainThread === {{{ cDefine('EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK') }}}) console.error('Specified canvas target "' + targetStr + '" is not an OffscreenCanvas in the current pthread, but EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK is set. Proxying WebGL rendering from pthread to main thread.');
        console.error('Performance warning: forcing renderViaOffscreenBackBuffer=true and preserveDrawingBuffer=true since proxying WebGL rendering.');
#endif
        // We will be proxying - if OffscreenCanvas is supported, we can proxy a bit more efficiently by avoiding having to create an Offscreen FBO.
        if (typeof OffscreenCanvas === 'undefined') {
          {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.renderViaOffscreenBackBuffer, '1', 'i32') }}}
          {{{ makeSetValue('attributes', C_STRUCTS.EmscriptenWebGLContextAttributes.preserveDrawingBuffer, '1', 'i32') }}}
        }
        return _emscripten_sync_run_in_main_thread_2({{{ cDefine('EM_PROXIED_CREATE_CONTEXT') }}}, target, attributes);
      }
    }
#endif

    if (!canvas) {
#if GL_DEBUG
      console.error('emscripten_webgl_create_context failed: Unknown canvas target "' + targetStr + '"!');
#endif
      return 0;
    }

#if OFFSCREENCANVAS_SUPPORT
    if (canvas.offscreenCanvas) canvas = canvas.offscreenCanvas;

#if GL_DEBUG
    if (typeof OffscreenCanvas !== 'undefined' && canvas instanceof OffscreenCanvas) console.log('emscripten_webgl_create_context: Creating an OffscreenCanvas-based WebGL context on target "' + targetStr + '"');
    else if (typeof HTMLCanvasElement !== 'undefined' && canvas instanceof HTMLCanvasElement) console.log('emscripten_webgl_create_context: Creating an HTMLCanvasElement-based WebGL context on target "' + targetStr + '"');
#endif

    if (contextAttributes.explicitSwapControl) {
      var supportsOffscreenCanvas = canvas.transferControlToOffscreen || (typeof OffscreenCanvas !== 'undefined' && canvas instanceof OffscreenCanvas);

      if (!supportsOffscreenCanvas) {
#if OFFSCREEN_FRAMEBUFFER
        if (!contextAttributes.renderViaOffscreenBackBuffer) {
          contextAttributes.renderViaOffscreenBackBuffer = true;
#if GL_DEBUG
          console.error('emscripten_webgl_create_context: Performance warning, OffscreenCanvas is not supported but explicitSwapControl was requested, so force-enabling renderViaOffscreenBackBuffer=true to allow explicit swapping!');
#endif
        }
#else
#if GL_DEBUG
        console.error('emscripten_webgl_create_context failed: OffscreenCanvas is not supported but explicitSwapControl was requested!');
#endif
        return 0;
#endif
      }

      if (canvas.transferControlToOffscreen) {
#if GL_DEBUG
        console.log('explicitSwapControl requested: canvas.transferControlToOffscreen() on canvas "' + targetStr + '" to get .commit() function and not rely on implicit WebGL swap');
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
          console.error('OffscreenCanvas is supported, and canvas "' + canvas.id + '" has already before been transferred offscreen, but there is no known OffscreenCanvas with that name!');
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
      console.error('emscripten_webgl_create_context: Performance warning, not building with OffscreenCanvas support enabled but explicitSwapControl was requested, so force-enabling renderViaOffscreenBackBuffer=true to allow explicit swapping!');
#endif
    }
#else
    if (contextAttributes.explicitSwapControl) {
#if GL_DEBUG
      console.error('emscripten_webgl_create_context failed: explicitSwapControl is not supported, please rebuild with -s OFFSCREENCANVAS_SUPPORT=1 to enable targeting the experimental OffscreenCanvas specification, or rebuild with -s OFFSCREEN_FRAMEBUFFER=1 to emulate explicitSwapControl in the absence of OffscreenCanvas support!');
#endif
      return 0;
    }
#endif // ~!OFFSCREEN_FRAMEBUFFER

#endif // ~!OFFSCREENCANVAS_SUPPORT

    var contextHandle = GL.createContext(canvas, contextAttributes);
    return contextHandle;
  },
#if USE_PTHREADS && OFFSCREEN_FRAMEBUFFER
  // Runs on the calling thread, proxies if needed.
  emscripten_webgl_make_context_current_calling_thread: function(contextHandle) {
    var success = GL.makeContextCurrent(contextHandle);
    if (success) GL.currentContextIsProxied = false; // If succeeded above, we will have a local GL context from this thread (worker or main).
    return success ? {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}} : {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
  },
  // This function gets called in a pthread, after it has successfully activated (with make_current()) a proxied GL context to itself from the main thread.
  // In this scenario, the pthread does not hold a high-level JS object to the GL context, because it lives on the main thread, in which case we record
  // an integer pointer as a token value to represent the GL context activation from another thread. (when this function is called, the main browser thread
  // has already accepted the GL context activation for our pthread, so that side is good)
  _emscripten_proxied_gl_context_activated_from_main_browser_thread: function(contextHandle) {
    GLctx = Module.ctx = GL.currentContext = contextHandle;
    GL.currentContextIsProxied = true;
  },
#else
  emscripten_webgl_make_context_current: function(contextHandle) {
    var success = GL.makeContextCurrent(contextHandle);
    return success ? {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}} : {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
  },
#endif

  emscripten_webgl_do_get_current_context: function() {
    return GL.currentContext ? GL.currentContext.handle : 0;
  },

  emscripten_webgl_get_drawing_buffer_size__proxy: 'sync_on_webgl_context_handle_thread',
  emscripten_webgl_get_drawing_buffer_size__sig: 'iiii',
  emscripten_webgl_get_drawing_buffer_size: function(contextHandle, width, height) {
    var GLContext = GL.getContext(contextHandle);

    if (!GLContext || !GLContext.GLctx || !width || !height) {
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
    }
    {{{ makeSetValue('width', '0', 'GLContext.GLctx.drawingBufferWidth', 'i32') }}};
    {{{ makeSetValue('height', '0', 'GLContext.GLctx.drawingBufferHeight', 'i32') }}};
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_webgl_do_commit_frame: function() {
#if TRACE_WEBGL_CALLS
    var threadId = (typeof _pthread_self !== 'undefined') ? _pthread_self : function() { return 1; };
    console.error('[Thread ' + threadId() + ', GL ctx: ' + GL.currentContext.handle + ']: emscripten_webgl_do_commit_frame()');
#endif
    if (!GL.currentContext || !GL.currentContext.GLctx) {
#if GL_DEBUG
      console.error('emscripten_webgl_commit_frame() failed: no GL context set current via emscripten_webgl_make_context_current()!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }

#if OFFSCREEN_FRAMEBUFFER
    if (GL.currentContext.defaultFbo) {
      GL.blitOffscreenFramebuffer(GL.currentContext);
#if GL_DEBUG && OFFSCREENCANVAS_SUPPORT
      if (GL.currentContext.GLctx.commit) console.error('emscripten_webgl_commit_frame(): Offscreen framebuffer should never have gotten created when canvas is in OffscreenCanvas mode, since it is redundant and not necessary');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    }
#endif
    if (!GL.currentContext.attributes.explicitSwapControl) {
#if GL_DEBUG
      console.error('emscripten_webgl_commit_frame() cannot be called for canvases with implicit swap control mode!');
#endif
      return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    }
    // We would do GL.currentContext.GLctx.commit(); here, but the current implementation
    // in browsers has removed it - swap is implicit, so this function is a no-op for now
    // (until/unless the spec changes).
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_webgl_get_context_attributes__proxy: 'sync_on_webgl_context_handle_thread',
  emscripten_webgl_get_context_attributes__sig: 'iii',
  emscripten_webgl_get_context_attributes__deps: ['_emscripten_webgl_power_preferences'],
  emscripten_webgl_get_context_attributes: function(c, a) {
    if (!a) return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_PARAM') }}};
    c = GL.contexts[c];
    if (!c) return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    var t = c.GLctx;
    if (!t) return {{{ cDefine('EMSCRIPTEN_RESULT_INVALID_TARGET') }}};
    t = t.getContextAttributes();

    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.alpha, 't.alpha', 'i32') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.depth, 't.depth', 'i32') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.stencil, 't.stencil', 'i32') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.antialias, 't.antialias', 'i32') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.premultipliedAlpha, 't.premultipliedAlpha', 'i32') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.preserveDrawingBuffer, 't.preserveDrawingBuffer', 'i32') }}};
    var power = t['powerPreference'] && __emscripten_webgl_power_preferences.indexOf(t['powerPreference']);
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.powerPreference, 'power', 'i32') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.failIfMajorPerformanceCaveat, 't.failIfMajorPerformanceCaveat', 'i32') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.majorVersion, 'c.version', 'i32') }}};
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.minorVersion, 0, 'i32') }}};
#if GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.enableExtensionsByDefault, 'c.attributes.enableExtensionsByDefault', 'i32') }}};
#endif
#if GL_SUPPORT_EXPLICIT_SWAP_CONTROL
    {{{ makeSetValue('a', C_STRUCTS.EmscriptenWebGLContextAttributes.explicitSwapControl, 'c.attributes.explicitSwapControl', 'i32') }}};
#endif
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_webgl_destroy_context__proxy: 'sync_on_webgl_context_handle_thread',
  emscripten_webgl_destroy_context__sig: 'vi',
  emscripten_webgl_destroy_context__deps: ['emscripten_webgl_get_current_context', 'emscripten_webgl_make_context_current'],
  emscripten_webgl_destroy_context: function(contextHandle) {
    if (GL.currentContext == contextHandle) GL.currentContext = 0;
    GL.deleteContext(contextHandle);
  },

  // Special function that will be invoked on the thread calling emscripten_webgl_destroy_context(), before routing
  // the call over to the target thread.
  emscripten_webgl_destroy_context_before_on_calling_thread: function(contextHandle) {
    if (_emscripten_webgl_get_current_context() == contextHandle) _emscripten_webgl_make_context_current(0);
  },

  emscripten_webgl_enable_extension__deps: [
#if GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS
#if MIN_WEBGL_VERSION == 1
    '_webgl_enable_ANGLE_instanced_arrays',
    '_webgl_enable_OES_vertex_array_object',
    '_webgl_enable_WEBGL_draw_buffers',
#endif
#if MAX_WEBGL_VERSION >= 2
    '_webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance',
#endif
    '_webgl_enable_WEBGL_multi_draw',
#endif
  ],
  emscripten_webgl_enable_extension__proxy: 'sync_on_webgl_context_handle_thread',
  emscripten_webgl_enable_extension__sig: 'iii',
  emscripten_webgl_enable_extension: function(contextHandle, extension) {
    var context = GL.getContext(contextHandle);
    var extString = UTF8ToString(extension);
#if GL_EXTENSIONS_IN_PREFIXED_FORMAT
    if (extString.indexOf('GL_') == 0) extString = extString.substr(3); // Allow enabling extensions both with "GL_" prefix and without.
#endif

#if GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS
    // Switch-board that pulls in code for all GL extensions, even if those are not used :/
    // Build with -s GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS = 0 to avoid this.

#if MIN_WEBGL_VERSION == 1
    // Obtain function entry points to WebGL 1 extension related functions.
    if (extString == 'ANGLE_instanced_arrays') __webgl_enable_ANGLE_instanced_arrays(GLctx);
    if (extString == 'OES_vertex_array_object') __webgl_enable_OES_vertex_array_object(GLctx);
    if (extString == 'WEBGL_draw_buffers') __webgl_enable_WEBGL_draw_buffers(GLctx);
#endif

#if MAX_WEBGL_VERSION >= 2
    if (extString == 'WEBGL_draw_instanced_base_vertex_base_instance') __webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);
#endif

    if (extString == 'WEBGL_multi_draw') __webgl_enable_WEBGL_multi_draw(GLctx);

#else

#if ASSERTIONS || GL_ASSERTIONS
    if (['ANGLE_instanced_arrays',
         'OES_vertex_array_object',
         'WEBGL_draw_buffers',
         'WEBGL_multi_draw',
         'WEBGL_draw_instanced_base_vertex_base_instance'].indexOf(extString) >= 0) {
      console.error('When building with -s GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=0, function emscripten_webgl_enable_extension() cannot be used to enable extension '
                    + extString + '! Use one of the functions emscripten_webgl_enable_*() to enable it!');
    }
#endif

#endif

    var ext = context.GLctx.getExtension(extString);
    return !!ext;
  },

  emscripten_supports_offscreencanvas: function() {
    // TODO: Add a new build mode, e.g. OFFSCREENCANVAS_SUPPORT=2, which
    // necessitates OffscreenCanvas support at build time, and "return 1;" here in that build mode.
#if OFFSCREENCANVAS_SUPPORT
    return typeof OffscreenCanvas !== 'undefined';
#else
    return 0;
#endif
  },

  _registerWebGlEventCallback__deps: ['$JSEvents', '$findEventTarget'],
  _registerWebGlEventCallback: function(target, userData, useCapture, callbackfunc, eventTypeId, eventTypeString, targetThread) {
#if USE_PTHREADS
    targetThread = JSEvents.getTargetThreadForEventCallback(targetThread);
#endif

#if !DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR
    if (!target) target = Module['canvas'];
#endif

    var webGlEventHandlerFunc = function(ev) {
      var e = ev || event;

#if USE_PTHREADS
      if (targetThread) JSEvents.queueEventHandlerOnThread_iiii(targetThread, callbackfunc, eventTypeId, 0, userData);
      else
#endif
      if ({{{ makeDynCall('iiii', 'callbackfunc') }}}(eventTypeId, 0, userData)) e.preventDefault();
    };

    var eventHandler = {
      target: findEventTarget(target),
      eventTypeString: eventTypeString,
      callbackfunc: callbackfunc,
      handlerFunc: webGlEventHandlerFunc,
      useCapture: useCapture
    };
    JSEvents.registerOrRemoveHandler(eventHandler);
  },

  emscripten_set_webglcontextlost_callback_on_thread__proxy: 'sync',
  emscripten_set_webglcontextlost_callback_on_thread__sig: 'iiiiii',
  emscripten_set_webglcontextlost_callback_on_thread__deps: ['_registerWebGlEventCallback'],
  emscripten_set_webglcontextlost_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST') }}}, "webglcontextlost", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_set_webglcontextrestored_callback_on_thread__proxy: 'sync',
  emscripten_set_webglcontextrestored_callback_on_thread__sig: 'iiiiii',
  emscripten_set_webglcontextrestored_callback_on_thread__deps: ['_registerWebGlEventCallback'],
  emscripten_set_webglcontextrestored_callback_on_thread: function(target, userData, useCapture, callbackfunc, targetThread) {
    __registerWebGlEventCallback(target, userData, useCapture, callbackfunc, {{{ cDefine('EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED') }}}, "webglcontextrestored", targetThread);
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_is_webgl_context_lost__proxy: 'sync_on_webgl_context_handle_thread',
  emscripten_is_webgl_context_lost__sig: 'ii',
  emscripten_is_webgl_context_lost: function(contextHandle) {
    return !GL.contexts[contextHandle] || GL.contexts[contextHandle].GLctx.isContextLost(); // No context ~> lost context.
  },

  emscripten_webgl_get_supported_extensions__sig: 'i',
  emscripten_webgl_get_supported_extensions__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_supported_extensions__deps: ['$stringToNewUTF8'],
  emscripten_webgl_get_supported_extensions: function() {
    return stringToNewUTF8(GLctx.getSupportedExtensions().join(' '));
  },

  emscripten_webgl_get_program_parameter_d__sig: 'fii',
  emscripten_webgl_get_program_parameter_d__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_program_parameter_d: function(program, param) {
    return GLctx.getProgramParameter(GL.programs[program], param);
  },

  emscripten_webgl_get_program_info_log_utf8__sig: 'ii',
  emscripten_webgl_get_program_info_log_utf8__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_program_info_log_utf8__deps: ['$stringToNewUTF8'],
  emscripten_webgl_get_program_info_log_utf8: function(program) {
    return stringToNewUTF8(GLctx.getProgramInfoLog(GL.programs[program]));
  },

  emscripten_webgl_get_shader_parameter_d__sig: 'fii',
  emscripten_webgl_get_shader_parameter_d__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_shader_parameter_d: function(shader, param) {
    return GLctx.getShaderParameter(GL.shaders[shader], param);
  },

  emscripten_webgl_get_shader_info_log_utf8__sig: 'ii',
  emscripten_webgl_get_shader_info_log_utf8__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_shader_info_log_utf8__deps: ['$stringToNewUTF8'],
  emscripten_webgl_get_shader_info_log_utf8: function(shader) {
    return stringToNewUTF8(GLctx.getShaderInfoLog(GL.shaders[shader]));
  },

  emscripten_webgl_get_shader_source_utf8__sig: 'ii',
  emscripten_webgl_get_shader_source_utf8__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_shader_source_utf8__deps: ['$stringToNewUTF8'],
  emscripten_webgl_get_shader_source_utf8: function(shader) {
    return stringToNewUTF8(GLctx.getShaderSource(GL.shaders[shader]));
  },

  emscripten_webgl_get_vertex_attrib_d__sig: 'iii',
  emscripten_webgl_get_vertex_attrib_d__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_vertex_attrib_d: function(index, param) {
    return GLctx.getVertexAttrib(index, param);
  },

  emscripten_webgl_get_vertex_attrib_o__sig: 'iii',
  emscripten_webgl_get_vertex_attrib_o__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_vertex_attrib_o: function(index, param) {
    var obj = GLctx.getVertexAttrib(index, param);
    return obj && obj.name;
  },

  emscripten_webgl_get_vertex_attrib_v__sig: 'iiiiii',
  emscripten_webgl_get_vertex_attrib_v__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_vertex_attrib_v__deps: ['$writeGLArray'],
  emscripten_webgl_get_vertex_attrib_v: function(index, param, dst, dstLength, dstType) {
    return writeGLArray(GLctx.getVertexAttrib(index, param), dst, dstLength, dstType);
  },

  emscripten_webgl_get_uniform_d__sig: 'fii',
  emscripten_webgl_get_uniform_d__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_uniform_d: function(program, location) {
    return GLctx.getUniform(GL.programs[program], GL.uniforms[location]);
  },

  emscripten_webgl_get_uniform_v__sig: 'iiiiii',
  emscripten_webgl_get_uniform_v__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_uniform_v__deps: ['$writeGLArray'],
  emscripten_webgl_get_uniform_v: function(program, location, dst, dstLength, dstType) {
    return writeGLArray(GLctx.getUniform(GL.programs[program], GL.uniforms[location]), dst, dstLength, dstType);
  },

  emscripten_webgl_get_parameter_v__sig: 'iiiii',
  emscripten_webgl_get_parameter_v__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_parameter_v__deps: ['$writeGLArray'],
  emscripten_webgl_get_parameter_v: function(param, dst, dstLength, dstType) {
    return writeGLArray(GLctx.getParameter(param), dst, dstLength, dstType);
  },

  emscripten_webgl_get_parameter_d__sig: 'fi',
  emscripten_webgl_get_parameter_d__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_parameter_d: function(param) {
    return GLctx.getParameter(param);
  },

  emscripten_webgl_get_parameter_o__sig: 'ii',
  emscripten_webgl_get_parameter_o__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_parameter_o: function(param) {
    var obj = GLctx.getParameter(param);
    return obj && obj.name;
  },

  emscripten_webgl_get_parameter_utf8__sig: 'ii',
  emscripten_webgl_get_parameter_utf8__deps: ['$stringToNewUTF8'],
  emscripten_webgl_get_parameter_utf8__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_parameter_utf8: function(param) {
    return stringToNewUTF8(GLctx.getParameter(param));
  },

  emscripten_webgl_get_parameter_i64v__sig: 'vii',
  emscripten_webgl_get_parameter_i64v__proxy: 'sync_on_current_webgl_context_thread',
  emscripten_webgl_get_parameter_i64v__deps: ['$writeI53ToI64'],
  emscripten_webgl_get_parameter_i64v: function(param, dst) {
    writeI53ToI64(dst, GLctx.getParameter(param));
  },
};

// Process 'sync_on_webgl_context_handle_thread' and 'sync_on_current_webgl_context_thread' pseudo-proxying modes
// to appropriate proxying mechanism, either proxying on-demand, unconditionally, or never, depending on build modes.
// 'sync_on_webgl_context_handle_thread' is used for function signatures that take a HTML5 WebGL context handle
// object as the first argument. 'sync_on_current_webgl_context_thread' is used for functions that operate on the
// implicit "current WebGL context" as activated via emscripten_webgl_make_current() function.
function handleWebGLProxying(funcs) {
  if (!USE_PTHREADS) return; // No proxying needed in singlethreaded builds

  function listOfNFunctionArgs(func) {
    var args = [];
    for(var i = 0; i < func.length; ++i) {
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

  for(var i in funcs) {
    // Is this a function that takes GL context handle as first argument?
    var proxyContextHandle = funcs[i + '__proxy'] == 'sync_on_webgl_context_handle_thread';

    // Is this a function that operates on the implict current GL context object?
    var proxyCurrentContext = funcs[i + '__proxy'] == 'sync_on_current_webgl_context_thread';

    if (!proxyContextHandle && !proxyCurrentContext) {
      continue; // no resolving of pseudo-proxying needed for this function.
    }

    if (targetingOffscreenCanvas && (targetingOffscreenFramebuffer || proxyContextHandle)) {
      // Dynamically check at runtime whether the current thread owns the GL context handle/current GL context
      // object. If not, proxy the call to main thread.
      // TODO: this handles the calling pthread and main thread cases, but not yet the case from pthread->pthread.
      funcs[i + '_calling_thread'] = funcs[i];
      funcs[i + '_main_thread'] = i + '_calling_thread';
      funcs[i + '_main_thread__proxy'] = 'sync';
      funcs[i + '_main_thread__sig'] = funcs[i + '__sig'];
      if (!funcs[i + '__deps']) funcs[i + '__deps'] = [];
      funcs[i + '__deps'].push(i + '_calling_thread');
      funcs[i + '__deps'].push(i + '_main_thread');
      delete funcs[i + '__proxy'];
      var funcArgs = listOfNFunctionArgs(funcs[i]);
      var funcArgsString = funcArgs.join(',');
      var retStatement = funcs[i + '__sig'][0] != 'v' ? 'return' : '';
      var contextCheck = proxyContextHandle ? 'GL.contexts[p0]' : 'GLctx';
      var funcBody = `${retStatement} ${contextCheck} ? _${i}_calling_thread(${funcArgsString}) : _${i}_main_thread(${funcArgsString});`;
      if (funcs[i + '_before_on_calling_thread']) {
        funcs[i + '__deps'].push(i + '_before_on_calling_thread');
        funcBody = `_${i}_before_on_calling_thread(${funcArgsString}); ` + funcBody;
      }
      funcArgs.push(funcBody);
      funcs[i] = new (Function.prototype.bind.apply(Function, [Function].concat(funcArgs)));
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
}

handleWebGLProxying(LibraryHtml5WebGL);

mergeInto(LibraryManager.library, LibraryHtml5WebGL);
