function emrun_register_handlers() {
  function post(msg) {
    var http = new XMLHttpRequest();
    http.open("POST", "stdio.html", true);
    http.send(msg);
  }
  // If the address contains localhost, or we are running the page from port 6931, we can assume we're running the test runner and should post stdout logs.
  if (document.URL.search("localhost") != -1 || document.URL.search(":6931/") != -1) {
    var emrun_http_sequence_number = 1;
    var prevPrint = Module['print'];
    var prevErr = Module['printErr'];
    function emrun_exit() { post('^exit^'+EXITSTATUS); };
    Module['addOnExit'](emrun_exit);
    Module['print'] = function emrun_print(text) { post('^out^'+(emrun_http_sequence_number++)+'^'+text); prevPrint(text); }
    Module['printErr'] = function emrun_printErr(text) { post('^err^'+(emrun_http_sequence_number++)+'^'+text); prevErr(text); }
  }
  // Notify emrun web server that this browser has successfully launched the page.
  post('^pageload^');
}
window.addEventListener('load', emrun_register_handlers);

// POSTs the given binary data represented as a (typed) array data back to the emrun-based web server.
// To use from C code, call e.g. EM_ASM_({emrun_file_dump("file.dat", HEAPU8.subarray($0, $0 + $1));}, my_data_pointer, my_data_pointer_byte_length);
function emrun_file_dump(filename, data) {
  var http = new XMLHttpRequest();
  Module['print']('Dumping out file "' + filename + '" with ' + data.length + ' bytes of data.');
  http.open("POST", "stdio.html?file=" + filename, true);
  http.send(data); // XXX  this does not work in workers, for some odd reason (issue #2681)
}
