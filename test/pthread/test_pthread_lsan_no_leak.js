(function () {
  var output = [];
  Module['printErr'] = function (text) {
    if (text == 'LSAN TEST COMPLETE') {
      reportResultToServer(output.length == 0 ? 1 : 0);
      return;
    }
    output.push(text);
  };
  Module['LSAN_OPTIONS'] = 'exitcode=0';
  // We manually report our result from the printErr hook above.
  Module['REPORT_EXIT'] = false;
})();
