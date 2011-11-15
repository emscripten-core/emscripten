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
if (!this['arguments']) {
  arguments = scriptArgs;
}

// Basic utilities

load('utility.js');

// Load settings, can be overridden by commandline

load('settings.js');

var settings_file = arguments[0];
var ll_file = arguments[1];

var settings = JSON.parse(read(settings_file));
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
EXPORTED_GLOBALS = set(EXPORTED_GLOBALS);

// Settings sanity checks

assert(!(USE_TYPED_ARRAYS === 2 && QUANTUM_SIZE !== 4), 'For USE_TYPED_ARRAYS == 2, must have normal QUANTUM_SIZE of 4');

// Output some info and warnings based on settings

if (!OPTIMIZE || !RELOOP || ASSERTIONS || CHECK_SIGNS || CHECK_OVERFLOWS || INIT_STACK || INIT_HEAP ||
    !SKIP_STACK_IN_SMALL || SAFE_HEAP || AUTO_OPTIMIZE || PROFILE || !DISABLE_EXCEPTION_CATCHING) {
  print('// Note: Some Emscripten settings will significantly limit the speed of the generated code.');
} else {
  print('// Note: For maximum-speed code, it is recommended to run the emscripten eliminator tool and/or closure compiler.');
}

if (CORRECT_SIGNS || CORRECT_OVERFLOWS || CORRECT_ROUNDINGS) {
  print('// Note: Some Emscripten settings may limit the speed of the generated code.');
}

// Load compiler code

load('framework.js');
load('modules.js');
load('parseTools.js');
load('intertyper.js');
load('analyzer.js');
load('jsifier.js');
eval(processMacros(preprocess(read('runtime.js'))));

//===============================
// Main
//===============================

// Read llvm

var raw = read(ll_file);
if (FAKE_X86_FP80) {
  raw = raw.replace(/x86_fp80/g, 'double');
}
var lines = raw.split('\n');
raw = null;

// Do it

//dprint(JSON.stringify(C_DEFINES));

JSify(analyzer(intertyper(lines)));

