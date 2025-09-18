var m = globalThis.Module || "undefined" != typeof Module ? Module : {}, n = "em-ww" == globalThis.name, q = "undefined" !== typeof AudioWorkletGlobalScope, t, z, J, K, H, E, v, X, F, C, B, Y, A, Z;

q && (n = !0);

function u(a) {
    t = a;
    v = a.L;
    w();
    m ||= {};
    m.wasm = a.G;
    y();
    a.G = a.M = 0;
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
                this.v = A.get(d.v);
                this.A = d.A;
                this.u = d.u;
                this.s = 4 * this.u;
                this.B = Array(Math.min((t.F - 16) / this.s | 0, 64));
                this.K();
            }
            K() {
                for (var d = B(), f = C(this.B.length * this.s) >> 2, g = this.B.length - 1; 0 <= g; g--) this.B[g] = E.subarray(f, f += this.u);
                F(d);
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, f, g) {
                var p = d.length, x = f.length, k, r, h = 12 * (p + x), l = 0;
                for (k of d) l += k.length;
                l *= this.s;
                var G = 0;
                for (k of f) G += k.length;
                l += G * this.s;
                var O = 0;
                for (k in g) ++O, h += 8, l += g[k].byteLength;
                var V = B(), D = h + l + 15 & -16;
                h = C(D);
                l = h + (D - l);
                D = h;
                for (k of d) {
                    H[h >> 2] = k.length;
                    H[h + 4 >> 2] = this.u;
                    H[h + 8 >> 2] = l;
                    h += 12;
                    for (r of k) E.set(r, l >> 2), l += this.s;
                }
                d = h;
                for (k = 0; r = g[k++]; ) H[h >> 2] = r.length, H[h + 4 >> 2] = l, h += 8, E.set(r, l >> 2), 
                l += 4 * r.length;
                g = h;
                for (k of f) H[h >> 2] = k.length, H[h + 4 >> 2] = this.u, H[h + 8 >> 2] = l, h += 12, 
                l += this.s * k.length;
                if (p = this.v(p, D, x, g, O, d, this.A)) for (k of f) for (r of k) r.set(this.B[--G]);
                F(V);
                return !!p;
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

n || (v = m.mem || new WebAssembly.Memory({
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
}, P = a => F(a), Q = () => B(), S = (a, b, c, e) => {
    b = R[b];
    R[a].connect(b.destination || b, c, e);
}, R = {}, T = 0, U = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, W = (a = 0) => {
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
}, aa = a => {
    var b = window.AudioContext || window.webkitAudioContext;
    if (a) {
        var c = H[a >> 2];
        a = {
            latencyHint: (c ? W(c) : "") || void 0,
            sampleRate: H[a + 4 >> 2] || void 0
        };
    } else a = void 0;
    if (c = b) b = new b(a), R[++T] = b, c = T;
    return c;
}, ba = (a, b, c, e, d) => {
    var f = c ? K[c + 4 >> 2] : 0;
    if (c) {
        var g = K[c >> 2];
        c = H[c + 8 >> 2];
        var p = f;
        if (c) {
            c >>= 2;
            for (var x = []; p--; ) x.push(H[c++]);
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
    a = new AudioWorkletNode(R[a], b ? W(b) : "", e);
    R[++T] = a;
    return T;
}, ca = (a, b, c, e) => {
    var d = [], f = (f = H[b >> 2]) ? W(f) : "", g = K[b + 4 >> 2];
    b = H[b + 8 >> 2];
    for (var p = 0; g--; ) d.push({
        name: p++,
        defaultValue: E[b >> 2],
        minValue: E[b + 4 >> 2],
        maxValue: E[b + 8 >> 2],
        automationRate: (K[b + 12 >> 2] ? "k" : "a") + "-rate"
    }), b += 16;
    R[a].audioWorklet.D.port.postMessage({
        _wpn: f,
        H: d,
        I: a,
        v: c,
        A: e
    });
}, da = () => !1, ea = 1, fa = a => {
    a = a.data;
    var b = a._wsc;
    b && A.get(b)(...a.C);
}, ha = a => C(a), ia = (a, b, c, e, d) => {
    var f = R[a], g = f.audioWorklet, p = () => {
        A.get(e)(a, 0, d);
    };
    if (!g) return p();
    g.addModule(m.js).then((() => {
        g.D = new AudioWorkletNode(f, "em-bootstrap", {
            processorOptions: {
                N: ea++,
                G: m.wasm,
                L: v,
                J: b,
                F: c
            }
        });
        g.D.port.onmessage = fa;
        A.get(e)(a, 1, d);
    })).catch(p);
};

function ja(a) {
    let b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = R[a];
    b.onclick = () => {
        "running" != a.state ? a.resume() : a.suspend();
    };
}

function y() {
    Z = {
        f: ja,
        g: S,
        d: aa,
        h: ba,
        e: ca,
        b: da,
        c: ia,
        a: v
    };
    z = WebAssembly.instantiate(m.wasm, {
        a: Z
    }).then((a => {
        a = (a.instance || a).exports;
        X = a.j;
        F = a.l;
        C = a.m;
        B = a.n;
        Y = a.o;
        A = A = a.k;
        m.stackSave = Q;
        m.stackAlloc = ha;
        m.stackRestore = P;
        m.wasmTable = A;
        n ? (Y(t.J, t.F), "undefined" === typeof AudioWorkletGlobalScope && (removeEventListener("message", N), 
        L = L.forEach(M), addEventListener("message", M))) : a.i();
        n || X();
    }));
}

n || y();