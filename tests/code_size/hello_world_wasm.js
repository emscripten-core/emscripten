var d = Module;

var e = new TextDecoder("utf8");

function f(a) {
    if (!a) return "";
    for (var b = a + NaN, c = a; !(c >= b) && g[c]; ) ++c;
    return e.decode(g.subarray(a, c));
}

var g, h, k;

WebAssembly.instantiate(d.wasm, {
    a: {
        a: function(a) {
            console.log(f(a));
        }
    }
}).then((function(a) {
    a = a.instance.exports;
    k = a.d;
    h = a.b;
    var b = h.buffer;
    g = new Uint8Array(b);
    a.c();
    k();
}));