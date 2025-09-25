var d = Module, e, f = new TextDecoder, g, h;

WebAssembly.instantiate(d.wasm, {
    a: {
        a: a => {
            var c = console, k = c.log;
            if (a) {
                for (var b = a, l = e, m = b + void 0; l[b] && !(b >= m); ) ++b;
                a = f.decode(e.subarray(a, b));
            } else a = "";
            k.call(c, a);
        }
    }
}).then((a => {
    a = a.instance.exports;
    g = a.d;
    h = a.b;
    e = new Uint8Array(h.buffer);
    a.c();
    g();
}));