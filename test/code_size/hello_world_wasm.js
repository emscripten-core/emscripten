var d = Module, e = new TextDecoder("utf8"), f, g, h;

WebAssembly.instantiate(d.wasm, {
    a: {
        a: function(a) {
            var c = console, k = c.log;
            if (a) {
                for (var l = a + void 0, b = a; !(b >= l) && f[b]; ) ++b;
                a = e.decode(f.subarray(a, b));
            } else a = "";
            k.call(c, a);
        }
    }
}).then((function(a) {
    a = a.instance.exports;
    h = a.d;
    g = a.b;
    f = new Uint8Array(g.buffer);
    a.c();
    h();
}));