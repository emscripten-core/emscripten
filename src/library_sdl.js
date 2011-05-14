// To use emscripten's SDL library here, you need to define
// Module.canvas and at least one of Module.context2D, Module.contextGL.

mergeInto(Library, {
  _SDL: {
    // Given binary data for an image, in a format like PNG or JPG, we convert it
    // to flat pixel data.
    _decodeImage: function(image) {
      var img = new Image();
      var canvas = document.createElement('canvas');
      img.src = imageToBase64(image);
      var ctx = canvas.getContext('2d');
      ctx.drawImage(Module.img,0,0);
      var imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
      return imageData;
    },

    defaults: {
      width: 320,
      height: 240
    },

    surfaces: {},

    makeSurface: function(width, height, depth, flags) {
      var surf = _malloc(14*QUANTUM_SIZE); // SDL_Surface has 14 fields of quantum size
      __SDL.surfaces[surf] = {
        width: width,
        height: height,
        canvas: Module.canvas,
        context: Module.context2D,
        surf: surf,
        buffer: _malloc(width*height*4)
      };
      return surf;
    },
  },

  SDL_Init__deps: ['_SDL'],
  SDL_Init: function(what) {
    return 0; // success
  },

  SDL_GetVideoInfo: function() {
    // %struct.SDL_VideoInfo = type { i32, i32, %struct.SDL_PixelFormat*, i32, i32 } - 5 fields of quantum size
    var ret = _malloc(5*QUANTUM_SIZE);
    // TODO: Use macros like in library.js
    IHEAP[ret] = 0; // TODO
    IHEAP[ret+QUANTUM_SIZE] = 0; // TODO
    IHEAP[ret+QUANTUM_SIZE*2] = 0; // TODO
    IHEAP[ret+QUANTUM_SIZE*3] = __SDL.defaults.width;
    IHEAP[ret+QUANTUM_SIZE*4] = __SDL.defaults.height;
    return ret;
  },

  SDL_ListModes: function(format, flags) {
    return -1; // -1 == all modes are ok. TODO
  },

  SDL_GL_SetAttribute: function(attr, value) {
    // TODO
  },

  SDL_SetVideoMode: function(width, height, depth, flags) {
    return __SDL.makeSurface(width, height, depth, flags);
  },

  SDL_Quit: function() {
    return 1;
  },

  SDL_LockSurface: function(surf) {
    var surfData = __SDL.surfaces[surf];
    surfData.image = surfData.context.getImageData(0, 0, surfData.width, surfData.height);
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
    var surfData = __SDL.surfaces[surf];
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
    surfData.context.putImageData(surfData.image, 0, 0);
    // Cleanup
    surfData.image = null;
  },

  SDL_Flip: function(surf) {
    // We actually do this in Unlock...
  },

  SDL_Delay: function(delay) {
    // No can do... unless you were a generator...
  },

  SDL_WM_SetCaption: function(title, icon) {
    title = Pointer_stringify(title);
    icon = Pointer_stringify(icon);
  },

  SDL_EnableKeyRepeat: function(delay, interval) {
    // TODO
  },

  SDL_ShowCursor: function(toggle) {
    // TODO
  },

  SDL_GetError: function() {
    return Pointer_make(intArrayFromString("SDL is cool"), null);
  }
});

