if (typeof window === 'object' && window) {
  window.onerror = function(e) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', encodeURI('http://localhost:8888?exception=' + e + ' from ' + ('' + window.location).substr(0, 80)));
    xhr.send();
  };
}
