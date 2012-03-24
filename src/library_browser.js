//"use strict";

// Utilities for browser environments

mergeInto(LibraryManager.library, {
  emscripten_set_main_loop: function(func, fps) {
    fps = fps || 60; // TODO: use requestAnimationFrame
    _emscripten_set_main_loop.cancel = false;
    var jsFunc = FUNCTION_TABLE[func];
    function doOne() {
      if (_emscripten_set_main_loop.cancel) return;
      jsFunc();
      setTimeout(doOne, 1000/fps); // doing this each time means that on exception, we stop
    }
    setTimeout(doOne, 1000/fps);
  },

  emscripten_cancel_main_loop: function(func) {
    _emscripten_set_main_loop.cancel = true;
  },

  $Browser: {
    // Given binary data for an image, in a format like PNG or JPG, we convert it
    // to flat pixel data. We do so using the browser's native code.
    // This is deprecated, it is preferred to load binary files, createObjectURL, etc.,
    // see the sdl_* tests.
    decodeImage: function(pixels, format) {
      function encodeBase64(data) {
        var BASE = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
        var PAD = '=';
        var ret = '';
        var leftchar = 0;
        var leftbits = 0;
        for (var i = 0; i < data.length; i++) {
          leftchar = (leftchar << 8) | data[i];
          leftbits += 8;
          while (leftbits >= 6) {
            var curr = (leftchar >> (leftbits-6)) & 0x3f;
            leftbits -= 6;
            ret += BASE[curr];
          }
        }
        if (leftbits == 2) {
          ret += BASE[(leftchar&3) << 4];
          ret += PAD + PAD;
        } else if (leftbits == 4) {
          ret += BASE[(leftchar&0xf) << 2];
          ret += PAD;
        }
        return ret;
      }
      var image = new Image();
      image.src = 'data:image/' + format + ';base64,' + encodeBase64(pixels);
      assert(image.complete, 'Image could not be decoded'); // page reload might fix it, decoding is async... need .onload handler...
      var canvas = document.createElement('canvas');
      canvas.width = image.width;
      canvas.height = image.height;
      var ctx = canvas.getContext('2d');
      ctx.drawImage(image, 0, 0);
      var imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
      return imageData;
    },
  }
});

