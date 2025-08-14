var c = Module, d = "em-ww" == globalThis.name, e, f, l, y, z, A, t;

d && (onmessage = a => {
    onmessage = null;
    e = a = a.data;
    f = a.o;
    g();
    c ||= {};
    c.wasm = a.j;
    h();
    a.j = a.s = 0;
});

function g() {}

d || (f = c.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), g());

var k = [], n = a => {
    a = a.data;
    let b = a._wsc;
    b && l.get(b)(...a.x);
}, p = a => {
    k.push(a);
}, q = {}, r = 1, u = (a, b) => {
    let m = q[r] = new Worker(c.js, {
        name: "em-ww"
    });
    m.postMessage({
        u: r,
        j: t,
        o: f,
        l: a,
        m: b
    });
    m.onmessage = n;
    return r++;
}, v = () => !1, w = (a, b) => {
    q[a].postMessage({
        _wsc: b,
        x: []
    });
};

d && (q[0] = globalThis, addEventListener("message", p));

function x() {
    console.log("Hello from wasm worker!");
}

function h() {
    A = {
        b: u,
        c: v,
        d: w,
        e: x,
        a: f
    };
    WebAssembly.instantiate(c.wasm, {
        a: A
    }).then((a => {
        var b = (a.instance || a).exports;
        t = a.module || c.wasm;
        y = b.g;
        z = b.i;
        l = b.h;
        d ? (z(e.l, e.m), removeEventListener("message", p), k = k.forEach(n), addEventListener("message", n)) : b.f();
        d || y();
    }));
}

d || h();