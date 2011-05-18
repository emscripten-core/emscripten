// To use emscripten's SDL library here, you need to define
// Module.canvas and at least one of Module.ctx2D, Module.ctxGL.

mergeInto(Library, {
  $SDL__deps: ['$Browser'],
  $SDL: {
    defaults: {
      width: 320,
      height: 240
    },

    surfaces: {},

    structs: {
      PixelFormat: Runtime.generateStructInfo([
        ['void*', 'palette'], ['i8', 'BitsPerPixel'], ['i8', 'BytesPerPixel'],
        ['i8', 'Rloss'], ['i8', 'Gloss'], ['i8', 'Bloss'], ['i8', 'Aloss'],
        ['i8', 'Rshift'], ['i8', 'Gshift'], ['i8', 'Bshift'], ['i8', 'Ashift'],
        ['i32', 'Rmask'], ['i32', 'Gmask'], ['i32', 'Bmask'], ['i32', 'Amask'] // Docs say i8, ./include/SDL_video.h says i32...
      ])
    },

    makeSurface: function(width, height, flags) {
      var surf = _malloc(14*QUANTUM_SIZE);  // SDL_Surface has 14 fields of quantum size
      var buffer = _malloc(width*height*4);
      var pixelFormat = _malloc(18*QUANTUM_SIZE);

      {{{ makeSetValue('surf+QUANTUM_SIZE*0', '0', 'flags', 'i32') }}}         // SDL_Surface.flags
      {{{ makeSetValue('surf+QUANTUM_SIZE*1', '0', 'pixelFormat', 'void*') }}} // SDL_Surface.format TODO
      {{{ makeSetValue('surf+QUANTUM_SIZE*2', '0', 'width', 'i32') }}}         // SDL_Surface.w
      {{{ makeSetValue('surf+QUANTUM_SIZE*3', '0', 'height', 'i32') }}}        // SDL_Surface.h
      {{{ makeSetValue('surf+QUANTUM_SIZE*4', '0', 'width*4', 'i16') }}}       // SDL_Surface.pitch, assuming RGBA for now,
                                                                               // since that is what ImageData gives us in browsers
      {{{ makeSetValue('surf+QUANTUM_SIZE*5', '0', 'buffer', 'void*') }}}      // SDL_Surface.pixels

      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.palette', '0', '0', 'i32') }}} // TODO
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.BitsPerPixel', '0', '32', 'i8') }}} // TODO
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.BytesPerPixel', '0', '4', 'i8') }}} // TODO

      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.Rmask', '0', '0xff', 'i32') }}}
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.Gmask', '0', '0xff', 'i32') }}}
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.Bmask', '0', '0xff', 'i32') }}}
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.Amask', '0', '0xff', 'i32') }}}

      SDL.surfaces[surf] = {
        width: width,
        height: height,
        canvas: Module.canvas,
        ctx: Module.ctx2D,
        surf: surf,
        buffer: buffer,
        pixelFormat: pixelFormat,
        alpha: 255
      };
      return surf;
    },

    freeSurface: function(surf) {
      _free(SDL.surfaces[surf].buffer);
      _free(SDL.surfaces[surf].pixelFormat);
      _free(surf);
      delete SDL.surfaces[surf];
    }
  },

  SDL_Init__deps: ['$SDL'],
  SDL_Init: function(what) {
    return 0; // success
  },

  SDL_WasInit: function() { return 0 }, // TODO

  SDL_GetVideoInfo: function() {
    // %struct.SDL_VideoInfo = type { i32, i32, %struct.SDL_PixelFormat*, i32, i32 } - 5 fields of quantum size
    var ret = _malloc(5*QUANTUM_SIZE);
    {{{ makeSetValue('ret+QUANTUM_SIZE*0', '0', '0', 'i32') }}} // TODO
    {{{ makeSetValue('ret+QUANTUM_SIZE*1', '0', '0', 'i32') }}} // TODO
    {{{ makeSetValue('ret+QUANTUM_SIZE*2', '0', '0', 'void*') }}}
    {{{ makeSetValue('ret+QUANTUM_SIZE*3', '0', 'SDL.defaults.width', 'i32') }}}
    {{{ makeSetValue('ret+QUANTUM_SIZE*4', '0', 'SDL.defaults.height', 'i32') }}}
    return ret;
  },

  SDL_ListModes: function(format, flags) {
    return -1; // -1 == all modes are ok. TODO
  },

  SDL_GL_SetAttribute: function(attr, value) {
    // TODO
  },

  SDL_SetVideoMode: function(width, height, depth, flags) {
    return SDL.makeSurface(width, height, flags);
  },

  SDL_Quit: function() {
    return 1;
  },

  SDL_LockSurface: function(surf) {
    var surfData = SDL.surfaces[surf];
    surfData.image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
    // Copy pixel data to somewhere accessible to 'C/C++'
    var num = surfData.image.data.length;
    for (var i = 0; i < num; i++) {
      // TODO: Use macros like in library.js
      IHEAP[surfData.buffer+i] = surfData.image.data[i];
    }
    // Mark in C/C++-accessible SDL structure
    // SDL_Surface has the following fields: Uint32 flags, SDL_PixelFormat *format; int w, h; Uint16 pitch; void *pixels; ...
    // So we have fields all of the same size, and 5 of them before us.
    // TODO: Use macros like in library.js
    IHEAP[surf + 5*QUANTUM_SIZE] = surfData.buffer;
  },

  SDL_UnlockSurface: function(surf) {
    var surfData = SDL.surfaces[surf];
    // Copy pixel data to image
    var num = surfData.image.data.length;
    for (var i = 0; i < num; i++) {
      // TODO: Use macros like in library.js
      surfData.image.data[i] = IHEAP[surfData.buffer+i];
    }
    for (var i = 0; i < num/4; i++) {
      surfData.image.data[i*4+3] = 255; // opacity, as canvases blend alpha
    }
    // Copy to canvas
    surfData.ctx.putImageData(surfData.image, 0, 0);
    // Cleanup
    surfData.image = null;
  },

  SDL_Flip: function(surf) {
    // We actually do this in Unlock...
  },

  SDL_Delay: function(delay) {
    throw 'SDL_Delay called - potential infinite loop';
  },

  SDL_WM_SetCaption: function(title, icon) {
    title = title && Pointer_stringify(title);
    icon = icon && Pointer_stringify(icon);
  },

  SDL_EnableKeyRepeat: function(delay, interval) {
    // TODO
  },

  SDL_ShowCursor: function(toggle) {
    // TODO
  },

  SDL_GetError: function() {
    return Pointer_make(intArrayFromString("SDL is cool"), null);
  },

  SDL_CreateRGBSurface: function(flags, width, height, depth, rmask, gmask, bmask, amask) {
    return SDL.makeSurface(width, height, flags);
  },

  SDL_FreeSurface: function(surf) {
    SDL.freeSurface(surf);
  },

  SDL_UpperBlit: function(src, srcrect, dst, dstrect) {
    assert(!srcrect && !dstrect); // TODO
    var srcData = SDL.surfaces[src];
    var dstData = SDL.surfaces[dst];
    assert(srcData.width === dstData.width && srcData.height === dstData.height);
    for (var i = 0; i < srcData.width*srcData.height*4; i++) {
      {{{ makeCopyValue('dstData.buffer', 'i', 'srcData.buffer', 'i', 'i8') }}}
    }
    return 0;
  },

  SDL_BlitSurface__deps: ['SDL_UpperBlit'],
  SDL_BlitSurface: function(src, srcrect, dst, dstrect) {
    return _SDL_Blit(src, srcrect, dst, dstrect);
  },

  SDL_SetAlpha: function(surf, flag, alpha) {
    SDL.surfaces[surf].alpha = alpha;
  },

  SDL_GL_SwapBuffers: function() {},

  // SDL_Image

  IMG_Load: function(filename) {
    filename = Pointer_stringify(filename);
    var format = filename.split('.').slice(-1)[0];
    var data = readBinary(filename);
    var raw = Browser.decodeImage(data, format);
    var surf = SDL.makeSurface(raw.width, raw.height, 0);
    // XXX Extremely inefficient!
    for (var i = 0; i < raw.width*raw.height*4; i++) {
      {{{ makeSetValue('SDL.surfaces[surf].buffer', 'i', 'raw.data[i]', 'i8') }}}
    }
    return surf;
  }
});

