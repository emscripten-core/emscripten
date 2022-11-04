var b = Module, d = new TextDecoder("utf8"), k = new function(a) {
    this.buffer = new ArrayBuffer(65536 * a.initial);
}({
    initial: 256,
    maximum: 256
}), l = k.buffer, g = new Uint8Array(l), m;

function c(a) {
    this.exports = function(h) {
        function n(f) {
            f.set = function(e, p) {
                this[e] = p;
            };
            f.get = function(e) {
                return this[e];
            };
            return f;
        }
        return function(f) {
            var e = f.a.a;
            return {
                b: function() {},
                c: function(p, q) {
                    e(1024);
                    return 0;
                },
                d: n([])
            };
        }(h);
    }(a);
}

(function(a, h) {
    return {
        then: function(n) {
            n({
                instance: new c(h)
            });
        }
    };
})(b.wasm, {
    a: {
        a: function(a) {
            var h = console, n = h.log;
            if (a) {
                for (var f = a + void 0, e = a; !(e >= f) && g[e]; ) ++e;
                a = d.decode(g.subarray(a, e));
            } else a = "";
            n.call(h, a);
        },
        memory: k
    }
}).then((function(a) {
    a = a.instance.exports;
    m = a.c;
    g.set(new Uint8Array(b.mem), 1024);
    a.b();
    m();
}));