var h = globalThis.Module || "undefined" != typeof Module ? Module : {}, p = "em-ww" == globalThis.name, q = "undefined" !== typeof AudioWorkletGlobalScope, r, t, v, x, y, H, X, Y, K, J, I, Z;

q && (p = !0);

function C(a) {
    h ||= {};
    Object.assign(h, a);
    y = a.mem;
    D();
    F();
    a.wasm = a.mem = 0;
}

p && !q && (onmessage = a => {
    onmessage = null;
    C(a.data);
});

if (q) {
    function a(c) {
        class e extends AudioWorkletProcessor {
            constructor(d) {
                super();
                d = d.processorOptions;
                this.v = H.get(d.cb);
                this.A = d.ud;
                this.s = d.sc;
            }
            static get parameterDescriptors() {
                return c;
            }
            process(d, k, f) {
                let m = d.length, w = k.length, E = 0, l, z, n, u = 4 * this.s, g = 12 * (m + w), V = I(), A, G, B;
                for (l of d) g += l.length * u;
                for (l of k) g += l.length * u;
                for (l in f) g += f[l].byteLength + 8, ++E;
                A = J(g);
                g = A >> 2;
                n = A + 12 * m;
                for (l of d) {
                    v[g] = l.length;
                    v[g + 1] = this.s;
                    v[g + 2] = n;
                    g += 3;
                    for (z of l) x.set(z, n >> 2), n += u;
                }
                G = n;
                g = G >> 2;
                d = (n += 12 * w) >> 2;
                for (l of k) v[g] = l.length, v[g + 1] = this.s, v[g + 2] = n, g += 3, n += u * l.length;
                u = n;
                g = u >> 2;
                n += 8 * E;
                for (l = 0; B = f[l++]; ) v[g] = B.length, v[g + 1] = n, g += 2, x.set(B, n >> 2), 
                n += 4 * B.length;
                if (f = this.v(m, A, w, G, E, u, this.A)) for (l of k) for (z of l) for (g = 0; g < this.s; ++g) z[g] = x[d++];
                K(V);
                return !!f;
            }
        }
        return e;
    }
    var L;
    class b extends AudioWorkletProcessor {
        constructor(c) {
            super();
            C(c.processorOptions);
            L = this.port;
            L.onmessage = async e => {
                e = e.data;
                e._wpn ? (registerProcessor(e._wpn, a(e.ap)), L.postMessage({
                    _wsc: e.cb,
                    x: [ e.ch, 1, e.ud ]
                })) : e._wsc && H.get(e._wsc)(...e.x);
            };
        }
        process() {}
    }
    registerProcessor("em-bootstrap", b);
}

function D() {
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
}), D());

var M = [], N = a => {
    a = a.data;
    let b = a._wsc;
    b && H.get(b)(...a.x);
}, O = a => {
    M.push(a);
}, P = a => K(a), Q = () => I(), S = (a, b, c, e) => {
    b = R[b];
    R[a].connect(b.destination || b, c, e);
}, R = {}, T = 0, U = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, W = (a = 0) => {
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
}, aa = a => {
    var b = window.AudioContext || window.webkitAudioContext;
    if (a >>= 2) {
        var c = v[a] ? (c = v[a]) ? W(c) : "" : void 0;
        a = {
            latencyHint: c,
            sampleRate: r[a + 1] || void 0
        };
    } else a = void 0;
    if (c = b) b = new b(a), R[++T] = b, c = T;
    return c;
}, ba = (a, b, c, e, d) => {
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
                cb: e,
                ud: d,
                sc: 128
            }
        };
    } else e = void 0;
    a = new AudioWorkletNode(R[a], b ? W(b) : "", e);
    R[++T] = a;
    return T;
}, ca = (a, b, c, e) => {
    b >>= 2;
    let d = [], k = v[b + 1], f = v[b + 2] >> 2, m = 0;
    for (;k--; ) d.push({
        name: m++,
        defaultValue: x[f++],
        minValue: x[f++],
        maxValue: x[f++],
        automationRate: [ "a", "k" ][v[f++]] + "-rate"
    });
    k = R[a].audioWorklet.u.port;
    f = k.postMessage;
    b = (b = v[b]) ? W(b) : "";
    f.call(k, {
        _wpn: b,
        ap: d,
        ch: a,
        cb: c,
        ud: e
    });
}, da = () => !1, ea = 1, fa = a => {
    a = a.data;
    let b = a._wsc;
    b && H.get(b)(...a.x);
}, ha = a => J(a), ia = (a, b, c, e, d) => {
    let k = R[a], f = k.audioWorklet, m = () => {
        H.get(e)(a, 0, d);
    };
    if (!f) return m();
    f.addModule(h.js).then((() => {
        f.u = new AudioWorkletNode(k, "em-bootstrap", {
            processorOptions: {
                $ww: ea++,
                wasm: h.wasm,
                mem: y,
                sb: b,
                sz: c
            }
        });
        f.u.port.onmessage = fa;
        H.get(e)(a, 1, d);
    })).catch(m);
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

function F() {
    X = {
        f: ja,
        g: S,
        d: aa,
        h: ba,
        e: ca,
        b: da,
        c: ia,
        a: y
    };
    WebAssembly.instantiate(h.wasm, {
        a: X
    }).then((a => {
        a = a.instance.exports;
        Y = a.j;
        K = a.l;
        J = a.m;
        I = a.n;
        Z = a.o;
        H = a.k;
        h.stackSave = Q;
        h.stackAlloc = ha;
        h.stackRestore = P;
        h.wasmTable = H;
        p ? (a = h, Z(a.sb, a.sz), "undefined" === typeof AudioWorkletGlobalScope && (removeEventListener("message", O), 
        M = M.forEach(N), addEventListener("message", N))) : a.i();
        p || Y();
    }));
}

p || F();