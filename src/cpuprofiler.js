// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// cpuprofiler.js is an interactive CPU execution profiler which measures the time spent in executing code that utilizes requestAnimationFrame(), setTimeout() and/or setInterval() handlers to run.
// Visit https://github.com/kripken/emscripten for the latest version.

// performance.now() might get faked later (this is done in the openwebgames.com test harness), so save the real one for cpu profiler.
// However, in Safari, assigning to the performance object will mysteriously vanish in other imported .js <script>, so for that, replace
// the whole object. That doesn't work for Chrome in turn, so need to resort to user agent sniffing.. (sad :/)
if (!performance.realNow) {
  var isSafari = /^((?!chrome|android).)*safari/i.test(navigator.userAgent);
  if (isSafari) {
    realPerformance = performance;
    performance = {
      realNow: function() { return realPerformance.now(); },
      now: function() { return realPerformance.now(); }
    };
  } else {
    performance.realNow = performance.now;
  }
}

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
  fpsCounterLastPrint: performance.realNow(),

  fpsCounterNumMostRecentFrames: 120,

  fpsCounterUpdateInterval: 2000, // msecs

  insideMainLoopRecursionCounter: 0, // Used to detect recursive entries to the main loop, which can happen in certain complex cases, e.g. if not using rAF to tick rendering to the canvas.

  // fpsCounter() is called once per frame to record an executed frame time, and to periodically update the FPS counter display.
  fpsCounter: function fpsCounter() {
    // Record the new frame time sample, and prune the history to K most recent frames.
    var now = performance.realNow();
    if (this.fpsCounterTicks.length < this.fpsCounterNumMostRecentFrames) this.fpsCounterTicks.push(now);
    else {
      for (var i = 0; i < this.fpsCounterTicks.length-1; ++i) this.fpsCounterTicks[i] = this.fpsCounterTicks[i+1];
      this.fpsCounterTicks[this.fpsCounterTicks.length-1] = now;
    }
  
    if (now - this.fpsCounterLastPrint > this.fpsCounterUpdateInterval) {
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

      // Compute the overhead added by WebGL:
      var hotGL = this.sections[0];
      var coldGL = this.sections[1];
      var webGLMSecsInsideMainLoop = (hotGL ? hotGL.accumulatedFrameTimeInsideMainLoop() : 0) + (coldGL ? coldGL.accumulatedFrameTimeInsideMainLoop() : 0);
      var webGLMSecsOutsideMainLoop = (hotGL ? hotGL.accumulatedFrameTimeOutsideMainLoop() : 0) + (coldGL ? coldGL.accumulatedFrameTimeOutsideMainLoop() : 0);
      var webGLMSecs = webGLMSecsInsideMainLoop + webGLMSecsOutsideMainLoop;

      var setIntervalSection = this.sections[2];
      var setTimeoutSection = this.sections[3];
      var totalCPUMsecs = totalRAFDt + setIntervalSection.accumulatedFrameTimeOutsideMainLoop() + setTimeoutSection.accumulatedFrameTimeOutsideMainLoop();

      var str = 'Last FPS: ' + fps.toFixed(2) + ', avg FPS:' + avgFps.toFixed(2) + ', min/avg/max dt: '
       + minDt.toFixed(2) + '/' + avgDt.toFixed(2) + '/' + maxDt.toFixed(2) + ' msecs, dt variance: ' + dtVariance.toFixed(3)
       + ', JavaScript CPU load: ' + asmJSLoad.toFixed(2) + '%';

      if (hotGL || coldGL) {
        str += '. WebGL CPU load: ' + (webGLMSecs * 100.0 / totalDt).toFixed(2) + '% (' + (webGLMSecs * 100.0 / totalCPUMsecs).toFixed(2) + '% of all CPU work)';
      }

      document.getElementById('fpsResult').innerHTML = str;
      this.fpsCounterLastPrint = now;
    }
  },

  // Creates a new section. Call once at startup.
  createSection: function createSection(number, name, drawColor, traceable) {
    while (this.sections.length <= number) this.sections.push(null); // Keep an array structure.
    var sect = this.sections[number];
    if (!sect) {
      sect = {
        count: 0,
        name: name,
        startTick: 0,
        accumulatedTimeInsideMainLoop: 0,
        accumulatedTimeOutsideMainLoop: 0,
        frametimesInsideMainLoop: [],
        frametimesOutsideMainLoop: [],
        drawColor: drawColor,
        traceable: traceable,
        accumulatedFrameTimeInsideMainLoop: function() {
          var total = 0;
          for(var i = 0; i < this.frametimesInsideMainLoop.length; ++i) if (this.frametimesInsideMainLoop[i]) total += this.frametimesInsideMainLoop[i];
          return total;
        },
        accumulatedFrameTimeOutsideMainLoop: function() {
          var total = 0;
          for(var i = 0; i < this.frametimesOutsideMainLoop.length; ++i) if (this.frametimesOutsideMainLoop[i]) total += this.frametimesOutsideMainLoop[i];
          return total;
        }
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
    if (sect.count == 1) sect.startTick = performance.realNow();
  },

  // Call at runtime when the code execution exits the given profiling section.
  // Be sure to match each startSection(x) call with a call to endSection(x).
  endSection: function endSection(sectionNumber) {
    var sect = this.sections[sectionNumber];
    --sect.count;
    if (sect.count == 0) {
      var timeInSection = performance.realNow() - sect.startTick;
      if (sect.traceable && timeInSection > this.logWebGLCallsSlowerThan) {
        var funcs = new Error().stack.toString().split('\n');
        var cs = '';
        for(var i = 2; i < 5 && i < funcs.length; ++i) {
          if (i != 2) cs += ' <- ';
          var fn = funcs[i];
          var at = fn.indexOf('@');
          if (at != -1) fn = fn.substr(0, at);
          fn = fn.trim();
          cs += '"' + fn + '"';
        }
        
        console.error('Trace: at t=' + performance.realNow().toFixed(1) + ', section "' + sect.name + '" called via ' + cs + ' took ' + timeInSection.toFixed(2) + ' msecs!');
      }
      if (this.insideMainLoopRecursionCounter) sect.accumulatedTimeInsideMainLoop += timeInSection;
      else sect.accumulatedTimeOutsideMainLoop += timeInSection;
    }
  },

  // Called in the beginning of each main loop frame tick.
  frameStart: function frameStart() {
    this.insideMainLoopRecursionCounter++;
    if (this.insideMainLoopRecursionCounter == 1) {
      this.currentFrameStartTime = performance.realNow();
      this.fpsCounter();
    }
  },

  // Called in the end of each main loop frame tick.
  frameEnd: function frameEnd() {
    this.insideMainLoopRecursionCounter--;
    if (this.insideMainLoopRecursionCounter != 0) return;

    // Aggregate total times spent in each section to memory store to wait until the next stats UI redraw period.
    for(var i = 0; i < this.sections.length; ++i) {
      var sect = this.sections[i];
      if (!sect) continue;
      sect.frametimesInsideMainLoop[this.currentHistogramX] = sect.accumulatedTimeInsideMainLoop;
      sect.frametimesOutsideMainLoop[this.currentHistogramX] = sect.accumulatedTimeOutsideMainLoop;
      sect.accumulatedTimeInsideMainLoop = 0;
      sect.accumulatedTimeOutsideMainLoop = 0;
    }
    
    var t = performance.realNow();
    var cpuMainLoopDuration = t - this.currentFrameStartTime;
    var durationBetweenFrameUpdates = t - this.previousFrameEndTime;
    this.previousFrameEndTime = t;

    this.timeSpentInMainloop[this.currentHistogramX] = cpuMainLoopDuration;
    this.timeSpentOutsideMainloop[this.currentHistogramX] = durationBetweenFrameUpdates - cpuMainLoopDuration;

    this.currentHistogramX = (this.currentHistogramX + 1) % this.canvas.width;
    // Redraw the UI if it is now time to do so.
    if ((this.currentHistogramX - this.lastUiUpdateEndX + this.canvas.width) % this.canvas.width >= this.uiUpdateInterval) {
      this.updateUi(this.lastUiUpdateEndX, this.currentHistogramX);
      this.lastUiUpdateEndX = this.currentHistogramX;
    }
  },

  colorBackground: '#324B4B',
  color60FpsBar: '#00FF00',
  color30FpsBar: '#FFFF00',
  colorTextLabel: '#C0C0C0',
  colorCpuTimeSpentInUserCode: '#0000BB',
  colorWorseThan30FPS: '#A06060',
  colorWorseThan60FPS: '#A0A030',
  color60FPS: '#40A040',
  colorHotGLFunction: '#FF00FF',
  colorColdGLFunction: '#0099CC',
  colorSetIntervalSection: '#FF0000',
  colorSetTimeoutSection: '#00FF00',

  hotGLFunctions: ['activeTexture', 'bindBuffer', 'bindFramebuffer', 'bindTexture', 'blendColor', 'blendEquation', 'blendEquationSeparate', 'blendFunc', 'blendFuncSeparate', 'bufferSubData', 'clear', 'clearColor', 'clearDepth', 'clearStencil', 'colorMask', 'compressedTexSubImage2D', 'copyTexSubImage2D', 'cullFace', 'depthFunc', 'depthMask', 'depthRange', 'disable', 'disableVertexAttribArray', 'drawArrays', 'drawArraysInstanced', 'drawElements', 'drawElementsInstanced', 'enable', 'enableVertexAttribArray', 'frontFace', 'lineWidth', 'pixelStorei', 'polygonOffset', 'sampleCoverage', 'scissor', 'stencilFunc', 'stencilFuncSeparate', 'stencilMask', 'stencilMaskSeparate', 'stencilOp', 'stencilOpSeparate', 'texSubImage2D', 'useProgram', 'viewport', 'beginQuery', 'endQuery', 'bindVertexArray', 'drawBuffers', 'copyBufferSubData', 'blitFramebuffer', 'invalidateFramebuffer', 'invalidateSubFramebuffer', 'readBuffer', 'texSubImage3D', 'copyTexSubImage3D', 'compressedTexSubImage3D', 'vertexAttribDivisor', 'drawRangeElements', 'clearBufferiv', 'clearBufferuiv', 'clearBufferfv', 'clearBufferfi', 'bindSampler', 'bindTransformFeedback', 'beginTransformFeedback', 'endTransformFeedback', 'transformFeedbackVaryings', 'pauseTransformFeedback', 'resumeTransformFeedback', 'bindBufferBase', 'bindBufferRange', 'uniformBlockBinding'],

  hookedWebGLContexts: [],
  logWebGLCallsSlowerThan: Infinity,

  toggleHelpTextVisible: function() {
    var help = document.getElementById('cpuprofiler_help_text');
    if (help.style) help.style.display = (help.style.display == 'none') ? 'block' : 'none';
  },

  // Installs the startup hooks and periodic UI update timer.
  initialize: function initialize() {
    // Create the UI display if it doesn't yet exist. If you want to customize the location/style of the cpuprofiler UI,
    // you can manually create this beforehand.
    cpuprofiler = document.getElementById('cpuprofiler');
    if (!cpuprofiler) {
      var css = '.colorbox { border: solid 1px black; margin-left: 10px; margin-right: 3px; display: inline-block; width: 20px; height: 10px; }';
      var style = document.createElement('style');
      style.type = 'text/css';
      style.appendChild(document.createTextNode(css));
      document.head.appendChild(style);

      var div = document.getElementById('cpuprofiler_container'); // Users can provide a container element where to place this if desired.
      if (!div) {
        div = document.createElement("div");
        document.body.appendChild(div);
      }
      var helpText = "<div style='margin-left: 10px;'>Color Legend:";
      helpText += "<div class='colorbox' style='background-color: " + this.colorCpuTimeSpentInUserCode + ";'></div>Main Loop (C/C++) Code"
      helpText += "<div class='colorbox' style='background-color: " + this.colorHotGLFunction + ";'></div>Hot WebGL Calls"
      helpText += "<div class='colorbox' style='background-color: " + this.colorColdGLFunction + ";'></div>Cold WebGL Calls"
      helpText += "<div class='colorbox' style='background-color: " + this.color60FPS + ";'></div>Browser Execution (&ge; 60fps)"
      helpText += "<div class='colorbox' style='background-color: " + this.colorWorseThan60FPS + ";'></div>Browser Execution (30-60fps)"
      helpText += "<div class='colorbox' style='background-color: " + this.colorWorseThan30FPS + ";'></div>Browser Execution (&lt; 30fps)"
      helpText += "<div class='colorbox' style='background-color: " + this.colorSetIntervalSection + ";'></div>setInterval()"
      helpText += "<div class='colorbox' style='background-color: " + this.colorSetTimeoutSection + ";'></div>setTimeout()"
      helpText += "</div>";
      helpText += "<div id='cpuprofiler_help_text' style='display:none; margin-top: 20px; margin-left: 10px;'>"
      helpText += "<p>cpuprofiler.js is an interactive CPU execution profiler which measures the time spent in executing code that utilizes requestAnimationFrame(), setTimeout() and/or setInterval() handlers to run. Each one pixel column in the above graph denotes a single executed application frame tick. The vertical axis represents in millisecond units the time taken to render a frame. Use this tool to interactively locate stuttering related events, and then use other profiling tools (<a href='https://developer.mozilla.org/en-US/docs/Tools/Performance'>Firefox profiler</a>, <a href='https://developer.mozilla.org/en-US/docs/Mozilla/Performance/Profiling_with_the_Built-in_Profiler'>geckoprofiler</a>) to identify their cause."
      helpText += "<p>The header line above the graph prints out timing statistics:"
      helpText += "<ul><li><b>Last FPS:</b> Displays the current FPS measured by averaging across " + this.fpsCounterNumMostRecentFrames + " most recently rendered frames.";
      helpText += "<li><b>Avg FPS:</b> Displays the total FPS measured by averaging across the whole visible graph, i.e. <span id='ntotalframes'>" + (document.documentElement.clientWidth - 32) + "</span> most recently rendered frames.";
      helpText += "<li><b>min/avg/max dt:</b> Displays the minimum, average and maximum durations that an application frame took overall, across the visible graph. These numbers include the time the browser was idle.";
      helpText += "<li><b>dt variance:</b> Computes the amount of <a href='https://en.wikipedia.org/wiki/Variance'>statistical variance</a> in the overall frame durations.";
      helpText += "<li><b>JavaScript CPU load:</b> This field estimates the amount of time the CPU was busy executing user code (requestAnimationFrame, setTimeout and setInterval handlers), with the simple assumption that the browser would be idle the remaining time.";
      helpText += "<li><b>WebGL CPU load:</b> This field estimates the amount of time the CPU was busy running code inside the browser WebGL API. The value in parentheses shows the ratio of time that WebGL consumes of all per-frame CPU work.";
      helpText += "</ul>Use the <span style='border: solid 1px #909090;'>Halt</span> button to abort page execution (Emscripten only). ";
      helpText += "<br>Press the <span style='border: solid 1px #909090;'>Profile WebGL</span> button to toggle the profiling of WebGL CPU overhead. When the button background is displayed in green, WebGL CPU profiling is active. This profiling mode has some overhead by itself, so when recording profiles with other tools, prefer to leave this disabled.";
      helpText += "<br>With the <span style='border: solid 1px #909090;'>Trace Calls</span> option, you can log WebGL and setInterval()/setTimeout() operations that take a long time to finish. These are typically cold operations like shader compilation or large reallocating buffer uploads, or other long event-based computation. For this option to be able to trace WebGL calls, the option Profile WebGL must also be enabled. The trace results appear in the web page console.";
      helpText += "<p>The different colors on the graph have the following meaning:";
      helpText += "<br><div class='colorbox' style='background-color: " + this.colorCpuTimeSpentInUserCode + ";'></div><b>Main Loop (C/C++) Code</b>: This is the time spent executing application JavaScript code inside the main loop event handler, generally via requestAnimationFrame().";
      helpText += "<br><div class='colorbox' style='background-color: " + this.colorHotGLFunction + ";'></div><b>Hot WebGL Calls</b>: This measures the CPU time spent in running common per-frame rendering related WebGL calls: <div style='margin-left: 100px; margin-top: 10px; max-width: 800px; font-size: 12px;'>" + this.hotGLFunctions.join(', ') + ', uniform* and vertexAttrib*.</div>';
      helpText += "<br><div class='colorbox' style='background-color: " + this.colorColdGLFunction + ";'></div><b>Cold WebGL Calls</b>: This shows the CPU time spent in all the remaining WebGL functions that are not considered 'hot' (not in the above list).";
      helpText += "<br><div class='colorbox' style='background-color: " + this.color60FPS + ";'></div><b>Browser Execution (&ge; 60fps)</b>: This is the time taken by browser that falls outside the tracked requestAnimationFrame(), setTimeout() and/or setInterval() handlers. If the page is running at 60fps, the browser time will be drawn with this color. Likely the browser was idle waiting for vsync.";
      helpText += "<br><div class='colorbox' style='background-color: " + this.colorWorseThan60FPS + ";'></div><b>Browser Execution (30-60fps)</b>: This is the same as above, except that when 60fps is not reached, the browser time is drawn in this color.";
      helpText += "<br><div class='colorbox' style='background-color: " + this.colorWorseThan30FPS + ";'></div><b>Browser Execution (&lt; 30fps)</b>: Same as above, except that the frame completed slowly, so the browser time is drawn in this color. Long spikes of this color indicate that the browser is running some internal operations (e.g. garbage collection) that can cause stuttering.";
      helpText += "<br><div class='colorbox' style='background-color: " + this.colorSetIntervalSection + ";'></div><b>setInterval()</b>: Specifies the amount of time spent in executing user code in setInterval() handlers.";
      helpText += "<br><div class='colorbox' style='background-color: " + this.colorSetTimeoutSection + ";'></div><b>setTimeout()</b>: Specifies the amount of time spent in executing user code in setTimeout() handlers.";
      helpText += "<p>For bugs and suggestions, visit <a href='https://github.com/kripken/emscripten/issues'>Emscripten bug tracker</a>.";
      helpText += "</div>";

      div.innerHTML = "<div style='color: black; border: 2px solid black; padding: 2px; margin-bottom: 10px; margin-left: 5px; margin-right: 5px; margin-top: 5px; background-color: #F0F0FF;'><span style='margin-left: 10px;'><b>Cpu Profiler</b><sup style='cursor: pointer;' onclick='emscriptenCpuProfiler.toggleHelpTextVisible();'>[?]</sup></span> <button style='display:inline; border: solid 1px #ADADAD; margin: 2px; background-color: #E1E1E1;' onclick='Module.noExitRuntime=false;Module.exit();'>Halt</button><button id='toggle_webgl_profile' style='display:inline; border: solid 1px #ADADAD; margin: 2px;  background-color: #E1E1E1;' onclick='emscriptenCpuProfiler.toggleHookWebGL()'>Profile WebGL</button><button id='toggle_webgl_trace' style='display:inline; border: solid 1px #ADADAD; margin: 2px;  background-color: #E1E1E1;' onclick='emscriptenCpuProfiler.toggleTraceWebGL()'>Trace Calls</button> slower than <input id='trace_limit' oninput='emscriptenCpuProfiler.disableTraceWebGL();' style='width:40px;' value='100'></input> msecs. <span id='fpsResult' style='margin-left: 5px;'></span><canvas style='border: 1px solid black; margin-left:auto; margin-right:auto; display: block;' id='cpuprofiler_canvas' width='800px' height='200'></canvas><div id='cpuprofiler'></div>" + helpText;
      document.getElementById('trace_limit').onkeydown = function(e) { if (e.which == 13 || e.keycode == 13) emscriptenCpuProfiler.enableTraceWebGL(); else emscriptenCpuProfiler.disableTraceWebGL(); };
      cpuprofiler = document.getElementById('cpuprofiler');

      if (location.search.indexOf('expandhelp') != -1) this.toggleHelpTextVisible();
    }
    
    this.canvas = document.getElementById('cpuprofiler_canvas');
    this.canvas.width = document.documentElement.clientWidth - 32;
    this.drawContext = this.canvas.getContext('2d');

    this.clearUi(0, this.canvas.width);
    this.drawGraphLabels();
    this.updateUi();
    Module['preMainLoop'] = function cpuprofiler_frameStart() { emscriptenCpuProfiler.frameStart(); }
    Module['postMainLoop'] = function cpuprofiler_frameEnd() { emscriptenCpuProfiler.frameEnd(); }
  },

  drawHorizontalLine: function drawHorizontalLine(startX, endX, pixelThickness, msecs) {
    var height = msecs * this.canvas.height / this.verticalTimeScale;
    this.drawContext.fillRect(startX,this.canvas.height - height, endX - startX, pixelThickness);
  },

  clearUi: function clearUi(startX, endX) {  
    // Background clear
    this.drawContext.fillStyle = this.colorBackground;
    this.drawContext.fillRect(startX, 0, endX - startX, this.canvas.height);

    this.drawContext.fillStyle = this.color60FpsBar;
    this.drawHorizontalLine(startX, endX, 1, 16.6666666);
    this.drawContext.fillStyle = this.color30FpsBar;
    this.drawHorizontalLine(startX, endX, 1, 33.3333333);
  },

  drawGraphLabels: function drawGraphLabels() {
    this.drawContext.fillStyle = this.colorTextLabel;
    this.drawContext.font = "bold 10px Arial";
    this.drawContext.textAlign = "right";
    this.drawContext.fillText("16.66... ms", this.canvas.width - 3, this.canvas.height - 16.6666 * this.canvas.height / this.verticalTimeScale - 3);
    this.drawContext.fillText("33.33... ms", this.canvas.width - 3, this.canvas.height - 33.3333 * this.canvas.height / this.verticalTimeScale - 3);
  },

  drawBar: function drawBar(x) {
    var timeSpentInSectionsInsideMainLoop = 0;
    for(var i = 0; i < this.sections.length; ++i) {
      var sect = this.sections[i];
      if (!sect) continue;
      timeSpentInSectionsInsideMainLoop += sect.frametimesInsideMainLoop[x];
    }
    var scale = this.canvas.height / this.verticalTimeScale;
    var y = this.canvas.height;
    var h = (this.timeSpentInMainloop[x]-timeSpentInSectionsInsideMainLoop) * scale;
    y -= h;
    this.drawContext.fillStyle = this.colorCpuTimeSpentInUserCode;
    this.drawContext.fillRect(x, y, 1, h);
    for(var i = 0; i < this.sections.length; ++i) {
      var sect = this.sections[i];
      if (!sect) continue;
      var h = (sect.frametimesInsideMainLoop[x] + sect.frametimesOutsideMainLoop[x]) * scale;
      y -= h;
      this.drawContext.fillStyle = sect.drawColor;
      this.drawContext.fillRect(x, y, 1, h);
    }
    var h = this.timeSpentOutsideMainloop[x] * scale;
    y -= h;
    var fps60Limit = this.canvas.height - (16.666666666 + 1.0) * this.canvas.height / this.verticalTimeScale; // Be very lax, allow 1msec extra jitter.
    var fps30Limit = this.canvas.height - (33.333333333 + 1.0) * this.canvas.height / this.verticalTimeScale; // Be very lax, allow 1msec extra jitter.
    if (y < fps30Limit) this.drawContext.fillStyle = this.colorWorseThan30FPS;
    else if (y < fps60Limit) this.drawContext.fillStyle = this.colorWorseThan60FPS;
    else this.drawContext.fillStyle = this.color60FPS;
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
        if (!sect) continue;
        if (sect.frametimesInsideMainLoop.length > this.canvas.width) sect.frametimesInsideMainLoop.length = this.canvas.width;
        if (sect.frametimesOutsideMainLoop.length > this.canvas.width) sect.frametimesOutsideMainLoop.length = this.canvas.width;
      }
      document.getElementById('ntotalframes').innerHTML = this.canvas.width + '';
      this.clearUi(0, this.canvas.width);
      this.drawGraphLabels();
      startX = 0; // Full redraw all columns.
    }

    // Also poll to autodetect if there is an Emscripten GL canvas available that we could hook into. This is a bit clumsy, but there's no good location to get an event after GL context has been created, so
    // need to resort to polling.
    if (location.search.indexOf('webglprofiler') != -1 && !this.automaticallyHookedWebGLProfiler) {
      this.hookWebGL();
      if (location.search.indexOf('tracegl') != -1) {
        var res = location.search.match(/tracegl=(\d+)/);
        var traceGl = res[1];
        document.getElementById('trace_limit').value = traceGl;
        this.enableTraceWebGL();
      }
      if (this.hookedWebGLContexts.length > 0) this.automaticallyHookedWebGLProfiler = true;
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
  },

  // Work around Microsoft Edge bug where webGLContext.function.length always returns 0.
  webGLFunctionLength: function(f) {
    var l0 = ['getContextAttributes','isContextLost','getSupportedExtensions','createBuffer','createFramebuffer','createProgram','createRenderbuffer','createTexture','finish','flush','getError', 'createVertexArray', 'createQuery', 'createSampler', 'createTransformFeedback', 'endTransformFeedback', 'pauseTransformFeedback', 'resumeTransformFeedback'];
    var l1 = ['getExtension','activeTexture','blendEquation','checkFramebufferStatus','clear','clearDepth','clearStencil','compileShader','createShader','cullFace','deleteBuffer','deleteFramebuffer','deleteProgram','deleteRenderbuffer','deleteShader','deleteTexture','depthFunc','depthMask','disable','disableVertexAttribArray','enable','enableVertexAttribArray','frontFace','generateMipmap','getAttachedShaders','getParameter','getProgramInfoLog','getShaderInfoLog','getShaderSource','isBuffer','isEnabled','isFramebuffer','isProgram','isRenderbuffer','isShader','isTexture','lineWidth','linkProgram','stencilMask','useProgram','validateProgram', 'deleteQuery', 'isQuery', 'deleteVertexArray', 'bindVertexArray', 'isVertexArray', 'drawBuffers', 'readBuffer', 'endQuery', 'deleteSampler', 'isSampler', 'isSync', 'deleteSync', 'deleteTransformFeedback', 'isTransformFeedback', 'beginTransformFeedback'];
    var l2 = ['attachShader','bindBuffer','bindFramebuffer','bindRenderbuffer','bindTexture','blendEquationSeparate','blendFunc','depthRange','detachShader','getActiveAttrib','getActiveUniform','getAttribLocation','getBufferParameter','getProgramParameter','getRenderbufferParameter','getShaderParameter','getShaderPrecisionFormat','getTexParameter','getUniform','getUniformLocation','getVertexAttrib','getVertexAttribOffset','hint','pixelStorei','polygonOffset','sampleCoverage','shaderSource','stencilMaskSeparate','uniform1f','uniform1fv','uniform1i','uniform1iv','uniform2fv','uniform2iv','uniform3fv','uniform3iv','uniform4fv','uniform4iv','vertexAttrib1f','vertexAttrib1fv','vertexAttrib2fv','vertexAttrib3fv','vertexAttrib4fv', 'vertexAttribDivisor', 'beginQuery', 'invalidateFramebuffer', 'getFragDataLocation', 'uniform1ui', 'uniform1uiv', 'uniform2uiv', 'uniform3uiv', 'uniform4uiv', 'vertexAttribI4iv', 'vertexAttribI4uiv', 'getQuery', 'getQueryParameter', 'bindSampler', 'getSamplerParameter', 'fenceSync', 'getSyncParameter', 'bindTransformFeedback', 'getTransformFeedbackVarying', 'getIndexedParameter', 'getUniformIndices', 'getUniformBlockIndex', 'getActiveUniformBlockName'];
    var l3 = ['bindAttribLocation','bufferData','bufferSubData','drawArrays','getFramebufferAttachmentParameter','stencilFunc','stencilOp','texParameterf','texParameteri','uniform2f','uniform2i','uniformMatrix2fv','uniformMatrix3fv','uniformMatrix4fv','vertexAttrib2f', 'getBufferSubData', 'getInternalformatParameter', 'uniform2ui', 'uniformMatrix2x3fv', 'uniformMatrix3x2fv', 'uniformMatrix2x4fv', 'uniformMatrix4x2fv', 'uniformMatrix3x4fv', 'uniformMatrix4x3fv', 'clearBufferiv', 'clearBufferuiv', 'clearBufferfv', 'samplerParameteri', 'samplerParameterf', 'clientWaitSync', 'waitSync', 'transformFeedbackVaryings', 'bindBufferBase', 'getActiveUniforms', 'getActiveUniformBlockParameter', 'uniformBlockBinding'];
    var l4 = ['blendColor','blendFuncSeparate','clearColor','colorMask','drawElements','framebufferRenderbuffer','renderbufferStorage','scissor','stencilFuncSeparate','stencilOpSeparate','uniform3f','uniform3i','vertexAttrib3f','viewport', 'drawArraysInstanced', 'uniform3ui', 'clearBufferfi'];
    var l5 = ['framebufferTexture2D','uniform4f','uniform4i','vertexAttrib4f', 'drawElementsInstanced', 'copyBufferSubData', 'framebufferTextureLayer', 'renderbufferStorageMultisample', 'texStorage2D', 'uniform4ui', 'vertexAttribI4i', 'vertexAttribI4ui', 'vertexAttribIPointer', 'bindBufferRange'];
    var l6 = ['texImage2D', 'vertexAttribPointer', 'invalidateSubFramebuffer', 'texStorage3D', 'drawRangeElements'];
    var l7 = ['compressedTexImage2D', 'readPixels', 'texSubImage2D'];
    var l8 = ['compressedTexSubImage2D', 'copyTexImage2D', 'copyTexSubImage2D', 'compressedTexImage3D'];
    var l9 = ['copyTexSubImage3D'];
    var l10 = ['blitFramebuffer', 'texImage3D', 'compressedTexSubImage3D'];
    var l11 = ['texSubImage3D'];
    if (l0.indexOf(f) != -1) return 0;
    if (l1.indexOf(f) != -1) return 1;
    if (l2.indexOf(f) != -1) return 2;
    if (l3.indexOf(f) != -1) return 3;
    if (l4.indexOf(f) != -1) return 4;
    if (l5.indexOf(f) != -1) return 5;
    if (l6.indexOf(f) != -1) return 6;
    if (l7.indexOf(f) != -1) return 7;
    if (l8.indexOf(f) != -1) return 8;
    if (l9.indexOf(f) != -1) return 9;
    if (l10.indexOf(f) != -1) return 10;
    if (l11.indexOf(f) != -1) return 11;
    throw 'Unexpected WebGL function ' + f;
  },

  detectWebGLContext: function() {
    if (Module['canvas'] && Module['canvas'].GLctxObject && Module['canvas'].GLctxObject.GLctx) return Module['canvas'].GLctxObject.GLctx;
    else if (typeof GLctx !== 'undefined') return GLctx;
    else if (Module.ctx) return Module.ctx;
    return null;
  },

  toggleHookWebGL: function(glCtx) {
    if (!glCtx) glCtx = this.detectWebGLContext();
    if (this.hookedWebGLContexts.indexOf(glCtx) != -1) this.unhookWebGL(glCtx);
    else this.hookWebGL(glCtx);
  },

  enableTraceWebGL: function() {
    document.getElementById("toggle_webgl_trace").style.background = '#00FF00';
    this.logWebGLCallsSlowerThan = parseInt(document.getElementById('trace_limit').value);
  },

  disableTraceWebGL: function() {
    document.getElementById("toggle_webgl_trace").style.background = '#E1E1E1';
    this.logWebGLCallsSlowerThan = Infinity;
  },

  toggleTraceWebGL: function() {
    if (this.logWebGLCallsSlowerThan == Infinity) {
      this.enableTraceWebGL();
    } else {
      this.disableTraceWebGL();
    }
  },

  unhookWebGL: function(glCtx) {
    if (!glCtx) glCtx = this.detectWebGLContext();
    if (!glCtx.cpuprofilerAlreadyHooked) return;
    glCtx.cpuprofilerAlreadyHooked = false;
    this.hookedWebGLContexts.splice(this.hookedWebGLContexts.indexOf(glCtx), 1);
    document.getElementById("toggle_webgl_profile").style.background = '#E1E1E1';

    for(var f in glCtx) {
      if (typeof glCtx[f] !== 'function' || f.indexOf('real_') == 0) continue;
      var realf = 'real_' + f;
      glCtx[f] = glCtx[realf];
      delete glCtx[realf];
    }
  },

  hookWebGLFunction: function(f, glCtx) {
    var this_ = this;
    var section = (this_.hotGLFunctions.indexOf(f) != -1 || f.indexOf('uniform') == 0 || f.indexOf('vertexAttrib') == 0) ? 0 : 1;
    var realf = 'real_' + f;
    glCtx[realf] = glCtx[f];
    var numArgs = this_.webGLFunctionLength(f); // On Firefox & Chrome, could do "glCtx[realf].length", but that doesn't work on Edge, which always reports 0.
    // Accessing 'arguments' is super slow, so to avoid overhead, statically reason the number of arguments.
    switch(numArgs) {
      case 0: glCtx[f] = function webgl_0() { this_.enterSection(section); var ret = glCtx[realf](); this_.endSection(section); return ret; }; break;
      case 1: glCtx[f] = function webgl_1(a1) { this_.enterSection(section); var ret =  glCtx[realf](a1); this_.endSection(section); return ret; }; break;
      case 2: glCtx[f] = function webgl_2(a1, a2) { this_.enterSection(section); var ret =  glCtx[realf](a1, a2); this_.endSection(section); return ret; }; break;
      case 3: glCtx[f] = function webgl_3(a1, a2, a3) { this_.enterSection(section); var ret =  glCtx[realf](a1, a2, a3); this_.endSection(section); return ret; }; break;
      case 4: glCtx[f] = function webgl_4(a1, a2, a3, a4) { this_.enterSection(section); var ret =  glCtx[realf](a1, a2, a3, a4); this_.endSection(section); return ret; }; break;
      case 5: glCtx[f] = function webgl_5(a1, a2, a3, a4, a5) { this_.enterSection(section); var ret =  glCtx[realf](a1, a2, a3, a4, a5); this_.endSection(section); return ret; }; break;
      case 6: glCtx[f] = function webgl_6(a1, a2, a3, a4, a5, a6) { this_.enterSection(section); var ret =  glCtx[realf](a1, a2, a3, a4, a5, a6); this_.endSection(section); return ret; }; break;
      case 7: glCtx[f] = function webgl_7(a1, a2, a3, a4, a5, a6, a7) { this_.enterSection(section); var ret =  glCtx[realf](a1, a2, a3, a4, a5, a6, a7); this_.endSection(section); return ret; }; break;
      case 8: glCtx[f] = function webgl_8(a1, a2, a3, a4, a5, a6, a7, a8) { this_.enterSection(section); var ret =  glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8); this_.endSection(section); return ret; }; break;
      case 9: glCtx[f] = function webgl_9(a1, a2, a3, a4, a5, a6, a7, a8, a9) { this_.enterSection(section); var ret =  glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8, a9); this_.endSection(section); return ret; }; break;
      case 10: glCtx[f] = function webgl_10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) { this_.enterSection(section); var ret =  glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10); this_.endSection(section); return ret; }; break;
      case 11: glCtx[f] = function webgl_11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) { this_.enterSection(section); var ret =  glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11); this_.endSection(section); return ret; }; break;
      default: throw 'hookWebGL failed! Unexpected length ' + glCtx[realf].length;
    }
  },

  hookWebGL: function(glCtx) {
    if (!glCtx) glCtx = this.detectWebGLContext();
    if (!glCtx) return;
    if (!((typeof WebGLRenderingContext !== 'undefined' && glCtx instanceof WebGLRenderingContext)
     || (typeof WebGL2RenderingContext !== 'undefined' && glCtx instanceof WebGL2RenderingContext))) {
      document.getElementById("toggle_webgl_profile").disabled = true;
      return;
    }

    if (glCtx.cpuprofilerAlreadyHooked) return;
    glCtx.cpuprofilerAlreadyHooked = true;
    this.hookedWebGLContexts.push(glCtx);
    document.getElementById("toggle_webgl_profile").style.background = '#00FF00';

    // Hot GL functions are ones that you'd expect to find during render loops (render calls, dynamic resource uploads), cold GL functions are load time functions (shader compilation, texture/mesh creation)
    // Distinguishing between these two allows pinpointing locations of troublesome GL usage that might cause performance issues.
    this.createSection(0, 'Hot GL', this.colorHotGLFunction, /*traceable=*/true);
    this.createSection(1, 'Cold GL', this.colorColdGLFunction, /*traceable=*/true);
    for(var f in glCtx) {
      if (typeof glCtx[f] !== 'function' || f.indexOf('real_') == 0) continue;
      this.hookWebGLFunction(f, glCtx);
    }
    var this_ = this;
    // The above injection won't work for texImage2D and texSubImage2D, which have multiple overloads.
    glCtx['texImage2D'] = function(a1, a2, a3, a4, a5, a6, a7, a8, a9) { 
      this_.enterSection(1);
      var ret = (a7 !== undefined) ? glCtx['real_texImage2D'](a1, a2, a3, a4, a5, a6, a7, a8, a9) : glCtx['real_texImage2D'](a1, a2, a3, a4, a5, a6);
      this_.endSection(1);
      return ret;
    };
    glCtx['texSubImage2D'] = function(a1, a2, a3, a4, a5, a6, a7, a8, a9) { 
      this_.enterSection(0);
      var ret = (a8 !== undefined) ? glCtx['real_texSubImage2D'](a1, a2, a3, a4, a5, a6, a7, a8, a9) : glCtx['real_texSubImage2D'](a1, a2, a3, a4, a5, a6, a7);
      this_.endSection(0);
      return ret;
    };
    glCtx['texSubImage3D'] = function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) { 
      this_.enterSection(0);
      var ret = (a9 !== undefined) ? glCtx['real_texSubImage3D'](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) : glCtx['real_texSubImage2D'](a1, a2, a3, a4, a5, a6, a7, a8);
      this_.endSection(0);
      return ret;
    };
  }
};

// Hook into setInterval to be able to capture the time spent executing them.
emscriptenCpuProfiler.createSection(2, 'setInterval', emscriptenCpuProfiler.colorSetIntervalSection, /*traceable=*/true);
realSetInterval = setInterval;
setInterval = function(fn, delay) {
  function wrappedSetInterval() {
    emscriptenCpuProfiler.enterSection(2);
    fn();
    emscriptenCpuProfiler.endSection(2);
  };
  return realSetInterval(wrappedSetInterval, delay);
}

// Hook into setTimeout to be able to capture the time spent executing them.
emscriptenCpuProfiler.createSection(3, 'setTimeout', emscriptenCpuProfiler.colorSetTimeoutSection, /*traceable=*/true);
realSetTimeout = setTimeout;
setTimeout = function(fn, delay) {
  function wrappedSetTimeout() {
    emscriptenCpuProfiler.enterSection(3);
    fn();
    emscriptenCpuProfiler.endSection(3);
  };
  return realSetTimeout(wrappedSetTimeout, delay);
}

// Backwards compatibility with previously compiled code. Don't call this anymore!
function cpuprofiler_add_hooks() { emscriptenCpuProfiler.initialize(); }

if (typeof Module !== 'undefined' && typeof document !== 'undefined') emscriptenCpuProfiler.initialize();
