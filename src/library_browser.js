//"use strict";

// Utilities for browser environments

mergeInto(LibraryManager.library, {
  $Browser: {
    // Given binary data for an image, in a format like PNG or JPG, we convert it
    // to flat pixel data. We do so using the browser's native code.
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
      var img = new Image();
      var canvas = document.createElement('canvas');
      img.src = 'data:image/' + format + ';base64,' + encodeBase64(pixels);
      var ctx = canvas.getContext('2d');
      ctx.drawImage(img, 0, 0);
      var imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
      return imageData;
    },
  }
});

