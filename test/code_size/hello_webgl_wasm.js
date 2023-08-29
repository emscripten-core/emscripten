var Module = function(D = {}) {
    function E(a) {
        f.pixelStorei(37440, !0);
        f.texImage2D(3553, 0, 6408, 6408, 5121, a);
        f.pixelStorei(37440, !1);
    }
    var g, r, F, v, x, G, H, N = new TextDecoder("utf8"), t = (a, b) => {
        if (!a) return "";
        b = a + b;
        for (var c = a; !(c >= b) && r[c]; ) ++c;
        return N.decode(r.subarray(a, c));
    }, O = [ 0, document, window ], y = a => {
        a = 2 < a ? t(a) : a;
        return O[a] || document.querySelector(a);
    }, P = 1, I = [], p = [], z = [], u = [], A = [], w = a => {
        for (var b = P++, c = a.length; c < b; c++) a[c] = null;
        return b;
    }, B, Q = [ "default", "low-power", "high-performance" ], J = (a, b, c, d) => {
        for (var e = 0; e < a; e++) {
            var h = f[c](), k = h && w(d);
            h && (h.name = k, d[k] = h);
            g[b + 4 * e >> 2] = k;
        }
    }, K = a => "]" == a.slice(-1) && a.lastIndexOf("["), L = a => {
        var b = f.U, c = b.O[a];
        "number" == typeof c && (b.O[a] = c = f.getUniformLocation(b, b.R[a] + (0 < c ? "[" + c + "]" : "")));
        return c;
    }, f, M;
    WebAssembly.instantiate(D.wasm, {
        a: {
            z: () => devicePixelRatio,
            b: () => Math.random(),
            J: (a, b) => {
                function c(d) {
                    H.get(a)(d, b) && requestAnimationFrame(c);
                }
                return requestAnimationFrame(c);
            },
            k: () => !1,
            j: (a, b, c) => {
                a = y(a);
                if (!a) return -4;
                a.width = b;
                a.height = c;
                return 0;
            },
            p: (a, b, c) => {
                a = y(a);
                if (!a) return -4;
                a.style.width = b + "px";
                a.style.height = c + "px";
                return 0;
            },
            h: (a, b) => {
                b >>= 2;
                b = {
                    alpha: !!g[b],
                    depth: !!g[b + 1],
                    stencil: !!g[b + 2],
                    antialias: !!g[b + 3],
                    premultipliedAlpha: !!g[b + 4],
                    preserveDrawingBuffer: !!g[b + 5],
                    powerPreference: Q[g[b + 6]],
                    failIfMajorPerformanceCaveat: !!g[b + 7],
                    W: g[b + 8],
                    ca: g[b + 9],
                    Z: g[b + 10],
                    V: g[b + 11],
                    da: g[b + 12],
                    ea: g[b + 13]
                };
                a = y(a);
                if (!a || b.V) b = 0; else if (a = a.getContext("webgl", b)) {
                    var c = w(A);
                    b = {
                        handle: c,
                        attributes: b,
                        version: b.W,
                        T: a
                    };
                    a.canvas && (a.canvas.X = b);
                    A[c] = b;
                    b = c;
                } else b = 0;
                return b;
            },
            i: a => {
                a >>= 2;
                for (var b = 0; 14 > b; ++b) g[a + b] = 0;
                g[a] = g[a + 1] = g[a + 3] = g[a + 4] = g[a + 8] = g[a + 10] = 1;
            },
            g: a => {
                B = A[a];
                D.Y = f = B && B.T;
                return !a || f ? 0 : -5;
            },
            c: (a, b) => {
                f.attachShader(p[a], u[b]);
            },
            I: (a, b, c) => {
                f.bindAttribLocation(p[a], b, t(c));
            },
            C: (a, b) => {
                f.bindBuffer(a, I[b]);
            },
            d: (a, b) => {
                f.bindTexture(a, z[b]);
            },
            E: function(a, b) {
                f.blendFunc(a, b);
            },
            B: (a, b, c, d) => {
                f.bufferData(a, c ? r.subarray(c, c + b) : b, d);
            },
            s: function(a) {
                f.clear(a);
            },
            t: function(a, b, c, d) {
                f.clearColor(a, b, c, d);
            },
            u: a => {
                f.compileShader(u[a]);
            },
            f: () => {
                var a = w(p), b = f.createProgram();
                b.name = a;
                b.ba = b.$ = b.aa = 0;
                b.P = 1;
                p[a] = b;
                return a;
            },
            w: a => {
                var b = w(u);
                u[b] = f.createShader(a);
                return b;
            },
            o: (a, b, c) => {
                f.drawArrays(a, b, c);
            },
            F: function(a) {
                f.enable(a);
            },
            y: a => {
                f.enableVertexAttribArray(a);
            },
            D: (a, b) => {
                J(a, b, "createBuffer", I);
            },
            l: (a, b) => {
                J(a, b, "createTexture", z);
            },
            e: (a, b) => {
                b = t(b);
                if (a = p[a]) {
                    var c = a, d = c.O, e = c.S, h;
                    if (!d) for (c.O = d = {}, c.R = {}, h = 0; h < f.getProgramParameter(c, 35718); ++h) {
                        var k = f.getActiveUniform(c, h), m = k.name;
                        k = k.size;
                        var l = K(m);
                        l = 0 < l ? m.slice(0, l) : m;
                        var q = c.P;
                        c.P += k;
                        e[l] = [ k, q ];
                        for (m = 0; m < k; ++m) d[q] = m, c.R[q++] = l;
                    }
                    c = a.O;
                    d = 0;
                    e = b;
                    h = K(b);
                    0 < h && (d = parseInt(b.slice(h + 1)) >>> 0, e = b.slice(0, h));
                    if ((e = a.S[e]) && d < e[0] && (d += e[1], c[d] = c[d] || f.getUniformLocation(a, b))) return d;
                }
                return -1;
            },
            H: a => {
                a = p[a];
                f.linkProgram(a);
                a.O = 0;
                a.S = {};
            },
            v: (a, b, c, d) => {
                for (var e = "", h = 0; h < b; ++h) {
                    var k = d ? g[d + 4 * h >> 2] : -1;
                    e += t(g[c + 4 * h >> 2], 0 > k ? void 0 : k);
                }
                f.shaderSource(u[a], e);
            },
            x: (a, b, c, d, e, h, k, m, l) => {
                var q = f, R = q.texImage2D;
                if (l) {
                    var n = m - 5120;
                    n = 1 == n ? r : 4 == n ? g : 6 == n ? x : 5 == n || 28922 == n ? v : F;
                    var C = 31 - Math.clz32(n.BYTES_PER_ELEMENT);
                    l = n.subarray(l >> C, l + e * (d * (({
                        5: 3,
                        6: 4,
                        8: 2,
                        29502: 3,
                        29504: 4
                    }[k - 6402] || 1) << C) + 4 - 1 & -4) >> C);
                } else l = null;
                R.call(q, a, b, c, d, e, h, k, m, l);
            },
            a: function(a, b, c) {
                f.texParameteri(a, b, c);
            },
            q: (a, b, c, d, e) => {
                f.uniform4f(L(a), b, c, d, e);
            },
            r: (a, b, c, d) => {
                b = x.subarray(d >> 2, d + 64 * b >> 2);
                f.uniformMatrix4fv(L(a), !!c, b);
            },
            G: a => {
                a = p[a];
                f.useProgram(a);
                f.U = a;
            },
            A: (a, b, c, d, e, h) => {
                f.vertexAttribPointer(a, b, c, !!d, e, h);
            },
            n: function(a, b, c, d) {
                var e = new Image;
                e.onload = () => {
                    v[c >> 2] = e.width;
                    v[d >> 2] = e.height;
                    f.bindTexture(3553, z[a]);
                    E(e);
                };
                e.src = t(b);
            },
            m: function(a, b, c) {
                var d = document.createElement("canvas");
                d.width = d.height = b;
                var e = d.getContext("2d");
                e.fillStyle = "black";
                e.globalCompositeOperator = "copy";
                e.globalAlpha = 0;
                e.fillRect(0, 0, d.width, d.height);
                e.globalAlpha = 1;
                e.fillStyle = "white";
                e.font = b + "px Arial Unicode";
                c && (e.shadowColor = "black", e.shadowOffsetX = 2, e.shadowOffsetY = 2, e.shadowBlur = 3, 
                e.strokeStyle = "gray", e.strokeText(String.fromCharCode(a), 0, d.height - 7));
                e.fillText(String.fromCharCode(a), 0, d.height - 7);
                E(d);
            }
        }
    }).then((a => {
        a = a.instance.exports;
        M = a.M;
        H = a.N;
        G = a.K;
        var b = G.buffer;
        r = new Uint8Array(b);
        F = new Uint16Array(b);
        g = new Int32Array(b);
        v = new Uint32Array(b);
        x = new Float32Array(b);
        a.L();
        M();
    }));
    return {};
};