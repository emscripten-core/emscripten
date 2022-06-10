var b = Module, c = new TextDecoder("utf8"), f = new function(e) {
    this.buffer = new ArrayBuffer(65536 * e.initial);
}({
    initial: 256,
    maximum: 256
}), h = f.buffer, d = new Uint8Array(h), k = {
    b: function(e) {
        var n = console, g = n.log;
        if (e) {
            for (var m = e + void 0, a = e; !(a >= m) && d[a]; ) ++a;
            e = c.decode(d.subarray(e, a));
        } else e = "";
        g.call(n, e);
    },
    a: f
}, p = (new function() {
    this.exports = function(e) {
        function n(g) {
            g.set = function(m, a) {
                this[m] = a;
            };
            g.get = function(m) {
                return this[m];
            };
            return g;
        }
        return function(g) {
            function m() {
                u(1024);
                return 0;
            }
            var a = g.a.buffer;
            new Int8Array(a);
            new Int16Array(a);
            var q = new Int32Array(a);
            new Uint8Array(a);
            new Uint16Array(a);
            new Uint32Array(a);
            new Float32Array(a);
            new Float64Array(a);
            var u = g.b;
            return {
                c: function() {},
                d: function(r, t) {
                    return m() | 0;
                },
                e: n([]),
                f: function(r, t) {
                    q[259] = t | 0;
                    q[258] = r | 0;
                    return m() | 0;
                }
            };
        }(e);
    }(k);
}).exports, l = p.f;

d.set(new Uint8Array(b.mem), 1024);

p.c();

l();