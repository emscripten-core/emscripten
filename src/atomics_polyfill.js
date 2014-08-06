// Polyfill for some derived operations that are not implemented primitively yet
// This is a temporary file until the implementation matures. TODO: Remove this once not needed.

Atomics.add =
    function (ita, idx, v) {
  var n = +v;
  do {
      var v0 = ita[idx];
  } while (Atomics.compareExchange(ita, idx, v0, v0+n) != v0);
  return v0;
    };

Atomics.sub =
    function (ita, idx, v) {
  var n = +v;
  do {
      var v0 = ita[idx];
  } while (Atomics.compareExchange(ita, idx, v0, v0-n) != v0);
  return v0;
    };

Atomics.or =
    function (ita, idx, v) {
  var n = v|0;
  do {
      var v0 = ita[idx];
  } while (Atomics.compareExchange(ita, idx, v0, v0|n) != v0);
  return v0;
    };

Atomics.xor =
    function (ita, idx, v) {
  var n = v|0;
  do {
      var v0 = ita[idx];
  } while (Atomics.compareExchange(ita, idx, v0, v0^n) != v0);
  return v0;
    };

Atomics.and =
    function (ita, idx, v) {
  var n = v|0;
  do {
      var v0 = ita[idx];
  } while (Atomics.compareExchange(ita, idx, v0, v0&n) != v0);
  return v0;
    };
