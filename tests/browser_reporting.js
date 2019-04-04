function reportResultToServer(result, sync, port) {
  port = port || 8888;
  if (reportResultToServer.reported) {
    // Only report one result per test, even if the test misbehaves and tries to report more.
    reportErrorToServer("excessive reported results from " + ('' + window.location).substr(0, 80) + ", not sending " + result);
    return;
  }
  reportResultToServer.reported = true;

  var xhr = new XMLHttpRequest();
  if (typeof Module === 'object' && Module && Module['pageThrewException']) result = 12345;
  xhr.open('GET', 'http://localhost:' + port + '/report_result?' + result + '|' + ('' + window.location).substr(0, 80), !sync);
  xhr.send();
  if (typeof Module === 'object' && Module && !Module['pageThrewException'] /* for easy debugging, don't close window on failure */) setTimeout(function() { window.close() }, 1000);
}

function reportErrorToServer(message) {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', encodeURI('http://localhost:8888?stderr=' + message));
  xhr.send();
}

if (typeof window === 'object' && window) {
  window.onerror = function(e) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', encodeURI('http://localhost:8888?exception=' + e + ' from ' + ('' + window.location).substr(0, 80)));
    xhr.send();
  };
}
