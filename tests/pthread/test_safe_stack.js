Module['onAbort'] = function (what) {
  if (what == 'stack overflow') {
    reportResultToServer(1);
  }
}
