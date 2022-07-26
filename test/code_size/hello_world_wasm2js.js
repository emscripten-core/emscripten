var b = Module, c = new TextDecoder("utf8"), f = new function(a) {
    this.buffer = new ArrayBuffer(65536 * a.initial);
}({
    initial: 256,
    maximum: 256
}), h = f.buffer, d = new Uint8Array(h), k = {
    a: function(a) {
        var n = console, e = n.log;
        if (a) {
            for (var g = a + void 0, m = a; !(m >= g) && d[m]; ) ++m;
            a = c.decode(d.subarray(a, m));
        } else a = "";
        e.call(n, a);
    },
    memory: f
}, p = (new function() {
    this.exports = function(a) {
        function n(e) {
            e.set = function(g, m) {
                this[g] = m;
            };
            e.get = function(g) {
                return this[g];
            };
            return e;
        }
        return function(e) {
            var g = e.a;
            return {
                b: function() {},
                c: function(m, q) {
                    g(1024);
                    return 0;
                },
                d: n([])
            };
        }(a);
    }(k);
}).exports, l = p.c;

d.set(new Uint8Array(b.mem), 1024);

p.b();

l();