var c = Module, d = !!globalThis.WorkerGlobalScope, e = globalThis.name == "em-ww", f, g, B, p, C, m, D, u;

e && (onmessage = a => {
    onmessage = null;
    f = a = a.data;
    g = a.o;
    c ||= {};
    c.wasm = a.m;
    h();
    a.m = a.o = 0;
});

e || (g = new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}));

var l = a => {
    k.push(a);
}, k = [], n = a => {
    a = a.data;
    var b = a._wsc;
    b && m.get(b)(...a.x);
}, q = () => {
    p(0, !d, !e, d && 1);
}, t = {}, v = (a, b, y) => {
    var r = t[a] = new Worker(c.js, {
        name: "em-ww"
    });
    r.postMessage({
        A: a,
        m: u,
        o: g,
        u: b,
        v: y
    });
    r.onmessage = n;
    return !0;
}, w = () => performance.now(), x = () => !1, z = (a, b) => {
    t[a].postMessage({
        _wsc: b,
        x: []
    });
};

e && (t[0] = globalThis, addEventListener("message", l));

function A() {
    console.log("Hello from wasm worker!");
}

function h() {
    D = {
        d: q,
        c: v,
        b: w,
        e: x,
        f: z,
        g: A,
        a: g
    };
    WebAssembly.instantiate(c.wasm, {
        a: D
    }).then(a => {
        var b = (a.instance || a).exports;
        u = a.module || c.wasm;
        B = b.i;
        p = b.k;
        C = b.l;
        m = b.j;
        e ? (C(f.A, f.u, f.v), removeEventListener("message", l), k = k.forEach(n), addEventListener("message", n)) : b.h();
        e || B();
    });
}

e || h();