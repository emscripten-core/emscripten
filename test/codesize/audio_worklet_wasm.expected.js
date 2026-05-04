var m = globalThis.Module || "undefined" != typeof Module ? Module : {}, q = !!globalThis.AudioWorkletGlobalScope, r = "em-ww" == globalThis.name || q, t, z, J, E, G, I, v, X, F, D, C, Y, A, Z;

function u(a) {
    t = a;
    v = a.H;
    x();
    m ||= {};
    m.wasm = a.G;
    y();
    a.G = a.H = 0;
}

r && !q && (onmessage = a => {
    onmessage = null;
    u(a.data);
});

if (q) {
    function a(c) {
        class l extends AudioWorkletProcessor {
            constructor(d) {
                super();
                d = d.processorOptions;
                this.v = A.get(d.v);
                this.A = d.A;
                this.u = d.u;
                this.s = 4 * this.u;
                this.B = Array(Math.min((t.F - 16) / this.s | 0, 64));
                this.L();
            }
            L() {
                for (var d = C(), h = D(this.B.length * this.s) >> 2, g = this.B.length - 1; 0 <= g; g--) this.B[g] = E.subarray(h, h += this.u);
                F(d);
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, h, g) {
                var e = d.length, w = h.length, f, p, k = 12 * (e + w), n = 0;
                for (f of d) n += f.length;
                n *= this.s;
                var H = 0;
                for (f of h) H += f.length;
                n += H * this.s;
                var O = 0;
                for (f in g) ++O, k += 8, n += g[f].byteLength;
                var V = C(), B = k + n + 15 & -16;
                k = D(B);
                n = k + (B - n);
                B = k;
                for (f of d) {
                    G[k >> 2] = f.length;
                    G[k + 4 >> 2] = this.u;
                    G[k + 8 >> 2] = n;
                    k += 12;
                    for (p of f) E.set(p, n >> 2), n += this.s;
                }
                d = k;
                for (f = 0; p = g[f++]; ) G[k >> 2] = p.length, G[k + 4 >> 2] = n, k += 8, E.set(p, n >> 2), 
                n += 4 * p.length;
                g = k;
                for (f of h) G[k >> 2] = f.length, G[k + 4 >> 2] = this.u, G[k + 8 >> 2] = n, k += 12, 
                n += this.s * f.length;
                if (e = this.v(e, B, w, g, O, d, this.A)) for (f of h) for (p of f) p.set(this.B[--H]);
                F(V);
                return !!e;
            }
        }
        return l;
    }
    var port = globalThis.port || {};
    class b extends AudioWorkletProcessor {
        constructor(c) {
            super();
            u(c.processorOptions);
            port instanceof MessagePort || (this.port.onmessage = port.onmessage, port = this.port);
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
    port.onmessage = async c => {
        await z;
        c = c.data;
        c._boot ? u(c) : c._wpn ? (registerProcessor(c._wpn, a(c.I)), port.postMessage({
            _wsc: c.v,
            C: [ c.J, 1, c.A ]
        })) : c._wsc && A.get(c._wsc)(...c.C);
    };
}

function x() {
    var a = v.buffer;
    I = new Uint8Array(a);
    J = new Int32Array(a);
    G = new Uint32Array(a);
    E = new Float32Array(a);
}

r || (v = m.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), x());

var K = [], L = a => {
    a = a.data;
    let b = a._wsc;
    b && A.get(b)(...a.x);
}, M = a => {
    K.push(a);
}, N = a => {
    a = a.data;
    var b = a._wsc;
    b && A.get(b)(...a.C);
}, Q = (a, b, c, l, d, h) => {
    var g = P[b], e = g.audioWorklet, w = () => {
        A.get(d)(b, 0, h);
    };
    if (!e) return w();
    e.addModule(m.js).then((() => {
        e.port || (e.port = {
            postMessage: f => {
                f._boot ? (e.D = new AudioWorkletNode(g, "em-bootstrap", {
                    processorOptions: f
                }), e.D.port.onmessage = p => {
                    e.port.onmessage(p);
                }) : e.D.port.postMessage(f);
            }
        });
        e.port.postMessage({
            _boot: 1,
            M: a,
            G: m.wasm,
            H: v,
            K: c,
            F: l
        });
        e.port.onmessage = N;
        A.get(d)(b, 1, h);
    })).catch(w);
}, R = (a, b, c, l) => {
    b = P[b];
    P[a].connect(b.destination || b, c, l);
}, P = {}, S = 0, T = globalThis.TextDecoder && new TextDecoder, U = (a = 0) => {
    for (var b = I, c = a, l = c + void 0; b[c] && !(c >= l); ) ++c;
    if (16 < c - a && b.buffer && T) return T.decode(b.slice(a, c));
    for (l = ""; a < c; ) {
        var d = b[a++];
        if (d & 128) {
            var h = b[a++] & 63;
            if (192 == (d & 224)) l += String.fromCharCode((d & 31) << 6 | h); else {
                var g = b[a++] & 63;
                d = 224 == (d & 240) ? (d & 15) << 12 | h << 6 | g : (d & 7) << 18 | h << 12 | g << 6 | b[a++] & 63;
                65536 > d ? l += String.fromCharCode(d) : (d -= 65536, l += String.fromCharCode(55296 | d >> 10, 56320 | d & 1023));
            }
        } else l += String.fromCharCode(d);
    }
    return l;
}, aa = a => {
    if (a) {
        var b = G[a >> 2];
        b = (b ? U(b) : "") || void 0;
        var c = J[a + 8 >> 2];
        a = {
            latencyHint: b,
            sampleRate: G[a + 4 >> 2] || void 0,
            N: 0 > c ? "hardware" : c || "default"
        };
    } else a = void 0;
    a = new AudioContext(a);
    P[++S] = a;
    return S;
}, ba = (a, b, c, l, d) => {
    var h = c ? J[c + 4 >> 2] : 0;
    if (c) {
        var g = J[c >> 2], e = G[c + 8 >> 2], w = h;
        if (e) {
            e >>= 2;
            for (var f = []; w--; ) f.push(G[e++]);
            e = f;
        } else e = void 0;
        c = {
            numberOfInputs: g,
            numberOfOutputs: h,
            outputChannelCount: e,
            channelCount: G[c + 12 >> 2] || void 0,
            channelCountMode: [ , "clamped-max", "explicit" ][J[c + 16 >> 2]],
            channelInterpretation: [ , "discrete" ][J[c + 20 >> 2]],
            processorOptions: {
                v: l,
                A: d,
                u: P[a].renderQuantumSize || 128
            }
        };
    } else c = void 0;
    a = new AudioWorkletNode(P[a], b ? U(b) : "", c);
    P[++S] = a;
    return S;
}, ca = (a, b, c, l) => {
    var d = (d = G[b >> 2]) ? U(d) : "", h = J[b + 4 >> 2];
    b = G[b + 8 >> 2];
    for (var g = [], e = 0; h--; ) g.push({
        name: e++,
        defaultValue: E[b >> 2],
        minValue: E[b + 4 >> 2],
        maxValue: E[b + 8 >> 2],
        automationRate: (J[b + 12 >> 2] ? "k" : "a") + "-rate"
    }), b += 16;
    P[a].audioWorklet.port.postMessage({
        _wpn: d,
        I: g,
        J: a,
        v: c,
        A: l
    });
}, da = () => !1, ea = () => q ? () => 138 : a => (a.set(crypto.getRandomValues(new Uint8Array(a.byteLength))), 
0), W = a => (W = ea())(a), fa = (a, b) => W(I.subarray(a, a + b));

function ha(a) {
    let b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = P[a];
    b.onclick = () => {
        "running" != a.state ? a.resume() : a.suspend();
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
        a: v,
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
        r ? (Y(t.M, t.K, t.F), q || (removeEventListener("message", M), K = K.forEach(L), 
        addEventListener("message", L))) : a.j();
        r || X();
    }));
}

r || y();