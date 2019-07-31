(function () {
  var output = [];
  Module['printErr'] = function (text) {
    if (text == 'LSAN TEST COMPLETE') {
      var result = output.join('\n');
      var passed = [
        'Direct leak of 2048 byte(s) in 1 object(s) allocated from',
        'Direct leak of 3432 byte(s) in 1 object(s) allocated from',
        'Direct leak of 1234 byte(s) in 1 object(s) allocated from',
        'Direct leak of 1337 byte(s) in 1 object(s) allocated from',
        'Direct leak of 42 byte(s) in 1 object(s) allocated from',
        'Direct leak of 420 byte(s) in 1 object(s) allocated from',
      ].every(function (snippet) {
        return result.indexOf(snippet) >= 0;
      });
      reportResultToServer(passed ? 1 : 0);
      return;
    }
    output.push(text);
  };
  Module['LSAN_OPTIONS'] = 'exitcode=0';
})();
