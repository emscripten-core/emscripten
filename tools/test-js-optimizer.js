var c;
var b = 5, x = void 0, y = null, t = !0;
var s = !1, s2 = !1, s3 = !0;
var block = [x];
function abc() {
  var cheez = [x];
  var fleefl;
  cheez = 10;
  fleefl = x;
  fleefl = cheez = x;
  var waka = x, flake = x, marfoosh = x;
  var waka2 = 5, flake2 = x, marfoosh2 = x;
  var waka3 = x, flake3 = 5, marfoosh3 = x;
  var waka4 = x, flake4 = x, marfoosh4 = 5;
  var test1 = t, test2 = s, test3 = y, test4 = z, test5 = b, test6 = x;
  s3 = 9;
}
function xyz() {
  var x = 52;
  var cheez = x;
  var t = 22;
  function inner1(c) {
    var i = x, j = y, k = 5, l = t, m = s2;
    var s2 = 8;
  }
  var inner2 = function(d) {
    var i = x, j = y, k = 5, l = t, m = s2;
    var s2 = 8;
  };
  var patama = s3;
}
function xyz2(x) {
  var cheez = x;
}
zzz = function(nada) {
  var cheez = x;
};
function expr() {
  if (!($0 < $1)) print("hi");
}
function loopy() {
  $while_body$2: while(1) { 
    $ok=1;
    $for_cond$4: while(1) { 
      if ($ok) break $for_cond$4;
      var $inc=$ok+1;
      if ($inc == 9999) break $while_body$2; // this forces a label to remain on the outer loop
    }
    continue $while_body$2;
  }
  next();
  b$while_body$2: while(1) { 
    $ok=1;
    b$for_cond$4: while(1) {
      if ($ok) break b$for_cond$4;
      var $inc=$ok+1;
    }
    continue b$while_body$2;
  }
  next();
  $once: do {
    if (!$ok) break $once; // forces the entire one-time do to remain (but unlabelled)
    something();
  } while(0);
  next();
  b$once: do {
    while (more()) {
      if (!$ok) break b$once; // forces the entire one-time do to remain, with label
    }
    something();
  } while(0);
  next();
  c$once: do {
    something();
  } while(0);
}
function ignoreLoopy() {
  b$for_cond$4: while(1) {
    if ($ok) break b$for_cond$4;
    var $inc=$ok+1;
  }
}
function bits() { // TODO: optimize this!
  print((($s & 65535) + ((($f & 65535) << 16 >> 16) * (($f & 65535) << 16 >> 16) | 0 | 0) % 256 | 0) & 65535);
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["abc", "xyz", "xyz2", "expr", "loopy", "bits"]

