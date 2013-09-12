
// proxy to/from worker

Module.ctx = Module.canvas.getContext('2d');

var worker = new Worker('{{{ filename }}}.js');

worker.onmessage = function(event) {
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
          postMessage({ target: 'canvas', boundingClientRect: Module.canvas.getBoundingClientRect() });
          break;
        }
        case 'render': {
          Module.canvasData.data.set(data.image.data);
          Module.ctx.putImageData(Module.canvasData, 0, 0);
          break;
        }
        default: throw 'eh?';
      }
      break;
    }
    default: throw 'what?';
  }
};

function cloneEvent(event) {
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
    worker.postMessage({ target: 'document', event: cloneEvent(event) });
    event.preventDefault();
  });
});

['unload'].forEach(function(event) {
  window.addEventListener(event, function(event) {
    worker.postMessage({ target: 'window', event: cloneEvent(event) });
  });
});

['mousedown', 'mouseup', 'mousemove', 'DOMMouseScroll', 'mousewheel', 'mouseout'].forEach(function(event) {
  Module.canvas.addEventListener(event, function(event) {
    worker.postMessage({ target: 'canvas', event: cloneEvent(event) });
    event.preventDefault();
  }, true);
});

