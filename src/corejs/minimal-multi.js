
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
  // todo main's argc/argv
  function postInstantiate(instance, args) {
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
    var argc = 1, argv;
    if (args && args.length) { // TODO
      argc = args.length + 1;
      argv = stackAlloc(argc * 4);
      HEAP32[argv >> 2] = 0; // no program name XXX
      for (var i = 0; i < args.length; i++) {
        var arg = args[i];
        var ptr = stackAlloc(arg.length + 1);
        HEAP32[(argv >> 2) + 1 + i] = ptr;
        for (var j = 0; j < arg.length; j++) {
          HEAPU8[ptr + j] = arg.charCodeAt(j);
        }
        HEAPU8[ptr + arg.length] = 0;
      }
    }
    main(argc, argv);
  }
  var filename = '{{{ WASM_BINARY_FILE }}}';
  if (typeof fetch === 'function') {
    // Web
    fetch(filename, { credentials: 'same-origin' })
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
  } else {
    var data, args;
    if (typeof require === 'function') {
      // node.js
      data = require('fs')['readFileSync'](filename);
      args = process['argv'].slice(2);
    } else if (typeof read === 'function') {
      // SpiderMonkey shell
      data = read(filename, 'binary');
      args = scriptArgs;
    } else {
      throw Error('where am i');
    }
    var instance = new WebAssembly.Instance(new WebAssembly.Module(data), imports);
    postInstantiate(instance, args);
  }
}

