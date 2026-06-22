function handleThrow(value) {
  globalThis.disableErrorReporting = true;
  var result = value === 42 ? 0 : 1;
  fetch('/report_result?' + result);
}

addEventListener('error', (event) => {
  handleThrow(event.error);
});

addEventListener('unhandledrejection', (event) => {
  handleThrow(event.reason);
});
