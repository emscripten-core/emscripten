var b = Module, c = b.$ww, f, e = b.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), g = e.buffer, h = [], m = {}, n = 1, p, q;

function k(a) {
    a = a.data;
    let d = a._wsc;
    d && f.get(d)(...a.x);
}

function l(a) {
    h.push(a);
}

c && (m[0] = this, addEventListener("message", l));

WebAssembly.instantiate(b.wasm, {
    a: {
        b: function(a, d) {
            let r = m[n] = new Worker(b.$wb);
            r.postMessage({
                $ww: n,
                wasm: b.wasm,
                js: b.js,
                mem: e,
                sb: a,
                sz: d
            });
            r.onmessage = k;
            return n++;
        },
        c: () => !1,
        d: function(a, d) {
            m[a].postMessage({
                _wsc: d,
                x: []
            });
        },
        e: function() {
            console.log("Hello from wasm worker!");
        },
        a: e
    }
}).then((a => {
    a = a.instance.exports;
    p = a.g;
    q = a.i;
    f = a.h;
    c ? (a = b, q(a.sb, a.sz), removeEventListener("message", l), h = h.forEach(k), 
    addEventListener("message", k)) : a.f();
    c || p();
}));