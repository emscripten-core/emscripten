
// Environment setup

var out, err;
out = err = function(x) {
  console.log(x);
};

// Set up memory and table

// Some extra static memory for main() argv strings
var extraStatic, extraStaticSize = 1024;

function setup(info) {
  memory = new WebAssembly.Memory({ initial: info.memorySize, maximum: info.memorySize });
  table = new WebAssembly.Table({ initial: info.tableSize, maximum: info.tableSize, element: 'anyfunc' });
  var staticEnd = info.staticStart + info.staticSize;
  extraStatic = staticEnd;
  var stackStart = extraStatic + extraStaticSize;
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
  // todo main's argc/argv
  function postInstantiate(instance, args) {
    var exports = instance['exports'];
    onload(exports);
    // allocate main() argc/argv
    var extraStaticMax = extraStatic + extraStaticSize;
    function extraAlloc(size) {
      var ret = extraStatic;
      extraStatic += size;
      assert(extraStatic <= extraStaticMax);
      return ret;
    }
    function writeCString(ptr, string) {
      for (var j = 0; j < string.length; j++) {
        HEAPU8[ptr + j] = string.charCodeAt(j);
      }
      HEAPU8[ptr + string.length] = 0;
    }
    function allocCString(string) {
      var ptr = extraAlloc(string.length + 1);
      writeCString(ptr, string);
      return ptr;
    }
    var argc = args.length + 1;
    var argv = extraAlloc(argc * 4);
    HEAP32[argv >> 2] = allocCString('program');
    for (var i = 0; i < args.length; i++) {
      HEAP32[(argv >> 2) + 1 + i] = allocCString(args[i]);
    }
    exports['_main'](argc, argv);
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

