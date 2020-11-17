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
    this.exports = function instantiate(t) {
        function r(u) {
            u.set = function(v, w) {
                this[v] = w;
            };
            u.get = function(v) {
                return this[v];
            };
            return u;
        }
        function s(x) {
            var a = Math.imul;
            var b = Math.fround;
            var c = Math.abs;
            var d = Math.clz32;
            var e = Math.min;
            var f = Math.max;
            var g = Math.floor;
            var h = Math.ceil;
            var i = Math.trunc;
            var j = Math.sqrt;
            var k = x.abort;
            var l = NaN;
            var m = Infinity;
            var n = x.a;
            function q(a, b) {
                a = a | 0;
                b = b | 0;
                n(1024);
                return 0;
            }
            function p() {}
            var o = r([]);
            return {
                b: o,
                c: p,
                d: q
            };
        }
        return s(t);
    }(k);
}).exports;

l = n.d;

f.set(new Uint8Array(b.mem), 1024);

n.c();

l();