var b = Module, c = b.$ww, e = b.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), f = e.buffer, g = [], h, k = a => {
    a = a.data;
    let d = a._wsc;
    d && h.get(d)(...a.x);
}, l = a => {
    g.push(a);
}, m = {}, n = 1, p, q;

c && (m[0] = this, addEventListener("message", l));

WebAssembly.instantiate(b.wasm, {
    a: {
        b: (a, d) => {
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
        d: (a, d) => {
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
    h = a.h;
    c ? (a = b, q(a.sb, a.sz), removeEventListener("message", l), g = g.forEach(k), 
    addEventListener("message", k)) : a.f();
    c || p();
}));