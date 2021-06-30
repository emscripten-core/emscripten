var d = Module;

var e = new TextDecoder("utf8"), f, g, h;

WebAssembly.instantiate(d.wasm, {
    a: {
        a: function(a) {
            if (a) {
                for (var b = a + void 0, c = a; !(c >= b) && f[c]; ) ++c;
                a = e.decode(f.subarray(a, c));
            } else a = "";
            console.log(a);
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