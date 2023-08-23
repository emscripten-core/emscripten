var b = Module, g = new function(a) {
    this.buffer = new ArrayBuffer(65536 * a.initial);
}({
    initial: 256,
    maximum: 256
}), k = g.buffer, d = new Uint8Array(k), l = new TextDecoder("utf8"), m;

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
        a: a => {
            var h = console, n = h.log;
            if (a) {
                for (var f = a + void 0, e = a; !(e >= f) && d[e]; ) ++e;
                a = l.decode(d.subarray(a, e));
            } else a = "";
            n.call(h, a);
        },
        memory: g
    }
}).then((a => {
    a = a.instance.exports;
    m = a.c;
    d.set(new Uint8Array(b.mem), 1024);
    a.b();
    m();
}));