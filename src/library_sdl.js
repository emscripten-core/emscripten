//"use strict";

// See browser tests for examples (tests/runner.py, search for sdl_). Run with
//    python tests/runner.py browser

// Notes:
//  SDL_VIDEORESIZE: This is sent when the canvas is resized. Note that the user
//                   cannot manually do so, so this is only sent when the
//                   program manually resizes it (emscripten_set_canvas_size
//                   or otherwise).

var LibrarySDL = {
  $SDL__deps: ['$FS', '$Browser'],
  $SDL: {
    defaults: {
      width: 320,
      height: 200,
      copyOnLock: true
    },

    version: null,

    surfaces: {},
    events: [],
    fonts: [null],

    // The currently preloaded audio elements ready to be played
    audios: [null],
    // The currently playing audio element.  There's only one music track.
    music: {
      audio: null,
      volume: 1.0
    },
    mixerFrequency: 22050,
    mixerFormat: 0x8010, // AUDIO_S16LSB
    mixerNumChannels: 2,
    mixerChunkSize: 1024,
    channelMinimumNumber: 0,

    GL: false, // Set to true if we call SDL_SetVideoMode with SDL_OPENGL, and if so, we do not create 2D canvases&contexts for blitting
               // Note that images loaded before SDL_SetVideoMode will not get this optimization

    keyboardState: null,
    keyboardMap: {},

    textInput: false,

    startTime: null,
    buttonState: 0,
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
      44: 54, // comma
      46: 55, // period
      47: 56, // slash
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
      9: 43, // tab
      27: 41, // escape
      32: 44, // space
      92: 49, // backslash
      305: 224, // ctrl
      308: 226, // alt
    },

    structs: {
      Rect: Runtime.generateStructInfo([
        ['i32', 'x'], ['i32', 'y'], ['i32', 'w'], ['i32', 'h'], 
      ]),
      PixelFormat: Runtime.generateStructInfo([
        ['i32', 'format'],
        ['void*', 'palette'], ['i8', 'BitsPerPixel'], ['i8', 'BytesPerPixel'],
        ['i8', 'padding1'], ['i8', 'padding2'],
        ['i32', 'Rmask'], ['i32', 'Gmask'], ['i32', 'Bmask'], ['i32', 'Amask'],
        ['i8', 'Rloss'], ['i8', 'Gloss'], ['i8', 'Bloss'], ['i8', 'Aloss'],
        ['i8', 'Rshift'], ['i8', 'Gshift'], ['i8', 'Bshift'], ['i8', 'Ashift']
      ]),
      KeyboardEvent: Runtime.generateStructInfo([
        ['i32', 'type'],
        ['i32', 'windowID'],
        ['i8', 'state'],
        ['i8', 'repeat'],
        ['i8', 'padding2'],
        ['i8', 'padding3'],
        ['i32', 'keysym']
      ]),
      keysym: Runtime.generateStructInfo([
        ['i32', 'scancode'],
        ['i32', 'sym'],
        ['i16', 'mod'],
        ['i32', 'unicode']
      ]),
      TextInputEvent: Runtime.generateStructInfo([
        ['i32', 'type'],
        ['i32', 'windowID'],
        ['b256', 'text'],
      ]),
      MouseMotionEvent: Runtime.generateStructInfo([
        ['i32', 'type'],
        ['i32', 'windowID'],
        ['i8', 'state'],
        ['i8', 'padding1'],
        ['i8', 'padding2'],
        ['i8', 'padding3'],
        ['i32', 'x'],
        ['i32', 'y'],
        ['i32', 'xrel'],
        ['i32', 'yrel']
      ]),
      MouseButtonEvent: Runtime.generateStructInfo([
        ['i32', 'type'],
        ['i32', 'windowID'],
        ['i8', 'button'],
        ['i8', 'state'],
        ['i8', 'padding1'],
        ['i8', 'padding2'],
        ['i32', 'x'],
        ['i32', 'y']
      ]),
      ResizeEvent: Runtime.generateStructInfo([
        ['i32', 'type'],
        ['i32', 'w'],
        ['i32', 'h']
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
      ]),
      version: Runtime.generateStructInfo([
        ['i8', 'major'],
        ['i8', 'minor'],
        ['i8', 'patch']
      ])
    },

    loadRect: function(rect) {
      return {
        x: {{{ makeGetValue('rect + SDL.structs.Rect.x', '0', 'i32') }}},
        y: {{{ makeGetValue('rect + SDL.structs.Rect.y', '0', 'i32') }}},
        w: {{{ makeGetValue('rect + SDL.structs.Rect.w', '0', 'i32') }}},
        h: {{{ makeGetValue('rect + SDL.structs.Rect.h', '0', 'i32') }}}
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
      return 'rgba(' + ((rgba >> 24)&255) + ',' + ((rgba >> 16)&255) + ',' + ((rgba >> 8)&255) + ',' + ((rgba&255)/255) + ')';
    },

    translateRGBAToCSSRGBA: function(r, g, b, a) {
      return 'rgba(' + r + ',' + g + ',' + b + ',' + (a/255) + ')';
    },

    translateRGBAToColor: function(r, g, b, a) {
      return (r << 24) + (g << 16) + (b << 8) + a;
    },

    makeSurface: function(width, height, flags, usePageCanvas, source, rmask, gmask, bmask, amask) {
      flags = flags || 0;
      var surf = _malloc(14*Runtime.QUANTUM_SIZE);  // SDL_Surface has 14 fields of quantum size
      var buffer = _malloc(width*height*4); // TODO: only allocate when locked the first time
      var pixelFormat = _malloc(18*Runtime.QUANTUM_SIZE);
      flags |= 1; // SDL_HWSURFACE - this tells SDL_MUSTLOCK that this needs to be locked

      //surface with SDL_HWPALETTE flag is 8bpp surface (1 byte)
      var is_SDL_HWPALETTE = flags & 0x00200000;  
      var bpp = is_SDL_HWPALETTE ? 1 : 4;
 
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*0', '0', 'flags', 'i32') }}}         // SDL_Surface.flags
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*1', '0', 'pixelFormat', 'void*') }}} // SDL_Surface.format TODO
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*2', '0', 'width', 'i32') }}}         // SDL_Surface.w
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*3', '0', 'height', 'i32') }}}        // SDL_Surface.h
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*4', '0', 'width * bpp', 'i32') }}}       // SDL_Surface.pitch, assuming RGBA or indexed for now,
                                                                               // since that is what ImageData gives us in browsers
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*5', '0', 'buffer', 'void*') }}}      // SDL_Surface.pixels
      {{{ makeSetValue('surf+Runtime.QUANTUM_SIZE*6', '0', '0', 'i32*') }}}      // SDL_Surface.offset

      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.format', '0', '-2042224636', 'i32') }}} // SDL_PIXELFORMAT_RGBA8888
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.palette', '0', '0', 'i32') }}} // TODO
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.BitsPerPixel', '0', 'bpp * 8', 'i8') }}}
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.BytesPerPixel', '0', 'bpp', 'i8') }}}

      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.Rmask', '0', 'rmask || 0x000000ff', 'i32') }}}
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.Gmask', '0', 'gmask || 0x0000ff00', 'i32') }}}
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.Bmask', '0', 'bmask || 0x00ff0000', 'i32') }}}
      {{{ makeSetValue('pixelFormat + SDL.structs.PixelFormat.Amask', '0', 'amask || 0xff000000', 'i32') }}}

      // Decide if we want to use WebGL or not
      var useWebGL = (flags & 0x04000000) != 0; // SDL_OPENGL
      SDL.GL = SDL.GL || useWebGL;
      var canvas;
      if (!usePageCanvas) {
        canvas = document.createElement('canvas');
        canvas.width = width;
        canvas.height = height;
      } else {
        canvas = Module['canvas'];
      }
      var ctx = Browser.createContext(canvas, useWebGL, usePageCanvas);
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

        isFlagSet: function (flag) {
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
      _free(SDL.surfaces[surf].buffer);
      _free(SDL.surfaces[surf].pixelFormat);
      _free(surf);
      SDL.surfaces[surf] = null;
    },

    receiveEvent: function(event) {
      switch(event.type) {
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
              return false;
            }
          }
          // fall through
        case 'keydown': case 'keyup': case 'keypress': case 'mousedown': case 'mouseup': case 'DOMMouseScroll': case 'mousewheel':
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
            if (!SDL.DOMButtons[event.button]) return false; // ignore extra ups, can happen if we leave the canvas while pressing down, then return,
                                                             // since we add a mouseup in that case
            SDL.DOMButtons[event.button] = 0;
          }

          if (event.type == 'keypress' && !SDL.textInput) {
            break;
          }

          SDL.events.push(event);
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
          break;
        }
        case 'unload':
          if (Browser.mainLoop.runner) {
            SDL.events.push(event);
            // Force-run a main event loop, since otherwise this event will never be caught!
            Browser.mainLoop.runner();
          }
          return true;
        case 'resize':
          SDL.events.push(event);
          break;
      }
      if (SDL.events.length >= 10000) {
        Module.printErr('SDL event queue full, dropping events');
        SDL.events = SDL.events.slice(0, 10000);
      }
      return false;
    },

    makeCEvent: function(event, ptr) {
      if (typeof event === 'number') {
        // This is a pointer to a native C event that was SDL_PushEvent'ed
        _memcpy(ptr, event, SDL.structs.KeyboardEvent.__size__); // XXX
        return;
      }

      switch(event.type) {
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
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.type', 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}}
          //{{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.which', '1', 'i32') }}}
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.state', 'down ? 1 : 0', 'i8') }}}
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.repeat', '0', 'i8') }}} // TODO

          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.keysym + SDL.structs.keysym.scancode', 'scan', 'i32') }}}
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.keysym + SDL.structs.keysym.sym', 'key', 'i32') }}}
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.keysym + SDL.structs.keysym.mod', '0', 'i32') }}}
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.keysym + SDL.structs.keysym.unicode', 'key', 'i32') }}}

          var code = SDL.keyCodes[event.keyCode] || event.keyCode;
          {{{ makeSetValue('SDL.keyboardState', 'code', 'down', 'i8') }}};
          if (down) {
            SDL.keyboardMap[code] = event.keyCode; // save the DOM input, which we can use to unpress it during blur
          } else {
            delete SDL.keyboardMap[code];
          }

          break;
        }
        case 'keypress': {
          {{{ makeSetValue('ptr', 'SDL.structs.TextInputEvent.type', 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}}
          // Not filling in windowID for now
          var cStr = intArrayFromString(String.fromCharCode(event.charCode));
          for (var i = 0; i < cStr.length; ++i) {
            {{{ makeSetValue('ptr', 'SDL.structs.TextInputEvent.text + i', 'cStr[i]', 'i8') }}};
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
          if (event.type != 'mousemove') {
            var down = event.type === 'mousedown';
            {{{ makeSetValue('ptr', 'SDL.structs.MouseButtonEvent.type', 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
            {{{ makeSetValue('ptr', 'SDL.structs.MouseButtonEvent.button', 'event.button+1', 'i8') }}}; // DOM buttons are 0-2, SDL 1-3
            {{{ makeSetValue('ptr', 'SDL.structs.MouseButtonEvent.state', 'down ? 1 : 0', 'i8') }}};
            {{{ makeSetValue('ptr', 'SDL.structs.MouseButtonEvent.x', 'Browser.mouseX', 'i32') }}};
            {{{ makeSetValue('ptr', 'SDL.structs.MouseButtonEvent.y', 'Browser.mouseY', 'i32') }}};
          } else {
            {{{ makeSetValue('ptr', 'SDL.structs.MouseMotionEvent.type', 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
            {{{ makeSetValue('ptr', 'SDL.structs.MouseMotionEvent.state', 'SDL.buttonState', 'i8') }}};
            {{{ makeSetValue('ptr', 'SDL.structs.MouseMotionEvent.x', 'Browser.mouseX', 'i32') }}};
            {{{ makeSetValue('ptr', 'SDL.structs.MouseMotionEvent.y', 'Browser.mouseY', 'i32') }}};
            {{{ makeSetValue('ptr', 'SDL.structs.MouseMotionEvent.xrel', 'Browser.mouseMovementX', 'i32') }}};
            {{{ makeSetValue('ptr', 'SDL.structs.MouseMotionEvent.yrel', 'Browser.mouseMovementY', 'i32') }}};
          }
          break;
        }
        case 'unload': {
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.type', 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          break;
        }
        case 'resize': {
          {{{ makeSetValue('ptr', 'SDL.structs.KeyboardEvent.type', 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          {{{ makeSetValue('ptr', 'SDL.structs.ResizeEvent.w', 'event.w', 'i32') }}};
          {{{ makeSetValue('ptr', 'SDL.structs.ResizeEvent.h', 'event.h', 'i32') }}};
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
      if (SDL.numChannels && SDL.numChannels >= num) return;
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
    }
  },

  SDL_Linked_Version: function() {
    if (SDL.version === null) {
      SDL.version = _malloc(SDL.structs.version.__size__);
      {{{ makeSetValue('SDL.version + SDL.structs.version.major', '0', '1', 'i8') }}}
      {{{ makeSetValue('SDL.version + SDL.structs.version.minor', '0', '3', 'i8') }}}
      {{{ makeSetValue('SDL.version + SDL.structs.version.patch', '0', '0', 'i8') }}}
    }
    return SDL.version;
  },

  SDL_Init: function(what) {
    SDL.startTime = Date.now();
    // capture all key events. we just keep down and up, but also capture press to prevent default actions
    if (!Module['doNotCaptureKeyboard']) {
      document.onkeydown = SDL.receiveEvent;
      document.onkeyup = SDL.receiveEvent;
      document.onkeypress = SDL.receiveEvent;
      document.onblur = SDL.receiveEvent;
      document.addEventListener("visibilitychange", SDL.receiveEvent);
    }
    window.onunload = SDL.receiveEvent;
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
    Browser.setCanvasSize(width, height, true);
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

    // Mark in C/C++-accessible SDL structure
    // SDL_Surface has the following fields: Uint32 flags, SDL_PixelFormat *format; int w, h; Uint16 pitch; void *pixels; ...
    // So we have fields all of the same size, and 5 of them before us.
    // TODO: Use macros like in library.js
    {{{ makeSetValue('surf', '5*Runtime.QUANTUM_SIZE', 'surfData.buffer', 'void*') }}};

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
      var num = surfData.image.data.length;
      var data = surfData.image.data;
      var buffer = surfData.buffer;
#if USE_TYPED_ARRAYS == 2
      assert(buffer % 4 == 0, 'Invalid buffer offset: ' + buffer);
      var src = buffer >> 2;
      var dst = 0;
      var isScreen = surf == SDL.screen;
      while (dst < num) {
        // TODO: access underlying data buffer and write in 32-bit chunks or more
        var val = HEAP32[src]; // This is optimized. Instead, we could do {{{ makeGetValue('buffer', 'dst', 'i32') }}};
        data[dst  ] = val & 0xff;
        data[dst+1] = (val >> 8) & 0xff;
        data[dst+2] = (val >> 16) & 0xff;
        data[dst+3] = isScreen ? 0xff : ((val >> 24) & 0xff);
        src++;
        dst += 4;
      }
#else
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
    throw 'SDL_Delay called! Potential infinite loop, quitting. ' + new Error().stack;
  },

  SDL_WM_SetCaption: function(title, icon) {
    title = title && Pointer_stringify(title);
    icon = icon && Pointer_stringify(icon);
  },

  SDL_EnableKeyRepeat: function(delay, interval) {
    // TODO
  },

  SDL_GetKeyboardState: function() {
    return SDL.keyboardState;
  },

  SDL_GetKeyState__deps: ['SDL_GetKeyboardState'],
  SDL_GetKeyState: function() {
    return _SDL_GetKeyboardState();
  },

  SDL_GetModState: function() {
    // TODO: numlock, capslock, etc.
    return (SDL.keyboardState[16] ? 0x0001 | 0x0002 : 0) | // KMOD_LSHIFT & KMOD_RSHIFT
           (SDL.keyboardState[17] ? 0x0040 | 0x0080 : 0) | // KMOD_LCTRL & KMOD_RCTRL
           (SDL.keyboardState[18] ? 0x0100 | 0x0200 : 0); // KMOD_LALT & KMOD_RALT
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
    return allocate(intArrayFromString("unknown SDL-emscripten error"), 'i8');
  },

  SDL_CreateRGBSurface: function(flags, width, height, depth, rmask, gmask, bmask, amask) {
    return SDL.makeSurface(width, height, flags, false, 'CreateRGBSurface', rmask, gmask, bmask, amask);
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
    dstData.ctx.drawImage(srcData.canvas, sr.x, sr.y, sr.w, sr.h, dr.x, dr.y, sr.w, sr.h);
    if (dst != SDL.screen) {
      // XXX As in IMG_Load, for compatibility we write out |pixels|
      console.log('WARNING: copying canvas data to memory for compatibility');
      _SDL_LockSurface(dst);
      dstData.locked--; // The surface is not actually locked in this hack
    }
    return 0;
  },

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
  },

  SDL_BlitSurface__deps: ['SDL_UpperBlit'],
  SDL_BlitSurface: function(src, srcrect, dst, dstrect) {
    return _SDL_UpperBlit(src, srcrect, dst, dstrect);
  },

  zoomSurface: function(src, x, y, smooth) {
    var srcData = SDL.surfaces[src];
    var w = srcData.width*x;
    var h = srcData.height*y;
    var ret = SDL.makeSurface(w, h, srcData.flags, false, 'zoomSurface');
    var dstData = SDL.surfaces[ret];
    dstData.ctx.drawImage(srcData.canvas, 0, 0, w, h);
    return ret;
  },

  rotozoomSurface: function(src, angle, zoom, smooth) {
    var srcData = SDL.surfaces[src];
    var w = srcData.width * zoom;
    var h = srcData.height * zoom;
    var diagonal = Math.ceil(Math.sqrt(Math.pow(w, 2) + Math.pow(h, 2)));
    var ret = SDL.makeSurface(diagonal, diagonal, srcData.flags, false, 'rotozoomSurface');
    var dstData = SDL.surfaces[ret];
    dstData.ctx.translate(diagonal / 2, diagonal / 2);
    dstData.ctx.rotate(angle * Math.PI / 180);
    dstData.ctx.drawImage(srcData.canvas, -w / 2, -h / 2, w, h);
    return ret;
  },

  SDL_SetAlpha: function(surf, flag, alpha) {
    SDL.surfaces[surf].alpha = alpha;
  },

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

  SDL_PumpEvents: function(){},

  SDL_SetColors: function(surf, colors, firstColor, nColors) {
    var surfData = SDL.surfaces[surf];

    // we should create colors array
    // only once cause client code
    // often wants to change portion 
    // of palette not all palette.
    if (!surfData.colors) {
      surfData.colors = new Uint8Array(256 * 3); //256 RGB colors
    } 

    for (var i = firstColor; i < firstColor + nColors; i++) {
      var index = i *3;
      surfData.colors[index] = {{{ makeGetValue('colors', 'i*4', 'i8', null, true) }}};
      surfData.colors[index +1] = {{{ makeGetValue('colors', 'i*4 +1', 'i8', null, true) }}};
      surfData.colors[index +2] = {{{ makeGetValue('colors', 'i*4 +2', 'i8', null, true) }}};
    }

    return 1;
  },

  SDL_MapRGB: function(fmt, r, g, b) {
    // Canvas screens are always RGBA
    return 0xff+((b&0xff)<<8)+((g&0xff)<<16)+((r&0xff)<<24)
  },

  SDL_MapRGBA: function(fmt, r, g, b, a) {
    // Canvas screens are always RGBA
    return (a&0xff)+((b&0xff)<<8)+((g&0xff)<<16)+((r&0xff)<<24)
  },

  SDL_WM_GrabInput: function() {},
  
  SDL_WM_ToggleFullScreen: function(surf) {
    if (Browser.isFullScreen) {
      Module['canvas'].cancelFullScreen();
      return 1;
    } else {
      return 0;
    }
  },

  // SDL_Image

  IMG_Init: function(flags) {
    return flags; // We support JPG, PNG, TIF because browsers do
  },

  IMG_Load__deps: ['SDL_LockSurface'],
  IMG_Load: function(filename) {
    filename = FS.standardizePath(Pointer_stringify(filename));
    if (filename[0] == '/') {
      // Convert the path to relative
      filename = filename.substr(1);
    }
    var raw = Module["preloadedImages"][filename];
    if (!raw) {
      if (raw === null) Module.printErr('Trying to reuse preloaded image, but freePreloadedMediaOnUse is set!');
      Runtime.warnOnce('Cannot find preloaded image ' + filename);
      return 0;
    }
    if (Module['freePreloadedMediaOnUse']) {
      Module["preloadedImages"][filename] = null;
    }
    var surf = SDL.makeSurface(raw.width, raw.height, 0, false, 'load:' + filename);
    var surfData = SDL.surfaces[surf];
    surfData.ctx.globalCompositeOperation = "copy";
    surfData.ctx.drawImage(raw, 0, 0, raw.width, raw.height, 0, 0, raw.width, raw.height);
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
  },
  SDL_LoadBMP: 'IMG_Load',
  SDL_LoadBMP_RW: 'IMG_Load',
  IMG_Load_RW: 'IMG_Load',

  // SDL_Audio

  // TODO fix SDL_OpenAudio, and add some tests for it.  It's currently broken.
  SDL_OpenAudio: function(desired, obtained) {
    SDL.allocateChannels(32);

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
      Runtime.dynCall('viii', SDL.audio.callback, [SDL.audio.userdata, SDL.audio.buffer, SDL.audio.bufferSize]);
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
          mozBuffer[i] = ({{{ makeGetValue('ptr', 'i*2', 'i16', 0, 0) }}}) / 0x8000; // hardcoded 16-bit audio, signed (TODO: reSign if not ta2?)
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

  SDL_CloseAudio__deps: ['SDL_PauseAudio', 'free'],
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

  Mix_LoadWAV_RW: function(filename, freesrc) {
    filename = FS.standardizePath(Pointer_stringify(filename));
    var raw = Module["preloadedAudios"][filename];
    if (!raw) {
      if (raw === null) Module.printErr('Trying to reuse preloaded audio, but freePreloadedMediaOnUse is set!');
      Runtime.warnOnce('Cannot find preloaded audio ' + filename);
      return 0;
    }
    if (Module['freePreloadedMediaOnUse']) {
      Module["preloadedAudios"][filename] = null;
    }
    var id = SDL.audios.length;
    // Keep the loaded audio in the audio arrays, ready for playback
    SDL.audios.push({
      source: filename,
      audio: raw
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
      channel = SDL.channelMinimumNumber;
      for (var i = SDL.channelMinimumNumber; i < SDL.numChannels; i++) {
        if (!SDL.channels[i].audio) {
          channel = i;
          break;
        }
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
    if (SDL.channelFinished) {
      audio['onended'] = function() { // TODO: cache these
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
        jsNode.onaudioprocess = function(event) {
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
    audio.paused = false;
    return channel;
  },
  Mix_PlayChannelTimed: 'Mix_PlayChannel', // XXX ignore Timing

  Mix_FadingChannel: function(channel) {
    return 0; // MIX_NO_FADING, TODO
  },

  Mix_HaltChannel: function(channel) {
    var info = SDL.channels[channel];
    if (info.audio) {
      info.audio.pause();
      info.audio = null;
    }
    if (SDL.channelFinished) {
      Runtime.getFuncWrapper(SDL.channelFinished, 'vi')(channel);
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

  Mix_LoadMUS: 'Mix_LoadWAV_RW',
  Mix_LoadMUS_RW: 'Mix_LoadWAV_RW',

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
      info.audio.paused = true;
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

  // SDL gfx

  boxRGBA: function(surf, x1, y1, x2, y2, r, g, b, a) {
    var surfData = SDL.surfaces[surf];
    assert(!surfData.locked); // but we could unlock and re-lock if we must..
    // TODO: if ctx does not change, leave as is, and also do not re-set xStyle etc.
    surfData.ctx.save();
    surfData.ctx.fillStyle = SDL.translateRGBAToCSSRGBA(r, g, b, a);
    surfData.ctx.fillRect(x1, y1, x2-x1, y2-y1);
    surfData.ctx.restore();
  },

  rectangleRGBA: function(surf, x1, y1, x2, y2, r, g, b, a) {
    var surfData = SDL.surfaces[surf];
    assert(!surfData.locked); // but we could unlock and re-lock if we must..
    surfData.ctx.save();
    surfData.ctx.strokeStyle = SDL.translateRGBAToCSSRGBA(r, g, b, a);
    surfData.ctx.strokeRect(x1, y1, x2-x1, y2-y1);
    surfData.ctx.restore();
  },

  lineRGBA: function(surf, x1, y1, x2, y2, r, g, b, a) {
    var surfData = SDL.surfaces[surf];
    assert(!surfData.locked); // but we could unlock and re-lock if we must..
    surfData.ctx.save();
    surfData.ctx.strokeStyle = SDL.translateRGBAToCSSRGBA(r, g, b, a);
    surfData.ctx.beginPath();
    surfData.ctx.moveTo(x1, y1);
    surfData.ctx.lineTo(x2, y2);
    surfData.ctx.stroke();
    surfData.ctx.restore();
  },

  pixelRGBA__deps: ['boxRGBA'],
  pixelRGBA: function(surf, x1, y1, r, g, b, a) {
    // This cannot be fast, to render many pixels this way!
    _boxRGBA(surf, x1, y1, x1, y1, r, g, b, a);
  },

  // GL

  SDL_GL_SetAttribute: function(attr, value) {
    console.log('TODO: SDL_GL_SetAttribute');
  },

  SDL_GL_GetProcAddress__deps: ['$GLEmulation'],
  SDL_GL_GetProcAddress: function(name_) {
    return GLEmulation.getProcAddress(Pointer_stringify(name_));
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

  // Misc

  SDL_InitSubSystem: function(flags) { return 0 },

  SDL_NumJoysticks: function() { return 0 },

  SDL_RWFromFile: function(filename, mode) {
    return filename; // XXX We just forward the filename
  },

  SDL_EnableUNICODE: function(on) {
    var ret = SDL.unicode || 0;
    SDL.unicode = on;
    return ret;
  },

  SDL_AddTimer: function(interval, callback, param) {
    return window.setTimeout(function() {
      Runtime.dynCall('ii', callback, [interval, param]);
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
  SDL_ThreadID: function() { throw 'SDL_ThreadID' },
  SDL_AllocRW: function() { throw 'SDL_AllocRW: TODO' },
  SDL_FreeRW: function() { throw 'SDL_FreeRW: TODO' },
  SDL_CondBroadcast: function() { throw 'SDL_CondBroadcast: TODO' },
  SDL_CondWaitTimeout: function() { throw 'SDL_CondWaitTimeout: TODO' },
  SDL_WM_ToggleFullScreen: function() { throw 'SDL_WM_ToggleFullScreen: TODO' },

  Mix_SetPostMix: function() { throw 'Mix_SetPostMix: TODO' },
  Mix_QuerySpec: function() { throw 'Mix_QuerySpec: TODO' },
  Mix_FadeInChannelTimed: function() { throw 'Mix_FadeInChannelTimed' },
  Mix_FadeOutChannel: function() { throw 'Mix_FadeOutChannel' },

  Mix_Linked_Version: function() { throw 'Mix_Linked_Version: TODO' },
  SDL_CreateRGBSurfaceFrom: function() { throw 'SDL_CreateRGBSurfaceFrom: TODO' },
  SDL_SaveBMP_RW: function() { throw 'SDL_SaveBMP_RW: TODO' },
};

autoAddDeps(LibrarySDL, '$SDL');
mergeInto(LibraryManager.library, LibrarySDL);

