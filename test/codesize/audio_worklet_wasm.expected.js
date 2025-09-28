var m = globalThis.Module || "undefined" != typeof Module ? Module : {}, q = !!globalThis.AudioWorkletGlobalScope, r = "em-ww" == globalThis.name || q, t, z, J, K, H, E, v, X, F, D, C, Y, A, Z;

function u(a) {
    t = a;
    v = a.L;
    w();
    m ||= {};
    m.wasm = a.G;
    y();
    a.G = a.M = 0;
}

r && !q && (onmessage = a => {
    onmessage = null;
    u(a.data);
});

if (q) {
    function a(e) {
        class d extends AudioWorkletProcessor {
            constructor(c) {
                super();
                c = c.processorOptions;
                this.v = A.get(c.v);
                this.A = c.A;
                this.u = c.u;
                this.s = 4 * this.u;
                this.B = Array(Math.min((t.F - 16) / this.s | 0, 64));
                this.K();
            }
            K() {
                for (var c = C(), g = D(this.B.length * this.s) >> 2, f = this.B.length - 1; 0 <= f; f--) this.B[f] = E.subarray(g, g += this.u);
                F(c);
            }
            static get parameterDescriptors() {
                return e;
            }
            process(c, g, f) {
                var n = c.length, x = g.length, k, p, h = 12 * (n + x), l = 0;
                for (k of c) l += k.length;
                l *= this.s;
                var G = 0;
                for (k of g) G += k.length;
                l += G * this.s;
                var O = 0;
                for (k in f) ++O, h += 8, l += f[k].byteLength;
                var V = C(), B = h + l + 15 & -16;
                h = D(B);
                l = h + (B - l);
                B = h;
                for (k of c) {
                    H[h >> 2] = k.length;
                    H[h + 4 >> 2] = this.u;
                    H[h + 8 >> 2] = l;
                    h += 12;
                    for (p of k) E.set(p, l >> 2), l += this.s;
                }
                c = h;
                for (k = 0; p = f[k++]; ) H[h >> 2] = p.length, H[h + 4 >> 2] = l, h += 8, E.set(p, l >> 2), 
                l += 4 * p.length;
                f = h;
                for (k of g) H[h >> 2] = k.length, H[h + 4 >> 2] = this.u, H[h + 8 >> 2] = l, h += 12, 
                l += this.s * k.length;
                if (n = this.v(n, B, x, f, O, c, this.A)) for (k of g) for (p of k) p.set(this.B[--G]);
                F(V);
                return !!n;
            }
        }
        return d;
    }
    var I;
    class b extends AudioWorkletProcessor {
        constructor(e) {
            super();
            u(e.processorOptions);
            I = this.port;
            I.onmessage = async d => {
                await z;
                d = d.data;
                d._wpn ? (registerProcessor(d._wpn, a(d.H)), I.postMessage({
                    _wsc: d.v,
                    C: [ d.I, 1, d.A ]
                })) : d._wsc && A.get(d._wsc)(...d.C);
            };
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
}

function w() {
    var a = v.buffer;
    J = new Uint8Array(a);
    K = new Int32Array(a);
    H = new Uint32Array(a);
    E = new Float32Array(a);
}

r || (v = m.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), w());

var L = [], M = a => {
    a = a.data;
    let b = a._wsc;
    b && A.get(b)(...a.x);
}, N = a => {
    L.push(a);
}, Q = (a, b, e, d) => {
    b = P[b];
    P[a].connect(b.destination || b, e, d);
}, P = {}, R = 0, S = globalThis.TextDecoder ? new TextDecoder : void 0, T = (a = 0) => {
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
        } else d += String.fromCharCode(c);
    }
    return d;
}, U = a => {
    if (a) {
        var b = H[a >> 2];
        a = {
            latencyHint: (b ? T(b) : "") || void 0,
            sampleRate: H[a + 4 >> 2] || void 0
        };
    } else a = void 0;
    a = new AudioContext(a);
    P[++R] = a;
    return R;
}, W = (a, b, e, d, c) => {
    var g = e ? K[e + 4 >> 2] : 0;
    if (e) {
        var f = K[e >> 2];
        e = H[e + 8 >> 2];
        var n = g;
        if (e) {
            e >>= 2;
            for (var x = []; n--; ) x.push(H[e++]);
            e = x;
        } else e = void 0;
        d = {
            numberOfInputs: f,
            numberOfOutputs: g,
            outputChannelCount: e,
            processorOptions: {
                v: d,
                A: c,
                u: 128
            }
        };
    } else d = void 0;
    a = new AudioWorkletNode(P[a], b ? T(b) : "", d);
    P[++R] = a;
    return R;
}, aa = (a, b, e, d) => {
    var c = (c = H[b >> 2]) ? T(c) : "", g = K[b + 4 >> 2];
    b = H[b + 8 >> 2];
    for (var f = [], n = 0; g--; ) f.push({
        name: n++,
        defaultValue: E[b >> 2],
        minValue: E[b + 4 >> 2],
        maxValue: E[b + 8 >> 2],
        automationRate: (K[b + 12 >> 2] ? "k" : "a") + "-rate"
    }), b += 16;
    P[a].audioWorklet.D.port.postMessage({
        _wpn: c,
        H: f,
        I: a,
        v: e,
        A: d
    });
}, ba = () => !1, ca = 1, da = a => {
    a = a.data;
    var b = a._wsc;
    b && A.get(b)(...a.C);
}, ea = (a, b, e, d, c) => {
    var g = P[a], f = g.audioWorklet, n = () => {
        A.get(d)(a, 0, c);
    };
    if (!f) return n();
    f.addModule(m.js).then((() => {
        f.D = new AudioWorkletNode(g, "em-bootstrap", {
            processorOptions: {
                N: ca++,
                G: m.wasm,
                L: v,
                J: b,
                F: e
            }
        });
        f.D.port.onmessage = da;
        A.get(d)(a, 1, c);
    })).catch(n);
};

function fa(a) {
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
        f: fa,
        g: Q,
        d: U,
        h: W,
        e: aa,
        b: ba,
        c: ea,
        a: v
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
        r ? (Y(t.J, t.F), q || (removeEventListener("message", N), L = L.forEach(M), addEventListener("message", M))) : a.i();
        r || X();
    }));
}

r || y();