//"use strict";

// Utilities for browser environments

mergeInto(LibraryManager.library, {
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

    fps = fps || 60; // TODO: use requestAnimationFrame
    _emscripten_set_main_loop.cancel = false;
    var jsFunc = FUNCTION_TABLE[func];
    function doOne() {
      if (_emscripten_set_main_loop.cancel) return;
      jsFunc();
      setTimeout(doOne, 1000/fps); // doing this each time means that on exception, we stop
    }
    setTimeout(doOne, 1000/fps);
  },

  emscripten_cancel_main_loop: function(func) {
    _emscripten_set_main_loop.cancel = true;
  },

  emscripten_async_call: function(func, millis) {
    Module['noExitRuntime'] = true;

    // TODO: cache these to avoid generating garbage
    setTimeout(function() {
      FUNCTION_TABLE[func]();
    }, millis);
  },

  $Browser: {
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

    // Given binary data for an image, in a format like PNG or JPG, we convert it
    // to flat pixel data. We do so using the browser's native code.
    // This is deprecated, it is preferred to load binary files, createObjectURL, etc.,
    // see the sdl_* tests.
    /*decodeImage: function(pixels, format) {
      function encodeBase64(data) {
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
      var image = new Image();
      image.src = 'data:image/' + format + ';base64,' + encodeBase64(pixels);
      assert(image.complete, 'Image could not be decoded'); // page reload might fix it, decoding is async... need .onload handler...
      var canvas = document.createElement('canvas');
      canvas.width = image.width;
      canvas.height = image.height;
      var ctx = canvas.getContext('2d');
      ctx.drawImage(image, 0, 0);
      var imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
      return imageData;
    },*/ 
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

