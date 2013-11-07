
function EventListener() {
  this.listeners = {};

  this.addEventListener = function addEventListener(event, func) {
    if (!this.listeners[event]) this.listeners[event] = [];
    this.listeners[event].push(func);
  };

  this.fireEvent = function fireEvent(event) {
    event.preventDefault = function(){};

    if (event.type in this.listeners) {
      this.listeners[event.type].forEach(function(listener) {
        listener(event);
      });
    }
  };
};

var window = this;
var windowExtra = new EventListener();
for (var x in windowExtra) window[x] = windowExtra[x];

window.close = function window_close() {
  postMessage({ target: 'window', method: 'close' });
};

var document = new EventListener();

document.createElement = function document_createElement(what) {
  switch(what) {
    case 'canvas': {
      var canvas = new EventListener();
      canvas.ensureData = function canvas_ensureData() {
        if (!canvas.data || canvas.data.width !== canvas.width || canvas.data.height !== canvas.height) {
          canvas.data = {
            width: canvas.width,
            height: canvas.height,
            data: new Uint8Array(canvas.width*canvas.height*4)
          };
          postMessage({ target: 'canvas', op: 'resize', width: canvas.width, height: canvas.height });
        }
      };
      canvas.getContext = function canvas_getContext(type) {
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
      canvas.boundingClientRect = {};
      canvas.getBoundingClientRect = function canvas_getBoundingClientRect() {
        return {
          width: canvas.boundingClientRect.width,
          height: canvas.boundingClientRect.height,
          top: canvas.boundingClientRect.top,
          left: canvas.boundingClientRect.left,
          bottom: canvas.boundingClientRect.bottom,
          right: canvas.boundingClientRect.right
        };
      };
      return canvas;
    }
    default: throw 'document.createElement ' + what;
  }
};

var console = {
  log: function(x) {
    Module.printErr(x);
  }
};

Module.canvas = document.createElement('canvas');

Module.setStatus = function(){};

Module.print = function Module_print(x) {
  postMessage({ target: 'stdout', content: x });
};
Module.printErr = function Module_printErr(x) {
  postMessage({ target: 'stderr', content: x });
};

// buffer messages until the program starts to run

var messageBuffer = null;

function messageResender() {
  if (calledMain) {
    assert(messageBuffer && messageBuffer.length > 0);
    messageBuffer.forEach(function(message) {
      onmessage(message);
    });
    messageBuffer = null;
  } else {
    setTimeout(messageResender, 100);
  }
}

onmessage = function onmessage(message) {
  if (!calledMain) {
    if (!messageBuffer) {
      messageBuffer = [];
      setTimeout(messageResender, 100);
    }
    messageBuffer.push(message);
  }
  switch (message.data.target) {
    case 'document': {
      document.fireEvent(message.data.event);
      break;
    }
    case 'window': {
      window.fireEvent(message.data.event);
      break;
    }
    case 'canvas': {
      if (message.data.event) {
        Module.canvas.fireEvent(message.data.event);
      } else if (message.data.boundingClientRect) {
        Module.canvas.boundingClientRect = message.data.boundingClientRect;
      } else throw 'ey?';
      break;
    }
    default: throw 'wha? ' + message.data.target;
  }
};

