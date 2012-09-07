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

    reproduceriter.py IN_DIR OUT_DIR FIRST_JS START_COMMAND

   IN_DIR should be the project directory, and OUT_DIR will be
   created with the instrumented code (OUT_DIR should not
   exist). FIRST_JS should be a path (relative to IN_DIR) to
   the first JavaScript file loaded by the project (this tool
   will add code to that). 

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

Notes:

 * When we start to replay events, the assumption is that
   there is nothing asynchronous that affects execution. So
   asynchronous loading of files should have already
   completed.

   TODO: start running recorded events with some trigger, for example the fullscreen button in BananaBread
'''

import os, sys, shutil

assert len(sys.argv) == 4, 'Usage: reproduceriter.py IN_DIR OUT_DIR'

# Process input args

in_dir = sys.argv[1]
out_dir = sys.argv[2]
first_js = sys.argv[3]

assert not os.path.exists(out_dir), 'OUT_DIR must not exist'
assert os.path.exists(os.path.join(in_dir, first_js))

# Copy project

shutil.copytree(in_dir, out_dir)

# Add our boilerplate

open(os.path.join(out_dir, first_js), 'w').write('''
var Recorder = (function() {
  var recorder;
  var init = '&reproduce=';
  var initLocation = window.location.search.indexOf(init);
  var replaying = initLocation >= 0
  if (replaying) {
    // load recording
    var dataPath = window.location.search.substring(initLocation);
    var baseURL = window.location.toString().replace('://', 'cheez999').split('?')[0].split('/')[0].replace('cheez999', '://');
    if (baseURL[baseURL.length-1] != '/') baseURL += '/';
    var request = new XMLHttpRequest();
    request.open('GET', baseURL + dataPath, false);
    request.send();
    recorder = JSON.parse(request.responseText);
    // prepare to replay
    // Math.random
    var warned = false;
    Math.random = function() {
      if (recorder.randoms.length > 0) {
        return recorder.randoms.shift();
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
        return recorder.dnows.shift();
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
        return recorder.pnows.shift();
      } else {
        if (!warned) {
          console.log('warning: consuming too many values!')
          warned = true;
        }
        return performance.now();
      }
    };
  } else {
    // prepare to record
    recorder = {};
    // Math.random
    recorder.randoms = [];
    var random = Math.random()
    Math.random = function() {
      var ret = random();
      recorder.randoms.push(ret);
      return ret;
    };
    // Date.now, performance.now
    recorder.dnows = [];
    var dnow = Date.now();
    Date.now = function() {
      var ret = dnow();
      recorder.dnows.push(ret);
      return ret;
    };
    recorder.pnows = [];
    var pnow = performance.now();
    performance.now = function() {
      var ret = pnow();
      recorder.pnows.push(ret);
      return ret;
    };
    // finish
    recorder.finish = function() {
      document.write(JSON.stringify(recorder));
      throw 'all done, remember to save!';
    };
  }
  return recorder;
})();
''' + open(os.path.join(in_dir, first_js)).read()
)

