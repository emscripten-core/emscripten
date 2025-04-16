addToLibrary({
  uploadFlipped: function(img) {
    GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, true);
    GLctx.texImage2D(0xDE1/*GLctx.TEXTURE_2D*/, 0, 0x1908/*GLctx.RGBA*/, 0x1908/*GLctx.RGBA*/, 0x1401/*GLctx.UNSIGNED_BYTE*/, img);
    GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, false);
  },
  upload_unicode_char_to_texture__deps: ['uploadFlipped'],
  upload_unicode_char_to_texture: function(unicodeChar, charSize, applyShadow) {
    var canvas = document.createElement('canvas');
    canvas.width = canvas.height = charSize;
//  document.body.appendChild(canvas); // Enable for debugging
    var ctx = canvas.getContext('2d');
    ctx.fillStyle = 'black';
    ctx['globalCompositeOperator'] = 'copy';
    ctx.globalAlpha = 0;
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    ctx.globalAlpha = 1;
    ctx.fillStyle = 'white';
    ctx.font = charSize + 'px Arial Unicode';
    if (applyShadow) {
      ctx.shadowColor = 'black';
      ctx.shadowOffsetX = 2;
      ctx.shadowOffsetY = 2;
      ctx.shadowBlur = 3;
      ctx.strokeStyle = 'gray';
      ctx.strokeText(String.fromCharCode(unicodeChar), 0, canvas.height-7);
    }
    ctx.fillText(String.fromCharCode(unicodeChar), 0, canvas.height-7);
    _uploadFlipped(canvas);
  },
  load_texture_from_url__deps: ['uploadFlipped'],
  load_texture_from_url: function(glTexture, url, outW, outH) {
    var img = new Image();
    img.onload = () => {
      HEAPU32[outW>>2] = img.width;
      HEAPU32[outH>>2] = img.height;
      GLctx.bindTexture(0xDE1/*GLctx.TEXTURE_2D*/, GL.textures[glTexture]);
      _uploadFlipped(img);
    };
    img.src = UTF8ToString(url);
  }
});
