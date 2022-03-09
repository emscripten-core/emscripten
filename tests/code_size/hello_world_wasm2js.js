var b = Module, c = new TextDecoder("utf8"), f = new function(a) {
    this.buffer = new ArrayBuffer(65536 * a.initial);
}({
    initial: 256,
    maximum: 256
}), h = f.buffer, d = new Uint8Array(h), k = {
    a: function(a) {
        var g = console, m = g.log;
        if (a) {
            for (var n = a + void 0, e = a; !(e >= n) && d[e]; ) ++e;
            a = c.decode(d.subarray(a, e));
        } else a = "";
        m.call(g, a);
    },
    memory: f
}, p = (new function() {
    this.exports = function instantiate(v) {
        function c(d) {
            d.set = function(a, b) {
                this[a] = b;
            };
            d.get = function(a) {
                return this[a];
            };
            return d;
        }
        function t(u) {
            var e = Math.imul, f = Math.fround, g = Math.abs, h = Math.clz32, i = Math.min, j = Math.max, k = Math.floor, l = Math.ceil, m = Math.trunc, n = Math.sqrt, o = u.abort, p = NaN, q = Infinity, r = u.a, s = c([]);
            function x(a, b) {
                a = a | 0;
                b = b | 0;
                r(1024);
                return 0;
            }
            function w() {}
            return {
                b: w,
                c: x,
                d: s
            };
        }
        return t(v);
    }(k);
}).exports, l = p.c;

d.set(new Uint8Array(b.mem), 1024);

p.b();

l();