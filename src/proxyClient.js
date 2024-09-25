/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
 * Proxy events/work to/from an emscripen worker built
 * with PROXY_TO_WORKER.  This code runs on the main
 * thread and is not part of the main emscripten output
 * file.
 */

#if !PROXY_TO_WORKER
#error "proxyClient.js should only be included in PROXY_TO_WORKER mode"
#endif

#if ENVIRONMENT_MAY_BE_NODE
var ENVIRONMENT_IS_NODE = typeof process == 'object' && typeof process.versions == 'object' && typeof process.versions.node == 'string' && process.type != 'renderer';
if (ENVIRONMENT_IS_NODE) {
  var NodeWorker = require('worker_threads').Worker;
  global.Worker = function(url, options) {
    // Special handling for `data:` URL argument, to match the behaviour
    // of the Web API.
    if (typeof url == 'string' && url.startsWith('data:')) {
#if EXPORT_ES6
      // worker_threads always assume data URLs are ES6 modules
      url = new URL(url);
#else
      // For class modules we decode the data URL and use `eval: true`.
      url = Buffer.from(url.split(",")[1], 'base64').toString();
      options ||= {}
      options.eval = true;
#endif
    }
    return new NodeWorker(url, options);
  }
  var Module = Module || {}
} else
#endif
if (typeof Module == 'undefined') {
  console.warn('no Module object defined - cannot proxy canvas rendering and input events, etc.');
  Module = {
    canvas: {
      addEventListener: () => {},
      getBoundingClientRect: () => ({ bottom: 0, height: 0, left: 0, right: 0, top: 0, width: 0 }),
    },
  };
}

if (!Module.hasOwnProperty('print')) {
  Module['print'] = (x) => console.log(x);
}

if (!Module.hasOwnProperty('printErr')) {
  Module['printErr'] = (x) => console.error(x);
}

// utils

function FPSTracker(text) {
  var last = 0;
  var mean = 0;
  var counter = 0;
  this.tick = () => {
    var now = Date.now();
    if (last > 0) {
      var diff = now - last;
      mean = 0.99*mean + 0.01*diff;
      if (counter++ === 60) {
        counter = 0;
        dump(text + ' fps: ' + (1000/mean).toFixed(2) + '\n');
      }
    }
    last = now;
  };
}

/*
function GenericTracker(text) {
  var mean = 0;
  var counter = 0;
  this.tick = (value) => {
    mean = 0.99*mean + 0.01*value;
    if (counter++ === 60) {
      counter = 0;
      dump(text + ': ' + (mean).toFixed(2) + '\n');
    }
  };
}
*/

// render

var renderFrameData = null;

function renderFrame() {
  var dst = Module.canvasData.data;
  if (dst.set) {
    dst.set(renderFrameData);
  } else {
    for (var i = 0; i < renderFrameData.length; i++) {
      dst[i] = renderFrameData[i];
    }
  }
  Module.ctx.putImageData(Module.canvasData, 0, 0);
  renderFrameData = null;
}

if (typeof window != 'undefined') {
  window.requestAnimationFrame = window.requestAnimationFrame || window.mozRequestAnimationFrame ||
                                 window.webkitRequestAnimationFrame || window.msRequestAnimationFrame ||
                                 renderFrame;
}

/*
(() => {
  var trueRAF = window.requestAnimationFrame;
  var tracker = new FPSTracker('client');
  window.requestAnimationFrame = (func) => {
    trueRAF(() => {
      tracker.tick();
      func();
    });
  }
})();
*/

// end render

// IDBStore

#include "IDBStore.js"

// Frame throttling

var frameId = 0;

// Worker

var filename;
filename ||= '<<< filename >>>';

var worker = new Worker(filename);

#if ENVIRONMENT_MAY_BE_NODE
if (ENVIRONMENT_IS_NODE) {
  worker.postMessage({target: 'worker-init'});
} else {
#endif
WebGLClient.prefetch();

setTimeout(() => {
  worker.postMessage({
    target: 'worker-init',
    width: Module.canvas.width,
    height: Module.canvas.height,
    boundingClientRect: cloneObject(Module.canvas.getBoundingClientRect()),
    URL: document.URL,
    currentScriptUrl: filename,
    preMain: true });
}, 0); // delay til next frame, to make sure html is ready
#if ENVIRONMENT_MAY_BE_NODE
}
#endif

var workerResponded = false;

worker.onmessage = (event) => {
  //dump('\nclient got ' + JSON.stringify(event.data).substr(0, 150) + '\n');
  if (!workerResponded) {
    workerResponded = true;
    Module.setStatus?.('');
  }

  var data = event.data;
  switch (data.target) {
    case 'stdout': {
      Module['print'](data.content);
      break;
    }
    case 'stderr': {
      Module['printErr'](data.content);
      break;
    }
    case 'window': {
      window[data.method]();
      break;
    }
    case 'canvas': {
      switch (data.op) {
        case 'getContext': {
          Module.ctx = Module.canvas.getContext(data.type, data.attributes);
          if (data.type !== '2d') {
            // possible GL_DEBUG entry point: Module.ctx = wrapDebugGL(Module.ctx);
            Module.glClient = new WebGLClient();
          }
          break;
        }
        case 'resize': {
          Module.canvas.width = data.width;
          Module.canvas.height = data.height;
          if (Module.ctx?.getImageData) Module.canvasData = Module.ctx.getImageData(0, 0, data.width, data.height);
          worker.postMessage({ target: 'canvas', boundingClientRect: cloneObject(Module.canvas.getBoundingClientRect()) });
          break;
        }
        case 'render': {
          if (renderFrameData) {
            // previous image was not rendered yet, just update image
            renderFrameData = data.image.data;
          } else {
            // previous image was rendered so update image and request another frame
            renderFrameData = data.image.data;
            window.requestAnimationFrame(renderFrame);
          }
          break;
        }
        case 'setObjectProperty': {
          Module.canvas[data.object][data.property] = data.value;
          break;
        }
        default: throw 'eh?';
      }
      break;
    }
    case 'gl': {
      Module.glClient.onmessage(data);
      break;
    }
    case 'tick': {
      frameId = data.id;
      worker.postMessage({ target: 'tock', id: frameId });
      break;
    }
    case 'Image': {
      assert(data.method === 'src');
      var img = new Image();
      img.onload = () => {
        assert(img.complete);
        var canvas = document.createElement('canvas');
        canvas.width = img.width;
        canvas.height = img.height;
        var ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0);
        var imageData = ctx.getImageData(0, 0, img.width, img.height);
        worker.postMessage({ target: 'Image', method: 'onload', id: data.id, width: img.width, height: img.height, data: imageData.data, preMain: true });
      };
      img.onerror = () => {
        worker.postMessage({ target: 'Image', method: 'onerror', id: data.id, preMain: true });
      };
      img.src = data.src;
      break;
    }
    case 'IDBStore': {
      switch (data.method) {
        case 'loadBlob': {
          IDBStore.getFile(data.db, data.id, (error, blob) => {
            worker.postMessage({
              target: 'IDBStore',
              method: 'response',
              blob: error ? null : blob
            });
          });
          break;
        }
        case 'storeBlob': {
          IDBStore.setFile(data.db, data.id, data.blob, (error) => {
            worker.postMessage({
              target: 'IDBStore',
              method: 'response',
              error: !!error
            });
          });
          break;
        }
      }
      break;
    }
    case 'custom': {
      if (Module['onCustomMessage']) {
        Module['onCustomMessage'](event);
      } else {
        throw 'Custom message received but client Module.onCustomMessage not implemented.';
      }
      break;
    }
    case 'setimmediate': {
      worker.postMessage({target: 'setimmediate'});
      break;
    }
    default: throw 'what? ' + data.target;
  }
};

function postCustomMessage(data, options = {}) {
  worker.postMessage({ target: 'custom', userData: data, preMain: options.preMain });
}

function cloneObject(event) {
  var ret = {};
  for (var x in event) {
    if (x == x.toUpperCase()) continue;
    var prop = event[x];
    if (typeof prop == 'number' || typeof prop == 'string') ret[x] = prop;
  }
  return ret;
};

#if ENVIRONMENT_MAY_BE_NODE
if (!ENVIRONMENT_IS_NODE) {
#endif

// Only prevent default on backspace/tab because we don't want unexpected navigation.
// Do not prevent default on the rest as we need the keypress event.
function shouldPreventDefault(event) {
  if (event.type === 'keydown' && event.keyCode !== 8 /* backspace */ && event.keyCode !== 9 /* tab */) {
    return false; // keypress, back navigation
  } else {
    return true; // NO keypress, NO back navigation
  }
};


['keydown', 'keyup', 'keypress', 'blur', 'visibilitychange'].forEach((event) => {
  document.addEventListener(event, (event) => {
    worker.postMessage({ target: 'document', event: cloneObject(event) });

    if (shouldPreventDefault(event)) {
      event.preventDefault();
    }
  });
});

['unload'].forEach((event) => {
  window.addEventListener(event, (event) => {
    worker.postMessage({ target: 'window', event: cloneObject(event) });
  });
});

['mousedown', 'mouseup', 'mousemove', 'DOMMouseScroll', 'mousewheel', 'mouseout'].forEach((event) => {
  Module.canvas.addEventListener(event, (event) => {
    worker.postMessage({ target: 'canvas', event: cloneObject(event) });
    event.preventDefault();
  }, true);
});

#if ENVIRONMENT_MAY_BE_NODE
}
#endif
