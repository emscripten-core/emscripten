// Set this to true to have stdout and stderr sent back to the server
var captureStdio = false;

var hasModule = typeof Module === 'object' && Module;

var reportingURL = 'http://localhost:8888';

async function reportResultToServer(result) {
  if (reportResultToServer.reported) {
    // Only report one result per test, even if the test misbehaves and tries to report more.
    reportStderrToServer(`excessive reported results, sending ${result}, test will fail`);
  }
  reportResultToServer.reported = true;
  if ((typeof ENVIRONMENT_IS_NODE !== 'undefined' && ENVIRONMENT_IS_NODE) || (typeof ENVIRONMENT_IS_AUDIO_WORKLET !== 'undefined' && ENVIRONMENT_IS_AUDIO_WORKLET)) {
    out(`RESULT: ${result}`);
  } else {
    await fetch(`${reportingURL}/report_result?${encodeURIComponent(result)}`);
    if (typeof window === 'object' && window && hasModule && !Module['pageThrewException']) {
      /* for easy debugging, don't close window on failure */
      window.close();
    }
  }
}

function sendFileToServer(filename, contents) {
  fetch(`${reportingURL}/upload?file=${encodeURIComponent(filename)}`, { method: "POST", body: contents });
}

function logMessageToServer(filename, message) {
  fetch(`${reportingURL}/log?file=${filename}`, { method: "POST", body: message })
}

function maybeReportResultToServer(result) {
  if (!reportResultToServer.reported) {
    reportResultToServer(result);
  }
}

function reportStderrToServer(message) {
  if (typeof ENVIRONMENT_IS_NODE !== 'undefined' && ENVIRONMENT_IS_NODE) {
    err(message);
  } else {
    logMessageToServer('stderr', message);
  }
}

function reportStdoutToServer(message) {
  if (typeof ENVIRONMENT_IS_NODE !== 'undefined' && ENVIRONMENT_IS_NODE) {
    out(message);
  } else {
    logMessageToServer('stdout', message);
  }
}

async function skipTest(message) {
  await reportResultToServer(`skipped:${message}`);
}

function reportTopLevelError(e) {
  // MINIMAL_RUNTIME doesn't handle exit or call the below onExit handler
  // so we detect the exit by parsing the uncaught exception message.
  var message = e.message || e;
  console.error(`got top level error: ${message}`);
  if (window.disableErrorReporting) return;
  if (message.includes('unwind')) return;
  var offset = message.indexOf('exit(');
  if (offset != -1) {
    var status = message.substring(offset + 5);
    offset = status.indexOf(')')
    status = status.substr(0, offset)
    console.error(status);
    var result = `exit:${status}`;
  } else {
    if (hasModule) Module['pageThrewException'] = true;
    result = `exception:${message} / ${e.stack}`;
  }
  // FIXME: Ideally we would just reportResultToServer rather than the `maybe`
  // form but some browser tests currently report exceptions after exit.
  maybeReportResultToServer(result);
}

if (typeof window === 'object' && window) {
  const urlString = window.location.search;
  const searchParams = new URLSearchParams(urlString);
  if (searchParams.has('capture_stdio')) {
    captureStdio = true;
  }

  window.addEventListener('error', event => {
    reportTopLevelError(event.error || event)
  });
  window.addEventListener('unhandledrejection', event => reportTopLevelError(event.reason));
}

if (hasModule) {
  if (!Module['onExit']) {
    Module['onExit'] = function(status) {
      // If Module['REPORT_EXIT'] is set to false, do not report the result of
      // onExit.
      if (Module['REPORT_EXIT'] !== false) {
        maybeReportResultToServer(`exit:${status}`);
      }
    }
    // Force these handlers to be proxied back to the main thread.
    // Without this tagging the handler will run each thread, which means
    // each thread uses its own copy of `maybeReportResultToServer` which
    // breaks the checking for duplicate reporting.
    Module['onExit'].proxy = true;
  }

  if (!Module['onAbort']) {
    Module['onAbort'] = function(reason) {
      maybeReportResultToServer(`abort:${reason}`);
    }
    Module['onAbort'].proxy = true;
  }

  if (captureStdio) {
    console.log("enabling remote stdio logging");
    const origPrint = Module['print'];
    const origPrintErr = Module['printErr'];

    Module['print'] = (...args) => {
      origPrint?.(...args);
      reportStdoutToServer(args.join(' '));
    };

    Module['printErr'] = (...args) => {
      origPrintErr?.(...args);
      reportStderrToServer(args.join(' '));
    };
  }
}
