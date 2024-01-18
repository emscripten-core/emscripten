addToLibrary({
  ensure_js_throws_with_assertions_enabled__deps: ['$bigintToI53Checked'],
  ensure_js_throws_with_assertions_enabled: function(js_code, error_type) {
    js_code = UTF8ToString(bigintToI53Checked(js_code));
    error_type = UTF8ToString(bigintToI53Checked(error_type));
    js_code += ";";
    try {
      eval(js_code);
    } catch(error_thrown) {
#if ASSERTIONS
      assert(error_thrown.name === error_type);
#else
      assert(false);
#endif
      return;
    }
#if ASSERTIONS
    assert(false);
#endif
  }
});
