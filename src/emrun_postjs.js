function emrun_register_handlers() {
  function post(msg) {
    var http = new XMLHttpRequest();
    http.open("POST", "stdio.html", true);
    http.send(msg);
  }
  // If the address contains localhost, or we are running the page from port 6931, we can assume we're running the test runner and should post stdout logs.
  if (document.URL.search("localhost") != -1 || document.URL.search(":6931/") != -1) {
    var emrun_http_sequence_number = 1;
    var prevExit = Module['exit'];
    var prevPrint = Module['print'];
    var prevErr = Module['printErr'];
    Module['exit'] = function emrun_exit(returncode) { post('^exit^'+returncode); prevExit(returncode); }
    Module['print'] = function emrun_print(text) { post('^out^'+(emrun_http_sequence_number++)+'^'+text); prevPrint(text); }
    Module['printErr'] = function emrun_printErr(text) { post('^err^'+(emrun_http_sequence_number++)+'^'+text); prevErr(text); }
  }
  // Notify emrun web server that this browser has successfully launched the page.
  post('^pageload^');
}
emrun_register_handlers();
