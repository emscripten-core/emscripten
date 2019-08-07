(function () {
  var output = [];
  Module['printErr'] = function (text) {
    if (text == 'LSAN TEST COMPLETE') {
      var result = output.join('\n');
      var passed = [
        'Direct leak of 3432 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:17:17',
        'Direct leak of 2048 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:39:10',
        'Direct leak of 1337 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:33:16',
        'Direct leak of 1234 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:19:13',
        'Direct leak of 420 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:34:13',
        'Direct leak of 42 byte(s) in 1 object(s) allocated from',
        'test_pthread_lsan_leak.cpp:12:21',
        'test_pthread_lsan_leak.cpp:38:3',
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
})();
