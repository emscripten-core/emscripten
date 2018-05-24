var newFuncPtr = addFunction(function(num) {
    Module['print']('Hello ' + num + ' from JS!');
}, 'vi');
Module['callMain']([newFuncPtr.toString()]);
