var c = Module, d, e = new TextDecoder, f, h;

WebAssembly.instantiate(c.wasm, {
    a: {
        a: a => {
            var g = console, k = g.log;
            if (a) {
                for (var b = a, l = d, m = b + NaN; l[b] && !(b >= m); ) ++b;
                a = e.decode(d.subarray(a, b));
            } else a = "";
            k.call(g, a);
        }
    }
}).then(a => {
    a = a.instance.exports;
    f = a.d;
    h = a.b;
    d = new Uint8Array(h.buffer);
    a.c();
    f();
});