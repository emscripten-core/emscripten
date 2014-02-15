
// proxy to/from worker

Module.ctx = Module.canvas.getContext('2d');

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

// end render

var worker = new Worker('{{{ filename }}}.js');

var workerResponded = false;

worker.onmessage = function worker_onmessage(event) {
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
        case 'resize': {
          Module.canvas.width = data.width;
          Module.canvas.height = data.height;
          Module.canvasData = Module.ctx.getImageData(0, 0, data.width, data.height);
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

