addEventListener('error', (event) => {
  globalThis.disableErrorReporting = true;
  var result = event.error === 'Hello!' ? 0 : 1;
  fetch('/report_result?' + result);
});
