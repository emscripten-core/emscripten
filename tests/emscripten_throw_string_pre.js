addEventListener('error', function(event) {
  window.disableErrorReporting = true;
  var result = event.error === 'Hello!' ? 0 : 1;
  var xhr = new XMLHttpRequest();
  xhr.open('GET', 'http://localhost:8888/report_result?' + result, true);
  xhr.send();
});
