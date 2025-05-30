var h = globalThis.Module || "undefined" != typeof Module ? Module : {}, p = "em-ww" == globalThis.name, q = "undefined" !== typeof AudioWorkletGlobalScope, r, t, v, x, y, C, I, X, L, K, J, Y, Z;

q && (p = !0);

function D(a) {
    C = a;
    y = a.I;
    E();
    h ||= {};
    h.wasm = a.C;
    G();
    a.C = a.J = 0;
}

p && !q && (onmessage = a => {
    onmessage = null;
    D(a.data);
});

if (q) {
    function a(c) {
        class e extends AudioWorkletProcessor {
            constructor(d) {
                super();
                d = d.processorOptions;
                this.u = I.get(d.u);
                this.v = d.v;
                this.s = d.s;
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, k, f) {
                let m = d.length, w = k.length, F = 0, l, z, n, u = 4 * this.s, g = 12 * (m + w), W = J(), A, H, B;
                for (l of d) g += l.length * u;
                for (l of k) g += l.length * u;
                for (l in f) g += f[l].byteLength + 8, ++F;
                A = K(g);
                g = A >> 2;
                n = A + 12 * m;
                for (l of d) {
                    v[g] = l.length;
                    v[g + 1] = this.s;
                    v[g + 2] = n;
                    g += 3;
                    for (z of l) x.set(z, n >> 2), n += u;
                }
                H = n;
                g = H >> 2;
                d = (n += 12 * w) >> 2;
                for (l of k) v[g] = l.length, v[g + 1] = this.s, v[g + 2] = n, g += 3, n += u * l.length;
                u = n;
                g = u >> 2;
                n += 8 * F;
                for (l = 0; B = f[l++]; ) v[g] = B.length, v[g + 1] = n, g += 2, x.set(B, n >> 2), 
                n += 4 * B.length;
                if (f = this.u(m, A, w, H, F, u, this.v)) for (l of k) for (z of l) for (g = 0; g < this.s; ++g) z[g] = x[d++];
                L(W);
                return !!f;
            }
        }
        return e;
    }
    var M;
    class b extends AudioWorkletProcessor {
        constructor(c) {
            super();
            D(c.processorOptions);
            M = this.port;
            M.onmessage = async e => {
                e = e.data;
                e._wpn ? (registerProcessor(e._wpn, a(e.D)), M.postMessage({
                    _wsc: e.u,
                    A: [ e.F, 1, e.v ]
                })) : e._wsc && I.get(e._wsc)(...e.A);
            };
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
}

function E() {
    var a = y.buffer;
    t = new Uint8Array(a);
    r = new Int32Array(a);
    v = new Uint32Array(a);
    x = new Float32Array(a);
}

p || (y = h.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), E());

var N = [], O = a => {
    a = a.data;
    let b = a._wsc;
    b && I.get(b)(...a.x);
}, P = a => {
    N.push(a);
}, Q = a => L(a), R = () => J(), aa = (a, b, c, e) => {
    b = S[b];
    S[a].connect(b.destination || b, c, e);
}, S = {}, T = 0, U = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, V = (a = 0) => {
    for (var b = t, c = a + NaN, e = a; b[e] && !(e >= c); ) ++e;
    if (16 < e - a && b.buffer && U) return U.decode(b.slice(a, e));
    for (c = ""; a < e; ) {
        var d = b[a++];
        if (d & 128) {
            var k = b[a++] & 63;
            if (192 == (d & 224)) c += String.fromCharCode((d & 31) << 6 | k); else {
                var f = b[a++] & 63;
                d = 224 == (d & 240) ? (d & 15) << 12 | k << 6 | f : (d & 7) << 18 | k << 12 | f << 6 | b[a++] & 63;
                65536 > d ? c += String.fromCharCode(d) : (d -= 65536, c += String.fromCharCode(55296 | d >> 10, 56320 | d & 1023));
            }
        } else c += String.fromCharCode(d);
    }
    return c;
}, ba = a => {
    var b = window.AudioContext || window.webkitAudioContext;
    if (a >>= 2) {
        var c = v[a] ? (c = v[a]) ? V(c) : "" : void 0;
        a = {
            latencyHint: c,
            sampleRate: r[a + 1] || void 0
        };
    } else a = void 0;
    if (c = b) b = new b(a), S[++T] = b, c = T;
    return c;
}, ca = (a, b, c, e, d) => {
    if (c >>= 2) {
        var k = r[c], f = r[c + 1];
        if (v[c + 2]) {
            var m = v[c + 2] >> 2;
            c = r[c + 1];
            let w = [];
            for (;c--; ) w.push(v[m++]);
            m = w;
        } else m = void 0;
        e = {
            numberOfInputs: k,
            numberOfOutputs: f,
            outputChannelCount: m,
            processorOptions: {
                u: e,
                v: d,
                s: 128
            }
        };
    } else e = void 0;
    a = new AudioWorkletNode(S[a], b ? V(b) : "", e);
    S[++T] = a;
    return T;
}, da = (a, b, c, e) => {
    b >>= 2;
    let d = [], k = v[b + 1], f = v[b + 2] >> 2, m = 0;
    for (;k--; ) d.push({
        name: m++,
        defaultValue: x[f++],
        minValue: x[f++],
        maxValue: x[f++],
        automationRate: [ "a", "k" ][v[f++]] + "-rate"
    });
    k = S[a].audioWorklet.B.port;
    f = k.postMessage;
    b = (b = v[b]) ? V(b) : "";
    f.call(k, {
        _wpn: b,
        D: d,
        F: a,
        u: c,
        v: e
    });
}, ea = () => !1, fa = 1, ha = a => {
    a = a.data;
    let b = a._wsc;
    b && I.get(b)(...a.A);
}, ia = a => K(a), ja = (a, b, c, e, d) => {
    let k = S[a], f = k.audioWorklet, m = () => {
        I.get(e)(a, 0, d);
    };
    if (!f) return m();
    f.addModule(h.js).then((() => {
        f.B = new AudioWorkletNode(k, "em-bootstrap", {
            processorOptions: {
                K: fa++,
                C: h.wasm,
                I: y,
                G: b,
                H: c
            }
        });
        f.B.port.onmessage = ha;
        I.get(e)(a, 1, d);
    })).catch(m);
};

function ka(a) {
    let b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = S[a];
    b.onclick = () => {
        "running" != a.state ? a.resume() : a.suspend();
    };
}

function G() {
    Z = {
        f: ka,
        g: aa,
        d: ba,
        h: ca,
        e: da,
        b: ea,
        c: ja,
        a: y
    };
    WebAssembly.instantiate(h.wasm, {
        a: Z
    }).then((a => {
        a = a.instance.exports;
        X = a.j;
        L = a.l;
        K = a.m;
        J = a.n;
        Y = a.o;
        I = a.k;
        h.stackSave = R;
        h.stackAlloc = ia;
        h.stackRestore = Q;
        h.wasmTable = I;
        p ? (Y(C.G, C.H), "undefined" === typeof AudioWorkletGlobalScope && (removeEventListener("message", P), 
        N = N.forEach(O), addEventListener("message", O))) : a.i();
        p || X();
    }));
}

p || G();