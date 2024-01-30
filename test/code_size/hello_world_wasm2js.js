var c = Module, g, h, k = new TextDecoder("utf8"), l;

function d(e) {
    this.exports = function(m) {
        function n(f) {
            f.set = function(a, p) {
                this[a] = p;
            };
            f.get = function(a) {
                return this[a];
            };
            return f;
        }
        return function(f) {
            var a = new ArrayBuffer(16908288), p = new Int8Array(a), r = Math.imul, t = f.a.a;
            f = n([]);
            return {
                b: Object.create(Object.prototype, {
                    grow: {
                        value: function(b) {
                            var q = a.byteLength / 65536 | 0;
                            b = q + (b | 0) | 0;
                            q < b && 65536 > b && (b = new ArrayBuffer(r(b, 65536)), new Int8Array(b).set(p), 
                            p = new Int8Array(b), a = b);
                            return q;
                        }
                    },
                    buffer: {
                        get: function() {
                            return a;
                        }
                    }
                }),
                c: function() {},
                d: function(b, q) {
                    t(1024);
                    return 0;
                },
                e: f
            };
        }(m);
    }(e);
}

(function(e, m) {
    return {
        then: function(n) {
            n({
                instance: new d(m)
            });
        }
    };
})(c.wasm, {
    a: {
        a: e => {
            var m = console, n = m.log;
            if (e) {
                for (var f = e + void 0, a = e; !(a >= f) && g[a]; ) ++a;
                e = k.decode(g.subarray(e, a));
            } else e = "";
            n.call(m, e);
        }
    }
}).then((e => {
    e = e.instance.exports;
    l = e.d;
    h = e.b;
    g = new Uint8Array(h.buffer);
    g.set(new Uint8Array(c.mem), 1024);
    e.c();
    l();
}));