var d = Module, e, f, g = new TextDecoder, h;

WebAssembly.instantiate(d.wasm, {
    a: {
        a: a => {
            var c = console, k = c.log;
            if (a) {
                for (var l = a + void 0, b = a; !(b >= l) && e[b]; ) ++b;
                a = g.decode(e.subarray(a, b));
            } else a = "";
            k.call(c, a);
        }
    }
}).then((a => {
    a = a.instance.exports;
    h = a.d;
    f = a.b;
    e = new Uint8Array(f.buffer);
    a.c();
    h();
}));