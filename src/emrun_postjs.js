/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * This file gets implicatly injected as a `--post-js` file when
 * emcc is run with `--emrun`
 */

if (typeof window == "object" && (typeof ENVIRONMENT_IS_PTHREAD == 'undefined' || !ENVIRONMENT_IS_PTHREAD)) {
  var emrun_register_handlers = () => {
    // When C code exit()s, we may still have remaining stdout and stderr
    // messages in flight. In that case, we can't close the browser until all
    // those XHRs have finished, so the following state variables track that all
    // communication is done, after which we can close.
    var emrun_num_post_messages_in_flight = 0;
    var emrun_should_close_itself = false;
    var postExit = (msg) => {
      var http = new XMLHttpRequest();
      // Don't do this immediately, this may race with the notification about
      // the return code reaching the server. Send a *sync* xhr so that we know
      // for sure that the server has gotten the return code before we continue.
      http.open("POST", "stdio.html", false);
      http.send(msg);
      try {
        // Try closing the current browser window, since it exit()ed itself.
        // This can shut down the browser process and then emrun does not need
        // to kill the whole browser process.
        window.close();
      } catch(e) {}
    };
    var post = (msg) => {
      var http = new XMLHttpRequest();
      ++emrun_num_post_messages_in_flight;
      http.onreadystatechange = () => {
        if (http.readyState == 4 /*DONE*/) {
          if (--emrun_num_post_messages_in_flight == 0 && emrun_should_close_itself) {
            postExit('^exit^'+EXITSTATUS);
          }
        }
      }
      http.open("POST", "stdio.html", true);
      http.send(msg);
    };
    // If the address contains localhost, or we are running the page from port
    // 6931, we can assume we're running the test runner and should post stdout
    // logs.
    if (document.URL.search("localhost") != -1 || document.URL.search(":6931/") != -1) {
      var emrun_http_sequence_number = 1;
      var prevPrint = out;
      var prevErr = err;
      addOnExit(() => {
        if (emrun_num_post_messages_in_flight == 0) {
          postExit('^exit^'+EXITSTATUS);
        } else {
          emrun_should_close_itself = true;
        }
      });
      out = (text) => {
        post('^out^'+(emrun_http_sequence_number++)+'^'+encodeURIComponent(text));
        prevPrint(text);
      };
      err = (text) => {
        post('^err^'+(emrun_http_sequence_number++)+'^'+encodeURIComponent(text));
        prevErr(text);
      };

      // Notify emrun web server that this browser has successfully launched the
      // page. Note that we may need to wait for the server to be ready.
      var tryToSendPageload = () => {
        try {
          post('^pageload^');
        } catch (e) {
          setTimeout(tryToSendPageload, 50);
        }
      };
      tryToSendPageload();
    }
  };

  // POSTs the given binary data represented as a (typed) array data back to the
  // emrun-based web server.
  // To use from C code, call e.g:
  //   EM_ASM({emrun_file_dump("file.dat", HEAPU8.subarray($0, $0 + $1));}, my_data_pointer, my_data_pointer_byte_length);
  var emrun_file_dump = (filename, data) => {
    var http = new XMLHttpRequest();
    out(`Dumping out file "${filename}" with ${data.length} bytes of data.`);
    http.open("POST", "stdio.html?file=" + filename, true);
    http.send(data); // XXX  this does not work in workers, for some odd reason (issue #2681)
  };

  if (typeof document != 'undefined') {
    emrun_register_handlers();
  }
}
