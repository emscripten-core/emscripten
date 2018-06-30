
// Environment setup

var out, err;
out = err = function(x) {
  console.log(x);
};

// Set up memory and table

var memory, table;

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

function start(imports, ctors, jsCtors) {
  function postInstantiate(instance) {
    var exports = instance['exports'];
    ctors.forEach(function(ctor) {
      exports[ctor]();
    });
    jsCtors.forEach(function(ctor) {
      if (typeof ctor === 'function') { // XXX FIXME
        ctor();
      }
    });
    var main = exports['_main'];
    main();
  }
  var where = 'mozjs';
  if (where === 'web') {
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
        postInstantiate(pair['instance']);
      });
  } else if (where === 'mozjs') {
    var data = read('b.wasm', 'binary');
    var instance = new WebAssembly.Instance(new WebAssembly.Module(data), imports);
    postInstantiate(instance);
  }
}

