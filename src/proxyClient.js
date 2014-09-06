
// proxy to/from worker

// utils

function FPSTracker(text) {
  var last = 0;
  var mean = 0;
  var counter = 0;
  this.tick = function() {
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
  this.tick = function(value) {
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

window.requestAnimationFrame = window.requestAnimationFrame || window.mozRequestAnimationFrame ||
                               window.webkitRequestAnimationFrame || window.msRequestAnimationFrame ||
                               renderFrame;

/*
(function() {
  var trueRAF = window.requestAnimationFrame;
  var tracker = new FPSTracker('client');
  window.requestAnimationFrame = function(func) {
    trueRAF(function() {
      tracker.tick();
      func();
    });
  }
})();
*/

// end render

// Frame throttling

var frameId = 0;

// Worker

var worker = new Worker('{{{ filename }}}.js');

WebGLClient.prefetch();

setTimeout(function() {
  worker.postMessage({ target: 'worker-init', width: Module.canvas.width, height: Module.canvas.height, URL: document.URL, preMain: true });
}, 0); // delay til next frame, to make sure html is ready

var workerResponded = false;

worker.onmessage = function worker_onmessage(event) {
  //dump('\nclient got ' + JSON.stringify(event.data).substr(0, 150) + '\n');
  if (!workerResponded) {
    workerResponded = true;
    if (Module.setStatus) Module.setStatus('');
  }

  var data = event.data;
  switch (data.target) {
    case 'stdout': {
      Module.print(data.content);
      break;
    }
    case 'stderr': {
      Module.printErr(data.content);
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
          if (Module.ctx && Module.ctx.getImageData) Module.canvasData = Module.ctx.getImageData(0, 0, data.width, data.height);
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
      img.onload = function() {
        assert(img.complete);
        var canvas = document.createElement('canvas');
        canvas.width = img.width;
        canvas.height = img.height;
        var ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0);
        var imageData = ctx.getImageData(0, 0, img.width, img.height);
        worker.postMessage({ target: 'Image', method: 'onload', id: data.id, width: img.width, height: img.height, data: imageData.data, preMain: true });
      };
      img.onerror = function() {
        worker.postMessage({ target: 'Image', method: 'onerror', id: data.id, preMain: true });
      };
      img.src = data.src;
      break;
    }
    default: throw 'what?';
  }
};

function cloneObject(event) {
  var ret = {};
  for (var x in event) {
    if (x == x.toUpperCase()) continue;
    var prop = event[x];
    if (typeof prop === 'number' || typeof prop === 'string') ret[x] = prop;
  }
  return ret;
};

['keydown', 'keyup', 'keypress', 'blur', 'visibilitychange'].forEach(function(event) {
  document.addEventListener(event, function(event) {
    worker.postMessage({ target: 'document', event: cloneObject(event) });
    event.preventDefault();
  });
});

['unload'].forEach(function(event) {
  window.addEventListener(event, function(event) {
    worker.postMessage({ target: 'window', event: cloneObject(event) });
  });
});

['mousedown', 'mouseup', 'mousemove', 'DOMMouseScroll', 'mousewheel', 'mouseout'].forEach(function(event) {
  Module.canvas.addEventListener(event, function(event) {
    worker.postMessage({ target: 'canvas', event: cloneObject(event) });
    event.preventDefault();
  }, true);
});

