
//== HEADLESS ==//

var headlessPrint = function(x) {
  //print(x);
}

var window = {
  // adjustable parameters
  location: {
    toString: function() {
      return '%s';
    },
    search: '?%s',
    pathname: '%s',
  },
  onIdle: function(){ headlessPrint('triggering click'); document.querySelector('.fullscreen-button.low-res').callEventListeners('click'); window.onIdle = null; },
  dirsToDrop: 0, // go back to root dir if first_js is in a subdir
  //

  headless: true,

  stopped: false,
  fakeNow: 0, // we don't use Date.now()
  rafs: [],
  timeouts: [],
  uid: 0,
  requestAnimationFrame: function(func) {
    func.uid = window.uid++;
    headlessPrint('adding raf ' + func.uid);
    window.rafs.push(func);
  },
  setTimeout: function(func, ms) {
    func.uid = window.uid++;
    headlessPrint('adding timeout ' + func.uid);
    window.timeouts.push({
      func: func,
      when: window.fakeNow + (ms || 0)
    });
    window.timeouts.sort(function(x, y) { return y.when - x.when });
  },
  runEventLoop: function() {
    // run forever until an exception stops this replay
    var iter = 0;
    while (!this.stopped) {
      var start = Date.realNow();
      headlessPrint('event loop: ' + (iter++));
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
        headlessPrint('calling raf: ' + raf.uid);// + ': ' + raf.toString().substring(0, 50));
        raf();
      }
      // timeouts
      var now = window.fakeNow;
      var timeouts = window.timeouts;
      window.timeouts = [];
      while (timeouts.length && timeouts[timeouts.length-1].when <= now) {
        var timeout = timeouts.pop();
        headlessPrint('calling timeout: ' + timeout.func.uid);// + ': ' + timeout.func.toString().substring(0, 50));
        timeout.func();
      }
      // increment 'time'
      window.fakeNow += 16.666;
      headlessPrint('main event loop iteration took ' + (Date.realNow() - start) + ' ms');
    }
  },
  eventListeners: {},
  addEventListener: function(id, func) {
    var listeners = this.eventListeners[id];
    if (!listeners) {
      listeners = this.eventListeners[id] = [];
    }
    listeners.push(func);
  },
  removeEventListener: function(id, func) {
    var listeners = this.eventListeners[id];
    if (!listeners) return;
    for (var i = 0; i < listeners.length; i++) {
      if (listeners[i] === func) {
        listeners.splice(i, 1);
        return;
      }
    }
  },
  callEventListeners: function(id) {
    var listeners = this.eventListeners[id];
    if (listeners) {
      listeners.forEach(function(listener) { listener() });
    }
  },
  URL: {
    createObjectURL: function(x) {
      return x; // the blob itself is returned
    },
    revokeObjectURL: function(x) {},
  },
  encodeURIComponent: function(x) { return x },
};
var setTimeout = window.setTimeout;
var document = {
  headless: true,
  eventListeners: {},
  addEventListener: window.addEventListener,
  removeEventListener: window.removeEventListener,
  callEventListeners: window.callEventListeners,
  getElementById: function(id) {
    switch(id) {
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
  createElement: function(what) {
    switch (what) {
      case 'canvas': return document.getElementById(what);
      case 'script': {
        var ret = {};
        window.setTimeout(function() {
          headlessPrint('loading script: ' + ret.src);
          load(ret.src);
          headlessPrint('   script loaded.');
          if (ret.onload) {
            window.setTimeout(function() {
              ret.onload(); // yeah yeah this might vanish
            });
          }
        });
        return ret;
      }
      default: throw 'createElement ' + what;
    }
  },
  elements: {},
  querySelector: function(id) {
    if (!document.elements[id]) {
      document.elements[id] = {
        classList: {
          add: function(){},
          remove: function(){},
        },
        eventListeners: {},
        addEventListener: document.addEventListener,
        removeEventListener: document.removeEventListener,
        callEventListeners: document.callEventListeners,
      };
    };
    return document.elements[id];
  },
  styleSheets: [{
    cssRules: [],
    insertRule: function(){},
  }],
  body: {
    appendChild: function(){},
  },
  exitPointerLock: function(){},
  cancelFullScreen: function(){},
};
var alert = function(x) {
  print(x);
};
var performance = {
  now: function() {
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
    open: function(mode, path, async) {
      path = fixPath(path);
      this.mode = mode;
      this.path = path;
      this.async = async;
    },
    send: function() {
      if (!this.async) {
        this.doSend();
      } else {
        var that = this;
        window.setTimeout(function() {
          that.doSend();
          if (that.onload) that.onload();
        }, 0);
      }
    },
    doSend: function() {
      if (this.responseType == 'arraybuffer') {
        this.response = read(this.path, 'binary');
      } else {
        this.responseText = read(this.path);
      }
    },
  };
};
var Audio = function() {
  return {
    play: function(){},
    pause: function(){},
    cloneNode: function() {
      return this;
    },
  };
};
var Image = function() {
  var that = this;
  window.setTimeout(function() {
    that.complete = true;
    that.width = 64;
    that.height = 64;
    if (that.onload) that.onload();
  });
};
var Worker = function(workerPath) {
  workerPath = fixPath(workerPath);
  var workerCode = read(workerPath);
  workerCode = workerCode.replace(/Module/g, 'zzModuleyy' + (Worker.id++)). // prevent collision with the global Module object. Note that this becomes global, so we need unique ids
                          //replace(/Date.now/g, 'Recorder.dnow'). // recorded values are just for the "main thread" - workers were not recorded, and should not consume
                          //replace(/performance.now/g, 'Recorder.pnow').
                          //replace(/Math.random/g, 'Recorder.random').
                          replace(/\nonmessage = /, '\nvar onmessage = '); // workers commonly do "onmessage = ", we need to varify that to sandbox
  headlessPrint('loading worker ' + workerPath + ' : ' + workerCode.substring(0, 50));
  eval(workerCode); // will implement onmessage()

  function duplicateJSON(json) {
    function handleTypedArrays(key, value) {
      if (value && value.toString && value.toString().substring(0, 8) == '[object ' && value.length && value.byteLength) {
        return Array.prototype.slice.call(value);
      }
      return value;
    }
    return JSON.parse(JSON.stringify(json, handleTypedArrays))
  }
  this.terminate = function(){};
  this.postMessage = function(msg) {
    msg.messageId = Worker.messageId++;
    headlessPrint('main thread sending message ' + msg.messageId + ' to worker ' + workerPath);
    window.setTimeout(function() {
      headlessPrint('worker ' + workerPath + ' receiving message ' + msg.messageId);
      onmessage({ data: duplicateJSON(msg) });
    });
  };
  var thisWorker = this;
  var postMessage = function(msg) {
    msg.messageId = Worker.messageId++;
    headlessPrint('worker ' + workerPath + ' sending message ' + msg.messageId);
    window.setTimeout(function() {
      headlessPrint('main thread receiving message ' + msg.messageId + ' from ' + workerPath);
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
var console = {
  log: function(x) {
    print(x);
  },
};
var MozBlobBuilder = function() {
  this.data = new Uint8Array(0);
  this.append = function(buffer) {
    var data = new Uint8Array(buffer);
    var combined = new Uint8Array(this.data.length + data.length);
    combined.set(this.data);
    combined.set(data, this.data.length);
    this.data = combined;
  };
  this.getBlob = function() {
    return this.data.buffer; // return the buffer as a "blob". XXX We might need to change this if it is not opaque
  };
};

// additional setup
if (!Module['canvas']) {
  Module['canvas'] = document.getElementById('canvas');
}

//== HEADLESS ==//

