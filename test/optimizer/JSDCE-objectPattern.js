// Of all these, only d must remain: the others are defined in the function, and
// used from that scope, not the global scope.
let a = 10;
let b = 20;
let c = 30;
let d = 40;

// This one must also remain: z in the function is not a local variable, but the
// id in an object, so it does not affect scoping at all, and we use the global
// z.
let z = 50;

globalThis.f = function([/*empty*/, r]) {
  let { a, b } = r;
  let { z: c } = r;
  let [/*empty*/, i, {foo : p, bar : q}] = r;
  return g(a, b, c, d, z);
};

// As above, but now with array destructuring.
let a2 = 10;
let b2 = 20;
let c2 = 30;
let d2 = 40;

globalThis.f2 = function(r2) {
  let [ a2, b2, c2 ] = r2;
  return g2(a2, b2, c2, d2);
};
