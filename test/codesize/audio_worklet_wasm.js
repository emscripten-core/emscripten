var m = globalThis.Module || "undefined" != typeof Module ? Module : {}, q = "em-ww" == globalThis.name, r = !!globalThis.AudioWorkletGlobalScope, t, z, J, K, H, E, v, X, F, D, C, Y, A, Z;

r && (q = !0);

function u(a) {
    t = a;
    v = a.L;
    w();
    m ||= {};
    m.wasm = a.G;
    y();
    a.G = a.M = 0;
}

q && !r && (onmessage = a => {
    onmessage = null;
    u(a.data);
});

if (r) {
    function a(c) {
        class e extends AudioWorkletProcessor {
            constructor(d) {
                super();
                d = d.processorOptions;
                this.v = A.get(d.v);
                this.A = d.A;
                this.u = d.u;
                this.s = 4 * this.u;
                this.B = Array(Math.min((t.F - 16) / this.s | 0, 64));
                this.K();
            }
            K() {
                for (var d = C(), f = D(this.B.length * this.s) >> 2, g = this.B.length - 1; 0 <= g; g--) this.B[g] = E.subarray(f, f += this.u);
                F(d);
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, f, g) {
                var n = d.length, x = f.length, k, p, h = 12 * (n + x), l = 0;
                for (k of d) l += k.length;
                l *= this.s;
                var G = 0;
                for (k of f) G += k.length;
                l += G * this.s;
                var O = 0;
                for (k in g) ++O, h += 8, l += g[k].byteLength;
                var V = C(), B = h + l + 15 & -16;
                h = D(B);
                l = h + (B - l);
                B = h;
                for (k of d) {
                    H[h >> 2] = k.length;
                    H[h + 4 >> 2] = this.u;
                    H[h + 8 >> 2] = l;
                    h += 12;
                    for (p of k) E.set(p, l >> 2), l += this.s;
                }
                d = h;
                for (k = 0; p = g[k++]; ) H[h >> 2] = p.length, H[h + 4 >> 2] = l, h += 8, E.set(p, l >> 2), 
                l += 4 * p.length;
                g = h;
                for (k of f) H[h >> 2] = k.length, H[h + 4 >> 2] = this.u, H[h + 8 >> 2] = l, h += 12, 
                l += this.s * k.length;
                if (n = this.v(n, B, x, g, O, d, this.A)) for (k of f) for (p of k) p.set(this.B[--G]);
                F(V);
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
                await z;
                e = e.data;
                e._wpn ? (registerProcessor(e._wpn, a(e.H)), I.postMessage({
                    _wsc: e.v,
                    C: [ e.I, 1, e.A ]
                })) : e._wsc && A.get(e._wsc)(...e.C);
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

q || (v = m.mem || new WebAssembly.Memory({
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
}, Q = (a, b, c, e) => {
    b = P[b];
    P[a].connect(b.destination || b, c, e);
}, P = {}, R = 0, S = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, T = (a = 0) => {
    for (var b = J, c = a, e = c + void 0; b[c] && !(c >= e); ) ++c;
    if (16 < c - a && b.buffer && S) return S.decode(b.slice(a, c));
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
}, U = a => {
    var b = window.AudioContext || window.webkitAudioContext;
    if (a) {
        var c = H[a >> 2];
        a = {
            latencyHint: (c ? T(c) : "") || void 0,
            sampleRate: H[a + 4 >> 2] || void 0
        };
    } else a = void 0;
    if (c = b) b = new b(a), P[++R] = b, c = R;
    return c;
}, W = (a, b, c, e, d) => {
    var f = c ? K[c + 4 >> 2] : 0;
    if (c) {
        var g = K[c >> 2];
        c = H[c + 8 >> 2];
        var n = f;
        if (c) {
            c >>= 2;
            for (var x = []; n--; ) x.push(H[c++]);
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
    a = new AudioWorkletNode(P[a], b ? T(b) : "", e);
    P[++R] = a;
    return R;
}, aa = (a, b, c, e) => {
    var d = [], f = (f = H[b >> 2]) ? T(f) : "", g = K[b + 4 >> 2];
    b = H[b + 8 >> 2];
    for (var n = 0; g--; ) d.push({
        name: n++,
        defaultValue: E[b >> 2],
        minValue: E[b + 4 >> 2],
        maxValue: E[b + 8 >> 2],
        automationRate: (K[b + 12 >> 2] ? "k" : "a") + "-rate"
    }), b += 16;
    P[a].audioWorklet.D.port.postMessage({
        _wpn: f,
        H: d,
        I: a,
        v: c,
        A: e
    });
}, ba = () => !1, ca = 1, da = a => {
    a = a.data;
    var b = a._wsc;
    b && A.get(b)(...a.C);
}, ea = (a, b, c, e, d) => {
    var f = P[a], g = f.audioWorklet, n = () => {
        A.get(e)(a, 0, d);
    };
    if (!g) return n();
    g.addModule(m.js).then((() => {
        g.D = new AudioWorkletNode(f, "em-bootstrap", {
            processorOptions: {
                N: ca++,
                G: m.wasm,
                L: v,
                J: b,
                F: c
            }
        });
        g.D.port.onmessage = da;
        A.get(e)(a, 1, d);
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
        q ? (Y(t.J, t.F), r || (removeEventListener("message", N), L = L.forEach(M), addEventListener("message", M))) : a.i();
        q || X();
    }));
}

q || y();