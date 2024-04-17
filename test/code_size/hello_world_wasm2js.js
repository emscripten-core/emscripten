var d = Module, WebAssembly = {
    Memory: function(b) {
        this.buffer = new ArrayBuffer(65536 * b.initial);
    },
    Module: function() {},
    Instance: function(b, p) {
        (function(t) {
            function u(c) {
                c.set = function(a, e) {
                    this[a] = e;
                };
                c.get = function(a) {
                    return this[a];
                };
                return c;
            }
            function q(c, a, e) {
                for (var v, n = 0, r = a, w = e.length, x = a + (3 * w >> 2) - ("=" == e[w - 2]) - ("=" == e[w - 1]); n < w; n += 4) a = f[e.charCodeAt(n + 1)], 
                v = f[e.charCodeAt(n + 2)], c[r++] = f[e.charCodeAt(n)] << 2 | a >> 4, r < x && (c[r++] = a << 4 | v >> 2), 
                r < x && (c[r++] = v << 6 | f[e.charCodeAt(n + 3)]);
            }
            for (var y, f = new Uint8Array(123), m = 25; 0 <= m; --m) f[48 + m] = 52 + m, f[65 + m] = m, 
            f[97 + m] = 26 + m;
            f[43] = 62;
            f[47] = 63;
            return function(c) {
                var a = new ArrayBuffer(16908288), e = new Uint8Array(a), v = c.a.a;
                y = e;
                q(y, 1024, "aGVsbG8h");
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
                    d: function(n, r) {
                        v(1024);
                        return 0;
                    },
                    e: c
                };
            }(t);
        })(p);
    },
    instantiate: function(b, p) {
        return {
            then: function(t) {
                var u = new WebAssembly.Module(b);
                t({
                    instance: new WebAssembly.Instance(u, p)
                });
            }
        };
    },
    RuntimeError: Error
}, g, h, k = new TextDecoder("utf8"), l;

WebAssembly.instantiate(d.wasm, {
    a: {
        a: b => {
            var p = console, t = p.log;
            if (b) {
                for (var u = b + void 0, q = b; !(q >= u) && g[q]; ) ++q;
                b = k.decode(g.subarray(b, q));
            } else b = "";
            t.call(p, b);
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