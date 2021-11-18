var b = Module;

var c = new TextDecoder("utf8"), d, f;

f = new function(a) {
    this.buffer = new ArrayBuffer(65536 * a.initial);
}({
    initial: 256,
    maximum: 256
});

var h = f.buffer;

d = new Uint8Array(h);

var k = {
    a: function(a) {
        var g = console, m = g.log;
        if (a) {
            for (var n = a + void 0, e = a; !(e >= n) && d[e]; ) ++e;
            a = c.decode(d.subarray(a, e));
        } else a = "";
        m.call(g, a);
    },
    memory: f
}, l, p = (new function() {
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
            var e = Math.imul;
            var f = Math.fround;
            var g = Math.abs;
            var h = Math.clz32;
            var i = Math.min;
            var j = Math.max;
            var k = Math.floor;
            var l = Math.ceil;
            var m = Math.trunc;
            var n = Math.sqrt;
            var o = u.abort;
            var p = NaN;
            var q = Infinity;
            var r = u.a;
            function x(a, b) {
                a = a | 0;
                b = b | 0;
                r(1024);
                return 0;
            }
            function w() {}
            var s = c([]);
            return {
                b: w,
                c: x,
                d: s
            };
        }
        return t(v);
    }(k);
}).exports;

l = p.c;

d.set(new Uint8Array(b.mem), 1024);

p.b();

l();