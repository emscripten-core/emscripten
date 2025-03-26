/**
 * @license
 * Copyright 2012 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var headlessPrint = (x) => {
  //print(x);
};

var window = {
  // adjustable parameters
  location: {
    toString() {
      return 'http://emscripten.org';
    },
    search: '',
    pathname: null,
  },
  onIdle() {
    headlessPrint('triggering click');
    document.querySelector('.fullscreen-button.low-res').callEventListeners('click');
    window.onIdle = null;
  },
  dirsToDrop: 0, // go back to root dir if first_js is in a subdir

  headless: true,

  stopped: false,
  fakeNow: 0, // we don't use Date.now()
  rafs: [],
  timeouts: [],
  uid: 0,
  requestAnimationFrame(func) {
    func.uid = window.uid++;
    headlessPrint(`adding raf ${func.uid}`);
    window.rafs.push(func);
  },
  setTimeout(func, ms) {
    func.uid = window.uid++;
    headlessPrint(`adding timeout ${func.uid}`);
    window.timeouts.push({
      func,
      when: window.fakeNow + (ms || 0)
    });
    window.timeouts.sort((x, y) => { return y.when - x.when });
  },
  runEventLoop() {
    // run forever until an exception stops this replay
    var iter = 0;
    while (!this.stopped) {
      var start = Date.realNow();
      headlessPrint(`event loop: ${(iter++)}`);
      if (window.rafs.length == 0 && window.timeouts.length == 0) {
        if (window.onIdle) {
          window.onIdle();
        } else {
          throw 'main loop is idle!';
        }
      }
      // rafs
      var currRafs = window.rafs;
      window.rafs = [];
      for (var i = 0; i < currRafs.length; i++) {
        var raf = currRafs[i];
        headlessPrint(`calling raf: ${raf.uid}`);// + ': ' + raf.toString().substring(0, 50));
        raf();
      }
      // timeouts
      var now = window.fakeNow;
      var timeouts = window.timeouts;
      window.timeouts = [];
      while (timeouts.length && timeouts[timeouts.length-1].when <= now) {
        var timeout = timeouts.pop();
        headlessPrint(`calling timeout: ${timeout.func.uid}`);// + ': ' + timeout.func.toString().substring(0, 50));
        timeout.func();
      }
      // increment 'time'
      window.fakeNow += 16.666;
      headlessPrint(`main event loop iteration took ${Date.realNow() - start} ms`);
    }
  },
  eventListeners: {},
  addEventListener(id, func) {
    var listeners = this.eventListeners[id];
    listeners ||= this.eventListeners[id] = [];
    listeners.push(func);
  },
  removeEventListener(id, func) {
    var listeners = this.eventListeners[id];
    if (!listeners) return;
    for (var i = 0; i < listeners.length; i++) {
      if (listeners[i] === func) {
        listeners.splice(i, 1);
        return;
      }
    }
  },
  callEventListeners(id) {
    var listeners = this.eventListeners[id];
    listeners?.forEach((listener) => listener());
  },
  URL: {
    createObjectURL(x) {
      return x; // the blob itself is returned
    },
    revokeObjectURL(x) {},
  },
  encodeURIComponent(x) { return x },
};
var setTimeout = window.setTimeout;
var document = {
  headless: true,
  eventListeners: {},
  addEventListener: window.addEventListener,
  removeEventListener: window.removeEventListener,
  callEventListeners: window.callEventListeners,
  getElementById(id) {
    switch (id) {
      case 'canvas': {
        if (this.canvas) return this.canvas;
        return this.canvas = headlessCanvas();
      }
      case 'status-text': case 'progress': {
        return {};
      }
      default: throw 'getElementById: ' + id;
    }
  },
  createElement(what) {
    switch (what) {
      case 'canvas': return document.getElementById(what);
      case 'script': {
        var ret = {};
        window.setTimeout(() => {
          headlessPrint(`loading script: ${ret.src}`);
          load(ret.src);
          headlessPrint('   script loaded.');
          if (ret.onload) {
            window.setTimeout(() => {
              ret.onload(); // yeah yeah this might vanish
            });
          }
        });
        return ret;
      }
      case 'div': {
        return {
          appendChild() {},
          requestFullscreen() {
            return document.getElementById('canvas').requestFullscreen();
          },
        };
      }
      default: throw `createElement ${what}${new Error().stack}`;
    }
  },
  elements: {},
  querySelector(id) {
    document.elements[id] ||= {
      classList: {
        add() {},
        remove() {},
      },
      eventListeners: {},
      addEventListener: document.addEventListener,
      removeEventListener: document.removeEventListener,
      callEventListeners: document.callEventListeners,
    };
    return document.elements[id];
  },
  styleSheets: [{
    cssRules: [],
    insertRule() {},
  }],
  body: {
    appendChild() {},
  },
  exitPointerLock() {},
  exitFullscreen() {},
};
var alert = function(x) {
  print(x);
};
var performance = {
  now() {
    return Date.now();
  },
};
function fixPath(path) {
  if (path[0] == '/') path = path.substring(1);
  for (var i = 0; i < window.dirsToDrop; i++) {
    path = '../' + path;
  }
  return path
}
var XMLHttpRequest = function() {
  return {
    open(mode, path, async) {
      path = fixPath(path);
      this.mode = mode;
      this.path = path;
      this.async = async;
    },
    send() {
      if (!this.async) {
        this.doSend();
      } else {
        window.setTimeout(() => {
          this.doSend();
          this.onload?.();
        }, 0);
      }
    },
    doSend() {
      if (this.responseType == 'arraybuffer') {
        this.response = read(this.path, 'binary');
      } else {
        this.responseText = read(this.path);
      }
    },
  };
};
var Audio = () => ({
  play() {},
  pause() {},
  cloneNode() {
    return this;
  },
});
var Image = () => {
  window.setTimeout(() => {
    this.complete = true;
    this.width = 64;
    this.height = 64;
    this.onload?.();
  });
};
var Worker = (workerPath) => {
  workerPath = fixPath(workerPath);
  var workerCode = read(workerPath);
  workerCode = workerCode.replace(/Module/g, 'zzModuleyy' + (Worker.id++)). // prevent collision with the global Module object. Note that this becomes global, so we need unique ids
                          replace(/\nonmessage = /, '\nvar onmessage = '); // workers commonly do "onmessage = ", we need to verify that to sandbox
  headlessPrint(`loading worker ${workerPath} : ${workerCode.substring(0, 50)}`);
  eval(workerCode); // will implement onmessage()

  function duplicateJSON(json) {
    function handleTypedArrays(key, value) {
      if (value?.toString && value.toString().substring(0, 8) == '[object ' && value.length && value.byteLength) {
        return Array.prototype.slice.call(value);
      }
      return value;
    }
    return JSON.parse(JSON.stringify(json, handleTypedArrays))
  }
  this.terminate = () => {};
  this.postMessage = (msg) => {
    msg.messageId = Worker.messageId++;
    headlessPrint(`main thread sending message ${msg.messageId} to worker ${workerPath}`);
    window.setTimeout(() => {
      headlessPrint(`worker ${workerPath} receiving message ${msg.messageId}`);
      onmessage({ data: duplicateJSON(msg) });
    });
  };
  var thisWorker = this;
  var postMessage = (msg) => {
    msg.messageId = Worker.messageId++;
    headlessPrint(`worker ${workerPath} sending message ${msg.messageId}`);
    window.setTimeout(() => {
      headlessPrint(`main thread receiving message ${msg.messageId} from ${workerPath}`);
      thisWorker.onmessage({ data: duplicateJSON(msg) });
    });
  };
};
Worker.id = 0;
Worker.messageId = 0;
var screen = { // XXX these values may need to be adjusted
  width: 2100,
  height: 1313,
  availWidth: 2100,
  availHeight: 1283,
};
if (typeof console == 'undefined') {
  console = {
    log(x) { print(x); },
  };
}

// additional setup
Module['canvas'] ||= document.getElementById('canvas');
