var c = Module, d = !!globalThis.WorkerGlobalScope, e = "em-ww" == globalThis.name, f, g, C, r, D, n, E, v;

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

var m = [], p = a => {
    a = a.data;
    let b = a._wsc;
    b && n.get(b)(...a.x);
}, q = a => {
    m.push(a);
}, t = () => {
    r(0, !d, !e, d && 1);
}, u = {}, w = (a, b, z) => {
    let l = u[a] = new Worker(c.js, {
        name: "em-ww"
    });
    l.postMessage({
        v: a,
        m: v,
        o: g,
        s: b,
        u: z
    });
    l.onmessage = p;
    return !0;
}, x = () => performance.now(), y = () => !1, A = (a, b) => {
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
        d: t,
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
        r = b.k;
        D = b.l;
        n = b.j;
        e ? (D(f.v, f.s, f.u), removeEventListener("message", q), m = m.forEach(p), addEventListener("message", p)) : b.h();
        e || C();
    }));
}

e || k();