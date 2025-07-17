var k = globalThis.Module || "undefined" != typeof Module ? Module : {}, p = "em-ww" == globalThis.name, q = "undefined" !== typeof AudioWorkletGlobalScope, u, v, y, N, O, J, K, E, W, L, H, F, X, Z;

q && (p = !0);

var r = new Promise((a => {
    u = a;
}));

function x(a) {
    v = a;
    y = a.I;
    C();
    k ||= {};
    k.wasm = a.C;
    D();
    a.C = a.J = 0;
}

p && !q && (onmessage = a => {
    onmessage = null;
    x(a.data);
});

if (q) {
    function a(c) {
        class d extends AudioWorkletProcessor {
            constructor(e) {
                super();
                e = e.processorOptions;
                this.u = E.get(e.u);
                this.v = e.v;
                this.s = e.s;
            }
            static get parameterDescriptors() {
                return c;
            }
            process(e, h, f) {
                let m = e.length, w = h.length, G = 0, l, z, n, t = 4 * this.s, g = 12 * (m + w), Y = F(), A, I, B;
                for (l of e) g += l.length * t;
                for (l of h) g += l.length * t;
                for (l in f) g += f[l].byteLength + 8, ++G;
                A = H(g);
                g = A >> 2;
                n = A + 12 * m;
                for (l of e) {
                    J[g] = l.length;
                    J[g + 1] = this.s;
                    J[g + 2] = n;
                    g += 3;
                    for (z of l) K.set(z, n >> 2), n += t;
                }
                I = n;
                g = I >> 2;
                e = (n += 12 * w) >> 2;
                for (l of h) J[g] = l.length, J[g + 1] = this.s, J[g + 2] = n, g += 3, n += t * l.length;
                t = n;
                g = t >> 2;
                n += 8 * G;
                for (l = 0; B = f[l++]; ) J[g] = B.length, J[g + 1] = n, g += 2, K.set(B, n >> 2), 
                n += 4 * B.length;
                if (f = this.u(m, A, w, I, G, t, this.v)) for (l of h) for (z of l) for (g = 0; g < this.s; ++g) z[g] = K[e++];
                L(Y);
                return !!f;
            }
        }
        return d;
    }
    var M;
    class b extends AudioWorkletProcessor {
        constructor(c) {
            super();
            x(c.processorOptions);
            M = this.port;
            M.onmessage = async d => {
                await r;
                d = d.data;
                d._wpn ? (registerProcessor(d._wpn, a(d.D)), M.postMessage({
                    _wsc: d.u,
                    A: [ d.F, 1, d.v ]
                })) : d._wsc && (await r, E.get(d._wsc)(...d.A));
            };
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
}

function C() {
    var a = y.buffer;
    N = new Uint8Array(a);
    O = new Int32Array(a);
    J = new Uint32Array(a);
    K = new Float32Array(a);
}

p || (y = k.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), C());

var P = [], Q = a => {
    a = a.data;
    let b = a._wsc;
    b && E.get(b)(...a.x);
}, R = a => {
    P.push(a);
}, aa = a => L(a), ba = () => F(), ca = (a, b, c, d) => {
    b = S[b];
    S[a].connect(b.destination || b, c, d);
}, S = {}, T = 0, U = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, V = (a = 0) => {
    for (var b = N, c = a, d = c + void 0; b[c] && !(c >= d); ) ++c;
    if (16 < c - a && b.buffer && U) return U.decode(b.slice(a, c));
    for (d = ""; a < c; ) {
        var e = b[a++];
        if (e & 128) {
            var h = b[a++] & 63;
            if (192 == (e & 224)) d += String.fromCharCode((e & 31) << 6 | h); else {
                var f = b[a++] & 63;
                e = 224 == (e & 240) ? (e & 15) << 12 | h << 6 | f : (e & 7) << 18 | h << 12 | f << 6 | b[a++] & 63;
                65536 > e ? d += String.fromCharCode(e) : (e -= 65536, d += String.fromCharCode(55296 | e >> 10, 56320 | e & 1023));
            }
        } else d += String.fromCharCode(e);
    }
    return d;
}, da = a => {
    var b = window.AudioContext || window.webkitAudioContext;
    if (a >>= 2) {
        var c = J[a] ? (c = J[a]) ? V(c) : "" : void 0;
        a = {
            latencyHint: c,
            sampleRate: O[a + 1] || void 0
        };
    } else a = void 0;
    if (c = b) b = new b(a), S[++T] = b, c = T;
    return c;
}, ea = (a, b, c, d, e) => {
    if (c >>= 2) {
        var h = O[c], f = O[c + 1];
        if (J[c + 2]) {
            var m = J[c + 2] >> 2;
            c = O[c + 1];
            let w = [];
            for (;c--; ) w.push(J[m++]);
            m = w;
        } else m = void 0;
        d = {
            numberOfInputs: h,
            numberOfOutputs: f,
            outputChannelCount: m,
            processorOptions: {
                u: d,
                v: e,
                s: 128
            }
        };
    } else d = void 0;
    a = new AudioWorkletNode(S[a], b ? V(b) : "", d);
    S[++T] = a;
    return T;
}, fa = (a, b, c, d) => {
    b >>= 2;
    let e = [], h = J[b + 1], f = J[b + 2] >> 2, m = 0;
    for (;h--; ) e.push({
        name: m++,
        defaultValue: K[f++],
        minValue: K[f++],
        maxValue: K[f++],
        automationRate: [ "a", "k" ][J[f++]] + "-rate"
    });
    h = S[a].audioWorklet.B.port;
    f = h.postMessage;
    b = (b = J[b]) ? V(b) : "";
    f.call(h, {
        _wpn: b,
        D: e,
        F: a,
        u: c,
        v: d
    });
}, ha = () => !1, ia = 1, ja = a => {
    a = a.data;
    let b = a._wsc;
    b && E.get(b)(...a.A);
}, ka = a => H(a), la = (a, b, c, d, e) => {
    let h = S[a], f = h.audioWorklet, m = () => {
        E.get(d)(a, 0, e);
    };
    if (!f) return m();
    f.addModule(k.js).then((() => {
        f.B = new AudioWorkletNode(h, "em-bootstrap", {
            processorOptions: {
                K: ia++,
                C: k.wasm,
                I: y,
                G: b,
                H: c
            }
        });
        f.B.port.onmessage = ja;
        E.get(d)(a, 1, e);
    })).catch(m);
};

function ma(a) {
    let b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = S[a];
    b.onclick = () => {
        "running" != a.state ? a.resume() : a.suspend();
    };
}

function D() {
    Z = {
        f: ma,
        g: ca,
        d: da,
        h: ea,
        e: fa,
        b: ha,
        c: la,
        a: y
    };
    WebAssembly.instantiate(k.wasm, {
        a: Z
    }).then((a => {
        a = a.instance.exports;
        W = a.j;
        L = a.l;
        H = a.m;
        F = a.n;
        X = a.o;
        E = a.k;
        k.stackSave = ba;
        k.stackAlloc = ka;
        k.stackRestore = aa;
        k.wasmTable = E;
        p ? (X(v.G, v.H), "undefined" === typeof AudioWorkletGlobalScope && (removeEventListener("message", R), 
        P = P.forEach(Q), addEventListener("message", Q))) : a.i();
        u?.(k);
        p || W();
    }));
}

p || D();