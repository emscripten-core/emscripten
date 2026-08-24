var c = Module, d = !!globalThis.WorkerGlobalScope, e = globalThis.name == "em-ww", f, g, B, p, C, l, D, u;

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

var k = [], m = a => {
    a = a.data;
    var b = a._wsc;
    b && l.get(b)(...a.x);
}, n = a => {
    k.push(a);
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
    r.onmessage = m;
    return !0;
}, w = () => performance.now(), x = () => !1, z = (a, b) => {
    t[a].postMessage({
        _wsc: b,
        x: []
    });
};

e && (t[0] = globalThis, addEventListener("message", n));

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
    }).then((a => {
        var b = (a.instance || a).exports;
        u = a.module || c.wasm;
        B = b.i;
        p = b.k;
        C = b.l;
        l = b.j;
        e ? (C(f.A, f.u, f.v), removeEventListener("message", n), k = k.forEach(m), addEventListener("message", m)) : b.h();
        e || B();
    }));
}

e || h();