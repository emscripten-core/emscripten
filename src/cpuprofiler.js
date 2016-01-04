var emscriptenCpuProfiler = {
  // UI update interval in milliseconds.
  uiUpdateInterval: 1,

  // Specifies the pixel column where the previous UI update finished at. (current "draw cursor" position next draw will resume from)
  lastUiUpdateEndX: 0,

  // An array which stores samples of msec durations spent in the emscripten main loop (emscripten_set_main_loop).
  // Carries # samples equal to the pixel width of the profiler display window, and old samples are erased in a rolling window fashion.
  timeSpentInMainloop: [],

  // Similar to 'timeSpentInMainloop', except this stores msec durations outside the emscripten main loop callback. (either idle CPU time, browser processing, or something else)
  timeSpentOutsideMainloop: [],

  // Specifies the sample coordinate into the timeSpentIn/OutsideMainloop arrays that is currently being populated.
  currentHistogramX: 0,

  // Wallclock time denoting when the currently executing main loop callback tick began.
  currentFrameStartTime: 0,

  // Wallclock time denoting when the previously executing main loop was finished.
  previousFrameEndTime: 0,

  // The total time spent in a frame can be further subdivided down into 'sections'. This array stores info structures representing each section.
  sections: [],

  // The 2D canvas DOM element to which the CPU profiler graph is rendered to.
  canvas: null,

  // The 2D drawing context on the canvas.
  drawContext: null,

  // How many milliseconds in total to fit vertically into the displayed CPU profiler window? Frametimes longer than this are out the graph and not visible.
  verticalTimeScale: 40,

  // History of wallclock times of N most recent frame times. Used to estimate current FPS.
  fpsCounterTicks: [],

  // When was the FPS UI display last updated?
  fpsCounterLastPrint: performance.now(),

  // fpsCounter() is called once per frame to record an executed frame time, and to periodically update the FPS counter display.
  fpsCounter: function fpsCounter() {
    // Record the new frame time sample, and prune the history to 120 most recent frames.
    var now = performance.now();
    this.fpsCounterTicks.push(now);
    if (this.fpsCounterTicks.length > 120) this.fpsCounterTicks = this.fpsCounterTicks.slice(this.fpsCounterTicks.length - 120, this.fpsCounterTicks.length);
  
    if (now - this.fpsCounterLastPrint > 2000) {
      var fps = ((this.fpsCounterTicks.length - 1) * 1000.0 / (this.fpsCounterTicks[this.fpsCounterTicks.length - 1] - this.fpsCounterTicks[0]));
      var totalDt = 0;
      var totalRAFDt = 0;
      var minDt = 99999999;
      var maxDt = 0;
      var nSamples = 0;
      for (var i = 0; i < this.timeSpentInMainloop.length; ++i) {
        var dt = this.timeSpentInMainloop[i] + this.timeSpentOutsideMainloop[i];
        totalRAFDt += this.timeSpentInMainloop[i];
        if (dt > 0) ++nSamples;
        totalDt += dt;
        minDt = Math.min(minDt, dt);
        maxDt = Math.max(maxDt, dt);
      }
      var avgDt = totalDt / nSamples;
      var avgFps = 1000.0 / avgDt;
      var dtVariance = 0;
      for (var i = 1; i < this.timeSpentInMainloop.length; ++i) {
        var dt = this.timeSpentInMainloop[i] + this.timeSpentOutsideMainloop[i];
        var d = dt - avgDt;
        dtVariance += d*d;
      }
      dtVariance /= nSamples;

      var asmJSLoad = totalRAFDt * 100.0 / totalDt;

      var str = 'Last FPS: ' + fps.toFixed(2) + ', avg FPS:' + avgFps.toFixed(2) + ', min/avg/max dt: '
       + minDt.toFixed(2) + '/' + avgDt.toFixed(2) + '/' + maxDt.toFixed(2) + ' msecs, dt variance: ' + dtVariance.toFixed(3)
       + ', JavaScript CPU load: ' + asmJSLoad.toFixed(2) + '%';
      document.getElementById('fpsResult').innerHTML = str;
      this.fpsCounterLastPrint = now;
    }
  },

  // Creates a new section. Call once at startup.
  createSection: function createSection(number, name, drawColor) {
    var sect = this.sections[number];
    if (!sect) {
      sect = {
        count: 0,
        name: name,
        startTick: 0,
        accumulatedTime: 0,
        frametimes: [],
        drawColor: drawColor
      };
    }
    sect.name = name;
    this.sections[number] = sect;
  },

  // Call at runtime whenever the code execution enter a given profiling section.
  enterSection: function enterSection(sectionNumber) {
    var sect = this.sections[sectionNumber];
    // Handle recursive entering without getting confused (subsequent re-entering is ignored)
    ++sect.count;
    if (sect.count == 1) sect.startTick = performance.now();
  },

  // Call at runtime when the code execution exits the given profiling section.
  // Be sure to match each startSection(x) call with a call to endSection(x).
  endSection: function endSection(sectionNumber) {
    var sect = this.sections[sectionNumber];
    --sect.count;
    if (sect.count == 0) sect.accumulatedTime += performance.now() - sect.startTick;
  },

  // Called in the beginning of each main loop frame tick.
  frameStart: function frameStart() {
    this.currentFrameStartTime = performance.now();
    this.fpsCounter();
  },

  // Called in the end of each main loop frame tick.
  frameEnd: function frameEnd() {
    // Aggregate total times spent in each section to memory store to wait until the next stats UI redraw period.
    var totalTimeInSections = 0;
    for (var i in this.sections) {
      var sect = this.sections[i];
      sect.frametimes[this.currentHistogramX] = sect.accumulatedTime;
      totalTimeInSections += sect.accumulatedTime;
      sect.accumulatedTime = 0;
    }
    
    var t = performance.now();
    var cpuMainLoopDuration = t - this.currentFrameStartTime;
    var durationBetweenFrameUpdates = t - this.previousFrameEndTime;
    this.previousFrameEndTime = t;

    this.timeSpentInMainloop[this.currentHistogramX] = cpuMainLoopDuration - totalTimeInSections;
    this.timeSpentOutsideMainloop[this.currentHistogramX] = durationBetweenFrameUpdates - cpuMainLoopDuration;

    this.currentHistogramX = (this.currentHistogramX + 1) % this.canvas.width;
    // Redraw the UI if it is now time to do so.
    if ((this.currentHistogramX - this.lastUiUpdateEndX + this.canvas.width) % this.canvas.width >= this.uiUpdateInterval) {
      this.updateUi(this.lastUiUpdateEndX, this.currentHistogramX);
      this.lastUiUpdateEndX = this.currentHistogramX;
    }
  },

  // Installs the startup hooks and periodic UI update timer.
  initialize: function initialize() {
    // Create the UI display if it doesn't yet exist. If you want to customize the location/style of the cpuprofiler UI,
    // you can manually create this beforehand.
    cpuprofiler = document.getElementById('cpuprofiler');
    if (!cpuprofiler) {
      var div = document.createElement("div");
      div.innerHTML = "<div style='border: 2px solid black; padding: 2px;'><button style='display:inline;' onclick='Module.noExitRuntime=false;Module.exit();'>Halt</button><span id='fpsResult'></span><canvas style='border: 1px solid black; margin-left:auto; margin-right:auto; display: block;' id='cpuprofiler_canvas' width='800px' height='200'></canvas><div id='cpuprofiler'></div>";
      document.body.appendChild(div);
      cpuprofiler = document.getElementById('cpuprofiler');
    }
    
    this.canvas = document.getElementById('cpuprofiler_canvas');
    this.canvas.width = document.documentElement.clientWidth - 32;
    this.drawContext = this.canvas.getContext('2d');
//    if (document.getElementById('output')) document.getElementById('output').style.display = 'none';
//    if (document.getElementById('status')) document.getElementById('status').style.display = 'none';

    this.clearUi(0, this.canvas.width);
    this.drawGraphLabels();
    this.updateUi();
    Module['preMainLoop'] = function cpuprofiler_frameStart() { emscriptenCpuProfiler.frameStart(); }
    Module['postMainLoop'] = function cpuprofiler_frameEnd() { emscriptenCpuProfiler.frameEnd(); }
    
    this.createSection(0, 'GL', '#FF00FF');
  },

  drawHorizontalLine: function drawHorizontalLine(startX, endX, pixelThickness, msecs) {
    var height = msecs * this.canvas.height / this.verticalTimeScale;
    this.drawContext.fillRect(startX,this.canvas.height - height, endX - startX, pixelThickness);
  },

  clearUi: function clearUi(startX, endX) {  
    // Background clear
    this.drawContext.fillStyle="#324B4B";
    this.drawContext.fillRect(startX, 0, endX - startX, this.canvas.height);

    this.drawContext.fillStyle="#00FF00";
    this.drawHorizontalLine(startX, endX, 1, 16.6666666);
    this.drawContext.fillStyle="#FFFF00";
    this.drawHorizontalLine(startX, endX, 1, 33.3333333);
  },

  drawGraphLabels: function drawGraphLabels() {
    this.drawContext.fillStyle = "#C0C0C0";
    this.drawContext.font = "bold 10px Arial";
    this.drawContext.textAlign = "right";
    this.drawContext.fillText("16.66... ms", this.canvas.width - 3, this.canvas.height - 16.6666 * this.canvas.height / this.verticalTimeScale - 3);
    this.drawContext.fillText("33.33... ms", this.canvas.width - 3, this.canvas.height - 33.3333 * this.canvas.height / this.verticalTimeScale - 3);
  },

  drawBar: function drawBar(x) {
    var scale = this.canvas.height / this.verticalTimeScale;
    var y = this.canvas.height;
    var h = this.timeSpentInMainloop[x] * scale;
    y -= h;
    this.drawContext.fillStyle = "#0000BB";
    this.drawContext.fillRect(x, y, 1, h);
    for (var i in this.sections) {
      var sect = this.sections[i];
      var h = sect.frametimes[x] * scale;
      y -= h;
      this.drawContext.fillStyle = sect.drawColor;
      this.drawContext.fillRect(x, y, 1, h);
    }
    var h = this.timeSpentOutsideMainloop[x] * scale;
    y -= h;
    var fps60Limit = this.canvas.height - (16.666666666 + 1.0) * this.canvas.height / this.verticalTimeScale; // Be very lax, allow 1msec extra jitter.
    var fps30Limit = this.canvas.height - (33.333333333 + 1.0) * this.canvas.height / this.verticalTimeScale; // Be very lax, allow 1msec extra jitter.
    if (y < fps30Limit) this.drawContext.fillStyle = "#FF0000";
    else if (y < fps60Limit) this.drawContext.fillStyle = "#FFFF00";
    else this.drawContext.fillStyle = "#60A060";
    this.drawContext.fillRect(x, y, 1, h);
  },

  // Main UI update/redraw entry point. Drawing occurs incrementally to touch as few pixels as possible and to cause the least impact to the overall performance
  // while profiling.
  updateUi: function updateUi(startX, endX) {  
    // Poll whether user as changed the browser window, and if so, resize the profiler window and redraw it.
    if (this.canvas.width != document.documentElement.clientWidth - 32) {
      this.canvas.width = document.documentElement.clientWidth - 32;
      if (this.timeSpentInMainloop.length > this.canvas.width) this.timeSpentInMainloop.length = this.canvas.width;
      if (this.timeSpentOutsideMainloop.length > this.canvas.width) this.timeSpentOutsideMainloop.length = this.canvas.width;
      if (this.lastUiUpdateEndX >= this.canvas.width) this.lastUiUpdateEndX = 0;
      if (this.currentHistogramX >= this.canvas.width) this.currentHistogramX = 0;
      for (var i in this.sections) {
        var sect = this.sections[i];
        if (sect.frametimes.length > this.canvas.width) sect.frametimes.length = this.canvas.width;
      }
      this.clearUi(0, this.canvas.width);
      this.drawGraphLabels();
      startX = 0; // Full redraw all columns.
    }

    var clearDistance = this.uiUpdateInterval * 2 + 1;
    var clearStart = endX + clearDistance;
    var clearEnd = clearStart + this.uiUpdateInterval;
    if (endX < startX) {
      this.clearUi(clearStart, clearEnd);
      this.clearUi(0, endX + clearDistance+this.uiUpdateInterval);
      this.drawGraphLabels();
    } else {
      this.clearUi(clearStart, clearEnd);
    }

    if (endX < startX) {
      for (var x = startX; x < this.canvas.width; ++x) this.drawBar(x);
      startX = 0;
    }
    for (var x = startX; x < endX; ++x) this.drawBar(x);
  }
};

// Backwards compatibility with previously compiled code. Don't call this anymore!
function cpuprofiler_add_hooks() { emscriptenCpuProfiler.initialize(); }

if (typeof Module !== 'undefined' && typeof document !== 'undefined') emscriptenCpuProfiler.initialize();
