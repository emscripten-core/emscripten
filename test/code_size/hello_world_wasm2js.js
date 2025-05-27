var d = Module, g, h, k = new TextDecoder, l;

function e(a) {
    this.exports = function(p) {
        function t(q, b, f) {
            for (var u, n = 0, r = b, v = f.length, x = b + (3 * v >> 2) - ("=" == f[v - 2]) - ("=" == f[v - 1]); n < v; n += 4) b = c[f.charCodeAt(n + 1)], 
            u = c[f.charCodeAt(n + 2)], q[r++] = c[f.charCodeAt(n)] << 2 | b >> 4, r < x && (q[r++] = b << 4 | u >> 2), 
            r < x && (q[r++] = u << 6 | c[f.charCodeAt(n + 3)]);
            return q;
        }
        for (var w, c = new Uint8Array(123), m = 25; 0 <= m; --m) c[48 + m] = 52 + m, c[65 + m] = m, 
        c[97 + m] = 26 + m;
        c[43] = 62;
        c[47] = 63;
        return function(q) {
            var b = new ArrayBuffer(16908288), f = new Uint8Array(b), u = q.a.a;
            w = f;
            t(w, 1024, "aGVsbG8h");
            return {
                b: Object.create(Object.prototype, {
                    grow: {},
                    buffer: {
                        get: function() {
                            return b;
                        }
                    }
                }),
                c: function() {},
                d: function(n, r) {
                    u(1024);
                    return 0;
                }
            };
        }(p);
    }(a);
}

(function(a, p) {
    return {
        then: function(t) {
            t({
                instance: new e(p)
            });
        }
    };
})(d.wasm, {
    a: {
        a: a => {
            var p = console, t = p.log;
            if (a) {
                for (var w = a + void 0, c = a; !(c >= w) && h[c]; ) ++c;
                a = k.decode(h.subarray(a, c));
            } else a = "";
            t.call(p, a);
        }
    }
}).then((a => {
    a = a.instance.exports;
    l = a.d;
    g = a.b;
    h = new Uint8Array(g.buffer);
    a.c();
    l();
}));