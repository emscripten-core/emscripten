let d = 40;

let z = 50;

globalThis.f = function([, r]) {
  let {a, b} = r;
  let {z: c} = r;
  let [, i, {foo: p, bar: q}] = r;
  return g(a, b, c, d, z);
};

let d2 = 40;

globalThis.f2 = function(r2) {
  let [a2, b2, c2] = r2;
  return g2(a2, b2, c2, d2);
};
