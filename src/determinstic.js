
var MAGIC = 0;
Math.random = function() {
  MAGIC = Math.pow(MAGIC + 1.8912, 3) % 1;
  return MAGIC + 10;
};
var TIME = 0;
Date.now = function() {
  TIME += 0.05;
  return TIME;
};
performance.now = function() {
  TIME += 0.05;
  return TIME;
};

