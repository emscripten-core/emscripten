//"use strict";

// To use emscripten's SDL library here, you need to define
// Module.canvas and at least one of Module.ctx2D, Module.ctxGL.
//
// More specifically, for 2D our SDL implementation will look for
// Module.canvas and Module.ctx2D. You should fill them using
// something like
//
//      function onLoad() {
//        // Pass canvas and context to the generated code
//        Module.canvas = document.getElementById('canvas');
//        Module.ctx2D = Module.canvas.getContext('2d');
//      }
//
// Note that this must be called during onload, since you will
// only be able to access the canvas element in the page after
// it loads. You will likely also want to disable running by
// default, with something like
//
//      var Module = {
//        noInitialRun: true
//      };
//
// which is defined BEFORE you load the compiled code.

// The test_emcc test in the tests/runner.py will test this
// in its last phase, where it generates HTML. You can see
// a concrete example there. The HTML source is in src/shell.html.
// Here is a more comprehensive example:

/*
<html>
  <head>
    <title>Demo</title>
    <script type='text/javascript'>
      var Module = {
        noInitialRun: true
      };

      // implement print
      var print = function(text) {
        var element = document.getElementById('output')
        element.innerHTML = text.replace('\n', '<br>', 'g') + element.innerHTML;
      }
    </script>
    <script src='doom.ccsimple.js' type='text/javascript'></script>
    <script type='text/javascript'>
      function onLoad() {
        // Pass canvas and context to the generated code, and do the actual run() here
        Module.canvas = document.getElementById('canvas');
        Module.ctx2D = Module.canvas.getContext('2d');
        if (!Module.ctx2D) {
          alert('Canvas not available :(');
          return;
        }
        Module.run();
      }
    </script>
  <body onload='onLoad()' style='background-color: black; color: white'>
    <center>
      <canvas id='canvas' width='320' height='200'></canvas>
    </center>
    <div id='output'></div>
  </body>
</html>
*/

// Other stuff to take into account:
//
//  * Make sure alpha values are proper in your input. If they are all 0, everything will be transparent!
//
//  * It's best to set the ctx stuff in your html file, as above. Otherwise you will need
//    to edit the generated .js file each time you generate it.
//
//  * Your code should not write a 32-bit value and expect that to set an RGBA pixel.
//    The reason is that that data will be read as 8-bit values, and according to the
//    load-store consistency assumption, it should be written that way (see docs/paper.pdf).
//    Instead, do something like        *ptr++ = R; *ptr++ = G; *ptr++ = B;
//
//  * A normal C++ main loop with SDL_Delay will not work in JavaScript - there is no way
//    to wait for a short time without locking up the web page entirely. The simplest
//    solution here is to have a singleIteration() function which is a single loop
//    iteration, and from JS to do something like      setInterval(_singleIteration, 1/30)
//
//  * SDL_Quit does nothing.

mergeInto(LibraryManager.library, {
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
      38:  273, // up arrow
      40:  274, // down arrow
      37:  276, // left arrow
      39:  275, // right arrow
      17:  305, // control (right, or left)
      18:  308, // alt
      109: 45, // minus
      16:  304 // shift
    },

    structs: {
      Rect: Runtime.generateStructInfo([
        ['i16', 'x'], ['i16', 'y'], ['i16', 'w'], ['i16', 'h'], 
      ]),
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
      ]),
      AudioSpec: Runtime.generateStructInfo([
        ['i32', 'freq'],
        ['i16', 'format'],
        ['i8', 'channels'],
        ['i8', 'silence'],
        ['i16', 'samples'],
        ['i32', 'size'],
        ['void*', 'callback'],
        ['void*', 'userdata']
      ])
    },

    makeSurface: function(width, height, flags) {
      var surf = _malloc(14*Runtime.QUANTUM_SIZE);  // SDL_Surface has 14 fields of quantum size
      var buffer = _malloc(width*height*4);
      var pixelFormat = _malloc(18*Runtime.QUANTUM_SIZE);
      flags |= 1; // SDL_HWSURFACE - this tells SDL_MUSTLOCK that this needs to be locked

      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*0', '0', 'flags', 'i32') }}}         // SDL_Surface.flags
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*1', '0', 'pixelFormat', 'void*') }}} // SDL_Surface.format TODO
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*2', '0', 'width', 'i32') }}}         // SDL_Surface.w
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*3', '0', 'height', 'i32') }}}        // SDL_Surface.h
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*4', '0', 'width*4', 'i16') }}}       // SDL_Surface.pitch, assuming RGBA for now,
                                                                               // since that is what ImageData gives us in browsers
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*5', '0', 'buffer', 'void*') }}}      // SDL_Surface.pixels
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*6', '0', '0', 'i32*') }}}      // SDL_Surface.offset

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
        canvas: Module['canvas'],
        ctx: Module['ctx2D'],
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
      //event.preventDefault();
      return false;
    },
    
    makeCEvent: function(event, ptr) {
      if (typeof event === 'number') {
        // This is a pointer to a native C event that was SDL_PushEvent'ed
        _memcpy(ptr, event, SDL.structs.KeyboardEvent.__size__);
        return;
      }

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
      addEventListener(event, SDL.receiveEvent, true);
    });
    return 0; // success
  },

  SDL_WasInit: function() { return 0 }, // TODO

  SDL_GetVideoInfo: function() {
    // %struct.SDL_VideoInfo = type { i32, i32, %struct.SDL_PixelFormat*, i32, i32 } - 5 fields of quantum size
    var ret = _malloc(5*Runtime.QUANTUM_SIZE);
    {{{ makeSetValue('ret+Runtime.QUANTUM_SIZE*0', '0', '0', 'i32') }}} // TODO
    {{{ makeSetValue('ret+Runtime.QUANTUM_SIZE*1', '0', '0', 'i32') }}} // TODO
    {{{ makeSetValue('ret+Runtime.QUANTUM_SIZE*2', '0', '0', 'void*') }}}
    {{{ makeSetValue('ret+Runtime.QUANTUM_SIZE*3', '0', 'SDL.defaults.width', 'i32') }}}
    {{{ makeSetValue('ret+Runtime.QUANTUM_SIZE*4', '0', 'SDL.defaults.height', 'i32') }}}
    return ret;
  },

  SDL_ListModes: function(format, flags) {
    return -1; // -1 == all modes are ok. TODO
  },

  SDL_GL_SetAttribute: function(attr, value) {
    // TODO
  },

  SDL_SetVideoMode: function(width, height, depth, flags) {
    Module['canvas'].width = width;
    Module['canvas'].height = height;
    return SDL.screen = SDL.makeSurface(width, height, flags);
  },

  SDL_Quit: function() {
    print('SDL_Quit called (and ignored)');
  },

  SDL_LockSurface: function(surf) {
    var surfData = SDL.surfaces[surf];
    if (!surfData.image) {
      surfData.image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
      var data = surfData.image.data;
      var num = data.length;
      for (var i = 0; i < num/4; i++) {
        data[i*4+3] = 255; // opacity, as canvases blend alpha
      }
    }
    if (SDL.defaults.copyScreenOnLock) {
      // Copy pixel data to somewhere accessible to 'C/C++'
      var num2 = surfData.image.data.length;
      for (var i = 0; i < num2; i++) {
        {{{ makeSetValue('surfData.buffer', 'i', 'surfData.image.data[i]', 'i8') }}};
      }
    }
    // Mark in C/C++-accessible SDL structure
    // SDL_Surface has the following fields: Uint32 flags, SDL_PixelFormat *format; int w, h; Uint16 pitch; void *pixels; ...
    // So we have fields all of the same size, and 5 of them before us.
    // TODO: Use macros like in library.js
    {{{ makeSetValue('surf', '5*Runtime.QUANTUM_SIZE', 'surfData.buffer', 'void*') }}};
  },

  SDL_UnlockSurface: function(surf) {
    var surfData = SDL.surfaces[surf];
    // Copy pixel data to image
    var num = surfData.image.data.length;
    if (!surfData.colors) {
      var data = surfData.image.data;
      var buffer = surfData.buffer;
#if USE_TYPED_ARRAYS == 2
      assert(buffer % 4 == 0, 'Invalid buffer offset: ' + buffer);
      var src = buffer >> 2;
      var dst = 0;
      while (dst < num) {
        var val = HEAP32[src]; // This is optimized. Instead, we could do {{{ makeGetValue('buffer', 'dst', 'i32') }}};
        data[dst]   = val & 0xff;
        data[dst+1] = (val >> 8) & 0xff;
        data[dst+2] = (val >> 16) & 0xff;
        data[dst+3] = 0xff;
        src++;
        dst += 4;
      }
#else
      for (var i = 0; i < num; i++) {
        // We may need to correct signs here. Potentially you can hardcode a write of 255 to alpha, say, and
        // the compiler may decide to write -1 in the llvm bitcode...
        data[i] = {{{ makeGetValue('buffer', 'i', 'i8') + (CORRECT_SIGNS ? '&0xff' : '') }}};
        if (i % 4 == 3) data[i] = 0xff;
      }
#endif
    } else {
      var width = Module['canvas'].width;
      var height = Module['canvas'].height;
      var s = surfData.buffer;
      var data = surfData.image.data;
      var colors = surfData.colors;
      for (var y = 0; y < height; y++) {
        var base = y*width*4;
        for (var x = 0; x < width; x++) {
          // See comment above about signs
          var val = {{{ makeGetValue('s++', '0', 'i8') + (CORRECT_SIGNS ? '&0xff' : '') }}};
          var color = colors[val] || [Math.floor(Math.random()*255),Math.floor(Math.random()*255),Math.floor(Math.random()*255)]; // XXX
          var start = base + x*4;
          data[start]   = color[0];
          data[start+1] = color[1];
          data[start+2] = color[2];
        }
        s += width*3;
      }
    }
    // Copy to canvas
    surfData.ctx.putImageData(surfData.image, 0, 0);
    // Note that we save the image, so future writes are fast. But, memory is not yet released
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
    return allocate(intArrayFromString("SDL is cool"), 'i8');
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
    {{{ makeCopyValues('dstData.buffer', 'srcData.buffer', 'srcData.width*srcData.height*4', 'i8', null, 1) }}}
    return 0;
  },

  SDL_FillRect: function(surf, rect, color) {
    var surfData = SDL.surfaces[surf];
    var c1 = color & 0xff;
    var c2 = (color >> 8) & 0xff;
    var c3 = (color >> 16) & 0xff;
    var rx = {{{ makeGetValue('rect + SDL.structs.Rect.x', '0', 'i16') }}};
    var ry = {{{ makeGetValue('rect + SDL.structs.Rect.y', '0', 'i16') }}};
    var rw = {{{ makeGetValue('rect + SDL.structs.Rect.w', '0', 'i16') }}};
    var rh = {{{ makeGetValue('rect + SDL.structs.Rect.h', '0', 'i16') }}};
    var data = surfData.image.data;
    var width = surfData.width;
    for (var y = ry; y < ry + rh; y++) {
      var base = y*width*4;
      for (var x = rx; x < rx + rw; x++) {
        var start = x*4 + base;
        data[start]   = c1;
        data[start+1] = c2;
        data[start+2] = c3;
      }
    }
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

  SDL_PushEvent: function(ptr) {
    SDL.events.push(ptr); // XXX Should we copy it? Not clear from API

    return 0;
  },

  SDL_SetColors: function(surf, colors, firstColor, nColors) {
    var surfData = SDL.surfaces[surf];
    surfData.colors = [];
    for (var i = firstColor; i < nColors; i++) {
      surfData.colors[i] = Array_copy(colors + i*4, colors + i*4 + 4);
    }
    return 1;
  },

  SDL_MapRGB: function(fmt, r, g, b) {
    // Canvas screens are always RGBA
    return r + (g << 8) + (b << 16);
  },

  SDL_WM_GrabInput: function() {},
  SDL_ShowCursor: function() {},

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
    // FIXME: Assumes 16-bit audio
    assert(obtained === 0, 'Cannot return obtained SDL audio params');

    SDL.audio = {
      freq: {{{ makeGetValue('desired', 'SDL.structs.AudioSpec.freq', 'i32', 0, 1) }}},
      format: {{{ makeGetValue('desired', 'SDL.structs.AudioSpec.format', 'i16', 0, 1) }}},
      channels: {{{ makeGetValue('desired', 'SDL.structs.AudioSpec.channels', 'i8', 0, 1) }}},
      samples: {{{ makeGetValue('desired', 'SDL.structs.AudioSpec.samples', 'i16', 0, 1) }}},
      callback: {{{ makeGetValue('desired', 'SDL.structs.AudioSpec.callback', 'void*', 0, 1) }}},
      userdata: {{{ makeGetValue('desired', 'SDL.structs.AudioSpec.userdata', 'void*', 0, 1) }}},
      paused: true,
      timer: null
    };

    var totalSamples = SDL.audio.samples*SDL.audio.channels;
    SDL.audio.bufferSize = totalSamples*2; // hardcoded 16-bit audio
    SDL.audio.buffer = _malloc(SDL.audio.bufferSize);
    SDL.audio.caller = function() {
      FUNCTION_TABLE[SDL.audio.callback](SDL.audio.userdata, SDL.audio.buffer, SDL.audio.bufferSize);
      SDL.audio.pushAudio(SDL.audio.buffer, SDL.audio.bufferSize);
    };
    // Mozilla Audio API. TODO: Other audio APIs
    try {
      SDL.audio.mozOutput = new Audio();
      SDL.audio.mozOutput['mozSetup'](SDL.audio.channels, SDL.audio.freq); // use string attributes on mozOutput for closure compiler
      SDL.audio.mozBuffer = new Float32Array(totalSamples);
      SDL.audio.pushAudio = function(ptr, size) {
        var mozBuffer = SDL.audio.mozBuffer;
        for (var i = 0; i < totalSamples; i++) {
          mozBuffer[i] = ({{{ makeGetValue('ptr', 'i*2', 'i16', 0, 1) }}} / 65536)-1; // hardcoded 16-bit audio
        }
        SDL.audio.mozOutput['mozWriteAudio'](mozBuffer);
      }
    } catch(e) {
      SDL.audio = null;
    }
    if (!SDL.audio) return -1;
    return 0;
  },

  SDL_PauseAudio: function(pauseOn) {
    if (SDL.audio.paused !== pauseOn) {
      SDL.audio.timer = pauseOn ? SDL.audio.timer && clearInterval(SDL.audio.timer) : setInterval(SDL.audio.caller, 1/35);
    }
    SDL.audio.paused = pauseOn;
  },

  SDL_CloseAudio: function() {
    if (SDL.audio) {
      _SDL_PauseAudio(1);
      _free(SDL.audio.buffer);
      SDL.audio = null;
    }
  },

  SDL_LockAudio: function() {},
  SDL_UnlockAudio: function() {},

  SDL_CreateMutex: function() { return 0 },
  SDL_LockMutex: function() {},
  SDL_UnlockMutex: function() {},
  SDL_DestroyMutex: function() {},

  SDL_CreateCond: function() { return 0 },
  SDL_CondSignal: function() {},
  SDL_CondWait: function() {},
  SDL_DestroyCond: function() {},

//SDL_CreateYUVOverlay
//SDL_CreateThread, SDL_WaitThread etc

  // SDL Mixer

  Mix_OpenAudio: function() { return -1 },
});

