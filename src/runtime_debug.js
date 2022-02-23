/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if ASSERTIONS
function legacyModuleProp(prop, newName) {
  if (!Object.getOwnPropertyDescriptor(Module, prop)) {
    Object.defineProperty(Module, prop, {
      configurable: true,
      get: function() {
        abort('Module.' + prop + ' has been replaced with plain ' + newName + ' (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)');
      }
    });
  }
}

function ignoredModuleProp(prop) {
  if (Object.getOwnPropertyDescriptor(Module, prop)) {
    abort('`Module.' + prop + '` was supplied but `' + prop + '` not included in INCOMING_MODULE_JS_API');
  }
}

function unexportedMessage(sym, isFSSybol) {
  var msg = "'" + sym + "' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)";
  if (isFSSybol) {
    msg += '. Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you';
  }
  return msg;
}

function unexportedRuntimeSymbol(sym, isFSSybol) {
  if (!Object.getOwnPropertyDescriptor(Module, sym)) {
    Object.defineProperty(Module, sym, {
      configurable: true,
      get: function() {
        abort(unexportedMessage(sym, isFSSybol));
      }
    });
  }
}

function unexportedRuntimeFunction(sym, isFSSybol) {
  if (!Object.getOwnPropertyDescriptor(Module, sym)) {
    Module[sym] = () => abort(unexportedMessage(sym, isFSSybol));
  }
}
#endif

#if RUNTIME_DEBUG
var runtimeDebug = true; // Switch to false at runtime to disable logging at the right times

var printObjectList = [];

function prettyPrint(arg) {
  if (typeof arg == 'undefined') return '!UNDEFINED!';
  if (typeof arg == 'boolean') arg = arg + 0;
  if (!arg) return arg;
  var index = printObjectList.indexOf(arg);
  if (index >= 0) return '<' + arg + '|' + index + '>';
  if (arg.toString() == '[object HTMLImageElement]') {
    return arg + '\n\n';
  }
  if (arg.byteLength) {
    return '{' + Array.prototype.slice.call(arg, 0, Math.min(arg.length, 400)) + '}'; // Useful for correct arrays, less so for compiled arrays, see the code below for that
    var buf = new ArrayBuffer(32);
    var i8buf = new Int8Array(buf);
    var i16buf = new Int16Array(buf);
    var f32buf = new Float32Array(buf);
    switch (arg.toString()) {
      case '[object Uint8Array]':
        i8buf.set(arg.subarray(0, 32));
        break;
      case '[object Float32Array]':
        f32buf.set(arg.subarray(0, 5));
        break;
      case '[object Uint16Array]':
        i16buf.set(arg.subarray(0, 16));
        break;
      default:
        alert('unknown array for debugging: ' + arg);
        throw 'see alert';
    }
    var ret = '{' + arg.byteLength + ':\n';
    var arr = Array.prototype.slice.call(i8buf);
    ret += 'i8:' + arr.toString().replace(/,/g, ',') + '\n';
    arr = Array.prototype.slice.call(f32buf, 0, 8);
    ret += 'f32:' + arr.toString().replace(/,/g, ',') + '}';
    return ret;
  }
  if (typeof arg == 'function') {
    return '<function>';
  } else if (typeof arg == 'object') {
    printObjectList.push(arg);
    return '<' + arg + '|' + (printObjectList.length-1) + '>';
  } else if (typeof arg == 'number') {
    if (arg > 0) return '0x' + arg.toString(16) + ' (' + arg + ')';
  }
  return arg;
}
#endif
