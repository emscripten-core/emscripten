
// Environment setup

var out, err;
out = err = function(x) {
  console.log(x);
};

// Set up memory and table

function setup(info) {
  memory = new WebAssembly.Memory({ initial: info.memorySize, maximum: info.memorySize });
  table = new WebAssembly.Table({ initial: info.tableSize, maximum: info.tableSize, element: 'anyfunc' });
  var staticEnd = info.staticStart + info.staticSize;
  var stackStart = staticEnd;
  var stackMax = stackStart + info.stackSize;
  var sbrkStart = stackMax;
  var sbrkPtr = 16;
  (new Int32Array(memory.buffer))[sbrkPtr >> 2] = sbrkStart;
  return {
    memory: memory,
    table: table,
    stackStart: stackStart,
    sbrkStart: sbrkStart,
    sbrkPtr: sbrkPtr,
  };
}

// Compile and run

function start(imports, onload) {
  fetch('b.wasm', { credentials: 'same-origin' })
    .then(function(response) {
      return response.arrayBuffer();
    })
    .then(function(arrayBuffer) {
      return new Uint8Array(arrayBuffer);
    })
    .then(function(binary) {
      return WebAssembly.instantiate(binary, imports);
    })
    .then(function(pair) {
      var instance = pair['instance'];
      var exports = instance['exports'];
      onload(exports);
      exports['_main']();
    });
}

