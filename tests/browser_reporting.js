var hasModule = typeof Module === 'object' && Module;
var hasWindow = typeof window === 'object' && window;
var keepWindowAlive = false;

/** @param {boolean=} sync
    @param {number=} port */
function reportResultToServer(result, sync, port) {
  port = port || 8888;
  if (reportResultToServer.reported) {
    // Only report one result per test, even if the test misbehaves and tries to report more.
    reportStderrToServer("excessive reported results, sending " + result + ", test will fail");
  }
  reportResultToServer.reported = true;
  var xhr = new XMLHttpRequest();
  xhr.open('GET', 'http://localhost:' + port + '/report_result?' + result, !sync);
  xhr.send();
  if (hasWindow && hasModule && !keepWindowAlive) {
    setTimeout(function() { window.close() }, 1000);
  }
}

/** @param {boolean=} sync
    @param {number=} port */
function maybeReportResultToServer(result, sync, port) {
  if (reportResultToServer.reported) return;
  reportResultToServer(result, sync, port);
}

function reportStderrToServer(message) {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', encodeURI('http://localhost:8888?stderr=' + message));
  xhr.send();
}

function reportExceptionToServer(e) {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', encodeURI('http://localhost:8888?exception=' + e.message + ' / ' + e.stack));
  xhr.send();
}

if (typeof window === 'object' && window) {
  function report_error(e) {
    var message = e.message || e;
    if (e.error) {
      message = e.error.message;
    }
    // MINIMAL_RUNTIME lets unwind exceptions remain uncaught, and these
    // should not be considered actually errors.
    if (message == 'unwind') {
      return;
    }
    console.error("emtest: got top level error: " + message);
    // We report aborts via the onAbort handler so can ignore them here
    if (message.indexOf(' abort(') != -1)
      return;
    // MINIMAL_RUNTIME doesn't handle exit or call the below onExit handler
    // so we detect the exit by parsing the uncaught exception message.
    var offset = message.indexOf('exit(');
    if (offset != -1) {
      var status = message.substring(offset + 5);
      offset = status.indexOf(')')
      status = status.substr(0, offset)
      console.error(status);
      maybeReportResultToServer('exit:' + status);
    } else {
      reportExceptionToServer(e);
      /*
       * Also report the exception as the result of the test if non has been
       * reported yet
       * For easy debugging, don't close window on failure.
       */
      keepWindowAlive = true;
      maybeReportResultToServer('exception:' + e.message);
    }
  }
  window.addEventListener('error', report_error);
  window.addEventListener('unhandledrejection', event => report_error(event.reason));
}

if (hasModule) {
  Module['onExit'] = function(status) {
    maybeReportResultToServer('exit:' + status);
  }

  Module['onAbort'] = function(reason) {
    maybeReportResultToServer('abort:' + reason);
  }
}
