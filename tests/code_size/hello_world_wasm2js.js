var b = Module;

var c = new TextDecoder("utf8");

function e(a) {
    if (!a) return "";
    for (var m = a + NaN, d = a; !(d >= m) && f[d]; ) ++d;
    return c.decode(f.subarray(a, d));
}

var f, g;

g = new function(a) {
    this.buffer = new ArrayBuffer(65536 * a.initial);
}({
    initial: 256,
    maximum: 256
});

var h = g.buffer;

f = new Uint8Array(h);

var k = {
    a: function(a) {
        console.log(e(a));
    },
    memory: g
}, l, n = (new function() {
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

l = n.c;

f.set(new Uint8Array(b.mem), 1024);

n.b();

l();