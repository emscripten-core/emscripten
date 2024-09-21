var d = Module, g, h, k = new TextDecoder, l;

function e(b) {
    this.exports = function(r) {
        function u(c) {
            c.set = function(a, f) {
                this[a] = f;
            };
            c.get = function(a) {
                return this[a];
            };
            return c;
        }
        function x(c, a, f) {
            for (var v, p = 0, t = a, w = f.length, y = a + (3 * w >> 2) - ("=" == f[w - 2]) - ("=" == f[w - 1]); p < w; p += 4) a = m[f.charCodeAt(p + 1)], 
            v = m[f.charCodeAt(p + 2)], c[t++] = m[f.charCodeAt(p)] << 2 | a >> 4, t < y && (c[t++] = a << 4 | v >> 2), 
            t < y && (c[t++] = v << 6 | m[f.charCodeAt(p + 3)]);
        }
        for (var q, m = new Uint8Array(123), n = 25; 0 <= n; --n) m[48 + n] = 52 + n, m[65 + n] = n, 
        m[97 + n] = 26 + n;
        m[43] = 62;
        m[47] = 63;
        return function(c) {
            var a = new ArrayBuffer(16908288), f = new Uint8Array(a), v = c.a.a;
            q = f;
            x(q, 1024, "aGVsbG8h");
            c = u([]);
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
                d: function(p, t) {
                    v(1024);
                    return 0;
                },
                e: c
            };
        }(r);
    }(b);
}

(function(b, r) {
    return {
        then: function(u) {
            u({
                instance: new e(r)
            });
        }
    };
})(d.wasm, {
    a: {
        a: b => {
            var r = console, u = r.log;
            if (b) {
                for (var x = b + void 0, q = b; !(q >= x) && g[q]; ) ++q;
                b = k.decode(g.subarray(b, q));
            } else b = "";
            u.call(r, b);
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