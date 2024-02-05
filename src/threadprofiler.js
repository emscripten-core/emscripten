#preprocess

/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var emscriptenThreadProfiler = {
  // UI update interval in milliseconds.
  uiUpdateIntervalMsecs: 1000,

  // UI div element.
  threadProfilerDiv: null,

  // Installs startup hook and periodic UI update timer.
  initialize() {
    this.threadProfilerDiv = document.getElementById('threadprofiler');
    if (!this.threadProfilerDiv) {
      var div = document.createElement("div");
      div.innerHTML = "<div id='threadprofiler' style='margin: 20px; border: solid 1px black;'></div>";
      document.body.appendChild(div);
      this.threadProfilerDiv = document.getElementById('threadprofiler');
    }
    var i = setInterval(function() { emscriptenThreadProfiler.updateUi() }, this.uiUpdateIntervalMsecs);
    addOnExit(() => clearInterval(i));
  },

  initializeNode() {
    addOnInit(() => {
      emscriptenThreadProfiler.dumpState();
      var i = setInterval(function() { emscriptenThreadProfiler.dumpState() }, this.uiUpdateIntervalMsecs);
      addOnExit(() => clearInterval(i));
    });
  },

  dumpState() {
    var mainThread = _emscripten_main_runtime_thread_id();

    var threads = [mainThread];
    for (var i in PThread.pthreads) {
      threads.push(PThread.pthreads[i].pthread_ptr);
    }
    for (var i = 0; i < threads.length; ++i) {
      var threadPtr = threads[i];
      var threadName = PThread.getThreadName(threadPtr);
      if (threadName) {
        threadName = `"${threadName}" (${ptrToString(threadPtr)})`;
      } else {
        threadName = `(${ptrToString(threadPtr)}})`;
      }

      console.log(`Thread ${threadName} now: ${PThread.threadStatusAsString(threadPtr)}. `);
    }
  },

  updateUi() {
    if (typeof PThread == 'undefined') {
      // Likely running threadprofiler on a singlethreaded build, or not
      // initialized yet, ignore updating.
      return;
    }
    var str = '';
    var mainThread = _emscripten_main_runtime_thread_id();

    var threads = [mainThread];
    for (var i in PThread.pthreads) {
      threads.push(PThread.pthreads[i].pthread_ptr);
    }

    for (var i = 0; i < threads.length; ++i) {
      var threadPtr = threads[i];
      var profilerBlock = Atomics.load({{{ getHeapForType('*') }}}, {{{ getHeapOffset('threadPtr + ' + C_STRUCTS.pthread.profilerBlock, '*') }}});
#if MEMORY64
      profilerBlock = Number(profilerBlock);
#endif
      var threadName = PThread.getThreadName(threadPtr);
      if (threadName) {
        threadName = `"${threadName}" (${ptrToString(threadPtr)})`;
      } else {
        threadName = `(${ptrToString(threadPtr)})`;
      }

      str += `Thread ${threadName} now: ${PThread.threadStatusAsString(threadPtr)}. `;

      var threadTimesInStatus = [];
      var totalTime = 0;
      var offset = profilerBlock + {{{ C_STRUCTS.thread_profiler_block.timeSpentInStatus }}};
      for (var j = 0; j < {{{ cDefs.EM_THREAD_STATUS_NUMFIELDS }}}; ++j, offset += 8) {
        threadTimesInStatus.push({{{ makeGetValue('offset', 0, 'double') }}});
        totalTime += threadTimesInStatus[j];
        {{{ makeSetValue('offset', 0, 0, 'double') }}};
      }
      var recent = '';
      if (threadTimesInStatus[1] > 0) recent += (threadTimesInStatus[1] / totalTime * 100.0).toFixed(1) + '% running. ';
      if (threadTimesInStatus[2] > 0) recent += (threadTimesInStatus[2] / totalTime * 100.0).toFixed(1) + '% sleeping. ';
      if (threadTimesInStatus[3] > 0) recent += (threadTimesInStatus[3] / totalTime * 100.0).toFixed(1) + '% waiting for futex. ';
      if (threadTimesInStatus[4] > 0) recent += (threadTimesInStatus[4] / totalTime * 100.0).toFixed(1) + '% waiting for mutex. ';
      if (threadTimesInStatus[5] > 0) recent += (threadTimesInStatus[5] / totalTime * 100.0).toFixed(1) + '% waiting for proxied ops. ';
      if (recent.length > 0) str += `Recent activity: ${recent}`;
      str += '<br />';
    }
    this.threadProfilerDiv.innerHTML = str;
  }
};

if (typeof document != 'undefined') {
  emscriptenThreadProfiler.initialize();
} else if (!ENVIRONMENT_IS_PTHREAD && typeof process != 'undefined') {
  emscriptenThreadProfiler.initializeNode();
}
