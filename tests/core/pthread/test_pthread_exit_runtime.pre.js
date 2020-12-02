Module.preRun = function() {
  Module['onExit'] = function(status) {
    out('onExit status: ' + status);
    if (typeof reportResultToServer !== 'undefined') {
      reportResultToServer('onExit status: ' + status);
    }
  };
}
