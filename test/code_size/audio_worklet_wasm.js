var k = globalThis.Module || "undefined" != typeof Module ? Module : {}, p = "em-ww" == globalThis.name, q = "undefined" !== typeof AudioWorkletGlobalScope, r, v, L, M, G, I, C, X, J, E, D, Y, Z;

q && (p = !0);

function u(a) {
    r = a;
    v = a.I;
    x();
    k ||= {};
    k.wasm = a.C;
    y();
    a.C = a.J = 0;
}

p && !q && (onmessage = a => {
    onmessage = null;
    u(a.data);
});

if (q) {
    function a(c) {
        class d extends AudioWorkletProcessor {
            constructor(e) {
                super();
                e = e.processorOptions;
                this.u = C.get(e.u);
                this.v = e.v;
                this.s = e.s;
            }
            static get parameterDescriptors() {
                return c;
            }
            process(e, h, f) {
                let m = e.length, w = h.length, F = 0, l, z, n, t = 4 * this.s, g = 12 * (m + w), W = D(), A, H, B;
                for (l of e) g += l.length * t;
                for (l of h) g += l.length * t;
                for (l in f) g += f[l].byteLength + 8, ++F;
                A = E(g);
                g = A >> 2;
                n = A + 12 * m;
                for (l of e) {
                    G[g] = l.length;
                    G[g + 1] = this.s;
                    G[g + 2] = n;
                    g += 3;
                    for (z of l) I.set(z, n >> 2), n += t;
                }
                H = n;
                g = H >> 2;
                e = (n += 12 * w) >> 2;
                for (l of h) G[g] = l.length, G[g + 1] = this.s, G[g + 2] = n, g += 3, n += t * l.length;
                t = n;
                g = t >> 2;
                n += 8 * F;
                for (l = 0; B = f[l++]; ) G[g] = B.length, G[g + 1] = n, g += 2, I.set(B, n >> 2), 
                n += 4 * B.length;
                if (f = this.u(m, A, w, H, F, t, this.v)) for (l of h) for (z of l) for (g = 0; g < this.s; ++g) z[g] = I[e++];
                J(W);
                return !!f;
            }
        }
        return d;
    }
    var K;
    class b extends AudioWorkletProcessor {
        constructor(c) {
            super();
            u(c.processorOptions);
            K = this.port;
            K.onmessage = async d => {
                d = d.data;
                d._wpn ? (registerProcessor(d._wpn, a(d.D)), K.postMessage({
                    _wsc: d.u,
                    A: [ d.F, 1, d.v ]
                })) : d._wsc && C.get(d._wsc)(...d.A);
            };
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
}

function x() {
    var a = v.buffer;
    L = new Uint8Array(a);
    M = new Int32Array(a);
    G = new Uint32Array(a);
    I = new Float32Array(a);
}

p || (v = k.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), x());

var N = [], O = a => {
    a = a.data;
    let b = a._wsc;
    b && C.get(b)(...a.x);
}, P = a => {
    N.push(a);
}, Q = a => J(a), R = () => D(), aa = (a, b, c, d) => {
    b = S[b];
    S[a].connect(b.destination || b, c, d);
}, S = {}, T = 0, U = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, V = (a = 0) => {
    for (var b = L, c = a, d = c + void 0; b[c] && !(c >= d); ) ++c;
    if (16 < c - a && b.buffer && U) return U.decode(b.slice(a, c));
    for (d = ""; a < c; ) {
        var e = b[a++];
        if (e & 128) {
            var h = b[a++] & 63;
            if (192 == (e & 224)) d += String.fromCharCode((e & 31) << 6 | h); else {
                var f = b[a++] & 63;
                e = 224 == (e & 240) ? (e & 15) << 12 | h << 6 | f : (e & 7) << 18 | h << 12 | f << 6 | b[a++] & 63;
                65536 > e ? d += String.fromCharCode(e) : (e -= 65536, d += String.fromCharCode(55296 | e >> 10, 56320 | e & 1023));
            }
        } else d += String.fromCharCode(e);
    }
    return d;
}, ba = a => {
    var b = window.AudioContext || window.webkitAudioContext;
    if (a >>= 2) {
        var c = G[a] ? (c = G[a]) ? V(c) : "" : void 0;
        a = {
            latencyHint: c,
            sampleRate: M[a + 1] || void 0
        };
    } else a = void 0;
    if (c = b) b = new b(a), S[++T] = b, c = T;
    return c;
}, ca = (a, b, c, d, e) => {
    if (c >>= 2) {
        var h = M[c], f = M[c + 1];
        if (G[c + 2]) {
            var m = G[c + 2] >> 2;
            c = M[c + 1];
            let w = [];
            for (;c--; ) w.push(G[m++]);
            m = w;
        } else m = void 0;
        d = {
            numberOfInputs: h,
            numberOfOutputs: f,
            outputChannelCount: m,
            processorOptions: {
                u: d,
                v: e,
                s: 128
            }
        };
    } else d = void 0;
    a = new AudioWorkletNode(S[a], b ? V(b) : "", d);
    S[++T] = a;
    return T;
}, da = (a, b, c, d) => {
    b >>= 2;
    let e = [], h = G[b + 1], f = G[b + 2] >> 2, m = 0;
    for (;h--; ) e.push({
        name: m++,
        defaultValue: I[f++],
        minValue: I[f++],
        maxValue: I[f++],
        automationRate: [ "a", "k" ][G[f++]] + "-rate"
    });
    h = S[a].audioWorklet.B.port;
    f = h.postMessage;
    b = (b = G[b]) ? V(b) : "";
    f.call(h, {
        _wpn: b,
        D: e,
        F: a,
        u: c,
        v: d
    });
}, ea = () => !1, fa = 1, ha = a => {
    a = a.data;
    let b = a._wsc;
    b && C.get(b)(...a.A);
}, ia = a => E(a), ja = (a, b, c, d, e) => {
    let h = S[a], f = h.audioWorklet, m = () => {
        C.get(d)(a, 0, e);
    };
    if (!f) return m();
    f.addModule(k.js).then((() => {
        f.B = new AudioWorkletNode(h, "em-bootstrap", {
            processorOptions: {
                K: fa++,
                C: k.wasm,
                I: v,
                G: b,
                H: c
            }
        });
        f.B.port.onmessage = ha;
        C.get(d)(a, 1, e);
    })).catch(m);
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
        a: v
    };
    WebAssembly.instantiate(k.wasm, {
        a: Z
    }).then((a => {
        a = a.instance.exports;
        X = a.j;
        J = a.l;
        E = a.m;
        D = a.n;
        Y = a.o;
        C = a.k;
        k.stackSave = R;
        k.stackAlloc = ia;
        k.stackRestore = Q;
        k.wasmTable = C;
        p ? (Y(r.G, r.H), "undefined" === typeof AudioWorkletGlobalScope && (removeEventListener("message", P), 
        N = N.forEach(O), addEventListener("message", O))) : a.i();
        p || X();
    }));
}

p || y();