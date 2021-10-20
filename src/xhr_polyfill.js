/**
 * @license
 * Copyright 2012 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// In d8 and other shells xhr might not exist. Polyfill it so that preload-file
// works.
// This is hackish and affects the global scope, but this file is only included
// when using the shell environment, which is not on by default, and only takes
// effect when XMLHttpRequest is not defined, which basically means when testing
// in the shell locally.
if (typeof XMLHttpRequest === 'undefined') {
  // Polyfill XHR for d8, so that --preload-file
  XMLHttpRequest = function() {
    return {
      open: function(mode, path, async) {
        this.mode = mode;
        this.path = path;
        this.async = async;
      },
      send: function() {
        if (!this.async) {
          this.doSend();
        } else {
          var that = this;
          setTimeout(function() {
            that.doSend();
            if (that.onload) that.onload();
          }, 0);
        }
      },
      doSend: function() {
        if (this.responseType == 'arraybuffer') {
          this.response = read(this.path, 'binary');
        } else {
          this.responseText = read(this.path);
        }
        this.status = 200;
      },
    };
  };
}

