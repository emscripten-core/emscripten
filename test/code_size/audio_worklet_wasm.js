var l = globalThis.Module || "undefined" != typeof Module ? Module : {}, n = "em-ww" == globalThis.name, q = "undefined" !== typeof AudioWorkletGlobalScope, t, v, I, J, F, C, z, X, E, B, A, Y, Z;

q && (n = !0);

function u(a) {
    t = a;
    v = a.L;
    w();
    l ||= {};
    l.wasm = a.H;
    y();
    a.H = a.M = 0;
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
                this.v = z.get(d.v);
                this.A = d.A;
                this.u = d.u;
                this.s = 4 * this.u;
                this.F = Math.min((t.D - 16) / this.s | 0, 16);
                d = A();
                var f = B(this.F * this.s) >> 2;
                this.G = [];
                for (var g = this.F; 0 < g; g--) this.G.unshift(C.subarray(f, f += this.u));
                E(d);
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, f, g) {
                var p = d.length, x = f.length, k, r, h = 12 * (p + x), m = 0;
                for (k of d) m += k.length * this.s;
                var G = 0;
                for (k of f) G += k.length;
                m += G * this.s;
                var N = 0;
                for (k in g) ++N, h += 8, m += g[k].byteLength;
                var U = A(), D = h + m + 15 & -16;
                h = B(D);
                m = h + (D - m);
                D = h;
                for (k of d) {
                    F[h >> 2] = k.length;
                    F[h + 4 >> 2] = this.u;
                    F[h + 8 >> 2] = m;
                    h += 12;
                    for (r of k) C.set(r, m >> 2), m += this.s;
                }
                d = h;
                for (k = 0; r = g[k++]; ) F[h >> 2] = r.length, F[h + 4 >> 2] = m, h += 8, C.set(r, m >> 2), 
                m += 4 * r.length;
                g = h;
                for (k of f) F[h >> 2] = k.length, F[h + 4 >> 2] = this.u, F[h + 8 >> 2] = m, h += 12, 
                m += this.s * k.length;
                if (p = this.v(p, D, x, g, N, d, this.A)) for (k of f) for (r of k) r.set(this.G[--G]);
                E(U);
                return !!p;
            }
        }
        return e;
    }
    var H;
    class b extends AudioWorkletProcessor {
        constructor(c) {
            super();
            u(c.processorOptions);
            H = this.port;
            H.onmessage = async e => {
                e = e.data;
                e._wpn ? (registerProcessor(e._wpn, a(e.I)), H.postMessage({
                    _wsc: e.v,
                    B: [ e.J, 1, e.A ]
                })) : e._wsc && z.get(e._wsc)(...e.B);
            };
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
}

function w() {
    var a = v.buffer;
    I = new Uint8Array(a);
    J = new Int32Array(a);
    F = new Uint32Array(a);
    C = new Float32Array(a);
}

n || (v = l.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), w());

var K = [], L = a => {
    a = a.data;
    let b = a._wsc;
    b && z.get(b)(...a.x);
}, M = a => {
    K.push(a);
}, O = a => E(a), P = () => A(), R = (a, b, c, e) => {
    b = Q[b];
    Q[a].connect(b.destination || b, c, e);
}, Q = {}, S = 0, T = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, V = (a = 0) => {
    for (var b = I, c = a, e = c + void 0; b[c] && !(c >= e); ) ++c;
    if (16 < c - a && b.buffer && T) return T.decode(b.slice(a, c));
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
}, W = a => {
    var b = window.AudioContext || window.webkitAudioContext;
    if (a) {
        var c = F[a >> 2];
        a = {
            latencyHint: (c ? V(c) : "") || void 0,
            sampleRate: F[a + 4 >> 2] || void 0
        };
    } else a = void 0;
    if (c = b) b = new b(a), Q[++S] = b, c = S;
    return c;
}, aa = (a, b, c, e, d) => {
    var f = c ? J[c + 4 >> 2] : 0;
    if (c) {
        var g = J[c >> 2];
        c = F[c + 8 >> 2];
        var p = f;
        if (c) {
            c >>= 2;
            for (var x = []; p--; ) x.push(F[c++]);
            c = x;
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
    a = new AudioWorkletNode(Q[a], b ? V(b) : "", e);
    Q[++S] = a;
    return S;
}, ba = (a, b, c, e) => {
    var d = [], f = (f = F[b >> 2]) ? V(f) : "", g = J[b + 4 >> 2];
    b = F[b + 8 >> 2];
    for (var p = 0; g--; ) d.push({
        name: p++,
        defaultValue: C[b >> 2],
        minValue: C[b + 4 >> 2],
        maxValue: C[b + 8 >> 2],
        automationRate: (J[b + 12 >> 2] ? "k" : "a") + "-rate"
    }), b += 16;
    Q[a].audioWorklet.C.port.postMessage({
        _wpn: f,
        I: d,
        J: a,
        v: c,
        A: e
    });
}, ca = () => !1, da = 1, ea = a => {
    a = a.data;
    var b = a._wsc;
    b && z.get(b)(...a.B);
}, fa = a => B(a), ha = (a, b, c, e, d) => {
    var f = Q[a], g = f.audioWorklet, p = () => {
        z.get(e)(a, 0, d);
    };
    if (!g) return p();
    g.addModule(l.js).then((() => {
        g.C = new AudioWorkletNode(f, "em-bootstrap", {
            processorOptions: {
                N: da++,
                H: l.wasm,
                L: v,
                K: b,
                D: c
            }
        });
        g.C.port.onmessage = ea;
        z.get(e)(a, 1, d);
    })).catch(p);
};

function ia(a) {
    let b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = Q[a];
    b.onclick = () => {
        "running" != a.state ? a.resume() : a.suspend();
    };
}

function y() {
    Z = {
        f: ia,
        g: R,
        d: W,
        h: aa,
        e: ba,
        b: ca,
        c: ha,
        a: v
    };
    WebAssembly.instantiate(l.wasm, {
        a: Z
    }).then((a => {
        a = (a.instance || a).exports;
        X = a.j;
        E = a.l;
        B = a.m;
        A = a.n;
        Y = a.o;
        z = a.k;
        l.stackSave = P;
        l.stackAlloc = fa;
        l.stackRestore = O;
        l.wasmTable = z;
        n ? (Y(t.K, t.D), "undefined" === typeof AudioWorkletGlobalScope && (removeEventListener("message", M), 
        K = K.forEach(L), addEventListener("message", L))) : a.i();
        n || X();
    }));
}

n || y();