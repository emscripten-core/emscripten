var l = globalThis.Module || "undefined" != typeof Module ? Module : {}, n = "em-ww" == globalThis.name, q = "undefined" !== typeof AudioWorkletGlobalScope, t, v, J, K, E, F, A, X, H, C, B, Y, Z;

q && (n = !0);

function u(a) {
    t = a;
    v = a.I;
    y();
    l ||= {};
    l.wasm = a.C;
    z();
    a.C = a.J = 0;
}

n && !q && (onmessage = a => {
    onmessage = null;
    u(a.data);
});

if (q) {
    function a(c) {
        class e extends AudioWorkletProcessor {
            constructor(d) {
                super();
                d = d.processorOptions;
                this.u = A.get(d.u);
                this.v = d.v;
                this.s = d.s;
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, g, h) {
                var p = d.length, w = g.length, k, r, x = 4 * this.s, f = 12 * (p + w), D = 0;
                for (k of d) f += k.length * x;
                for (k of g) f += k.length * x;
                for (k in h) f += h[k].byteLength + 8, ++D;
                var V = B(), G = C(f);
                f = G;
                var m = G + 12 * p;
                for (k of d) {
                    E[f >> 2] = k.length;
                    E[f + 4 >> 2] = this.s;
                    E[f + 8 >> 2] = m;
                    f += 12;
                    for (r of k) F.set(r, m >> 2), m += x;
                }
                var O = m;
                f = O;
                d = m += 12 * w;
                for (k of g) E[f >> 2] = k.length, E[f + 4 >> 2] = this.s, E[f + 8 >> 2] = m, f += 12, 
                m += x * k.length;
                f = x = m;
                m += 8 * D;
                for (k = 0; r = h[k++]; ) E[f >> 2] = r.length, E[f + 4 >> 2] = m, f += 8, F.set(r, m >> 2), 
                m += 4 * r.length;
                if (h = this.u(p, G, w, O, D, x, this.v)) {
                    d >>= 2;
                    for (k of g) for (r of k) for (f = 0; f < this.s; ++f) r[f] = F[d++];
                }
                H(V);
                return !!h;
            }
        }
        return e;
    }
    var I;
    class b extends AudioWorkletProcessor {
        constructor(c) {
            super();
            u(c.processorOptions);
            I = this.port;
            I.onmessage = async e => {
                e = e.data;
                e._wpn ? (registerProcessor(e._wpn, a(e.D)), I.postMessage({
                    _wsc: e.u,
                    A: [ e.F, 1, e.v ]
                })) : e._wsc && A.get(e._wsc)(...e.A);
            };
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
}

function y() {
    var a = v.buffer;
    J = new Uint8Array(a);
    K = new Int32Array(a);
    E = new Uint32Array(a);
    F = new Float32Array(a);
}

n || (v = l.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), y());

var L = [], M = a => {
    a = a.data;
    let b = a._wsc;
    b && A.get(b)(...a.x);
}, N = a => {
    L.push(a);
}, P = a => H(a), Q = () => B(), S = (a, b, c, e) => {
    b = R[b];
    R[a].connect(b.destination || b, c, e);
}, R = {}, T = 0, U = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, W = (a = 0) => {
    for (var b = J, c = a, e = c + void 0; b[c] && !(c >= e); ) ++c;
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
}, aa = a => {
    var b = window.AudioContext || window.webkitAudioContext;
    if (a) {
        var c = E[a >> 2];
        a = {
            latencyHint: (c ? W(c) : "") || void 0,
            sampleRate: E[a + 4 >> 2] || void 0
        };
    } else a = void 0;
    if (c = b) b = new b(a), R[++T] = b, c = T;
    return c;
}, ba = (a, b, c, e, d) => {
    var g = c ? K[c + 4 >> 2] : 0;
    if (c) {
        var h = K[c >> 2];
        c = E[c + 8 >> 2];
        var p = g;
        if (c) {
            c >>= 2;
            for (var w = []; p--; ) w.push(E[c++]);
            c = w;
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
    a = new AudioWorkletNode(R[a], b ? W(b) : "", e);
    R[++T] = a;
    return T;
}, ca = (a, b, c, e) => {
    var d = [], g = (g = E[b >> 2]) ? W(g) : "", h = K[b + 4 >> 2];
    b = E[b + 8 >> 2];
    for (var p = 0; h--; ) d.push({
        name: p++,
        defaultValue: F[b >> 2],
        minValue: F[b + 4 >> 2],
        maxValue: F[b + 8 >> 2],
        automationRate: (K[b + 12 >> 2] ? "k" : "a") + "-rate"
    }), b += 16;
    R[a].audioWorklet.B.port.postMessage({
        _wpn: g,
        D: d,
        F: a,
        u: c,
        v: e
    });
}, da = () => !1, ea = 1, fa = a => {
    a = a.data;
    var b = a._wsc;
    b && A.get(b)(...a.A);
}, ha = a => C(a), ia = (a, b, c, e, d) => {
    var g = R[a], h = g.audioWorklet, p = () => {
        A.get(e)(a, 0, d);
    };
    if (!h) return p();
    h.addModule(l.js).then((() => {
        h.B = new AudioWorkletNode(g, "em-bootstrap", {
            processorOptions: {
                K: ea++,
                C: l.wasm,
                I: v,
                G: b,
                H: c
            }
        });
        h.B.port.onmessage = fa;
        A.get(e)(a, 1, d);
    })).catch(p);
};

function ja(a) {
    let b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = R[a];
    b.onclick = () => {
        "running" != a.state ? a.resume() : a.suspend();
    };
}

function z() {
    Z = {
        f: ja,
        g: S,
        d: aa,
        h: ba,
        e: ca,
        b: da,
        c: ia,
        a: v
    };
    WebAssembly.instantiate(l.wasm, {
        a: Z
    }).then((a => {
        a = (a.instance || a).exports;
        X = a.j;
        H = a.l;
        C = a.m;
        B = a.n;
        Y = a.o;
        A = a.k;
        l.stackSave = Q;
        l.stackAlloc = ha;
        l.stackRestore = P;
        l.wasmTable = A;
        n ? (Y(t.G, t.H), "undefined" === typeof AudioWorkletGlobalScope && (removeEventListener("message", N), 
        L = L.forEach(M), addEventListener("message", M))) : a.i();
        n || X();
    }));
}

n || z();