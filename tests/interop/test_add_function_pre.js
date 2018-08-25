Module['postRun'] = function() {
  console.log("postRun");
  var newFuncPtr = addFunction(function(num) {
      out('Hello ' + num + ' from JS!');
  }, 'vi');

  Module['callMain']([newFuncPtr.toString()]);
};
