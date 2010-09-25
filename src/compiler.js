// LLVM => JavaScript compiler, main entry point

// Prep - allow this to run in both SpiderMonkey and V8
if (!this['load']) {
  load = function(f) { eval(snarf(f)) }
}
if (!this['read']) {
  read = function(f) { snarf(f) }
}

load('settings.js');

load('utility.js');
load('enzymatic.js');
load('library.js');
load('parseTools.js');
load('intertyper.js');
load('analyzer.js');
load('jsifier.js');

//===============================
// Main
//===============================

// Override settings.js
var settings = JSON.parse(readline());
for (setting in settings) {
  this[setting] = settings[setting];
}

// Read llvm
var lines = [];
var line;
do {
  line = readline();
  if (line == null) break;
  lines.push(line);
} while(true);
var data = lines.join("\n");

// Do it
print(JSify(analyzer(intertyper(data))));

