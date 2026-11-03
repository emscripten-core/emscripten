function handle(error) {
  globalThis.disableErrorReporting = true;
  var result = error === 'Hello!' ? 0 : 1;
  fetch('/report_result?' + result);
}
addEventListener('error', (event) => {
  handle(event.error);
});
addEventListener('unhandledrejection', (event) => {
  handle(event.reason);
});
