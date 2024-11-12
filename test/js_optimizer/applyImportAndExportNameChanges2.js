var Module;
if (!Module) Module = "__EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__";
var ENVIRONMENT_IS_NODE = typeof process === "object";
if (ENVIRONMENT_IS_NODE) {
    var fs = require("fs");
    Module["wasm"] = fs.readFileSync(__dirname + "/a.wasm")
}

function out(text) {
    console.log(text)
}

function err(text) {
    console.error(text)
}

function ready() {
    run()
}

function abort(what) {
    throw what
}
var UTF8Decoder = typeof TextDecoder != "undefined" ? new TextDecoder("utf8") : undefined;

function UTF8ArrayToString(u8Array, idx, maxBytesToRead) {
    var endIdx = idx + maxBytesToRead;
    var endPtr = idx;
    while (u8Array[endPtr] && !(endPtr >= endIdx)) ++endPtr;
    if (endPtr - idx > 16 && u8Array.subarray && UTF8Decoder) {
        return UTF8Decoder.decode(u8Array.subarray(idx, endPtr))
    } else {
        var str = "";
        while (idx < endPtr) {
            var u0 = u8Array[idx++];
            if (!(u0 & 128)) {
                str += String.fromCharCode(u0);
                continue
            }
            var u1 = u8Array[idx++] & 63;
            if ((u0 & 224) == 192) {
                str += String.fromCharCode((u0 & 31) << 6 | u1);
                continue
            }
            var u2 = u8Array[idx++] & 63;
            if ((u0 & 240) == 224) {
                u0 = (u0 & 15) << 12 | u1 << 6 | u2
            } else {
                u0 = (u0 & 7) << 18 | u1 << 12 | u2 << 6 | u8Array[idx++] & 63
            }
            if (u0 < 65536) {
                str += String.fromCharCode(u0)
            } else {
                var ch = u0 - 65536;
                str += String.fromCharCode(55296 | ch >> 10, 56320 | ch & 1023)
            }
        }
    }
    return str
}

function UTF8ToString(ptr, maxBytesToRead) {
    return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : ""
}
var TOTAL_MEMORY = 16777216,
    STATIC_BASE = 1024,
    DYNAMICTOP_PTR = 6016;
var wasmMaximumMemory = TOTAL_MEMORY;
var wasmMemory = new WebAssembly.Memory({
    "initial": TOTAL_MEMORY >> 16,
    "maximum": wasmMaximumMemory >> 16
});
var buffer = wasmMemory.buffer;
var HEAP8 = new Int8Array(buffer);
var HEAP16 = new Int16Array(buffer);
var HEAP32 = new Int32Array(buffer);
var HEAPU8 = new Uint8Array(buffer);
var HEAPU16 = new Uint16Array(buffer);
var HEAPU32 = new Uint32Array(buffer);
var HEAPF32 = new Float32Array(buffer);
var HEAPF64 = new Float64Array(buffer);
HEAP32[DYNAMICTOP_PTR >> 2] = 5249152;
var SYSCALLS = {
    buffers: [null, [],
        []
    ],
    printChar: (function(stream, curr) {
        var buffer = SYSCALLS.buffers[stream];
        if (curr === 0 || curr === 10) {
            (stream === 1 ? out : err)(UTF8ArrayToString(buffer));
            buffer.length = 0
        } else {
            buffer.push(curr)
        }
    }),
    varargs: 0,
    get: (function(varargs) {
        SYSCALLS.varargs += 4;
        var ret = HEAP32[SYSCALLS.varargs - 4 >> 2];
        return ret
    }),
    getStr: (function() {
        var ret = UTF8ToString(SYSCALLS.get());
        return ret
    }),
    get64: (function() {
        var low = SYSCALLS.get(),
            high = SYSCALLS.get();
        return low
    }),
    getZero: (function() {
        SYSCALLS.get()
    })
};

function ___syscall140(which, varargs) {
    SYSCALLS.varargs = varargs;
    try {
        var stream = SYSCALLS.getStreamFromFD(),
            offset_high = SYSCALLS.get(),
            offset_low = SYSCALLS.get(),
            result = SYSCALLS.get(),
            whence = SYSCALLS.get();
        var offset = offset_low;
        FS.llseek(stream, offset, whence);
        HEAP32[result >> 2] = stream.position;
        if (stream.getdents && offset === 0 && whence === 0) stream.getdents = null;
        return 0
    } catch (e) {
        if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
        return -e.errno
    }
}

function ___syscall146(which, varargs) {
    SYSCALLS.varargs = varargs;
    try {
        var stream = SYSCALLS.get(),
            iov = SYSCALLS.get(),
            iovcnt = SYSCALLS.get();
        var ret = 0;
        for (var i = 0; i < iovcnt; i++) {
            var ptr = HEAP32[iov + i * 8 >> 2];
            var len = HEAP32[iov + (i * 8 + 4) >> 2];
            for (var j = 0; j < len; j++) {
                SYSCALLS.printChar(stream, HEAPU8[ptr + j])
            }
            ret += len
        }
        return ret
    } catch (e) {
        if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
        return -e.errno
    }
}

function ___syscall54(which, varargs) {
    SYSCALLS.varargs = varargs;
    try {
        return 0
    } catch (e) {
        if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
        return -e.errno
    }
}

function ___syscall6(which, varargs) {
    SYSCALLS.varargs = varargs;
    try {
        var stream = SYSCALLS.getStreamFromFD();
        FS.close(stream);
        return 0
    } catch (e) {
        if (typeof FS === "undefined" || !(e instanceof FS.ErrnoError)) abort(e);
        return -e.errno
    }
}

function _emscripten_get_now() {
    abort()
}

function _emscripten_random() {
    return Math.random()
}

function _emscripten_memcpy_js(dest, src, num) {
    HEAPU8.set(HEAPU8.subarray(src, src + num), dest)
}
if (ENVIRONMENT_IS_NODE) {
    _emscripten_get_now = function _emscripten_get_now_actual() {
        var t = process.hrtime();
        return t[0] * 1e3 + t[1] / 1e6
    }
} else if (typeof self === "object" && self["performance"] && typeof self["performance"]["now"] === "function") {
    _emscripten_get_now = (function() {
        return self["performance"]["now"]()
    })
} else if (typeof performance === "object" && typeof performance["now"] === "function") {
    _emscripten_get_now = (function() {
        return performance["now"]()
    })
} else {
    _emscripten_get_now = Date.now
}
var wasmImports = {
    abort: abort,
    ___syscall140: ___syscall140,
    ___syscall146: ___syscall146,
    ___syscall54: ___syscall54,
    ___syscall6: ___syscall6,
    _emscripten_get_now: _emscripten_get_now,
    _emscripten_memcpy_js: _emscripten_memcpy_js,
    _emscripten_random: _emscripten_random
};

function run() {
    var ret = _main()
}

function initRuntime(wasmExports) {
    wasmExports["__GLOBAL__sub_I_test_global_initializer_cpp"]()
}
var env = wasmImports;
env["memory"] = wasmMemory;
env["table"] = new WebAssembly.Table({
    "initial": 6,
    "maximum": 6,
    "element": "anyfunc"
});
env["__memory_base"] = STATIC_BASE;
env["__table_base"] = 0;
var imports = {
    "env": env,
    "global": {
        "NaN": NaN,
        "Infinity": Infinity
    },
    "global.Math": Math,
    "asm2wasm": {
        "f64-rem": (function(x, y) {
            return x % y
        }),
        "debugger": (function() {
            debugger
        })
    }
};
var ___errno_location, _llvm_bswap_i32, _main, _memcpy, _memset, dynCall_ii, dynCall_iiii;
WebAssembly.instantiate(Module["wasm"], imports).then(((output) => {
    var wasmExports = output.instance.exports;
    ___errno_location = wasmExports["___errno_location"];
    _llvm_bswap_i32 = wasmExports["_llvm_bswap_i32"];
    _main = wasmExports["_main"];
    _memcpy = wasmExports["_memcpy"];
    _memset = wasmExports["_memset"];
    dynCall_ii = wasmExports["dynCall_ii"];
    dynCall_iiii = wasmExports["dynCall_iiii"];
    initRuntime(wasmExports);
    ready()
}))



// EXTRA_INFO: {"mapping": {"_llvm_bswap_i32": "k", "_emscripten_random": "c", "dynCall_ii": "o", "__GLOBAL__sub_I_test_global_initializer_cpp": "i", "___errno_location": "j", "dynCall_iiii": "p", "___syscall6": "f", "_memset": "n", "_memcpy": "m", "abort": "b", "___syscall146": "a", "_emscripten_memcpy_js": "d", "___syscall54": "g", "___syscall140": "h", "_emscripten_get_now": "e", "_main": "l"}}
