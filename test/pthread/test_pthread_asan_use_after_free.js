(function () {
  var output = [];
  Module['printErr'] = function (text) {
    if (text == '==42==ABORTING') {
      var result = output.join('\n');
      var passed = [
        'ERROR: AddressSanitizer: heap-use-after-free on address',
        'WRITE of size 4',
        'is located 0 bytes inside of 40-byte region',
        'freed by thread T2 here:',
        'previously allocated by thread T1 here:',
        'Thread T2 created by T1 here:',
        'SUMMARY: AddressSanitizer: heap-use-after-free',
        'Shadow bytes around the buggy address:',
        'Shadow byte legend (one shadow byte represents 8 application bytes):',
      ].every(function (snippet) {
        return result.indexOf(snippet) >= 0;
      });
      reportResultToServer(passed ? 1 : 0);
      return;
    }
    output.push(text);
    console.log(text);
  };
  // We manually report our result from the printErr hook above.
  Module['REPORT_EXIT'] = false;
})();
