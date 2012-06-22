//"use strict";

// Utilities for browser environments

mergeInto(LibraryManager.library, {
  $Browser__postset: 'Module["requestFullScreen"] = function() { Browser.requestFullScreen() };\n', // export requestFullScreen
  $Browser: {
    mainLoop: {
      scheduler: null,
      shouldPause: false,
      paused: false
    },
    pointerLock: false,
    moduleContextCreatedCallbacks: [],

    createContext: function(canvas, useWebGL, setInModule) {
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
                    Module.printErr('[gl_f:' + prop + ':' + printArgs + ']');
                  }
                  var ret = tempCtx[prop].apply(tempCtx, arguments);
                  if (GL.debug && typeof ret != 'undefined') {
                    Module.printErr('[     gl:' + prop + ':return:' + Runtime.prettyPrint(ret) + ']');
                  }
                  return ret;
                }
                break;
              }
              case 'number': case 'string': {
                wrapper.__defineGetter__(prop, function() {
                  //Module.printErr('[gl_g:' + prop + ':' + tempCtx[prop] + ']');
                  return tempCtx[prop];
                });
                wrapper.__defineSetter__(prop, function(value) {
                  if (GL.debug) {
                    Module.printErr('[gl_s:' + prop + ':' + value + ']');
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

        // Warn on context loss
        canvas.addEventListener('webglcontextlost', function(event) {
          alert('WebGL context lost. You will need to reload the page.');
        }, false);
      }
      if (setInModule) {
        Module.ctx = ctx;
        Module.useWebGL = useWebGL;
        Browser.moduleContextCreatedCallbacks.forEach(function(callback) { callback() });
      }
      return ctx;
    },

    requestFullScreen: function() {
      var canvas = Module.canvas;
      function fullScreenChange() {
        if (Module['onFullScreen']) Module['onFullScreen']();
        if (document['webkitFullScreenElement'] === canvas ||
            document['mozFullScreenElement'] === canvas ||
            document['fullScreenElement'] === canvas) {
          canvas.requestPointerLock = canvas['requestPointerLock'] ||
                                      canvas['mozRequestPointerLock'] ||
                                      canvas['webkitRequestPointerLock'];
          canvas.requestPointerLock();
        }
      }

      document.addEventListener('fullscreenchange', fullScreenChange, false);
      document.addEventListener('mozfullscreenchange', fullScreenChange, false);
      document.addEventListener('webkitfullscreenchange', fullScreenChange, false);

      function pointerLockChange() {
        Browser.pointerLock = document['pointerLockElement'] === canvas ||
                              document['mozPointerLockElement'] === canvas ||
                              document['webkitPointerLockElement'] === canvas;
      }

      document.addEventListener('pointerlockchange', pointerLockChange, false);
      document.addEventListener('mozpointerlockchange', pointerLockChange, false);
      document.addEventListener('webkitpointerlockchange', pointerLockChange, false);

      canvas.requestFullScreen = canvas['requestFullScreen'] ||
                                 canvas['mozRequestFullScreen'] ||
                                 (canvas['webkitRequestFullScreen'] ? function() { canvas['webkitRequestFullScreen'](Element['ALLOW_KEYBOARD_INPUT']) } : null);
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

    xhrLoad: function(url, onload, onerror) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, true);
      xhr.responseType = 'arraybuffer';
      xhr.onload = function() {
        if (xhr.status == 200) {
          onload(xhr.response);
        } else {
          onerror();
        }
      };
      xhr.onerror = onerror;
      xhr.send(null);
    },

    asyncLoad: function(url, callback) {
      Browser.xhrLoad(url, function(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file "' + url + '" failed (no arrayBuffer).');
        callback(new Uint8Array(arrayBuffer));
        removeRunDependency();
      }, function(event) {
        throw 'Loading data file "' + url + '" failed.';
      });
      addRunDependency();
    }
  },

  emscripten_async_wget: function(url, file, onload, onerror) {
    url = Pointer_stringify(url);

    Browser.xhrLoad(url, function(response) {
      var absolute = Pointer_stringify(file);
      var index = absolute.lastIndexOf('/');
      FS.createDataFile(
        absolute.substr(0, index),
        absolute.substr(index +1), 
        new Uint8Array(response), 
        true, true);

      if (onload) {
        FUNCTION_TABLE[onload](file);
      }
    }, function(event) {
      if (onerror) {
        FUNCTION_TABLE[onerror](file);
      }
    });
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

    var jsFunc = FUNCTION_TABLE[func];
    var wrapper = function() {
      if (Browser.mainLoop.shouldPause) {
        // catch pauses from non-main loop sources
        Browser.mainLoop.paused = true;
        Browser.mainLoop.shouldPause = false;
        return;
      }
      jsFunc();
      if (Browser.mainLoop.shouldPause) {
        // catch pauses from the main loop itself
        Browser.mainLoop.paused = true;
        Browser.mainLoop.shouldPause = false;
        return;
      }
      Browser.mainLoop.scheduler();
    }
    if (fps && fps > 0) {
      Browser.mainLoop.scheduler = function() {
        setTimeout(wrapper, 1000/fps); // doing this each time means that on exception, we stop
      }
    } else {
      Browser.mainLoop.scheduler = function() {
        Browser.requestAnimationFrame(wrapper);
      }
    }
    Browser.mainLoop.scheduler();
  },

  emscripten_cancel_main_loop: function(func) {
    Browser.mainLoop.scheduler = null;
    Browser.mainLoop.shouldPause = true;
  },

  emscripten_pause_main_loop: function(func) {
    Browser.mainLoop.shouldPause = true;
  },

  emscripten_resume_main_loop: function(func) {
    if (Browser.mainLoop.paused) {
      Browser.mainLoop.paused = false;
      Browser.mainLoop.scheduler();
    }
    Browser.mainLoop.shouldPause = false;
  },

  emscripten_async_call: function(func, millis) {
    Module['noExitRuntime'] = true;

    var asyncCall = Runtime.getFuncWrapper(func);
    if (millis >= 0) {
      setTimeout(asyncCall, millis);
    } else {
      Browser.requestAnimationFrame(asyncCall);
    }
  },

  emscripten_hide_mouse: function() {
    var styleSheet = document.styleSheets[0];
    var rules = styleSheet.cssRules;
    for (var i = 0; i < rules.length; i++) {
      if (rules[i].cssText.substr(0, 5) == 'canvas') {
        styleSheet.deleteRule(i);
        i--;
      }
    }
    styleSheet.insertRule('canvas.emscripten { border: 1px solid black; cursor: none; }', 0);
  },

  emscripten_set_canvas_size: function(width, height) {
    Module['canvas'].width = width;
    Module['canvas'].height = height;
  },

  emscripten_get_now: function() {
    if (window['performance'] && window['performance']['now']) {
      return window['performance']['now']();
    } else {
      return Date.now();
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

