var b = Module, c = "em-ww" == globalThis.name, e, f, l, x, y, z;

c && (onmessage = a => {
    onmessage = null;
    f = a = a.data;
    e = a.o;
    g();
    b ||= {};
    b.wasm = a.j;
    h();
    a.j = a.s = 0;
});

function g() {}

c || (e = b.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), g());

var k = [], n = a => {
    a = a.data;
    let d = a._wsc;
    d && l.get(d)(...a.x);
}, p = a => {
    k.push(a);
}, q = {}, r = 1, t = (a, d) => {
    let m = q[r] = new Worker(b.js, {
        name: "em-ww"
    });
    m.postMessage({
        u: r,
        j: b.wasm,
        o: e,
        l: a,
        m: d
    });
    m.onmessage = n;
    return r++;
}, u = () => !1, v = (a, d) => {
    q[a].postMessage({
        _wsc: d,
        x: []
    });
};

c && (q[0] = globalThis, addEventListener("message", p));

function w() {
    console.log("Hello from wasm worker!");
}

function h() {
    z = {
        b: t,
        c: u,
        d: v,
        e: w,
        a: e
    };
    WebAssembly.instantiate(b.wasm, {
        a: z
    }).then((a => {
        a = a.instance.exports;
        x = a.g;
        y = a.i;
        l = a.h;
        c ? (y(f.l, f.m), removeEventListener("message", p), k = k.forEach(n), addEventListener("message", n)) : a.f();
        c || x();
    }));
}

c || h();