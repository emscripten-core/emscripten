(function () {
  var output = [];
  Module['printErr'] = function (text) {
    if (text == 'LSAN TEST COMPLETE') {
      var result = output.join('\n');
      var passed = [
        'Direct leak of 3432 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:18:17',
        'Direct leak of 2048 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:36:10',
        'Direct leak of 1337 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:30:16',
        'Direct leak of 1234 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:20:13',
        'Direct leak of 420 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:31:13',
        'Direct leak of 42 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:13:21',
        'test_pthread_lsan_leak.cpp:35:3',
        '8513 byte(s) leaked in 6 allocation(s).',
        ''
      ].every(function (snippet) {
        return result.indexOf(snippet) >= 0;
      });
      reportResultToServer(passed ? 1 : 0);
      return;
    }
    output.push(text);
  };
  Module['LSAN_OPTIONS'] = 'exitcode=0';
  // We manually report our result from the printErr hook above.
  Module['REPORT_EXIT'] = false;
})();
