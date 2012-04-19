//"use strict";

// Utilities for browser environments

mergeInto(LibraryManager.library, {
  emscripten_set_main_loop: function(func, fps) {
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
        wrapper.objectMap = new WeakMap();
        wrapper.objectCounter = 1;
        for (var prop in tempCtx) {
          (function(prop) {
            switch (typeof tempCtx[prop]) {
              case 'function': {
                wrapper[prop] = function() {
                  var printArgs = Array.prototype.slice.call(arguments).map(function(arg) {
                    if (typeof arg == 'undefined') return '!UNDEFINED!';
                    if (!arg) return arg;
                    if (wrapper.objectMap[arg]) return '<' + arg + '|' + wrapper.objectMap[arg] + '>';
                    if (arg.toString() == '[object HTMLImageElement]') {
                      return arg + '\n\n';
                    }
                    if (arg.byteLength) {
                      return '{' + Array.prototype.slice.call(arg, 0, Math.min(arg.length, 40)) + '}'; // Useful for correct arrays, less so for compiled arrays, see the code below for that
                      var buf = new ArrayBuffer(32);
                      var i8buf = new Int8Array(buf);
                      var i16buf = new Int16Array(buf);
                      var f32buf = new Float32Array(buf);
                      switch(arg.toString()) {
                        case '[object Uint8Array]':
                          i8buf.set(arg.subarray(0, 32));
                          break;
                        case '[object Float32Array]':
                          f32buf.set(arg.subarray(0, 5));
                          break;
                        case '[object Uint16Array]':
                          i16buf.set(arg.subarray(0, 16));
                          break;
                        default:
                          alert('unknown array for debugging: ' + arg);
                          throw 'see alert';
                      }
                      var ret = '{' + arg.byteLength + ':\n';
                      var arr = Array.prototype.slice.call(i8buf);
                      ret += 'i8:' + arr.toString().replace(/,/g, ',') + '\n';
                      arr = Array.prototype.slice.call(f32buf, 0, 8);
                      ret += 'f32:' + arr.toString().replace(/,/g, ',') + '}';
                      return ret;
                    }
                    return arg;
                  });
                  console.log('[gl_f:' + prop + ':' + printArgs + ']');
                  var ret = tempCtx[prop].apply(tempCtx, arguments);
                  var printRet = ret;
                  if (typeof ret == 'object') {
                    wrapper.objectMap[ret] = wrapper.objectCounter++;
                    printRet = '<' + ret + '|' + wrapper.objectMap[ret] + '>';
                  }
                  if (typeof printRet != 'undefined') console.log('[     gl:' + prop + ':return:' + printRet + ']');
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
                  console.log('[gl_s:' + prop + ':' + value + ']');
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

