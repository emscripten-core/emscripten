addEventListener('error', (event) => {
  globalThis.disableErrorReporting = true;
  var result = event.error === 42 ? 0 : 1;
  fetch('/report_result?' + result);
});
