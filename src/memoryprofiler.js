/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if MEMORYPROFILER

var emscriptenMemoryProfiler = {
  // If true, walks all allocated pointers at graphing time to print a detailed
  // memory fragmentation map. If false, used memory is only graphed in one
  // block (at the bottom of DYNAMIC memory space). Set this to false to improve
  // performance at the expense of accuracy.
  detailedHeapUsage: true,

  // Allocations of memory blocks larger than this threshold will get their
  // detailed callstack captured and logged at runtime.
  trackedCallstackMinSizeBytes: (typeof new Error().stack == 'undefined') ? Infinity : 16*1024*1024,

  // Allocations from call sites having more than this many outstanding
  // allocated pointers will get their detailed callstack captured and logged at
  // runtime.
  trackedCallstackMinAllocCount: (typeof new Error().stack == 'undefined') ? Infinity : 10000,

  // If true, we hook into stackAlloc to be able to catch better estimate of the
  // maximum used STACK space.  You might only ever want to set this to false
  // for performance reasons. Since stack allocations may occur often, this
  // might impact performance.
  hookStackAlloc: true,

  // How often the log page is refreshed.
  uiUpdateIntervalMsecs: 2000,

  // Tracks data for the allocation statistics.
  allocationsAtLoc: {},
  allocationSitePtrs: {},

  // Stores an associative array of records HEAP ptr -> size so that we can
  // retrieve how much memory was freed in calls to _free() and decrement the
  // tracked usage accordingly.
  // E.g. sizeOfAllocatedPtr[address] returns the size of the heap pointer
  // starting at 'address'.
  sizeOfAllocatedPtr: {},

  // Conceptually same as the above array, except this one tracks only pointers
  // that were allocated during the application preRun step, which corresponds
  // to the data added to the VFS with --preload-file.
  sizeOfPreRunAllocatedPtr: {},

  resizeMemorySources: [],
    // stack: <string>,
    // begin: <int>,
    // end: <int>

  sbrkSources: [],
    // stack: <string>,
    // begin: <int>,
    // end: <int>

  // Once set to true, preRun is finished and the above array is not touched anymore.
  pagePreRunIsFinished: false,

  // Grand total of memory currently allocated via malloc(). Decremented on free()s.
  totalMemoryAllocated: 0,

  // The running count of the number of times malloc() and free() have been
  // called in the app. Used to keep track of # of currently alive pointers.
  // TODO: Perhaps in the future give a statistic of allocations per second to
  // see how trashing memory usage is.
  totalTimesMallocCalled: 0,
  totalTimesFreeCalled: 0,

  // Tracks the highest seen location of the stack pointer.
  stackTopWatermark: Infinity,

  // The canvas DOM element to which to draw the allocation map.
  canvas: null,

  // The 2D drawing context on the canvas.
  drawContext: null,

  // Converts number f to string with at most two decimals, without redundant trailing zeros.
  truncDec(f = 0) {
    var str = f.toFixed(2);
    if (str.includes('.00', str.length-3)) return str.substr(0, str.length-3);
    else if (str.includes('0', str.length-1)) return str.substr(0, str.length-1);
    else return str;
  },

  // Converts a number of bytes pretty-formatted as a string.
  formatBytes(bytes) {
    if (bytes >= 1000*1024*1024) return emscriptenMemoryProfiler.truncDec(bytes/(1024*1024*1024)) + ' GB';
    else if (bytes >= 1000*1024) return emscriptenMemoryProfiler.truncDec(bytes/(1024*1024)) + ' MB';
    else if (bytes >= 1000) return emscriptenMemoryProfiler.truncDec(bytes/1024) + ' KB';
    else return emscriptenMemoryProfiler.truncDec(bytes) + ' B';
  },

  // HSV values in [0..1[, returns a RGB string in format '#rrggbb'
  hsvToRgb(h, s, v) {
    var h_i = (h*6)|0;
    var f = h*6 - h_i;
    var p = v * (1 - s);
    var q = v * (1 - f*s);
    var t = v * (1 - (1 - f) * s);
    var r, g, b;
    switch (h_i) {
      case 0: r = v; g = t; b = p; break;
      case 1: r = q; g = v; b = p; break;
      case 2: r = p; g = v; b = t; break;
      case 3: r = p; g = q; b = v; break;
      case 4: r = t; g = p; b = v; break;
      case 5: r = v; g = p; b = q; break;
    }
    function toHex(v) {
      v = (v*255|0).toString(16);
      return (v.length == 1) ? '0' + v : v;
    }
    return '#' + toHex(r) + toHex(g) + toHex(b);
  },

  onSbrkGrow(oldLimit, newLimit) {
    var self = emscriptenMemoryProfiler;
    // On first sbrk(), account for the initial size.
    if (self.sbrkSources.length == 0) {
      self.sbrkSources.push({
        stack: "initial heap sbrk limit<br>",
        begin: 0,
        end: oldLimit,
        color: self.hsvToRgb(self.sbrkSources.length * 0.618033988749895 % 1, 0.5, 0.95)
      });
    }
    if (newLimit <= oldLimit) return;
    self.sbrkSources.push({
      stack: self.filterCallstackForHeapResize(new Error().stack.toString()),
      begin: oldLimit,
      end: newLimit,
      color: self.hsvToRgb(self.sbrkSources.length * 0.618033988749895 % 1, 0.5, 0.95)
    });
  },

  onMemoryResize(oldSize, newSize) {
    var self = emscriptenMemoryProfiler;
    // On first heap resize, account for the initial size.
    if (self.resizeMemorySources.length == 0) {
      self.resizeMemorySources.push({
        stack: "initial heap size<br>",
        begin: 0,
        end: oldSize,
        color: self.resizeMemorySources.length % 2 ? '#ff00ff' : '#ff80ff'
      });
    }
    if (newSize <= oldSize) return;
    self.resizeMemorySources.push({
      stack: self.filterCallstackForHeapResize(new Error().stack.toString()),
      begin: oldSize,
      end: newSize,
      color: self.resizeMemorySources.length % 2 ? '#ff00ff' : '#ff80ff'
    });
    console.log('memory resize: ' + oldSize + ' ' + newSize);
  },

  recordStackWatermark() {
    if (typeof runtimeInitialized == 'undefined' || runtimeInitialized) {
      var self = emscriptenMemoryProfiler;
      self.stackTopWatermark = Math.min(self.stackTopWatermark, _emscripten_stack_get_current());
    }
  },

  onMalloc(ptr, size) {
    if (!ptr) return;
    if (emscriptenMemoryProfiler.sizeOfAllocatedPtr[ptr])
    {
// Uncomment to debug internal workings of tracing:
//      console.error('Allocation error in onMalloc! Pointer ' + ptr + ' had already been tracked as allocated!');
//      console.error('Previous site of allocation: ' + emscriptenMemoryProfiler.allocationSitePtrs[ptr]);
//      console.error('This doubly attempted site of allocation: ' + new Error().stack.toString());
//      throw 'malloc internal inconsistency!';
      return;
    }
    var self = emscriptenMemoryProfiler;
    // Gather global stats.
    self.totalMemoryAllocated += size;
    ++self.totalTimesMallocCalled;

    self.recordStackWatermark();

    // Remember the size of the allocated block to know how much will be _free()d later.
    self.sizeOfAllocatedPtr[ptr] = size;
    // Also track if this was a _malloc performed at preRun time.
    if (!self.pagePreRunIsFinished) self.sizeOfPreRunAllocatedPtr[ptr] = size;

    var loc = new Error().stack.toString();
    self.allocationsAtLoc[loc] ||= [0, 0, self.filterCallstackForMalloc(loc)];
    self.allocationsAtLoc[loc][0] += 1;
    self.allocationsAtLoc[loc][1] += size;
    self.allocationSitePtrs[ptr] = loc;
  },

  onFree(ptr) {
    if (!ptr) return;

    var self = emscriptenMemoryProfiler;

    // Decrement global stats.
    var sz = self.sizeOfAllocatedPtr[ptr];
    if (!isNaN(sz)) self.totalMemoryAllocated -= sz;
    else
    {
// Uncomment to debug internal workings of tracing:
//      console.error('Detected double free of pointer ' + ptr + ' at location:\n'+ new Error().stack.toString());
//      throw 'double free!';
      return;
    }

    self.recordStackWatermark();

    var loc = self.allocationSitePtrs[ptr];
    if (loc) {
      var allocsAtThisLoc = self.allocationsAtLoc[loc];
      if (allocsAtThisLoc) {
        allocsAtThisLoc[0] -= 1;
        allocsAtThisLoc[1] -= sz;
        if (allocsAtThisLoc[0] <= 0) delete self.allocationsAtLoc[loc];
      }
    }
    delete self.allocationSitePtrs[ptr];
    delete self.sizeOfAllocatedPtr[ptr];
    delete self.sizeOfPreRunAllocatedPtr[ptr]; // Also free if this happened to be a _malloc performed at preRun time.
    ++self.totalTimesFreeCalled;
  },

  onRealloc(oldAddress, newAddress, size) {
    emscriptenMemoryProfiler.onFree(oldAddress);
    emscriptenMemoryProfiler.onMalloc(newAddress, size);
  },

  onPreloadComplete() {
    emscriptenMemoryProfiler.pagePreRunIsFinished = true;
  },

  // Installs startup hook and periodic UI update timer.
  initialize() {
    // Inject the memoryprofiler hooks.
    Module['onMalloc'] = (ptr, size) => emscriptenMemoryProfiler.onMalloc(ptr, size);
    Module['onRealloc'] = (oldAddress, newAddress, size) => emscriptenMemoryProfiler.onRealloc(oldAddress, newAddress, size);;
    Module['onFree'] = (ptr) => emscriptenMemoryProfiler.onFree(ptr);
    emscriptenMemoryProfiler.recordStackWatermark();

    // Add a tracking mechanism to detect when VFS loading is complete.
    Module['preRun'] ||= [];
    Module['preRun'].push(emscriptenMemoryProfiler.onPreloadComplete);

    if (emscriptenMemoryProfiler.hookStackAlloc && typeof stackAlloc == 'function') {
      // Inject stack allocator.
      var prevStackAlloc = stackAlloc;
      var hookedStackAlloc = (size) => {
        var ptr = prevStackAlloc(size);
        emscriptenMemoryProfiler.recordStackWatermark();
        return ptr;
      };
      stackAlloc = hookedStackAlloc;
    }

    if (location.search.toLowerCase().includes('trackbytes=')) {
      emscriptenMemoryProfiler.trackedCallstackMinSizeBytes = parseInt(location.search.substr(location.search.toLowerCase().indexOf('trackbytes=') + 'trackbytes='.length), undefined /* https://github.com/google/closure-compiler/issues/3230 / https://github.com/google/closure-compiler/issues/3548 */);
    }
    if (location.search.toLowerCase().includes('trackcount=')) {
      emscriptenMemoryProfiler.trackedCallstackMinAllocCount = parseInt(location.search.substr(location.search.toLowerCase().indexOf('trackcount=') + 'trackcount='.length), undefined);
    }

    emscriptenMemoryProfiler.memoryprofiler_summary = document.getElementById('memoryprofiler_summary');
    var div;
    if (!emscriptenMemoryProfiler.memoryprofiler_summary) {
      div = document.createElement("div");
      div.innerHTML = "<div style='border: 2px solid black; padding: 2px;'><canvas style='border: 1px solid black; margin-left: auto; margin-right: auto; display: block;' id='memoryprofiler_canvas' width='100%' height='50'></canvas><input type='checkbox' id='showHeapResizes' onclick='emscriptenMemoryProfiler.updateUi()'>Display heap and sbrk() resizes. Filter sbrk() and heap resize callstacks by keywords: <input type='text' id='sbrkFilter'>(reopen page with ?sbrkFilter=foo,bar query params to prepopulate this list)<br/>Track all allocation sites larger than <input id='memoryprofiler_min_tracked_alloc_size' type=number value="+emscriptenMemoryProfiler.trackedCallstackMinSizeBytes+"></input> bytes, and all allocation sites with more than <input id='memoryprofiler_min_tracked_alloc_count' type=number value="+emscriptenMemoryProfiler.trackedCallstackMinAllocCount+"></input> outstanding allocations. (visit this page via URL query params foo.html?trackbytes=1000&trackcount=100 to apply custom thresholds starting from page load)<br/><div id='memoryprofiler_summary'></div><input id='memoryprofiler_clear_alloc_stats' type='button' value='Clear alloc stats' ></input><br />Sort allocations by:<select id='memoryProfilerSort'><option value='bytes'>Bytes</option><option value='count'>Count</option><option value='fixed'>Fixed</option></select><div id='memoryprofiler_ptrs'></div>";
    }
    var populateHtmlBody = function() {
      if (div) {
        document.body.appendChild(div);

        function getValueOfParam(key) {
          var results = (new RegExp("[\\?&]"+key+"=([^&#]*)")).exec(location.href);
          return results ? results[1] : '';
        }
        // Allow specifying a precreated filter in page URL ?query parameters for convenience.
        if (document.getElementById('sbrkFilter').value = getValueOfParam('sbrkFilter')) {
          document.getElementById('showHeapResizes').checked = true;
        }
      }
      var self = emscriptenMemoryProfiler;
      self.memoryprofiler_summary = document.getElementById('memoryprofiler_summary');
      self.memoryprofiler_ptrs = document.getElementById('memoryprofiler_ptrs');

      document.getElementById('memoryprofiler_min_tracked_alloc_size').addEventListener("change", function(e) { self.trackedCallstackMinSizeBytes=parseInt(this.value, undefined /* https://github.com/google/closure-compiler/issues/3230 / https://github.com/google/closure-compiler/issues/3548 */); });
      document.getElementById('memoryprofiler_min_tracked_alloc_count').addEventListener("change", function(e) { self.trackedCallstackMinAllocCount=parseInt(this.value, undefined); });
      document.getElementById('memoryprofiler_clear_alloc_stats').addEventListener("click", (e) => {self.allocationsAtLoc = {}; self.allocationSitePtrs = {};});
      self.canvas = document.getElementById('memoryprofiler_canvas');
      self.canvas.width = document.documentElement.clientWidth - 32;
      self.drawContext = self.canvas.getContext('2d');

      self.updateUi();
      setInterval(() => emscriptenMemoryProfiler.updateUi(), self.uiUpdateIntervalMsecs);

    };
    // User might initialize memoryprofiler in the <head> of a page, when
    // document.body does not yet exist. In that case, delay initialization
    // of the memoryprofiler UI until page has loaded
    if (document.body) populateHtmlBody();
    else setTimeout(populateHtmlBody, 1000);
  },

  // Given a pointer 'bytes', compute the linear 1D position on the graph as
  // pixels, rounding down for start address of a block.
  bytesToPixelsRoundedDown(bytes) {
    return (bytes * emscriptenMemoryProfiler.canvas.width * emscriptenMemoryProfiler.canvas.height / HEAP8.length) | 0;
  },

  // Same as bytesToPixelsRoundedDown, but rounds up for the end address of a
  // block. The different rounding will guarantee that even 'thin' allocations
  // should get at least one pixel dot in the graph.
  bytesToPixelsRoundedUp(bytes) {
    return ((bytes * emscriptenMemoryProfiler.canvas.width * emscriptenMemoryProfiler.canvas.height + HEAP8.length - 1) / HEAP8.length) | 0;
  },

  // Graphs a range of allocated memory. The memory range will be drawn as a
  // top-to-bottom, left-to-right stripes or columns of pixels.
  fillLine(startBytes, endBytes) {
    var self = emscriptenMemoryProfiler;
    var startPixels = self.bytesToPixelsRoundedDown(startBytes);
    var endPixels = self.bytesToPixelsRoundedUp(endBytes);

    // Starting pos (top-left corner) of this allocation on the graph.
    var x0 = (startPixels / self.canvas.height) | 0;
    var y0 = startPixels - x0 * self.canvas.height;
    // Ending pos (bottom-right corner) of this allocation on the graph.
    var x1 = (endPixels / self.canvas.height) | 0;
    var y1 = endPixels - x1 * self.canvas.height;

    // Draw the left side partial column of the allocation block.
    if (y0 > 0 && x0 < x1) {
      self.drawContext.fillRect(x0, y0, 1, self.canvas.height - y0);
      // Proceed to the start of the next full column.
      y0 = 0;
      ++x0;
    }
    // Draw the right side partial column.
    if (y1 < self.canvas.height && x0 < x1) {
      self.drawContext.fillRect(x1, 0, 1, y1);
      // Decrement to the previous full column.
      y1 = self.canvas.height - 1;
      --x1;
    }
    // After filling the previous leftovers with one-pixel-wide lines, we are
    // only left with a rectangular shape of full columns to blit.
    self.drawContext.fillRect(x0, 0, x1 - x0 + 1, self.canvas.height);
  },

  // Fills a rectangle of given height % that overlaps the byte range given.
  fillRect(startBytes, endBytes, heightPercentage) {
    var self = emscriptenMemoryProfiler;
    var startPixels = self.bytesToPixelsRoundedDown(startBytes);
    var endPixels = self.bytesToPixelsRoundedUp(endBytes);

    var x0 = (startPixels / self.canvas.height) | 0;
    var x1 = (endPixels / self.canvas.height) | 0;
    self.drawContext.fillRect(x0, self.canvas.height * (1.0 - heightPercentage), x1 - x0 + 1, self.canvas.height);
  },

  countOpenALAudioDataSize() {
    if (typeof AL == 'undefined' || !AL.currentContext) return 0;

    var totalMemory = 0;

    for (var i in AL.currentContext.buf) {
      var buffer = AL.currentContext.buf[i];
      for (var channel = 0; channel < buffer.numberOfChannels; ++channel) totalMemory += buffer.getChannelData(channel).length * 4;
    }
    return totalMemory;
  },

  // Print accurate map of individual allocations. This will show information about
  // memory fragmentation and allocation sizes.
  // Warning: This will walk through all allocations, so it is slow!
  printAllocsWithCyclingColors(colors, allocs) {
    var colorIndex = 0;
    for (var i in allocs) {
      emscriptenMemoryProfiler.drawContext.fillStyle = colors[colorIndex];
      colorIndex = (colorIndex + 1) % colors.length;
      var start = i|0;
      var sz = allocs[start]|0;
      emscriptenMemoryProfiler.fillLine(start, start + sz);
    }
  },

  filterURLsFromCallstack(callstack) {
    // Hide paths from URLs to make the log more readable
    callstack = callstack.replace(/@((file)|(http))[\w:\/\.]*\/([\w\.]*)/g, '@$4');
    callstack = callstack.replace(/\n/g, '<br />');
    return callstack;
  },

  // given callstack of func1\nfunc2\nfunc3... and function name, cuts the tail from the callstack
  // for anything after the function func.
  filterCallstackAfterFunctionName(callstack, func) {
    var i = callstack.indexOf(func);
    if (i != -1) {
      var end = callstack.indexOf('<br />', i);
      if (end != -1) {
        return callstack.substr(0, end);
      }
    }
    return callstack;
  },

  filterCallstackForMalloc(callstack) {
    // Do not show Memoryprofiler's own callstacks in the callstack prints.
    var i = callstack.indexOf('emscripten_trace_record_');
    if (i != -1) {
      callstack = callstack.substr(callstack.indexOf('\n', i)+1);
    }
    return emscriptenMemoryProfiler.filterURLsFromCallstack(callstack);
  },

  filterCallstackForHeapResize(callstack) {
    // Do not show Memoryprofiler's own callstacks in the callstack prints.
    var i = callstack.indexOf('emscripten_asm_const_iii');
    var j = callstack.indexOf('growMemory');
    i = (i == -1) ? j : (j == -1 ? i : Math.min(i, j));
    if (i != -1) {
      callstack = callstack.substr(callstack.indexOf('\n', i)+1);
    }
    callstack = callstack.replace(/(wasm-function\[\d+\]):0x[0-9a-f]+/g, "$1");
    return emscriptenMemoryProfiler.filterURLsFromCallstack(callstack);
  },

  printHeapResizeLog(heapResizes) {
    var html = '';
    for (var i = 0; i < heapResizes.length; ++i) {
      var j = i+1;
      while (j < heapResizes.length) {
        if ((heapResizes[j].filteredStack || heapResizes[j].stack) == (heapResizes[i].filteredStack || heapResizes[i].stack)) {
          ++j;
        } else {
          break;
        }
      }
      var resizeFirst = heapResizes[i];
      var resizeLast = heapResizes[j-1];
      var count = j - i;
      html += '<div style="background-color: ' + resizeFirst.color + '"><b>' + resizeFirst.begin + '-' + resizeLast.end + ' (' + count + ' times, ' + emscriptenMemoryProfiler.formatBytes(resizeLast.end-resizeFirst.begin) + ')</b>:' + (resizeFirst.filteredStack || resizeFirst.stack) + '</div><br>';
      i = j-1;
    }
    return html;
  },

  // Main UI update entry point.
  updateUi() {
    // It is common to set 'overflow: hidden;' on canvas pages that do WebGL. When MemoryProfiler is being used, there will be a long block of text on the page, so force-enable scrolling.
    if (document.body.style.overflow != '') document.body.style.overflow = '';
    function colorBar(color) {
      return '<span style="padding:0px; border:solid 1px black; width:28px;height:14px; vertical-align:middle; display:inline-block; background-color:'+color+';"></span>';
    }

    // Naive function to compute how many bits will be needed to represent the number 'n' in binary. This will be our pointer 'word width' in the UI.
    function nBits(n) {
      var i = 0;
      while (n >= 1) {
        ++i;
        n /= 2;
      }
      return i;
    }

    // Returns i formatted to string as fixed-width hexadecimal.
    function toHex(i, width) {
      var str = i.toString(16);
      while (str.length < width) str = '0' + str;
      return '0x'+str;
    }

    var self = emscriptenMemoryProfiler;

    // Poll whether user as changed the browser window, and if so, resize the profiler window and redraw it.
    if (self.canvas.width != document.documentElement.clientWidth - 32) {
      self.canvas.width = document.documentElement.clientWidth - 32;
    }

    if (typeof runtimeInitialized != 'undefined' && !runtimeInitialized) {
      return;
    }
    var stackBase = _emscripten_stack_get_base();
    var stackMax = _emscripten_stack_get_end();
    var stackCurrent = _emscripten_stack_get_current();
    var width = (nBits(HEAP8.length) + 3) / 4; // Pointer 'word width'
    var html = 'Total HEAP size: ' + self.formatBytes(HEAP8.length) + '.';
    html += '<br />' + colorBar('#202020') + 'STATIC memory area size: ' + self.formatBytes(stackMax - {{{ GLOBAL_BASE }}});
    html += '. {{{ GLOBAL_BASE }}}: ' + toHex({{{ GLOBAL_BASE }}}, width);

    html += '<br />' + colorBar('#FF8080') + 'STACK memory area size: ' + self.formatBytes(stackBase - stackMax);
    html += '. STACK_BASE: ' + toHex(stackBase, width);
    html += '. STACKTOP: ' + toHex(stackCurrent, width);
    html += '. STACK_MAX: ' + toHex(stackMax, width) + '.';
    html += '<br />STACK memory area used now (should be zero): ' + self.formatBytes(stackBase - stackCurrent) + '.' + colorBar('#FFFF00') + ' STACK watermark highest seen usage (approximate lower-bound!): ' + self.formatBytes(stackBase - self.stackTopWatermark);

    var heap_base = Module['___heap_base'];
    var heap_end = _sbrk();
    html += "<br />DYNAMIC memory area size: " + self.formatBytes(heap_end - heap_base);
    html += ". start: " + toHex(heap_base, width);
    html += ". end: " + toHex(heap_end, width) + ".";
    html += "<br />" + colorBar("#6699CC") + colorBar("#003366") + colorBar("#0000FF") + "DYNAMIC memory area used: " + self.formatBytes(self.totalMemoryAllocated) + " (" + (self.totalMemoryAllocated * 100 / (HEAP8.length - heap_base)).toFixed(2) + "% of all dynamic memory and unallocated heap)";
    html += "<br />Free memory: " + colorBar("#70FF70") + "DYNAMIC: " + self.formatBytes(heap_end - heap_base - self.totalMemoryAllocated) + ", " + colorBar('#FFFFFF') + 'Unallocated HEAP: ' + self.formatBytes(HEAP8.length - heap_end) + " (" + ((HEAP8.length - heap_base - self.totalMemoryAllocated) * 100 / (HEAP8.length - heap_base)).toFixed(2) + "% of all dynamic memory and unallocated heap)";

    var preloadedMemoryUsed = 0;
    for (var i in self.sizeOfPreRunAllocatedPtr) preloadedMemoryUsed += self.sizeOfPreRunAllocatedPtr[i]|0;
    html += '<br />' + colorBar('#FF9900') + colorBar('#FFDD33') + 'Preloaded memory used, most likely memory reserved by files in the virtual filesystem : ' + self.formatBytes(preloadedMemoryUsed);

    html += '<br />OpenAL audio data: ' + self.formatBytes(self.countOpenALAudioDataSize()) + ' (outside HEAP)';
    html += '<br /># of total malloc()s/free()s performed in app lifetime: ' + self.totalTimesMallocCalled + '/' + self.totalTimesFreeCalled + ' (currently alive pointers: ' + (self.totalTimesMallocCalled-self.totalTimesFreeCalled) + ')';

    // Background clear
    self.drawContext.fillStyle = "#FFFFFF";
    self.drawContext.fillRect(0, 0, self.canvas.width, self.canvas.height);

    self.drawContext.fillStyle = "#FF8080";
    self.fillLine(stackMax, stackBase);

    self.drawContext.fillStyle = "#FFFF00";
    self.fillLine(self.stackTopWatermark, stackBase);

    self.drawContext.fillStyle = "#FF0000";
    self.fillLine(stackCurrent, stackBase);

    self.drawContext.fillStyle = "#70FF70";
    self.fillLine(heap_base, heap_end);

    if (self.detailedHeapUsage) {
      self.printAllocsWithCyclingColors(["#6699CC", "#003366", "#0000FF"], self.sizeOfAllocatedPtr);
      self.printAllocsWithCyclingColors(["#FF9900", "#FFDD33"], self.sizeOfPreRunAllocatedPtr);
    } else {
      // Print only a single naive blob of individual allocations. This will not be accurate, but is constant-time.
      self.drawContext.fillStyle = "#0000FF";
      self.fillLine(heap_base, heap_base + self.totalMemoryAllocated);
    }

    if (document.getElementById('showHeapResizes').checked) {
      // Print heap resize traces.
      for (var i in self.resizeMemorySources) {
        var resize = self.resizeMemorySources[i];
        self.drawContext.fillStyle = resize.color;
        self.fillRect(resize.begin, resize.end, 0.5);
      }

      // Print sbrk() traces.
      var uniqueSources = {};
      var filterWords = document.getElementById('sbrkFilter').value.split(',');
      for (var i in self.sbrkSources) {
        var sbrk = self.sbrkSources[i];
        var stack = sbrk.stack;
        for (var j in filterWords) {
          var s = filterWords[j].trim();
          if (s.length > 0)
          stack = self.filterCallstackAfterFunctionName(stack, s);
        }
        sbrk.filteredStack = stack;
        uniqueSources[stack] ||= self.hsvToRgb(Object.keys(uniqueSources).length * 0.618033988749895 % 1, 0.5, 0.95);
        self.drawContext.fillStyle = sbrk.color = uniqueSources[stack];
        self.fillRect(sbrk.begin, sbrk.end, 0.25);
      }

      // Print a divider line to make the sbrk()/heap resize block more prominently visible compared to the rest of the allocations.
      function line(x0, y0, x1, y1) {
        self.drawContext.beginPath();
        self.drawContext.moveTo(x0, y0);
        self.drawContext.lineTo(x1, y1);
        self.drawContext.lineWidth = 2;
        self.drawContext.stroke();
      }
      if (self.sbrkSources.length > 0) line(0, 0.75*self.canvas.height, self.canvas.width, 0.75*self.canvas.height);
      if (self.resizeMemorySources.length > 0) line(0, 0.5*self.canvas.height, self.canvas.width, 0.5*self.canvas.height);
    }

    self.memoryprofiler_summary.innerHTML = html;

    var sort = document.getElementById('memoryProfilerSort');
    var sortOrder = sort.options[sort.selectedIndex].value;

    html = '';

    // Print out sbrk() and memory resize subdivisions:
    if (document.getElementById('showHeapResizes').checked) {
      // Print heap resize traces.
      html += '<div style="background-color: #c0c0c0"><h4>Heap resize locations:</h4>';
      html += self.printHeapResizeLog(self.resizeMemorySources);
      html += '</div>'

      // Print heap sbrk traces.
      html += '<div style="background-color: #c0c0ff"><h4>Memory sbrk() locations:</h4>';
      html += self.printHeapResizeLog(self.sbrkSources);
      html += '</div>'
    } else {
      // Print out statistics of individual allocations if they were tracked.
      if (Object.keys(self.allocationsAtLoc).length > 0) {
        var calls = [];
        for (var i in self.allocationsAtLoc) {
          if (self.allocationsAtLoc[i][0] >= self.trackedCallstackMinAllocCount || self.allocationsAtLoc[i][1] >= self.trackedCallstackMinSizeBytes) {
            calls.push(self.allocationsAtLoc[i]);
          }
        }
        if (calls.length > 0) {
          if (sortOrder != 'fixed') {
            var sortIdx = (sortOrder == 'count') ? 0 : 1;
            calls.sort((a,b) => b[sortIdx] - a[sortIdx]);
          }
          html += '<h4>Allocation sites with more than ' + self.formatBytes(self.trackedCallstackMinSizeBytes) + ' of accumulated allocations, or more than ' + self.trackedCallstackMinAllocCount + ' simultaneously outstanding allocations:</h4>'
          for (var i in calls) {
            html += "<b>" + self.formatBytes(calls[i][1]) + '/' + calls[i][0] + " allocs</b>: " + calls[i][2] + "<br />";
          }
        }
      }
    }
    self.memoryprofiler_ptrs.innerHTML = html;
  }
};

// Backwards compatibility with previously compiled code. Don't call this
// anymore!
function memoryprofiler_add_hooks() {
  emscriptenMemoryProfiler.initialize();
}

if (typeof document != 'undefined' && typeof window != 'undefined' && typeof process == 'undefined') {
  emscriptenMemoryProfiler.initialize();
}

// Declared in globalThis so that `onclick` handlers work when `-sMODULARIZE=1`
globalThis.emscriptenMemoryProfiler = emscriptenMemoryProfiler;

#endif
