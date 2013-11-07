
var MAGIC = 0;
Math.random = function() {
  MAGIC = Math.pow(MAGIC + 1.8912, 3) % 1;
  return MAGIC;
};
var TIME = 10000;
Date.now = function() {
  return TIME++;
};
performance.now = Date.now;

function hashMemory(id) {
  var ret = 0;
  for (var i = 0; i < HEAPU8.length; i++) {
    ret = (ret*17 + HEAPU8[i])|0;
  }
  print(id + ':' + ret);
}

