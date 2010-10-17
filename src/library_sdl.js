mergeInto(Library, {
  SDL_Init: function(what) {
    SDL_SURFACES = {};
    return 1;
  },

  SDL_SetVideoMode: function(width, height, depth, flags, canvas) {
  //                                                      ^^^^^^ a 'canvas' parameter is added here; supply a canvas from JS there
    var surf = _malloc(14*QUANTUM_SIZE); // SDL_Surface has 14 fields of quantum size
    SDL_SURFACES[surf] = {
      width: width,
      height: height,
      canvas: canvas,
      ctx: canvas.getContext('2d'),
      surf: surf,
      buffer: _malloc(width*height*4)
    };
    return surf;
  },

  SDL_Quit: function() {
    return 1;
  },

  SDL_LockSurface: function(surf) {
    var surfData = SDL_SURFACES[surf];
    surfData.image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
    // Copy pixel data to somewhere accessible to 'C/C++'
    var num = surfData.image.data.length;
    for (var i = 0; i < num; i++) {
      IHEAP[surfData.buffer+i] = surfData.image.data[i];
    }
    // Mark in C/C++-accessible SDL structure
    // SDL_Surface has the following fields: Uint32 flags, SDL_PixelFormat *format; int w, h; Uint16 pitch; void *pixels; ...
    // So we have fields all of the same size, and 5 of them before us.
    IHEAP[surf + 5*QUANTUM_SIZE] = surfData.buffer;
  },

  SDL_UnlockSurface: function(surf) {
    var surfData = SDL_SURFACES[surf];
    // Copy pixel data to image
    var num = surfData.image.data.length;
    for (var i = 0; i < num; i++) {
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
    // No can do... unless you were a generator...
  },
});

