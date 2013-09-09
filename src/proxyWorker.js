
function EventListener() {
  this.listeners = {};

  this.addEventListener = function(event, func) {
    if (!this.listeners[event]) this.listeners[event] = [];
    this.listeners[event].push(func);
  };
};

var window = this;
var windowExtra = new EventListener();
for (var x in windowExtra) window[x] = windowExtra[x];

window.close = function() {
  postMessage({ target: 'window', method: 'close' });
};

var document = new EventListener();

document.createElement = function(what) {
  switch(what) {
    case 'canvas': {
      var canvas = new EventListener();
      canvas.ensureData = function() {
        if (!canvas.data || canvas.data.width !== canvas.width || canvas.data.height !== canvas.height) {
          canvas.data = {
            width: canvas.width,
            height: canvas.height,
            data: new Uint8Array(canvas.width*canvas.height*4)
          };
          postMessage({ target: 'canvas', op: 'resize', width: canvas.width, height: canvas.height });
        }
      };
      canvas.getContext = function(type) {
        assert(type == '2d');
        return {
          getImageData: function(x, y, w, h) {
            assert(x == 0 && y == 0 && w == canvas.width && h == canvas.height);
            canvas.ensureData();
            return {
              width: canvas.data.width,
              height: canvas.data.height,
              data: new Uint8Array(canvas.data.data) // TODO: can we avoid this copy?
            };
          },
          putImageData: function(image, x, y) {
            canvas.ensureData();
            assert(x == 0 && y == 0 && image.width == canvas.width && image.height == canvas.height);
            canvas.data.data.set(image.data); // TODO: can we avoid this copy?
            postMessage({ target: 'canvas', op: 'render', image: canvas.data });
          }
        };
      };
      return canvas;
    }
    default: throw 'document.createElement ' + what;
  }
};

Module.canvas = document.createElement('canvas');

Module.setStatus = function(){};

Module.print = function(x) {
  postMessage({ target: 'stdout', content: x });
};
Module.printErr = function(x) {
  postMessage({ target: 'stderr', content: x });
};

