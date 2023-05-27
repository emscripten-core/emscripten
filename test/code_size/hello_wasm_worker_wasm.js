var b = Module, d = b.$ww, f, e = b.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), g = e.buffer, h = [], n = new function() {
    this.j = [ void 0 ];
    this.m = [];
    this.get = a => this.j[a];
    this.has = a => void 0 !== this.j[a];
    this.l = a => {
        var c = this.m.pop() || this.j.length;
        this.j[c] = a;
        return c;
    };
}, p, q;

function k(a) {
    a = a.data;
    let c = a._wsc;
    c && f.get(c)(...a.x);
}

function m(a) {
    h.push(a);
}

d && (n.j[0] = this, addEventListener("message", m));

WebAssembly.instantiate(b.wasm, {
    a: {
        b: function(a, c) {
            let l = new Worker(b.$wb), r = n.l(l);
            l.postMessage({
                $ww: r,
                wasm: b.wasm,
                js: b.js,
                mem: e,
                sb: a,
                sz: c
            });
            l.onmessage = k;
            return r;
        },
        c: function() {
            return !1;
        },
        d: function(a, c) {
            n.get(a).postMessage({
                _wsc: c,
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
    d ? (a = b, q(a.sb, a.sz), removeEventListener("message", m), h = h.forEach(k), 
    addEventListener("message", k)) : a.f();
    d || p();
}));