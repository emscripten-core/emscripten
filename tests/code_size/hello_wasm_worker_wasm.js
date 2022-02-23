var Module = Module;

var ENVIRONMENT_IS_WASM_WORKER = Module["$ww"];

function ready() {
    if (!ENVIRONMENT_IS_WASM_WORKER) {
        run();
    }
}

function abort(what) {
    throw what;
}

function TextDecoderWrapper(encoding) {
    var textDecoder = new TextDecoder(encoding);
    this.decode = data => {
        if (data.buffer instanceof SharedArrayBuffer) {
            data = new Uint8Array(data);
        }
        return textDecoder.decode.call(textDecoder, data);
    };
}

var UTF8Decoder = new TextDecoderWrapper("utf8");

function UTF8ToString(ptr, maxBytesToRead) {
    if (!ptr) return "";
    var maxPtr = ptr + maxBytesToRead;
    for (var end = ptr; !(end >= maxPtr) && HEAPU8[end]; ) ++end;
    return UTF8Decoder.decode(HEAPU8.subarray(ptr, end));
}

var HEAP8, HEAP16, HEAP32, HEAPU8, HEAPU16, HEAPU32, HEAPF32, HEAPF64, wasmMemory, buffer, wasmTable;

function updateGlobalBufferAndViews(b) {
    buffer = b;
    HEAP8 = new Int8Array(b);
    HEAP16 = new Int16Array(b);
    HEAP32 = new Int32Array(b);
    HEAPU8 = new Uint8Array(b);
    HEAPU16 = new Uint16Array(b);
    HEAPU32 = new Uint32Array(b);
    HEAPF32 = new Float32Array(b);
    HEAPF64 = new Float64Array(b);
}

wasmMemory = Module["mem"] || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: true
});

updateGlobalBufferAndViews(wasmMemory.buffer);

var ASM_CONSTS = {
    1916: function() {
        console.log("Hello from wasm worker!");
    }
};

function getWasmTableEntry(funcPtr) {
    return wasmTable.get(funcPtr);
}

function ___assert_fail(condition, filename, line, func) {
    abort("Assertion failed: " + UTF8ToString(condition) + ", at: " + [ filename ? UTF8ToString(filename) : "unknown filename", line, func ? UTF8ToString(func) : "unknown function" ]);
}

var _wasm_workers = {};

var _wasm_workers_id = 1;

function __wasm_worker_appendToQueue(e) {
    __wasm_worker_delayedMessageQueue.push(e);
}

function __wasm_worker_runPostMessage(e) {
    let data = e.data, wasmCall = data["_wsc"];
    wasmCall && getWasmTableEntry(wasmCall)(...data["x"]);
}

function __emscripten_create_wasm_worker_with_tls(stackLowestAddress, stackSize, tlsAddress) {
    let worker = _wasm_workers[_wasm_workers_id] = new Worker(Module["$wb"]);
    worker.postMessage({
        $ww: _wasm_workers_id,
        wasm: Module["wasm"],
        js: Module["js"],
        mem: wasmMemory,
        sb: stackLowestAddress,
        sz: stackSize,
        tb: tlsAddress
    });
    worker.addEventListener("message", __wasm_worker_runPostMessage);
    return _wasm_workers_id++;
}

var __wasm_worker_delayedMessageQueue = [];

function __wasm_worker_initializeRuntime() {
    let m = Module;
    _emscripten_wasm_worker_initialize(m["sb"], m["sz"], m["tb"]);
    removeEventListener("message", __wasm_worker_appendToQueue);
    __wasm_worker_delayedMessageQueue.forEach(__wasm_worker_runPostMessage);
    __wasm_worker_delayedMessageQueue = null;
    addEventListener("message", __wasm_worker_runPostMessage);
}

var readAsmConstArgsArray = [];

function readAsmConstArgs(sigPtr, buf) {
    readAsmConstArgsArray.length = 0;
    var ch;
    buf >>= 2;
    while (ch = HEAPU8[sigPtr++]) {
        var readAsmConstArgsDouble = ch < 105;
        if (readAsmConstArgsDouble && buf & 1) buf++;
        readAsmConstArgsArray.push(readAsmConstArgsDouble ? HEAPF64[buf++ >> 1] : HEAP32[buf]);
        ++buf;
    }
    return readAsmConstArgsArray;
}

function _emscripten_asm_const_int(code, sigPtr, argbuf) {
    var args = readAsmConstArgs(sigPtr, argbuf);
    return ASM_CONSTS[code].apply(null, args);
}

function _emscripten_resize_heap(requestedSize) {
    var oldSize = HEAPU8.length;
    requestedSize = requestedSize >>> 0;
    return false;
}

function _emscripten_wasm_worker_post_function_v(id, funcPtr) {
    _wasm_workers[id].postMessage({
        _wsc: funcPtr,
        x: []
    });
}

if (ENVIRONMENT_IS_WASM_WORKER) {
    _wasm_workers[0] = this;
    addEventListener("message", __wasm_worker_appendToQueue);
}

var asmLibraryArg = {
    b: ___assert_fail,
    c: __emscripten_create_wasm_worker_with_tls,
    f: _emscripten_asm_const_int,
    d: _emscripten_resize_heap,
    e: _emscripten_wasm_worker_post_function_v,
    a: wasmMemory
};

function run() {
    var ret = _main();
}

function initRuntime(asm) {
    if (ENVIRONMENT_IS_WASM_WORKER) return __wasm_worker_initializeRuntime();
    asm["g"]();
}

var imports = {
    a: asmLibraryArg
};

var _main, _emscripten_wasm_worker_initialize;

WebAssembly.instantiate(Module["wasm"], imports).then((function(output) {
    var asm = output.instance.exports;
    _main = asm["h"];
    _emscripten_wasm_worker_initialize = asm["j"];
    wasmTable = asm["i"];
    initRuntime(asm);
    ready();
}));