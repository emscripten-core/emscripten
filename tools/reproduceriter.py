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

    reproduceriter.py IN_DIR OUT_DIR FIRST_JS

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

    emscripten/tools/reproduceriter.py bb bench js/game-setup.js
'''

import os, sys, shutil, re

assert len(sys.argv) == 4, 'Usage: reproduceriter.py IN_DIR OUT_DIR FIRST_JS'

# Process input args

in_dir = sys.argv[1]
out_dir = sys.argv[2]
first_js = sys.argv[3]

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
      js = re.sub('document\.on(\w+) = ([\w.]+);', lambda m: 'Recorder.onEvent("' + m.group(1) + '", ' + m.group(2) + ');', js)
      js = re.sub('''([\w'"\[\]]+)\.addEventListener\(([\w,. ]+)\);''', lambda m: 'Recorder.addListener(' + m.group(1) + ', ' + m.group(2) + ');', js)
      open(fullname, 'w').write(js)

# Add our boilerplate

print 'add boilerplate...'

open(os.path.join(out_dir, first_js), 'w').write('''
var Recorder = (function() {
  var recorder;
  var init = '&reproduce=';
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
    var dnow = Date.now;
    Date.now = function() {
      var ret = dnow();
      recorder.dnows.push(ret);
      return ret;
    };
    recorder.pnows = [];
    var pnow = performance.now;
    performance.now = function() {
      var ret = pnow();
      recorder.pnows.push(ret);
      return ret;
    };
    // Events
    recorder.devents = []; // document events
    recorder.onEvent = function(which, callback) {
      document['on' + which] = function(event) {
        event.frameCounter = recorder.frameCounter;
        event.which = which;
        recorder.devents.push(event);
        return callback(event); // XXX do we need to record the return value?
      };
    };
    recorder.tevents = []; // custom-target events. Currently we assume a single such custom target (aside from document), e.g., a canvas for the game.
    recorder.addListener = function(target, which, callback, arg) {
      target.addEventListener(which, function(event) {
        event.frameCounter = recorder.frameCounter;
        event.which = which;
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
      });
    };
  } else {
    // Load recording
    var dataPath = window.location.search.substring(initLocation);
    var baseURL = window.location.toString().replace('://', 'cheez999').split('?')[0].split('/')[0].replace('cheez999', '://');
    if (baseURL[baseURL.length-1] != '/') baseURL += '/';
    var request = new XMLHttpRequest();
    request.open('GET', baseURL + dataPath, false);
    request.send();
    recorder = JSON.parse(request.responseText);
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
          recorder['on' + event.which](event);
        }
        while (recorder.tevents.length && recorder.tevents[recorder.tevents.length-1].frameCounter <= recorder.frameCounter) {
          var event = recorder.tevents.pop();
          recorder['event' + event.which](event);
        }
      }
      count();
    };
    // Math.random
    var warned = false;
    Math.random = function() {
      if (recorder.randoms.length > 0) {
        return recorder.randoms.pop();
      } else {
        if (!warned) {
          console.log('warning: consuming too many values!')
          warned = true;
        }
        return Math.random();
      }
    };
    // Date.now, performance.now
    var warned = false;
    Date.now = function() {
      if (recorder.dnows.length > 0) {
        return recorder.dnows.pop();
      } else {
        if (!warned) {
          console.log('warning: consuming too many values!')
          warned = true;
        }
        return Date.now();
      }
    };
    var warned = false;
    performance.now = function() {
      if (recorder.pnows.length > 0) {
        return recorder.pnows.pop();
      } else {
        if (!warned) {
          console.log('warning: consuming too many values!')
          warned = true;
        }
        return performance.now();
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
  }
  return recorder;
})();
''' + open(os.path.join(in_dir, first_js)).read()
)

print 'done!'

