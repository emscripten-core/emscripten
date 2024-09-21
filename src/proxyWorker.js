/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
 * Implements the server/worker side of proxyClient.js.
 * This code gets included in the main emscripten output
 * when PROXY_TO_WORKER is used. The resulting code then
 * needs to be run in a worker and receive events from
 * proxyClient.js running on the main thread.
 */

#if !PROXY_TO_WORKER
#error "proxyClient.js should only be included in PROXY_TO_WORKER mode"
#endif

#if ENVIRONMENT_MAY_BE_NODE
if (!ENVIRONMENT_IS_NODE) {
#endif

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
  }
}

function Element() { throw 'TODO: Element' }
function HTMLCanvasElement() { throw 'TODO: HTMLCanvasElement' }
function HTMLVideoElement() { throw 'TODO: HTMLVideoElement' }

var KeyboardEvent = {
  'DOM_KEY_LOCATION_RIGHT': 2,
};

function PropertyBag() {
  this.addProperty = () => {};
  this.removeProperty = () => {};
  this.setProperty = () => {};
};

var IndexedObjects = {
  nextId: 1,
  cache: {},
  add(object) {
    object.id = this.nextId++;
    this.cache[object.id] = object;
  }
};

function EventListener() {
  this.listeners = {};

  this.addEventListener = function addEventListener(event, func) {
    this.listeners[event] ||= [];
    this.listeners[event].push(func);
  };

  this.removeEventListener = function(event, func) {
    var list = this.listeners[event];
    if (!list) return;
    var me = list.indexOf(func);
    if (me < 0) return;
    list.splice(me, 1);
  };

  this.fireEvent = function(event) {
    event.preventDefault = () => {};

    if (event.type in this.listeners) {
      this.listeners[event.type].forEach((listener) => listener(event));
    }
  }
}

function Image() {
  IndexedObjects.add(this);
  EventListener.call(this);
  var src = '';
  Object.defineProperty(this, 'src', {
    set: (value) => {
      src = value;
      assert(this.id);
      postMessage({ target: 'Image', method: 'src', src, id: this.id });
    },
    get: () => src
  });
}
Image.prototype.onload = () => {};
Image.prototype.onerror = () => {};

var HTMLImageElement = Image;

var window = this;
var windowExtra = new EventListener();
for (var x in windowExtra) window[x] = windowExtra[x];

window.close = () => {
  postMessage({ target: 'window', method: 'close' });
};

window.alert = (text) => {
  err(`alert forever: ${text}`);
  while (1) {};
};

window.scrollX = window.scrollY = 0; // TODO: proxy these

window.WebGLRenderingContext = WebGLWorker;

window.requestAnimationFrame = (() => {
  // similar to Browser.requestAnimationFrame
  var nextRAF = 0;
  return (func) => {
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

document.createElement = (what) => {
  switch (what) {
    case 'canvas': {
      var canvas = new EventListener();
      canvas.ensureData = () => {
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
      canvas.getContext = (type, attributes) => {
        if (canvas === Module['canvas']) {
          postMessage({ target: 'canvas', op: 'getContext', type, attributes });
        }
        if (type === '2d') {
          return {
            getImageData: (x, y, w, h) => {
              assert(x == 0 && y == 0 && w == canvas.width && h == canvas.height);
              canvas.ensureData();
              return {
                width: canvas.data.width,
                height: canvas.data.height,
                data: new Uint8Array(canvas.data.data) // TODO: can we avoid this copy?
              };
            },
            putImageData: (image, x, y) => {
              canvas.ensureData();
              assert(x == 0 && y == 0 && image.width == canvas.width && image.height == canvas.height);
              canvas.data.data.set(image.data); // TODO: can we avoid this copy?
              if (canvas === Module['canvas']) {
                postMessage({ target: 'canvas', op: 'render', image: canvas.data });
              }
            },
            drawImage: (image, x, y, w, h, ox, oy, ow, oh) => {
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
      canvas.getBoundingClientRect = () => ({
        width: canvas.boundingClientRect.width,
        height: canvas.boundingClientRect.height,
        top: canvas.boundingClientRect.top,
        left: canvas.boundingClientRect.left,
        bottom: canvas.boundingClientRect.bottom,
        right: canvas.boundingClientRect.right
      });
      canvas.style = new PropertyBag();
      canvas.exitPointerLock = () => {};

      canvas.width_ ||= 0;
      canvas.height_ ||= 0;
      Object.defineProperty(canvas, 'width', {
        set: (value) => {
          canvas.width_ = value;
          if (canvas === Module['canvas']) {
            postMessage({ target: 'canvas', op: 'resize', width: canvas.width_, height: canvas.height_ });
          }
        },
        get: () => canvas.width_
      });
      Object.defineProperty(canvas, 'height', {
        set: (value) => {
          canvas.height_ = value;
          if (canvas === Module['canvas']) {
            postMessage({ target: 'canvas', op: 'resize', width: canvas.width_, height: canvas.height_ });
          }
        },
        get: () => canvas.height_
      });

      var style = {
        parentCanvas: canvas,
        removeProperty: () => {},
        setProperty:  () => {},
      };

      Object.defineProperty(style, 'cursor', {
        set: (value) => {
          if (!style.cursor_ || style.cursor_ !== value) {
            style.cursor_ = value;
            if (style.parentCanvas === Module['canvas']) {
              postMessage({ target: 'canvas', op: 'setObjectProperty', object: 'style', property: 'cursor', value: style.cursor_ });
            }
          }
        },
        get: () => style.cursor,
      });

      canvas.style = style;
      return canvas;
    }
    default: {
      throw 'document.createElement ' + what;
    }
  }
};

document.getElementById = (id) => {
  if (id === 'canvas' || id === 'application-canvas') {
    return Module.canvas;
  }
  throw 'document.getElementById failed on ' + id;
};

document.querySelector = (id) => {
  if (id === '#canvas' || id === '#application-canvas' || id === 'canvas' || id === 'application-canvas') {
    return Module.canvas;
  }
  throw 'document.querySelector failed on ' + id;
};

document.documentElement = {};

document.styleSheets = [{
  cssRules: [], // TODO: forward to client
  insertRule(rule, i) {
    this.cssRules.splice(i, 0, rule);
  }
}];

document.URL = 'http://worker.not.yet.ready.wait.for.window.onload?fake';

function Audio() {
  warnOnce('faking Audio elements, no actual sound will play');
}
Audio.prototype = new EventListener();
Object.defineProperty(Audio.prototype, 'src', {
  set(value) {
    if (value[0] === 'd') return; // ignore data urls
    this.onerror();
  },
});

Audio.prototype.play = () => {};
Audio.prototype.pause = () => {};

Audio.prototype.cloneNode = () => new Audio;

function AudioContext() {
  warnOnce('faking WebAudio elements, no actual sound will play');
  var makeNode = () => {
    return {
      connect: () => {},
      disconnect: () => {},
    }
  };
  this.listener = {
    setPosition: () => {},
    setOrientation: () => {},
  };
  this.decodeAudioData = () => {}; // ignore callbacks
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

Module.setStatus = () => {};

out = (x) => {
  //dump('OUT: ' + x + '\n');
  postMessage({ target: 'stdout', content: x });
};
err = (x) => {
  //dump('ERR: ' + x + '\n');
  postMessage({ target: 'stderr', content: x });
};

// Frame throttling

var frameId = 0;
var clientFrameId = 0;

var postMainLoop = Module['postMainLoop'];
Module['postMainLoop'] = () => {
  postMainLoop?.();
  // frame complete, send a frame id
  postMessage({ target: 'tick', id: frameId++ });
  commandBuffer = [];
};

// Wait to start running until we receive some info from the client

#if PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif
  addRunDependency('gl-prefetch');
  addRunDependency('worker-init');
#if PTHREADS
}
#endif

#if ENVIRONMENT_MAY_BE_NODE
}
#endif

// buffer messages until the program starts to run

var messageBuffer = null;
var messageResenderTimeout = null;
var calledMain = false;

// Set calledMain to true during postRun which happens once main returns
Module['postRun'] ||= [];
if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
Module['postRun'].push(() => { calledMain = true; });

function messageResender() {
  if (calledMain) {
    assert(messageBuffer && messageBuffer.length > 0);
    messageResenderTimeout = null;
    messageBuffer.forEach(onmessage);
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
#if ENVIRONMENT_MAY_BE_NODE
      if (ENVIRONMENT_IS_NODE)
#endif
      document.URL = message.data.URL;
#if PTHREADS
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
    case 'setimmediate': {
      Module['setImmediates']?.shift()();
      break;
    }
    default: throw 'wha? ' + message.data.target;
  }
};

#if PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif
  onmessage = onMessageFromMainEmscriptenThread;
#if PTHREADS
}
#endif

// proxyWorker.js has defined 'document' and 'window' objects above, so need to
// initialize them for library_html5.js explicitly here.
if (typeof specialHTMLTargets != 'undefined') {
  specialHTMLTargets = [0, document, window];
}

function postCustomMessage(data) {
  postMessage({ target: 'custom', userData: data });
}
