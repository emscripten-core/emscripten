// LLVM => JavaScript compiler, main entry point

try {
  // On SpiderMonkey, prepare a large amount of GC space
  gcparam('maxBytes', 1024*1024*1024);
} catch(e) {}

// Prep - allow this to run in both SpiderMonkey and V8
if (!this['load']) {
  load = function(f) { eval(snarf(f)) };
}
if (!this['read']) {
  read = function(f) { snarf(f) };
}

// Basic utilities

load('utility.js');

// Load settings, can be overridden by commandline

load('settings.js');

var settings = JSON.parse(readline());
for (setting in settings) {
  this[setting] = settings[setting];
}
var CONSTANTS = { 'QUANTUM_SIZE': QUANTUM_SIZE };

if (CORRECT_SIGNS >= 2) {
  CORRECT_SIGNS_LINES = set(CORRECT_SIGNS_LINES); // for fast checking
}
if (CORRECT_OVERFLOWS >= 2) {
  CORRECT_OVERFLOWS_LINES = set(CORRECT_OVERFLOWS_LINES); // for fast checking
}
if (CORRECT_ROUNDINGS >= 2) {
  CORRECT_ROUNDINGS_LINES = set(CORRECT_ROUNDINGS_LINES); // for fast checking
}
if (SAFE_HEAP >= 2) {
  SAFE_HEAP_LINES = set(SAFE_HEAP_LINES); // for fast checking
}

EXPORTED_FUNCTIONS = set(EXPORTED_FUNCTIONS);

// Load compiler code

load('framework.js');
load('modules.js');
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

var inter = intertyper(lines);

// Load library, with preprocessing and macros. Must be done after intertyper, so we know if we have debug info or not
for (suffix in set('', '_sdl', '_gl', '_browser')) {
  eval(processMacros(preprocess(read('library' + suffix + '.js'), CONSTANTS)));
}
LibraryManager.init();

JSify(analyzer(inter));

