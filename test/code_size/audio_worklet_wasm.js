var g = globalThis.Module || "undefined" != typeof Module ? Module : {}, k = "undefined" !== typeof AudioWorkletGlobalScope, m = "em-ww" == globalThis.name, n, p, q, r, t, x, U, V, A, Q, C, W;

m && !k && (onmessage = a => {
    onmessage = null;
    a = a.data;
    g ||= {};
    Object.assign(g, a);
    t = a.mem;
    u();
    v();
    a.wasm = a.mem = 0;
});

function u() {
    var a = t.buffer;
    p = new Uint8Array(a);
    n = new Int32Array(a);
    g.HEAPU32 = q = new Uint32Array(a);
    g.HEAPF32 = r = new Float32Array(a);
}

if (k || !m) t = g.mem || new WebAssembly.Memory({
    initial: 256,
    maximum: 256,
    shared: !0
}), u();

var w = [], y = a => {
    a = a.data;
    let b = a._wsc;
    b && x.get(b)(...a.x);
}, z = a => {
    w.push(a);
}, B = a => A(a), D = () => C(), G = (a, b, c, e) => {
    b = E[b];
    E[a].connect(b.destination || b, c, e);
}, E = {}, H = 0, I = "undefined" != typeof TextDecoder ? new TextDecoder : void 0, J = (a = 0) => {
    for (var b = p, c = a + NaN, e = a; b[e] && !(e >= c); ) ++e;
    if (16 < e - a && b.buffer && I) return I.decode(b.slice(a, e));
    for (c = ""; a < e; ) {
        var d = b[a++];
        if (d & 128) {
            var h = b[a++] & 63;
            if (192 == (d & 224)) c += String.fromCharCode((d & 31) << 6 | h); else {
                var f = b[a++] & 63;
                d = 224 == (d & 240) ? (d & 15) << 12 | h << 6 | f : (d & 7) << 18 | h << 12 | f << 6 | b[a++] & 63;
                65536 > d ? c += String.fromCharCode(d) : (d -= 65536, c += String.fromCharCode(55296 | d >> 10, 56320 | d & 1023));
            }
        } else c += String.fromCharCode(d);
    }
    return c;
}, K = a => {
    var b = window.AudioContext || window.webkitAudioContext;
    if (a >>= 2) {
        var c = q[a] ? (c = q[a]) ? J(c) : "" : void 0;
        a = {
            latencyHint: c,
            sampleRate: n[a + 1] || void 0
        };
    } else a = void 0;
    if (c = b) b = new b(a), E[++H] = b, c = H;
    return c;
}, L = (a, b, c, e, d) => {
    if (c >>= 2) {
        var h = n[c], f = n[c + 1];
        if (q[c + 2]) {
            var l = q[c + 2] >> 2;
            c = n[c + 1];
            let F = [];
            for (;c--; ) F.push(q[l++]);
            l = F;
        } else l = void 0;
        e = {
            numberOfInputs: h,
            numberOfOutputs: f,
            outputChannelCount: l,
            processorOptions: {
                cb: e,
                ud: d,
                sc: 128
            }
        };
    } else e = void 0;
    a = new AudioWorkletNode(E[a], b ? J(b) : "", e);
    E[++H] = a;
    return H;
}, M = (a, b, c, e) => {
    b >>= 2;
    let d = [], h = q[b + 1], f = q[b + 2] >> 2, l = 0;
    for (;h--; ) d.push({
        name: l++,
        defaultValue: r[f++],
        minValue: r[f++],
        maxValue: r[f++],
        automationRate: [ "a", "k" ][q[f++]] + "-rate"
    });
    h = E[a].audioWorklet.s.port;
    f = h.postMessage;
    b = (b = q[b]) ? J(b) : "";
    f.call(h, {
        _wpn: b,
        ap: d,
        ch: a,
        cb: c,
        ud: e
    });
}, N = () => !1, O = 1, P = a => {
    a = a.data;
    let b = a._wsc;
    b && x.get(b)(...a.x);
}, R = a => Q(a), S = (a, b, c, e, d) => {
    let h = E[a], f = h.audioWorklet, l = () => {
        x.get(e)(a, 0, d);
    };
    if (!f) return l();
    f.addModule("a.aw.js").then((() => {
        f.s = new AudioWorkletNode(h, "message", {
            processorOptions: {
                $ww: O++,
                wasm: g.wasm,
                mem: t,
                sb: b,
                sz: c
            }
        });
        f.s.port.onmessage = P;
        return f.addModule(g.js);
    })).then((() => {
        x.get(e)(a, 1, d);
    })).catch(l);
};

function T(a) {
    let b = document.createElement("button");
    b.innerHTML = "Toggle playback";
    document.body.appendChild(b);
    a = E[a];
    b.onclick = () => {
        "running" != a.state ? a.resume() : a.suspend();
    };
}

function v() {
    U = {
        f: T,
        g: G,
        d: K,
        h: L,
        e: M,
        b: N,
        c: S,
        a: t
    };
    WebAssembly.instantiate(g.wasm, {
        a: U
    }).then((a => {
        a = a.instance.exports;
        V = a.j;
        A = a.l;
        Q = a.m;
        C = a.n;
        W = a.o;
        x = a.k;
        g.stackSave = D;
        g.stackAlloc = R;
        g.stackRestore = B;
        g.wasmTable = x;
        m ? (a = g, W(a.sb, a.sz), "undefined" === typeof AudioWorkletGlobalScope && (removeEventListener("message", z), 
        w = w.forEach(y), addEventListener("message", y))) : a.i();
        m || V();
    }));
}

k && v();

m || v();