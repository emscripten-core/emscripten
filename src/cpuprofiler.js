// CONFIGURATION: UI update interval in milliseconds:
var CPUPROFILER_UI_UPDATE_INTERVAL = 1;
var cpuprofiler_ui_lastupdate_endX = 0;

var cpuprofiler_t_mainloop = [];
var cpuprofiler_t_outside_mainloop = [];

var cpuprofiler_histogram_x = 0;

var cpuprofiler_currentframe_starttime = 0;
var cpuprofiler_prevframe_endtime = 0;
var cpuprofiler_time_between_frameupdates = 0;

var sections = [];

// FPSCounter
var fpsCounterTicks = [];
var fpsCounterLastPrint = performance.now();
var avgs = [];
function fpsReset() { avgs = []; }

function fpsCounter()
{
  var t = performance.now();
  fpsCounterTicks.push(t);
  var i = 0;
  if (fpsCounterTicks.length > 120) fpsCounterTicks = fpsCounterTicks.slice(fpsCounterTicks.length-120, fpsCounterTicks.length);
  
  if (t - fpsCounterLastPrint > 2000)
  {
    var fps = ((fpsCounterTicks.length-1) * 1000.0 / (fpsCounterTicks[fpsCounterTicks.length-1]-fpsCounterTicks[0]));
    /*
    avgs.push(fps);
    if (avgs.length > 60) avgs = avgs.slice(avgs.length-60, avgs.length);
    var avgsSorted = avgs.slice(0);
    avgsSorted.sort();
    var avgFps = (avgsSorted[(avgsSorted.length/2)|0]).toFixed(2);
*/
    var totalDt = 0;
    var totalRAFDt = 0;
    var minDt = 99999999;
    var maxDt = 0;
    var nSamples = 0;
    for(var i = 0; i < cpuprofiler_t_mainloop.length; ++i)
    {
      var dt = cpuprofiler_t_mainloop[i] + cpuprofiler_t_outside_mainloop[i];
      totalRAFDt += cpuprofiler_t_mainloop[i];
      if (dt > 0) ++nSamples;
      totalDt += dt;
      minDt = Math.min(minDt, dt);
      maxDt = Math.max(maxDt, dt);
    }
    var avgDt = totalDt / nSamples;
    var avgFps = 1000.0 / avgDt;
    var dtVariance = 0;
    for(var i = 1; i < cpuprofiler_t_mainloop.length; ++i)
    {
      var dt = cpuprofiler_t_mainloop[i] + cpuprofiler_t_outside_mainloop[i];
      var d = dt - avgDt;
      dtVariance += d*d;      
    }
    dtVariance /= nSamples;
    /*
    var avgDt = (fpsCounterTicks[fpsCounterTicks.length-1] - fpsCounterTicks[0]) / (fpsCounterTicks.length-1);
    var dtVariance = 0;
    for(var i = 1; i < fpsCounterTicks.length; ++i)
    {
      var d = (fpsCounterTicks[i] - fpsCounterTicks[i-1]) - avgDt;
      dtVariance += d*d;
    }
    dtVariance /= fpsCounterTicks.length-1;
    */

//    var str = 'Last FPS: ' + fps.toFixed(2) + ', avg FPS:' + avgFps + ' (' + avgsSorted.length + ' samples), min/avg/max dt: '
//     + minDt.toFixed(2) + '/' + avgDt.toFixed(2) + '/' + maxDt.toFixed(2) + ' msecs, dt variance: ' + dtVariance.toFixed(3);

    var asmJSLoad = totalRAFDt * 100.0 / totalDt;

    var str = 'Last FPS: ' + fps.toFixed(2) + ', avg FPS:' + avgFps.toFixed(2) + ', min/avg/max dt: '
     + minDt.toFixed(2) + '/' + avgDt.toFixed(2) + '/' + maxDt.toFixed(2) + ' msecs, dt variance: ' + dtVariance.toFixed(3)
     + ', JavaScript CPU load: ' + asmJSLoad.toFixed(2) + '%';
    document.getElementById('fpsResult').innerHTML = str;
    fpsCounterLastPrint = t;
  }
}
// ~FPSCounter

function cpuprofiler_createSection(number, name, drawColor) {
  var sect = sections[number];
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
  sections[number] = sect;
}

function cpuprofiler_startSection(sectionNumber) {
  var sect = sections[sectionNumber];
  sect.count++;
  if (sect.count == 1) {
    sect.startTick = performance.now();
  }
}

function cpuprofiler_endSection(sectionNumber) {
  var sect = sections[sectionNumber];
  sect.count--;
  if (sect.count == 0) {
    sect.accumulatedTime += performance.now() - sect.startTick;
  }
}

function cpuprofiler_frameStart() {
  cpuprofiler_currentframe_starttime = performance.now();//_emscripten_get_now();
  fpsCounter();
}

function cpuprofiler_frameEnd() {
  var maxTime = 40.0; // Always show at least 20 msecs.

  // Aggregate total times to memory store to wait until the next stats UI redraw period.
  var totalTimeInSections = 0;
  for(var i in sections) {
    var sect = sections[i];
    sect.frametimes[cpuprofiler_histogram_x] = sect.accumulatedTime;
    totalTimeInSections += sect.accumulatedTime;
    sect.accumulatedTime = 0;
  }
  
  var t = performance.now();//_emscripten_get_now();
  var cpuMainLoopDuration = t - cpuprofiler_currentframe_starttime;
  var durationBetweenFrameUpdates = t - cpuprofiler_prevframe_endtime;
  cpuprofiler_prevframe_endtime = t;
/* xx
  if (cpuprofiler_histogram_x == 0) {
    cpuprofiler_clear_ui();
  }
  */
  cpuprofiler_t_mainloop[cpuprofiler_histogram_x] = cpuMainLoopDuration - totalTimeInSections;
  cpuprofiler_t_outside_mainloop[cpuprofiler_histogram_x] = durationBetweenFrameUpdates - cpuMainLoopDuration;
//  cpuprofiler_drawBar(cpuprofiler_histogram_x, duration, maxTime);
// xx  cpuprofiler_drawStackedBar(cpuprofiler_histogram_x, durationBetweenFrameUpdates-cpuMainLoopDuration, cpuMainLoopDuration, maxTime);

  cpuprofiler_histogram_x = (cpuprofiler_histogram_x+1)%cpuprofiler_canvas.width;

  if ((cpuprofiler_histogram_x - cpuprofiler_ui_lastupdate_endX + cpuprofiler_canvas.width) % cpuprofiler_canvas.width >= CPUPROFILER_UI_UPDATE_INTERVAL) {
    cpuprofiler_update_ui(cpuprofiler_ui_lastupdate_endX, cpuprofiler_histogram_x);
    cpuprofiler_ui_lastupdate_endX = cpuprofiler_histogram_x;
  }
}

// Installs startup hook and periodic UI update timer.
function cpuprofiler_add_hooks() {
  cpuprofiler = document.getElementById('cpuprofiler');
  if (!cpuprofiler) {
    var div = document.createElement("div");
    div.innerHTML = "<div style='border: 2px solid black; padding: 2px;'><button style='display:inline;' onclick='Module.noExitRuntime=false;Module.exit();'>Halt</button><span id='fpsResult'></span><canvas style='border: 1px solid black;' id='cpuprofiler_canvas' width='800' height='200'></canvas><div id='cpuprofiler'></div>";
    document.body.appendChild(div);
    cpuprofiler = document.getElementById('cpuprofiler');
  }
  
  cpuprofiler_canvas = document.getElementById('cpuprofiler_canvas');
  cpuprofiler_canvas.width = document.documentElement.clientWidth - 32;
  cpuprofiler_canvas_size = cpuprofiler_canvas.width * cpuprofiler_canvas.height;
  cpuprofiler_canvas_context = cpuprofiler_canvas.getContext('2d');
  if (document.getElementById('output'))
    document.getElementById('output').style.display = 'none';
  if (document.getElementById('status'))
    document.getElementById('status').style.display = 'none';

  cpuprofiler_histogram = [];
//  setInterval(cpuprofiler_update_ui, CPUPROFILER_UI_UPDATE_INTERVAL);

  cpuprofiler_clear_ui(0, cpuprofiler_canvas.width);
  cpuprofiler_draw_graph_labels();
  cpuprofiler_update_ui();
  Module['preMainLoop'] = cpuprofiler_frameStart;
  Module['postMainLoop'] = cpuprofiler_frameEnd;
  
  cpuprofiler_createSection(0, 'GL', '#FF00FF');
}
/*
function cpuprofiler_setBarColor(msecs, dark) {
  if (msecs <= 17) {
    if (dark) {
      cpuprofiler_canvas_context.fillStyle="#008000";
    } else {
      cpuprofiler_canvas_context.fillStyle="#00FF00";
    }
  } else if (msecs <= 34) {
    if (dark) {
      cpuprofiler_canvas_context.fillStyle="#808000";
    } else {
      cpuprofiler_canvas_context.fillStyle="#FFFF00";
    }
  } else {
    if (dark) {
      cpuprofiler_canvas_context.fillStyle="#800000";
    } else {
      cpuprofiler_canvas_context.fillStyle="#FF0000";
    }
  }
}
*/
// Graphs a range of allocated memory. The memory range will be drawn as a top-to-bottom, left-to-right stripes or columns of pixels.
function cpuprofiler_drawBar(x, msecs, maxTime) {
  cpuprofiler_setBarColor(msecs, false);
  var height = msecs * cpuprofiler_canvas.height / maxTime;
  cpuprofiler_canvas_context.fillRect(x,cpuprofiler_canvas.height-height,1,height);
}

function cpuprofiler_drawStackedBar(x, msecs1, msecs2, maxTime) {
  var height1 = msecs1 * cpuprofiler_canvas.height / maxTime;
  var height2 = msecs2 * cpuprofiler_canvas.height / maxTime;
  var totalHeight = height1+height2;
  //cpuprofiler_setBarColor(msecs1+msecs2, true);
  cpuprofiler_canvas_context.fillStyle="#A0A0A0";
  cpuprofiler_canvas_context.fillRect(x,cpuprofiler_canvas.height-totalHeight,1,height2);
//  cpuprofiler_setBarColor(msecs1+msecs2, false);
  cpuprofiler_canvas_context.fillStyle="#0000BB";
  cpuprofiler_canvas_context.fillRect(x,cpuprofiler_canvas.height-height1,1,height1);
}

function cpuprofiler_drawHorizontalLine(startX, endX, pixelThickness, msecs, maxTime) {
  var height = msecs * cpuprofiler_canvas.height / maxTime;
//  cpuprofiler_canvas_context.fillRect(0,cpuprofiler_canvas.height-height,cpuprofiler_canvas.width,pixelThickness);
  cpuprofiler_canvas_context.fillRect(startX,cpuprofiler_canvas.height-height,endX-startX,pixelThickness);
}

function cpuprofiler_clear_ui(startX, endX) {  
  // Background clear
  cpuprofiler_canvas_context.fillStyle="#324B4B";
//  cpuprofiler_canvas_context.fillRect(0, 0, cpuprofiler_canvas.width, cpuprofiler_canvas.height);
  cpuprofiler_canvas_context.fillRect(startX, 0, endX-startX, cpuprofiler_canvas.height);

  var maxTime = 40.0; // Always show at least 20 msecs.
//  for(var x in cpuprofiler_histogram) {
//    maxTime = Math.max(cpuprofiler_histogram[x], maxTime);
//  }
  
  cpuprofiler_canvas_context.fillStyle="#00FF00";
  cpuprofiler_drawHorizontalLine(startX, endX, 1, 16.6666666, maxTime);
  cpuprofiler_canvas_context.fillStyle="#FFFF00";
  cpuprofiler_drawHorizontalLine(startX, endX, 1, 33.3333333, maxTime);
}

function cpuprofiler_draw_graph_labels() {
  var maxTime = 40.0; // Always show at least 20 msecs.

  cpuprofiler_canvas_context.fillStyle = "#C0C0C0";
  cpuprofiler_canvas_context.font = "bold 10px Arial";
  cpuprofiler_canvas_context.textAlign = "right";
  cpuprofiler_canvas_context.fillText("16.66... ms", cpuprofiler_canvas.width - 3, cpuprofiler_canvas.height - 16.6666*cpuprofiler_canvas.height/maxTime-3);
  cpuprofiler_canvas_context.fillText("33.33... ms", cpuprofiler_canvas.width - 3, cpuprofiler_canvas.height - 33.3333*cpuprofiler_canvas.height/maxTime-3);
}

var maxTime = 40.0; // Always show at least 20 msecs.

function drawBar(x) {
//  cpuprofiler_drawStackedBar(x, cpuprofiler_t_mainloop[x], cpuprofiler_t_outside_mainloop[x], maxTime);
  
  var scale = cpuprofiler_canvas.height / maxTime;
  var y = cpuprofiler_canvas.height;
  var h = cpuprofiler_t_mainloop[x] * scale;
  y -= h;
  cpuprofiler_canvas_context.fillStyle="#0000BB";
  cpuprofiler_canvas_context.fillRect(x,y,1,h);
  for(var i in sections) {
    var sect = sections[i];
    var h = sect.frametimes[x] * scale;
    y -= h;
    cpuprofiler_canvas_context.fillStyle = sect.drawColor;
    cpuprofiler_canvas_context.fillRect(x,y,1,h);
  }
  var h = cpuprofiler_t_outside_mainloop[x] * scale;
  y -= h;
  var fps60Limit = cpuprofiler_canvas.height - (16.666666666 + 1.0) * cpuprofiler_canvas.height / maxTime; // Be very lax, allow 1msec extra jitter.
  var fps30Limit = cpuprofiler_canvas.height - (33.333333333 + 1.0) * cpuprofiler_canvas.height / maxTime; // Be very lax, allow 1msec extra jitter.
  if (y < fps30Limit)
    cpuprofiler_canvas_context.fillStyle="#FF0000";
  else if (y < fps60Limit) 
    cpuprofiler_canvas_context.fillStyle="#FFFF00";
  else
    cpuprofiler_canvas_context.fillStyle="#60A060";
  cpuprofiler_canvas_context.fillRect(x,y,1,h);
}

// Main UI update entry point.
function cpuprofiler_update_ui(startX, endX) {  
//  for(var x in cpuprofiler_histogram) {
//    maxTime = Math.max(cpuprofiler_histogram[x], maxTime);
//  }
  
  var clearDistance = CPUPROFILER_UI_UPDATE_INTERVAL*2+1;
  var clearStart = endX + clearDistance;
  var clearEnd = clearStart + CPUPROFILER_UI_UPDATE_INTERVAL;
  if (endX < startX) {
    cpuprofiler_clear_ui(clearStart, clearEnd);
    cpuprofiler_clear_ui(0, endX+clearDistance+CPUPROFILER_UI_UPDATE_INTERVAL);
    cpuprofiler_draw_graph_labels();
    
  } else
  cpuprofiler_clear_ui(clearStart, clearEnd);

  if (endX < startX) {
    for(var x = startX; x < cpuprofiler_canvas.width; ++x) {
      drawBar(x);
    }
    startX = 0;
  }
  for(var x = startX; x < endX; ++x) {
    drawBar(x);
  }

//  for(var x in cpuprofiler_histogram) {
//    cpuprofiler_drawStackedBar(x, cpuprofiler_t_mainloop[x], cpuprofiler_t_outside_mainloop[x], maxTime);
/*
    var msecs = cpuprofiler_histogram[x];
    if (msecs <= 17) {
      cpuprofiler_canvas_context.fillStyle="#00FF00";
    } else if (msecs <= 34) {
      cpuprofiler_canvas_context.fillStyle="#FFFF00";
    } else {
      cpuprofiler_canvas_context.fillStyle="#FF0000";
    }
    cpuprofiler_drawBar(x, msecs, maxTime);*/
}

cpuprofiler_add_hooks();
