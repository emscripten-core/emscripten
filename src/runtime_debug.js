/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if TRACING
var trace_channels = {
  'RUNTIME': 1,
  'EXCEPTION': 1,
  'LIBRARY': 1,
  'SYSCALL': 1,
  'SOCKET': 1,
  'DYLINK': 1,
  'FS': 1,
  'OPENAL': 1,
  'WEBSOCKET': 1,
  'GL': 1,
  'ASYNCIFY': 1,
  'PTHREADS': 1,
  'FETCH': 1,
};

var trace_channels_enabled = {
  // The core RUNTIME channel is the only one that is
  // enabled by default.
  'RUNTIME': true,
#if EXCEPTION_DEBUG
  'EXCEPTION': true,
#endif
#if LIBRARY_DEBUG
  'LIBRARY': true,
#endif
#if SYSCALL_DEBUG
  'SYSCALL': true,
#endif
#if SOCKET_DEBUG
  'SOCKET': true,
#endif
#if DYLINK_DEBUG
  'DYLINK': true,
#endif
#if FS_DEBUG
  'FS': true,
#endif
#if OPENAL_DEBUG
  'OPENAL': true,
#endif
#if WEBSOCKET_DEBUG
  'WEBSOCKET_DEBUG': true,
#endif
#if GL_DEBUG
  'GL': true,
#endif
#if ASYNCIFY_DEBUG
  'ASYNCIFY': true;
#endif
#if PTHREADS_DEBUG
  'PTHREADS': true,
#endif
#if FETCH_DEBUG
  'FETCH': true,
#endif
};

function enable_trace_channel(channel) {
#if ASSERTIONS
  assert(channel in trace_channels, 'unknown trace channel: ' + channel);
#endif
  err('enabling trace channel: ' + channel);
  trace_channels_enabled[channel] = true;
}

function trace_channel_enabled(channel) {
  return trace_channels_enabled[channel] === true;
}

function trace(channel, msg) {
#if ASSERTIONS
  assert(channel in trace_channels, 'unknown trace channel: ' + channel);
#endif
  if (trace_channels_enabled[channel]) {
    err(channel + ': ' + msg);
  }
}

#if ENVIRONMENT_MAY_BE_NODE
if (ENVIRONMENT_IS_NODE && process.env['EM_DEBUG']) {
  if (process.env['EM_DEBUG'] == 'all') {
    for (var channel in trace_channels) {
      trace_channels_enabled[channel] = true;
    }
  } else {
    var channels = process.env['EM_DEBUG'].split(',');
    for (var i in channels) {
      enable_trace_channel(channels[i].toUpperCase());
    }
  }
}
#endif
#endif

#if LIBRARY_DEBUG
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
  if (typeof arg == 'object') {
    printObjectList.push(arg);
    return '<' + arg + '|' + (printObjectList.length-1) + '>';
  }
  if (typeof arg == 'number') {
    if (arg > 0) return '0x' + arg.toString(16) + ' (' + arg + ')';
  }
  return arg;
}
#endif
