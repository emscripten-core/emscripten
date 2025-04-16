var b = Module, c = !!b.$ww, e = b.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), f = e.buffer, g = [], h, k = a => {
    a = a.data;
    let d = a._wsc;
    d && h.get(d)(...a.x);
}, l = a => {
    g.push(a);
}, n = {}, p = 1, q, r;

c && (n[0] = this, addEventListener("message", l));

WebAssembly.instantiate(b.wasm, {
    a: {
        b: (a, d) => {
            let m = n[p] = new Worker(b.$wb);
            m.postMessage({
                $ww: p,
                wasm: b.wasm,
                js: b.js,
                mem: e,
                sb: a,
                sz: d
            });
            m.onmessage = k;
            return p++;
        },
        c: () => !1,
        d: (a, d) => {
            n[a].postMessage({
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
    q = a.g;
    r = a.i;
    h = a.h;
    c ? (a = b, r(a.sb, a.sz), removeEventListener("message", l), g = g.forEach(k), 
    addEventListener("message", k)) : a.f();
    c || q();
}));