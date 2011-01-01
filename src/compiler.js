// LLVM => JavaScript compiler, main entry point

// Prep - allow this to run in both SpiderMonkey and V8
if (!this['load']) {
  load = function(f) { eval(snarf(f)) };
}
if (!this['read']) {
  read = function(f) { snarf(f) };
}

// Load settings, can be overridden by commandline

load('settings.js');

var settings = JSON.parse(readline());
for (setting in settings) {
  this[setting] = settings[setting];
}
var CONSTANTS = { 'QUANTUM_SIZE': QUANTUM_SIZE };

// Load compiler code

load('utility.js');
load('framework.js');
load('parseTools.js');
load('intertyper.js');
load('analyzer.js');
load('jsifier.js');
load('runtime.js');

//===============================
// Main
//===============================

// Read llvm

var lines = [];
var line;
do {
  line = readline();
  if (line == null) break;
  lines.push(line);
} while(true);

// Do it

print(JSify(analyzer(intertyper(lines))));

