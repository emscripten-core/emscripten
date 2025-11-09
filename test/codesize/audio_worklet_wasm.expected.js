var m = globalThis.Module || "undefined" != typeof Module ? Module : {}, p = !!globalThis.AudioWorkletGlobalScope, t = "em-ww" == globalThis.name || p, u, z, I, J, G, E, w, X, F, D, C, Y, A, Z;

function v(a) {
    u = a;
    w = a.L;
    x();
    m ||= {};
    m.wasm = a.G;
    y();
    a.G = a.M = 0;
}

t && !p && (onmessage = a => {
    onmessage = null;
    v(a.data);
});

if (p) {
    function a(b) {
        class h extends AudioWorkletProcessor {
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
                var l = d.length, q = g.length, f, r, k = 12 * (l + q), n = 0;
                for (f of d) n += f.length;
                n *= this.s;
                var H = 0;
                for (f of g) H += f.length;
                n += H * this.s;
                var N = 0;
                for (f in e) ++N, k += 8, n += e[f].byteLength;
                var U = C(), B = k + n + 15 & -16;
                k = D(B);
                n = k + (B - n);
                B = k;
                for (f of d) {
                    G[k >> 2] = f.length;
                    G[k + 4 >> 2] = this.u;
                    G[k + 8 >> 2] = n;
                    k += 12;
                    for (r of f) E.set(r, n >> 2), n += this.s;
                }
                d = k;
                for (f = 0; r = e[f++]; ) G[k >> 2] = r.length, G[k + 4 >> 2] = n, k += 8, E.set(r, n >> 2), 
                n += 4 * r.length;
                e = k;
                for (f of g) G[k >> 2] = f.length, G[k + 4 >> 2] = this.u, G[k + 8 >> 2] = n, k += 12, 
                n += this.s * f.length;
                if (l = this.v(l, B, q, e, N, d, this.A)) for (f of g) for (r of f) r.set(this.B[--H]);
                F(U);
                return !!l;
            }
        }
        return h;
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
    let c = a._wsc;
    c && A.get(c)(...a.x);
}, M = a => {
    K.push(a);
}, P = (a, c, b, h) => {
    c = O[c];
    O[a].connect(c.destination || c, b, h);
}, O = {}, Q = 0, R = globalThis.TextDecoder && new TextDecoder, S = (a = 0) => {
    for (var c = I, b = a, h = b + void 0; c[b] && !(b >= h); ) ++b;
    if (16 < b - a && c.buffer && R) return R.decode(c.slice(a, b));
    for (h = ""; a < b; ) {
        var d = c[a++];
        if (d & 128) {
            var g = c[a++] & 63;
            if (192 == (d & 224)) h += String.fromCharCode((d & 31) << 6 | g); else {
                var e = c[a++] & 63;
                d = 224 == (d & 240) ? (d & 15) << 12 | g << 6 | e : (d & 7) << 18 | g << 12 | e << 6 | c[a++] & 63;
                65536 > d ? h += String.fromCharCode(d) : (d -= 65536, h += String.fromCharCode(55296 | d >> 10, 56320 | d & 1023));
            }
        } else h += String.fromCharCode(d);
    }
    return h;
}, T = a => {
    if (a) {
        var c = G[a >> 2];
        c = (c ? S(c) : "") || void 0;
        var b = J[a + 8 >> 2];
        a = {
            latencyHint: c,
            sampleRate: G[a + 4 >> 2] || void 0,
            N: 0 > b ? "hardware" : b || "default"
        };
    } else a = void 0;
    a = new AudioContext(a);
    O[++Q] = a;
    return Q;
}, V = (a, c, b, h, d) => {
    var g = b ? J[b + 4 >> 2] : 0;
    if (b) {
        var e = J[b >> 2], l = G[b + 8 >> 2], q = g;
        if (l) {
            l >>= 2;
            for (var f = []; q--; ) f.push(G[l++]);
            l = f;
        } else l = void 0;
        b = {
            numberOfInputs: e,
            numberOfOutputs: g,
            outputChannelCount: l,
            channelCount: G[b + 12 >> 2] || void 0,
            channelCountMode: [ , "clamped-max", "explicit" ][J[b + 16 >> 2]],
            channelInterpretation: [ , "discrete" ][J[b + 20 >> 2]],
            processorOptions: {
                v: h,
                A: d,
                u: O[a].renderQuantumSize || 128
            }
        };
    } else b = void 0;
    a = new AudioWorkletNode(O[a], c ? S(c) : "", b);
    O[++Q] = a;
    return Q;
}, W = (a, c, b, h) => {
    var d = (d = G[c >> 2]) ? S(d) : "", g = J[c + 4 >> 2];
    c = G[c + 8 >> 2];
    for (var e = [], l = 0; g--; ) e.push({
        name: l++,
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
        A: h
    });
}, aa = () => !1, ba = 1, ca = a => {
    a = a.data;
    var c = a._wsc;
    c && A.get(c)(...a.C);
}, da = (a, c, b, h, d) => {
    var g = O[a], e = g.audioWorklet, l = () => {
        A.get(h)(a, 0, d);
    };
    if (!e) return l();
    e.addModule(m.js).then((() => {
        e.port || (e.port = {
            postMessage: q => {
                q._boot ? (e.D = new AudioWorkletNode(g, "em-bootstrap", {
                    processorOptions: q
                }), e.D.port.onmessage = f => {
                    e.port.onmessage(f);
                }) : e.D.port.postMessage(q);
            }
        });
        e.port.postMessage({
            _boot: 1,
            O: ba++,
            G: m.wasm,
            L: w,
            J: c,
            F: b
        });
        e.port.onmessage = ca;
        A.get(h)(a, 1, d);
    })).catch(l);
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
        t ? (Y(u.J, u.F), p || (removeEventListener("message", M), K = K.forEach(L), addEventListener("message", L))) : a.i();
        t || X();
    }));
}

t || y();