var d = Module, e, f, g = new TextDecoder, h;

WebAssembly.instantiate(d.wasm, {
    a: {
        a: a => {
            var c = console, k = c.log;
            if (a) {
                for (var b = a, l = f, m = b + void 0; l[b] && !(b >= m); ) ++b;
                a = g.decode(f.subarray(a, b));
            } else a = "";
            k.call(c, a);
        }
    }
}).then((a => {
    a = a.instance.exports;
    h = a.d;
    e = a.b;
    f = new Uint8Array(e.buffer);
    a.c();
    h();
}));