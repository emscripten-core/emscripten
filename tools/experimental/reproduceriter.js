// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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
    recorder.random = Math.random;
    Math.random = function() {
      var ret = recorder.random();
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
    var pnow = performance.now || performance.webkitNow || performance.mozNow || performance.oNow || performance.msNow;
    recorder.pnow = function() { return pnow.call(performance) };
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
    recorder.random = Math.random;
    Math.random = function() {
      if (recorder.randoms.length > 0) {
        return recorder.randoms.pop();
      } else {
        recorder.finish();
        throw 'consuming too many random values!';
      }
    };
    // Date.now, performance.now
    recorder.dnow = Date.now;
    Date.now = function() {
      if (recorder.dnows.length > 0) {
        return recorder.dnows.pop();
      } else {
        recorder.finish();
        throw 'consuming too many Date.now values!';
      }
    };
    var pnow = performance.now || performance.webkitNow || performance.mozNow || performance.oNow || performance.msNow;
    recorder.pnow = function() { return pnow.call(performance) };
    performance.now = function() {
      if (recorder.pnows.length > 0) {
        return recorder.pnows.pop();
      } else {
        recorder.finish();
        throw 'consuming too many performance.now values!';
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

