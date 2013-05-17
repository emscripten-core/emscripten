//"use strict";

// Utilities for browser environments

mergeInto(LibraryManager.library, {
  $Browser__postset: 'Module["requestFullScreen"] = function(lockPointer, resizeCanvas) { Browser.requestFullScreen(lockPointer, resizeCanvas) };\n' + // exports
                     'Module["requestAnimationFrame"] = function(func) { Browser.requestAnimationFrame(func) };\n' +
                     'Module["pauseMainLoop"] = function() { Browser.mainLoop.pause() };\n' +
                     'Module["resumeMainLoop"] = function() { Browser.mainLoop.resume() };\n',
  $Browser: {
    mainLoop: {
      scheduler: null,
      shouldPause: false,
      paused: false,
      queue: [],
      pause: function() {
        Browser.mainLoop.shouldPause = true;
      },
      resume: function() {
        if (Browser.mainLoop.paused) {
          Browser.mainLoop.paused = false;
          Browser.mainLoop.scheduler();
        }
        Browser.mainLoop.shouldPause = false;
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
      }
    },
    isFullScreen: false,
    pointerLock: false,
    moduleContextCreatedCallbacks: [],
    workers: [],

    init: function() {
      if (Browser.initted) return;
      Browser.initted = true;
      try {
        new Blob();
        Browser.hasBlobConstructor = true;
      } catch(e) {
        Browser.hasBlobConstructor = false;
        console.log("warning: no blob constructor, cannot create blobs with mimetypes");
      }
      Browser.BlobBuilder = typeof MozBlobBuilder != "undefined" ? MozBlobBuilder : (typeof WebKitBlobBuilder != "undefined" ? WebKitBlobBuilder : (!Browser.hasBlobConstructor ? console.log("warning: no BlobBuilder") : null));
      Browser.URLObject = typeof window != "undefined" ? (window.URL ? window.URL : window.webkitURL) : console.log("warning: cannot create object URLs");

      // Support for plugins that can process preloaded files. You can add more of these to
      // your app by creating and appending to Module.preloadPlugins.
      //
      // Each plugin is asked if it can handle a file based on the file's name. If it can,
      // it is given the file's raw data. When it is done, it calls a callback with the file's
      // (possibly modified) data. For example, a plugin might decompress a file, or it
      // might create some side data structure for use later (like an Image element, etc.).

      function getMimetype(name) {
        return {
          'jpg': 'image/jpeg',
          'jpeg': 'image/jpeg',
          'png': 'image/png',
          'bmp': 'image/bmp',
          'ogg': 'audio/ogg',
          'wav': 'audio/wav',
          'mp3': 'audio/mpeg'
        }[name.substr(name.lastIndexOf('.')+1)];
      }

      if (!Module["preloadPlugins"]) Module["preloadPlugins"] = [];

      var imagePlugin = {};
      imagePlugin['canHandle'] = function(name) {
        return !Module.noImageDecoding && /\.(jpg|jpeg|png|bmp)$/.exec(name);
      };
      imagePlugin['handle'] = function(byteArray, name, onload, onerror) {
        var b = null;
        if (Browser.hasBlobConstructor) {
          try {
            b = new Blob([byteArray], { type: getMimetype(name) });
          } catch(e) {
            Runtime.warnOnce('Blob constructor present but fails: ' + e + '; falling back to blob builder');
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
        img.onload = function() {
          assert(img.complete, 'Image ' + name + ' could not be decoded');
          var canvas = document.createElement('canvas');
          canvas.width = img.width;
          canvas.height = img.height;
          var ctx = canvas.getContext('2d');
          ctx.drawImage(img, 0, 0);
          Module["preloadedImages"][name] = canvas;
          Browser.URLObject.revokeObjectURL(url);
          if (onload) onload(byteArray);
        };
        img.onerror = function(event) {
          console.log('Image ' + url + ' could not be decoded');
          if (onerror) onerror();
        };
        img.src = url;
      };
      Module['preloadPlugins'].push(imagePlugin);

      var audioPlugin = {};
      audioPlugin['canHandle'] = function(name) {
        return !Module.noAudioDecoding && name.substr(-4) in { '.ogg': 1, '.wav': 1, '.mp3': 1 };
      };
      audioPlugin['handle'] = function(byteArray, name, onload, onerror) {
        var done = false;
        function finish(audio) {
          if (done) return;
          done = true;
          Module["preloadedAudios"][name] = audio;
          if (onload) onload(byteArray);
        }
        function fail() {
          if (done) return;
          done = true;
          Module["preloadedAudios"][name] = new Audio(); // empty shim
          if (onerror) onerror();
        }
        if (Browser.hasBlobConstructor) {
          try {
            var b = new Blob([byteArray], { type: getMimetype(name) });
          } catch(e) {
            return fail();
          }
          var url = Browser.URLObject.createObjectURL(b); // XXX we never revoke this!
#if ASSERTIONS
          assert(typeof url == 'string', 'createObjectURL must return a url as a string');
#endif
          var audio = new Audio();
          audio.addEventListener('canplaythrough', function() { finish(audio) }, false); // use addEventListener due to chromium bug 124926
          audio.onerror = function(event) {
            if (done) return;
            console.log('warning: browser could not fully decode audio ' + name + ', trying slower base64 approach');
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
          setTimeout(function() {
            finish(audio); // try to use it even though it is not necessarily ready to play
          }, 10000);
        } else {
          return fail();
        }
      };
      Module['preloadPlugins'].push(audioPlugin);

      // Canvas event setup

      var canvas = Module['canvas'];
      canvas.requestPointerLock = canvas['requestPointerLock'] ||
                                  canvas['mozRequestPointerLock'] ||
                                  canvas['webkitRequestPointerLock'];
      canvas.exitPointerLock = document['exitPointerLock'] ||
                               document['mozExitPointerLock'] ||
                               document['webkitExitPointerLock'];
      canvas.exitPointerLock = canvas.exitPointerLock.bind(document);

      function pointerLockChange() {
        Browser.pointerLock = document['pointerLockElement'] === canvas ||
                              document['mozPointerLockElement'] === canvas ||
                              document['webkitPointerLockElement'] === canvas;
      }

      document.addEventListener('pointerlockchange', pointerLockChange, false);
      document.addEventListener('mozpointerlockchange', pointerLockChange, false);
      document.addEventListener('webkitpointerlockchange', pointerLockChange, false);

      if (Module['elementPointerLock']) {
        canvas.addEventListener("click", function(ev) {
          if (!Browser.pointerLock && canvas.requestPointerLock) {
            canvas.requestPointerLock();
            ev.preventDefault();
          }
        }, false);
      }
    },

    createContext: function(canvas, useWebGL, setInModule) {
#if !USE_TYPED_ARRAYS
      if (useWebGL) {
        Module.print('(USE_TYPED_ARRAYS needs to be enabled for WebGL)');
        return null;
      }
#endif
      var ctx;
      try {
        if (useWebGL) {
          ctx = canvas.getContext('experimental-webgl', {
#if GL_TESTING
            preserveDrawingBuffer: true,
#endif
            alpha: false
          });
        } else {
          ctx = canvas.getContext('2d');
        }
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
        Browser.init();
      }
      return ctx;
    },

    destroyContext: function(canvas, useWebGL, setInModule) {},

    fullScreenHandlersInstalled: false,
    lockPointer: undefined,
    resizeCanvas: undefined,
    requestFullScreen: function(lockPointer, resizeCanvas) {
      Browser.lockPointer = lockPointer;
      Browser.resizeCanvas = resizeCanvas;
      if (typeof Browser.lockPointer === 'undefined') Browser.lockPointer = true;
      if (typeof Browser.resizeCanvas === 'undefined') Browser.resizeCanvas = false;

      var canvas = Module['canvas'];
      function fullScreenChange() {
        Browser.isFullScreen = false;
        if ((document['webkitFullScreenElement'] || document['webkitFullscreenElement'] ||
             document['mozFullScreenElement'] || document['mozFullscreenElement'] ||
             document['fullScreenElement'] || document['fullscreenElement']) === canvas) {
          canvas.cancelFullScreen = document['cancelFullScreen'] ||
                                    document['mozCancelFullScreen'] ||
                                    document['webkitCancelFullScreen'];
          canvas.cancelFullScreen = canvas.cancelFullScreen.bind(document);
          if (Browser.lockPointer) canvas.requestPointerLock();
          Browser.isFullScreen = true;
          if (Browser.resizeCanvas) Browser.setFullScreenCanvasSize();
        } else if (Browser.resizeCanvas){
          Browser.setWindowedCanvasSize();
        }
        if (Module['onFullScreen']) Module['onFullScreen'](Browser.isFullScreen);
      }

      if (!Browser.fullScreenHandlersInstalled) {
        Browser.fullScreenHandlersInstalled = true;
        document.addEventListener('fullscreenchange', fullScreenChange, false);
        document.addEventListener('mozfullscreenchange', fullScreenChange, false);
        document.addEventListener('webkitfullscreenchange', fullScreenChange, false);
      }

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

    mouseX: 0,
    mouseY: 0,
    mouseMovementX: 0,
    mouseMovementY: 0,

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
        Browser.mouseX = SDL.mouseX + Browser.mouseMovementX;
        Browser.mouseY = SDL.mouseY + Browser.mouseMovementY;
      } else {
        // Otherwise, calculate the movement based on the changes
        // in the coordinates.
        var rect = Module["canvas"].getBoundingClientRect();
        var x = event.pageX - (window.scrollX + rect.left);
        var y = event.pageY - (window.scrollY + rect.top);

        // the canvas might be CSS-scaled compared to its backbuffer;
        // SDL-using content will want mouse coordinates in terms
        // of backbuffer units.
        var cw = Module["canvas"].width;
        var ch = Module["canvas"].height;
        x = x * (cw / rect.width);
        y = y * (ch / rect.height);

        Browser.mouseMovementX = x - Browser.mouseX;
        Browser.mouseMovementY = y - Browser.mouseY;
        Browser.mouseX = x;
        Browser.mouseY = y;
      }
    },

    xhrLoad: function(url, onload, onerror) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, true);
      xhr.responseType = 'arraybuffer';
      xhr.onload = function() {
        if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
          onload(xhr.response);
        } else {
          onerror();
        }
      };
      xhr.onerror = onerror;
      xhr.send(null);
    },

    asyncLoad: function(url, onload, onerror, noRunDep) {
      Browser.xhrLoad(url, function(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file "' + url + '" failed (no arrayBuffer).');
        onload(new Uint8Array(arrayBuffer));
        if (!noRunDep) removeRunDependency('al ' + url);
      }, function(event) {
        if (onerror) {
          onerror();
        } else {
          throw 'Loading data file "' + url + '" failed.';
        }
      });
      if (!noRunDep) addRunDependency('al ' + url);
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
      canvas.width = width;
      canvas.height = height;
      if (!noUpdates) Browser.updateResizeListeners();
    },

    windowedWidth: 0,
    windowedHeight: 0,
    setFullScreenCanvasSize: function() {
      var canvas = Module['canvas'];
      this.windowedWidth = canvas.width;
      this.windowedHeight = canvas.height;
      canvas.width = screen.width;
      canvas.height = screen.height;
      var flags = {{{ makeGetValue('SDL.screen+Runtime.QUANTUM_SIZE*0', '0', 'i32', 0, 1) }}};
      flags = flags | 0x00800000; // set SDL_FULLSCREEN flag
      {{{ makeSetValue('SDL.screen+Runtime.QUANTUM_SIZE*0', '0', 'flags', 'i32') }}}
      Browser.updateResizeListeners();
    },
    
    setWindowedCanvasSize: function() {
      var canvas = Module['canvas'];
      canvas.width = this.windowedWidth;
      canvas.height = this.windowedHeight;
      var flags = {{{ makeGetValue('SDL.screen+Runtime.QUANTUM_SIZE*0', '0', 'i32', 0, 1) }}};
      flags = flags & ~0x00800000; // clear SDL_FULLSCREEN flag
      {{{ makeSetValue('SDL.screen+Runtime.QUANTUM_SIZE*0', '0', 'flags', 'i32') }}}
      Browser.updateResizeListeners();
    }
    
  },

  emscripten_async_wget: function(url, file, onload, onerror) {
    var _url = Pointer_stringify(url);
    var _file = Pointer_stringify(file);
    var index = _file.lastIndexOf('/');
    FS.createPreloadedFile(
      _file.substr(0, index),
      _file.substr(index +1),
      _url, true, true,
      function() {
        if (onload) Runtime.dynCall('vi', onload, [file]);
      },
      function() {
        if (onerror) Runtime.dynCall('vi', onerror, [file]);
      }
    );
  },

  emscripten_async_wget_data: function(url, arg, onload, onerror) {
    Browser.asyncLoad(Pointer_stringify(url), function(byteArray) {
      var buffer = _malloc(byteArray.length);
      HEAPU8.set(byteArray, buffer);
      Runtime.dynCall('viii', onload, [arg, buffer, byteArray.length]);
      _free(buffer);
    }, function() {
      if (onerror) Runtime.dynCall('vi', onerror, [arg]);
    }, true /* no need for run dependency, this is async but will not do any prepare etc. step */ );
  },

  emscripten_async_wget2: function(url, file, request, param, arg, onload, onerror, onprogress) {
    var _url = Pointer_stringify(url);
    var _file = Pointer_stringify(file);
    var _request = Pointer_stringify(request);
    var _param = Pointer_stringify(param);
    var index = _file.lastIndexOf('/');
     
    var http = new XMLHttpRequest();
    http.open(_request, _url, true);
    http.responseType = 'arraybuffer';
    
    // LOAD
    http.onload = function(e) {
      if (http.status == 200) {
        FS.createDataFile( _file.substr(0, index), _file.substr(index + 1), new Uint8Array(http.response), true, true);
        if (onload) Runtime.dynCall('vii', onload, [arg, file]);
      } else {
        if (onerror) Runtime.dynCall('vii', onerror, [arg, http.status]);
      }
    };
      
    // ERROR
    http.onerror = function(e) {
      if (onerror) Runtime.dynCall('vii', onerror, [arg, http.status]);
    };
	
    // PROGRESS
    http.onprogress = function(e) {
      var percentComplete = (e.position / e.totalSize)*100;
      if (onprogress) Runtime.dynCall('vii', onprogress, [arg, percentComplete]);
    };
	  
    // Useful because the browser can limit the number of redirection
    try {  
      if (http.channel instanceof Ci.nsIHttpChannel)
      http.channel.redirectionLimit = 0;
    } catch (ex) { /* whatever */ }

    if (_request == "POST") {
      //Send the proper header information along with the request
      http.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
      http.setRequestHeader("Content-length", _param.length);
      http.setRequestHeader("Connection", "close");
      http.send(_param);
    } else {
      http.send(null);
    }
  },
  
  emscripten_async_prepare: function(file, onload, onerror) {
    var _file = Pointer_stringify(file);
    var data = FS.analyzePath(_file);
    if (!data.exists) return -1;
    var index = _file.lastIndexOf('/');
    FS.createPreloadedFile(
      _file.substr(0, index),
      _file.substr(index +1),
      new Uint8Array(data.object.contents), true, true,
      function() {
        if (onload) Runtime.dynCall('vi', onload, [file]);
      },
      function() {
        if (onerror) Runtime.dynCall('vi', onerror, [file]);
      },
      true // don'tCreateFile - it's already there
    );
    return 0;
  },

  emscripten_async_prepare_data: function(data, size, suffix, arg, onload, onerror) {
    var _suffix = Pointer_stringify(suffix);
    if (!Browser.asyncPrepareDataCounter) Browser.asyncPrepareDataCounter = 0;
    var name = 'prepare_data_' + (Browser.asyncPrepareDataCounter++) + '.' + _suffix;
    var cname = _malloc(name.length+1);
    writeStringToMemory(name, cname);
    FS.createPreloadedFile(
      '',
      name,
      {{{ makeHEAPView('U8', 'data', 'data + size') }}},
      true, true,
      function() {
        if (onload) Runtime.dynCall('vii', onload, [arg, cname]);
      },
      function() {
        if (onerror) Runtime.dynCall('vi', onerror, [arg]);
      },
      true // don'tCreateFile - it's already there
    );
    return 0;
  },

  emscripten_async_run_script__deps: ['emscripten_run_script'],
  emscripten_async_run_script: function(script, millis) {
    Module['noExitRuntime'] = true;

    // TODO: cache these to avoid generating garbage
    setTimeout(function() {
      _emscripten_run_script(script);
    }, millis);
  },

  emscripten_set_main_loop: function(func, fps, simulateInfiniteLoop) {
    Module['noExitRuntime'] = true;

    Browser.mainLoop.runner = function() {
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
        console.log('main loop blocker "' + blocker.name + '" took ' + (Date.now() - start) + ' ms'); //, left: ' + Browser.mainLoop.remainingBlockers);
        Browser.mainLoop.updateStatus();
        setTimeout(Browser.mainLoop.runner, 0);
        return;
      }
      if (Browser.mainLoop.shouldPause) {
        // catch pauses from non-main loop sources
        Browser.mainLoop.paused = true;
        Browser.mainLoop.shouldPause = false;
        return;
      }

      if (Module['preMainLoop']) {
        Module['preMainLoop']();
      }

      Runtime.dynCall('v', func);

      if (Module['postMainLoop']) {
        Module['postMainLoop']();
      }

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
        setTimeout(Browser.mainLoop.runner, 1000/fps); // doing this each time means that on exception, we stop
      }
    } else {
      Browser.mainLoop.scheduler = function() {
        Browser.requestAnimationFrame(Browser.mainLoop.runner);
      }
    }
    Browser.mainLoop.scheduler();

    if (simulateInfiniteLoop) {
      throw 'SimulateInfiniteLoop';
    }
  },

  emscripten_cancel_main_loop: function() {
    Browser.mainLoop.scheduler = null;
    Browser.mainLoop.shouldPause = true;
  },

  emscripten_pause_main_loop: function() {
    Browser.mainLoop.pause();
  },

  emscripten_resume_main_loop: function() {
    Browser.mainLoop.resume();
  },

  _emscripten_push_main_loop_blocker: function(func, arg, name) {
    Browser.mainLoop.queue.push({ func: function() {
      Runtime.dynCall('vi', func, [arg]);
    }, name: Pointer_stringify(name), counted: true });
    Browser.mainLoop.updateStatus();
  },

  _emscripten_push_uncounted_main_loop_blocker: function(func, arg, name) {
    Browser.mainLoop.queue.push({ func: function() {
      Runtime.dynCall('vi', func, [arg]);
    }, name: Pointer_stringify(name), counted: false });
    Browser.mainLoop.updateStatus();
  },

  emscripten_set_main_loop_expected_blockers: function(num) {
    Browser.mainLoop.expectedBlockers = num;
    Browser.mainLoop.remainingBlockers = num;
    Browser.mainLoop.updateStatus();
  },

  emscripten_async_call: function(func, arg, millis) {
    Module['noExitRuntime'] = true;

    function wrapper() {
      Runtime.getFuncWrapper(func, 'vi')(arg);
    }

    if (millis >= 0) {
      setTimeout(wrapper, millis);
    } else {
      Browser.requestAnimationFrame(wrapper);
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
    Browser.setCanvasSize(width, height);
  },

  emscripten_get_now: function() {
    if (ENVIRONMENT_IS_NODE) {
        var t = process['hrtime']();
        return t[0] * 1e3 + t[1] / 1e6;
    }
    else if (window['performance'] && window['performance']['now']) {
      return window['performance']['now']();
    } else {
      return Date.now();
    }
  },

  emscripten_create_worker: function(url) {
    url = Pointer_stringify(url);
    var id = Browser.workers.length;
    var info = {
      worker: new Worker(url),
      callbacks: [],
      awaited: 0,
      buffer: 0,
      bufferSize: 0
    };
    info.worker.onmessage = function(msg) {
      var info = Browser.workers[id];
      if (!info) return; // worker was destroyed meanwhile
      var callbackId = msg.data['callbackId'];
      var callbackInfo = info.callbacks[callbackId];
      if (!callbackInfo) return; // no callback or callback removed meanwhile
      info.awaited--;
      info.callbacks[callbackId] = null; // TODO: reuse callbackIds, compress this
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

  emscripten_destroy_worker: function(id) {
    var info = Browser.workers[id];
    info.worker.terminate();
    if (info.buffer) _free(info.buffer);
    Browser.workers[id] = null;
  },

  emscripten_call_worker: function(id, funcName, data, size, callback, arg) {
    funcName = Pointer_stringify(funcName);
    var info = Browser.workers[id];
    var callbackId = -1;
    if (callback) {
      callbackId = info.callbacks.length;
      info.callbacks.push({
        func: Runtime.getFuncWrapper(callback, 'viii'),
        arg: arg
      });
      info.awaited++;
    }
    info.worker.postMessage({
      'funcName': funcName,
      'callbackId': callbackId,
      'data': data ? new Uint8Array({{{ makeHEAPView('U8', 'data', 'data + size') }}}) : 0 // XXX copy to a new typed array as a workaround for chrome bug 169705
    });
  },

  emscripten_worker_respond: function(data, size) {
    if (!inWorkerCall) throw 'not in worker call!';
    if (workerResponded) throw 'already responded!';
    workerResponded = true;
    postMessage({
      'callbackId': workerCallbackId,
      'data': data ? new Uint8Array({{{ makeHEAPView('U8', 'data', 'data + size') }}}) : 0 // XXX copy to a new typed array as a workaround for chrome bug 169705
    });
  },

  emscripten_get_worker_queue_size: function(id) {
    var info = Browser.workers[id];
    if (!info) return -1;
    return info.awaited;
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

