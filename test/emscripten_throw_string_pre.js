addEventListener('error', (event) => {
  window.disableErrorReporting = true;
  var result = event.error === 'Hello!' ? 0 : 1;
  fetch('http://localhost:8888/report_result?' + result);
});
