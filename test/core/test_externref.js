addToLibrary({
  get_externref_js: function () {
    var test_object = { foo: 1 };
    return test_object;
  },

  log_externref_js: function (ref) {
    console.log('log_externref_js: ' +  JSON.stringify(ref));
  },
});
