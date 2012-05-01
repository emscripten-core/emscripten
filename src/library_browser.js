//"use strict";

// Utilities for browser environments

mergeInto(LibraryManager.library, {
  $Browser: {
    pointerLock: false,
    asyncCalls: {},

    createContext: function(canvas, useWebGL) {
#if !USE_TYPED_ARRAYS
      if (useWebGL) {
        Module.print('(USE_TYPED_ARRAYS needs to be enabled for WebGL)');
        return null;
      }
#endif
      try {
        var ctx = canvas.getContext(useWebGL ? 'experimental-webgl' : '2d');
        if (!ctx) throw ':(';
      } catch (e) {
        Module.print('Could not create canvas - ' + e);
        return null;
      }
      if (useWebGL) {
#if GL_DEBUG
        // Useful to debug native webgl apps: var Module = { printErr: function(x) { console.log(x) } };
        var tempCtx = ctx;
        var wrapper = {};
        for (var prop in tempCtx) {
          (function(prop) {
            switch (typeof tempCtx[prop]) {
              case 'function': {
                wrapper[prop] = function() {
                  if (GL.debug) {
                    var printArgs = Array.prototype.slice.call(arguments).map(Runtime.prettyPrint);
                    console.log('[gl_f:' + prop + ':' + printArgs + ']');
                  }
                  var ret = tempCtx[prop].apply(tempCtx, arguments);
                  if (GL.debug && typeof ret != 'undefined') {
                    console.log('[     gl:' + prop + ':return:' + Runtime.prettyPrint(ret) + ']');
                  }
                  return ret;
                }
                break;
              }
              case 'number': case 'string': {
                wrapper.__defineGetter__(prop, function() {
                  //console.log('[gl_g:' + prop + ':' + tempCtx[prop] + ']');
                  return tempCtx[prop];
                });
                wrapper.__defineSetter__(prop, function(value) {
                  if (GL.debug) {
                    console.log('[gl_s:' + prop + ':' + value + ']');
                  }
                  tempCtx[prop] = value;
                });
                break;
              }
            }
          })(prop);
        }
        ctx = wrapper;
#endif
        // Set the background of the WebGL canvas to black
        canvas.style.backgroundColor = "black";
      }
      return ctx;
    },

    requestFullScreen: function() {
      var canvas = Module.canvas;
      function fullScreenChange() {
        if (document.webkitFullScreenElement === canvas ||
            document.mozFullScreenElement === canvas ||
            document.fullScreenElement === canvas) {
          canvas.requestPointerLock = canvas.requestPointerLock ||
                                      canvas.mozRequestPointerLock ||
                                      canvas.webkitRequestPointerLock;
          canvas.requestPointerLock();
        }
      }

      document.addEventListener('fullscreenchange', fullScreenChange, false);
      document.addEventListener('mozfullscreenchange', fullScreenChange, false);
      document.addEventListener('webkitfullscreenchange', fullScreenChange, false);

      function pointerLockChange() {
        Browser.pointerLock = document.pointerLockElement === canvas ||
                              document.mozPointerLockElement === canvas ||
                              document.webkitPointerLockElement === canvas;
      }

      document.addEventListener('pointerlockchange', pointerLockChange, false);
      document.addEventListener('mozpointerlockchange', pointerLockChange, false);
      document.addEventListener('webkitpointerlockchange', pointerLockChange, false);

      canvas.requestFullScreen = canvas.requestFullScreen ||
                                 canvas.mozRequestFullScreen ||
                                 canvas.webkitRequestFullScreen;
      canvas.requestFullScreen(); 
    },

    requestAnimationFrame: function(func) {
      if (!window.requestAnimationFrame) {
        window.requestAnimationFrame = window['requestAnimationFrame'] ||
                                       window['mozRequestAnimationFrame'] ||
                                       window['webkitRequestAnimationFrame'] ||
                                       window['msRequestAnimationFrame'] ||
                                       window['oRequestAnimationFrame'] ||
                                       window['setTimeout'];
      }
      window.requestAnimationFrame(func);
    },

    getMovementX: function(delta, event) {
      if (!Browser.pointerLock) return delta;
      return event.movementX ||
             event.mozMovementX ||
             event.webkitMovementX ||
             0; // delta;
    },

    getMovementY: function(delta, event) {
      if (!Browser.pointerLock) return delta;
      return event.movementY ||
             event.mozMovementY ||
             event.webkitMovementY ||
             0; // delta;
    }
  },

  emscripten_async_run_script__deps: ['emscripten_run_script'],
  emscripten_async_run_script: function(script, millis) {
    Module['noExitRuntime'] = true;

    // TODO: cache these to avoid generating garbage
    setTimeout(function() {
      _emscripten_run_script(script);
    }, millis);
  },

  emscripten_set_main_loop: function(func, fps) {
    Module['noExitRuntime'] = true;

    _emscripten_set_main_loop.cancel = false;
    var jsFunc = FUNCTION_TABLE[func];

    if (fps && fps > 0) {
      function doOne() {
        if (_emscripten_set_main_loop.cancel) return;
        jsFunc();
        setTimeout(doOne, 1000/fps); // doing this each time means that on exception, we stop
      }
      setTimeout(doOne, 1000/fps);
    } else {
      function doOneRAF() {
        if (_emscripten_set_main_loop.cancel) return;
        jsFunc();
        Browser.requestAnimationFrame(doOneRAF);
      }
      Browser.requestAnimationFrame(doOneRAF);
    }
  },

  emscripten_cancel_main_loop: function(func) {
    _emscripten_set_main_loop.cancel = true;
  },

  emscripten_async_call: function(func, millis) {
    Module['noExitRuntime'] = true;

    if (!Browser.asyncCalls[func]) {
      Browser.asyncCalls[func] = function() {
        FUNCTION_TABLE[func]();
      };
    }
    if (millis >= 0) {
      setTimeout(Browser.asyncCalls[func], millis);
    } else {
      Browser.requestAnimationFrame(Browser.asyncCalls[func]);
    }
  }
});

/* Useful stuff for browser debugging

function slowLog(label, text) {
  if (!slowLog.labels) slowLog.labels = {};
  if (!slowLog.labels[label]) slowLog.labels[label] = 0;
  var now = Date.now();
  if (now - slowLog.labels[label] > 1000) {
    Module.print(label + ': ' + text);
    slowLog.labels[label] = now;
  }
}

*/

