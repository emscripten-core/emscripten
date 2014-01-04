//"use strict";

// See browser tests for examples (tests/runner.py, search for sdl_). Run with
//    python tests/runner.py browser

// Notes:
//  SDL_VIDEORESIZE: This is sent when the canvas is resized. Note that the user
//                   cannot manually do so, so this is only sent when the
//                   program manually resizes it (emscripten_set_canvas_size
//                   or otherwise).

var LibrarySDL = {
  $SDL__deps: ['$FS', '$PATH', '$Browser'],
  $SDL: {
    defaults: {
      width: 320,
      height: 200,
      copyOnLock: true
    },

    version: null,

    surfaces: {},
    // A pool of freed canvas elements. Reusing them avoids GC pauses.
    canvasPool: [],
    events: [],
    fonts: [null],

    // The currently preloaded audio elements ready to be played
    audios: [null],
    rwops: [null],
    // The currently playing audio element.  There's only one music track.
    music: {
      audio: null,
      volume: 1.0
    },
    mixerFrequency: 22050,
    mixerFormat: {{{ cDefine('AUDIO_S16LSB') }}}, //0x8010, // AUDIO_S16LSB
    mixerNumChannels: 2,
    mixerChunkSize: 1024,
    channelMinimumNumber: 0,

    GL: false, // Set to true if we call SDL_SetVideoMode with SDL_OPENGL, and if so, we do not create 2D canvases&contexts for blitting
               // Note that images loaded before SDL_SetVideoMode will not get this optimization

    // all possible GL attributes, with their default value
    glAttributes: {
      0: 3,    /* SDL_GL_RED_SIZE */
      1: 3,    /* SDL_GL_GREEN_SIZE */
      2: 2,    /* SDL_GL_BLUE_SIZE */
      3: 0,    /* SDL_GL_ALPHA_SIZE */
      4: 0,    /* SDL_GL_BUFFER_SIZE */
      5: 1,    /* SDL_GL_DOUBLEBUFFER */
      6: 16,   /* SDL_GL_DEPTH_SIZE */
      7: 0,    /* SDL_GL_STENCIL_SIZE */
      8: 0,    /* SDL_GL_ACCUM_RED_SIZE */
      9: 0,    /* SDL_GL_ACCUM_GREEN_SIZE */
      10: 0,   /* SDL_GL_ACCUM_BLUE_SIZE */
      11: 0,   /* SDL_GL_ACCUM_ALPHA_SIZE */
      12: 0,   /* SDL_GL_STEREO */
      13: 0,   /* SDL_GL_MULTISAMPLEBUFFERS */
      14: 0,   /* SDL_GL_MULTISAMPLESAMPLES */
      15: 1,   /* SDL_GL_ACCELERATED_VISUAL */
      16: 0,   /* SDL_GL_RETAINED_BACKING */
      17: 0,   /* SDL_GL_CONTEXT_MAJOR_VERSION */
      18: 0    /* SDL_GL_CONTEXT_MINOR_VERSION */
    },

    keyboardState: null,
    keyboardMap: {},

    canRequestFullscreen: false,
    isRequestingFullscreen: false,

    textInput: false,

    startTime: null,
    initFlags: 0, // The flags passed to SDL_Init
    buttonState: 0,
    modState: 0,
    DOMButtons: [0, 0, 0],

    DOMEventToSDLEvent: {},

    keyCodes: { // DOM code ==> SDL code. See https://developer.mozilla.org/en/Document_Object_Model_%28DOM%29/KeyboardEvent and SDL_keycode.h
      46: 127, // SDLK_DEL == '\177'
      38:  1106, // up arrow
      40:  1105, // down arrow
      37:  1104, // left arrow
      39:  1103, // right arrow

      33: 1099, // pagedup
      34: 1102, // pagedown

      17:  1248, // control (right, or left)
      18:  1250, // alt
      173: 45, // minus
      16:  1249, // shift
      
      96: 88 | 1<<10, // keypad 0
      97: 89 | 1<<10, // keypad 1
      98: 90 | 1<<10, // keypad 2
      99: 91 | 1<<10, // keypad 3
      100: 92 | 1<<10, // keypad 4
      101: 93 | 1<<10, // keypad 5
      102: 94 | 1<<10, // keypad 6
      103: 95 | 1<<10, // keypad 7
      104: 96 | 1<<10, // keypad 8
      105: 97 | 1<<10, // keypad 9

      112: 58 | 1<<10, // F1
      113: 59 | 1<<10, // F2
      114: 60 | 1<<10, // F3
      115: 61 | 1<<10, // F4
      116: 62 | 1<<10, // F5
      117: 63 | 1<<10, // F6
      118: 64 | 1<<10, // F7
      119: 65 | 1<<10, // F8
      120: 66 | 1<<10, // F9
      121: 67 | 1<<10, // F10
      122: 68 | 1<<10, // F11
      123: 69 | 1<<10, // F12

      188: 44, // comma
      190: 46, // period
      191: 47, // slash (/)
      192: 96, // backtick/backquote (`)
    },

    scanCodes: { // SDL keycode ==> SDL scancode. See SDL_scancode.h
      97: 4, // A
      98: 5,
      99: 6,
      100: 7,
      101: 8,
      102: 9,
      103: 10,
      104: 11,
      105: 12,
      106: 13,
      107: 14,
      108: 15,
      109: 16,
      110: 17,
      111: 18,
      112: 19,
      113: 20,
      114: 21,
      115: 22,
      116: 23,
      117: 24,
      118: 25,
      119: 26,
      120: 27,
      121: 28,
      122: 29, // Z
       49: 30, // 1
       50: 31,
       51: 32,
       52: 33,
       53: 34,
       54: 35,
       55: 36,
       56: 37,
       57: 38, // 9
       48: 39, // 0
       13: 40, // return
       27: 41, // escape
        8: 42, // backspace
        9: 43, // tab
       32: 44, // space
       61: 46, // equals
       91: 47, // left bracket
       93: 48, // right bracket
       92: 49, // backslash
       59: 51, // ;
       96: 52, // apostrophe
       44: 54, // comma
       46: 55, // period
       47: 56, // slash
      305: 224, // ctrl
      308: 226, // alt
    },
    loadRect: function(rect) {
      return {
        x: {{{ makeGetValue('rect + ' + C_STRUCTS.SDL_Rect.x, '0', 'i32') }}},
        y: {{{ makeGetValue('rect + ' + C_STRUCTS.SDL_Rect.y, '0', 'i32') }}},
        w: {{{ makeGetValue('rect + ' + C_STRUCTS.SDL_Rect.w, '0', 'i32') }}},
        h: {{{ makeGetValue('rect + ' + C_STRUCTS.SDL_Rect.h, '0', 'i32') }}}
      };
    },

    // Load SDL color into a CSS-style color specification
    loadColorToCSSRGB: function(color) {
      var rgba = {{{ makeGetValue('color', '0', 'i32') }}};
      return 'rgb(' + (rgba&255) + ',' + ((rgba >> 8)&255) + ',' + ((rgba >> 16)&255) + ')';
    },
    loadColorToCSSRGBA: function(color) {
      var rgba = {{{ makeGetValue('color', '0', 'i32') }}};
      return 'rgba(' + (rgba&255) + ',' + ((rgba >> 8)&255) + ',' + ((rgba >> 16)&255) + ',' + (((rgba >> 24)&255)/255) + ')';
    },

    translateColorToCSSRGBA: function(rgba) {
      return 'rgba(' + (rgba&0xff) + ',' + (rgba>>8 & 0xff) + ',' + (rgba>>16 & 0xff) + ',' + (rgba>>>24)/0xff + ')';
    },

    translateRGBAToCSSRGBA: function(r, g, b, a) {
      return 'rgba(' + (r&0xff) + ',' + (g&0xff) + ',' + (b&0xff) + ',' + (a&0xff)/255 + ')';
    },

    translateRGBAToColor: function(r, g, b, a) {
      return r | g << 8 | b << 16 | a << 24;
    },

    makeSurface: function(width, height, flags, usePageCanvas, source, rmask, gmask, bmask, amask) {
      flags = flags || 0;
      var surf = _malloc({{{ C_STRUCTS.SDL_Surface.__size__ }}});  // SDL_Surface has 15 fields of quantum size
      var buffer = _malloc(width*height*4); // TODO: only allocate when locked the first time
      var pixelFormat = _malloc({{{ C_STRUCTS.SDL_PixelFormat.__size__ }}});
      flags |= 1; // SDL_HWSURFACE - this tells SDL_MUSTLOCK that this needs to be locked

      //surface with SDL_HWPALETTE flag is 8bpp surface (1 byte)
      var is_SDL_HWPALETTE = flags & 0x00200000;  
      var bpp = is_SDL_HWPALETTE ? 1 : 4;
 
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.flags, 'flags', 'i32') }}}         // SDL_Surface.flags
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.format, 'pixelFormat', 'void*') }}} // SDL_Surface.format TODO
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.w, 'width', 'i32') }}}         // SDL_Surface.w
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.h, 'height', 'i32') }}}        // SDL_Surface.h
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.pitch, 'width * bpp', 'i32') }}}       // SDL_Surface.pitch, assuming RGBA or indexed for now,
                                                                               // since that is what ImageData gives us in browsers
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.pixels, 'buffer', 'void*') }}}      // SDL_Surface.pixels
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.clip_rect, '0', 'i32*') }}}      // SDL_Surface.offset

      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.refcount, '1', 'i32') }}}

      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.format, cDefine('SDL_PIXELFORMAT_RGBA8888'), 'i32') }}} // SDL_PIXELFORMAT_RGBA8888
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.palette, '0', 'i32') }}} // TODO
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.BitsPerPixel, 'bpp * 8', 'i8') }}}
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.BytesPerPixel, 'bpp', 'i8') }}}

      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.Rmask, 'rmask || 0x000000ff', 'i32') }}}
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.Gmask, 'gmask || 0x0000ff00', 'i32') }}}
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.Bmask, 'bmask || 0x00ff0000', 'i32') }}}
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.Amask, 'amask || 0xff000000', 'i32') }}}

      // Decide if we want to use WebGL or not
      var useWebGL = (flags & 0x04000000) != 0; // SDL_OPENGL
      SDL.GL = SDL.GL || useWebGL;
      var canvas;
      if (!usePageCanvas) {
        if (SDL.canvasPool.length > 0) {
          canvas = SDL.canvasPool.pop();
        } else {
          canvas = document.createElement('canvas');
        }
        canvas.width = width;
        canvas.height = height;
      } else {
        canvas = Module['canvas'];
      }

      var webGLContextAttributes = {
        antialias: ((SDL.glAttributes[13 /*SDL_GL_MULTISAMPLEBUFFERS*/] != 0) && (SDL.glAttributes[14 /*SDL_GL_MULTISAMPLESAMPLES*/] > 1)),
        depth: (SDL.glAttributes[6 /*SDL_GL_DEPTH_SIZE*/] > 0),
        stencil: (SDL.glAttributes[7 /*SDL_GL_STENCIL_SIZE*/] > 0)
      };
      
      var ctx = Browser.createContext(canvas, useWebGL, usePageCanvas, webGLContextAttributes);
            
      SDL.surfaces[surf] = {
        width: width,
        height: height,
        canvas: canvas,
        ctx: ctx,
        surf: surf,
        buffer: buffer,
        pixelFormat: pixelFormat,
        alpha: 255,
        flags: flags,
        locked: 0,
        usePageCanvas: usePageCanvas,
        source: source,

        isFlagSet: function(flag) {
          return flags & flag;
        }
      };

      return surf;
    },

    // Copy data from the C++-accessible storage to the canvas backing 
    // for surface with HWPALETTE flag(8bpp depth)
    copyIndexedColorData: function(surfData, rX, rY, rW, rH) {
      // HWPALETTE works with palette
      // setted by SDL_SetColors
      if (!surfData.colors) {
        return;
      }
      
      var fullWidth  = Module['canvas'].width;
      var fullHeight = Module['canvas'].height;

      var startX  = rX || 0;
      var startY  = rY || 0;
      var endX    = (rW || (fullWidth - startX)) + startX;
      var endY    = (rH || (fullHeight - startY)) + startY;
      
      var buffer  = surfData.buffer;
      var data    = surfData.image.data;
      var colors  = surfData.colors;

      for (var y = startY; y < endY; ++y) {
        var indexBase = y * fullWidth;
        var colorBase = indexBase * 4;
        for (var x = startX; x < endX; ++x) {
          // HWPALETTE have only 256 colors (not rgba)
          var index = {{{ makeGetValue('buffer + indexBase + x', '0', 'i8', null, true) }}} * 3;
          var colorOffset = colorBase + x * 4;

          data[colorOffset   ] = colors[index   ];
          data[colorOffset +1] = colors[index +1];
          data[colorOffset +2] = colors[index +2];
          //unused: data[colorOffset +3] = color[index +3];
        }
      }
    },

    freeSurface: function(surf) {
      var refcountPointer = surf + {{{ C_STRUCTS.SDL_Surface.refcount }}};
      var refcount = {{{ makeGetValue('refcountPointer', '0', 'i32') }}};
      if (refcount > 1) {
        {{{ makeSetValue('refcountPointer', '0', 'refcount - 1', 'i32') }}};
        return;
      }

      var info = SDL.surfaces[surf];
      if (!info.usePageCanvas && info.canvas) SDL.canvasPool.push(info.canvas);
      _free(info.buffer);
      _free(info.pixelFormat);
      _free(surf);
      SDL.surfaces[surf] = null;
    },

    touchX: 0, touchY: 0,
    savedKeydown: null,

    receiveEvent: function(event) {
      switch(event.type) {
        case 'touchstart':
          event.preventDefault();
          var touch = event.touches[0];
          touchX = touch.pageX;
          touchY = touch.pageY;
          var event = {
            type: 'mousedown',
            button: 0,
            pageX: touchX,
            pageY: touchY
          };
          SDL.DOMButtons[0] = 1;
          SDL.events.push(event);
          break;
        case 'touchmove':
          event.preventDefault();
          var touch = event.touches[0];
          touchX = touch.pageX;
          touchY = touch.pageY;
          event = {
            type: 'mousemove',
            button: 0,
            pageX: touchX,
            pageY: touchY
          };
          SDL.events.push(event);
          break;
        case 'touchend':
          event.preventDefault();
          event = {
            type: 'mouseup',
            button: 0,
            pageX: touchX,
            pageY: touchY
          };
          SDL.DOMButtons[0] = 0;
          SDL.events.push(event);
          break;
        case 'mousemove':
          if (Browser.pointerLock) {
            // workaround for firefox bug 750111
            if ('mozMovementX' in event) {
              event['movementX'] = event['mozMovementX'];
              event['movementY'] = event['mozMovementY'];
            }
            // workaround for Firefox bug 782777
            if (event['movementX'] == 0 && event['movementY'] == 0) {
              // ignore a mousemove event if it doesn't contain any movement info
              // (without pointer lock, we infer movement from pageX/pageY, so this check is unnecessary)
              event.preventDefault();
              return;
            }
          }
          // fall through
        case 'keydown': case 'keyup': case 'keypress': case 'mousedown': case 'mouseup': case 'DOMMouseScroll': case 'mousewheel':
          // If we preventDefault on keydown events, the subsequent keypress events
          // won't fire. However, it's fine (and in some cases necessary) to
          // preventDefault for keys that don't generate a character. Otherwise,
          // preventDefault is the right thing to do in general.
          if (event.type !== 'keydown' || (event.keyCode === 8 /* backspace */ || event.keyCode === 9 /* tab */)) {
            event.preventDefault();
          }

          if (event.type == 'DOMMouseScroll' || event.type == 'mousewheel') {
            var button = (event.type == 'DOMMouseScroll' ? event.detail : -event.wheelDelta) > 0 ? 4 : 3;
            var event2 = {
              type: 'mousedown',
              button: button,
              pageX: event.pageX,
              pageY: event.pageY
            };
            SDL.events.push(event2);
            event = {
              type: 'mouseup',
              button: button,
              pageX: event.pageX,
              pageY: event.pageY
            };
          } else if (event.type == 'mousedown') {
            SDL.DOMButtons[event.button] = 1;
          } else if (event.type == 'mouseup') {
            // ignore extra ups, can happen if we leave the canvas while pressing down, then return,
            // since we add a mouseup in that case
            if (!SDL.DOMButtons[event.button]) {
              return;
            }

            SDL.DOMButtons[event.button] = 0;
          }

          // We can only request fullscreen as the result of user input.
          // Due to this limitation, we toggle a boolean on keydown which
          // SDL_WM_ToggleFullScreen will check and subsequently set another
          // flag indicating for us to request fullscreen on the following
          // keyup. This isn't perfect, but it enables SDL_WM_ToggleFullScreen
          // to work as the result of a keypress (which is an extremely
          // common use case).
          if (event.type === 'keydown') {
            SDL.canRequestFullscreen = true;
          } else if (event.type === 'keyup') {
            if (SDL.isRequestingFullscreen) {
              Module['requestFullScreen'](true, true);
              SDL.isRequestingFullscreen = false;
            }
            SDL.canRequestFullscreen = false;
          }

          // SDL expects a unicode character to be passed to its keydown events.
          // Unfortunately, the browser APIs only provide a charCode property on
          // keypress events, so we must backfill in keydown events with their
          // subsequent keypress event's charCode.
          if (event.type === 'keypress' && SDL.savedKeydown) {
            // charCode is read-only
            SDL.savedKeydown.keypressCharCode = event.charCode;
            SDL.savedKeydown = null;
          } else if (event.type === 'keydown') {
            SDL.savedKeydown = event;
          }

          // Don't push keypress events unless SDL_StartTextInput has been called.
          if (event.type !== 'keypress' || SDL.textInput) {
            SDL.events.push(event);
          }
          break;
        case 'mouseout':
          // Un-press all pressed mouse buttons, because we might miss the release outside of the canvas
          for (var i = 0; i < 3; i++) {
            if (SDL.DOMButtons[i]) {
              SDL.events.push({
                type: 'mouseup',
                button: i,
                pageX: event.pageX,
                pageY: event.pageY
              });
              SDL.DOMButtons[i] = 0;
            }
          }
          event.preventDefault();
          break;
        case 'blur':
        case 'visibilitychange': {
          // Un-press all pressed keys: TODO
          for (var code in SDL.keyboardMap) {
            SDL.events.push({
              type: 'keyup',
              keyCode: SDL.keyboardMap[code]
            });
          }
          event.preventDefault();
          break;
        }
        case 'unload':
          if (Browser.mainLoop.runner) {
            SDL.events.push(event);
            // Force-run a main event loop, since otherwise this event will never be caught!
            Browser.mainLoop.runner();
          }
          return;
        case 'resize':
          SDL.events.push(event);
          // manually triggered resize event doesn't have a preventDefault member
          if (event.preventDefault) {
            event.preventDefault();
          }
          break;
      }
      if (SDL.events.length >= 10000) {
        Module.printErr('SDL event queue full, dropping events');
        SDL.events = SDL.events.slice(0, 10000);
      }
      return;
    },

    handleEvent: function(event) {
      if (event.handled) return;
      event.handled = true;

      switch (event.type) {
        case 'keydown': case 'keyup': {
          var down = event.type === 'keydown';
          var code = event.keyCode;
          if (code >= 65 && code <= 90) {
            code += 32; // make lowercase for SDL
          } else {
            code = SDL.keyCodes[event.keyCode] || event.keyCode;
          }

          {{{ makeSetValue('SDL.keyboardState', 'code', 'down', 'i8') }}};
          // TODO: lmeta, rmeta, numlock, capslock, KMOD_MODE, KMOD_RESERVED
          SDL.modState = ({{{ makeGetValue('SDL.keyboardState', '1248', 'i8') }}} ? 0x0040 | 0x0080 : 0) | // KMOD_LCTRL & KMOD_RCTRL
            ({{{ makeGetValue('SDL.keyboardState', '1249', 'i8') }}} ? 0x0001 | 0x0002 : 0) | // KMOD_LSHIFT & KMOD_RSHIFT
            ({{{ makeGetValue('SDL.keyboardState', '1250', 'i8') }}} ? 0x0100 | 0x0200 : 0); // KMOD_LALT & KMOD_RALT

          if (down) {
            SDL.keyboardMap[code] = event.keyCode; // save the DOM input, which we can use to unpress it during blur
          } else {
            delete SDL.keyboardMap[code];
          }

          break;
        }
        case 'mousedown': case 'mouseup':
          if (event.type == 'mousedown') {
            // SDL_BUTTON(x) is defined as (1 << ((x)-1)).  SDL buttons are 1-3,
            // and DOM buttons are 0-2, so this means that the below formula is
            // correct.
            SDL.buttonState |= 1 << event.button;
          } else if (event.type == 'mouseup') {
            SDL.buttonState &= ~(1 << event.button);
          }
          // fall through
        case 'mousemove': {
          Browser.calculateMouseEvent(event);
          break;
        }
      }
    },

    makeCEvent: function(event, ptr) {
      if (typeof event === 'number') {
        // This is a pointer to a native C event that was SDL_PushEvent'ed
        _memcpy(ptr, event, {{{ C_STRUCTS.SDL_KeyboardEvent.__size__ }}}); // XXX
        return;
      }

      SDL.handleEvent(event);

      switch (event.type) {
        case 'keydown': case 'keyup': {
          var down = event.type === 'keydown';
          //Module.print('Received key event: ' + event.keyCode);
          var key = event.keyCode;
          if (key >= 65 && key <= 90) {
            key += 32; // make lowercase for SDL
          } else {
            key = SDL.keyCodes[event.keyCode] || event.keyCode;
          }
          var scan;
          if (key >= 1024) {
            scan = key - 1024;
          } else {
            scan = SDL.scanCodes[key] || key;
          }

          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.state, 'down ? 1 : 0', 'i8') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.repeat, '0', 'i8') }}} // TODO
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.keysym + C_STRUCTS.SDL_Keysym.scancode, 'scan', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.keysym + C_STRUCTS.SDL_Keysym.sym, 'key', 'i32') }}}
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.keysym + C_STRUCTS.SDL_Keysym.mod, 'SDL.modState', 'i16') }}}
          // some non-character keys (e.g. backspace and tab) won't have keypressCharCode set, fill in with the keyCode.
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.keysym + C_STRUCTS.SDL_Keysym.unicode, 'event.keypressCharCode || key', 'i32') }}}

          break;
        }
        case 'keypress': {
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_TextInputEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}}
          // Not filling in windowID for now
          var cStr = intArrayFromString(String.fromCharCode(event.charCode));
          for (var i = 0; i < cStr.length; ++i) {
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_TextInputEvent.text + ' + i', 'cStr[i]', 'i8') }}};
          }
          break;
        }
        case 'mousedown': case 'mouseup': case 'mousemove': {
          if (event.type != 'mousemove') {
            var down = event.type === 'mousedown';
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.button, 'event.button+1', 'i8') }}}; // DOM buttons are 0-2, SDL 1-3
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.state, 'down ? 1 : 0', 'i8') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.x, 'Browser.mouseX', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.y, 'Browser.mouseY', 'i32') }}};
          } else {
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.state, 'SDL.buttonState', 'i8') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.x, 'Browser.mouseX', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.y, 'Browser.mouseY', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.xrel, 'Browser.mouseMovementX', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.yrel, 'Browser.mouseMovementY', 'i32') }}};
          }
          break;
        }
        case 'unload': {
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          break;
        }
        case 'resize': {
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_ResizeEvent.w, 'event.w', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_ResizeEvent.h, 'event.h', 'i32') }}};
          break;
        }
        case 'joystick_button_up': case 'joystick_button_down': {
          var state = event.type === 'joystick_button_up' ? 0 : 1;
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_JoyButtonEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_JoyButtonEvent.which, 'event.index', 'i8') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_JoyButtonEvent.button, 'event.button', 'i8') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_JoyButtonEvent.state, 'state', 'i8') }}};
          break;
        }
        case 'joystick_axis_motion': {
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_JoyAxisEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_JoyAxisEvent.which, 'event.index', 'i8') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_JoyAxisEvent.axis, 'event.axis', 'i8') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_JoyAxisEvent.value, 'SDL.joystickAxisValueConversion(event.value)', 'i32') }}};
          break;
        }
        default: throw 'Unhandled SDL event: ' + event.type;
      }
    },

    estimateTextWidth: function(fontData, text) {
      var h = fontData.size;
      var fontString = h + 'px ' + fontData.name;
      var tempCtx = SDL.ttfContext;
#if ASSERTIONS
      assert(tempCtx, 'TTF_Init must have been called');
#endif
      tempCtx.save();
      tempCtx.font = fontString;
      var ret = tempCtx.measureText(text).width | 0;
      tempCtx.restore();
      return ret;
    },

    // Sound

    // Channels are a SDL abstraction for allowing multiple sound tracks to be
    // played at the same time.  We don't need to actually implement the mixing
    // since the browser engine handles that for us.  Therefore, in JS we just
    // maintain a list of channels and return IDs for them to the SDL consumer.
    allocateChannels: function(num) { // called from Mix_AllocateChannels and init
      if (SDL.numChannels && SDL.numChannels >= num && num != 0) return;
      SDL.numChannels = num;
      SDL.channels = [];
      for (var i = 0; i < num; i++) {
        SDL.channels[i] = {
          audio: null,
          volume: 1.0
        };
      }
    },

    setGetVolume: function(info, volume) {
      if (!info) return 0;
      var ret = info.volume * 128; // MIX_MAX_VOLUME
      if (volume != -1) {
        info.volume = volume / 128;
        if (info.audio) info.audio.volume = info.volume;
      }
      return ret;
    },

    // Debugging

    debugSurface: function(surfData) {
      console.log('dumping surface ' + [surfData.surf, surfData.source, surfData.width, surfData.height]);
      var image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
      var data = image.data;
      var num = Math.min(surfData.width, surfData.height);
      for (var i = 0; i < num; i++) {
        console.log('   diagonal ' + i + ':' + [data[i*surfData.width*4 + i*4 + 0], data[i*surfData.width*4 + i*4 + 1], data[i*surfData.width*4 + i*4 + 2], data[i*surfData.width*4 + i*4 + 3]]);
      }
    },

    // Joystick helper methods and state

    joystickEventState: 1, // SDL_ENABLE
    lastJoystickState: {}, // Map from SDL_Joystick* to their last known state. Required to determine if a change has occurred.
    // Maps Joystick names to pointers. Allows us to avoid reallocating memory for
    // joystick names each time this function is called.
    joystickNamePool: {},
    recordJoystickState: function(joystick, state) {
      // Standardize button state.
      var buttons = new Array(state.buttons.length);
      for (var i = 0; i < state.buttons.length; i++) {
        buttons[i] = SDL.getJoystickButtonState(state.buttons[i]);
      }

      SDL.lastJoystickState[joystick] = {
        buttons: buttons,
        axes: state.axes.slice(0),
        timestamp: state.timestamp,
        index: state.index,
        id: state.id
      };
    },
    // Retrieves the button state of the given gamepad button.
    // Abstracts away implementation differences.
    // Returns 'true' if pressed, 'false' otherwise.
    getJoystickButtonState: function(button) {
      if (typeof button === 'object') {
        // Current gamepad API editor's draft (Firefox Nightly)
        // https://dvcs.w3.org/hg/gamepad/raw-file/default/gamepad.html#idl-def-GamepadButton
        return button.pressed;
      } else {
        // Current gamepad API working draft (Firefox / Chrome Stable)
        // http://www.w3.org/TR/2012/WD-gamepad-20120529/#gamepad-interface
        return button > 0;
      }
    },
    // Queries for and inserts controller events into the SDL queue.
    queryJoysticks: function() {
      for (var joystick in SDL.lastJoystickState) {
        var state = SDL.getGamepad(joystick - 1);
        var prevState = SDL.lastJoystickState[joystick];
        // Check only if the timestamp has differed.
        // NOTE: Timestamp is not available in Firefox.
        if (typeof state.timestamp !== 'number' || state.timestamp !== prevState.timestamp) {
          var i;
          for (i = 0; i < state.buttons.length; i++) {
            var buttonState = SDL.getJoystickButtonState(state.buttons[i]);
            // NOTE: The previous state already has a boolean representation of
            //       its button, so no need to standardize its button state here.
            if (buttonState !== prevState.buttons[i]) {
              // Insert button-press event.
              SDL.events.push({
                type: buttonState ? 'joystick_button_down' : 'joystick_button_up',
                joystick: joystick,
                index: joystick - 1,
                button: i
              });
            }
          }
          for (i = 0; i < state.axes.length; i++) {
            if (state.axes[i] !== prevState.axes[i]) {
              // Insert axes-change event.
              SDL.events.push({
                type: 'joystick_axis_motion',
                joystick: joystick,
                index: joystick - 1,
                axis: i,
                value: state.axes[i]
              });
            }
          }

          SDL.recordJoystickState(joystick, state);
        }
      }
    },
    // Converts the double-based browser axis value [-1, 1] into SDL's 16-bit
    // value [-32768, 32767]
    joystickAxisValueConversion: function(value) {
      // Ensures that 0 is 0, 1 is 32767, and -1 is 32768.
      return Math.ceil(((value+1) * 32767.5) - 32768);
    },

    getGamepads: function() {
      var fcn = navigator.getGamepads || navigator.webkitGamepads || navigator.mozGamepads || navigator.gamepads || navigator.webkitGetGamepads;
      if (fcn !== undefined) {
        // The function must be applied on the navigator object.
        return fcn.apply(navigator);
      } else {
        return [];
      }
    },

    // Helper function: Returns the gamepad if available, or null if not.
    getGamepad: function(deviceIndex) {
      var gamepads = SDL.getGamepads();
      if (gamepads.length > deviceIndex && deviceIndex >= 0) {
        return gamepads[deviceIndex];
      }
      return null;
    },
  },

  SDL_Linked_Version: function() {
    if (SDL.version === null) {
      SDL.version = _malloc({{{ C_STRUCTS.SDL_version.__size__ }}});
      {{{ makeSetValue('SDL.version + ' + C_STRUCTS.SDL_version.major, '0', '1', 'i8') }}}
      {{{ makeSetValue('SDL.version + ' + C_STRUCTS.SDL_version.minor, '0', '3', 'i8') }}}
      {{{ makeSetValue('SDL.version + ' + C_STRUCTS.SDL_version.patch, '0', '0', 'i8') }}}
    }
    return SDL.version;
  },

  SDL_Init: function(initFlags) {
    SDL.startTime = Date.now();
    SDL.initFlags = initFlags;

    // capture all key events. we just keep down and up, but also capture press to prevent default actions
    if (!Module['doNotCaptureKeyboard']) {
      document.addEventListener("keydown", SDL.receiveEvent);
      document.addEventListener("keyup", SDL.receiveEvent);
      document.addEventListener("keypress", SDL.receiveEvent);
      window.addEventListener("blur", SDL.receiveEvent);
      document.addEventListener("visibilitychange", SDL.receiveEvent);
    }

    if (initFlags & 0x200) {
      // SDL_INIT_JOYSTICK
      // Firefox will not give us Joystick data unless we register this NOP
      // callback.
      // https://bugzilla.mozilla.org/show_bug.cgi?id=936104
      addEventListener("gamepadconnected", function() {});
    }

    window.addEventListener("unload", SDL.receiveEvent);
    SDL.keyboardState = _malloc(0x10000); // Our SDL needs 512, but 64K is safe for older SDLs
    _memset(SDL.keyboardState, 0, 0x10000);
    // Initialize this structure carefully for closure
    SDL.DOMEventToSDLEvent['keydown'] = 0x300 /* SDL_KEYDOWN */;
    SDL.DOMEventToSDLEvent['keyup'] = 0x301 /* SDL_KEYUP */;
    SDL.DOMEventToSDLEvent['keypress'] = 0x303 /* SDL_TEXTINPUT */;
    SDL.DOMEventToSDLEvent['mousedown'] = 0x401 /* SDL_MOUSEBUTTONDOWN */;
    SDL.DOMEventToSDLEvent['mouseup'] = 0x402 /* SDL_MOUSEBUTTONUP */;
    SDL.DOMEventToSDLEvent['mousemove'] = 0x400 /* SDL_MOUSEMOTION */;
    SDL.DOMEventToSDLEvent['unload'] = 0x100 /* SDL_QUIT */;
    SDL.DOMEventToSDLEvent['resize'] = 0x7001 /* SDL_VIDEORESIZE/SDL_EVENT_COMPAT2 */;
    // These are not technically DOM events; the HTML gamepad API is poll-based.
    // However, we define them here, as the rest of the SDL code assumes that
    // all SDL events originate as DOM events.
    SDL.DOMEventToSDLEvent['joystick_axis_motion'] = 0x600 /* SDL_JOYAXISMOTION */;
    SDL.DOMEventToSDLEvent['joystick_button_down'] = 0x603 /* SDL_JOYBUTTONDOWN */;
    SDL.DOMEventToSDLEvent['joystick_button_up'] = 0x604 /* SDL_JOYBUTTONUP */;
    return 0; // success
  },

  SDL_WasInit__deps: ['SDL_Init'],
  SDL_WasInit: function() {
    if (SDL.startTime === null) {
      _SDL_Init();
    }
    return 1;
  },

  SDL_GetVideoInfo: function() {
    // %struct.SDL_VideoInfo = type { i32, i32, %struct.SDL_PixelFormat*, i32, i32 } - 5 fields of quantum size
    var ret = _malloc(5*Runtime.QUANTUM_SIZE);
    {{{ makeSetValue('ret+Runtime.QUANTUM_SIZE*0', '0', '0', 'i32') }}} // TODO
    {{{ makeSetValue('ret+Runtime.QUANTUM_SIZE*1', '0', '0', 'i32') }}} // TODO
    {{{ makeSetValue('ret+Runtime.QUANTUM_SIZE*2', '0', '0', 'void*') }}}
    {{{ makeSetValue('ret+Runtime.QUANTUM_SIZE*3', '0', 'Module["canvas"].width', 'i32') }}}
    {{{ makeSetValue('ret+Runtime.QUANTUM_SIZE*4', '0', 'Module["canvas"].height', 'i32') }}}
    return ret;
  },

  SDL_ListModes: function(format, flags) {
    return -1; // -1 == all modes are ok. TODO
  },

  SDL_VideoModeOK: function(width, height, depth, flags) {
    // SDL_VideoModeOK returns 0 if the requested mode is not supported under any bit depth, or returns the 
    // bits-per-pixel of the closest available mode with the given width, height and requested surface flags
    return depth; // all modes are ok.
  },

  SDL_AudioDriverName__deps: ['SDL_VideoDriverName'],
  SDL_AudioDriverName: function(buf, max_size) {
    return _SDL_VideoDriverName(buf, max_size);
  },

  SDL_VideoDriverName: function(buf, max_size) {
    if (SDL.startTime === null) {
      return 0; //return NULL
    }
    //driverName - emscripten_sdl_driver
    var driverName = [101, 109, 115, 99, 114, 105, 112, 116, 101, 
      110, 95, 115, 100, 108, 95, 100, 114, 105, 118, 101, 114];

    var index = 0;
    var size  = driverName.length;

    if (max_size <= size) {
      size = max_size - 1; //-1 cause null-terminator
    }

    while (index < size) {
        var value = driverName[index];
        {{{ makeSetValue('buf', 'index', 'value', 'i8') }}};
        index++;
    }

    {{{ makeSetValue('buf', 'index', '0', 'i8') }}};
    return buf;
  },

  SDL_SetVideoMode: function(width, height, depth, flags) {
    ['mousedown', 'mouseup', 'mousemove', 'DOMMouseScroll', 'mousewheel', 'mouseout'].forEach(function(event) {
      Module['canvas'].addEventListener(event, SDL.receiveEvent, true);
    });

    // (0,0) means 'use fullscreen' in native; in Emscripten, use the current canvas size.
    if (width == 0 && height == 0) {
      var canvas = Module['canvas'];
      width = canvas.width;
      height = canvas.height;
    }

    Browser.setCanvasSize(width, height, true);
    // Free the old surface first.
    if (SDL.screen) {
      SDL.freeSurface(SDL.screen);
      SDL.screen = null;
    }
    SDL.screen = SDL.makeSurface(width, height, flags, true, 'screen');
    if (!SDL.addedResizeListener) {
      SDL.addedResizeListener = true;
      Browser.resizeListeners.push(function(w, h) {
        SDL.receiveEvent({
          type: 'resize',
          w: w,
          h: h
        });
      });
    }
    return SDL.screen;
  },

  SDL_GetVideoSurface: function() {
    return SDL.screen;
  },

  SDL_QuitSubSystem: function(flags) {
    Module.print('SDL_QuitSubSystem called (and ignored)');
  },

  SDL_Quit: function() {
    for (var i = 0; i < SDL.numChannels; ++i) {
      if (SDL.channels[i].audio) {
        SDL.channels[i].audio.pause();
      }
    }
    if (SDL.music.audio) {
      SDL.music.audio.pause();
    }
    Module.print('SDL_Quit called (and ignored)');
  },

  // Copy data from the canvas backing to a C++-accessible storage
  SDL_LockSurface: function(surf) {
    var surfData = SDL.surfaces[surf];

    surfData.locked++;
    if (surfData.locked > 1) return 0;

    // Mark in C/C++-accessible SDL structure
    // SDL_Surface has the following fields: Uint32 flags, SDL_PixelFormat *format; int w, h; Uint16 pitch; void *pixels; ...
    // So we have fields all of the same size, and 5 of them before us.
    // TODO: Use macros like in library.js
    {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.pixels, 'surfData.buffer', 'void*') }}};

    if (surf == SDL.screen && Module.screenIsReadOnly && surfData.image) return 0;

    surfData.image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
    if (surf == SDL.screen) {
      var data = surfData.image.data;
      var num = data.length;
      for (var i = 0; i < num/4; i++) {
        data[i*4+3] = 255; // opacity, as canvases blend alpha
      }
    }

    if (SDL.defaults.copyOnLock) {
      // Copy pixel data to somewhere accessible to 'C/C++'
      if (surfData.isFlagSet(0x00200000 /* SDL_HWPALETTE */)) {
        // If this is neaded then
        // we should compact the data from 32bpp to 8bpp index.
        // I think best way to implement this is use
        // additional colorMap hash (color->index).
        // Something like this:
        //
        // var size = surfData.width * surfData.height;
        // var data = '';
        // for (var i = 0; i<size; i++) {
        //   var color = SDL.translateRGBAToColor(
        //     surfData.image.data[i*4   ], 
        //     surfData.image.data[i*4 +1], 
        //     surfData.image.data[i*4 +2], 
        //     255);
        //   var index = surfData.colorMap[color];
        //   {{{ makeSetValue('surfData.buffer', 'i', 'index', 'i8') }}};
        // }
        throw 'CopyOnLock is not supported for SDL_LockSurface with SDL_HWPALETTE flag set' + new Error().stack;
      } else {
#if USE_TYPED_ARRAYS == 2
      HEAPU8.set(surfData.image.data, surfData.buffer);
#else
      var num2 = surfData.image.data.length;
      for (var i = 0; i < num2; i++) {
        {{{ makeSetValue('surfData.buffer', 'i', 'surfData.image.data[i]', 'i8') }}};
      }
#endif
      }
    }

    return 0;
  },

  // Copy data from the C++-accessible storage to the canvas backing
  SDL_UnlockSurface: function(surf) {
    assert(!SDL.GL); // in GL mode we do not keep around 2D canvases and contexts

    var surfData = SDL.surfaces[surf];

    surfData.locked--;
    if (surfData.locked > 0) return;

    // Copy pixel data to image
    if (surfData.isFlagSet(0x00200000 /* SDL_HWPALETTE */)) {
      SDL.copyIndexedColorData(surfData);
    } else if (!surfData.colors) {
      var data = surfData.image.data;
      var buffer = surfData.buffer;
#if USE_TYPED_ARRAYS == 2
      assert(buffer % 4 == 0, 'Invalid buffer offset: ' + buffer);
      var src = buffer >> 2;
      var dst = 0;
      var isScreen = surf == SDL.screen;
      var num;
      if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) {
        // IE10/IE11: ImageData objects are backed by the deprecated CanvasPixelArray,
        // not UInt8ClampedArray. These don't have buffers, so we need to revert
        // to copying a byte at a time. We do the undefined check because modern
        // browsers do not define CanvasPixelArray anymore.
        num = data.length;
        while (dst < num) {
          var val = HEAP32[src]; // This is optimized. Instead, we could do {{{ makeGetValue('buffer', 'dst', 'i32') }}};
          data[dst  ] = val & 0xff;
          data[dst+1] = (val >> 8) & 0xff;
          data[dst+2] = (val >> 16) & 0xff;
          data[dst+3] = isScreen ? 0xff : ((val >> 24) & 0xff);
          src++;
          dst += 4;
        }
      } else {
        var data32 = new Uint32Array(data.buffer);
        num = data32.length;
        if (isScreen) {
          while (dst < num) {
            // HEAP32[src++] is an optimization. Instead, we could do {{{ makeGetValue('buffer', 'dst', 'i32') }}};
            data32[dst++] = HEAP32[src++] | 0xff000000;
          }
        } else {
          while (dst < num) {
            data32[dst++] = HEAP32[src++];
          }
        }
      }
#else
      var num = surfData.image.data.length;
      for (var i = 0; i < num; i++) {
        // We may need to correct signs here. Potentially you can hardcode a write of 255 to alpha, say, and
        // the compiler may decide to write -1 in the llvm bitcode...
        data[i] = {{{ makeGetValue('buffer', 'i', 'i8', null, true) }}};
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
          var val = {{{ makeGetValue('s++', '0', 'i8', null, true) }}} * 3;
          var start = base + x*4;
          data[start]   = colors[val];
          data[start+1] = colors[val+1];
          data[start+2] = colors[val+2];
        }
        s += width*3;
      }
    }
    // Copy to canvas
    surfData.ctx.putImageData(surfData.image, 0, 0);
    // Note that we save the image, so future writes are fast. But, memory is not yet released
  },

  SDL_Flip: function(surf) {
    // We actually do this in Unlock, since the screen surface has as its canvas
    // backing the page canvas element
  },

  SDL_UpdateRect: function(surf, x, y, w, h) {
    // We actually do the whole screen in Unlock...
  },

  SDL_UpdateRects: function(surf, numrects, rects) {
    // We actually do the whole screen in Unlock...
  },

  SDL_Delay: function(delay) {
    if (!ENVIRONMENT_IS_WORKER) abort('SDL_Delay called on the main thread! Potential infinite loop, quitting.');
    // horrible busy-wait, but in a worker it at least does not block rendering
    var now = Date.now();
    while (Date.now() - now < delay) {}
  },

  SDL_WM_SetCaption: function(title, icon) {
    title = title && Pointer_stringify(title);
    icon = icon && Pointer_stringify(icon);
  },

  SDL_EnableKeyRepeat: function(delay, interval) {
    // TODO
  },

  SDL_GetKeyboardState: function(numKeys) {
    if (numKeys) {
      {{{ makeSetValue('numKeys', 0, 0x10000, 'i32') }}};
    }
    return SDL.keyboardState;
  },

  SDL_GetKeyState__deps: ['SDL_GetKeyboardState'],
  SDL_GetKeyState: function() {
    return _SDL_GetKeyboardState();
  },
  
  SDL_GetKeyName: function(key) {
    if (!SDL.keyName) {
      SDL.keyName = allocate(intArrayFromString('unknown key'), 'i8', ALLOC_NORMAL);
    }
    return SDL.keyName;
  },

  SDL_GetModState: function() {
    return SDL.modState;
  },

  SDL_GetMouseState: function(x, y) {
    if (x) {{{ makeSetValue('x', '0', 'Browser.mouseX', 'i32') }}};
    if (y) {{{ makeSetValue('y', '0', 'Browser.mouseY', 'i32') }}};
    return SDL.buttonState;
  },

  SDL_WarpMouse: function(x, y) {
    return; // TODO: implement this in a non-buggy way. Need to keep relative mouse movements correct after calling this
    var rect = Module["canvas"].getBoundingClientRect();
    SDL.events.push({
      type: 'mousemove',
      pageX: x + (window.scrollX + rect.left),
      pageY: y + (window.scrollY + rect.top)
    });
  },

  SDL_ShowCursor: function(toggle) {
    switch (toggle) {
      case 0: // SDL_DISABLE
        if (Browser.isFullScreen) { // only try to lock the pointer when in full screen mode
          Module['canvas'].requestPointerLock();
          return 0;
        } else { // else return SDL_ENABLE to indicate the failure
          return 1;
        }
        break;
      case 1: // SDL_ENABLE
        Module['canvas'].exitPointerLock();
        return 1;
        break;
      case -1: // SDL_QUERY
        return !Browser.pointerLock;
        break;
      default:
        console.log( "SDL_ShowCursor called with unknown toggle parameter value: " + toggle + "." );
        break;
    }
  },

  SDL_GetError: function() {
    if (!SDL.errorMessage) {
      SDL.errorMessage = allocate(intArrayFromString("unknown SDL-emscripten error"), 'i8', ALLOC_NORMAL);
    }
    return SDL.errorMessage;
  },

  SDL_CreateRGBSurface: function(flags, width, height, depth, rmask, gmask, bmask, amask) {
    return SDL.makeSurface(width, height, flags, false, 'CreateRGBSurface', rmask, gmask, bmask, amask);
  },

  SDL_CreateRGBSurfaceFrom: function(pixels, width, height, depth, pitch, rmask, gmask, bmask, amask) {
    // TODO: Actually fill pixel data to created surface.
    // TODO: Take into account depth and pitch parameters.
    console.log('TODO: Partially unimplemented SDL_CreateRGBSurfaceFrom called!');
    return SDL.makeSurface(width, height, 0, false, 'CreateRGBSurfaceFrom', rmask, gmask, bmask, amask);
  },

  SDL_DisplayFormatAlpha: function(surf) {
    var oldData = SDL.surfaces[surf];
    var ret = SDL.makeSurface(oldData.width, oldData.height, oldData.flags, false, 'copy:' + oldData.source);
    var newData = SDL.surfaces[ret];
    //newData.ctx.putImageData(oldData.ctx.getImageData(0, 0, oldData.width, oldData.height), 0, 0);
    newData.ctx.drawImage(oldData.canvas, 0, 0);
    return ret;
  },

  SDL_FreeSurface: function(surf) {
    if (surf) SDL.freeSurface(surf);
  },

  SDL_UpperBlit__deps: ['SDL_LockSurface'],
  SDL_UpperBlit: function(src, srcrect, dst, dstrect) {
    var srcData = SDL.surfaces[src];
    var dstData = SDL.surfaces[dst];
    var sr, dr;
    if (srcrect) {
      sr = SDL.loadRect(srcrect);
    } else {
      sr = { x: 0, y: 0, w: srcData.width, h: srcData.height };
    }
    if (dstrect) {
      dr = SDL.loadRect(dstrect);
    } else {
      dr = { x: 0, y: 0, w: -1, h: -1 };
    }
    var oldAlpha = dstData.ctx.globalAlpha;
    dstData.ctx.globalAlpha = srcData.alpha/255;
    dstData.ctx.drawImage(srcData.canvas, sr.x, sr.y, sr.w, sr.h, dr.x, dr.y, sr.w, sr.h);
    dstData.ctx.globalAlpha = oldAlpha;
    if (dst != SDL.screen) {
      // XXX As in IMG_Load, for compatibility we write out |pixels|
      console.log('WARNING: copying canvas data to memory for compatibility');
      _SDL_LockSurface(dst);
      dstData.locked--; // The surface is not actually locked in this hack
    }
    return 0;
  },

  SDL_LowerBlit: 'SDL_UpperBlit',

  SDL_FillRect: function(surf, rect, color) {
    var surfData = SDL.surfaces[surf];
    assert(!surfData.locked); // but we could unlock and re-lock if we must..
    
    if (surfData.isFlagSet(0x00200000 /* SDL_HWPALETTE */)) {
      //in SDL_HWPALETTE color is index (0..255)
      //so we should translate 1 byte value to
      //32 bit canvas
      var index = color * 3;
      color = SDL.translateRGBAToColor(surfData.colors[index], surfData.colors[index +1], surfData.colors[index +2], 255);
    }

    var r = rect ? SDL.loadRect(rect) : { x: 0, y: 0, w: surfData.width, h: surfData.height };
    surfData.ctx.save();
    surfData.ctx.fillStyle = SDL.translateColorToCSSRGBA(color);
    surfData.ctx.fillRect(r.x, r.y, r.w, r.h);
    surfData.ctx.restore();
    return 0;
  },

  SDL_BlitSurface__deps: ['SDL_UpperBlit'],
  SDL_BlitSurface: function(src, srcrect, dst, dstrect) {
    return _SDL_UpperBlit(src, srcrect, dst, dstrect);
  },

  zoomSurface: function(src, x, y, smooth) {
    var srcData = SDL.surfaces[src];
    var w = srcData.width * x;
    var h = srcData.height * y;
    var ret = SDL.makeSurface(Math.abs(w), Math.abs(h), srcData.flags, false, 'zoomSurface');
    var dstData = SDL.surfaces[ret];
    if (x >= 0 && y >= 0) dstData.ctx.drawImage(srcData.canvas, 0, 0, w, h);
    else {
      dstData.ctx.save();
      dstData.ctx.scale(x < 0 ? -1 : 1, y < 0 ? -1 : 1);
      dstData.ctx.drawImage(srcData.canvas, w < 0 ? w : 0, h < 0 ? h : 0, Math.abs(w), Math.abs(h));
      // XXX I think this should work according to the spec, but currently
      // fails on FF: dstData.ctx.drawImage(srcData.canvas, 0, 0, w, h);
      dstData.ctx.restore();
    }
    return ret;
  },

  rotozoomSurface__deps: ['zoomSurface'],
  rotozoomSurface: function(src, angle, zoom, smooth) {
    if (angle % 360 === 0) {
      return _zoomSurface(src, zoom, zoom, smooth);
    }
    var srcData = SDL.surfaces[src];
    var w = srcData.width * zoom;
    var h = srcData.height * zoom;
    var diagonal = Math.ceil(Math.sqrt(Math.pow(w, 2) + Math.pow(h, 2)));
    var ret = SDL.makeSurface(diagonal, diagonal, srcData.flags, false, 'rotozoomSurface');
    var dstData = SDL.surfaces[ret];
    dstData.ctx.translate(diagonal / 2, diagonal / 2);
    dstData.ctx.rotate(-angle * Math.PI / 180);
    dstData.ctx.drawImage(srcData.canvas, -w / 2, -h / 2, w, h);
    return ret;
  },

  SDL_SetAlpha: function(surf, flag, alpha) {
    SDL.surfaces[surf].alpha = alpha;
  },

  SDL_SetColorKey: function(surf, flag, key) {
    // SetColorKey assigns one color to be rendered as transparent. I don't
    // think the canvas API allows for anything like this, and iterating through
    // each pixel to replace that color seems prohibitively expensive.
    Runtime.warnOnce('SDL_SetColorKey is a no-op for performance reasons');
    return 0;
  },

  SDL_GetTicks: function() {
    return Math.floor(Date.now() - SDL.startTime);
  },

  SDL_PollEvent: function(ptr) {
    if (SDL.initFlags & 0x200 && SDL.joystickEventState) {
      // If SDL_INIT_JOYSTICK was supplied AND the joystick system is configured
      // to automatically query for events, query for joystick events.
      SDL.queryJoysticks();
    }
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

  SDL_PeepEvents: function(events, numEvents, action, from, to) {
    switch(action) {
      case 2: { // SDL_GETEVENT
        assert(numEvents == 1);
        var got = 0;
        while (SDL.events.length > 0 && numEvents > 0) {
          var type = SDL.DOMEventToSDLEvent[SDL.events[0].type];
          if (type < from || type > to) break;
          SDL.makeCEvent(SDL.events.shift(), events);
          got++;
          numEvents--;
          // events += sizeof(..)
        }
        return got;
      }
      default: throw 'SDL_PeepEvents does not yet support that action: ' + action;
    }
  },

  SDL_PumpEvents: function(){
    SDL.events.forEach(function(event) {
      SDL.handleEvent(event);
    });
  },

  SDL_SetColors: function(surf, colors, firstColor, nColors) {
    var surfData = SDL.surfaces[surf];

    // we should create colors array
    // only once cause client code
    // often wants to change portion 
    // of palette not all palette.
    if (!surfData.colors) {
      surfData.colors = new Uint8Array(256 * 3); //256 RGB colors
    } 

    for (var i = 0; i < nColors; ++i) {
      var index = (firstColor + i) * 3;
      surfData.colors[index] = {{{ makeGetValue('colors', 'i*4', 'i8', null, true) }}};
      surfData.colors[index + 1] = {{{ makeGetValue('colors', 'i*4 + 1', 'i8', null, true) }}};
      surfData.colors[index + 2] = {{{ makeGetValue('colors', 'i*4 + 2', 'i8', null, true) }}};
    }

    return 1;
  },

  SDL_SetPalette__deps: ['SDL_SetColors'],
  SDL_SetPalette: function(surf, flags, colors, firstColor, nColors) {
    return _SDL_SetColors(surf, colors, firstColor, nColors);
  },

  SDL_MapRGB: function(fmt, r, g, b) {
    // Canvas screens are always RGBA. We assume the machine is little-endian.
    return r&0xff|(g&0xff)<<8|(b&0xff)<<16|0xff000000;
  },

  SDL_MapRGBA: function(fmt, r, g, b, a) {
    // Canvas screens are always RGBA. We assume the machine is little-endian.
    return r&0xff|(g&0xff)<<8|(b&0xff)<<16|(a&0xff)<<24;
  },

  SDL_GetAppState: function() {
    var state = 0;

    if (Browser.pointerLock) {
      state |= 0x01;  // SDL_APPMOUSEFOCUS
    }
    if (document.hasFocus()) {
      state |= 0x02;  // SDL_APPINPUTFOCUS
    }
    state |= 0x04;  // SDL_APPACTIVE

    return state;
  },

  SDL_WM_GrabInput: function() {},
  
  SDL_WM_ToggleFullScreen: function(surf) {
    if (Browser.isFullScreen) {
      Module['canvas'].cancelFullScreen();
      return 1;
    } else {
      if (!SDL.canRequestFullscreen) {
        return 0;
      }
      SDL.isRequestingFullscreen = true;
      return 1;
    }
  },

  // SDL_Image

  IMG_Init: function(flags) {
    return flags; // We support JPG, PNG, TIF because browsers do
  },

  IMG_Load_RW__deps: ['SDL_LockSurface', 'SDL_FreeRW'],
  IMG_Load_RW: function(rwopsID, freeSrc) {
    try {
      // stb_image integration support
      function cleanup() {
        if (rwops && freeSrc) _SDL_FreeRW(rwopsID);
      };
      function addCleanup(func) {
        var old = cleanup;
        cleanup = function added_cleanup() {
          old();
          func();
        }
      }
      function callStbImage(func, params) {
        var x = Module['_malloc']({{{ QUANTUM_SIZE }}});
        var y = Module['_malloc']({{{ QUANTUM_SIZE }}});
        var comp = Module['_malloc']({{{ QUANTUM_SIZE }}});
        addCleanup(function() {
          Module['_free'](x);
          Module['_free'](y);
          Module['_free'](comp);
          if (data) Module['_stbi_image_free'](data);
        });
        var data = Module['_' + func].apply(null, params.concat([x, y, comp, 0]));
        if (!data) return null;
        return {
          rawData: true,
          data: data,
          width: {{{ makeGetValue('x', 0, 'i32') }}},
          height: {{{ makeGetValue('y', 0, 'i32') }}},
          size: {{{ makeGetValue('x', 0, 'i32') }}} * {{{ makeGetValue('y', 0, 'i32') }}} * {{{ makeGetValue('comp', 0, 'i32') }}},
          bpp: {{{ makeGetValue('comp', 0, 'i32') }}}
        };
      }

      var rwops = SDL.rwops[rwopsID];
      if (rwops === undefined) {
        return 0;
      }

      var filename = rwops.filename;
      if (filename === undefined) {
#if STB_IMAGE
        var raw = callStbImage('stbi_load_from_memory', [rwops.bytes, rwops.count]);
        if (!raw) return 0;
#else
        Runtime.warnOnce('Only file names that have been preloaded are supported for IMG_Load_RW. Consider using STB_IMAGE=1 if you want synchronous image decoding (see settings.js)');
        return 0;
#endif
      }

      if (!raw) {
        filename = PATH.resolve(filename);
        var raw = Module["preloadedImages"][filename];
        if (!raw) {
          if (raw === null) Module.printErr('Trying to reuse preloaded image, but freePreloadedMediaOnUse is set!');
#if STB_IMAGE
          var name = Module['_malloc'](filename.length+1);
          writeStringToMemory(filename, name);
          addCleanup(function() {
            Module['_free'](name);
          });
          var raw = callStbImage('stbi_load', [name]);
          if (!raw) return 0;
#else
          Runtime.warnOnce('Cannot find preloaded image ' + filename);
          Runtime.warnOnce('Cannot find preloaded image ' + filename + '. Consider using STB_IMAGE=1 if you want synchronous image decoding (see settings.js)');
          return 0;
#endif
        } else if (Module['freePreloadedMediaOnUse']) {
          Module["preloadedImages"][filename] = null;
        }
      }

      var surf = SDL.makeSurface(raw.width, raw.height, 0, false, 'load:' + filename);
      var surfData = SDL.surfaces[surf];
      surfData.ctx.globalCompositeOperation = "copy";
      if (!raw.rawData) {
        surfData.ctx.drawImage(raw, 0, 0, raw.width, raw.height, 0, 0, raw.width, raw.height);
      } else {
        var imageData = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
        if (raw.bpp == 4) {
          imageData.data.set({{{ makeHEAPView('U8', 'raw.data', 'raw.data+raw.size') }}});
        } else if (raw.bpp == 3) {
          var pixels = raw.size/3;
          var data = imageData.data;
          var sourcePtr = raw.data;
          var destPtr = 0;
          for (var i = 0; i < pixels; i++) {
            data[destPtr++] = {{{ makeGetValue('sourcePtr++', 0, 'i8', null, 1) }}};
            data[destPtr++] = {{{ makeGetValue('sourcePtr++', 0, 'i8', null, 1) }}};
            data[destPtr++] = {{{ makeGetValue('sourcePtr++', 0, 'i8', null, 1) }}};
            data[destPtr++] = 255;
          }
        } else {
          Module.printErr('cannot handle bpp ' + raw.bpp);
          return 0;
        }
        surfData.ctx.putImageData(imageData, 0, 0);
      }
      surfData.ctx.globalCompositeOperation = "source-over";
      // XXX SDL does not specify that loaded images must have available pixel data, in fact
      //     there are cases where you just want to blit them, so you just need the hardware
      //     accelerated version. However, code everywhere seems to assume that the pixels
      //     are in fact available, so we retrieve it here. This does add overhead though.
      _SDL_LockSurface(surf);
      surfData.locked--; // The surface is not actually locked in this hack
      if (SDL.GL) {
        // After getting the pixel data, we can free the canvas and context if we do not need to do 2D canvas blitting
        surfData.canvas = surfData.ctx = null;
      }
      return surf;
    } finally {
      cleanup();
    }
  },
  SDL_LoadBMP: 'IMG_Load',
  SDL_LoadBMP_RW: 'IMG_Load_RW',
  IMG_Load__deps: ['IMG_Load_RW', 'SDL_RWFromFile'],
  IMG_Load: function(filename){
    var rwops = _SDL_RWFromFile(filename);
    var result = _IMG_Load_RW(rwops, 1);
    return result;
  },

  // SDL_Audio

  SDL_OpenAudio: function(desired, obtained) {
    try {
      SDL.audio = {
        freq: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.freq, 'i32', 0, 1) }}},
        format: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.format, 'i16', 0, 1) }}},
        channels: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.channels, 'i8', 0, 1) }}},
        samples: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.samples, 'i16', 0, 1) }}}, // Samples in the CB buffer per single sound channel.
        callback: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.callback, 'void*', 0, 1) }}},
        userdata: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.userdata, 'void*', 0, 1) }}},
        paused: true,
        timer: null
      };
      // The .silence field tells the constant sample value that corresponds to the safe un-skewed silence value for the wave data.
      if (SDL.audio.format == 0x0008 /*AUDIO_U8*/) {
        SDL.audio.silence = 128; // Audio ranges in [0, 255], so silence is half-way in between.
      } else if (SDL.audio.format == 0x8010 /*AUDIO_S16LSB*/) {
        SDL.audio.silence = 0; // Signed data in range [-32768, 32767], silence is 0.
      } else {
        throw 'Invalid SDL audio format ' + SDL.audio.format + '!';
      }
      // Round the desired audio frequency up to the next 'common' frequency value.
      // Web Audio API spec states 'An implementation must support sample-rates in at least the range 22050 to 96000.'
      if (SDL.audio.freq <= 0) {
        throw 'Unsupported sound frequency ' + SDL.audio.freq + '!';
      } else if (SDL.audio.freq <= 22050) {
        SDL.audio.freq = 22050; // Take it safe and clamp everything lower than 22kHz to that.
      } else if (SDL.audio.freq <= 32000) {
        SDL.audio.freq = 32000;
      } else if (SDL.audio.freq <= 44100) {
        SDL.audio.freq = 44100;
      } else if (SDL.audio.freq <= 48000) {
        SDL.audio.freq = 48000;
      } else if (SDL.audio.freq <= 96000) {
        SDL.audio.freq = 96000;
      } else {
        throw 'Unsupported sound frequency ' + SDL.audio.freq + '!';
      }
      if (SDL.audio.channels == 0) {
        SDL.audio.channels = 1; // In SDL both 0 and 1 mean mono.
      } else if (SDL.audio.channels < 0 || SDL.audio.channels > 32) {
        throw 'Unsupported number of audio channels for SDL audio: ' + SDL.audio.channels + '!';
      } else if (SDL.audio.channels != 1 && SDL.audio.channels != 2) { // Unsure what SDL audio spec supports. Web Audio spec supports up to 32 channels.
        console.log('Warning: Using untested number of audio channels ' + SDL.audio.channels);
      }
      if (SDL.audio.samples < 128 || SDL.audio.samples > 524288 /* arbitrary cap */) {
        throw 'Unsupported audio callback buffer size ' + SDL.audio.samples + '!';
      } else if ((SDL.audio.samples & (SDL.audio.samples-1)) != 0) {
        throw 'Audio callback buffer size ' + SDL.audio.samples + ' must be a power-of-two!';
      }
      
      var totalSamples = SDL.audio.samples*SDL.audio.channels;
      SDL.audio.bytesPerSample = (SDL.audio.format == 0x0008 /*AUDIO_U8*/ || SDL.audio.format == 0x8008 /*AUDIO_S8*/) ? 1 : 2;
      SDL.audio.bufferSize = totalSamples*SDL.audio.bytesPerSample;
      SDL.audio.buffer = _malloc(SDL.audio.bufferSize);
      
      // To account for jittering in frametimes, always have multiple audio buffers queued up for the audio output device.
      // This helps that we won't starve that easily if a frame takes long to complete.
      SDL.audio.numSimultaneouslyQueuedBuffers = Module['SDL_numSimultaneouslyQueuedBuffers'] || 3;
      
      // Create a callback function that will be routinely called to ask more audio data from the user application.
      SDL.audio.caller = function SDL_audio_caller() {
        if (!SDL.audio) {
          return;
        }
        Runtime.dynCall('viii', SDL.audio.callback, [SDL.audio.userdata, SDL.audio.buffer, SDL.audio.bufferSize]);
        SDL.audio.pushAudio(SDL.audio.buffer, SDL.audio.bufferSize);
      };
      
      SDL.audio.audioOutput = new Audio();
      // As a workaround use Mozilla Audio Data API on Firefox until it ships with Web Audio and sound quality issues are fixed.
      if (typeof(SDL.audio.audioOutput['mozSetup'])==='function') {
        SDL.audio.audioOutput['mozSetup'](SDL.audio.channels, SDL.audio.freq); // use string attributes on mozOutput for closure compiler
        SDL.audio.mozBuffer = new Float32Array(totalSamples);
        SDL.audio.nextPlayTime = 0;
        SDL.audio.pushAudio = function SDL_audio_pushAudio(ptr, size) {
          --SDL.audio.numAudioTimersPending;
          var mozBuffer = SDL.audio.mozBuffer;
          // The input audio data for SDL audio is either 8-bit or 16-bit interleaved across channels, output for Mozilla Audio Data API
          // needs to be Float32 interleaved, so perform a sample conversion.
          if (SDL.audio.format == 0x8010 /*AUDIO_S16LSB*/) {
            for (var i = 0; i < totalSamples; i++) {
              mozBuffer[i] = ({{{ makeGetValue('ptr', 'i*2', 'i16', 0, 0) }}}) / 0x8000;
            }
          } else if (SDL.audio.format == 0x0008 /*AUDIO_U8*/) {
            for (var i = 0; i < totalSamples; i++) {
              var v = ({{{ makeGetValue('ptr', 'i', 'i8', 0, 0) }}});
              mozBuffer[i] = ((v >= 0) ? v-128 : v+128) /128;
            }
          }
          // Submit the audio data to audio device.
          SDL.audio.audioOutput['mozWriteAudio'](mozBuffer);
          
          // Compute when the next audio callback should be called.
          var curtime = Date.now() / 1000.0 - SDL.audio.startTime;
#if ASSERTIONS
          if (curtime > SDL.audio.nextPlayTime && SDL.audio.nextPlayTime != 0) {
            console.log('warning: Audio callback had starved sending audio by ' + (curtime - SDL.audio.nextPlayTime) + ' seconds.');
          }
#endif
          var playtime = Math.max(curtime, SDL.audio.nextPlayTime);
          var buffer_duration = SDL.audio.samples / SDL.audio.freq;
          SDL.audio.nextPlayTime = playtime + buffer_duration;
          // Schedule the next audio callback call to occur when the current one finishes.
          SDL.audio.timer = Browser.safeSetTimeout(SDL.audio.caller, 1000.0 * (playtime-curtime));
          ++SDL.audio.numAudioTimersPending;
          // And also schedule extra buffers _now_ if we have too few in queue.
          if (SDL.audio.numAudioTimersPending < SDL.audio.numSimultaneouslyQueuedBuffers) {
            ++SDL.audio.numAudioTimersPending;
            Browser.safeSetTimeout(SDL.audio.caller, 1.0);
          }
        }
      } else {
        // Initialize Web Audio API if we haven't done so yet. Note: Only initialize Web Audio context ever once on the web page,
        // since initializing multiple times fails on Chrome saying 'audio resources have been exhausted'.
        if (!SDL.audioContext) {
          if (typeof(AudioContext) === 'function') {
            SDL.audioContext = new AudioContext();
          } else if (typeof(webkitAudioContext) === 'function') {
            SDL.audioContext = new webkitAudioContext();
          } else {
            throw 'Web Audio API is not available!';
          }
        }
        SDL.audio.soundSource = new Array(); // Use an array of sound sources as a ring buffer to queue blocks of synthesized audio to Web Audio API.
        SDL.audio.nextSoundSource = 0; // Index of the next sound buffer in the ring buffer queue to play.
        SDL.audio.nextPlayTime = 0; // Time in seconds when the next audio block is due to start.
        
        // The pushAudio function with a new audio buffer whenever there is new audio data to schedule to be played back on the device.
        SDL.audio.pushAudio=function(ptr,sizeBytes) {
          try {
            --SDL.audio.numAudioTimersPending;

            var sizeSamples = sizeBytes / SDL.audio.bytesPerSample; // How many samples fit in the callback buffer?
            var sizeSamplesPerChannel = sizeSamples / SDL.audio.channels; // How many samples per a single channel fit in the cb buffer?
            if (sizeSamplesPerChannel != SDL.audio.samples) {
              throw 'Received mismatching audio buffer size!';
            }
            // Allocate new sound buffer to be played.
            var source = SDL.audioContext['createBufferSource']();
            if (SDL.audio.soundSource[SDL.audio.nextSoundSource]) {
              SDL.audio.soundSource[SDL.audio.nextSoundSource]['disconnect'](); // Explicitly disconnect old source, since we know it shouldn't be running anymore.
            }
            SDL.audio.soundSource[SDL.audio.nextSoundSource] = source;
            var soundBuffer = SDL.audioContext['createBuffer'](SDL.audio.channels,sizeSamplesPerChannel,SDL.audio.freq);
            SDL.audio.soundSource[SDL.audio.nextSoundSource]['connect'](SDL.audioContext['destination']);

            // The input audio data is interleaved across the channels, i.e. [L, R, L, R, L, R, ...] and is either 8-bit or 16-bit as
            // supported by the SDL API. The output audio wave data for Web Audio API must be in planar buffers of [-1,1]-normalized Float32 data,
            // so perform a buffer conversion for the data.
            var numChannels = SDL.audio.channels;
            for(var i = 0; i < numChannels; ++i) {
              var channelData = soundBuffer['getChannelData'](i);
              if (channelData.length != sizeSamplesPerChannel) {
                throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + sizeSamplesPerChannel + ' samples!';
              }
              if (SDL.audio.format == 0x8010 /*AUDIO_S16LSB*/) {
                for(var j = 0; j < sizeSamplesPerChannel; ++j) {
                  channelData[j] = ({{{ makeGetValue('ptr', '(j*numChannels + i)*2', 'i16', 0, 0) }}}) / 0x8000;
                }
              } else if (SDL.audio.format == 0x0008 /*AUDIO_U8*/) {
                for(var j = 0; j < sizeSamplesPerChannel; ++j) {
                  var v = ({{{ makeGetValue('ptr', 'j*numChannels + i', 'i8', 0, 0) }}});
                  channelData[j] = ((v >= 0) ? v-128 : v+128) /128;
                }
              }
            }
            // Workaround https://bugzilla.mozilla.org/show_bug.cgi?id=883675 by setting the buffer only after filling. The order is important here!
            source['buffer'] = soundBuffer;
            
            // Schedule the generated sample buffer to be played out at the correct time right after the previously scheduled
            // sample buffer has finished.
            var curtime = SDL.audioContext['currentTime'];
#if ASSERTIONS
            if (curtime > SDL.audio.nextPlayTime && SDL.audio.nextPlayTime != 0) {
              console.log('warning: Audio callback had starved sending audio by ' + (curtime - SDL.audio.nextPlayTime) + ' seconds.');
            }
#endif
            var playtime = Math.max(curtime, SDL.audio.nextPlayTime);
            SDL.audio.soundSource[SDL.audio.nextSoundSource]['start'](playtime);
            var buffer_duration = sizeSamplesPerChannel / SDL.audio.freq;
            SDL.audio.nextPlayTime = playtime + buffer_duration;
            SDL.audio.nextSoundSource = (SDL.audio.nextSoundSource + 1) % 4;
            var secsUntilNextCall = playtime-curtime;
            
            // Queue the next audio frame push to be performed when the previously queued buffer has finished playing.
            if (SDL.audio.numAudioTimersPending == 0) {
              var preemptBufferFeedMSecs = buffer_duration/2.0;
              SDL.audio.timer = Browser.safeSetTimeout(SDL.audio.caller, Math.max(0.0, 1000.0*secsUntilNextCall-preemptBufferFeedMSecs));
              ++SDL.audio.numAudioTimersPending;
            }

            // If we are risking starving, immediately queue extra buffers.
            if (secsUntilNextCall <= buffer_duration && SDL.audio.numAudioTimersPending < SDL.audio.numSimultaneouslyQueuedBuffers) {
              ++SDL.audio.numAudioTimersPending;
              Browser.safeSetTimeout(SDL.audio.caller, 1.0);
            }
          } catch(e) {
            console.log('Web Audio API error playing back audio: ' + e.toString());
          }
        }
      }

      if (obtained) {
        // Report back the initialized audio parameters.
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.freq, 'SDL.audio.freq', 'i32') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.format, 'SDL.audio.format', 'i16') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.channels, 'SDL.audio.channels', 'i8') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.silence, 'SDL.audio.silence', 'i8') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.samples, 'SDL.audio.samples', 'i16') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.callback, 'SDL.audio.callback', '*') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.userdata, 'SDL.audio.userdata', '*') }}};
      }
      SDL.allocateChannels(32);

    } catch(e) {
      console.log('Initializing SDL audio threw an exception: "' + e.toString() + '"! Continuing without audio.');
      SDL.audio = null;
      SDL.allocateChannels(0);
      if (obtained) {
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.freq, 0, 'i32') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.format, 0, 'i16') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.channels, 0, 'i8') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.silence, 0, 'i8') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.samples, 0, 'i16') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.callback, 0, '*') }}};
        {{{ makeSetValue('obtained', C_STRUCTS.SDL_AudioSpec.userdata, 0, '*') }}};
      }
    }
    if (!SDL.audio) {
      return -1;
    }
    return 0;
  },

  SDL_PauseAudio: function(pauseOn) {
    if (!SDL.audio) {
      return;
    }
    if (pauseOn) {
      if (SDL.audio.timer !== undefined) {
        clearTimeout(SDL.audio.timer);
        SDL.audio.numAudioTimersPending = 0;
        SDL.audio.timer = undefined;
      }
    } else if (!SDL.audio.timer) {
      // Start the audio playback timer callback loop.
      SDL.audio.numAudioTimersPending = 1;
      SDL.audio.timer = Browser.safeSetTimeout(SDL.audio.caller, 1);
      SDL.audio.startTime = Date.now() / 1000.0; // Only used for Mozilla Audio Data API. Not needed for Web Audio API.
    }
    SDL.audio.paused = pauseOn;
  },

  SDL_CloseAudio__deps: ['SDL_PauseAudio', 'free'],
  SDL_CloseAudio: function() {
    if (SDL.audio) {
      try{
        for(var i = 0; i < SDL.audio.soundSource.length; ++i) {
          if (!(typeof(SDL.audio.soundSource[i]==='undefined'))) {
            SDL.audio.soundSource[i].stop(0);
          }
        }
      } catch(e) {}
      SDL.audio.soundSource = null;
      _SDL_PauseAudio(1);
      _free(SDL.audio.buffer);
      SDL.audio = null;
      SDL.allocateChannels(0);
    }
  },

  SDL_LockAudio: function() {},
  SDL_UnlockAudio: function() {},

  SDL_CreateMutex: function() { return 0 },
  SDL_LockMutex: function() {},
  SDL_UnlockMutex: function() {},
  SDL_mutexP: function() { return 0 },
  SDL_mutexV: function() { return 0 },
  SDL_DestroyMutex: function() {},

  SDL_CreateCond: function() { return 0 },
  SDL_CondSignal: function() {},
  SDL_CondWait: function() {},
  SDL_DestroyCond: function() {},

  SDL_StartTextInput: function() {
    SDL.textInput = true;
  },
  SDL_StopTextInput: function() {
    SDL.textInput = false;
  },

  // SDL Mixer

  Mix_Init: function(flags) {
    if (!flags) return 0;
    return 8; /* MIX_INIT_OGG */
  },
  Mix_Quit: function(){},

  Mix_OpenAudio: function(frequency, format, channels, chunksize) {
    SDL.allocateChannels(32);
    // Just record the values for a later call to Mix_QuickLoad_RAW
    SDL.mixerFrequency = frequency;
    SDL.mixerFormat = format;
    SDL.mixerNumChannels = channels;
    SDL.mixerChunkSize = chunksize;
    return 0;
  },

  Mix_CloseAudio: 'SDL_CloseAudio',

  Mix_AllocateChannels: function(num) {
    SDL.allocateChannels(num);
    return num;
  },

  Mix_ChannelFinished: function(func) {
    SDL.channelFinished = func;
  },

  Mix_Volume: function(channel, volume) {
    if (channel == -1) {
      for (var i = 0; i < SDL.numChannels-1; i++) {
        _Mix_Volume(i, volume);
      }
      return _Mix_Volume(SDL.numChannels-1, volume);
    }
    return SDL.setGetVolume(SDL.channels[channel], volume);
  },

  Mix_SetPanning: function() {
    return 0; // error
  },

  Mix_LoadWAV_RW: function(rwopsID, freesrc) {
    var rwops = SDL.rwops[rwopsID];

    if (rwops === undefined)
      return 0;

    var filename = '';
    var audio;
    var bytes;

    if (rwops.filename !== undefined) {
      filename = PATH.resolve(rwops.filename);
      var raw = Module["preloadedAudios"][filename];
      if (!raw) {
        if (raw === null) Module.printErr('Trying to reuse preloaded audio, but freePreloadedMediaOnUse is set!');
        Runtime.warnOnce('Cannot find preloaded audio ' + filename);

        // see if we can read the file-contents from the in-memory FS
        try {
          bytes = FS.readFile(filename);
        } catch (e) {
          Module.printErr('Couldn\'t find file for: ' + filename);
          return 0;
        }
      }
      if (Module['freePreloadedMediaOnUse']) {
        Module["preloadedAudios"][filename] = null;
      }
      audio = raw;
    }
    else if (rwops.bytes !== undefined) {
      bytes = HEAPU8.subarray(rwops.bytes, rwops.bytes + rwops.count);
    }
    else {
      return 0;
    }

    // Here, we didn't find a preloaded audio but we either were passed a filepath for
    // which we loaded bytes, or we were passed some bytes
    if (audio === undefined && bytes) {
      var blob = new Blob([bytes], {type: rwops.mimetype});
      var url = URL.createObjectURL(blob);
      audio = new Audio();
      audio.src = url;
    }

    var id = SDL.audios.length;
    // Keep the loaded audio in the audio arrays, ready for playback
    SDL.audios.push({
      source: filename,
      audio: audio
    });
    return id;
  },

  Mix_QuickLoad_RAW: function(mem, len) {
    var audio = new Audio();
    // Record the number of channels and frequency for later usage
    audio.numChannels = SDL.mixerNumChannels;
    audio.frequency = SDL.mixerFrequency;
    var numSamples = len >> 1; // len is the length in bytes, and the array contains 16-bit PCM values
    var buffer = new Float32Array(numSamples);
    for (var i = 0; i < numSamples; ++i) {
      buffer[i] = ({{{ makeGetValue('mem', 'i*2', 'i16', 0, 0) }}}) / 0x8000; // hardcoded 16-bit audio, signed (TODO: reSign if not ta2?)
    }
    // FIXME: doesn't make sense to keep the audio element in the buffer
    var id = SDL.audios.length;
    SDL.audios.push({
      source: '',
      audio: audio,
      buffer: buffer
    });
    return id;
  },

  Mix_FreeChunk: function(id) {
    SDL.audios[id] = null;
  },
  Mix_ReserveChannels: function(num) {
    SDL.channelMinimumNumber = num;
  },
  Mix_PlayChannel: function(channel, id, loops) {
    // TODO: handle loops

    // Get the audio element associated with the ID
    var info = SDL.audios[id];
    if (!info) return -1;
    var audio = info.audio;
    if (!audio) return -1;

    // If the user asks us to allocate a channel automatically, get the first
    // free one.
    if (channel == -1) {
      for (var i = SDL.channelMinimumNumber; i < SDL.numChannels; i++) {
        if (!SDL.channels[i].audio) {
          channel = i;
          break;
        }
      }
      if (channel == -1) {
        Module.printErr('All ' + SDL.numChannels + ' channels in use!');
        return -1;
      }
    }
    // We clone the audio node to utilize the preloaded audio buffer, since
    // the browser has already preloaded the audio file.
    var channelInfo = SDL.channels[channel];
    channelInfo.audio = audio = audio.cloneNode(true);
    audio.numChannels = info.audio.numChannels;
    audio.frequency = info.audio.frequency;
    // TODO: handle N loops. Behavior matches Mix_PlayMusic
    audio.loop = loops != 0; 
    audio['onended'] = function SDL_audio_onended() { // TODO: cache these
      channelInfo.audio = null;
      if (SDL.channelFinished) {
        Runtime.getFuncWrapper(SDL.channelFinished, 'vi')(channel);
      }
    }
    // Either play the element, or load the dynamic data into it
    if (info.buffer) {
      var contextCtor = null;
      if (audio && ('mozSetup' in audio)) { // Audio Data API
        try {
          audio['mozSetup'](audio.numChannels, audio.frequency);
          audio["mozWriteAudio"](info.buffer);
        } catch (e) {
          // Workaround for Firefox bug 783052
          // ignore this exception!
        }
      /*
      } else if (contextCtor = (window.AudioContext || // WebAudio API
                                window.webkitAudioContext)) {
        var currentIndex = 0;
        var numChannels = parseInt(audio.numChannels);
        var context = new contextCtor();
        var source = context.createBufferSource();
        source.loop = false;
        source.buffer = context.createBuffer(numChannels, 1, audio.frequency);
        var jsNode = context.createJavaScriptNode(2048, numChannels, numChannels);
        jsNode.onaudioprocess = function jsNode_onaudioprocess(event) {
          var buffers = new Array(numChannels);
          for (var i = 0; i < numChannels; ++i) {
            buffers[i] = event.outputBuffer.getChannelData(i);
          }
          var remaining = info.buffer.length - currentIndex;
          if (remaining > 2048) {
            remaining = 2048;
          }
          for (var i = 0; i < remaining;) {
            for (var j = 0; j < numChannels; ++j) {
              buffers[j][i] = info.buffer[currentIndex + i + j] * audio.volume;
            }
            i += j;
          }
          currentIndex += remaining * numChannels;
          for (var i = remaining; i < 2048;) {
            for (var j = 0; j < numChannels; ++j) {
              buffers[j][i] = 0; // silence
            }
            i += j;
          }
        };
        source.connect(jsNode);
        jsNode.connect(context.destination);
        source.noteOn(0);
      */
      }
    } else {
      audio.play();
    }
    audio.volume = channelInfo.volume;
    return channel;
  },
  Mix_PlayChannelTimed: 'Mix_PlayChannel', // XXX ignore Timing

  Mix_FadingChannel: function(channel) {
    return 0; // MIX_NO_FADING, TODO
  },

  Mix_HaltChannel: function(channel) {
    function halt(channel) {
      var info = SDL.channels[channel];
      if (info.audio) {
        info.audio.pause();
        info.audio = null;
      }
      if (SDL.channelFinished) {
        Runtime.getFuncWrapper(SDL.channelFinished, 'vi')(channel);
      }
    }
    if (channel != -1) {
      halt(channel);
    } else {
      for (var i = 0; i < SDL.channels.length; ++i) halt(i);
    }
    return 0;
  },

  Mix_HookMusicFinished__deps: ['Mix_HaltMusic'],
  Mix_HookMusicFinished: function(func) {
    SDL.hookMusicFinished = func;
    if (SDL.music.audio) { // ensure the callback will be called, if a music is already playing
      SDL.music.audio['onended'] = _Mix_HaltMusic;
    }
  },

  Mix_VolumeMusic: function(volume) {
    return SDL.setGetVolume(SDL.music, volume);
  },

  Mix_LoadMUS_RW: 'Mix_LoadWAV_RW',
  Mix_LoadMUS__deps: ['Mix_LoadMUS_RW', 'SDL_RWFromFile', 'SDL_FreeRW'],
  Mix_LoadMUS: function(filename) {
    var rwops = _SDL_RWFromFile(filename);
    var result = _Mix_LoadMUS_RW(rwops);
    _SDL_FreeRW(rwops);
    return result;
  },

  Mix_FreeMusic: 'Mix_FreeChunk',

  Mix_PlayMusic__deps: ['Mix_HaltMusic'],
  Mix_PlayMusic: function(id, loops) {
    loops = Math.max(loops, 1);
    var audio = SDL.audios[id].audio;
    if (!audio) return 0;
    audio.loop = loops != 0; // TODO: handle N loops for finite N
    if (SDL.audios[id].buffer) {
      audio["mozWriteAudio"](SDL.audios[id].buffer);
    } else {
      audio.play();
    }
    audio.volume = SDL.music.volume;
    audio['onended'] = _Mix_HaltMusic; // will send callback
    if (SDL.music.audio) {
      if (!SDL.music.audio.paused) {
        Module.printErr('Music is already playing. ' + SDL.music.source);
      }
      SDL.music.audio.pause();
    }
    SDL.music.audio = audio;
    return 0;
  },

  Mix_PauseMusic: function() {
    var audio = SDL.music.audio;
    if (!audio) return 0;
    audio.pause();
    return 0;
  },

  Mix_ResumeMusic: function() {
    var audio = SDL.music.audio;
    if (!audio) return 0;
    audio.play();
    return 0;
  },

  Mix_HaltMusic: function() {
    var audio = SDL.music.audio;
    if (!audio) return 0;
    audio.src = audio.src; // rewind
    audio.pause();
    SDL.music.audio = null;
    if (SDL.hookMusicFinished) {
      Runtime.dynCall('v', SDL.hookMusicFinished);
    }
    return 0;
  },

  Mix_FadeInMusicPos: 'Mix_PlayMusic', // XXX ignore fading in effect

  Mix_FadeOutMusic: 'Mix_HaltMusic', // XXX ignore fading out effect

  Mix_PlayingMusic: function() {
    return (SDL.music.audio && !SDL.music.audio.paused) ? 1 : 0;
  },

  // http://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_38.html#SEC38
  // "Note: Does not check if the channel has been paused."
  Mix_Playing: function(channel) {
    if (channel === -1) {
      var count = 0;
      for (var i = 0; i < SDL.channels.length; i++) {
        count += _Mix_Playing(i);
      }
      return count;
    }
    var info = SDL.channels[channel];
    if (info && info.audio && !info.audio.paused) {
      return 1;
    }
    return 0;
  },
  
  Mix_Pause: function(channel) {
    if (channel === -1) {
      for (var i = 0; i<SDL.channels.length;i++) {
        _Mix_Pause(i);
      }
      return;
    }
    var info = SDL.channels[channel];
    if (info && info.audio) {
      info.audio.pause();
    } else {
      Module.printErr('Mix_Pause: no sound found for channel: ' + channel);
    }
  },
  
  // http://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_39.html#SEC39
  Mix_Paused: function(channel) {
    if (channel === -1) {
      var pausedCount = 0;
      for (var i = 0; i<SDL.channels.length;i++) {
        pausedCount += _Mix_Paused(i);
      }
      return pausedCount;
    }
    var info = SDL.channels[channel];
    if (info && info.audio && info.audio.paused) {
      return 1;
    }
    return 0;
  },

  Mix_PausedMusic: function() {
    return (SDL.music.audio && SDL.music.audio.paused) ? 1 : 0;
  },

  // http://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_33.html#SEC33
  Mix_Resume: function(channel) {
    if (channel === -1) {
      for (var i = 0; i<SDL.channels.length;i++) {
        _Mix_Resume(i);
      }
      return;
    }
    var info = SDL.channels[channel];
    if (info && info.audio) {
      info.audio.play();
    }
  },

  // SDL TTF

  TTF_Init: function() {
    var canvas = document.createElement('canvas');
    SDL.ttfContext = canvas.getContext('2d');
    return 0;
  },

  TTF_OpenFont: function(filename, size) {
    filename = FS.standardizePath(Pointer_stringify(filename));
    var id = SDL.fonts.length;
    SDL.fonts.push({
      name: filename, // but we don't actually do anything with it..
      size: size
    });
    return id;
  },

  TTF_CloseFont: function(font) {
    SDL.fonts[font] = null;
  },

  TTF_RenderText_Solid: function(font, text, color) {
    // XXX the font and color are ignored
    text = Pointer_stringify(text) || ' '; // if given an empty string, still return a valid surface
    var fontData = SDL.fonts[font];
    var w = SDL.estimateTextWidth(fontData, text);
    var h = fontData.size;
    var color = SDL.loadColorToCSSRGB(color); // XXX alpha breaks fonts?
    var fontString = h + 'px ' + fontData.name;
    var surf = SDL.makeSurface(w, h, 0, false, 'text:' + text); // bogus numbers..
    var surfData = SDL.surfaces[surf];
    surfData.ctx.save();
    surfData.ctx.fillStyle = color;
    surfData.ctx.font = fontString;
    surfData.ctx.textBaseline = 'top';
    surfData.ctx.fillText(text, 0, 0);
    surfData.ctx.restore();
    return surf;
  },
  TTF_RenderText_Blended: 'TTF_RenderText_Solid', // XXX ignore blending vs. solid
  TTF_RenderText_Shaded: 'TTF_RenderText_Solid', // XXX ignore blending vs. solid
  TTF_RenderUTF8_Solid: 'TTF_RenderText_Solid',

  TTF_SizeText: function(font, text, w, h) {
    var fontData = SDL.fonts[font];
    if (w) {
      {{{ makeSetValue('w', '0', 'SDL.estimateTextWidth(fontData, Pointer_stringify(text))', 'i32') }}};
    }
    if (h) {
      {{{ makeSetValue('h', '0', 'fontData.size', 'i32') }}};
    }
    return 0;
  },

  TTF_FontAscent: function(font) {
    var fontData = SDL.fonts[font];
    return Math.floor(fontData.size*0.98); // XXX
  },

  TTF_FontDescent: function(font) {
    var fontData = SDL.fonts[font];
    return Math.floor(fontData.size*0.02); // XXX
  },

  TTF_FontHeight: function(font) {
    var fontData = SDL.fonts[font];
    return fontData.size;
  },

  // SDL gfx

  $SDL_gfx: {
    drawRectangle: function(surf, x1, y1, x2, y2, action, cssColor) {
      x1 = x1 << 16 >> 16;
      y1 = y1 << 16 >> 16;
      x2 = x2 << 16 >> 16;
      y2 = y2 << 16 >> 16;
      var surfData = SDL.surfaces[surf];
      assert(!surfData.locked); // but we could unlock and re-lock if we must..
      // TODO: if ctx does not change, leave as is, and also do not re-set xStyle etc.
      var x = x1 < x2 ? x1 : x2;
      var y = y1 < y2 ? y1 : y2;
      var w = Math.abs(x2 - x1);
      var h = Math.abs(y2 - y1);
      surfData.ctx.save();
      surfData.ctx[action + 'Style'] = cssColor;
      surfData.ctx[action + 'Rect'](x, y, w, h);
      surfData.ctx.restore();
    },
    drawLine: function(surf, x1, y1, x2, y2, cssColor) {
      x1 = x1 << 16 >> 16;
      y1 = y1 << 16 >> 16;
      x2 = x2 << 16 >> 16;
      y2 = y2 << 16 >> 16;
      var surfData = SDL.surfaces[surf];
      assert(!surfData.locked); // but we could unlock and re-lock if we must..
      surfData.ctx.save();
      surfData.ctx.strokeStyle = cssColor;
      surfData.ctx.beginPath();
      surfData.ctx.moveTo(x1, y1);
      surfData.ctx.lineTo(x2, y2);
      surfData.ctx.stroke();
      surfData.ctx.restore();
    },
    // See http://stackoverflow.com/questions/2172798/how-to-draw-an-oval-in-html5-canvas
    drawEllipse: function(surf, x, y, rx, ry, action, cssColor) {
      x = x << 16 >> 16;
      y = y << 16 >> 16;
      rx = rx << 16 >> 16;
      ry = ry << 16 >> 16;
      var surfData = SDL.surfaces[surf];
      assert(!surfData.locked); // but we could unlock and re-lock if we must..

      surfData.ctx.save();
      surfData.ctx.beginPath();
      surfData.ctx.translate(x, y);
      surfData.ctx.scale(rx, ry);
      surfData.ctx.arc(0, 0, 1, 0, 2 * Math.PI);
      surfData.ctx.restore();

      surfData.ctx.save();
      surfData.ctx[action + 'Style'] = cssColor;
      surfData.ctx[action]();
      surfData.ctx.restore();
    },
    // the gfx library uses something different from the rest of SDL...
    translateColorToCSSRGBA: function(rgba) {
      return 'rgba(' + (rgba>>>24) + ',' + (rgba>>16 & 0xff) + ',' + (rgba>>8 & 0xff) + ',' + (rgba&0xff) + ')';
    }
  },

  boxColor__deps: ['$SDL_gfx'],
  boxColor: function(surf, x1, y1, x2, y2, color) {
    return SDL_gfx.drawRectangle(surf, x1, y1, x2, y2, 'fill', SDL_gfx.translateColorToCSSRGBA(color));
  },

  boxRGBA__deps: ['$SDL_gfx'],
  boxRGBA: function(surf, x1, y1, x2, y2, r, g, b, a) {
    return SDL_gfx.drawRectangle(surf, x1, y1, x2, y2, 'fill', SDL.translateRGBAToCSSRGBA(r, g, b, a));
  },

  rectangleColor__deps: ['$SDL_gfx'],
  rectangleColor: function(surf, x1, y1, x2, y2, color) {
    return SDL_gfx.drawRectangle(surf, x1, y1, x2, y2, 'stroke', SDL_gfx.translateColorToCSSRGBA(color));
  },

  rectangleRGBA__deps: ['$SDL_gfx'],
  rectangleRGBA: function(surf, x1, y1, x2, y2, r, g, b, a) {
    return SDL_gfx.drawRectangle(surf, x1, y1, x2, y2, 'stroke', SDL.translateRGBAToCSSRGBA(r, g, b, a));
  },

  ellipseColor__deps: ['$SDL_gfx'],
  ellipseColor: function(surf, x, y, rx, ry, color) {
    return SDL_gfx.drawEllipse(surf, x, y, rx, ry, 'stroke', SDL_gfx.translateColorToCSSRGBA(color));
  },

  ellipseRGBA__deps: ['$SDL_gfx'],
  ellipseRGBA: function(surf, x, y, rx, ry, r, g, b, a) {
    return SDL_gfx.drawEllipse(surf, x, y, rx, ry, 'stroke', SDL.translateRGBAToCSSRGBA(r, g, b, a));
  },

  filledEllipseColor__deps: ['$SDL_gfx'],
  filledEllipseColor: function(surf, x, y, rx, ry, color) {
    return SDL_gfx.drawEllipse(surf, x, y, rx, ry, 'fill', SDL_gfx.translateColorToCSSRGBA(color));
  },

  filledEllipseRGBA__deps: ['$SDL_gfx'],
  filledEllipseRGBA: function(surf, x, y, rx, ry, r, g, b, a) {
    return SDL_gfx.drawEllipse(surf, x, y, rx, ry, 'fill', SDL.translateRGBAToCSSRGBA(r, g, b, a));
  },

  lineColor__deps: ['$SDL_gfx'],
  lineColor: function(surf, x1, y1, x2, y2, color) {
    return SDL_gfx.drawLine(surf, x1, y1, x2, y2, SDL_gfx.translateColorToCSSRGBA(color));
  },

  lineRGBA__deps: ['$SDL_gfx'],
  lineRGBA: function(surf, x1, y1, x2, y2, r, g, b, a) {
    return SDL_gfx.drawLine(surf, x1, y1, x2, y2, SDL.translateRGBAToCSSRGBA(r, g, b, a));
  },

  pixelRGBA__deps: ['boxRGBA'],
  pixelRGBA: function(surf, x1, y1, r, g, b, a) {
    // This cannot be fast, to render many pixels this way!
    _boxRGBA(surf, x1, y1, x1, y1, r, g, b, a);
  },

  // GL

  SDL_GL_SetAttribute: function(attr, value) {
    if (!(attr in SDL.glAttributes)) {
      abort('Unknown SDL GL attribute (' + attr + '). Please check if your SDL version is supported.');
    }

    SDL.glAttributes[attr] = value;
  },

  SDL_GL_GetAttribute: function(attr, value) {
    if (!(attr in SDL.glAttributes)) {
      abort('Unknown SDL GL attribute (' + attr + '). Please check if your SDL version is supported.');
    }

    if (value) {{{ makeSetValue('value', '0', 'SDL.glAttributes[attr]', 'i32') }}};

    return 0;
  },

  SDL_GL_GetProcAddress__deps: ['emscripten_GetProcAddress'],
  SDL_GL_GetProcAddress: function(name_) {
    return _emscripten_GetProcAddress(Pointer_stringify(name_));
  },

  SDL_GL_SwapBuffers: function() {},

  // SDL 2

  SDL_GL_ExtensionSupported: function(extension) {
    return Module.ctx.getExtension(extension) | 0;
  },

  SDL_DestroyWindow: function(window) {},

  SDL_DestroyRenderer: function(renderer) {},

  SDL_GetWindowFlags: function(x, y) {
    if (Browser.isFullScreen) {
       return 1;
    }

    return 0;
  },

  SDL_GL_SwapWindow: function(window) {},

  SDL_GL_MakeCurrent: function(window, context) {},

  SDL_GL_DeleteContext: function(context) {},

  SDL_GL_SetSwapInterval: function(state) {},

  SDL_SetWindowTitle: function(window, title) {
    if (title) document.title = Pointer_stringify(title);
  },

  SDL_GetWindowSize: function(window, width, height){
    var w = Module['canvas'].width;
    var h = Module['canvas'].height;
    if (width) {{{ makeSetValue('width', '0', 'w', 'i32') }}};
    if (height) {{{ makeSetValue('height', '0', 'h', 'i32') }}};
  },

  SDL_LogSetOutputFunction: function(callback, userdata) {},

  SDL_SetWindowFullscreen: function(window, fullscreen) {
    if (Browser.isFullScreen) {
      Module['canvas'].cancelFullScreen();
      return 1;
    } else {
      return 0;
    }
  },

  SDL_GetWindowFlags: function() {},

  SDL_ClearError: function() {},

  SDL_getenv: 'getenv',

  // TODO

  SDL_SetGamma: function(r, g, b) {
    return -1;
  },

  SDL_SetGammaRamp: function(redTable, greenTable, blueTable) {
    return -1;
  },

  // Joysticks

  SDL_NumJoysticks: function() {
    var count = 0;
    var gamepads = SDL.getGamepads();
    // The length is not the number of gamepads; check which ones are defined.
    for (var i = 0; i < gamepads.length; i++) {
      if (gamepads[i] !== undefined) count++;
    }
    return count;
  },

  SDL_JoystickName: function(deviceIndex) {
    var gamepad = SDL.getGamepad(deviceIndex);
    if (gamepad) {
      var name = gamepad.id;
      if (SDL.joystickNamePool.hasOwnProperty(name)) {
        return SDL.joystickNamePool[name];
      }
      return SDL.joystickNamePool[name] = allocate(intArrayFromString(name), 'i8', ALLOC_NORMAL);
    }
    return 0;
  },

  SDL_JoystickOpen: function(deviceIndex) {
    var gamepad = SDL.getGamepad(deviceIndex);
    if (gamepad) {
      // Use this as a unique 'pointer' for this joystick.
      var joystick = deviceIndex+1;
      SDL.recordJoystickState(joystick, gamepad);
      return joystick;
    }
    return 0;
  },

  SDL_JoystickOpened: function(deviceIndex) {
    return SDL.lastJoystickState.hasOwnProperty(deviceIndex+1) ? 1 : 0;
  },

  SDL_JoystickIndex: function(joystick) {
    // joystick pointers are simply the deviceIndex+1.
    return joystick - 1;
  },

  SDL_JoystickNumAxes: function(joystick) {
    var gamepad = SDL.getGamepad(joystick - 1);
    if (gamepad) {
      return gamepad.axes.length;
    }
    return 0;
  },

  SDL_JoystickNumBalls: function(joystick) { return 0; },

  SDL_JoystickNumHats: function(joystick) { return 0; },

  SDL_JoystickNumButtons: function(joystick) {
    var gamepad = SDL.getGamepad(joystick - 1);
    if (gamepad) {
      return gamepad.buttons.length;
    }
    return 0;
  },

  SDL_JoystickUpdate: function() {
    SDL.queryJoysticks();
  },

  SDL_JoystickEventState: function(state) {
    if (state < 0) {
      // SDL_QUERY: Return current state.
      return SDL.joystickEventState;
    }
    return SDL.joystickEventState = state;
  },

  SDL_JoystickGetAxis: function(joystick, axis) {
    var gamepad = SDL.getGamepad(joystick - 1);
    if (gamepad && gamepad.axes.length > axis) {
      return SDL.joystickAxisValueConversion(gamepad.axes[axis]);
    }
    return 0;
  },

  SDL_JoystickGetHat: function(joystick, hat) { return 0; },

  SDL_JoystickGetBall: function(joystick, ball, dxptr, dyptr) { return -1; },

  SDL_JoystickGetButton: function(joystick, button) {
    var gamepad = SDL.getGamepad(joystick - 1);
    if (gamepad && gamepad.buttons.length > button) {
      return SDL.getJoystickButtonState(gamepad.buttons[button]) ? 1 : 0;
    }
    return 0;
  },

  SDL_JoystickClose: function(joystick) {
    delete SDL.lastJoystickState[joystick];
  },

  // Misc

  SDL_InitSubSystem: function(flags) { return 0 },

  SDL_RWFromConstMem: function(mem, size) {
    var id = SDL.rwops.length; // TODO: recycle ids when they are null
    SDL.rwops.push({ bytes: mem, count: size });
    return id;
  },
  SDL_RWFromMem: 'SDL_RWFromConstMem',

  SDL_RWFromFile: function(_name, mode) {
    var id = SDL.rwops.length; // TODO: recycle ids when they are null
    var name = Pointer_stringify(_name)
    SDL.rwops.push({ filename: name, mimetype: Browser.getMimetype(name) });
    return id;
  },
  
  SDL_FreeRW: function(rwopsID) {
    SDL.rwops[rwopsID] = null;
    while (SDL.rwops.length > 0 && SDL.rwops[SDL.rwops.length-1] === null) {
      SDL.rwops.pop();
    }
  },

  SDL_EnableUNICODE: function(on) {
    var ret = SDL.unicode || 0;
    SDL.unicode = on;
    return ret;
  },

  SDL_AddTimer: function(interval, callback, param) {
    return window.setTimeout(function() {
      Runtime.dynCall('iii', callback, [interval, param]);
    }, interval);
  },
  SDL_RemoveTimer: function(id) {
    window.clearTimeout(id);
    return true;
  },

  SDL_CreateThread: function() {
    throw 'SDL threads cannot be supported in the web platform because they assume shared state. See emscripten_create_worker etc. for a message-passing concurrency model that does let you run code in another thread.'
  },

  SDL_WaitThread: function() { throw 'SDL_WaitThread' },
  SDL_GetThreadID: function() { throw 'SDL_GetThreadID' },
  SDL_ThreadID: function() { return 0; },
  SDL_AllocRW: function() { throw 'SDL_AllocRW: TODO' },
  SDL_CondBroadcast: function() { throw 'SDL_CondBroadcast: TODO' },
  SDL_CondWaitTimeout: function() { throw 'SDL_CondWaitTimeout: TODO' },
  SDL_WM_IconifyWindow: function() { throw 'SDL_WM_IconifyWindow TODO' },

  Mix_SetPostMix: function() { Runtime.warnOnce('Mix_SetPostMix: TODO') },
  Mix_QuerySpec: function() { throw 'Mix_QuerySpec: TODO' },
  Mix_FadeInChannelTimed: function() { throw 'Mix_FadeInChannelTimed' },
  Mix_FadeOutChannel: function() { throw 'Mix_FadeOutChannel' },

  Mix_Linked_Version: function() { throw 'Mix_Linked_Version: TODO' },
  SDL_SaveBMP_RW: function() { throw 'SDL_SaveBMP_RW: TODO' },

  SDL_WM_SetIcon: function() { /* This function would set the application window icon surface, which doesn't apply for web canvases, so a no-op. */ },
  SDL_HasRDTSC: function() { return 0; },
  SDL_HasMMX: function() { return 0; },
  SDL_HasMMXExt: function() { return 0; },
  SDL_Has3DNow: function() { return 0; },
  SDL_Has3DNowExt: function() { return 0; },
  SDL_HasSSE: function() { return 0; },
  SDL_HasSSE2: function() { return 0; },
  SDL_HasAltiVec: function() { return 0; }
};

autoAddDeps(LibrarySDL, '$SDL');
mergeInto(LibraryManager.library, LibrarySDL);

