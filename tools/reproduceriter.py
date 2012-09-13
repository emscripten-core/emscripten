#!/usr/bin/env python

'''

* This is a work in progress *

Reproducer Rewriter
===================

Processes a project and rewrites it so as to generate deterministic,
reproducible automatic results. For example, you can run this on a
game, and then when the game is run it will record user input and
sources of nondeterminism like Math.random(). You can then run
that recording as a benchmark or as a demo, it should give nearly
identical results every time it is run to the data that was
recorded.

Usage:

1. Run this script as

    reproduceriter.py IN_DIR OUT_DIR FIRST_JS [WINDOW_LOCATION] [ON_IDLE]

   IN_DIR should be the project directory, and OUT_DIR will be
   created with the instrumented code (OUT_DIR will be overwritten
   if it exists). FIRST_JS should be a path (relative to IN_DIR) to
   the first JavaScript file loaded by the project (this tool
   will add code to that). 

   You will need to call

    Recorder.start();

   at the right time to start the relevant event loop. For
   example, if your application is a game that starts receiving
   events when in fullscreen, add something like

    if (typeof Recorder != 'undefined') Recorder.start();

   in the button that launches fullscreen. start() will start
   either recording when in record mode, or replaying when
   in replay mode, so you need this in both modes.

2. Run the instrumented project in OUR_DIR and interact with
   the program. When you are done recording, open the web
   console and run

    Recorder.finish();

   This will write out the recorded data into the current tab.
   Save it as

    repro.data

   in OUT_DIR.

3. To re-play the recorded data, run the instrumented build
   with

    &reproduce=repro.data

   Note that as mentioned above you need to call

    Recorder.start();

   when the recorded event loop should start to replay.

Notes:

 * When we start to replay events, the assumption is that
   there is nothing asynchronous that affects execution. So
   asynchronous loading of files should have already
   completed.

   TODO: start running recorded events with some trigger, for example the fullscreen button in BananaBread

Examples

 * BananaBread: Unpack into a directory called bb, then one
   directory up, run

    emscripten/tools/reproduceriter.py bb bench js/game-setup.js game.html?low,low,reproduce=repro.data "function(){ print('triggering click'); document.querySelector('.fullscreen-button.low-res').callEventListeners('click'); window.onIdle = null; }"

   The last parameter specifies what to do when the event loop is idle: We fire an event and then set onIdle (which was this function) to null, so this is a one-time occurence.
'''

import os, sys, shutil, re

assert len(sys.argv) >= 4, 'Usage: reproduceriter.py IN_DIR OUT_DIR FIRST_JS [WINDOW_LOCATION]'

# Process input args

in_dir = sys.argv[1]
out_dir = sys.argv[2]
first_js = sys.argv[3]
window_location = sys.argv[4] if len(sys.argv) >= 5 else ''
on_idle = sys.argv[5] if len(sys.argv) >= 6 else ''

dirs_to_drop = 0 if not os.path.dirname(first_js) else len(os.path.dirname(first_js).split('/'))

if os.path.exists(out_dir):
  shutil.rmtree(out_dir)
assert os.path.exists(os.path.join(in_dir, first_js))

# Copy project

print 'copying tree...'

shutil.copytree(in_dir, out_dir)

# Add customizations in all JS files

print 'add customizations...'

for parent, dirs, files in os.walk(out_dir):
  for filename in files:
    if filename.endswith('.js'):
      fullname = os.path.join(parent, filename)
      print '   ', fullname
      js = open(fullname).read()
      js = re.sub('document\.on(\w+) ?= ?([\w.$]+)', lambda m: 'Recorder.onEvent("' + m.group(1) + '", ' + m.group(2) + ')', js)
      js = re.sub('''([\w.'"\[\]]+)\.addEventListener\(([\w,. $]+)\)''', lambda m: 'Recorder.addListener(' + m.group(1) + ', ' + m.group(2) + ')', js)
      open(fullname, 'w').write(js)

# Add our boilerplate

print 'add boilerplate...'

open(os.path.join(out_dir, first_js), 'w').write('''

// environment for shell
if (typeof nagivator == 'undefined') {
''' + #open(os.path.dirname(__file__) + os.path.sep + 'dom.js').read() +
'''
///*
  var window = {
    location: {
      toString: function() {
        return '%s';
      },
      search: '?%s',
    },
    fakeNow: 0, // we don't use Date.now()
    rafs: [],
    timeouts: [],
    requestAnimationFrame: function(func) {
      window.rafs.push(func);
    },
    setTimeout: function(func, ms) {
      window.timeouts.push({
        func: func,
        when: window.fakeNow + (ms || 0)
      });
      window.timeouts.sort(function(x, y) { return y.when - x.when });
    },
    onIdle: %s,
    runEventLoop: function() {
      // run forever until an exception stops this replay
      var i = 0;
      while (1) {
        print('event loop: ' + (i++));
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
          print('calling raf: ' + currRafs[i].toString().substring(0, 50));
          currRafs[i]();
        }
        // timeouts
        var now = window.fakeNow;
        while (window.timeouts.length && window.timeouts[window.timeouts.length-1].when <= now) {
          var timeout = window.timeouts.pop();
          print('calling timeout: ' + timeout.func.toString().substring(0, 50));
          timeout.func();
        }
        // increment 'time'
        window.fakeNow += 16.666;
      }
    },
  };
  var setTimeout = window.setTimeout;
  var document = {
    eventListeners: {},
    addEventListener: function(id, func) {
      var listeners = this.eventListeners[id];
      if (!listeners) {
        listeners = this.eventListeners[id] = [];
      }
      listeners.push(func);
    },
    callEventListeners: function(id) {
      var listeners = this.eventListeners[id];
      if (listeners) {
        listeners.forEach(function(listener) { listener() });
      }
    },
    getElementById: function(id) {
      switch(id) {
        case 'canvas': {
          return {
            getContext: function(which) {
              switch(which) {
                case 'experimental-webgl': {
                  return {
                    getExtension: function() { return 1 },
                  };
                }
                default: throw 'canvas.getContext: ' + which;
              }
            },
            requestPointerLock: function() {
              document.callEventListeners('pointerlockchange');
            },
          };
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
            print('loading script: ' + ret.src);
            load(ret.src);
            print('   script loaded.');
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
  };
//*/
  var alert = function(x) {
    print(x);
  };
  var performance = {
    now: function() {
      print('performance.now! ' + new Error().stack);
      return Date.now(); // XXX XXX XXX
    },
  };
  function fixPath(path) {
    if (path[0] == '/') path = path.substring(1);
    var dirsToDrop = %d; // go back to root dir if first_js is in a subdir
    for (var i = 0; i < dirsToDrop; i++) {
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
    return { play: function(){} };
  };
  var Worker = function(path) {
    path = fixPath(path);
    var workerCode = read(path);
    workerCode = workerCode.replace(/Module/g, 'zzModuleyy' + (Worker.id++)); // prevent collision with the global Module object. Note that this becomes global, so we need unique ids
    print('loading worker ' + path + ' : ' + workerCode.substring(0, 50));
    eval(workerCode); // will implement onmessage()

    this.terminate = function(){};
    this.postMessage = function(msg) {
      window.setTimeout(function() {
        onmessage(msg);
      });
    };
    var thisWorker = this;
    var postMessage = function(msg) {
      if (thisWorker.onmessage) {
        window.setTimeout(function() {
          thisWorker.onmessage(msg);
        });
      }
    };
  };
  Worker.id = 0;
  var screen = {
    width: 800,
    height: 600,
    availWidth: 800,
    availHeight: 600,
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
      return this.data.buffer; // XXX we should change this
    };
  };
}

var Recorder = (function() {
  var recorder;
  var init = 'reproduce=';
  var initLocation = window.location.search.indexOf(init);
  var replaying = initLocation >= 0;
  var raf = window['requestAnimationFrame'] ||
            window['mozRequestAnimationFrame'] ||
            window['webkitRequestAnimationFrame'] ||
            window['msRequestAnimationFrame'] ||
            window['oRequestAnimationFrame'];
  if (!replaying) {
    // Prepare to record
    recorder = {};
    // Start
    recorder.frameCounter = 0; // the frame counter is used to know when to replay events
    recorder.start = function() {
      alert("Starting recording! Don't forget to Recorder.finish() afterwards!");
      function count() {
        recorder.frameCounter++;
        raf(count);
      }
      count();
      recorder.started = true;
    };
    // Math.random
    recorder.randoms = [];
    var random = Math.random;
    Math.random = function() {
      var ret = random();
      recorder.randoms.push(ret);
      return ret;
    };
    // Date.now, performance.now
    recorder.dnows = [];
    recorder.dnow = Date.now;
    Date.now = function() {
      var ret = recorder.dnow();
      recorder.dnows.push(ret);
      return ret;
    };
    recorder.pnows = [];
    recorder.pnow = performance.now;
    performance.now = function() {
      var ret = recorder.pnow();
      recorder.pnows.push(ret);
      return ret;
    };
    // Events
    recorder.devents = []; // document events
    recorder.onEvent = function(which, callback) {
      document['on' + which] = function(event) {
        if (!recorder.started) return true;
        event.frameCounter = recorder.frameCounter;
        recorder.devents.push(event);
        return callback(event); // XXX do we need to record the return value?
      };
    };
    recorder.tevents = []; // custom-target events. Currently we assume a single such custom target (aside from document), e.g., a canvas for the game.
    recorder.addListener = function(target, which, callback, arg) {
      target.addEventListener(which, function(event) {
        if (!recorder.started) return true;
        event.frameCounter = recorder.frameCounter;
        recorder.tevents.push(event);
        return callback(event); // XXX do we need to record the return value?
      }, arg);
    };
    // Finish
    recorder.finish = function() {
      // Reorder data because pop() is faster than shift()
      recorder.randoms.reverse();
      recorder.dnows.reverse();
      recorder.pnows.reverse();
      recorder.devents.reverse();
      recorder.tevents.reverse();
      // Make JSON.stringify work on data from native event objects (and only store relevant ones)
      var importantProperties = {
        type: 1,
        movementX: 1, mozMovementX: 1, webkitMovementX: 1,
        movementY: 1, mozMovementY: 1, webkitMovementY: 1,
        detail: 1,
        wheelDelta: 1,
        pageX: 1,
        pageY: 1,
        button: 1,
        keyCode: 1,
        frameCounter: 1
      };
      function importantize(event) {
        var ret = {};
        for (var prop in importantProperties) {
          if (prop in event) {
            ret[prop] = event[prop];
          }
        }
        return ret;
      }
      recorder.devents = recorder.devents.map(importantize);
      recorder.tevents = recorder.tevents.map(importantize);
      // Write out
      alert('Writing out data, remember to save!');
      setTimeout(function() {
        document.open();
        document.write(JSON.stringify(recorder));
        document.close();
      }, 0);
      return '.';
    };
  } else {
    // Load recording
    var dataPath = window.location.search.substring(initLocation + init.length);
    var baseURL = window.location.toString().replace('://', 'cheez999').split('?')[0].split('/').slice(0, -1).join('/').replace('cheez999', '://');
    if (baseURL[baseURL.length-1] != '/') baseURL += '/';
    var path = baseURL + dataPath;
    alert('Loading replay from ' + path);
    var request = new XMLHttpRequest();
    request.open('GET', path, false);
    request.send();
    var raw = request.responseText;
    raw = raw.substring(raw.indexOf('{'), raw.lastIndexOf('}')+1); // remove <html> etc
    recorder = JSON.parse(raw);
    // prepare to replay
    // Start
    recorder.frameCounter = 0; // the frame counter is used to know when to replay events
    recorder.start = function() {
      function count() {
        recorder.frameCounter++;
        raf(count);
        // replay relevant events for this frame
        while (recorder.devents.length && recorder.devents[recorder.devents.length-1].frameCounter <= recorder.frameCounter) {
          var event = recorder.devents.pop();
          recorder['on' + event.type](event);
        }
        while (recorder.tevents.length && recorder.tevents[recorder.tevents.length-1].frameCounter <= recorder.frameCounter) {
          var event = recorder.tevents.pop();
          recorder['event' + event.type](event);
        }
      }
      count();
    };
    // Math.random
    Math.random = function() {
      if (recorder.randoms.length > 0) {
        return recorder.randoms.pop();
      } else {
        recorder.finish();
        throw 'consuming too many values!';
      }
    };
    // Date.now, performance.now
    recorder.dnow = Date.now;
    Date.now = function() {
      if (recorder.dnows.length > 0) {
        return recorder.dnows.pop();
      } else {
        recorder.finish();
        throw 'consuming too many values!';
      }
    };
    var pnow = performance.now || performance.webkitNow || performance.mozNow || performance.oNow || performance.msNow || dnow;
    recorder.pnow = function() { return pnow.call(performance) };
    performance.now = function() {
      if (recorder.pnows.length > 0) {
        return recorder.pnows.pop();
      } else {
        recorder.finish();
        throw 'consuming too many values!';
      }
    };
    // Events
    recorder.onEvent = function(which, callback) {
      recorder['on' + which] = callback;
    };
    recorder.eventCallbacks = {};
    recorder.addListener = function(target, which, callback, arg) {
      recorder['event' + which] = callback;
    };
    recorder.onFinish = [];
    // Benchmarking hooks - emscripten specific
    setTimeout(function() {
      var totalTime = 0;
      var totalSquared = 0;
      var iterations = 0;
      var maxTime = 0;
      var curr = 0;
      Module.preMainLoop = function() {
        curr = recorder.pnow();
      }
      Module.postMainLoop = function() {
        var time = recorder.pnow() - curr;
        totalTime += time;
        totalSquared += time*time;
        maxTime = Math.max(maxTime, time);
        iterations++;
      };
      recorder.onFinish.push(function() {
        var mean = totalTime / iterations;
        var meanSquared = totalSquared / iterations;
        console.log('mean frame   : ' + mean + ' ms');
        console.log('frame std dev: ' + Math.sqrt(meanSquared - (mean*mean)) + ' ms');
        console.log('max frame    : ' + maxTime + ' ms');
      });    
    });
    // Finish
    recorder.finish = function() {
      recorder.onFinish.forEach(function(finish) {
        finish();
      });
    };
  }
  recorder.replaying = replaying;
  return recorder;
})();
''' % (window_location, window_location.split('?')[-1], on_idle or 'null', dirs_to_drop) + open(os.path.join(in_dir, first_js)).read() + '''
if (typeof nagivator == 'undefined') {
  window.runEventLoop();
}
''')

print 'done!'

