
if (runDependencies == 0) {
  var ret = run();
  if (typeof ret != 'undefined') Module.print('Exit Status: ' + ret);
}

