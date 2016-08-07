
if (typeof console === 'undefined') {
  // we can't call Module.printErr because that might be circular
  var console = {
    log: function(x) {
      if (typeof dump === 'function') dump('log: ' + x + '\n');
    },
    debug: function(x) {
      if (typeof dump === 'function') dump('debug: ' + x + '\n');
    },
    info: function(x) {
      if (typeof dump === 'function') dump('info: ' + x + '\n');
    },
    warn: function(x) {
      if (typeof dump === 'function') dump('warn: ' + x + '\n');
    },
    error: function(x) {
      if (typeof dump === 'function') dump('error: ' + x + '\n');
    },
  };
}

/*
function proxify(object, nick) {
  return new Proxy(object, {
    get: function(target, name) {
      var ret = target[name];
      if (ret === undefined) console.log('PROXY ' + [nick, target, name, ret, typeof ret]);
      return ret;
    }
  });
}
*/

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
  }
}

function Element() { throw 'TODO: Element' }
function HTMLCanvasElement() { throw 'TODO: HTMLCanvasElement' }
function HTMLVideoElement() { throw 'TODO: HTMLVideoElement' }

var KeyboardEvent = {
  'DOM_KEY_LOCATION_RIGHT': 2,
};

function PropertyBag() {
  this.addProperty = function(){};
  this.removeProperty = function(){};
  this.setProperty = function(){};
};

var IndexedObjects = {
  nextId: 1,
  cache: {},
  add: function(object) {
    object.id = this.nextId++;
    this.cache[object.id] = object;
  }
};

function EventListener() {
  this.listeners = {};

  this.addEventListener = function addEventListener(event, func) {
    if (!this.listeners[event]) this.listeners[event] = [];
    this.listeners[event].push(func);
  };

  this.removeEventListener = function(event, func) {
    var list = this.listeners[event];
    if (!list) return;
    var me = list.indexOf(func);
    if (me < 0) return;
    list.splice(me, 1);
  };

  this.fireEvent = function fireEvent(event) {
    event.preventDefault = function(){};

    if (event.type in this.listeners) {
      this.listeners[event.type].forEach(function(listener) {
        listener(event);
      });
    }
  };
}

function Image() {
  IndexedObjects.add(this);
  EventListener.call(this);
  var src = '';
  Object.defineProperty(this, 'src', {
    set: function(value) {
      src = value;
      assert(this.id);
      postMessage({ target: 'Image', method: 'src', src: src, id: this.id });
    },
    get: function() {
      return src;
    }
  });
}
Image.prototype.onload = function(){};
Image.prototype.onerror = function(){};

var HTMLImageElement = Image;

var window = this;
var windowExtra = new EventListener();
for (var x in windowExtra) window[x] = windowExtra[x];

window.close = function window_close() {
  postMessage({ target: 'window', method: 'close' });
};

window.alert = function(text) {
  Module.printErr('alert forever: ' + text);
  while (1){};
};

window.scrollX = window.scrollY = 0; // TODO: proxy these

window.WebGLRenderingContext = WebGLWorker;

window.requestAnimationFrame = (function() {
  // similar to Browser.requestAnimationFrame
  var nextRAF = 0;
  return function(func) {
    // try to keep 60fps between calls to here
    var now = Date.now();
    if (nextRAF === 0) {
      nextRAF = now + 1000/60;
    } else {
      while (now + 2 >= nextRAF) { // fudge a little, to avoid timer jitter causing us to do lots of delay:0
        nextRAF += 1000/60;
      }
    }
    var delay = Math.max(nextRAF - now, 0);
    setTimeout(func, delay);
  };
})();

var webGLWorker = new WebGLWorker();

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
          if (canvas === Module['canvas']) {
            postMessage({ target: 'canvas', op: 'resize', width: canvas.width, height: canvas.height });
          }
        }
      };
      canvas.getContext = function canvas_getContext(type, attributes) {
        if (canvas === Module['canvas']) {
          postMessage({ target: 'canvas', op: 'getContext', type: type, attributes: attributes });
        }
        if (type === '2d') {
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
              if (canvas === Module['canvas']) {
                postMessage({ target: 'canvas', op: 'render', image: canvas.data });
              }
            },
            drawImage: function(image, x, y, w, h, ox, oy, ow, oh) {
              assert (!x && !y && !ox && !oy);
              assert(w === ow && h === oh);
              assert(canvas.width === w || w === undefined);
              assert(canvas.height === h || h === undefined);
              assert(image.width === canvas.width && image.height === canvas.height);
              canvas.ensureData();
              canvas.data.data.set(image.data.data); // TODO: can we avoid this copy?
              if (canvas === Module['canvas']) {
                postMessage({ target: 'canvas', op: 'render', image: canvas.data });
              }
            }
          };
        } else {
          return webGLWorker;
        }
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
      canvas.style = new PropertyBag();
      canvas.exitPointerLock = function(){};

      canvas.width_ = canvas.width_ || 0;
      canvas.height_ = canvas.height_ || 0;
      Object.defineProperty(canvas, 'width', {
        set: function(value) {
          canvas.width_ = value;
          if (canvas === Module['canvas']) {
            postMessage({ target: 'canvas', op: 'resize', width: canvas.width_, height: canvas.height_ });
          }
        },
        get: function() {
          return canvas.width_;
        }
      });
      Object.defineProperty(canvas, 'height', {
        set: function(value) {
          canvas.height_ = value;
          if (canvas === Module['canvas']) {
            postMessage({ target: 'canvas', op: 'resize', width: canvas.width_, height: canvas.height_ });
          }
        },
        get: function() {
          return canvas.height_;
        }
      });

      var style = {
        parentCanvas: canvas,
        removeProperty: function(){},
        setProperty:  function(){},
      };

      Object.defineProperty(style, 'cursor', {
        set: function(value) {
          if (!style.cursor_ || style.cursor_ !== value) {
            style.cursor_ = value;
            if (style.parentCanvas === Module['canvas']) {
              postMessage({ target: 'canvas', op: 'setObjectProperty', object: 'style', property: 'cursor', value: style.cursor_ });
            }
          }
        },
        get: function() {
          return style.cursor_;
        }
      });

      canvas.style = style;

      return canvas;
    }
    default: throw 'document.createElement ' + what;
  }
};

document.getElementById = function(id) {
  if (id === 'canvas' || id === 'application-canvas') {
    return Module.canvas;
  }
  throw 'document.getElementById failed on ' + id;
};

document.documentElement = {};

document.styleSheets = [{
  cssRules: [], // TODO: forward to client
  insertRule: function(rule, i) {
    this.cssRules.splice(i, 0, rule);
  }
}];

document.URL = 'http://worker.not.yet.ready.wait.for.window.onload?fake';

function Audio() {
  Runtime.warnOnce('faking Audio elements, no actual sound will play');
}
Audio.prototype = new EventListener();
Object.defineProperty(Audio.prototype, 'src', {
  set: function(value) {
    if (value[0] === 'd') return; // ignore data urls
    this.onerror();
  },
});

Audio.prototype.play = function(){};
Audio.prototype.pause = function(){};

Audio.prototype.cloneNode = function() {
  return new Audio;
}

function AudioContext() {
  Runtime.warnOnce('faking WebAudio elements, no actual sound will play');
  function makeNode() {
    return {
      connect: function(){},
      disconnect: function(){},
    }
  }
  this.listener = {
    setPosition: function() {},
    setOrientation: function() {},
  };
  this.decodeAudioData = function() {}; // ignore callbacks
  this.createBuffer = makeNode;
  this.createBufferSource = makeNode;
  this.createGain = makeNode;
  this.createPanner = makeNode;
}

var screen = {
  width: 0,
  height: 0
};

Module.canvas = document.createElement('canvas');

Module.setStatus = function(){};

Module.print = function Module_print(x) {
  //dump('OUT: ' + x + '\n');
  postMessage({ target: 'stdout', content: x });
};
Module.printErr = function Module_printErr(x) {
  //dump('ERR: ' + x + '\n');
  postMessage({ target: 'stderr', content: x });
};

// Frame throttling

var frameId = 0;
var clientFrameId = 0;

var postMainLoop = Module['postMainLoop'];
Module['postMainLoop'] = function() {
  if (postMainLoop) postMainLoop();
  // frame complete, send a frame id
  postMessage({ target: 'tick', id: frameId++ });
  commandBuffer = [];
};

// Wait to start running until we receive some info from the client

#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif
  addRunDependency('gl-prefetch');
  addRunDependency('worker-init');
#if USE_PTHREADS
}
#endif

// buffer messages until the program starts to run

var messageBuffer = null;
var messageResenderTimeout = null;

function messageResender() {
  if (calledMain) {
    assert(messageBuffer && messageBuffer.length > 0);
    messageResenderTimeout = null;
    messageBuffer.forEach(function(message) {
      onmessage(message);
    });
    messageBuffer = null;
  } else {
    messageResenderTimeout = setTimeout(messageResender, 100);
  }
}

function onMessageFromMainEmscriptenThread(message) {
  if (!calledMain && !message.data.preMain) {
    if (!messageBuffer) {
      messageBuffer = [];
      messageResenderTimeout = setTimeout(messageResender, 100);
    }
    messageBuffer.push(message);
    return;
  }
  if (calledMain && messageResenderTimeout) {
    clearTimeout(messageResenderTimeout);
    messageResender();
  }
  //dump('worker got ' + JSON.stringify(message.data).substr(0, 150) + '\n');
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
    case 'gl': {
      webGLWorker.onmessage(message.data);
      break;
    }
    case 'tock': {
      clientFrameId = message.data.id;
      break;
    }
    case 'Image': {
      var img = IndexedObjects.cache[message.data.id];
      switch (message.data.method) {
        case 'onload': {
          img.width = message.data.width;
          img.height = message.data.height;
          img.data = { width: img.width, height: img.height, data: message.data.data };
          img.complete = true;
          img.onload();
          break;
        }
        case 'onerror': {
          img.onerror({ srcElement: img });
          break;
        }
      }
      break;
    }
    case 'IDBStore': {
      assert(message.data.method === 'response');
      assert(IDBStore.pending);
      IDBStore.pending(message.data);
      break;
    }
    case 'worker-init': {
      Module.canvas = document.createElement('canvas');
      screen.width = Module.canvas.width_ = message.data.width;
      screen.height = Module.canvas.height_ = message.data.height;
      Module.canvas.boundingClientRect = message.data.boundingClientRect;
      document.URL = message.data.URL;
#if USE_PTHREADS
      currentScriptUrl = message.data.currentScriptUrl;
#endif
      window.fireEvent({ type: 'load' });
      removeRunDependency('worker-init');
      break;
    }
    case 'custom': {
      if (Module['onCustomMessage']) {
        Module['onCustomMessage'](message);
      } else {
        throw 'Custom message received but worker Module.onCustomMessage not implemented.';
      }
      break;
    }
    default: throw 'wha? ' + message.data.target;
  }
};

#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif
  onmessage = onMessageFromMainEmscriptenThread;
#if USE_PTHREADS
}
#endif

function postCustomMessage(data) {
  postMessage({ target: 'custom', userData: data });
}
