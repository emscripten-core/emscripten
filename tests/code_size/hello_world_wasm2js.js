var b = Module;

var c = new TextDecoder("utf8"), d, f;

f = new function(a) {
    this.buffer = new ArrayBuffer(65536 * a.initial);
}({
    initial: 256,
    maximum: 256
});

var g = f.buffer;

d = new Uint8Array(g);

var h = {
    a: function(a) {
        if (a) {
            for (var l = a + void 0, e = a; !(e >= l) && d[e]; ) ++e;
            a = c.decode(d.subarray(a, e));
        } else a = "";
        console.log(a);
    },
    memory: f
}, k, m = (new function() {
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
    }(h);
}).exports;

k = m.c;

d.set(new Uint8Array(b.mem), 1024);

m.b();

k();