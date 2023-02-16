var address = 0;

Module.onRuntimeInitialized = function() {
  address = Module['_fail_address']();
  assert(address);
  assert(HEAP8[address] == 0);
}

Module.onExit = function(status) {
  out('onExit status: ' + status);
  // Verify that the join never returned
  assert(address);
  assert(HEAP8[address] == 0, 'fail should never get set!');
  if (typeof reportResultToServer !== 'undefined') {
    reportResultToServer('onExit status: ' + status);
  }
};
