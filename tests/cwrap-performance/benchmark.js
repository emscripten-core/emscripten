(function(){

if (typeof require !== "undefined") {
  var Module = require("./add.js");
}

var performance = window.performance, Module = window.Module;
if (typeof window === 'undefined' || window.performance === undefined) {
  var polyfill = (typeof process !== 'undefined') ? (function() {
		var t = process['hrtime']();
		return t[0]*1e3 + t[1]/1e6; // result in ms
	}) : (function() {
	  return Date.now();
	});
	var performance = {
		now : polyfill
	}
}

if (typeof assert === 'undefined') {
  function assert (condition, msg) {
    if (!condition) throw "Assertion failed : " + msg;
  }
}

// wrap add
var t = performance.now();
for (var i=0; i<100; i++) var c_add = Module['cwrap']('add', 'number', ['number', 'number']);
var diff = performance.now() - t;
console.log('Time to wrap "add" 100 times: ' + diff + 'ms');

// wrap concat
var t = performance.now();
for (var i=0; i<100; i++) var c_concat = Module['cwrap']('concat', 'string', ['string', 'string']);
var diff = performance.now() - t;
console.log('Time to wrap "concat" 100 times: ' + diff + 'ms');

console.log('\n');

// execute add
t = performance.now();
for (var i=0; i<1e4; i++) {
	var res = Module['ccall']('add', 'number', ['number', 'number'], [1,3]);
}
diff = performance.now() - t;
assert(res == 4, 'add returns correct result. Expecting 4, got '+res);
console.log('Time to execute ccall on "add" 10,000 times: ' + diff + 'ms');

// execute concat
t = performance.now();
for (var i=0; i<1e4; i++) {
	var res = Module['ccall']('concat', 'string', ['string', 'string'], ["A", 'B']);
}
diff = performance.now() - t;
assert(res == "AB", 'concat returns correct result');
console.log('Time to execute ccall on "concat" 10,000 times: ' + diff + 'ms');


console.log('\n');


// execute add
t = performance.now();
for (var i=0; i<1e4; i++) {
	var res = c_add(1,3);
}
diff = performance.now() - t;
assert(res == 4, 'add returns correct result. Expecting 4, got '+res);
console.log('Time to execute "add" 10,000 times: ' + diff + 'ms');

// execute concat
t = performance.now();
for (var i=0; i<1e4; i++) {
	var res = c_concat("A","B");
}
diff = performance.now() - t;
assert(res == "AB", 'concat returns correct result. Expecting "AB", got '+JSON.stringify(res)+'.');
console.log('Time to execute "concat" 10,000 times: ' + diff + 'ms');

})();
