var e = Module;

var f = e.$ww, g = new function(a) {
    var c = new TextDecoder(a);
    this.l = b => {
        b.buffer instanceof SharedArrayBuffer && (b = new Uint8Array(b));
        return c.decode.call(c, b);
    };
}("utf8");

function h(a) {
    if (!a) return "";
    for (var c = a + NaN, b = a; !(b >= c) && k[b]; ) ++b;
    return g.l(k.subarray(a, b));
}

var l, k, m, n, p;

n = e.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
});

var q = n.buffer;

l = new Int32Array(q);

k = new Uint8Array(q);

new Uint16Array(q);

m = new Float64Array(q);

var r = {
    1908: function() {
        console.log("Hello from wasm worker!");
    }
}, t = {}, u = 1;

function v(a) {
    w.push(a);
}

function x(a) {
    a = a.data;
    let c = a._wsc;
    c && p.get(c)(...a.x);
}

var w = [], y = [];

f && (t[0] = this, addEventListener("message", v));

var z, A;

WebAssembly.instantiate(e.wasm, {
    a: {
        b: function(a, c, b, d) {
            throw "Assertion failed: " + h(a) + ",at: " + [ c ? h(c) : "unknown filename", b, d ? h(d) : "unknown function" ];
        },
        c: function(a, c, b) {
            let d = t[u] = new Worker(e.$wb);
            d.postMessage({
                $ww: u,
                wasm: e.wasm,
                js: e.js,
                mem: n,
                sb: a,
                sz: c,
                tb: b
            });
            d.addEventListener("message", x);
            return u++;
        },
        f: function(a, c, b) {
            y.length = 0;
            var d;
            for (b >>= 2; d = k[c++]; ) (d = 105 > d) && b & 1 && b++, y.push(d ? m[b++ >> 1] : l[b]), 
            ++b;
            return r[a].apply(null, y);
        },
        d: function() {
            return !1;
        },
        e: function(a, c) {
            t[a].postMessage({
                _wsc: c,
                x: []
            });
        },
        a: n
    }
}).then((function(a) {
    a = a.instance.exports;
    z = a.h;
    A = a.j;
    p = a.i;
    f ? (a = e, A(a.sb, a.sz, a.tb), removeEventListener("message", v), w.forEach(x), 
    w = null, addEventListener("message", x)) : a.g();
    f || z();
}));