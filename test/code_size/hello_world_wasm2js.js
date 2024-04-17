var d = Module, g, h, k = new TextDecoder, l;

function e(b) {
    this.exports = function(t) {
        function u(f) {
            f.set = function(a, m) {
                this[a] = m;
            };
            f.get = function(a) {
                return this[a];
            };
            return f;
        }
        function x(f, a, m) {
            for (var v, n = 0, c = a, w = m.length, y = a + (3 * w >> 2) - ("=" == m[w - 2]) - ("=" == m[w - 1]); n < w; n += 4) a = p[m.charCodeAt(n + 1)], 
            v = p[m.charCodeAt(n + 2)], f[c++] = p[m.charCodeAt(n)] << 2 | a >> 4, c < y && (f[c++] = a << 4 | v >> 2), 
            c < y && (f[c++] = v << 6 | p[m.charCodeAt(n + 3)]);
        }
        for (var r, p = new Uint8Array(123), q = 25; 0 <= q; --q) p[48 + q] = 52 + q, p[65 + q] = q, 
        p[97 + q] = 26 + q;
        p[43] = 62;
        p[47] = 63;
        return function(f) {
            var a = new ArrayBuffer(16908288), m = new Uint8Array(a), v = f.a.a, n = 66576;
            r = m;
            x(r, 1024, "aGVsbG8h");
            f = u([]);
            return {
                b: Object.create(Object.prototype, {
                    grow: {},
                    buffer: {
                        get: function() {
                            return a;
                        }
                    }
                }),
                c: function() {},
                d: function(c, w) {
                    v(1024);
                    return 0;
                },
                e: f,
                f: function(c) {
                    n = c | 0;
                },
                g: function(c) {
                    n = c = n - (c | 0) & -16;
                    return c | 0;
                },
                h: function() {
                    return n | 0;
                }
            };
        }(t);
    }(b);
}

(function(b, t) {
    return {
        then: function(u) {
            u({
                instance: new e(t)
            });
        }
    };
})(d.wasm, {
    a: {
        a: b => {
            var t = console, u = t.log;
            if (b) {
                for (var x = b + void 0, r = b; !(r >= x) && g[r]; ) ++r;
                b = k.decode(g.subarray(b, r));
            } else b = "";
            u.call(t, b);
        }
    }
}).then((b => {
    b = b.instance.exports;
    l = b.d;
    h = b.b;
    g = new Uint8Array(h.buffer);
    b.c();
    l();
}));