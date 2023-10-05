let d = 40;

let z = 50;

globalThis.f = function(r) {
 let {a: a, b: b} = r;
 let {z: c} = r;
 return g(a, b, c, d, z);
};
