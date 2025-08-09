var k = globalThis.Module || "undefined" != typeof Module ? Module : {}, p = "em-ww" == globalThis.name, q = "undefined" !== typeof AudioWorkletGlobalScope, t, v, J, K, F, D, A, X, E, C, B, Y, Z;

q && (p = !0);

function u(a) {
    t = a;
    v = a.L;
    x();
    k ||= {};
    k.wasm = a.H;
    z();
    a.H = a.M = 0;
}

p && !q && (onmessage = a => {
    onmessage = null;
    u(a.data);
});

if (q) {
    function a(c) {
        class e extends AudioWorkletProcessor {
            constructor(d) {
                super();
                d = d.processorOptions;
                this.v = A.get(d.v);
                this.A = d.A;
                this.u = d.u;
                this.s = 4 * this.u;
                this.F = Math.min((t.D - 16) / this.s | 0, 16);
                d = B();
                var f = C(this.F * this.s) >> 2;
                this.G = [];
                for (var g = this.F; 0 < g; g--) this.G.unshift(D.subarray(f, f += this.u));
                E(d);
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, f, g) {
                var n = d.length, w = f.length, h, r, y = 12 * (n + w);
                for (h of d) y += h.length * this.s;
                var G = 0;
                for (h of f) G += h.length;
                y += G * this.s;
                var H = 0;
                for (h in g) y += g[h].byteLength + 8, ++H;
                var W = B(), O = y + 15 & -16, l = C(O), P = l, m = P;
                l += 12 * n;
                for (h of d) {
                    F[m >> 2] = h.length;
                    F[m + 4 >> 2] = this.u;
                    F[m + 8 >> 2] = l;
                    m += 12;
                    for (r of h) D.set(r, l >> 2), l += this.s;
                }
                m = d = l;
                l += 8 * H;
                for (h = 0; r = g[h++]; ) F[m >> 2] = r.length, F[m + 4 >> 2] = l, m += 8, D.set(r, l >> 2), 
                l += 4 * r.length;
                m = g = l += O - y;
                l += 12 * w;
                for (h of f) F[m >> 2] = h.length, F[m + 4 >> 2] = this.u, F[m + 8 >> 2] = l, m += 12, 
                l += this.s * h.length;
                if (n = this.v(n, P, w, g, H, d, this.A)) for (h of f) for (r of h) r.set(this.G[--G]);
                E(W);
                return !!n;
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
                e._wpn ? (registerProcessor(e._wpn, a(e.I)), I.postMessage({
                    _wsc: e.v,
                    B: [ e.J, 1, e.A ]
                })) : e._wsc && A.get(e._wsc)(...e.B);
            };
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
}

function x() {
    var a = v.buffer;
    J = new Uint8Array(a);
    K = new Int32Array(a);
    F = new Uint32Array(a);
    D = new Float32Array(a);
}

p || (v = k.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), x());

var L = [], M = a => {
    a = a.data;
    let b = a._wsc;
    b && A.get(b)(...a.x);
}, N = a => {
    L.push(a);
}, Q = a => E(a), R = () => B(), aa = (a, b, c, e) => {
    b = S[b];
    S[a].connect(b.destination || b, c, e);
}, S = {}, T = 0, U = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, V = (a = 0) => {
    for (var b = J, c = a, e = c + void 0; b[c] && !(c >= e); ) ++c;
    if (16 < c - a && b.buffer && U) return U.decode(b.slice(a, c));
    for (e = ""; a < c; ) {
        var d = b[a++];
        if (d & 128) {
            var f = b[a++] & 63;
            if (192 == (d & 224)) e += String.fromCharCode((d & 31) << 6 | f); else {
                var g = b[a++] & 63;
                d = 224 == (d & 240) ? (d & 15) << 12 | f << 6 | g : (d & 7) << 18 | f << 12 | g << 6 | b[a++] & 63;
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
    var f = c ? K[c + 4 >> 2] : 0;
    if (c) {
        var g = K[c >> 2];
        c = F[c + 8 >> 2];
        var n = f;
        if (c) {
            c >>= 2;
            for (var w = []; n--; ) w.push(F[c++]);
            c = w;
        } else c = void 0;
        e = {
            numberOfInputs: g,
            numberOfOutputs: f,
            outputChannelCount: c,
            processorOptions: {
                v: e,
                A: d,
                u: 128
            }
        };
    } else e = void 0;
    a = new AudioWorkletNode(S[a], b ? V(b) : "", e);
    S[++T] = a;
    return T;
}, da = (a, b, c, e) => {
    var d = [], f = (f = F[b >> 2]) ? V(f) : "", g = K[b + 4 >> 2];
    b = F[b + 8 >> 2];
    for (var n = 0; g--; ) d.push({
        name: n++,
        defaultValue: D[b >> 2],
        minValue: D[b + 4 >> 2],
        maxValue: D[b + 8 >> 2],
        automationRate: (K[b + 12 >> 2] ? "k" : "a") + "-rate"
    }), b += 16;
    S[a].audioWorklet.C.port.postMessage({
        _wpn: f,
        I: d,
        J: a,
        v: c,
        A: e
    });
}, ea = () => !1, fa = 1, ha = a => {
    a = a.data;
    var b = a._wsc;
    b && A.get(b)(...a.B);
}, ia = a => C(a), ja = (a, b, c, e, d) => {
    var f = S[a], g = f.audioWorklet, n = () => {
        A.get(e)(a, 0, d);
    };
    if (!g) return n();
    g.addModule(k.js).then((() => {
        g.C = new AudioWorkletNode(f, "em-bootstrap", {
            processorOptions: {
                N: fa++,
                H: k.wasm,
                L: v,
                K: b,
                D: c
            }
        });
        g.C.port.onmessage = ha;
        A.get(e)(a, 1, d);
    })).catch(n);
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

function z() {
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
        a = (a.instance || a).exports;
        X = a.j;
        E = a.l;
        C = a.m;
        B = a.n;
        Y = a.o;
        A = a.k;
        k.stackSave = R;
        k.stackAlloc = ia;
        k.stackRestore = Q;
        k.wasmTable = A;
        p ? (Y(t.K, t.D), "undefined" === typeof AudioWorkletGlobalScope && (removeEventListener("message", N), 
        L = L.forEach(M), addEventListener("message", M))) : a.i();
        p || X();
    }));
}

p || z();