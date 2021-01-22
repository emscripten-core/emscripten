mergeInto(LibraryManager.library, {
  test_wbind_iijdf__deps: ['$wbind', '$wbindArray'],
  test_wbind_iijdf: function(funcPtr) {
    var ret = wbind(funcPtr)(1, BigInt(0x300000002), 4.2, 5.3);
    console.log(ret);

    ret = wbindArray(funcPtr)([1, BigInt(0x300000002), 4.2, 5.3]);
    console.log(ret);
  },

  test_wbind_ji__deps: ['$wbind', '$wbindArray'],
  test_wbind_ji: function(funcPtr) {
    var ret = wbind(funcPtr)(1);
    console.log(ret);

    ret = wbindArray(funcPtr)([1]);
    console.log(ret);
  }
});
