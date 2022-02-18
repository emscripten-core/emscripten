Module['onCustomMessage'] = function(message) {
  function replyToWorker(msg, cmd) {
    let w = Module.PThread.pthreads[msg.worker].worker;
    w.postMessage({
        cmd: 'custom',
        customCmd: cmd,
        payload: msg.payload + '[' + cmd + ']'
    });
  }

  function sendTestResult(success) {
    if (window.sentTestResult) return;
    window.sentTestResult = true;
    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://localhost:8888/report_result?' + (success ? 1 : 0));
    xhr.send();
    setTimeout(function() { window.close() }, 1000);
  }

  switch (message.customCmd) {
    case 'first_msg': {
      replyToWorker(message, 'reply_first');
      break;
    }
    case 'second_msg': {
      replyToWorker(message, 'reply_second');
      break;
    }
    case 'last_msg': {
      sendTestResult(message.payload === '[first][reply_first][second][reply_second][last]');
      break;
    }
    default: {
      sendTestResult(false);
    }
  }
};
