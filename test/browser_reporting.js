var hasModule = typeof Module === 'object' && Module;

/** @param {boolean=} sync
    @param {number=} port */
function reportResultToServer(result, sync, port) {
  port = port || 8888;
  if (reportResultToServer.reported) {
    // Only report one result per test, even if the test misbehaves and tries to report more.
    reportErrorToServer("excessive reported results, sending " + result + ", test will fail");
  }
  reportResultToServer.reported = true;
  if ((typeof ENVIRONMENT_IS_NODE !== 'undefined' && ENVIRONMENT_IS_NODE) || (typeof ENVIRONMENT_IS_AUDIO_WORKLET !== 'undefined' && ENVIRONMENT_IS_AUDIO_WORKLET)) {
    out('RESULT: ' + result);
  } else {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://localhost:' + port + '/report_result?' + result, !sync);
    xhr.send();
    if (typeof window === 'object' && window && hasModule && !Module['pageThrewException']) {
      /* for easy debugging, don't close window on failure */
      setTimeout(function() { window.close() }, 1000);
    }
  }
}

/** @param {boolean=} sync
    @param {number=} port */
function maybeReportResultToServer(result, sync, port) {
  if (reportResultToServer.reported) return;
  reportResultToServer(result, sync, port);
}

function reportErrorToServer(message) {
  var xhr = new XMLHttpRequest();
  if (typeof ENVIRONMENT_IS_NODE !== 'undefined' && ENVIRONMENT_IS_NODE) {
    err(message);
  } else {
    xhr.open('GET', encodeURI('http://localhost:8888?stderr=' + message));
    xhr.send();
  }
}

function report_error(e) {
  // MINIMAL_RUNTIME doesn't handle exit or call the below onExit handler
  // so we detect the exit by parsing the uncaught exception message.
  var message = e.message || e;
  console.error("got top level error: " + message);
  if (window.disableErrorReporting) return;
  if (message.includes('unwind')) return;
  var offset = message.indexOf('exit(');
  if (offset != -1) {
    var status = message.substring(offset + 5);
    offset = status.indexOf(')')
    status = status.substr(0, offset)
    console.error(status);
    var result = 'exit:' + status;
  } else {
    if (hasModule) Module['pageThrewException'] = true;
    result = 'exception:' + message + ' / ' + e.stack;
  }
  // FIXME: Ideally we would just reportResultToServer rather than the `maybe`
  // form but some browser tests currently report exceptions after exit.
  maybeReportResultToServer(result);
}

if (typeof window === 'object' && window) {
  window.addEventListener('error', event => report_error(event.error));
  window.addEventListener('unhandledrejection', event => report_error(event.reason));
}

if (hasModule) {
  if (!Module['onExit']) {
    Module['onExit'] = function(status) {
      maybeReportResultToServer('exit:' + status);
    }
  }

  if (!Module['onAbort']) {
    Module['onAbort'] = function(reason) {
      maybeReportResultToServer('abort:' + reason);
    }
  }
}
