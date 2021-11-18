var d = Module;

var e = new TextDecoder("utf8"), f, g, h;

WebAssembly.instantiate(d.wasm, {
    a: {
        a: function(a) {
            var b = console, k = b.log;
            if (a) {
                for (var l = a + void 0, c = a; !(c >= l) && f[c]; ) ++c;
                a = e.decode(f.subarray(a, c));
            } else a = "";
            k.call(b, a);
        }
    }
}).then((function(a) {
    a = a.instance.exports;
    h = a.d;
    g = a.b;
    var b = g.buffer;
    f = new Uint8Array(b);
    a.c();
    h();
}));