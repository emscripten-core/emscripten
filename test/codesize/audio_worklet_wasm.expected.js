var m = globalThis.Module || typeof Module != "undefined" ? Module : {}, r = !!globalThis.WorkerGlobalScope, t = !!globalThis.AudioWorkletGlobalScope, u = globalThis.name == "em-ww" || t, v, K, E, G, J, x, W, P, F, D, C, X, A, Z, H;

function w(a) {
    v = a;
    x = a.I;
    y();
    m ||= {};
    m.wasm = a.H;
    z();
    a.H = a.I = 0;
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
                this.A = A.get(d.A);
                this.B = d.B;
                this.u = d.u;
                this.v = this.u * 4;
                this.C = Array(Math.min((v.G - 16) / this.v | 0, 64));
                this.J();
            }
            J() {
                for (var d = C(), g = D(this.C.length * this.v) >> 2, f = this.C.length - 1; f >= 0; f--) this.C[f] = E.subarray(g, g += this.u);
                F(d);
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, g, f) {
                var l = d.length, n = g.length, e, q, h = (l + n) * 12, p = 0;
                for (e of d) p += e.length;
                p *= this.v;
                var I = 0;
                for (e of g) I += e.length;
                p += I * this.v;
                var O = 0;
                for (e in f) ++O, h += 8, p += f[e].byteLength;
                var Y = C(), B = h + p + 15 & -16;
                h = D(B);
                p = h + (B - p);
                B = h;
                for (e of d) {
                    G[h >> 2] = e.length;
                    G[h + 4 >> 2] = this.u;
                    G[h + 8 >> 2] = p;
                    h += 12;
                    for (q of e) E.set(q, p >> 2), p += this.v;
                }
                d = h;
                for (e = 0; q = f[e++]; ) G[h >> 2] = q.length, G[h + 4 >> 2] = p, h += 8, E.set(q, p >> 2), 
                p += q.length * 4;
                f = h;
                for (e of g) G[h >> 2] = e.length, G[h + 4 >> 2] = this.u, G[h + 8 >> 2] = p, h += 12, 
                p += this.v * e.length;
                if (l = this.A(l, B, n, f, O, d, this.B)) for (e of g) for (q of e) q.set(this.C[--I]);
                F(Y);
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
        c._boot ? w(c) : c._wpn ? (registerProcessor(c._wpn, a(c.K)), port.postMessage({
            _wsc: c.A,
            D: [ c.L, 1, c.B ]
        })) : c._wsc && A.get(c._wsc)(...c.D);
    };
}

function y() {
    var a = x.buffer;
    J = new Uint8Array(a);
    K = new Int32Array(a);
    G = new Uint32Array(a);
    E = new Float32Array(a);
}

u || (x = m.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), y());

var L = [], M = a => {
    a = a.data;
    var b = a._wsc;
    b && A.get(b)(...a.x);
}, N = a => {
    L.push(a);
}, aa = () => {
    P(0, !r && !t, !u, r && !t);
}, ba = a => {
    a = a.data;
    var b = a._wsc;
    b && A.get(b)(...a.D);
}, ca = (a, b, c, k, d, g, f) => {
    var l = Q[b], n = l.audioWorklet;
    d = () => {
        A.get(g)(b, 0, f);
    };
    if (!n) return d();
    n.addModule(m.js).then((() => {
        n.port || (n.port = {
            postMessage: e => {
                e._boot ? (n.F = new AudioWorkletNode(l, "em-bootstrap", {
                    processorOptions: e
                }), n.F.port.onmessage = q => {
                    n.port.onmessage(q);
                }) : n.F.port.postMessage(e);
            }
        });
        n.port.postMessage({
            _boot: 1,
            N: a,
            H: m.wasm,
            I: x,
            M: c,
            G: k
        });
        n.port.onmessage = ba;
        A.get(g)(b, 1, f);
    })).catch(d);
}, da = (a, b, c, k) => {
    b = Q[b];
    Q[a].connect(b.destination || b, c, k);
}, Q = {}, R = 0, S = globalThis.TextDecoder && new TextDecoder, T = (a = 0) => {
    for (var b = J, c = a, k = c + void 0; b[c] && !(c >= k); ) ++c;
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
}, ea = a => {
    if (a) {
        var b = G[a >> 2];
        b = (b ? T(b) : "") || void 0;
        var c = K[a + 8 >> 2];
        a = {
            latencyHint: b,
            sampleRate: G[a + 4 >> 2] || void 0,
            O: c < 0 ? "hardware" : c || "default"
        };
    } else a = void 0;
    a = new AudioContext(a);
    Q[++R] = a;
    return R;
}, fa = (a, b, c, k, d) => {
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
                A: k,
                B: d,
                u: Q[a].renderQuantumSize || 128
            }
        };
    } else c = void 0;
    a = new AudioWorkletNode(Q[a], b ? T(b) : "", c);
    Q[++R] = a;
    return R;
}, ha = (a, b, c, k) => {
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
        K: f,
        L: a,
        A: c,
        B: k
    });
}, U = globalThis.performance?.now ? () => performance.now() : Date.now, ia = a => {
    a = [ a ? T(a) : "" ];
    return console.log(...a);
}, ja = () => !1, ka = () => t ? () => 138 : a => (a.set(crypto.getRandomValues(new Uint8Array(a.byteLength))), 
0), V = a => (V = ka())(a), la = (a, b) => V(J.subarray(a, a + b));

function ma(a) {
    var b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = Q[a];
    b.onclick = () => {
        a.state != "running" ? a.resume() : a.suspend();
    };
}

function z() {
    Z = {
        i: ma,
        e: aa,
        d: ca,
        k: da,
        g: ea,
        l: fa,
        h: ha,
        b: U,
        c: ia,
        j: ja,
        a: x,
        f: la
    };
    H = WebAssembly.instantiate(m.wasm, {
        a: Z
    }).then((a => {
        a = (a.instance || a).exports;
        W = a.n;
        P = a.p;
        F = a.q;
        D = a.r;
        C = a.s;
        X = a.t;
        A = a.o;
        u ? (X(v.N, v.M, v.G), t || (removeEventListener("message", N), L = L.forEach(M), 
        addEventListener("message", M))) : a.m();
        u || W();
    }));
}

u || z();