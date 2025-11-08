var d = Module, g, h = new TextDecoder, k, l;

function e(a) {
    this.exports = function(q) {
        function u(r, b, m) {
            for (var v, p = 0, t = b, w = m.length, x = b + (3 * w >> 2) - ("=" == m[w - 2]) - ("=" == m[w - 1]); p < w; p += 4) b = c[m.charCodeAt(p + 1)], 
            v = c[m.charCodeAt(p + 2)], r[t++] = c[m.charCodeAt(p)] << 2 | b >> 4, t < x && (r[t++] = b << 4 | v >> 2), 
            t < x && (r[t++] = v << 6 | c[m.charCodeAt(p + 3)]);
            return r;
        }
        for (var n, c = new Uint8Array(123), f = 25; 0 <= f; --f) c[48 + f] = 52 + f, c[65 + f] = f, 
        c[97 + f] = 26 + f;
        c[43] = 62;
        c[47] = 63;
        return function(r) {
            var b = new ArrayBuffer(16908288), m = new Uint8Array(b), v = r.a.a;
            n = m;
            u(n, 1024, "aGVsbG8h");
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
                d: function(p, t) {
                    v(1024);
                    return 0;
                }
            };
        }(q);
    }(a);
}

(function(a, q) {
    return {
        then: function(u) {
            u({
                instance: new e(q)
            });
        }
    };
})(d.wasm, {
    a: {
        a: a => {
            var q = console, u = q.log;
            if (a) {
                for (var n = a, c = g, f = n + void 0; c[n] && !(n >= f); ) ++n;
                a = h.decode(g.subarray(a, n));
            } else a = "";
            u.call(q, a);
        }
    }
}).then((a => {
    a = a.instance.exports;
    k = a.d;
    l = a.b;
    g = new Uint8Array(l.buffer);
    a.c();
    k();
}));