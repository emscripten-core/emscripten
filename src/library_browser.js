/**
 * @license
 * Copyright 2011 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Utilities for browser environments
var LibraryBrowser = {
  $Browser__deps: [
    '$setMainLoop',
    '$callUserCallback',
    '$safeSetTimeout',
    'emscripten_set_main_loop_timing',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush',
    '$runtimeKeepalivePop'
#endif
  ],
  $Browser__postset: 'Module["requestFullscreen"] = function Module_requestFullscreen(lockPointer, resizeCanvas) { Browser.requestFullscreen(lockPointer, resizeCanvas) };\n' + // exports
#if ASSERTIONS
                     'Module["requestFullScreen"] = function Module_requestFullScreen() { Browser.requestFullScreen() };\n' +
#endif
                     'Module["requestAnimationFrame"] = function Module_requestAnimationFrame(func) { Browser.requestAnimationFrame(func) };\n' +
                     'Module["setCanvasSize"] = function Module_setCanvasSize(width, height, noUpdates) { Browser.setCanvasSize(width, height, noUpdates) };\n' +
                     'Module["pauseMainLoop"] = function Module_pauseMainLoop() { Browser.mainLoop.pause() };\n' +
                     'Module["resumeMainLoop"] = function Module_resumeMainLoop() { Browser.mainLoop.resume() };\n' +
                     'Module["getUserMedia"] = function Module_getUserMedia() { Browser.getUserMedia() };\n' +
                     'Module["createContext"] = function Module_createContext(canvas, useWebGL, setInModule, webGLContextAttributes) { return Browser.createContext(canvas, useWebGL, setInModule, webGLContextAttributes) };\n' +
#if MAIN_MODULE
                     'var preloadedWasm = {};\n' +
#endif
                     'var preloadedImages = {};\n' +
                     'var preloadedAudios = {};\n',

  $Browser: {
    mainLoop: {
      running: false,
      scheduler: null,
      method: '',
      // Each main loop is numbered with a ID in sequence order. Only one main
      // loop can run at a time. This variable stores the ordinal number of the
      // main loop that is currently allowed to run. All previous main loops
      // will quit themselves. This is incremented whenever a new main loop is
      // created.
      /** @type{number} */
      currentlyRunningMainloop: 0,
      // The main loop tick function that will be called at each iteration.
      func: null,
      // The argument that will be passed to the main loop. (of type void*)
      arg: 0,
      timingMode: 0,
      timingValue: 0,
      currentFrameNumber: 0,
      queue: [],
      pause: function() {
        Browser.mainLoop.scheduler = null;
        // Incrementing this signals the previous main loop that it's now become old, and it must return.
        Browser.mainLoop.currentlyRunningMainloop++;
      },
      resume: function() {
        Browser.mainLoop.currentlyRunningMainloop++;
        var timingMode = Browser.mainLoop.timingMode;
        var timingValue = Browser.mainLoop.timingValue;
        var func = Browser.mainLoop.func;
        Browser.mainLoop.func = null;
        // do not set timing and call scheduler, we will do it on the next lines
        setMainLoop(func, 0, false, Browser.mainLoop.arg, true);
        _emscripten_set_main_loop_timing(timingMode, timingValue);
        Browser.mainLoop.scheduler();
      },
      updateStatus: function() {
        if (Module['setStatus']) {
          var message = Module['statusMessage'] || 'Please wait...';
          var remaining = Browser.mainLoop.remainingBlockers;
          var expected = Browser.mainLoop.expectedBlockers;
          if (remaining) {
            if (remaining < expected) {
              Module['setStatus'](message + ' (' + (expected - remaining) + '/' + expected + ')');
            } else {
              Module['setStatus'](message);
            }
          } else {
            Module['setStatus']('');
          }
        }
      },
      runIter: function(func) {
        if (ABORT) return;
        if (Module['preMainLoop']) {
          var preRet = Module['preMainLoop']();
          if (preRet === false) {
            return; // |return false| skips a frame
          }
        }
        callUserCallback(func);
        if (Module['postMainLoop']) Module['postMainLoop']();
      }
    },
    isFullscreen: false,
    pointerLock: false,
    moduleContextCreatedCallbacks: [],
    workers: [],

    init: function() {
      if (!Module["preloadPlugins"]) Module["preloadPlugins"] = []; // needs to exist even in workers

      if (Browser.initted) return;
      Browser.initted = true;

      try {
        new Blob();
        Browser.hasBlobConstructor = true;
      } catch(e) {
        Browser.hasBlobConstructor = false;
        out("warning: no blob constructor, cannot create blobs with mimetypes");
      }
      Browser.BlobBuilder = typeof MozBlobBuilder != "undefined" ? MozBlobBuilder : (typeof WebKitBlobBuilder != "undefined" ? WebKitBlobBuilder : (!Browser.hasBlobConstructor ? out("warning: no BlobBuilder") : null));
      Browser.URLObject = typeof window != "undefined" ? (window.URL ? window.URL : window.webkitURL) : undefined;
      if (!Module.noImageDecoding && typeof Browser.URLObject == 'undefined') {
        out("warning: Browser does not support creating object URLs. Built-in browser image decoding will not be available.");
        Module.noImageDecoding = true;
      }

      // Support for plugins that can process preloaded files. You can add more of these to
      // your app by creating and appending to Module.preloadPlugins.
      //
      // Each plugin is asked if it can handle a file based on the file's name. If it can,
      // it is given the file's raw data. When it is done, it calls a callback with the file's
      // (possibly modified) data. For example, a plugin might decompress a file, or it
      // might create some side data structure for use later (like an Image element, etc.).

      var imagePlugin = {};
      imagePlugin['canHandle'] = function imagePlugin_canHandle(name) {
        return !Module.noImageDecoding && /\.(jpg|jpeg|png|bmp)$/i.test(name);
      };
      imagePlugin['handle'] = function imagePlugin_handle(byteArray, name, onload, onerror) {
        var b = null;
        if (Browser.hasBlobConstructor) {
          try {
            b = new Blob([byteArray], { type: Browser.getMimetype(name) });
            if (b.size !== byteArray.length) { // Safari bug #118630
              // Safari's Blob can only take an ArrayBuffer
              b = new Blob([(new Uint8Array(byteArray)).buffer], { type: Browser.getMimetype(name) });
            }
          } catch(e) {
            warnOnce('Blob constructor present but fails: ' + e + '; falling back to blob builder');
          }
        }
        if (!b) {
          var bb = new Browser.BlobBuilder();
          bb.append((new Uint8Array(byteArray)).buffer); // we need to pass a buffer, and must copy the array to get the right data range
          b = bb.getBlob();
        }
        var url = Browser.URLObject.createObjectURL(b);
#if ASSERTIONS
        assert(typeof url == 'string', 'createObjectURL must return a url as a string');
#endif
        var img = new Image();
        img.onload = () => {
          assert(img.complete, 'Image ' + name + ' could not be decoded');
          var canvas = /** @type {!HTMLCanvasElement} */ (document.createElement('canvas'));
          canvas.width = img.width;
          canvas.height = img.height;
          var ctx = canvas.getContext('2d');
          ctx.drawImage(img, 0, 0);
          preloadedImages[name] = canvas;
          Browser.URLObject.revokeObjectURL(url);
          if (onload) onload(byteArray);
        };
        img.onerror = (event) => {
          out('Image ' + url + ' could not be decoded');
          if (onerror) onerror();
        };
        img.src = url;
      };
      Module['preloadPlugins'].push(imagePlugin);

      var audioPlugin = {};
      audioPlugin['canHandle'] = function audioPlugin_canHandle(name) {
        return !Module.noAudioDecoding && name.substr(-4) in { '.ogg': 1, '.wav': 1, '.mp3': 1 };
      };
      audioPlugin['handle'] = function audioPlugin_handle(byteArray, name, onload, onerror) {
        var done = false;
        function finish(audio) {
          if (done) return;
          done = true;
          preloadedAudios[name] = audio;
          if (onload) onload(byteArray);
        }
        function fail() {
          if (done) return;
          done = true;
          preloadedAudios[name] = new Audio(); // empty shim
          if (onerror) onerror();
        }
        if (Browser.hasBlobConstructor) {
          try {
            var b = new Blob([byteArray], { type: Browser.getMimetype(name) });
          } catch(e) {
            return fail();
          }
          var url = Browser.URLObject.createObjectURL(b); // XXX we never revoke this!
#if ASSERTIONS
          assert(typeof url == 'string', 'createObjectURL must return a url as a string');
#endif
          var audio = new Audio();
          audio.addEventListener('canplaythrough', function() { finish(audio) }, false); // use addEventListener due to chromium bug 124926
          audio.onerror = function audio_onerror(event) {
            if (done) return;
            out('warning: browser could not fully decode audio ' + name + ', trying slower base64 approach');
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
          safeSetTimeout(function() {
            finish(audio); // try to use it even though it is not necessarily ready to play
          }, 10000);
        } else {
          return fail();
        }
      };
      Module['preloadPlugins'].push(audioPlugin);

#if MAIN_MODULE
      // Use string keys here to avoid minification since the plugin consumer
      // also uses string keys.
      var wasmPlugin = {
        'asyncWasmLoadPromise': new Promise(function(resolve, reject) { return resolve(); }),
        'canHandle': function(name) {
          return !Module.noWasmDecoding && name.endsWith('.so')
        },
        'handle': function(byteArray, name, onload, onerror) {
          // loadWebAssemblyModule can not load modules out-of-order, so rather
          // than just running the promises in parallel, this makes a chain of
          // promises to run in series.
          wasmPlugin['asyncWasmLoadPromise'] = wasmPlugin['asyncWasmLoadPromise'].then(
            function() {
              return loadWebAssemblyModule(byteArray, {loadAsync: true, nodelete: true});
            }).then(
              function(module) {
                preloadedWasm[name] = module;
                onload();
              },
              function(err) {
                console.warn("Couldn't instantiate wasm: " + name + " '" + err + "'");
                onerror();
              });
        }
      };
      Module['preloadPlugins'].push(wasmPlugin);
#endif // MAIN_MODULE

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
                                    function(){};
        canvas.exitPointerLock = document['exitPointerLock'] ||
                                 document['mozExitPointerLock'] ||
                                 document['webkitExitPointerLock'] ||
                                 document['msExitPointerLock'] ||
                                 function(){}; // no-op if function does not exist
        canvas.exitPointerLock = canvas.exitPointerLock.bind(document);

        document.addEventListener('pointerlockchange', pointerLockChange, false);
        document.addEventListener('mozpointerlockchange', pointerLockChange, false);
        document.addEventListener('webkitpointerlockchange', pointerLockChange, false);
        document.addEventListener('mspointerlockchange', pointerLockChange, false);

        if (Module['elementPointerLock']) {
          canvas.addEventListener("click", function(ev) {
            if (!Browser.pointerLock && Module['canvas'].requestPointerLock) {
              Module['canvas'].requestPointerLock();
              ev.preventDefault();
            }
          }, false);
        }
      }
    },

    // Tries to handle an input byteArray using preload plugins. Returns true if
    // it was handled.
    handledByPreloadPlugin: function(byteArray, fullname, finish, onerror) {
      // Ensure plugins are ready.
      Browser.init();

      var handled = false;
      Module['preloadPlugins'].forEach(function(plugin) {
        if (handled) return;
        if (plugin['canHandle'](fullname)) {
          plugin['handle'](byteArray, fullname, finish, onerror);
          handled = true;
        }
      });
      return handled;
    },

    createContext: function(/** @type {HTMLCanvasElement} */ canvas, useWebGL, setInModule, webGLContextAttributes) {
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
        if (!useWebGL) assert(typeof GLctx == 'undefined', 'cannot set in module if GLctx is used, but we are a non-GL context that would replace it');

        Module.ctx = ctx;
        if (useWebGL) GL.makeContextCurrent(contextHandle);
        Module.useWebGL = useWebGL;
        Browser.moduleContextCreatedCallbacks.forEach(function(callback) { callback() });
        Browser.init();
      }
      return ctx;
    },

    destroyContext: function(canvas, useWebGL, setInModule) {},

    fullscreenHandlersInstalled: false,
    lockPointer: undefined,
    resizeCanvas: undefined,
    requestFullscreen: function(lockPointer, resizeCanvas) {
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
        if (Module['onFullScreen']) Module['onFullScreen'](Browser.isFullscreen);
        if (Module['onFullscreen']) Module['onFullscreen'](Browser.isFullscreen);
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
                                         (canvasContainer['webkitRequestFullscreen'] ? function() { canvasContainer['webkitRequestFullscreen'](Element['ALLOW_KEYBOARD_INPUT']) } : null) ||
                                         (canvasContainer['webkitRequestFullScreen'] ? function() { canvasContainer['webkitRequestFullScreen'](Element['ALLOW_KEYBOARD_INPUT']) } : null);

      canvasContainer.requestFullscreen();
    },

#if ASSERTIONS
    requestFullScreen: function() {
      abort('Module.requestFullScreen has been replaced by Module.requestFullscreen (without a capital S)');
    },
#endif

    exitFullscreen: function() {
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
          (function() {});
      CFS.apply(document, []);
      return true;
    },

    nextRAF: 0,

    fakeRequestAnimationFrame: function(func) {
      // try to keep 60fps between calls to here
      var now = Date.now();
      if (Browser.nextRAF === 0) {
        Browser.nextRAF = now + 1000/60;
      } else {
        while (now + 2 >= Browser.nextRAF) { // fudge a little, to avoid timer jitter causing us to do lots of delay:0
          Browser.nextRAF += 1000/60;
        }
      }
      var delay = Math.max(Browser.nextRAF - now, 0);
      setTimeout(func, delay);
    },

    requestAnimationFrame: function(func) {
      if (typeof requestAnimationFrame == 'function') {
        requestAnimationFrame(func);
        return;
      }
      var RAF = Browser.fakeRequestAnimationFrame;
#if LEGACY_VM_SUPPORT
      if (typeof window != 'undefined') {
        RAF = window['requestAnimationFrame'] ||
              window['mozRequestAnimationFrame'] ||
              window['webkitRequestAnimationFrame'] ||
              window['msRequestAnimationFrame'] ||
              window['oRequestAnimationFrame'] ||
              RAF;
      }
#endif
      RAF(func);
    },

    // abort and pause-aware versions TODO: build main loop on top of this?

    safeSetTimeout: function(func) {
      // Legacy function, this is used by the SDL2 port so we need to keep it
      // around at least until that is updated.
      return safeSetTimeout(func);
    },
    safeRequestAnimationFrame: function(func) {
      {{{ runtimeKeepalivePush() }}}
      return Browser.requestAnimationFrame(function() {
        {{{ runtimeKeepalivePop() }}}
        callUserCallback(func);
      });
    },

    getMimetype: function(name) {
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

    getUserMedia: function(func) {
      if (!window.getUserMedia) {
        window.getUserMedia = navigator['getUserMedia'] ||
                              navigator['mozGetUserMedia'];
      }
      window.getUserMedia(func);
    },


    getMovementX: function(event) {
      return event['movementX'] ||
             event['mozMovementX'] ||
             event['webkitMovementX'] ||
             0;
    },

    getMovementY: function(event) {
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
    getMouseWheelDelta: function(event) {
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

    calculateMouseEvent: function(event) { // event should be mousemove, mousedown or mouseup
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

        // check if SDL is available
        if (typeof SDL != "undefined") {
          Browser.mouseX = SDL.mouseX + Browser.mouseMovementX;
          Browser.mouseY = SDL.mouseY + Browser.mouseMovementY;
        } else {
          // just add the mouse delta to the current absolut mouse position
          // FIXME: ideally this should be clamped against the canvas size and zero
          Browser.mouseX += Browser.mouseMovementX;
          Browser.mouseY += Browser.mouseMovementY;
        }
      } else {
        // Otherwise, calculate the movement based on the changes
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

        if (event.type === 'touchstart' || event.type === 'touchend' || event.type === 'touchmove') {
          var touch = event.touch;
          if (touch === undefined) {
            return; // the "touch" property is only defined in SDL

          }
          var adjustedX = touch.pageX - (scrollX + rect.left);
          var adjustedY = touch.pageY - (scrollY + rect.top);

          adjustedX = adjustedX * (cw / rect.width);
          adjustedY = adjustedY * (ch / rect.height);

          var coords = { x: adjustedX, y: adjustedY };

          if (event.type === 'touchstart') {
            Browser.lastTouches[touch.identifier] = coords;
            Browser.touches[touch.identifier] = coords;
          } else if (event.type === 'touchend' || event.type === 'touchmove') {
            var last = Browser.touches[touch.identifier];
            if (!last) last = coords;
            Browser.lastTouches[touch.identifier] = last;
            Browser.touches[touch.identifier] = coords;
          }
          return;
        }

        var x = event.pageX - (scrollX + rect.left);
        var y = event.pageY - (scrollY + rect.top);

        // the canvas might be CSS-scaled compared to its backbuffer;
        // SDL-using content will want mouse coordinates in terms
        // of backbuffer units.
        x = x * (cw / rect.width);
        y = y * (ch / rect.height);

        Browser.mouseMovementX = x - Browser.mouseX;
        Browser.mouseMovementY = y - Browser.mouseY;
        Browser.mouseX = x;
        Browser.mouseY = y;
      }
    },

    resizeListeners: [],

    updateResizeListeners: function() {
      var canvas = Module['canvas'];
      Browser.resizeListeners.forEach(function(listener) {
        listener(canvas.width, canvas.height);
      });
    },

    setCanvasSize: function(width, height, noUpdates) {
      var canvas = Module['canvas'];
      Browser.updateCanvasDimensions(canvas, width, height);
      if (!noUpdates) Browser.updateResizeListeners();
    },

    windowedWidth: 0,
    windowedHeight: 0,
    setFullscreenCanvasSize: function() {
      // check if SDL is available
      if (typeof SDL != "undefined") {
        var flags = {{{ makeGetValue('SDL.screen', '0', 'u32') }}};
        flags = flags | 0x00800000; // set SDL_FULLSCREEN flag
        {{{ makeSetValue('SDL.screen', '0', 'flags', 'i32') }}};
      }
      Browser.updateCanvasDimensions(Module['canvas']);
      Browser.updateResizeListeners();
    },

    setWindowedCanvasSize: function() {
      // check if SDL is available
      if (typeof SDL != "undefined") {
        var flags = {{{ makeGetValue('SDL.screen', '0', 'u32') }}};
        flags = flags & ~0x00800000; // clear SDL_FULLSCREEN flag
        {{{ makeSetValue('SDL.screen', '0', 'flags', 'i32') }}};
      }
      Browser.updateCanvasDimensions(Module['canvas']);
      Browser.updateResizeListeners();
    },

    updateCanvasDimensions : function(canvas, wNative, hNative) {
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

  emscripten_run_preload_plugins__deps: ['$PATH',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush', '$runtimeKeepalivePop',
#endif
  ],
  emscripten_run_preload_plugins__proxy: 'sync',
  emscripten_run_preload_plugins__sig: 'iiii',
  emscripten_run_preload_plugins: function(file, onload, onerror) {
    {{{ runtimeKeepalivePush() }}}

    var _file = UTF8ToString(file);
    var data = FS.analyzePath(_file);
    if (!data.exists) return -1;
    FS.createPreloadedFile(
      PATH.dirname(_file),
      PATH.basename(_file),
      new Uint8Array(data.object.contents), true, true,
      function() {
        {{{ runtimeKeepalivePop() }}}
        if (onload) {{{ makeDynCall('vi', 'onload') }}}(file);
      },
      function() {
        {{{ runtimeKeepalivePop() }}}
        if (onerror) {{{ makeDynCall('vi', 'onerror') }}}(file);
      },
      true // don'tCreateFile - it's already there
    );
    return 0;
  },

#if !MINIMAL_RUNTIME
  emscripten_run_preload_plugins_data__deps: ['$runtimeKeepalivePush', '$runtimeKeepalivePop'],
#endif
  emscripten_run_preload_plugins_data__proxy: 'sync',
  emscripten_run_preload_plugins_data__sig: 'viiiiii',
  emscripten_run_preload_plugins_data: function(data, size, suffix, arg, onload, onerror) {
    {{{ runtimeKeepalivePush() }}}

    var _suffix = UTF8ToString(suffix);
    if (!Browser.asyncPrepareDataCounter) Browser.asyncPrepareDataCounter = 0;
    var name = 'prepare_data_' + (Browser.asyncPrepareDataCounter++) + '.' + _suffix;
    var lengthAsUTF8 = lengthBytesUTF8(name);
    var cname = _malloc(lengthAsUTF8+1);
    stringToUTF8(name, cname, lengthAsUTF8+1);
    FS.createPreloadedFile(
      '/',
      name,
      {{{ makeHEAPView('U8', 'data', 'data + size') }}},
      true, true,
      function() {
        {{{ runtimeKeepalivePop() }}}
        if (onload) {{{ makeDynCall('vii', 'onload') }}}(arg, cname);
      },
      function() {
        {{{ runtimeKeepalivePop() }}}
        if (onerror) {{{ makeDynCall('vi', 'onerror') }}}(arg);
      },
      true // don'tCreateFile - it's already there
    );
  },

  // Callable from pthread, executes in pthread context.
  emscripten_async_run_script__deps: ['emscripten_run_script', '$safeSetTimeout'],
  emscripten_async_run_script: function(script, millis) {
    // TODO: cache these to avoid generating garbage
    safeSetTimeout(function() {
      _emscripten_run_script(script);
    }, millis);
  },

  // TODO: currently not callable from a pthread, but immediately calls onerror() if not on main thread.
  emscripten_async_load_script: function(url, onload, onerror) {
    onload = {{{ makeDynCall('v', 'onload') }}};
    onerror = {{{ makeDynCall('v', 'onerror') }}};

#if USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) {
      err('emscripten_async_load_script("' + UTF8ToString(url) + '") failed, emscripten_async_load_script is currently not available in pthreads!');
      return onerror ? onerror() : undefined;
    }
#endif
    {{{ runtimeKeepalivePush() }}}

    assert(runDependencies === 0, 'async_load_script must be run when no other dependencies are active');
    var script = document.createElement('script');
    script.onload = function script_onload() {
      {{{ runtimeKeepalivePop() }}}
      if (onload) {
        if (runDependencies > 0) {
          dependenciesFulfilled = onload;
        } else {
          onload();
        }
      }
    };
    script.onerror = function() {
      {{{ runtimeKeepalivePop() }}}
      if (onerror) onerror();
    };
    script.src = UTF8ToString(url);
    document.body.appendChild(script);
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_get_main_loop_timing__sig: 'vii',
  emscripten_get_main_loop_timing: function(mode, value) {
    if (mode) {{{ makeSetValue('mode', 0, 'Browser.mainLoop.timingMode', 'i32') }}};
    if (value) {{{ makeSetValue('value', 0, 'Browser.mainLoop.timingValue', 'i32') }}};
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_set_main_loop_timing__sig: 'iii',
  emscripten_set_main_loop_timing: function(mode, value) {
    Browser.mainLoop.timingMode = mode;
    Browser.mainLoop.timingValue = value;

    if (!Browser.mainLoop.func) {
#if ASSERTIONS
      err('emscripten_set_main_loop_timing: Cannot set timing mode for main loop since a main loop does not exist! Call emscripten_set_main_loop first to set one up.');
#endif
      return 1; // Return non-zero on failure, can't set timing mode when there is no main loop.
    }

    if (!Browser.mainLoop.running) {
      {{{ runtimeKeepalivePush() }}}
      Browser.mainLoop.running = true;
    }
    if (mode == 0 /*EM_TIMING_SETTIMEOUT*/) {
      Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_setTimeout() {
        var timeUntilNextTick = Math.max(0, Browser.mainLoop.tickStartTime + value - _emscripten_get_now())|0;
        setTimeout(Browser.mainLoop.runner, timeUntilNextTick); // doing this each time means that on exception, we stop
      };
      Browser.mainLoop.method = 'timeout';
    } else if (mode == 1 /*EM_TIMING_RAF*/) {
      Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_rAF() {
        Browser.requestAnimationFrame(Browser.mainLoop.runner);
      };
      Browser.mainLoop.method = 'rAF';
    } else if (mode == 2 /*EM_TIMING_SETIMMEDIATE*/) {
      if (typeof setImmediate == 'undefined') {
        // Emulate setImmediate. (note: not a complete polyfill, we don't emulate clearImmediate() to keep code size to minimum, since not needed)
        var setImmediates = [];
        var emscriptenMainLoopMessageId = 'setimmediate';
        var Browser_setImmediate_messageHandler = function(/** @type {Event} */ event) {
          // When called in current thread or Worker, the main loop ID is structured slightly different to accommodate for --proxy-to-worker runtime listening to Worker events,
          // so check for both cases.
          if (event.data === emscriptenMainLoopMessageId || event.data.target === emscriptenMainLoopMessageId) {
            event.stopPropagation();
            setImmediates.shift()();
          }
        }
        addEventListener("message", Browser_setImmediate_messageHandler, true);
        setImmediate = /** @type{function(function(): ?, ...?): number} */(function Browser_emulated_setImmediate(func) {
          setImmediates.push(func);
          if (ENVIRONMENT_IS_WORKER) {
            if (Module['setImmediates'] === undefined) Module['setImmediates'] = [];
            Module['setImmediates'].push(func);
            postMessage({target: emscriptenMainLoopMessageId}); // In --proxy-to-worker, route the message via proxyClient.js
          } else postMessage(emscriptenMainLoopMessageId, "*"); // On the main thread, can just send the message to itself.
        })
      }
      Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_setImmediate() {
        setImmediate(Browser.mainLoop.runner);
      };
      Browser.mainLoop.method = 'immediate';
    }
    return 0;
  },

  emscripten_set_main_loop__deps: ['$setMainLoop'],
  emscripten_set_main_loop__sig: 'viii',
  emscripten_set_main_loop: function(func, fps, simulateInfiniteLoop) {
    var browserIterationFunc = {{{ makeDynCall('v', 'func') }}};
    setMainLoop(browserIterationFunc, fps, simulateInfiniteLoop);
  },

  // Runs natively in pthread, no __proxy needed.
  $setMainLoop__deps: [
    'emscripten_set_main_loop_timing', 'emscripten_get_now',
#if OFFSCREEN_FRAMEBUFFER
    'emscripten_webgl_commit_frame',
#endif
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush',
    '$maybeExit',
#endif
  ],
  $setMainLoop__docs: `
  /**
   * @param {number=} arg
   * @param {boolean=} noSetTiming
   */`,
  $setMainLoop: function(browserIterationFunc, fps, simulateInfiniteLoop, arg, noSetTiming) {
    assert(!Browser.mainLoop.func, 'emscripten_set_main_loop: there can only be one main loop function at once: call emscripten_cancel_main_loop to cancel the previous one before setting a new one with different parameters.');

    Browser.mainLoop.func = browserIterationFunc;
    Browser.mainLoop.arg = arg;

#if USE_CLOSURE_COMPILER
    // Closure compiler bug(?): Closure does not see that the assignment
    //   var thisMainLoopId = Browser.mainLoop.currentlyRunningMainloop
    // is a value copy of a number (even with the JSDoc @type annotation)
    // but optimizeis the code as if the assignment was a reference assignment,
    // which results in Browser.mainLoop.pause() not working. Hence use a
    // workaround to make Closure believe this is a value copy that should occur:
    // (TODO: Minimize this down to a small test case and report - was unable
    // to reproduce in a small written test case)
    /** @type{number} */
    var thisMainLoopId = (function(){return Browser.mainLoop.currentlyRunningMainloop; })();
#else
    var thisMainLoopId = Browser.mainLoop.currentlyRunningMainloop;
#endif
    function checkIsRunning() {
      if (thisMainLoopId < Browser.mainLoop.currentlyRunningMainloop) {
#if RUNTIME_DEBUG
        err('main loop exiting..');
#endif
        {{{ runtimeKeepalivePop() }}}
#if !MINIMAL_RUNTIME
        maybeExit();
#endif
        return false;
      }
      return true;
    }

    // We create the loop runner here but it is not actually running until
    // _emscripten_set_main_loop_timing is called (which might happen a
    // later time).  This member signifies that the current runner has not
    // yet been started so that we can call runtimeKeepalivePush when it
    // gets it timing set for the first time.
    Browser.mainLoop.running = false;
    Browser.mainLoop.runner = function Browser_mainLoop_runner() {
      if (ABORT) return;
      if (Browser.mainLoop.queue.length > 0) {
        var start = Date.now();
        var blocker = Browser.mainLoop.queue.shift();
        blocker.func(blocker.arg);
        if (Browser.mainLoop.remainingBlockers) {
          var remaining = Browser.mainLoop.remainingBlockers;
          var next = remaining%1 == 0 ? remaining-1 : Math.floor(remaining);
          if (blocker.counted) {
            Browser.mainLoop.remainingBlockers = next;
          } else {
            // not counted, but move the progress along a tiny bit
            next = next + 0.5; // do not steal all the next one's progress
            Browser.mainLoop.remainingBlockers = (8*remaining + next)/9;
          }
        }
        out('main loop blocker "' + blocker.name + '" took ' + (Date.now() - start) + ' ms'); //, left: ' + Browser.mainLoop.remainingBlockers);
        Browser.mainLoop.updateStatus();

        // catches pause/resume main loop from blocker execution
        if (!checkIsRunning()) return;

        setTimeout(Browser.mainLoop.runner, 0);
        return;
      }

      // catch pauses from non-main loop sources
      if (!checkIsRunning()) return;

      // Implement very basic swap interval control
      Browser.mainLoop.currentFrameNumber = Browser.mainLoop.currentFrameNumber + 1 | 0;
      if (Browser.mainLoop.timingMode == 1/*EM_TIMING_RAF*/ && Browser.mainLoop.timingValue > 1 && Browser.mainLoop.currentFrameNumber % Browser.mainLoop.timingValue != 0) {
        // Not the scheduled time to render this frame - skip.
        Browser.mainLoop.scheduler();
        return;
      } else if (Browser.mainLoop.timingMode == 0/*EM_TIMING_SETTIMEOUT*/) {
        Browser.mainLoop.tickStartTime = _emscripten_get_now();
      }

      // Signal GL rendering layer that processing of a new frame is about to start. This helps it optimize
      // VBO double-buffering and reduce GPU stalls.
#if FULL_ES2 || LEGACY_GL_EMULATION
      GL.newRenderingFrameStarted();
#endif

#if USE_PTHREADS && OFFSCREEN_FRAMEBUFFER && GL_SUPPORT_EXPLICIT_SWAP_CONTROL
      // If the current GL context is a proxied regular WebGL context, and was initialized with implicit swap mode on the main thread, and we are on the parent thread,
      // perform the swap on behalf of the user.
      if (typeof GL != 'undefined' && GL.currentContext && GL.currentContextIsProxied) {
        var explicitSwapControl = {{{ makeGetValue('GL.currentContext', 0, 'i32') }}};
        if (!explicitSwapControl) _emscripten_webgl_commit_frame();
      }
#endif

#if OFFSCREENCANVAS_SUPPORT
      // If the current GL context is an OffscreenCanvas, but it was initialized with implicit swap mode, perform the swap on behalf of the user.
      if (typeof GL != 'undefined' && GL.currentContext && !GL.currentContextIsProxied && !GL.currentContext.attributes.explicitSwapControl && GL.currentContext.GLctx.commit) {
        GL.currentContext.GLctx.commit();
      }
#endif

#if ASSERTIONS
      if (Browser.mainLoop.method === 'timeout' && Module.ctx) {
        warnOnce('Looks like you are rendering without using requestAnimationFrame for the main loop. You should use 0 for the frame rate in emscripten_set_main_loop in order to use requestAnimationFrame, as that can greatly improve your frame rates!');
        Browser.mainLoop.method = ''; // just warn once per call to set main loop
      }
#endif

      Browser.mainLoop.runIter(browserIterationFunc);

#if STACK_OVERFLOW_CHECK
      checkStackCookie();
#endif

      // catch pauses from the main loop itself
      if (!checkIsRunning()) return;

      // Queue new audio data. This is important to be right after the main loop invocation, so that we will immediately be able
      // to queue the newest produced audio samples.
      // TODO: Consider adding pre- and post- rAF callbacks so that GL.newRenderingFrameStarted() and SDL.audio.queueNewAudioData()
      //       do not need to be hardcoded into this function, but can be more generic.
      if (typeof SDL == 'object' && SDL.audio && SDL.audio.queueNewAudioData) SDL.audio.queueNewAudioData();

      Browser.mainLoop.scheduler();
    }

    if (!noSetTiming) {
      if (fps && fps > 0) _emscripten_set_main_loop_timing(0/*EM_TIMING_SETTIMEOUT*/, 1000.0 / fps);
      else _emscripten_set_main_loop_timing(1/*EM_TIMING_RAF*/, 1); // Do rAF by rendering each frame (no decimating)

      Browser.mainLoop.scheduler();
    }

    if (simulateInfiniteLoop) {
      throw 'unwind';
    }
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_set_main_loop_arg__deps: ['$setMainLoop'],
  emscripten_set_main_loop_arg__sig: 'viiii',
  emscripten_set_main_loop_arg: function(func, arg, fps, simulateInfiniteLoop) {
    var browserIterationFunc = function() { {{{ makeDynCall('vi', 'func') }}}(arg); };
    setMainLoop(browserIterationFunc, fps, simulateInfiniteLoop, arg);
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_cancel_main_loop__sig: 'v',
  emscripten_cancel_main_loop: function() {
    Browser.mainLoop.pause();
    Browser.mainLoop.func = null;
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_pause_main_loop__sig: 'v',
  emscripten_pause_main_loop: function() {
    Browser.mainLoop.pause();
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_resume_main_loop__sig: 'v',
  emscripten_resume_main_loop: function() {
    Browser.mainLoop.resume();
  },

  // Runs natively in pthread, no __proxy needed.
  _emscripten_push_main_loop_blocker: function(func, arg, name) {
    Browser.mainLoop.queue.push({ func: function() {
      {{{ makeDynCall('vi', 'func') }}}(arg);
    }, name: UTF8ToString(name), counted: true });
    Browser.mainLoop.updateStatus();
  },

  // Runs natively in pthread, no __proxy needed.
  _emscripten_push_uncounted_main_loop_blocker: function(func, arg, name) {
    Browser.mainLoop.queue.push({ func: function() {
      {{{ makeDynCall('vi', 'func') }}}(arg);
    }, name: UTF8ToString(name), counted: false });
    Browser.mainLoop.updateStatus();
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_set_main_loop_expected_blockers__sig: 'vi',
  emscripten_set_main_loop_expected_blockers: function(num) {
    Browser.mainLoop.expectedBlockers = num;
    Browser.mainLoop.remainingBlockers = num;
    Browser.mainLoop.updateStatus();
  },

  // Runs natively in pthread, no __proxy needed.
  emscripten_async_call__sig: 'vppi',
  emscripten_async_call__deps: ['$safeSetTimeout'],
  emscripten_async_call: function(func, arg, millis) {
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
      Browser.safeRequestAnimationFrame(wrapper);
    }
  },

  emscripten_get_window_title__proxy: 'sync',
  emscripten_get_window_title__sig: 'iv',
  emscripten_get_window_title: function() {
    var buflen = 256;

    if (!_emscripten_get_window_title.buffer) {
      _emscripten_get_window_title.buffer = _malloc(buflen);
    }

    stringToUTF8(document.title, _emscripten_get_window_title.buffer, buflen);

    return _emscripten_get_window_title.buffer;
  },

  emscripten_set_window_title__proxy: 'sync',
  emscripten_set_window_title__sig: 'vi',
  emscripten_set_window_title: function(title) {
    setWindowTitle(UTF8ToString(title));
  },

  emscripten_get_screen_size__proxy: 'sync',
  emscripten_get_screen_size__sig: 'vii',
  emscripten_get_screen_size: function(width, height) {
    {{{ makeSetValue('width', '0', 'screen.width', 'i32') }}};
    {{{ makeSetValue('height', '0', 'screen.height', 'i32') }}};
  },

  emscripten_hide_mouse__proxy: 'sync',
  emscripten_hide_mouse__sig: 'v',
  emscripten_hide_mouse: function() {
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
  emscripten_set_canvas_size__sig: 'vii',
  emscripten_set_canvas_size: function(width, height) {
    Browser.setCanvasSize(width, height);
  },

  emscripten_get_canvas_size__proxy: 'sync',
  emscripten_get_canvas_size__sig: 'viii',
  emscripten_get_canvas_size: function(width, height, isFullscreen) {
    var canvas = Module['canvas'];
    {{{ makeSetValue('width', '0', 'canvas.width', 'i32') }}};
    {{{ makeSetValue('height', '0', 'canvas.height', 'i32') }}};
    {{{ makeSetValue('isFullscreen', '0', 'Browser.isFullscreen ? 1 : 0', 'i32') }}};
  },

  // To avoid creating worker parent->child chains, always proxies to execute on the main thread.
  emscripten_create_worker__proxy: 'sync',
  emscripten_create_worker__sig: 'ii',
  emscripten_create_worker: function(url) {
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

  emscripten_destroy_worker__proxy: 'sync',
  emscripten_destroy_worker__sig: 'vi',
  emscripten_destroy_worker: function(id) {
    var info = Browser.workers[id];
    info.worker.terminate();
    if (info.buffer) _free(info.buffer);
    Browser.workers[id] = null;
  },

  emscripten_call_worker__proxy: 'sync',
  emscripten_call_worker__sig: 'viiiiii',
#if !MINIMAL_RUNTIME
  emscripten_call_worker__deps: ['$runtimeKeepalivePush'],
#endif
  emscripten_call_worker: function(id, funcName, data, size, callback, arg) {
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
        func: {{{ makeDynCall('viii', 'callback') }}},
        arg: arg
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
  emscripten_worker_respond_provisionally__sig: 'vii',
  emscripten_worker_respond_provisionally: function(data, size) {
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
  emscripten_worker_respond__sig: 'vii',
  emscripten_worker_respond: function(data, size) {
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
  emscripten_get_worker_queue_size__sig: 'i',
  emscripten_get_worker_queue_size: function(id) {
    var info = Browser.workers[id];
    if (!info) return -1;
    return info.awaited;
  },

  emscripten_get_preloaded_image_data__deps: ['$PATH_FS'],
  emscripten_get_preloaded_image_data__proxy: 'sync',
  emscripten_get_preloaded_image_data__sig: 'iiii',
  emscripten_get_preloaded_image_data: function(path, w, h) {
    if ((path | 0) === path) path = UTF8ToString(path);

    path = PATH_FS.resolve(path);

    var canvas = /** @type {HTMLCanvasElement} */(preloadedImages[path]);
    if (canvas) {
      var ctx = canvas.getContext("2d");
      var image = ctx.getImageData(0, 0, canvas.width, canvas.height);
      var buf = _malloc(canvas.width * canvas.height * 4);

      HEAPU8.set(image.data, buf);

      {{{ makeSetValue('w', '0', 'canvas.width', 'i32') }}};
      {{{ makeSetValue('h', '0', 'canvas.height', 'i32') }}};
      return buf;
    }

    return 0;
  },

  emscripten_get_preloaded_image_data_from_FILE__deps: ['emscripten_get_preloaded_image_data'],
  emscripten_get_preloaded_image_data_from_FILE__proxy: 'sync',
  emscripten_get_preloaded_image_data_from_FILE__sig: 'iiii',
  emscripten_get_preloaded_image_data_from_FILE: function(file, w, h) {
    var fd = Module['_fileno'](file);
    var stream = FS.getStream(fd);
    if (stream) {
      return _emscripten_get_preloaded_image_data(stream.path, w, h);
    }

    return 0;
  }
};

autoAddDeps(LibraryBrowser, '$Browser');

mergeInto(LibraryManager.library, LibraryBrowser);

/* Useful stuff for browser debugging

function slowLog(label, text) {
  if (!slowLog.labels) slowLog.labels = {};
  if (!slowLog.labels[label]) slowLog.labels[label] = 0;
  var now = Date.now();
  if (now - slowLog.labels[label] > 1000) {
    out(label + ': ' + text);
    slowLog.labels[label] = now;
  }
}

*/
