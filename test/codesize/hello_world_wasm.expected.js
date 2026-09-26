var c = Module, d = new TextDecoder, e, f, h;

WebAssembly.instantiate(c.wasm, {
    a: {
        a: a => {
            var g = console, k = g.log;
            if (a) {
                for (var b = a, l = e, m = b + NaN; l[b] && !(b >= m); ) ++b;
                a = d.decode(e.subarray(a, b));
            } else a = "";
            k.call(g, a);
        }
    }
}).then((a => {
    a = a.instance.exports;
    f = a.d;
    h = a.b;
    e = new Uint8Array(h.buffer);
    a.c();
    f();
}));