// Will contain source of visualizations.
var memProf = (function () {
  return {
    mallocProxy: function () { Module.print('mallocProxy'); },
    freeProxy: function () { Module.print('freeProxy'); },
  };
})();

