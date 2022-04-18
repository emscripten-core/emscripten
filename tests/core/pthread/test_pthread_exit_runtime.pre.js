var address = 0;

Module.onRuntimeInitialized = function() {
  address = Module['_join_returned_address']();
  assert(address);
  assert(HEAP8[address] == 0);
}

Module.onExit = function(status) {
  out('onExit status: ' + status);
  // Verify that the join never returned
  assert(address);
  assert(HEAP8[address] == 0, 'pthread_join should not have returned!');
  if (typeof reportResultToServer !== 'undefined') {
    reportResultToServer('onExit status: ' + status);
  }
};
