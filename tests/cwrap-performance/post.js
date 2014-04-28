if (typeof process !== "undefined") {
	performance = {
		now : function() {
			var t = process.hrtime();
			return t[0]*1e3 + t[1]/1e6; // result in ms
		}
	}
}

// wrap add
var t = performance.now();
var c_add = Module.cwrap('add', 'number', ['number', 'number']);
var diff = performance.now() - t;
console.log('Time to wrap "add": ' + diff + 'ms');

// wrap concat
var t = performance.now();
var c_concat = Module.cwrap('concat', 'number', ['string', 'string', 'number']);
var diff = performance.now() - t;
console.log('Time to wrap "concat": ' + diff + 'ms');

console.log('\n');

// execute add
t = performance.now();
for (var i=0; i<1e5; i++) {
	var res = Module.ccall('add', 'number', ['number', 'number'], [1,3]);
}
diff = performance.now() - t;
assert(res == 4, 'add returns correct result. Expecting 4, got '+res);
console.log('Time to execute ccall on "add" 10,000 times: ' + diff + 'ms');

// execute concat
var strPtr = Runtime.stackAlloc(20);
t = performance.now();
for (var i=0; i<1e5; i++) {
	Module.ccall('concat', 'number', ['string', 'string', 'number'], ["A", 'B', strPtr]);
}
diff = performance.now() - t;
var res = Pointer_stringify(strPtr);
assert(res == "AB", 'concat returns correct result');
console.log('Time to execute ccall on "concat" 10,000 times: ' + diff + 'ms');


console.log('\n');


// execute add
t = performance.now();
for (var i=0; i<1e5; i++) {
	var res = c_add(1,3);
}
diff = performance.now() - t;
assert(res == 4, 'add returns correct result. Expecting 4, got '+res);
console.log('Time to execute "add" 10,000 times: ' + diff + 'ms');

// execute concat
var strPtr = Runtime.stackAlloc(20);
t = performance.now();
for (var i=0; i<1e5; i++) {
	c_concat("A","B", strPtr);
}
diff = performance.now() - t;
var res = Pointer_stringify(strPtr);
assert(res == "AB", 'concat returns correct result');
console.log('Time to execute "concat" 10,000 times: ' + diff + 'ms');
