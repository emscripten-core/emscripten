var c = Module, g, h, k = new TextDecoder("utf8"), l;

function d(b) {
    this.exports = function(f) {
        function m(e) {
            e.set = function(a, n) {
                this[a] = n;
            };
            e.get = function(a) {
                return this[a];
            };
            return e;
        }
        return function(e) {
            var a = new ArrayBuffer(16777216), n = e.a.a;
            e = m([]);
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
                d: function(p, q) {
                    n(1024);
                    return 0;
                },
                e: e
            };
        }(f);
    }(b);
}

(function(b, f) {
    return {
        then: function(m) {
            m({
                instance: new d(f)
            });
        }
    };
})(c.wasm, {
    a: {
        a: b => {
            var f = console, m = f.log;
            if (b) {
                for (var e = b + void 0, a = b; !(a >= e) && g[a]; ) ++a;
                b = k.decode(g.subarray(b, a));
            } else b = "";
            m.call(f, b);
        }
    }
}).then((b => {
    b = b.instance.exports;
    l = b.d;
    h = b.b;
    g = new Uint8Array(h.buffer);
    g.set(new Uint8Array(c.mem), 1024);
    b.c();
    l();
}));