mergeInto(LibraryManager.library, {
  test_multiline_string__deps: ['$stringToNewUTF8'],
  test_multiline_string__sig: 'p',
  test_multiline_string: function() {
    // do not get confused by ` inside a comment.
    var a = `abc
def
ghi`;
    /* or a ` inside a C comment. */
    var b = `abc
def
ghi`;
    return stringToNewUTF8(a + b);
  }
});
