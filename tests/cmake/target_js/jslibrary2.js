var mylib = {};

mylib.lib_function2 = function() {
  console.log('lib_function2');
}

mergeInto(LibraryManager.library, mylib);
