#preprocess

/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

const emscriptenThreadProfiler = {
  // UI update interval in milliseconds.
  uiUpdateIntervalMsecs: 1000,

  // UI div element.
  threadProfilerDiv: null,

  // Installs startup hook and periodic UI update timer.
  initialize() {
    const self = emscriptenThreadProfiler;
    self.threadProfilerDiv = document.getElementById('threadprofiler');
    if (!self.threadProfilerDiv) {
      const div = document.createElement("div");
      div.innerHTML = "<div id='threadprofiler' style='margin: 20px; border: solid 1px black;'></div>";
      document.body.appendChild(div);
      self.threadProfilerDiv = document.getElementById('threadprofiler');
    }
    const i = setInterval(() => self.updateUi(), self.uiUpdateIntervalMsecs);
    addOnExit(() => clearInterval(i));
  },

  initializeNode() {
    addOnInit(() => {
      const self = emscriptenThreadProfiler;
      self.dumpState();
      const i = setInterval(() => self.dumpState(), self.uiUpdateIntervalMsecs);
      addOnExit(() => clearInterval(i));
    });
  },

  dumpState() {
    const mainThread = _emscripten_main_runtime_thread_id();

    const threads = [mainThread];
    for (const thread of Object.values(PThread.pthreads)) {
      threads.push(thread.pthread_ptr);
    }
    for (const threadPtr of threads) {
      let threadName = PThread.getThreadName(threadPtr);
      if (threadName) {
        threadName = `"${threadName}" (${ptrToString(threadPtr)})`;
      } else {
        threadName = `(${ptrToString(threadPtr)})`;
      }

      console.log(`Thread ${threadName} now: ${PThread.threadStatusAsString(threadPtr)}. `);
    }
  },

  updateUi() {
    if (typeof PThread === 'undefined') {
      // Likely running threadprofiler on a singlethreaded build, or not
      // initialized yet, ignore updating.
      return;
    }
    if (!runtimeInitialized) {
      return;
    }
    let str = '';
    const mainThread = _emscripten_main_runtime_thread_id();

    const threads = [mainThread];
    for (const thread of Object.values(PThread.pthreads)) {
      threads.push(thread.pthread_ptr);
    }

    for (const threadPtr of threads) {
      let profilerBlock = Atomics.load({{{ getHeapForType('*') }}}, {{{ getHeapOffset('threadPtr + ' + C_STRUCTS.pthread.profilerBlock, '*') }}});
#if MEMORY64
      profilerBlock = Number(profilerBlock);
#endif
      let threadName = PThread.getThreadName(threadPtr);
      if (threadName) {
        threadName = `"${threadName}" (${ptrToString(threadPtr)})`;
      } else {
        threadName = `(${ptrToString(threadPtr)})`;
      }

      str += `Thread ${threadName} now: ${PThread.threadStatusAsString(threadPtr)}. `;

      const threadTimesInStatus = [];
      let totalTime = 0;
      let offset = profilerBlock + {{{ C_STRUCTS.thread_profiler_block.timeSpentInStatus }}};
      for (let j = 0; j < {{{ cDefs.EM_THREAD_STATUS_NUMFIELDS }}}; ++j, offset += 8) {
        threadTimesInStatus.push({{{ makeGetValue('offset', 0, 'double') }}});
        totalTime += threadTimesInStatus[j];
        {{{ makeSetValue('offset', 0, 0, 'double') }}};
      }
      let recent = '';
      if (threadTimesInStatus[1] > 0) recent += (threadTimesInStatus[1] / totalTime * 100.0).toFixed(1) + '% running. ';
      if (threadTimesInStatus[2] > 0) recent += (threadTimesInStatus[2] / totalTime * 100.0).toFixed(1) + '% sleeping. ';
      if (threadTimesInStatus[3] > 0) recent += (threadTimesInStatus[3] / totalTime * 100.0).toFixed(1) + '% waiting for futex. ';
      if (threadTimesInStatus[4] > 0) recent += (threadTimesInStatus[4] / totalTime * 100.0).toFixed(1) + '% waiting for mutex. ';
      if (threadTimesInStatus[5] > 0) recent += (threadTimesInStatus[5] / totalTime * 100.0).toFixed(1) + '% waiting for proxied ops. ';
      if (recent.length > 0) str += `Recent activity: ${recent}`;
      str += '<br />';
    }
    emscriptenThreadProfiler.threadProfilerDiv.innerHTML = str;
  }
};

if (globalThis.document) {
  emscriptenThreadProfiler.initialize();
} else if (!ENVIRONMENT_IS_PTHREAD && globalThis.process) {
  emscriptenThreadProfiler.initializeNode();
}
