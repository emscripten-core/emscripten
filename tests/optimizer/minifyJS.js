var x, z, y;
x++;
x = x - 1;
y++;
// no z, least important
var a, b;
function a(a, b) {
  function a(a, b) {
    x++;
  }
}
var c = function silly() {};
Module; // don't touch me

// uglify parsing of for-in
var key;
for (key in Module) {
}
for (var value in Module) {
  value;
}
c(true);
c(false);
c(undefined);
c(somethingELSE);
c(Math.min(1, 3));
c(d); // d is not defined anywhere here, so it is a global, and we cannot use it
function func(Math, d, x, z, y) {
  Math;
  d;
  x;
  z;
  y;
  (function() {
    var Math, d, x, z, y;
    Math;
    x;
    z;
    y;
  });
}
// more uglify
(function() {
  var new_node;
  if (new_node) {
    for (var i in new_node.contents) {
      throw new FS.ErrnoError(ERRNO_CODES.ENOTEMPTY);
    }
  }
})();
function wasmSetup() {
  function fixImports(xx) { return xx }
  var fixImports2 = function(xx) { return xx }
  Module["asm"] = function(global, env, providedBuffer) {
    env = fixImports(env);
    providedBuffer = fixImports2(providedBuffer);
  };
  var saveName = function saveMyNamePlease(){};
}
var cheez = function(name, value) {
  var local;
  local + name;
};
var name, value;

