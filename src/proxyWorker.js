
function EventListener() {
  this.listeners = {};

  this.addEventListener = function(event, func) {
    if (!this.listeners[event]) this.listeners[event] = [];
    this.listeners[event].push(func);
  };
};

var window = new EventListener();
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
            buffer: new Uint8Array(canvas.width*canvas.height*4)
          };
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
              data: new Uint8Array(canvas.data.buffer) // TODO: can we avoid this copy?
            };
          },
          putImageData: function(image, x, y) {
            canvas.ensureData();
            assert(x == 0 && y == 0 && image.width == canvas.width && image.height == canvas.height);
            canvas.data.buffer.set(image.data); // TODO: can we avoid this copy?
          }
        };
      };
      return canvas;
    }
    default: throw 'document.createElement ' + what;
  }
};

Module.canvas = document.createElement('canvas');

