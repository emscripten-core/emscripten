var b = Module, c = b.$ww, f, e = b.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), g = e.buffer, h = {}, k = 1, m = [], p, q;

function l(a) {
    m.push(a);
}

function n(a) {
    a = a.data;
    let d = a._wsc;
    d && f.get(d)(...a.x);
}

c && (h[0] = this, addEventListener("message", l));

WebAssembly.instantiate(b.wasm, {
    a: {
        b: function(a, d) {
            let r = h[k] = new Worker(b.$wb);
            r.postMessage({
                $ww: k,
                wasm: b.wasm,
                js: b.js,
                mem: e,
                sb: a,
                sz: d
            });
            r.addEventListener("message", n);
            return k++;
        },
        c: function() {
            return !1;
        },
        d: function(a, d) {
            h[a].postMessage({
                _wsc: d,
                x: []
            });
        },
        e: function() {
            console.log("Hello from wasm worker!");
        },
        a: e
    }
}).then((function(a) {
    a = a.instance.exports;
    p = a.g;
    q = a.i;
    f = a.h;
    c ? (a = b, q(a.sb, a.sz), removeEventListener("message", l), m = m.forEach(n), 
    addEventListener("message", n)) : a.f();
    c || p();
}));