var m = globalThis.Module || "undefined" != typeof Module ? Module : {}, r = !!globalThis.AudioWorkletGlobalScope, t = "em-ww" == globalThis.name || r, u, z, I, J, G, E, w, X, F, D, C, Y, A, Z;

function v(a) {
    u = a;
    w = a.L;
    x();
    m ||= {};
    m.wasm = a.G;
    y();
    a.G = a.M = 0;
}

t && !r && (onmessage = a => {
    onmessage = null;
    v(a.data);
});

if (r) {
    function a(b) {
        class f extends AudioWorkletProcessor {
            constructor(d) {
                super();
                d = d.processorOptions;
                this.v = A.get(d.v);
                this.A = d.A;
                this.u = d.u;
                this.s = 4 * this.u;
                this.B = Array(Math.min((u.F - 16) / this.s | 0, 64));
                this.K();
            }
            K() {
                for (var d = C(), g = D(this.B.length * this.s) >> 2, e = this.B.length - 1; 0 <= e; e--) this.B[e] = E.subarray(g, g += this.u);
                F(d);
            }
            static get parameterDescriptors() {
                return b;
            }
            process(d, g, e) {
                var n = d.length, p = g.length, h, q, k = 12 * (n + p), l = 0;
                for (h of d) l += h.length;
                l *= this.s;
                var H = 0;
                for (h of g) H += h.length;
                l += H * this.s;
                var N = 0;
                for (h in e) ++N, k += 8, l += e[h].byteLength;
                var U = C(), B = k + l + 15 & -16;
                k = D(B);
                l = k + (B - l);
                B = k;
                for (h of d) {
                    G[k >> 2] = h.length;
                    G[k + 4 >> 2] = this.u;
                    G[k + 8 >> 2] = l;
                    k += 12;
                    for (q of h) E.set(q, l >> 2), l += this.s;
                }
                d = k;
                for (h = 0; q = e[h++]; ) G[k >> 2] = q.length, G[k + 4 >> 2] = l, k += 8, E.set(q, l >> 2), 
                l += 4 * q.length;
                e = k;
                for (h of g) G[k >> 2] = h.length, G[k + 4 >> 2] = this.u, G[k + 8 >> 2] = l, k += 12, 
                l += this.s * h.length;
                if (n = this.v(n, B, p, e, N, d, this.A)) for (h of g) for (q of h) q.set(this.B[--H]);
                F(U);
                return !!n;
            }
        }
        return f;
    }
    var port = globalThis.port || {};
    class c extends AudioWorkletProcessor {
        constructor(b) {
            super();
            v(b.processorOptions);
            port instanceof MessagePort || (this.port.onmessage = port.onmessage, port = this.port);
        }
        process() {}
    }
    registerProcessor("em-bootstrap", c);
    port.onmessage = async b => {
        await z;
        b = b.data;
        b._boot ? v(b) : b._wpn ? (registerProcessor(b._wpn, a(b.H)), port.postMessage({
            _wsc: b.v,
            C: [ b.I, 1, b.A ]
        })) : b._wsc && A.get(b._wsc)(...b.C);
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
    let b = a._wsc;
    b && A.get(b)(...a.x);
}, N = a => {
    L.push(a);
}, Q = (a, b, e, d) => {
    b = P[b];
    P[a].connect(b.destination || b, e, d);
}, P = {}, R = 0, S = globalThis.TextDecoder && new TextDecoder, T = (a = 0) => {
    for (var b = J, e = a, d = e + void 0; b[e] && !(e >= d); ) ++e;
    if (16 < e - a && b.buffer && S) return S.decode(b.slice(a, e));
    for (d = ""; a < e; ) {
        var c = b[a++];
        if (c & 128) {
            var g = b[a++] & 63;
            if (192 == (c & 224)) d += String.fromCharCode((c & 31) << 6 | g); else {
                var f = b[a++] & 63;
                c = 224 == (c & 240) ? (c & 15) << 12 | g << 6 | f : (c & 7) << 18 | g << 12 | f << 6 | b[a++] & 63;
                65536 > c ? d += String.fromCharCode(c) : (c -= 65536, d += String.fromCharCode(55296 | c >> 10, 56320 | c & 1023));
            }
        } else f += String.fromCharCode(d);
    }
    return f;
}, T = a => {
    if (a) {
        var c = G[a >> 2];
        a = {
            latencyHint: (c ? S(c) : "") || void 0,
            sampleRate: G[a + 4 >> 2] || void 0
        };
    } else a = void 0;
    a = new AudioContext(a);
    O[++Q] = a;
    return Q;
}, V = (a, c, b, f, d) => {
    var g = b ? J[b + 4 >> 2] : 0;
    if (b) {
        var e = J[b >> 2];
        b = G[b + 8 >> 2];
        var n = g;
        if (b) {
            b >>= 2;
            for (var p = []; n--; ) p.push(G[b++]);
            b = p;
        } else b = void 0;
        f = {
            numberOfInputs: e,
            numberOfOutputs: g,
            outputChannelCount: b,
            processorOptions: {
                v: f,
                A: d,
                u: 128
            }
        };
    } else f = void 0;
    a = new AudioWorkletNode(O[a], c ? S(c) : "", f);
    O[++Q] = a;
    return Q;
}, W = (a, c, b, f) => {
    var d = (d = G[c >> 2]) ? S(d) : "", g = J[c + 4 >> 2];
    c = G[c + 8 >> 2];
    for (var e = [], n = 0; g--; ) e.push({
        name: n++,
        defaultValue: E[c >> 2],
        minValue: E[c + 4 >> 2],
        maxValue: E[c + 8 >> 2],
        automationRate: (J[c + 12 >> 2] ? "k" : "a") + "-rate"
    }), c += 16;
    O[a].audioWorklet.port.postMessage({
        _wpn: d,
        H: e,
        I: a,
        v: b,
        A: f
    });
}, aa = () => !1, ba = 1, ca = a => {
    a = a.data;
    var c = a._wsc;
    c && A.get(c)(...a.C);
}, da = (a, c, b, f, d) => {
    var g = O[a], e = g.audioWorklet, n = () => {
        A.get(f)(a, 0, d);
    };
    if (!e) return n();
    e.addModule(m.js).then((() => {
        e.port || (e.port = {
            postMessage: p => {
                p._boot ? (e.D = new AudioWorkletNode(g, "em-bootstrap", {
                    processorOptions: p
                }), e.D.port.onmessage = h => {
                    e.port.onmessage(h);
                }) : e.D.port.postMessage(p);
            }
        });
        e.port.postMessage({
            _boot: 1,
            N: ba++,
            G: m.wasm,
            L: w,
            J: c,
            F: b
        });
        e.port.onmessage = ca;
        A.get(f)(a, 1, d);
    })).catch(n);
};

function ea(a) {
    let c = document.createElement("button");
    c.innerHTML = "Toggle playback";
    document.body.appendChild(c);
    a = O[a];
    c.onclick = () => {
        "running" != a.state ? a.resume() : a.suspend();
    };
}

function y() {
    Z = {
        f: ea,
        g: P,
        d: T,
        h: V,
        e: W,
        b: aa,
        c: da,
        a: w
    };
    z = WebAssembly.instantiate(m.wasm, {
        a: Z
    }).then((a => {
        a = (a.instance || a).exports;
        X = a.j;
        F = a.l;
        D = a.m;
        C = a.n;
        Y = a.o;
        A = a.k;
        t ? (Y(u.J, u.F), r || (removeEventListener("message", M), K = K.forEach(L), addEventListener("message", L))) : a.i();
        t || X();
    }));
}

t || y();