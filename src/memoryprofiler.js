var emscriptenMemoryProfiler = {
  // If true, walks all allocated pointers at graphing time to print a detailed memory fragmentation map. If false, used
  // memory is only graphed in one block (at the bottom of DYNAMIC memory space). Set this to false to improve performance at the expense of
  // accuracy.
  detailedHeapUsage: true,

  // Allocations of memory blocks larger than this threshold will get their detailed callstack captured and logged at runtime.
  // Warning: This can be extremely slow. Set to a very very large value like 1024*1024*1024*4 to disable.
  // Disabled if stack information is not available in this browser
  trackedCallstackMinSizeBytes: (typeof new Error().stack === 'undefined') ? 1024*1024*1024*4 : 16*1024*1024, 

  // Controls whether all outstanding allocation are printed to html page by callstack.
  allocateStatistics: false,

  // If allocateStatistics = true, then all callstacks that have recorded more than the following number of allocations will be printed to html page.
  allocateStatisticsNumcallsMinReported: 100,

  // If true, we hook into Runtime.stackAlloc to be able to catch better estimate of the maximum used STACK space.
  // You might only ever want to set this to false for performance reasons. Since stack allocations may occur often, this might impact performance.
  hookStackAlloc: true,

  // How often the log page is refreshed.
  uiUpdateIntervalMsecs: 2000,

  // Tracks data for the allocation statistics.
  allocationSiteStatistics: {},
  allocationSitePtrs: {},

  // Stores an associative array of records HEAP ptr -> size so that we can retrieve how much memory was freed in calls to 
  // _free() and decrement the tracked usage accordingly.
  // E.g. allocatedPtrSizes[address] returns the size of the heap pointer starting at 'address'.
  allocatedPtrSizes: {},

  // Conceptually same as the above array, except this one tracks only pointers that were allocated during the application preRun step, which
  // corresponds to the data added to the VFS with --preload-file.
  preRunMallocs: {},

  // Once set to true, preRun is finished and the above array is not touched anymore.
  pagePreRunIsFinished: false,

  // Stores an associative array of records HEAP ptr -> function string name so that we can identify each allocated pointer 
  // by the location in code the allocation occurred in.
  callstackOfAllocatedPtr: {},

  // Stores an associative array of accumulated amount of memory allocated per location.
  // E.g. allocatedBytesPerCallstack[callstack_function_name_string] returns the total number of allocated bytes performed from 'callstack_function_name_string'.
  allocatedBytesPerCallstack: {},

  // Grand total of memory currently allocated via malloc(). Decremented on free()s.
  totalMemoryAllocated: 0,

  // The running count of the number of times malloc() and free() have been called in the app. Used to keep track of # of currently alive pointers.
  // TODO: Perhaps in the future give a statistic of allocations per second to see how trashing memory usage is.
  totalTimesMallocCalled: 0,
  totalTimesFreeCalled: 0,

  // Tracks the highest seen location of the STACKTOP variable.
  stackTopWatermark: 0,

  // The canvas DOM element to which to draw the allocation map.
  canvas: null,

  // The 2D drawing context on the canvas.
  drawContext: null,

  // Converts number f to string with at most two decimals, without redundant trailing zeros.
  truncDec: function truncDec(f) {
    var str = f.toFixed(2);
    if (str.indexOf('.00', str.length-3) !== -1) return str.substr(0, str.length-3);
    else if (str.indexOf('0', str.length-1) !== -1) return str.substr(0, str.length-1);
    else return str;
  },

  // Converts a number of bytes pretty-formatted as a string.
  formatBytes: function formatBytes(bytes) {
    if (bytes >= 1000*1024*1024) return this.truncDec(bytes/(1024*1024*1024)) + ' GB';
    else if (bytes >= 1000*1024) return this.truncDec(bytes/(1024*1024)) + ' MB';
    else if (bytes >= 1000) return this.truncDec(bytes/1024) + ' KB';
    else return this.truncDec(bytes) + ' B';
  },

  onMalloc: function onMalloc(ptr, size) {
    // Gather global stats.
    this.totalMemoryAllocated += size;
    ++this.totalTimesMallocCalled;
    this.stackTopWatermark = Math.max(this.stackTopWatermark, STACKTOP);
    
    // Remember the size of the allocated block to know how much will be _free()d later.
    this.allocatedPtrSizes[ptr] = size;
    // Also track if this was a _malloc performed at preRun time.
    if (!this.pagePreRunIsFinished) this.preRunMallocs[ptr] = size;

    if (this.allocateStatistics) {
      var loc = new Error().stack.toString();
      var str = loc;
      if (!this.allocationSiteStatistics[str]) this.allocationSiteStatistics[str] = [0, 0];
      this.allocationSiteStatistics[str][0] += 1;
      this.allocationSiteStatistics[str][1] += size;
      this.allocationSitePtrs[ptr] = loc;
    }

    // If this is a large enough allocation, track its detailed callstack info.
    if (size > this.trackedCallstackMinSizeBytes) {
      // Get the caller function as string.
      var loc = new Error().stack.toString();
      var nl = loc.indexOf('\n')+1;
      loc = loc.substr(nl);
      loc = loc.replace(/\n/g, '<br />');
      var caller = loc;
      this.callstackOfAllocatedPtr[ptr] = caller;
      if (this.allocatedBytesPerCallstack[caller] > 0) this.allocatedBytesPerCallstack[caller] += size;
      else this.allocatedBytesPerCallstack[caller] = size;
    }
  },

  onFree: function onFree(ptr) {
    if (!ptr) return;

    // Decrement global stats.
    var sz = this.allocatedPtrSizes[ptr];
    if (!isNaN(sz)) {
      this.totalMemoryAllocated -= sz;
    }
    
    delete this.allocatedPtrSizes[ptr];
    delete this.preRunMallocs[ptr]; // Also free if this happened to be a _malloc performed at preRun time.
    this.stackTopWatermark = Math.max(this.stackTopWatermark, STACKTOP);

    if (this.allocateStatistics) {
      var loc = this.allocationSitePtrs[ptr];
      if (loc) {
        var str = loc;
        this.allocationSiteStatistics[str][0] -= 1;
        this.allocationSiteStatistics[str][1] -= sz;
      }
      this.allocationSitePtrs[ptr] = null;
    }

    // Decrement per-alloc stats if this was a large allocation.
    if (sz > this.trackedCallstackMinSizeBytes) {
      var caller = this.callstackOfAllocatedPtr[ptr];
      delete this.callstackOfAllocatedPtr[ptr];
      this.allocatedBytesPerCallstack[caller] -= sz;
      if (this.allocatedBytesPerCallstack[caller] <= 0) delete this.allocatedBytesPerCallstack[caller];
    }
    ++this.totalTimesFreeCalled;
  },

  onRealloc: function onRealloc(oldAddress, newAddress, size) {
    this.onFree(oldAddress);
    this.onMalloc(newAddress, size);
  },

  onPreloadComplete: function onPreloadComplete() {
    this.pagePreRunIsFinished = true;
  },

  // Installs startup hook and periodic UI update timer.
  initialize: function initialize() {
    // Inject the memoryprofiler hooks.
    Module['onMalloc'] = function onMalloc(ptr, size) { emscriptenMemoryProfiler.onMalloc(ptr, size); };
    Module['onRealloc'] = function onRealloc(oldAddress, newAddress, size) { emscriptenMemoryProfiler.onRealloc(oldAddress, newAddress, size); };
    Module['onFree'] = function onFree(ptr) { emscriptenMemoryProfiler.onFree(ptr); };

    // Add a tracking mechanism to detect when VFS loading is complete.
    Module['preRun'].push(function() { emscriptenMemoryProfiler.onPreloadComplete(); });

    if (this.hookStackAlloc) {
      // Inject stack allocator.
      var prevStackAlloc = Runtime.stackAlloc;
      function hookedStackAlloc(size) {
        emscriptenMemoryProfiler.stackTopWatermark = Math.max(emscriptenMemoryProfiler.stackTopWatermark, STACKTOP + size);
        return prevStackAlloc(size);
      }
      Runtime.stackAlloc = hookedStackAlloc;
    }

    memoryprofiler = document.getElementById('memoryprofiler');
    if (!memoryprofiler) {
      var div = document.createElement("div");
      div.innerHTML = "<div style='border: 2px solid black; padding: 2px;'><canvas style='border: 1px solid black; margin-left: auto; margin-right: auto; display: block;' id='memoryprofiler_canvas' width='100%' height='50'></canvas>trackedCallstackMinSizeBytes=<input id='memoryprofiler_min_tracked_alloc_size' type=number value="+this.trackedCallstackMinSizeBytes+"></input><br/><input id='memoryprofiler_enable_allocation_stats' type='checkbox'>Print allocation statistics by callstack to html log (warning: slow!)</input><input id='memoryprofiler_clear_alloc_stats' type='button' value='Clear alloc stats' ></input><div id='memoryprofiler'></div>";
      document.body.appendChild(div);
      memoryprofiler = document.getElementById('memoryprofiler');

      var self = this;

      document.getElementById('memoryprofiler_min_tracked_alloc_size').addEventListener("change", function(e){self.trackedCallstackMinSizeBytes=this.value;});
      document.getElementById('memoryprofiler_clear_alloc_stats').addEventListener("click", function(e){self.allocationSiteStatistics = {}; self.allocationSitePtrs = {};});
      document.getElementById('memoryprofiler_enable_allocation_stats').addEventListener("change", function(e){self.allocateStatistics=this.checked;});
    }
  
    this.canvas = document.getElementById('memoryprofiler_canvas');
    this.canvas.width = document.documentElement.clientWidth - 32;
    this.drawContext = this.canvas.getContext('2d');

    this.updateUi();
    setInterval(function() { emscriptenMemoryProfiler.updateUi() }, this.uiUpdateIntervalMsecs);
  },

  // Given a pointer 'bytes', compute the linear 1D position on the graph as pixels, rounding down for start address of a block.
  bytesToPixelsRoundedDown: function bytesToPixelsRoundedDown(bytes) {
    return (bytes * this.canvas.width * this.canvas.height / TOTAL_MEMORY) | 0;
  },

  // Same as bytesToPixelsRoundedDown, but rounds up for the end address of a block. The different rounding will
  // guarantee that even 'thin' allocations should get at least one pixel dot in the graph.
  bytesToPixelsRoundedUp: function bytesToPixelsRoundedUp(bytes) {
    return ((bytes * this.canvas.width * this.canvas.height + TOTAL_MEMORY - 1) / TOTAL_MEMORY) | 0;
  },

  // Graphs a range of allocated memory. The memory range will be drawn as a top-to-bottom, left-to-right stripes or columns of pixels.
  fillLine: function fillLine(startBytes, endBytes) {
    var startPixels = this.bytesToPixelsRoundedDown(startBytes);
    var endPixels = this.bytesToPixelsRoundedUp(endBytes);

    // Starting pos (top-left corner) of this allocation on the graph.
    var x0 = (startPixels / this.canvas.height) | 0;
    var y0 = startPixels - x0 * this.canvas.height;
    // Ending pos (bottom-right corner) of this allocation on the graph.
    var x1 = (endPixels / this.canvas.height) | 0;
    var y1 = endPixels - x1 * this.canvas.height;

    // Draw the left side partial column of the allocation block.
    if (y0 > 0 && x0 < x1) {
      this.drawContext.fillRect(x0, y0, 1, this.canvas.height - y0);
      // Proceed to the start of the next full column.
      y0 = 0;
      ++x0;
    }
    // Draw the right side partial column.
    if (y1 < this.canvas.height && x0 < x1) {
      this.drawContext.fillRect(x1, 0, 1, y1);
      // Decrement to the previous full column.
      y1 = this.canvas.height - 1;
      --x1;
    }
    // After filling the previous leftovers with one-pixel-wide lines, we are only left with a rectangular shape of full columns to blit.
    this.drawContext.fillRect(x0, 0, x1 - x0 + 1, this.canvas.height);
  },

  countOpenALAudioDataSize: function countOpenALAudioDataSize() {
    if (typeof AL == "undefined" || !AL.currentContext) return 0;

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
  printAllocsWithCyclingColors: function printAllocsWithCyclingColors(colors, allocs) {
    var colorIndex = 0;
    for (var i in allocs) {
      this.drawContext.fillStyle = colors[colorIndex];
      colorIndex = (colorIndex + 1) % colors.length;
      var start = i|0;
      var sz = allocs[start]|0;
      this.fillLine(start, start + sz);
    }
  },

  // Main UI update entry point.
  updateUi: function updateUi() {
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

    // Poll whether user as changed the browser window, and if so, resize the profiler window and redraw it.
    if (this.canvas.width != document.documentElement.clientWidth - 32) {
      this.canvas.width = document.documentElement.clientWidth - 32;
    }

    var width = (nBits(TOTAL_MEMORY) + 3) / 4; // Pointer 'word width'
    var html = 'Total HEAP size: ' + this.formatBytes(TOTAL_MEMORY) + '.';
    html += '<br />' + colorBar('#202020') + 'STATIC memory area size: ' + this.formatBytes(STATICTOP - STATIC_BASE);
    html += '. STATIC_BASE: ' + toHex(STATIC_BASE, width);
    html += '. STATICTOP: ' + toHex(STATICTOP, width) + '.';

    html += '<br />' + colorBar('#FF8080') + 'STACK memory area size: ' + this.formatBytes(STACK_MAX - STACK_BASE);
    html += '. STACK_BASE: ' + toHex(STACK_BASE, width);
    html += '. STACKTOP: ' + toHex(STACKTOP, width);
    html += '. STACK_MAX: ' + toHex(STACK_MAX, width) + '.';
    html += '<br />STACK memory area used now (should be zero): ' + this.formatBytes(STACKTOP - STACK_BASE) + '.' + colorBar('#FFFF00') + ' STACK watermark highest seen usage (approximate lower-bound!): ' + this.formatBytes(this.stackTopWatermark - STACK_BASE);

    var DYNAMICTOP = HEAP32[DYNAMICTOP_PTR>>2];
    html += '<br />' + colorBar('#70FF70') + 'DYNAMIC memory area size: ' + this.formatBytes(DYNAMICTOP-DYNAMIC_BASE);
    html += '. DYNAMIC_BASE: ' + toHex(DYNAMIC_BASE, width);
    html += '. DYNAMICTOP: ' + toHex(DYNAMICTOP, width) + '.';
    html += '<br />' + colorBar('#6699CC') + colorBar('#003366') + colorBar('#0000FF') + 'DYNAMIC memory area used: ' + this.formatBytes(this.totalMemoryAllocated) + ' (' + (this.totalMemoryAllocated * 100.0 / (TOTAL_MEMORY - DYNAMIC_BASE)).toFixed(2) + '% of all free memory)';

    var preloadedMemoryUsed = 0;
    for (i in this.preRunMallocs) preloadedMemoryUsed += this.preRunMallocs[i]|0;
    html += '<br />' + colorBar('#FF9900') + colorBar('#FFDD33') + 'Preloaded memory used, most likely memory reserved by files in the virtual filesystem : ' + this.formatBytes(preloadedMemoryUsed);

    html += '<br />OpenAL audio data: ' + this.formatBytes(this.countOpenALAudioDataSize()) + ' (outside HEAP)';
    html += '<br />' + colorBar('#FFFFFF') + 'Unallocated HEAP space: ' + this.formatBytes(TOTAL_MEMORY - DYNAMICTOP);
    html += '<br /># of total malloc()s/free()s performed in app lifetime: ' + this.totalTimesMallocCalled + '/' + this.totalTimesFreeCalled + ' (delta: ' + (this.totalTimesMallocCalled-this.totalTimesFreeCalled) + ')';

    // Background clear
    this.drawContext.fillStyle = "#FFFFFF";
    this.drawContext.fillRect(0, 0, this.canvas.width, this.canvas.height);

    this.drawContext.fillStyle = "#202020";
    this.fillLine(STATIC_BASE, STATICTOP);

    this.drawContext.fillStyle = "#FF8080";
    this.fillLine(STACK_BASE, STACK_MAX);

    this.drawContext.fillStyle = "#FFFF00";
    this.fillLine(STACK_BASE, this.stackTopWatermark);

    this.drawContext.fillStyle = "#FF0000";
    this.fillLine(STACK_BASE, STACKTOP);

    this.drawContext.fillStyle = "#70FF70";
    this.fillLine(DYNAMIC_BASE, DYNAMICTOP);

    if (this.detailedHeapUsage) {
      this.printAllocsWithCyclingColors(["#6699CC", "#003366", "#0000FF"], this.allocatedPtrSizes);
      this.printAllocsWithCyclingColors(["#FF9900", "#FFDD33"], this.preRunMallocs);
    } else {
      // Print only a single naive blob of individual allocations. This will not be accurate, but is constant-time.
      this.drawContext.fillStyle = "#0000FF";
      this.fillLine(DYNAMIC_BASE, DYNAMIC_BASE + this.totalMemoryAllocated);
    }

    function isEmpty(cont) {
      for (i in cont) return false;
      return true;
    }

    // Print out statistics of individual allocations if they were tracked.
     if (!isEmpty(this.allocatedBytesPerCallstack)) {
      html += '<h4>Notable allocation sites<h4>'
      for (var i in this.allocatedBytesPerCallstack) {
        html += '<b>'+this.formatBytes(this.allocatedBytesPerCallstack[i]|0)+'</b>: ' + i + '<br />';
      }
    }

    if (!isEmpty(this.allocationSiteStatistics)) {
      var calls = [];
      for (var i in this.allocationSiteStatistics) {
        var numcalls = this.allocationSiteStatistics[i][0];
        var size = this.allocationSiteStatistics[i][1];
        if (numcalls >= this.allocateStatisticsNumcallsMinReported ||
            size >= this.trackedCallstackMinSizeBytes) calls.push(i);
      }

      var self = this;
      calls.sort(function(a,b) { return self.allocationSiteStatistics[b][0] - self.allocationSiteStatistics[a][0]; });
      html += '<h4>Allocated pointers by call stack:<h4>';
      var ndemangled = 10;
      for (var i in calls) {
        var callstack = calls[i];
        var numcalls = this.allocationSiteStatistics[callstack][0];
        var size = this.allocationSiteStatistics[callstack][1];
        if (ndemangled > 0) {
          callstack = demangleAll(callstack);
          callstack = callstack.split('\n').join('<br />');
          --ndemangled;
        }
        html += callstack + ': <b> calls ' + numcalls + ', size ' + this.formatBytes(size)  + '</b><br /><br />';
      }
    }
    memoryprofiler.innerHTML = html;
  }
};

// Backwards compatibility with previously compiled code. Don't call this anymore!
function memoryprofiler_add_hooks() { emscriptenMemoryProfiler.initialize(); }

if (typeof Module !== 'undefined' && typeof document !== 'undefined') emscriptenMemoryProfiler.initialize();
