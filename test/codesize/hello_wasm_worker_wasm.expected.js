var c = Module, d = !!globalThis.WorkerGlobalScope, e = globalThis.name == "em-ww", f, g, C, q, D, m, E, v;

e && (onmessage = a => {
    onmessage = null;
    f = a = a.data;
    g = a.o;
    h();
    c ||= {};
    c.wasm = a.m;
    k();
    a.m = a.o = 0;
});

function h() {}

e || (g = c.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), h());

var l = [], n = a => {
    a = a.data;
    var b = a._wsc;
    b && m.get(b)(...a.x);
}, p = a => {
    l.push(a);
}, r = () => {
    q(0, !d, !e, d && 1);
}, u = {}, w = (a, b, z) => {
    var t = u[a] = new Worker(c.js, {
        name: "em-ww"
    });
    t.postMessage({
        A: a,
        m: v,
        o: g,
        u: b,
        v: z
    });
    t.onmessage = n;
    return !0;
}, x = () => performance.now(), y = () => !1, A = (a, b) => {
    u[a].postMessage({
        _wsc: b,
        x: []
    });
};

e && (u[0] = globalThis, addEventListener("message", p));

function B() {
    console.log("Hello from wasm worker!");
}

function k() {
    E = {
        d: r,
        c: w,
        b: x,
        e: y,
        f: A,
        g: B,
        a: g
    };
    WebAssembly.instantiate(c.wasm, {
        a: E
    }).then((a => {
        var b = (a.instance || a).exports;
        v = a.module || c.wasm;
        C = b.i;
        q = b.k;
        D = b.l;
        m = b.j;
        e ? (D(f.A, f.u, f.v), removeEventListener("message", p), l = l.forEach(n), addEventListener("message", n)) : b.h();
        e || C();
    }));
}

e || k();