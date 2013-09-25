var mylib = {};

mylib.lib_function = function() {
  console.log('lib_function');
}

mergeInto(LibraryManager.library, mylib);
