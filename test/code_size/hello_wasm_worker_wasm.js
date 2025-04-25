var b = Module, c = "em-ww" == globalThis.name, e, k, w, x, y;

c && (onmessage = a => {
    onmessage = null;
    a = a.data;
    b ||= {};
    Object.assign(b, a);
    e = a.mem;
    f();
    g();
    a.wasm = a.mem = 0;
});

function f() {}

c || (e = b.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), f());

var h = [], m = a => {
    a = a.data;
    let d = a._wsc;
    d && k.get(d)(...a.x);
}, n = a => {
    h.push(a);
}, p = {}, q = 1, r = (a, d) => {
    let l = p[q] = new Worker(b.js, {
        name: "em-ww"
    });
    l.postMessage({
        $ww: q,
        wasm: b.wasm,
        mem: e,
        sb: a,
        sz: d
    });
    l.onmessage = m;
    return q++;
}, t = () => !1, u = (a, d) => {
    p[a].postMessage({
        _wsc: d,
        x: []
    });
};

c && (p[0] = globalThis, addEventListener("message", n));

function v() {
    console.log("Hello from wasm worker!");
}

function g() {
    w = {
        b: r,
        c: t,
        d: u,
        e: v,
        a: e
    };
    WebAssembly.instantiate(b.wasm, {
        a: w
    }).then((a => {
        a = a.instance.exports;
        x = a.g;
        y = a.i;
        k = a.h;
        c ? (a = b, y(a.sb, a.sz), removeEventListener("message", n), h = h.forEach(m), 
        addEventListener("message", m)) : a.f();
        c || x();
    }));
}

c || g();