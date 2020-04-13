addEventListener('error', function(event) {
  event.preventDefault();
  event.stopPropagation();
  var result = event.error === 42 ? 0 : 1;
  var xhr = new XMLHttpRequest();
  xhr.open('GET', 'http://localhost:8888/report_result?' + result, true);
  xhr.send();
});
