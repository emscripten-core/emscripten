/**
 * @license
 * Copyright 2011 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Utilities for browser environments
var LibraryBrowser = {
  $Browser__deps: [
    '$callUserCallback',
    '$safeSetTimeout',
    '$warnOnce',
#if FILESYSTEM
    '$preloadPlugins',
#if MAIN_MODULE
    '$preloadedWasm',
#endif
#endif
  ],
  $Browser__postset: `
    // exports
    Module["requestFullscreen"] = Browser.requestFullscreen;
#if ASSERTIONS
    Module["requestFullScreen"] = Browser.requestFullScreen;
#endif
    Module["setCanvasSize"] = Browser.setCanvasSize;
    Module["getUserMedia"] = Browser.getUserMedia;
    Module["createContext"] = Browser.createContext;
    var preloadedImages = {};
    var preloadedAudios = {};`,

  $Browser: {
    useWebGL: false,
    isFullscreen: false,
    pointerLock: false,
    moduleContextCreatedCallbacks: [],
    workers: [],

    init() {
      if (Browser.initted) return;
      Browser.initted = true;

#if FILESYSTEM
      // Support for plugins that can process preloaded files. You can add more of these to
      // your app by creating and appending to preloadPlugins.
      //
      // Each plugin is asked if it can handle a file based on the file's name. If it can,
      // it is given the file's raw data. When it is done, it calls a callback with the file's
      // (possibly modified) data. For example, a plugin might decompress a file, or it
      // might create some side data structure for use later (like an Image element, etc.).

      var imagePlugin = {};
      imagePlugin['canHandle'] = function imagePlugin_canHandle(name) {
        return !Module['noImageDecoding'] && /\.(jpg|jpeg|png|bmp|webp)$/i.test(name);
      };
      imagePlugin['handle'] = function imagePlugin_handle(byteArray, name, onload, onerror) {
        var b = new Blob([byteArray], { type: Browser.getMimetype(name) });
        if (b.size !== byteArray.length) { // Safari bug #118630
          // Safari's Blob can only take an ArrayBuffer
          b = new Blob([(new Uint8Array(byteArray)).buffer], { type: Browser.getMimetype(name) });
        }
        var url = URL.createObjectURL(b);
#if ASSERTIONS
        assert(typeof url == 'string', 'createObjectURL must return a url as a string');
#endif
        var img = new Image();
        img.onload = () => {
#if ASSERTIONS
          assert(img.complete, `Image ${name} could not be decoded`);
#endif
          var canvas = /** @type {!HTMLCanvasElement} */ (document.createElement('canvas'));
          canvas.width = img.width;
          canvas.height = img.height;
          var ctx = canvas.getContext('2d');
          ctx.drawImage(img, 0, 0);
          preloadedImages[name] = canvas;
          URL.revokeObjectURL(url);
          onload?.(byteArray);
        };
        img.onerror = (event) => {
          err(`Image ${url} could not be decoded`);
          onerror?.();
        };
        img.src = url;
      };
      preloadPlugins.push(imagePlugin);

      var audioPlugin = {};
      audioPlugin['canHandle'] = function audioPlugin_canHandle(name) {
        return !Module['noAudioDecoding'] && name.substr(-4) in { '.ogg': 1, '.wav': 1, '.mp3': 1 };
      };
      audioPlugin['handle'] = function audioPlugin_handle(byteArray, name, onload, onerror) {
        var done = false;
        function finish(audio) {
          if (done) return;
          done = true;
          preloadedAudios[name] = audio;
          onload?.(byteArray);
        }
        function fail() {
          if (done) return;
          done = true;
          preloadedAudios[name] = new Audio(); // empty shim
          onerror?.();
        }
        var b = new Blob([byteArray], { type: Browser.getMimetype(name) });
        var url = URL.createObjectURL(b); // XXX we never revoke this!
#if ASSERTIONS
        assert(typeof url == 'string', 'createObjectURL must return a url as a string');
#endif
        var audio = new Audio();
        audio.addEventListener('canplaythrough', () => finish(audio), false); // use addEventListener due to chromium bug 124926
        audio.onerror = function audio_onerror(event) {
          if (done) return;
          err(`warning: browser could not fully decode audio ${name}, trying slower base64 approach`);
          function encode64(data) {
            var BASE = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
            var PAD = '=';
            var ret = '';
            var leftchar = 0;
            var leftbits = 0;
            for (var i = 0; i < data.length; i++) {
              leftchar = (leftchar << 8) | data[i];
              leftbits += 8;
              while (leftbits >= 6) {
                var curr = (leftchar >> (leftbits-6)) & 0x3f;
                leftbits -= 6;
                ret += BASE[curr];
              }
            }
            if (leftbits == 2) {
              ret += BASE[(leftchar&3) << 4];
              ret += PAD + PAD;
            } else if (leftbits == 4) {
              ret += BASE[(leftchar&0xf) << 2];
              ret += PAD;
            }
            return ret;
          }
          audio.src = 'data:audio/x-' + name.substr(-3) + ';base64,' + encode64(byteArray);
          finish(audio); // we don't wait for confirmation this worked - but it's worth trying
        };
        audio.src = url;
        // workaround for chrome bug 124926 - we do not always get oncanplaythrough or onerror
        safeSetTimeout(() => {
          finish(audio); // try to use it even though it is not necessarily ready to play
        }, 10000);
      };
      preloadPlugins.push(audioPlugin);
#endif

      // Canvas event setup

      function pointerLockChange() {
        Browser.pointerLock = document['pointerLockElement'] === Module['canvas'] ||
                              document['mozPointerLockElement'] === Module['canvas'] ||
                              document['webkitPointerLockElement'] === Module['canvas'] ||
                              document['msPointerLockElement'] === Module['canvas'];
      }
      var canvas = Module['canvas'];
      if (canvas) {
        // forced aspect ratio can be enabled by defining 'forcedAspectRatio' on Module
        // Module['forcedAspectRatio'] = 4 / 3;

        canvas.requestPointerLock = canvas['requestPointerLock'] ||
                                    canvas['mozRequestPointerLock'] ||
                                    canvas['webkitRequestPointerLock'] ||
                                    canvas['msRequestPointerLock'] ||
                                    (() => {});
        canvas.exitPointerLock = document['exitPointerLock'] ||
                                 document['mozExitPointerLock'] ||
                                 document['webkitExitPointerLock'] ||
                                 document['msExitPointerLock'] ||
                                 (() => {}); // no-op if function does not exist
        canvas.exitPointerLock = canvas.exitPointerLock.bind(document);

        document.addEventListener('pointerlockchange', pointerLockChange, false);
        document.addEventListener('mozpointerlockchange', pointerLockChange, false);
        document.addEventListener('webkitpointerlockchange', pointerLockChange, false);
        document.addEventListener('mspointerlockchange', pointerLockChange, false);

        if (Module['elementPointerLock']) {
          canvas.addEventListener("click", (ev) => {
            if (!Browser.pointerLock && Module['canvas'].requestPointerLock) {
              Module['canvas'].requestPointerLock();
              ev.preventDefault();
            }
          }, false);
        }
      }
    },

    createContext(/** @type {HTMLCanvasElement} */ canvas, useWebGL, setInModule, webGLContextAttributes) {
      if (useWebGL && Module.ctx && canvas == Module.canvas) return Module.ctx; // no need to recreate GL context if it's already been created for this canvas.

      var ctx;
      var contextHandle;
      if (useWebGL) {
        // For GLES2/desktop GL compatibility, adjust a few defaults to be different to WebGL defaults, so that they align better with the desktop defaults.
        var contextAttributes = {
          antialias: false,
          alpha: false,
#if MIN_WEBGL_VERSION >= 2
          majorVersion: 2,
#elif MAX_WEBGL_VERSION >= 2 // library_browser.js defaults: use the WebGL version chosen at compile time (unless overridden below)
          majorVersion: (typeof WebGL2RenderingContext != 'undefined') ? 2 : 1,
#else
          majorVersion: 1,
#endif
        };

        if (webGLContextAttributes) {
          for (var attribute in webGLContextAttributes) {
            contextAttributes[attribute] = webGLContextAttributes[attribute];
          }
        }

        // This check of existence of GL is here to satisfy Closure compiler, which yells if variable GL is referenced below but GL object is not
        // actually compiled in because application is not doing any GL operations. TODO: Ideally if GL is not being used, this function
        // Browser.createContext() should not even be emitted.
        if (typeof GL != 'undefined') {
          contextHandle = GL.createContext(canvas, contextAttributes);
          if (contextHandle) {
            ctx = GL.getContext(contextHandle).GLctx;
          }
        }
      } else {
        ctx = canvas.getContext('2d');
      }

      if (!ctx) return null;

      if (setInModule) {
#if ASSERTIONS
        if (!useWebGL) assert(typeof GLctx == 'undefined', 'cannot set in module if GLctx is used, but we are a non-GL context that would replace it');
#endif
        Module.ctx = ctx;
        if (useWebGL) GL.makeContextCurrent(contextHandle);
        Browser.useWebGL = useWebGL;
        Browser.moduleContextCreatedCallbacks.forEach((callback) => callback());
        Browser.init();
      }
      return ctx;
    },

    fullscreenHandlersInstalled: false,
    lockPointer: undefined,
    resizeCanvas: undefined,
    requestFullscreen(lockPointer, resizeCanvas) {
      Browser.lockPointer = lockPointer;
      Browser.resizeCanvas = resizeCanvas;
      if (typeof Browser.lockPointer == 'undefined') Browser.lockPointer = true;
      if (typeof Browser.resizeCanvas == 'undefined') Browser.resizeCanvas = false;

      var canvas = Module['canvas'];
      function fullscreenChange() {
        Browser.isFullscreen = false;
        var canvasContainer = canvas.parentNode;
        if ((document['fullscreenElement'] || document['mozFullScreenElement'] ||
             document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
             document['webkitCurrentFullScreenElement']) === canvasContainer) {
          canvas.exitFullscreen = Browser.exitFullscreen;
          if (Browser.lockPointer) canvas.requestPointerLock();
          Browser.isFullscreen = true;
          if (Browser.resizeCanvas) {
            Browser.setFullscreenCanvasSize();
          } else {
            Browser.updateCanvasDimensions(canvas);
          }
        } else {
          // remove the full screen specific parent of the canvas again to restore the HTML structure from before going full screen
          canvasContainer.parentNode.insertBefore(canvas, canvasContainer);
          canvasContainer.parentNode.removeChild(canvasContainer);

          if (Browser.resizeCanvas) {
            Browser.setWindowedCanvasSize();
          } else {
            Browser.updateCanvasDimensions(canvas);
          }
        }
        Module['onFullScreen']?.(Browser.isFullscreen);
        Module['onFullscreen']?.(Browser.isFullscreen);
      }

      if (!Browser.fullscreenHandlersInstalled) {
        Browser.fullscreenHandlersInstalled = true;
        document.addEventListener('fullscreenchange', fullscreenChange, false);
        document.addEventListener('mozfullscreenchange', fullscreenChange, false);
        document.addEventListener('webkitfullscreenchange', fullscreenChange, false);
        document.addEventListener('MSFullscreenChange', fullscreenChange, false);
      }

      // create a new parent to ensure the canvas has no siblings. this allows browsers to optimize full screen performance when its parent is the full screen root
      var canvasContainer = document.createElement("div");
      canvas.parentNode.insertBefore(canvasContainer, canvas);
      canvasContainer.appendChild(canvas);

      // use parent of canvas as full screen root to allow aspect ratio correction (Firefox stretches the root to screen size)
      canvasContainer.requestFullscreen = canvasContainer['requestFullscreen'] ||
                                          canvasContainer['mozRequestFullScreen'] ||
                                          canvasContainer['msRequestFullscreen'] ||
                                         (canvasContainer['webkitRequestFullscreen'] ? () => canvasContainer['webkitRequestFullscreen'](Element['ALLOW_KEYBOARD_INPUT']) : null) ||
                                         (canvasContainer['webkitRequestFullScreen'] ? () => canvasContainer['webkitRequestFullScreen'](Element['ALLOW_KEYBOARD_INPUT']) : null);

      canvasContainer.requestFullscreen();
    },

#if ASSERTIONS
    requestFullScreen() {
      abort('Module.requestFullScreen has been replaced by Module.requestFullscreen (without a capital S)');
    },
#endif

    exitFullscreen() {
      // This is workaround for chrome. Trying to exit from fullscreen
      // not in fullscreen state will cause "TypeError: Document not active"
      // in chrome. See https://github.com/emscripten-core/emscripten/pull/8236
      if (!Browser.isFullscreen) {
        return false;
      }

      var CFS = document['exitFullscreen'] ||
                document['cancelFullScreen'] ||
                document['mozCancelFullScreen'] ||
                document['msExitFullscreen'] ||
                document['webkitCancelFullScreen'] ||
          (() => {});
      CFS.apply(document, []);
      return true;
    },

    // abort and pause-aware versions TODO: build main loop on top of this?

    safeSetTimeout(func, timeout) {
      // Legacy function, this is used by the SDL2 port so we need to keep it
      // around at least until that is updated.
      // See https://github.com/libsdl-org/SDL/pull/6304
      return safeSetTimeout(func, timeout);
    },

    getMimetype(name) {
      return {
        'jpg': 'image/jpeg',
        'jpeg': 'image/jpeg',
        'png': 'image/png',
        'bmp': 'image/bmp',
        'ogg': 'audio/ogg',
        'wav': 'audio/wav',
        'mp3': 'audio/mpeg'
      }[name.substr(name.lastIndexOf('.')+1)];
    },

    getUserMedia(func) {
      window.getUserMedia ||= navigator['getUserMedia'] ||
                              navigator['mozGetUserMedia'];
      window.getUserMedia(func);
    },


    getMovementX(event) {
      return event['movementX'] ||
             event['mozMovementX'] ||
             event['webkitMovementX'] ||
             0;
    },

    getMovementY(event) {
      return event['movementY'] ||
             event['mozMovementY'] ||
             event['webkitMovementY'] ||
             0;
    },

    // Browsers specify wheel direction according to the page CSS pixel Y direction:
    // Scrolling mouse wheel down (==towards user/away from screen) on Windows/Linux (and macOS without 'natural scroll' enabled)
    // is the positive wheel direction. Scrolling mouse wheel up (towards the screen) is the negative wheel direction.
    // This function returns the wheel direction in the browser page coordinate system (+: down, -: up). Note that this is often the
    // opposite of native code: In native APIs the positive scroll direction is to scroll up (away from the user).
    // NOTE: The mouse wheel delta is a decimal number, and can be a fractional value within -1 and 1. If you need to represent
    //       this as an integer, don't simply cast to int, or you may receive scroll events for wheel delta == 0.
    // NOTE: We convert all units returned by events into steps, i.e. individual wheel notches.
    //       These conversions are only approximations. Changing browsers, operating systems, or even settings can change the values.
    getMouseWheelDelta(event) {
      var delta = 0;
      switch (event.type) {
        case 'DOMMouseScroll':
          // 3 lines make up a step
          delta = event.detail / 3;
          break;
        case 'mousewheel':
          // 120 units make up a step
          delta = event.wheelDelta / 120;
          break;
        case 'wheel':
          delta = event.deltaY
          switch (event.deltaMode) {
            case 0:
              // DOM_DELTA_PIXEL: 100 pixels make up a step
              delta /= 100;
              break;
            case 1:
              // DOM_DELTA_LINE: 3 lines make up a step
              delta /= 3;
              break;
            case 2:
              // DOM_DELTA_PAGE: A page makes up 80 steps
              delta *= 80;
              break;
            default:
              throw 'unrecognized mouse wheel delta mode: ' + event.deltaMode;
          }
          break;
        default:
          throw 'unrecognized mouse wheel event: ' + event.type;
      }
      return delta;
    },

    mouseX: 0,
    mouseY: 0,
    mouseMovementX: 0,
    mouseMovementY: 0,
    touches: {},
    lastTouches: {},

    // Return the mouse coordinates relative to the top, left of the canvas, corrected for scroll offset.
    calculateMouseCoords(pageX, pageY) {
      // Calculate the movement based on the changes
      // in the coordinates.
      var rect = Module["canvas"].getBoundingClientRect();
      var cw = Module["canvas"].width;
      var ch = Module["canvas"].height;

      // Neither .scrollX or .pageXOffset are defined in a spec, but
      // we prefer .scrollX because it is currently in a spec draft.
      // (see: http://www.w3.org/TR/2013/WD-cssom-view-20131217/)
      var scrollX = ((typeof window.scrollX != 'undefined') ? window.scrollX : window.pageXOffset);
      var scrollY = ((typeof window.scrollY != 'undefined') ? window.scrollY : window.pageYOffset);
#if ASSERTIONS
      // If this assert lands, it's likely because the browser doesn't support scrollX or pageXOffset
      // and we have no viable fallback.
      assert((typeof scrollX != 'undefined') && (typeof scrollY != 'undefined'), 'Unable to retrieve scroll position, mouse positions likely broken.');
#endif
      var adjustedX = pageX - (scrollX + rect.left);
      var adjustedY = pageY - (scrollY + rect.top);

      // the canvas might be CSS-scaled compared to its backbuffer;
      // SDL-using content will want mouse coordinates in terms
      // of backbuffer units.
      adjustedX = adjustedX * (cw / rect.width);
      adjustedY = adjustedY * (ch / rect.height);

      return { x: adjustedX, y: adjustedY };
    },

    // Directly set the Browser state with new mouse coordinates calculated using calculateMouseCoords.
    setMouseCoords(pageX, pageY) {
      const {x, y} = Browser.calculateMouseCoords(pageX, pageY);
      Browser.mouseMovementX = x - Browser.mouseX;
      Browser.mouseMovementY = y - Browser.mouseY;
      Browser.mouseX = x;
      Browser.mouseY = y;
    },

    // Unpack a "mouse" event, handling SDL touch paths and pointerlock compatibility stuff.
    calculateMouseEvent(event) { // event should be mousemove, mousedown or mouseup
      if (Browser.pointerLock) {
        // When the pointer is locked, calculate the coordinates
        // based on the movement of the mouse.
        // Workaround for Firefox bug 764498
        if (event.type != 'mousemove' &&
            ('mozMovementX' in event)) {
          Browser.mouseMovementX = Browser.mouseMovementY = 0;
        } else {
          Browser.mouseMovementX = Browser.getMovementX(event);
          Browser.mouseMovementY = Browser.getMovementY(event);
        }

        // add the mouse delta to the current absolute mouse position
        Browser.mouseX += Browser.mouseMovementX;
        Browser.mouseY += Browser.mouseMovementY;
      } else {
        if (event.type === 'touchstart' || event.type === 'touchend' || event.type === 'touchmove') {
          var touch = event.touch;
          if (touch === undefined) {
            return; // the "touch" property is only defined in SDL

          }
          var coords = Browser.calculateMouseCoords(touch.pageX, touch.pageY);

          if (event.type === 'touchstart') {
            Browser.lastTouches[touch.identifier] = coords;
            Browser.touches[touch.identifier] = coords;
          } else if (event.type === 'touchend' || event.type === 'touchmove') {
            var last = Browser.touches[touch.identifier];
            last ||= coords;
            Browser.lastTouches[touch.identifier] = last;
            Browser.touches[touch.identifier] = coords;
          }
          return;
        }

        Browser.setMouseCoords(event.pageX, event.pageY);
      }
    },

    resizeListeners: [],

    updateResizeListeners() {
      var canvas = Module['canvas'];
      Browser.resizeListeners.forEach((listener) => listener(canvas.width, canvas.height));
    },

    setCanvasSize(width, height, noUpdates) {
      var canvas = Module['canvas'];
      Browser.updateCanvasDimensions(canvas, width, height);
      if (!noUpdates) Browser.updateResizeListeners();
    },

    windowedWidth: 0,
    windowedHeight: 0,
    setFullscreenCanvasSize() {
      // check if SDL is available
      if (typeof SDL != "undefined") {
        var flags = {{{ makeGetValue('SDL.screen', '0', 'u32') }}};
        flags = flags | 0x00800000; // set SDL_FULLSCREEN flag
        {{{ makeSetValue('SDL.screen', '0', 'flags', 'i32') }}};
      }
      Browser.updateCanvasDimensions(Module['canvas']);
      Browser.updateResizeListeners();
    },

    setWindowedCanvasSize() {
      // check if SDL is available
      if (typeof SDL != "undefined") {
        var flags = {{{ makeGetValue('SDL.screen', '0', 'u32') }}};
        flags = flags & ~0x00800000; // clear SDL_FULLSCREEN flag
        {{{ makeSetValue('SDL.screen', '0', 'flags', 'i32') }}};
      }
      Browser.updateCanvasDimensions(Module['canvas']);
      Browser.updateResizeListeners();
    },

    updateCanvasDimensions(canvas, wNative, hNative) {
      if (wNative && hNative) {
        canvas.widthNative = wNative;
        canvas.heightNative = hNative;
      } else {
        wNative = canvas.widthNative;
        hNative = canvas.heightNative;
      }
      var w = wNative;
      var h = hNative;
      if (Module['forcedAspectRatio'] && Module['forcedAspectRatio'] > 0) {
        if (w/h < Module['forcedAspectRatio']) {
          w = Math.round(h * Module['forcedAspectRatio']);
        } else {
          h = Math.round(w / Module['forcedAspectRatio']);
        }
      }
      if (((document['fullscreenElement'] || document['mozFullScreenElement'] ||
           document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
           document['webkitCurrentFullScreenElement']) === canvas.parentNode) && (typeof screen != 'undefined')) {
         var factor = Math.min(screen.width / w, screen.height / h);
         w = Math.round(w * factor);
         h = Math.round(h * factor);
      }
      if (Browser.resizeCanvas) {
        if (canvas.width  != w) canvas.width  = w;
        if (canvas.height != h) canvas.height = h;
        if (typeof canvas.style != 'undefined') {
          canvas.style.removeProperty( "width");
          canvas.style.removeProperty("height");
        }
      } else {
        if (canvas.width  != wNative) canvas.width  = wNative;
        if (canvas.height != hNative) canvas.height = hNative;
        if (typeof canvas.style != 'undefined') {
          if (w != wNative || h != hNative) {
            canvas.style.setProperty( "width", w + "px", "important");
            canvas.style.setProperty("height", h + "px", "important");
          } else {
            canvas.style.removeProperty( "width");
            canvas.style.removeProperty("height");
          }
        }
      }
    },
  },

  emscripten_run_preload_plugins__deps: ['$PATH'],
  emscripten_run_preload_plugins__proxy: 'sync',
  emscripten_run_preload_plugins: (file, onload, onerror) => {
    {{{ runtimeKeepalivePush() }}}

    var _file = UTF8ToString(file);
    var data = FS.analyzePath(_file);
    if (!data.exists) return -1;
    FS.createPreloadedFile(
      PATH.dirname(_file),
      PATH.basename(_file),
      // TODO: This copy is not needed if the contents are already a Uint8Array,
      //       which they often are (and always are in WasmFS).
      new Uint8Array(data.object.contents), true, true,
      () => {
        {{{ runtimeKeepalivePop() }}}
        if (onload) {{{ makeDynCall('vp', 'onload') }}}(file);
      },
      () => {
        {{{ runtimeKeepalivePop() }}}
        if (onerror) {{{ makeDynCall('vp', 'onerror') }}}(file);
      },
      true // don'tCreateFile - it's already there
    );
    return 0;
  },

  $Browser_asyncPrepareDataCounter: 0,

  emscripten_run_preload_plugins_data__proxy: 'sync',
  emscripten_run_preload_plugins_data__deps: ['$stringToNewUTF8', '$Browser_asyncPrepareDataCounter'],
  emscripten_run_preload_plugins_data: (data, size, suffix, arg, onload, onerror) => {
    {{{ runtimeKeepalivePush() }}}

    var _suffix = UTF8ToString(suffix);
    var name = 'prepare_data_' + (Browser_asyncPrepareDataCounter++) + '.' + _suffix;
    var cname = stringToNewUTF8(name);
    FS.createPreloadedFile(
      '/',
      name,
      {{{ makeHEAPView('U8', 'data', 'data + size') }}},
      true, true,
      () => {
        {{{ runtimeKeepalivePop() }}}
        if (onload) {{{ makeDynCall('vpp', 'onload') }}}(arg, cname);
      },
      () => {
        {{{ runtimeKeepalivePop() }}}
        if (onerror) {{{ makeDynCall('vp', 'onerror') }}}(arg);
      },
      true // don'tCreateFile - it's already there
    );
  },

  // Callable from pthread, executes in pthread context.
  emscripten_async_run_script__deps: ['emscripten_run_script', '$safeSetTimeout'],
  emscripten_async_run_script: (script, millis) => {
    // TODO: cache these to avoid generating garbage
    safeSetTimeout(() => _emscripten_run_script(script), millis);
  },

  // TODO: currently not callable from a pthread, but immediately calls onerror() if not on main thread.
  emscripten_async_load_script__deps: ['$UTF8ToString'],
  emscripten_async_load_script: (url, onload, onerror) => {
    url = UTF8ToString(url);
#if PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) {
      err(`emscripten_async_load_script("${url}") failed, emscripten_async_load_script is currently not available in pthreads!`);
      onerror && {{{ makeDynCall('v', 'onerror') }}}();
      return;
    }
#endif
#if ASSERTIONS
    assert(runDependencies === 0, 'async_load_script must be run when no other dependencies are active');
#endif
    {{{ runtimeKeepalivePush() }}}

    var loadDone = () => {
      {{{ runtimeKeepalivePop() }}}
      if (onload) {
        var onloadCallback = () => callUserCallback({{{ makeDynCall('v', 'onload') }}});
        if (runDependencies > 0) {
          dependenciesFulfilled = onloadCallback;
        } else {
          onloadCallback();
        }
      }
    }

    var loadError = () => {
      {{{ runtimeKeepalivePop() }}}
      if (onerror) {
        callUserCallback({{{ makeDynCall('v', 'onerror') }}});
      }
    };

#if ENVIRONMENT_MAY_BE_NODE && DYNAMIC_EXECUTION
    if (ENVIRONMENT_IS_NODE) {
      readAsync(url, false).then((data) => {
        eval(data);
        loadDone();
      }, loadError);
      return;
    }
#endif

    var script = document.createElement('script');
    script.onload = loadDone;
    script.onerror = loadError;
    script.src = url;
    document.body.appendChild(script);
  },

  $safeRequestAnimationFrame__deps: ['$MainLoop'],
  $safeRequestAnimationFrame: (func) => {
    {{{ runtimeKeepalivePush() }}}
    return MainLoop.requestAnimationFrame(() => {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(func);
    });
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_async_call__deps: ['$safeSetTimeout', '$safeRequestAnimationFrame'],
  emscripten_async_call: (func, arg, millis) => {
    function wrapper() {
      {{{ makeDynCall('vp', 'func') }}}(arg);
    }

    if (millis >= 0
#if ENVIRONMENT_MAY_BE_NODE
      // node does not support requestAnimationFrame
      || ENVIRONMENT_IS_NODE
#endif
    ) {
      safeSetTimeout(wrapper, millis);
    } else {
      safeRequestAnimationFrame(wrapper);
    }
  },

  emscripten_get_window_title__proxy: 'sync',
  emscripten_get_window_title: () => {
    var buflen = 256;

    if (!_emscripten_get_window_title.buffer) {
      _emscripten_get_window_title.buffer = _malloc(buflen);
    }

    stringToUTF8(document.title, _emscripten_get_window_title.buffer, buflen);

    return _emscripten_get_window_title.buffer;
  },

  emscripten_set_window_title__proxy: 'sync',
  emscripten_set_window_title: (title) => document.title = UTF8ToString(title),

  emscripten_get_screen_size__proxy: 'sync',
  emscripten_get_screen_size: (width, height) => {
    {{{ makeSetValue('width', '0', 'screen.width', 'i32') }}};
    {{{ makeSetValue('height', '0', 'screen.height', 'i32') }}};
  },

  emscripten_hide_mouse__proxy: 'sync',
  emscripten_hide_mouse: () => {
    var styleSheet = document.styleSheets[0];
    var rules = styleSheet.cssRules;
    for (var i = 0; i < rules.length; i++) {
      if (rules[i].cssText.substr(0, 6) == 'canvas') {
        styleSheet.deleteRule(i);
        i--;
      }
    }
    styleSheet.insertRule('canvas.emscripten { border: 1px solid black; cursor: none; }', 0);
  },

  emscripten_set_canvas_size__proxy: 'sync',
  emscripten_set_canvas_size: (width, height) => {
    Browser.setCanvasSize(width, height);
  },

  emscripten_get_canvas_size__proxy: 'sync',
  emscripten_get_canvas_size: (width, height, isFullscreen) => {
    var canvas = Module['canvas'];
    {{{ makeSetValue('width', '0', 'canvas.width', 'i32') }}};
    {{{ makeSetValue('height', '0', 'canvas.height', 'i32') }}};
    {{{ makeSetValue('isFullscreen', '0', 'Browser.isFullscreen ? 1 : 0', 'i32') }}};
  },

  // To avoid creating worker parent->child chains, always proxies to execute on the main thread.
  emscripten_create_worker__proxy: 'sync',
  emscripten_create_worker__deps: ['$UTF8ToString', 'malloc', 'free'],
  emscripten_create_worker: (url) => {
    url = UTF8ToString(url);
    var id = Browser.workers.length;
    var info = {
      worker: new Worker(url),
      callbacks: [],
      awaited: 0,
      buffer: 0,
      bufferSize: 0
    };
    info.worker.onmessage = function info_worker_onmessage(msg) {
      if (ABORT) return;
      var info = Browser.workers[id];
      if (!info) return; // worker was destroyed meanwhile
      var callbackId = msg.data['callbackId'];
      var callbackInfo = info.callbacks[callbackId];
      if (!callbackInfo) return; // no callback or callback removed meanwhile
      // Don't trash our callback state if we expect additional calls.
      if (msg.data['finalResponse']) {
        info.awaited--;
        info.callbacks[callbackId] = null; // TODO: reuse callbackIds, compress this
        {{{ runtimeKeepalivePop() }}}
      }
      var data = msg.data['data'];
      if (data) {
        if (!data.byteLength) data = new Uint8Array(data);
        if (!info.buffer || info.bufferSize < data.length) {
          if (info.buffer) _free(info.buffer);
          info.bufferSize = data.length;
          info.buffer = _malloc(data.length);
        }
        HEAPU8.set(data, info.buffer);
        callbackInfo.func(info.buffer, data.length, callbackInfo.arg);
      } else {
        callbackInfo.func(0, 0, callbackInfo.arg);
      }
    };
    Browser.workers.push(info);
    return id;
  },

  emscripten_destroy_worker__deps: ['free'],
  emscripten_destroy_worker__proxy: 'sync',
  emscripten_destroy_worker: (id) => {
    var info = Browser.workers[id];
    info.worker.terminate();
    if (info.buffer) _free(info.buffer);
    Browser.workers[id] = null;
  },

  emscripten_call_worker__proxy: 'sync',
  emscripten_call_worker: (id, funcName, data, size, callback, arg) => {
    funcName = UTF8ToString(funcName);
    var info = Browser.workers[id];
    var callbackId = -1;
    if (callback) {
      // If we are waiting for a response from the worker we need to keep
      // the runtime alive at least long enough to receive it.
      // The corresponding runtimeKeepalivePop is in the `finalResponse`
      // handler above.
      {{{ runtimeKeepalivePush() }}}
      callbackId = info.callbacks.length;
      info.callbacks.push({
        func: {{{ makeDynCall('vpip', 'callback') }}},
        arg
      });
      info.awaited++;
    }
    var transferObject = {
      'funcName': funcName,
      'callbackId': callbackId,
      'data': data ? new Uint8Array({{{ makeHEAPView('U8', 'data', 'data + size') }}}) : 0
    };
    if (data) {
      info.worker.postMessage(transferObject, [transferObject.data.buffer]);
    } else {
      info.worker.postMessage(transferObject);
    }
  },

#if BUILD_AS_WORKER
  emscripten_worker_respond_provisionally__proxy: 'sync',
  emscripten_worker_respond_provisionally: (data, size) => {
    if (workerResponded) throw 'already responded with final response!';
    var transferObject = {
      'callbackId': workerCallbackId,
      'finalResponse': false,
      'data': data ? new Uint8Array({{{ makeHEAPView('U8', 'data', 'data + size') }}}) : 0
    };
    if (data) {
      postMessage(transferObject, [transferObject.data.buffer]);
    } else {
      postMessage(transferObject);
    }
  },

  emscripten_worker_respond__proxy: 'sync',
  emscripten_worker_respond: (data, size) => {
    if (workerResponded) throw 'already responded with final response!';
    workerResponded = true;
    var transferObject = {
      'callbackId': workerCallbackId,
      'finalResponse': true,
      'data': data ? new Uint8Array({{{ makeHEAPView('U8', 'data', 'data + size') }}}) : 0
    };
    if (data) {
      postMessage(transferObject, [transferObject.data.buffer]);
    } else {
      postMessage(transferObject);
    }
  },
#endif

  emscripten_get_worker_queue_size__proxy: 'sync',
  emscripten_get_worker_queue_size: (id) => {
    var info = Browser.workers[id];
    if (!info) return -1;
    return info.awaited;
  },

  emscripten_get_preloaded_image_data__deps: ['$getPreloadedImageData', '$UTF8ToString'],
  emscripten_get_preloaded_image_data__proxy: 'sync',
  emscripten_get_preloaded_image_data: (path, w, h) => getPreloadedImageData(UTF8ToString(path), w, h),

  $getPreloadedImageData__internal: true,
  $getPreloadedImageData__data: ['$PATH_FS', 'malloc'],
  $getPreloadedImageData: (path, w, h) => {
    path = PATH_FS.resolve(path);

    var canvas = /** @type {HTMLCanvasElement} */(preloadedImages[path]);
    if (!canvas) return 0;

    var ctx = canvas.getContext("2d");
    var image = ctx.getImageData(0, 0, canvas.width, canvas.height);
    var buf = _malloc(canvas.width * canvas.height * 4);

    HEAPU8.set(image.data, buf);

    {{{ makeSetValue('w', '0', 'canvas.width', 'i32') }}};
    {{{ makeSetValue('h', '0', 'canvas.height', 'i32') }}};
    return buf;
  },

#if !WASMFS // WasmFS implements this in wasm
  emscripten_get_preloaded_image_data_from_FILE__deps: ['$getPreloadedImageData', 'fileno'],
  emscripten_get_preloaded_image_data_from_FILE__proxy: 'sync',
  emscripten_get_preloaded_image_data_from_FILE: (file, w, h) => {
    var fd = _fileno(file);
    var stream = FS.getStream(fd);
    if (stream) {
      return getPreloadedImageData(stream.path, w, h);
    }

    return 0;
  }
#endif
};

autoAddDeps(LibraryBrowser, '$Browser');

addToLibrary(LibraryBrowser);
