if (typeof window === 'object' && window) {
  window.onerror = function(e) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', encodeURI('http://localhost:8888?exception=' + e));
    xhr.send();
  };
}
