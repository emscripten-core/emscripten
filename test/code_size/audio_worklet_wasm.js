var l = globalThis.Module || "undefined" != typeof Module ? Module : {}, n = "em-ww" == globalThis.name, q = "undefined" !== typeof AudioWorkletGlobalScope, r, u, K, L, F, H, B, X, I, D, C, Y, Z;

q && (n = !0);

function t(a) {
    r = a;
    u = a.I;
    x();
    l ||= {};
    l.wasm = a.C;
    y();
    a.C = a.J = 0;
}

n && !q && (onmessage = a => {
    onmessage = null;
    t(a.data);
});

if (q) {
    function a(c) {
        class e extends AudioWorkletProcessor {
            constructor(d) {
                super();
                d = d.processorOptions;
                this.u = B.get(d.u);
                this.v = d.v;
                this.s = d.s;
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, g, h) {
                let p = d.length, v = g.length, E = 0, k, z, m, w = 4 * this.s, f = 12 * (p + v), W = C(), G, P, A;
                for (k of d) f += k.length * w;
                for (k of g) f += k.length * w;
                for (k in h) f += h[k].byteLength + 8, ++E;
                f = G = D(f);
                m = G + 12 * p;
                for (k of d) {
                    F[f >> 2] = k.length;
                    F[f + 4 >> 2] = this.s;
                    F[f + 8 >> 2] = m;
                    f += 12;
                    for (z of k) H.set(z, m >> 2), m += w;
                }
                f = P = m;
                d = m += 12 * v;
                for (k of g) F[f >> 2] = k.length, F[f + 4 >> 2] = this.s, F[f + 8 >> 2] = m, f += 12, 
                m += w * k.length;
                f = w = m;
                m += 8 * E;
                for (k = 0; A = h[k++]; ) F[f >> 2] = A.length, F[f + 4 >> 2] = m, f += 8, H.set(A, m >> 2), 
                m += 4 * A.length;
                if (h = this.u(p, G, v, P, E, w, this.v)) {
                    d >>= 2;
                    for (k of g) for (z of k) for (f = 0; f < this.s; ++f) z[f] = H[d++];
                }
                I(W);
                return !!h;
            }
        }
        return e;
    }
    var J;
    class b extends AudioWorkletProcessor {
        constructor(c) {
            super();
            t(c.processorOptions);
            J = this.port;
            J.onmessage = async e => {
                e = e.data;
                e._wpn ? (registerProcessor(e._wpn, a(e.D)), J.postMessage({
                    _wsc: e.u,
                    A: [ e.F, 1, e.v ]
                })) : e._wsc && B.get(e._wsc)(...e.A);
            };
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
}

function x() {
    var a = u.buffer;
    K = new Uint8Array(a);
    L = new Int32Array(a);
    F = new Uint32Array(a);
    H = new Float32Array(a);
}

n || (u = l.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), x());

var M = [], N = a => {
    a = a.data;
    let b = a._wsc;
    b && B.get(b)(...a.x);
}, O = a => {
    M.push(a);
}, Q = a => I(a), R = () => C(), aa = (a, b, c, e) => {
    b = S[b];
    S[a].connect(b.destination || b, c, e);
}, S = {}, T = 0, U = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, V = (a = 0) => {
    for (var b = K, c = a, e = c + void 0; b[c] && !(c >= e); ) ++c;
    if (16 < c - a && b.buffer && U) return U.decode(b.slice(a, c));
    for (e = ""; a < c; ) {
        var d = b[a++];
        if (d & 128) {
            var g = b[a++] & 63;
            if (192 == (d & 224)) e += String.fromCharCode((d & 31) << 6 | g); else {
                var h = b[a++] & 63;
                d = 224 == (d & 240) ? (d & 15) << 12 | g << 6 | h : (d & 7) << 18 | g << 12 | h << 6 | b[a++] & 63;
                65536 > d ? e += String.fromCharCode(d) : (d -= 65536, e += String.fromCharCode(55296 | d >> 10, 56320 | d & 1023));
            }
        } else e += String.fromCharCode(d);
    }
    return e;
}, ba = a => {
    var b = window.AudioContext || window.webkitAudioContext;
    if (a) {
        var c = F[a >> 2];
        a = {
            latencyHint: (c ? V(c) : "") || void 0,
            sampleRate: F[a + 4 >> 2] || void 0
        };
    } else a = void 0;
    if (c = b) b = new b(a), S[++T] = b, c = T;
    return c;
}, ca = (a, b, c, e, d) => {
    var g = c ? L[c + 4 >> 2] : 0;
    if (c) {
        var h = L[c >> 2];
        c = F[c + 8 >> 2];
        var p = g;
        if (c) {
            c >>= 2;
            for (var v = []; p--; ) v.push(F[c++]);
            c = v;
        } else c = void 0;
        e = {
            numberOfInputs: h,
            numberOfOutputs: g,
            outputChannelCount: c,
            processorOptions: {
                u: e,
                v: d,
                s: 128
            }
        };
    } else e = void 0;
    a = new AudioWorkletNode(S[a], b ? V(b) : "", e);
    S[++T] = a;
    return T;
}, da = (a, b, c, e) => {
    var d = [], g = (g = F[b >> 2]) ? V(g) : "", h = L[b + 4 >> 2];
    b = F[b + 8 >> 2];
    for (var p = 0; h--; ) d.push({
        name: p++,
        defaultValue: H[b >> 2],
        minValue: H[b + 4 >> 2],
        maxValue: H[b + 8 >> 2],
        automationRate: (L[b + 12 >> 2] ? "k" : "a") + "-rate"
    }), b += 16;
    S[a].audioWorklet.B.port.postMessage({
        _wpn: g,
        D: d,
        F: a,
        u: c,
        v: e
    });
}, ea = () => !1, fa = 1, ha = a => {
    a = a.data;
    var b = a._wsc;
    b && B.get(b)(...a.A);
}, ia = a => D(a), ja = (a, b, c, e, d) => {
    var g = S[a], h = g.audioWorklet, p = () => {
        B.get(e)(a, 0, d);
    };
    if (!h) return p();
    h.addModule(l.js).then((() => {
        h.B = new AudioWorkletNode(g, "em-bootstrap", {
            processorOptions: {
                K: fa++,
                C: l.wasm,
                I: u,
                G: b,
                H: c
            }
        });
        h.B.port.onmessage = ha;
        B.get(e)(a, 1, d);
    })).catch(p);
};

function ka(a) {
    let b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = S[a];
    b.onclick = () => {
        "running" != a.state ? a.resume() : a.suspend();
    };
}

function y() {
    Z = {
        f: ka,
        g: aa,
        d: ba,
        h: ca,
        e: da,
        b: ea,
        c: ja,
        a: u
    };
    WebAssembly.instantiate(l.wasm, {
        a: Z
    }).then((a => {
        a = (a.instance || a).exports;
        X = a.j;
        I = a.l;
        D = a.m;
        C = a.n;
        Y = a.o;
        B = a.k;
        l.stackSave = R;
        l.stackAlloc = ia;
        l.stackRestore = Q;
        l.wasmTable = B;
        n ? (Y(r.G, r.H), "undefined" === typeof AudioWorkletGlobalScope && (removeEventListener("message", O), 
        M = M.forEach(N), addEventListener("message", N))) : a.i();
        n || X();
    }));
}

n || y();