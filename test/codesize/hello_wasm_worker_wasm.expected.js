var c = Module, d = !!globalThis.WorkerGlobalScope, e = "em-ww" == globalThis.name, f, g, C, r, D, n, E, w;

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

var l = [], p = a => {
    a = a.data;
    let b = a._wsc;
    b && n.get(b)(...a.x);
}, q = a => {
    l.push(a);
}, t = () => {
    r(0, !d, !e, d && 1);
}, u = {}, v = 1, x = (a, b) => {
    let m = u[v] = new Worker(c.js, {
        name: "em-ww"
    });
    m.postMessage({
        v: v,
        m: w,
        o: g,
        s: a,
        u: b
    });
    m.onmessage = p;
    return v++;
}, y = () => performance.now(), z = () => !1, A = (a, b) => {
    u[a].postMessage({
        _wsc: b,
        x: []
    });
};

e && (u[0] = globalThis, addEventListener("message", q));

function B() {
    console.log("Hello from wasm worker!");
}

function k() {
    E = {
        e: t,
        c: x,
        b: y,
        d: z,
        f: A,
        g: B,
        a: g
    };
    WebAssembly.instantiate(c.wasm, {
        a: E
    }).then((a => {
        var b = (a.instance || a).exports;
        w = a.module || c.wasm;
        C = b.i;
        r = b.k;
        D = b.l;
        n = b.j;
        e ? (D(f.v, f.s, f.u), removeEventListener("message", q), l = l.forEach(p), addEventListener("message", p)) : b.h();
        e || C();
    }));
}

e || k();