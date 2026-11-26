var m = globalThis.Module || typeof Module !== "undefined" ? Module : {}, r = !!globalThis.WorkerGlobalScope, t = !!globalThis.AudioWorkletGlobalScope, u = globalThis.name == "em-ww" || t, v, x, K, L, E, G, J, W, P, F, D, C, X, A, Y, H;

function w(a) {
    v = a;
    x = a.J;
    y();
    m ||= {};
    m.wasm = a.I;
    z();
    a.I = a.J = 0;
}

u && !t && (onmessage = a => {
    onmessage = null;
    w(a.data);
});

if (t) {
    function a(c) {
        class k extends AudioWorkletProcessor {
            constructor(d) {
                super();
                d = d.processorOptions;
                this.B = A.get(d.B);
                this.C = d.C;
                this.v = d.v;
                this.A = this.v * 4;
                this.D = Array(Math.min((v.H - 16) / this.A | 0, 64));
                this.K();
            }
            K() {
                for (var d = C(), g = D(this.D.length * this.A) >> 2, f = this.D.length - 1; f >= 0; f--) this.D[f] = E.subarray(g, g += this.v);
                F(d);
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, g, f) {
                var l = d.length, n = g.length, e, q, h = (l + n) * 12, p = 0;
                for (e of d) p += e.length;
                p *= this.A;
                var I = 0;
                for (e of g) I += e.length;
                p += I * this.A;
                var O = 0;
                for (e in f) ++O, h += 8, p += f[e].byteLength;
                var Z = C(), B = h + p + 15 & -16;
                h = D(B);
                p = h + (B - p);
                B = h;
                for (e of d) {
                    G[h >> 2] = e.length;
                    G[h + 4 >> 2] = this.v;
                    G[h + 8 >> 2] = p;
                    h += 12;
                    for (q of e) E.set(q, p >> 2), p += this.A;
                }
                d = h;
                for (e = 0; q = f[e++]; ) G[h >> 2] = q.length, G[h + 4 >> 2] = p, h += 8, E.set(q, p >> 2), 
                p += q.length * 4;
                f = h;
                for (e of g) G[h >> 2] = e.length, G[h + 4 >> 2] = this.v, G[h + 8 >> 2] = p, h += 12, 
                p += this.A * e.length;
                if (l = this.B(l, B, n, f, O, d, this.C)) for (e of g) for (q of e) q.set(this.D[--I]);
                F(Z);
                return !!l;
            }
        }
        return k;
    }
    var port = globalThis.port || {};
    class b extends AudioWorkletProcessor {
        constructor(c) {
            super();
            w(c.processorOptions);
            port instanceof MessagePort || (this.port.onmessage = port.onmessage, port = this.port);
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
    port.onmessage = async c => {
        await H;
        c = c.data;
        c._boot ? w(c) : c._wpn ? (registerProcessor(c._wpn, a(c.L)), port.postMessage({
            _wsc: c.B,
            F: [ c.M, 1, c.C ]
        })) : c._wsc && A.get(c._wsc)(...c.F);
    };
}

function y() {
    var a = x.buffer;
    J = new Uint8Array(a);
    K = new Int32Array(a);
    G = new Uint32Array(a);
    E = new Float32Array(a);
    L = new BigInt64Array(a);
}

u || (x = new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), y());

var M = [], aa = a => {
    M.push(a);
}, N = ({data: a}) => {
    var b = a._wsc;
    b && A.get(b)(...a.x);
}, ba = () => {
    P(0, !r && !t, !u, r && !t);
}, ca = ({data: a}) => {
    var b = a._wsc;
    b && A.get(b)(...a.F);
}, Q = {}, da = (a, b, c, k, d, g, f) => {
    var l = Q[b], n = l.audioWorklet;
    d = () => {
        A.get(g)(b, 0, f);
    };
    if (!n) return d();
    n.addModule(m.js).then(() => {
        n.port || (n.port = {
            postMessage: e => {
                e._boot ? (n.G = new AudioWorkletNode(l, "em-bootstrap", {
                    processorOptions: e
                }), n.G.port.onmessage = q => {
                    n.port.onmessage(q);
                }) : n.G.port.postMessage(e);
            }
        });
        n.port.postMessage({
            _boot: 1,
            O: a,
            I: m.wasm,
            J: x,
            N: c,
            H: k
        });
        n.port.onmessage = ca;
        A.get(g)(b, 1, f);
    }).catch(d);
}, ea = !!globalThis.performance?.now, R = globalThis.performance?.now ? () => performance.now() : Date.now, fa = (a, b) => {
    if (!(a >= 0 && a <= 3)) return 28;
    if (a === 0) a = 1e6; else if (ea) a = globalThis.performance?.now ? 1e3 : 1e6; else return 52;
    L[b >> 3] = BigInt(a);
    return 0;
}, ha = (a, b, c, k) => {
    b = Q[b];
    Q[a].connect(b.destination || b, c, k);
}, S = globalThis.TextDecoder && new TextDecoder, T = (a = 0) => {
    for (var b = J, c = a, k = c + NaN; b[c] && !(c >= k); ) ++c;
    if (c - a > 16 && b.buffer && S) return S.decode(b.slice(a, c));
    for (k = ""; a < c; ) {
        var d = b[a++];
        if (d & 128) {
            var g = b[a++] & 63;
            if ((d & 224) == 192) k += String.fromCharCode((d & 31) << 6 | g); else {
                var f = b[a++] & 63;
                d = (d & 240) == 224 ? (d & 15) << 12 | g << 6 | f : (d & 7) << 18 | g << 12 | f << 6 | b[a++] & 63;
                d < 65536 ? k += String.fromCharCode(d) : (d -= 65536, k += String.fromCharCode(55296 | d >> 10, 56320 | d & 1023));
            }
        } else k += String.fromCharCode(d);
    }
    return k;
}, U = 0, ia = a => {
    if (a) {
        var b = G[a >> 2];
        b = (b ? T(b) : "") || void 0;
        var c = K[a + 8 >> 2];
        a = {
            latencyHint: b,
            sampleRate: G[a + 4 >> 2] || void 0,
            P: c < 0 ? "hardware" : c || "default"
        };
    } else a = void 0;
    a = new AudioContext(a);
    Q[++U] = a;
    return U;
}, ja = (a, b, c, k, d) => {
    var g = c ? K[c + 4 >> 2] : 0;
    if (c) {
        var f = K[c >> 2], l = G[c + 8 >> 2], n = g;
        if (l) {
            l >>= 2;
            for (var e = []; n--; ) e.push(G[l++]);
            l = e;
        } else l = void 0;
        c = {
            numberOfInputs: f,
            numberOfOutputs: g,
            outputChannelCount: l,
            channelCount: G[c + 12 >> 2] || void 0,
            channelCountMode: [ , "clamped-max", "explicit" ][K[c + 16 >> 2]],
            channelInterpretation: [ , "discrete" ][K[c + 20 >> 2]],
            processorOptions: {
                B: k,
                C: d,
                v: Q[a].renderQuantumSize || 128
            }
        };
    } else c = void 0;
    a = new AudioWorkletNode(Q[a], b ? T(b) : "", c);
    Q[++U] = a;
    return U;
}, ka = (a, b, c, k) => {
    var d = (d = G[b >> 2]) ? T(d) : "", g = K[b + 4 >> 2];
    b = G[b + 8 >> 2];
    for (var f = [], l = 0; g--; ) f.push({
        name: l++,
        defaultValue: E[b >> 2],
        minValue: E[b + 4 >> 2],
        maxValue: E[b + 8 >> 2],
        automationRate: (K[b + 12 >> 2] ? "k" : "a") + "-rate"
    }), b += 16;
    Q[a].audioWorklet.port.postMessage({
        _wpn: d,
        L: f,
        M: a,
        B: c,
        C: k
    });
}, la = a => {
    a = [ a ? T(a) : "" ];
    return console.log(...a);
}, ma = () => !1, na = () => t ? () => 138 : a => (a.set(crypto.getRandomValues(new Uint8Array(a.byteLength))), 
0), V = a => (V = na())(a), oa = (a, b) => V(J.subarray(a, a + b));

function pa(a) {
    var b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = Q[a];
    b.onclick = () => {
        a.state != "running" ? a.resume() : a.suspend();
    };
}

function z() {
    Y = {
        i: pa,
        d: ba,
        l: da,
        f: fa,
        j: ha,
        g: ia,
        m: ja,
        h: ka,
        b: R,
        c: la,
        k: ma,
        a: x,
        e: oa
    };
    H = WebAssembly.instantiate(m.wasm, {
        a: Y
    }).then(a => {
        a = (a.instance || a).exports;
        W = a.o;
        P = a.q;
        F = a.r;
        D = a.s;
        C = a.t;
        X = a.u;
        A = a.p;
        u ? (X(v.O, v.N, v.H), t || (removeEventListener("message", aa), M = M.forEach(N), 
        addEventListener("message", N))) : a.n();
        u || W();
    });
}

u || z();