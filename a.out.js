var a;
a || (a = typeof Module !== 'undefined' ? Module : {});
var c = {}, l;
for (l in a) {
  a.hasOwnProperty(l) && (c[l] = a[l]);
}
var m = [], n = "./this.program";
function r(b, d) {
  throw d;
}
var t = !1, u = !1, v = !1, aa = !1, ba = !1;
t = "object" === typeof window;
u = "function" === typeof importScripts;
v = (aa = "object" === typeof process && "object" === typeof process.versions && "string" === typeof process.versions.node) && !t && !u;
ba = !t && !v && !u;
if (a.ENVIRONMENT) {
  throw Error("Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -s ENVIRONMENT=web or -s ENVIRONMENT=node)");
}
var w = "", x, y, z, A;
if (v) {
  w = __dirname + "/", x = function(b, d) {
    z || (z = require("fs"));
    A || (A = require("path"));
    b = A.normalize(b);
    return z.readFileSync(b, d ? null : "utf8");
  }, y = function(b) {
    b = x(b, !0);
    b.buffer || (b = new Uint8Array(b));
    assert(b.buffer);
    return b;
  }, 1 < process.argv.length && (n = process.argv[1].replace(/\\/g, "/")), m = process.argv.slice(2), "undefined" !== typeof module && (module.exports = a), process.on("uncaughtException", function(b) {
    if (!(b instanceof B)) {
      throw b;
    }
  }), process.on("unhandledRejection", C), r = function(b) {
    process.exit(b);
  }, a.inspect = function() {
    return "[Emscripten Module object]";
  };
} else {
  if (ba) {
    "undefined" != typeof read && (x = function(b) {
      return read(b);
    }), y = function(b) {
      if ("function" === typeof readbuffer) {
        return new Uint8Array(readbuffer(b));
      }
      b = read(b, "binary");
      assert("object" === typeof b);
      return b;
    }, "undefined" != typeof scriptArgs ? m = scriptArgs : "undefined" != typeof arguments && (m = arguments), "function" === typeof quit && (r = function(b) {
      quit(b);
    }), "undefined" !== typeof print && ("undefined" === typeof console && (console = {}), console.log = print, console.warn = console.error = "undefined" !== typeof printErr ? printErr : print);
  } else {
    if (t || u) {
      u ? w = self.location.href : document.currentScript && (w = document.currentScript.src), w = 0 !== w.indexOf("blob:") ? w.substr(0, w.lastIndexOf("/") + 1) : "", x = function(b) {
        var d = new XMLHttpRequest;
        d.open("GET", b, !1);
        d.send(null);
        return d.responseText;
      }, u && (y = function(b) {
        var d = new XMLHttpRequest;
        d.open("GET", b, !1);
        d.responseType = "arraybuffer";
        d.send(null);
        return new Uint8Array(d.response);
      });
    } else {
      throw Error("environment detection error");
    }
  }
}
var D = a.print || console.log.bind(console), E = a.printErr || console.warn.bind(console);
for (l in c) {
  c.hasOwnProperty(l) && (a[l] = c[l]);
}
c = null;
a.arguments && (m = a.arguments);
Object.getOwnPropertyDescriptor(a, "arguments") || Object.defineProperty(a, "arguments", {configurable:!0, get:function() {
  C("Module.arguments has been replaced with plain arguments_");
}});
a.thisProgram && (n = a.thisProgram);
Object.getOwnPropertyDescriptor(a, "thisProgram") || Object.defineProperty(a, "thisProgram", {configurable:!0, get:function() {
  C("Module.thisProgram has been replaced with plain thisProgram");
}});
a.quit && (r = a.quit);
Object.getOwnPropertyDescriptor(a, "quit") || Object.defineProperty(a, "quit", {configurable:!0, get:function() {
  C("Module.quit has been replaced with plain quit_");
}});
assert("undefined" === typeof a.memoryInitializerPrefixURL, "Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead");
assert("undefined" === typeof a.pthreadMainPrefixURL, "Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead");
assert("undefined" === typeof a.cdInitializerPrefixURL, "Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead");
assert("undefined" === typeof a.filePackagePrefixURL, "Module.filePackagePrefixURL option was removed, use Module.locateFile instead");
assert("undefined" === typeof a.read, "Module.read option was removed (modify read_ in JS)");
assert("undefined" === typeof a.readAsync, "Module.readAsync option was removed (modify readAsync in JS)");
assert("undefined" === typeof a.readBinary, "Module.readBinary option was removed (modify readBinary in JS)");
assert("undefined" === typeof a.setWindowTitle, "Module.setWindowTitle option was removed (modify setWindowTitle in JS)");
Object.getOwnPropertyDescriptor(a, "read") || Object.defineProperty(a, "read", {configurable:!0, get:function() {
  C("Module.read has been replaced with plain read_");
}});
Object.getOwnPropertyDescriptor(a, "readAsync") || Object.defineProperty(a, "readAsync", {configurable:!0, get:function() {
  C("Module.readAsync has been replaced with plain readAsync");
}});
Object.getOwnPropertyDescriptor(a, "readBinary") || Object.defineProperty(a, "readBinary", {configurable:!0, get:function() {
  C("Module.readBinary has been replaced with plain readBinary");
}});
ca = da = function() {
  C("cannot use the stack before compiled code is ready to run, and has provided stack access");
};
function F(b) {
  G || (G = {});
  G[b] || (G[b] = 1, E(b));
}
var G, H;
a.wasmBinary && (H = a.wasmBinary);
Object.getOwnPropertyDescriptor(a, "wasmBinary") || Object.defineProperty(a, "wasmBinary", {configurable:!0, get:function() {
  C("Module.wasmBinary has been replaced with plain wasmBinary");
}});
var I;
a.noExitRuntime && (I = a.noExitRuntime);
Object.getOwnPropertyDescriptor(a, "noExitRuntime") || Object.defineProperty(a, "noExitRuntime", {configurable:!0, get:function() {
  C("Module.noExitRuntime has been replaced with plain noExitRuntime");
}});
"object" !== typeof WebAssembly && C("No WebAssembly support found. Build with -s WASM=0 to target JavaScript instead.");
var J, ea = new WebAssembly.Table({initial:6, maximum:6, element:"anyfunc"}), K = !1;
function assert(b, d) {
  b || C("Assertion failed: " + d);
}
var fa = "undefined" !== typeof TextDecoder ? new TextDecoder("utf8") : void 0;
"undefined" !== typeof TextDecoder && new TextDecoder("utf-16le");
function ha(b) {
  for (var d = 0, e = 0; e < b.length; ++e) {
    var h = b.charCodeAt(e);
    55296 <= h && 57343 >= h && (h = 65536 + ((h & 1023) << 10) | b.charCodeAt(++e) & 1023);
    127 >= h ? ++d : d = 2047 >= h ? d + 2 : 65535 >= h ? d + 3 : d + 4;
  }
  var k = d + 1;
  d = da(k);
  e = d;
  h = ia;
  if (0 < k) {
    k = e + k - 1;
    for (var f = 0; f < b.length; ++f) {
      var g = b.charCodeAt(f);
      if (55296 <= g && 57343 >= g) {
        var p = b.charCodeAt(++f);
        g = 65536 + ((g & 1023) << 10) | p & 1023;
      }
      if (127 >= g) {
        if (e >= k) {
          break;
        }
        h[e++] = g;
      } else {
        if (2047 >= g) {
          if (e + 1 >= k) {
            break;
          }
          h[e++] = 192 | g >> 6;
        } else {
          if (65535 >= g) {
            if (e + 2 >= k) {
              break;
            }
            h[e++] = 224 | g >> 12;
          } else {
            if (e + 3 >= k) {
              break;
            }
            2097152 <= g && F("Invalid Unicode code point 0x" + g.toString(16) + " encountered when serializing a JS string to an UTF-8 string on the asm.js/wasm heap! (Valid unicode code points should be in range 0-0x1FFFFF).");
            h[e++] = 240 | g >> 18;
            h[e++] = 128 | g >> 12 & 63;
          }
          h[e++] = 128 | g >> 6 & 63;
        }
        h[e++] = 128 | g & 63;
      }
    }
    h[e] = 0;
  }
  return d;
}
var buffer, ia, L, M, N;
assert(!0, "stack must start aligned");
assert(!0, "heap must start aligned");
a.TOTAL_STACK && assert(5242880 === a.TOTAL_STACK, "the stack size can no longer be determined at runtime");
var O = a.TOTAL_MEMORY || 16777216;
Object.getOwnPropertyDescriptor(a, "TOTAL_MEMORY") || Object.defineProperty(a, "TOTAL_MEMORY", {configurable:!0, get:function() {
  C("Module.TOTAL_MEMORY has been replaced with plain INITIAL_TOTAL_MEMORY");
}});
assert(5242880 <= O, "TOTAL_MEMORY should be larger than TOTAL_STACK, was " + O + "! (TOTAL_STACK=5242880)");
assert("undefined" !== typeof Int32Array && "undefined" !== typeof Float64Array && void 0 !== Int32Array.prototype.subarray && void 0 !== Int32Array.prototype.set, "JS engine does not provide full typed array support");
a.wasmMemory ? J = a.wasmMemory : J = new WebAssembly.Memory({initial:O / 65536, maximum:O / 65536});
J && (buffer = J.buffer);
O = buffer.byteLength;
assert(0 === O % 65536);
var P = buffer;
buffer = P;
a.HEAP8 = ia = new Int8Array(P);
a.HEAP16 = new Int16Array(P);
a.HEAP32 = M = new Int32Array(P);
a.HEAPU8 = L = new Uint8Array(P);
a.HEAPU16 = new Uint16Array(P);
a.HEAPU32 = N = new Uint32Array(P);
a.HEAPF32 = new Float32Array(P);
a.HEAPF64 = new Float64Array(P);
M[896] = 5246624;
function ja() {
  assert(!0);
  N[937] = 34821223;
  N[938] = 2310721022;
  M[0] = 1668509029;
}
function Q() {
  var b = N[937], d = N[938];
  34821223 == b && 2310721022 == d || C("Stack overflow! Stack cookie has been overwritten, expected hex dwords 0x89BACDFE and 0x2135467, but received 0x" + d.toString(16) + " " + b.toString(16));
  1668509029 !== M[0] && C("Runtime error: The application has corrupted its heap memory area (address zero)!");
}
var ka = new Int16Array(1), la = new Int8Array(ka.buffer);
ka[0] = 25459;
if (115 !== la[0] || 99 !== la[1]) {
  throw "Runtime error: expected the system to be little-endian!";
}
function R(b) {
  for (; 0 < b.length;) {
    var d = b.shift();
    if ("function" == typeof d) {
      d();
    } else {
      var e = d.b;
      "number" === typeof e ? void 0 === d.a ? a.dynCall_v(e) : a.dynCall_vi(e, d.a) : e(void 0 === d.a ? null : d.a);
    }
  }
}
var ma = [], na = [], oa = [], pa = [], S = !1, T = !1;
function qa() {
  var b = a.preRun.shift();
  ma.unshift(b);
}
assert(Math.imul, "This browser does not support Math.imul(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");
assert(Math.fround, "This browser does not support Math.fround(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");
assert(Math.clz32, "This browser does not support Math.clz32(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");
assert(Math.trunc, "This browser does not support Math.trunc(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");
var U = 0, V = null, W = null, X = {};
function ra() {
  U++;
  a.monitorRunDependencies && a.monitorRunDependencies(U);
  assert(!X["wasm-instantiate"]);
  X["wasm-instantiate"] = 1;
  null === V && "undefined" !== typeof setInterval && (V = setInterval(function() {
    if (K) {
      clearInterval(V), V = null;
    } else {
      var b = !1, d;
      for (d in X) {
        b || (b = !0, E("still waiting on run dependencies:")), E("dependency: " + d);
      }
      b && E("(end of list)");
    }
  }, 10000));
}
a.preloadedImages = {};
a.preloadedAudios = {};
function C(b) {
  if (a.onAbort) {
    a.onAbort(b);
  }
  D(b);
  E(b);
  K = !0;
  b = "abort(" + b + ") at ";
  a: {
    var d = Error();
    if (!d.stack) {
      try {
        throw Error();
      } catch (e) {
        d = e;
      }
      if (!d.stack) {
        d = "(no stack trace available)";
        break a;
      }
    }
    d = d.stack.toString();
  }
  a.extraStackTrace && (d += "\n" + a.extraStackTrace());
  d = sa(d);
  throw new WebAssembly.RuntimeError(b + d);
}
function ta() {
  C("Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with  -s FORCE_FILESYSTEM=1");
}
a.FS_createDataFile = function() {
  ta();
};
a.FS_createPreloadedFile = function() {
  ta();
};
function ua() {
  var b = Y;
  return String.prototype.startsWith ? b.startsWith("data:application/octet-stream;base64,") : 0 === b.indexOf("data:application/octet-stream;base64,");
}
var Y = "a.out.wasm";
if (!ua()) {
  var wa = Y;
  Y = a.locateFile ? a.locateFile(wa, w) : w + wa;
}
function xa() {
  try {
    if (H) {
      return new Uint8Array(H);
    }
    if (y) {
      return y(Y);
    }
    throw "both async and sync fetching of the wasm failed";
  } catch (b) {
    C(b);
  }
}
function ya() {
  return H || !t && !u || "function" !== typeof fetch ? new Promise(function(b) {
    b(xa());
  }) : fetch(Y, {credentials:"same-origin"}).then(function(b) {
    if (!b.ok) {
      throw "failed to load wasm binary file at '" + Y + "'";
    }
    return b.arrayBuffer();
  }).catch(function() {
    return xa();
  });
}
na.push({b:function() {
  za();
}});
function sa(b) {
  return b.replace(/\b_Z[\w\d_]+/g, function(d) {
    F("warning: build with  -s DEMANGLE_SUPPORT=1  to link in libcxxabi demangling");
    return d === d ? d : d + " [" + d + "]";
  });
}
function Aa() {
  var b = a._fflush;
  b && b(0);
  Ba[1].length && Ca(1, 10);
  Ba[2].length && Ca(2, 10);
}
var Ba = [null, [], []];
function Ca(b, d) {
  var e = Ba[b];
  assert(e);
  if (0 === d || 10 === d) {
    b = 1 === b ? D : E;
    d = 0;
    for (var h = d + NaN, k = d; e[k] && !(k >= h);) {
      ++k;
    }
    if (16 < k - d && e.subarray && fa) {
      d = fa.decode(e.subarray(d, k));
    } else {
      for (h = ""; d < k;) {
        var f = e[d++];
        if (f & 128) {
          var g = e[d++] & 63;
          if (192 == (f & 224)) {
            h += String.fromCharCode((f & 31) << 6 | g);
          } else {
            var p = e[d++] & 63;
            224 == (f & 240) ? f = (f & 15) << 12 | g << 6 | p : (240 != (f & 248) && F("Invalid UTF-8 leading byte 0x" + f.toString(16) + " encountered when deserializing a UTF-8 string on the asm.js/wasm heap to a JS string!"), f = (f & 7) << 18 | g << 12 | p << 6 | e[d++] & 63);
            65536 > f ? h += String.fromCharCode(f) : (f -= 65536, h += String.fromCharCode(55296 | f >> 10, 56320 | f & 1023));
          }
        } else {
          h += String.fromCharCode(f);
        }
      }
      d = h;
    }
    b(d);
    e.length = 0;
  } else {
    e.push(d);
  }
}
var Da = {__handle_stack_overflow:function() {
  C("stack overflow");
}, __lock:function() {
}, __unlock:function() {
}, emscripten_get_sbrk_ptr:function() {
  return 3584;
}, emscripten_memcpy_big:function(b, d, e) {
  L.set(L.subarray(d, d + e), b);
}, emscripten_resize_heap:function(b) {
  C("Cannot enlarge memory arrays to size " + b + " bytes (OOM). Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value " + ia.length + ", (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ");
}, fd_write:function(b, d, e, h) {
  try {
    for (var k = 0, f = 0; f < e; f++) {
      for (var g = M[d + 8 * f >> 2], p = M[d + (8 * f + 4) >> 2], q = 0; q < p; q++) {
        Ca(b, L[g + q]);
      }
      k += p;
    }
    M[h >> 2] = k;
    return 0;
  } catch (va) {
    return C(va), va.c;
  }
}, memory:J, setTempRet0:function() {
}, table:ea}, Ea = function() {
  function b(f) {
    a.asm = f.exports;
    U--;
    a.monitorRunDependencies && a.monitorRunDependencies(U);
    assert(X["wasm-instantiate"]);
    delete X["wasm-instantiate"];
    0 == U && (null !== V && (clearInterval(V), V = null), W && (f = W, W = null, f()));
  }
  function d(f) {
    assert(a === k, "the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?");
    k = null;
    b(f.instance);
  }
  function e(f) {
    return ya().then(function(g) {
      return WebAssembly.instantiate(g, h);
    }).then(f, function(g) {
      E("failed to asynchronously prepare wasm: " + g);
      C(g);
    });
  }
  var h = {env:Da, wasi_snapshot_preview1:Da};
  ra();
  var k = a;
  if (a.instantiateWasm) {
    try {
      return a.instantiateWasm(h, b);
    } catch (f) {
      return E("Module.instantiateWasm callback failed with error: " + f), !1;
    }
  }
  (function() {
    if (H || "function" !== typeof WebAssembly.instantiateStreaming || ua() || "function" !== typeof fetch) {
      return e(d);
    }
    fetch(Y, {credentials:"same-origin"}).then(function(f) {
      return WebAssembly.instantiateStreaming(f, h).then(d, function(g) {
        E("wasm streaming compile failed: " + g);
        E("falling back to ArrayBuffer instantiation");
        e(d);
      });
    });
  })();
  return {};
}();
a.asm = Ea;
var za = a.___wasm_call_ctors = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.__wasm_call_ctors.apply(null, arguments);
};
a._main = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.main.apply(null, arguments);
};
a.___errno_location = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.__errno_location.apply(null, arguments);
};
a._fflush = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.fflush.apply(null, arguments);
};
a._setThrew = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.setThrew.apply(null, arguments);
};
a._malloc = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.malloc.apply(null, arguments);
};
a._free = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.free.apply(null, arguments);
};
a.___set_stack_limit = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.__set_stack_limit.apply(null, arguments);
};
var ca = a.stackSave = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.stackSave.apply(null, arguments);
}, da = a.stackAlloc = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.stackAlloc.apply(null, arguments);
};
a.stackRestore = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.stackRestore.apply(null, arguments);
};
a.__growWasmMemory = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.__growWasmMemory.apply(null, arguments);
};
a.dynCall_iidiiii = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.dynCall_iidiiii.apply(null, arguments);
};
a.dynCall_vii = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.dynCall_vii.apply(null, arguments);
};
a.dynCall_ii = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.dynCall_ii.apply(null, arguments);
};
a.dynCall_iiii = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.dynCall_iiii.apply(null, arguments);
};
a.dynCall_jiji = function() {
  assert(S, "you need to wait for the runtime to be ready (e.g. wait for main() to be called)");
  assert(!T, "the runtime was exited (use NO_EXIT_RUNTIME to keep it alive after main() exits)");
  return a.asm.dynCall_jiji.apply(null, arguments);
};
a.asm = Ea;
Object.getOwnPropertyDescriptor(a, "intArrayFromString") || (a.intArrayFromString = function() {
  C("'intArrayFromString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "intArrayToString") || (a.intArrayToString = function() {
  C("'intArrayToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "ccall") || (a.ccall = function() {
  C("'ccall' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "cwrap") || (a.cwrap = function() {
  C("'cwrap' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "setValue") || (a.setValue = function() {
  C("'setValue' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "getValue") || (a.getValue = function() {
  C("'getValue' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "allocate") || (a.allocate = function() {
  C("'allocate' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "getMemory") || (a.getMemory = function() {
  C("'getMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "AsciiToString") || (a.AsciiToString = function() {
  C("'AsciiToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "stringToAscii") || (a.stringToAscii = function() {
  C("'stringToAscii' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "UTF8ArrayToString") || (a.UTF8ArrayToString = function() {
  C("'UTF8ArrayToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "UTF8ToString") || (a.UTF8ToString = function() {
  C("'UTF8ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "stringToUTF8Array") || (a.stringToUTF8Array = function() {
  C("'stringToUTF8Array' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "stringToUTF8") || (a.stringToUTF8 = function() {
  C("'stringToUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "lengthBytesUTF8") || (a.lengthBytesUTF8 = function() {
  C("'lengthBytesUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "UTF16ToString") || (a.UTF16ToString = function() {
  C("'UTF16ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "stringToUTF16") || (a.stringToUTF16 = function() {
  C("'stringToUTF16' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "lengthBytesUTF16") || (a.lengthBytesUTF16 = function() {
  C("'lengthBytesUTF16' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "UTF32ToString") || (a.UTF32ToString = function() {
  C("'UTF32ToString' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "stringToUTF32") || (a.stringToUTF32 = function() {
  C("'stringToUTF32' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "lengthBytesUTF32") || (a.lengthBytesUTF32 = function() {
  C("'lengthBytesUTF32' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "allocateUTF8") || (a.allocateUTF8 = function() {
  C("'allocateUTF8' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "allocateUTF8OnStack") || (a.allocateUTF8OnStack = function() {
  C("'allocateUTF8OnStack' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "stackTrace") || (a.stackTrace = function() {
  C("'stackTrace' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "addOnPreRun") || (a.addOnPreRun = function() {
  C("'addOnPreRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "addOnInit") || (a.addOnInit = function() {
  C("'addOnInit' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "addOnPreMain") || (a.addOnPreMain = function() {
  C("'addOnPreMain' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "addOnExit") || (a.addOnExit = function() {
  C("'addOnExit' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "addOnPostRun") || (a.addOnPostRun = function() {
  C("'addOnPostRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "writeStringToMemory") || (a.writeStringToMemory = function() {
  C("'writeStringToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "writeArrayToMemory") || (a.writeArrayToMemory = function() {
  C("'writeArrayToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "writeAsciiToMemory") || (a.writeAsciiToMemory = function() {
  C("'writeAsciiToMemory' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "addRunDependency") || (a.addRunDependency = function() {
  C("'addRunDependency' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "removeRunDependency") || (a.removeRunDependency = function() {
  C("'removeRunDependency' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "ENV") || (a.ENV = function() {
  C("'ENV' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "FS") || (a.FS = function() {
  C("'FS' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "FS_createFolder") || (a.FS_createFolder = function() {
  C("'FS_createFolder' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "FS_createPath") || (a.FS_createPath = function() {
  C("'FS_createPath' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "FS_createDataFile") || (a.FS_createDataFile = function() {
  C("'FS_createDataFile' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "FS_createPreloadedFile") || (a.FS_createPreloadedFile = function() {
  C("'FS_createPreloadedFile' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "FS_createLazyFile") || (a.FS_createLazyFile = function() {
  C("'FS_createLazyFile' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "FS_createLink") || (a.FS_createLink = function() {
  C("'FS_createLink' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "FS_createDevice") || (a.FS_createDevice = function() {
  C("'FS_createDevice' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "FS_unlink") || (a.FS_unlink = function() {
  C("'FS_unlink' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
});
Object.getOwnPropertyDescriptor(a, "GL") || (a.GL = function() {
  C("'GL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "dynamicAlloc") || (a.dynamicAlloc = function() {
  C("'dynamicAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "loadDynamicLibrary") || (a.loadDynamicLibrary = function() {
  C("'loadDynamicLibrary' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "loadWebAssemblyModule") || (a.loadWebAssemblyModule = function() {
  C("'loadWebAssemblyModule' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "getLEB") || (a.getLEB = function() {
  C("'getLEB' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "getFunctionTables") || (a.getFunctionTables = function() {
  C("'getFunctionTables' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "alignFunctionTables") || (a.alignFunctionTables = function() {
  C("'alignFunctionTables' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "registerFunctions") || (a.registerFunctions = function() {
  C("'registerFunctions' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "addFunction") || (a.addFunction = function() {
  C("'addFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "removeFunction") || (a.removeFunction = function() {
  C("'removeFunction' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "getFuncWrapper") || (a.getFuncWrapper = function() {
  C("'getFuncWrapper' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "prettyPrint") || (a.prettyPrint = function() {
  C("'prettyPrint' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "makeBigInt") || (a.makeBigInt = function() {
  C("'makeBigInt' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "dynCall") || (a.dynCall = function() {
  C("'dynCall' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "getCompilerSetting") || (a.getCompilerSetting = function() {
  C("'getCompilerSetting' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "print") || (a.print = function() {
  C("'print' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "printErr") || (a.printErr = function() {
  C("'printErr' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "getTempRet0") || (a.getTempRet0 = function() {
  C("'getTempRet0' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "setTempRet0") || (a.setTempRet0 = function() {
  C("'setTempRet0' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "callMain") || (a.callMain = function() {
  C("'callMain' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "abort") || (a.abort = function() {
  C("'abort' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "warnOnce") || (a.warnOnce = function() {
  C("'warnOnce' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "stackSave") || (a.stackSave = function() {
  C("'stackSave' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "stackRestore") || (a.stackRestore = function() {
  C("'stackRestore' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
Object.getOwnPropertyDescriptor(a, "stackAlloc") || (a.stackAlloc = function() {
  C("'stackAlloc' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
});
a.writeStackCookie = ja;
a.checkStackCookie = Q;
a.abortStackOverflow = function(b) {
  C("Stack overflow! Attempted to allocate " + b + " bytes on the stack, but stack has only " + (3744 - ca() + b) + " bytes available!");
};
Object.getOwnPropertyDescriptor(a, "ALLOC_NORMAL") || Object.defineProperty(a, "ALLOC_NORMAL", {configurable:!0, get:function() {
  C("'ALLOC_NORMAL' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
}});
Object.getOwnPropertyDescriptor(a, "ALLOC_STACK") || Object.defineProperty(a, "ALLOC_STACK", {configurable:!0, get:function() {
  C("'ALLOC_STACK' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
}});
Object.getOwnPropertyDescriptor(a, "ALLOC_DYNAMIC") || Object.defineProperty(a, "ALLOC_DYNAMIC", {configurable:!0, get:function() {
  C("'ALLOC_DYNAMIC' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
}});
Object.getOwnPropertyDescriptor(a, "ALLOC_NONE") || Object.defineProperty(a, "ALLOC_NONE", {configurable:!0, get:function() {
  C("'ALLOC_NONE' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)");
}});
Object.getOwnPropertyDescriptor(a, "calledRun") || Object.defineProperty(a, "calledRun", {configurable:!0, get:function() {
  C("'calledRun' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ). Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you");
}});
var Z;
function B(b) {
  this.name = "ExitStatus";
  this.message = "Program terminated with exit(" + b + ")";
  this.status = b;
}
W = function Fa() {
  Z || Ga();
  Z || (W = Fa);
};
function Ga(b) {
  function d() {
    if (!Z && (Z = !0, !K)) {
      Q();
      assert(!S);
      S = !0;
      R(na);
      Q();
      R(oa);
      if (a.onRuntimeInitialized) {
        a.onRuntimeInitialized();
      }
      if (Ha) {
        var e = b;
        assert(0 == U, 'cannot call main when async dependencies remain! (listen on Module["onRuntimeInitialized"])');
        assert(0 == ma.length, "cannot call main when preRun functions remain to be called");
        var h = a._main;
        e = e || [];
        var k = e.length + 1, f = da(4 * (k + 1));
        M[f >> 2] = ha(n);
        for (var g = 1; g < k; g++) {
          M[(f >> 2) + g] = ha(e[g - 1]);
        }
        M[(f >> 2) + k] = 0;
        try {
          a.___set_stack_limit(3744);
          var p = h(k, f);
          Ia();
          if (!I || 0 !== p) {
            if (!I && (K = !0, Q(), T = !0, a.onExit)) {
              a.onExit(p);
            }
            r(p, new B(p));
          }
        } catch (q) {
          q instanceof B || ("unwind" == q ? I = !0 : ((e = q) && "object" === typeof q && q.stack && (e = [q, q.stack]), E("exception thrown: " + e), r(1, q)));
        } finally {
        }
      }
      Q();
      if (a.postRun) {
        for ("function" == typeof a.postRun && (a.postRun = [a.postRun]); a.postRun.length;) {
          e = a.postRun.shift(), pa.unshift(e);
        }
      }
      R(pa);
    }
  }
  b = b || m;
  if (!(0 < U)) {
    ja();
    if (a.preRun) {
      for ("function" == typeof a.preRun && (a.preRun = [a.preRun]); a.preRun.length;) {
        qa();
      }
    }
    R(ma);
    0 < U || (a.setStatus ? (a.setStatus("Running..."), setTimeout(function() {
      setTimeout(function() {
        a.setStatus("");
      }, 1);
      d();
    }, 1)) : d(), Q());
  }
}
a.run = Ga;
function Ia() {
  var b = D, d = E, e = !1;
  D = E = function() {
    e = !0;
  };
  try {
    Aa && Aa(0);
  } catch (h) {
  }
  D = b;
  E = d;
  e && (F("stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the FAQ), or make sure to emit a newline when you printf etc."), F("(this may also be due to not including full filesystem support - try building with -s FORCE_FILESYSTEM=1)"));
}
if (a.preInit) {
  for ("function" == typeof a.preInit && (a.preInit = [a.preInit]); 0 < a.preInit.length;) {
    a.preInit.pop()();
  }
}
var Ha = !0;
a.noInitialRun && (Ha = !1);
I = !0;
Ga();

