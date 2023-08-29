var c = Module, d, e;

WebAssembly.instantiate(c.wasm, {
    a: {}
}).then((a => {
    a = a.instance.exports;
    e = a.c;
    d = a.a;
    a.b();
    e();
}));