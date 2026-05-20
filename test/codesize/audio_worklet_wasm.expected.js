var m = globalThis.Module || typeof Module != "undefined" ? Module : {}, r = !!globalThis.AudioWorkletGlobalScope, t = globalThis.name == "em-ww" || r, u, z, J, E, G, I, w, X, F, D, C, Y, A, Z;

function v(a) {
    u = a;
    w = a.I;
    x();
    m ||= {};
    m.wasm = a.H;
    y();
    a.H = a.I = 0;
}

t && !r && (onmessage = a => {
    onmessage = null;
    v(a.data);
});

if (r) {
    function a(c) {
        class k extends AudioWorkletProcessor {
            constructor(d) {
                super();
                d = d.processorOptions;
                this.A = A.get(d.A);
                this.B = d.B;
                this.u = d.u;
                this.v = this.u * 4;
                this.C = Array(Math.min((u.G - 16) / this.v | 0, 64));
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
                var H = 0;
                for (e of g) H += e.length;
                p += H * this.v;
                var O = 0;
                for (e in f) ++O, h += 8, p += f[e].byteLength;
                var V = C(), B = h + p + 15 & -16;
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
                if (l = this.A(l, B, n, f, O, d, this.B)) for (e of g) for (q of e) q.set(this.C[--H]);
                F(V);
                return !!l;
            }
        }
        return k;
    }
    var port = globalThis.port || {};
    class b extends AudioWorkletProcessor {
        constructor(c) {
            super();
            v(c.processorOptions);
            port instanceof MessagePort || (this.port.onmessage = port.onmessage, port = this.port);
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
    port.onmessage = async c => {
        await z;
        c = c.data;
        c._boot ? v(c) : c._wpn ? (registerProcessor(c._wpn, a(c.K)), port.postMessage({
            _wsc: c.A,
            D: [ c.L, 1, c.B ]
        })) : c._wsc && A.get(c._wsc)(...c.D);
    };
}

function x() {
    var a = w.buffer;
    I = new Uint8Array(a);
    J = new Int32Array(a);
    G = new Uint32Array(a);
    E = new Float32Array(a);
}

t || (w = m.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), x());

var K = [], L = a => {
    a = a.data;
    var b = a._wsc;
    b && A.get(b)(...a.x);
}, M = a => {
    K.push(a);
}, N = a => {
    a = a.data;
    var b = a._wsc;
    b && A.get(b)(...a.D);
}, Q = (a, b, c, k, d, g, f) => {
    var l = P[b], n = l.audioWorklet;
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
            I: w,
            M: c,
            G: k
        });
        n.port.onmessage = N;
        A.get(g)(b, 1, f);
    })).catch(d);
}, R = (a, b, c, k) => {
    b = P[b];
    P[a].connect(b.destination || b, c, k);
}, P = {}, S = 0, T = globalThis.TextDecoder && new TextDecoder, U = (a = 0) => {
    for (var b = I, c = a, k = c + void 0; b[c] && !(c >= k); ) ++c;
    if (c - a > 16 && b.buffer && T) return T.decode(b.slice(a, c));
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
}, aa = a => {
    if (a) {
        var b = G[a >> 2];
        b = (b ? U(b) : "") || void 0;
        var c = J[a + 8 >> 2];
        a = {
            latencyHint: b,
            sampleRate: G[a + 4 >> 2] || void 0,
            O: c < 0 ? "hardware" : c || "default"
        };
    } else a = void 0;
    a = new AudioContext(a);
    P[++S] = a;
    return S;
}, ba = (a, b, c, k, d) => {
    var g = c ? J[c + 4 >> 2] : 0;
    if (c) {
        var f = J[c >> 2], l = G[c + 8 >> 2], n = g;
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
            channelCountMode: [ , "clamped-max", "explicit" ][J[c + 16 >> 2]],
            channelInterpretation: [ , "discrete" ][J[c + 20 >> 2]],
            processorOptions: {
                A: k,
                B: d,
                u: P[a].renderQuantumSize || 128
            }
        };
    } else c = void 0;
    a = new AudioWorkletNode(P[a], b ? U(b) : "", c);
    P[++S] = a;
    return S;
}, ca = (a, b, c, k) => {
    var d = (d = G[b >> 2]) ? U(d) : "", g = J[b + 4 >> 2];
    b = G[b + 8 >> 2];
    for (var f = [], l = 0; g--; ) f.push({
        name: l++,
        defaultValue: E[b >> 2],
        minValue: E[b + 4 >> 2],
        maxValue: E[b + 8 >> 2],
        automationRate: (J[b + 12 >> 2] ? "k" : "a") + "-rate"
    }), b += 16;
    P[a].audioWorklet.port.postMessage({
        _wpn: d,
        K: f,
        L: a,
        A: c,
        B: k
    });
}, da = () => !1, ea = () => r ? () => 138 : a => (a.set(crypto.getRandomValues(new Uint8Array(a.byteLength))), 
0), W = a => (W = ea())(a), fa = (a, b) => W(I.subarray(a, a + b));

function ha(a) {
    var b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = P[a];
    b.onclick = () => {
        a.state != "running" ? a.resume() : a.suspend();
    };
}

function y() {
    Z = {
        g: ha,
        c: Q,
        h: R,
        e: aa,
        i: ba,
        f: ca,
        b: da,
        a: w,
        d: fa
    };
    z = WebAssembly.instantiate(m.wasm, {
        a: Z
    }).then((a => {
        a = (a.instance || a).exports;
        X = a.k;
        F = a.m;
        D = a.n;
        C = a.o;
        Y = a.p;
        A = a.l;
        t ? (Y(u.N, u.M, u.G), r || (removeEventListener("message", M), K = K.forEach(L), 
        addEventListener("message", L))) : a.j();
        t || X();
    }));
}

t || y();