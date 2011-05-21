// To use emscripten's SDL library here, you need to define
// Module.canvas and at least one of Module.ctx2D, Module.ctxGL.

mergeInto(Library, {
  $SDL__deps: ['$Browser'],
  $SDL: {
    defaults: {
      width: 320,
      height: 200,
      copyScreenOnLock: false
    },

    surfaces: {},
    events: [],

    keyCodes: {
      38: 273, // up arrow
      40: 274, // down arrow
      37: 276, // left arrow
      39: 275, // right arrow
      17: 305, // control (right, or left)
    },

    structs: {
      PixelFormat: Runtime.generateStructInfo([
        ['void*', 'palette'], ['i8', 'BitsPerPixel'], ['i8', 'BytesPerPixel'],
        ['i8', 'Rloss'], ['i8', 'Gloss'], ['i8', 'Bloss'], ['i8', 'Aloss'],
        ['i8', 'Rshift'], ['i8', 'Gshift'], ['i8', 'Bshift'], ['i8', 'Ashift'],
        ['i32', 'Rmask'], ['i32', 'Gmask'], ['i32', 'Bmask'], ['i32', 'Amask'] // Docs say i8, ./include/SDL_video.h says i32...
      ]),
      KeyboardEvent: Runtime.generateStructInfo([
        ['i8', 'type'],
        ['i8', 'which'],
        ['i8', 'state'],
        ['i32', 'keysym']
      ]),
      keysym: Runtime.generateStructInfo([
        ['i8', 'scancode'],
        ['i32', 'sym'],
        ['i32', 'mod'],
        ['i16', 'unicode']
      ])
    },

    makeSurface: function(width, height, flags) {
      var surf = _malloc(14*QUANTUM_SIZE);  // SDL_Surface has 14 fields of quantum size
      var buffer = _malloc(width*height*4);
      var pixelFormat = _malloc(18*QUANTUM_SIZE);
      flags |= 1; // SDL_HWSURFACE - this tells SDL_MUSTLOCK that this needs to be locked

      {{{ makeSetValue('surf+QUANTUM_SIZE*0', '0', 'flags', 'i32') }}}         // SDL_Surface.flags
      {{{ makeSetValue('surf+QUANTUM_SIZE*1', '0', 'pixelFormat', 'void*') }}} // SDL_Surface.format TODO
      {{{ makeSetValue('surf+QUANTUM_SIZE*2', '0', 'width', 'i32') }}}         // SDL_Surface.w
      {{{ makeSetValue('surf+QUANTUM_SIZE*3', '0', 'height', 'i32') }}}        // SDL_Surface.h
      {{{ makeSetValue('surf+QUANTUM_SIZE*4', '0', 'width*4', 'i16') }}}       // SDL_Surface.pitch, assuming RGBA for now,
                                                                               // since that is what ImageData gives us in browsers
      {{{ makeSetValue('surf+QUANTUM_SIZE*5', '0', 'buffer', 'void*') }}}      // SDL_Surface.pixels
      {{{ makeSetValue('surf+QUANTUM_SIZE*6', '0', '0', 'i32*') }}}      // SDL_Surface.offset

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
    },

    receiveEvent: function(event) {
      switch(event.type) {
        case 'keydown': case 'keyup':
          //print('zz receive Event: ' + event.keyCode);
          SDL.events.push(event);
      }
    },
    
    makeCEvent: function(event, ptr) {
      switch(event.type) {
        case 'keydown': case 'keyup':
          var down = event.type === 'keydown';
          var key = SDL.keyCodes[event.keyCode] || event.keyCode;
          if (key >= 65 && key <= 90) {
            key = String.fromCharCode(key).toLowerCase().charCodeAt(0);
          }
          //print('zz passing over Event: ' + key);
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.type', 'down ? 2 : 3', 'i8') }}}
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.which', '1', 'i8') }}}
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.state', 'down ? 1 : 0', 'i8') }}}

          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.keysym + SDL.structs.keysym.scancode', 'key', 'i8') }}}
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.keysym + SDL.structs.keysym.sym', 'key', 'i32') }}}
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.keysym + SDL.structs.keysym.mod', '0', 'i32') }}}
          //{{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.keysym + SDL.structs.keysym.unicode', 'key', 'i32') }}}

          break;
        case 'keypress': break // TODO
      default:
        throw 'Unhandled SDL event: ' + event.type;
      }
    }
  },

  SDL_Init__deps: ['$SDL'],
  SDL_Init: function(what) {
    SDL.startTime = Date.now();
    ['keydown', 'keyup', 'keypress'].forEach(function(event) {
      addEventListener(event, SDL.receiveEvent);
    });
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
    return SDL.screen = SDL.makeSurface(width, height, flags);
  },

  SDL_Quit: function() {
    var surfData = SDL.surfaces[SDL.screen];
    surfData.image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
    var num = surfData.image.data.length;
    for (var i = 0; i < num; i++) {
      surfData.image.data[i] = Math.floor(Math.random()*255);
    }
    surfData.ctx.putImageData(surfData.image, 0, 0);
    throw 'SDL_Quit!';
  },

  SDL_LockSurface: function(surf) {
    var surfData = SDL.surfaces[surf];
    surfData.image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
    if (SDL.defaults.copyScreenOnLock) {
      // Copy pixel data to somewhere accessible to 'C/C++'
      var num = surfData.image.data.length;
      for (var i = 0; i < num; i++) {
        IHEAP[surfData.buffer+i] = surfData.image.data[i];
      }
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
    if (!surfData.colors) {
      for (var i = 0; i < num; i++) {
        surfData.image.data[i] = IHEAP[surfData.buffer+i];
      }
    } else {
      var width = Module.canvas.width;
      var height = Module.canvas.height;
      var s = surfData.buffer;
      for (var y = 0; y < height; y++) {
        var base = y*width*4;
        for (var x = 0; x < width; x++) {
          var val = IHEAP[s++];
          var color = surfData.colors[val];
          surfData.image.data[base+x*4+0] = color[0];
          surfData.image.data[base+x*4+1] = color[1];
          surfData.image.data[base+x*4+2] = color[2];
          //surfData.image.data[base+x*4+3] = color[3];
        }
        s += width*3;
      }
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

  SDL_UpdateRect: function(surf, x, y, w, h) {
    // We actually do the whole screen in Unlock...
  },

  SDL_Delay: function(delay) {
    print('SDL_Delay called! - potential infinite loop');
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

  SDL_GetTicks: function() {
    return Math.floor(Date.now() - SDL.startTime);
  },

  SDL_PollEvent: function(ptr) {
    if (SDL.events.length === 0) return 0;
    if (ptr) {
      SDL.makeCEvent(SDL.events.shift(), ptr);
    }
    return 1;
  },

  SDL_SetColors: function(surf, colors, firstColor, nColors) {
    var surfData = SDL.surfaces[surf];
    surfData.colors = [];
    for (var i = firstColor; i < nColors; i++) {
      surfData.colors[i] = IHEAP.slice(colors + i*4, colors + i*4 + 4);
    }
    return 1;
  },

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
  },

  // SDL_Audio

  SDL_OpenAudio: function(desired, obtained) {
    return -1;
  },

  SDL_CloseAudio: function() {},
  SDL_LockAudio: function() {},
  SDL_UnlockAudio: function() {},
});

