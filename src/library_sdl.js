/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

//"use strict";

// See browser tests for examples (test/runner.py, search for sdl_). Run with
//    test/runner browser

// Notes:
//  SDL_VIDEORESIZE: This is sent when the canvas is resized. Note that the user
//                   cannot manually do so, so this is only sent when the
//                   program manually resizes it (emscripten_set_canvas_element_size
//                   or otherwise).

var LibrarySDL = {
  $SDL__deps: [
    '$PATH', '$Browser', 'SDL_GetTicks', 'SDL_LockSurface',
    '$MainLoop',
    // For makeCEvent().
    '$intArrayFromString',
    // Many SDL functions depend on malloc/free
    'malloc', 'free',
  ],
  $SDL: {
    defaults: {
      width: 320,
      height: 200,
      // If true, SDL_LockSurface will copy the contents of each surface back to
      // the Emscripten HEAP so that C code can access it. If false, the surface
      // contents are captured only back to JS code.
      copyOnLock: true,
      // If true, SDL_LockSurface will discard the contents of each surface when
      // SDL_LockSurface() is called. This greatly improves performance of
      // SDL_LockSurface(). If discardOnLock is true, copyOnLock is ignored.
      discardOnLock: false,
      // If true, emulate compatibility with desktop SDL by ignoring alpha on
      // the screen frontbuffer canvas. Setting this to false will improve
      // performance considerably and enables alpha-blending on the frontbuffer,
      // so be sure to properly write 0xFF alpha for opaque pixels if you set
      // this to false!
      opaqueFrontBuffer: true
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
    mixerFormat: {{{ cDefs.AUDIO_S16LSB }}},
    mixerNumChannels: 2,
    mixerChunkSize: 1024,
    channelMinimumNumber: 0,

    // Set to true if we call SDL_SetVideoMode with SDL_OPENGL, and if so, we do
    // not create 2D canvases&contexts for blitting
    // Note that images loaded before SDL_SetVideoMode will not get this
    // optimization
    GL: false,

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
    unicode: false,
    ttfContext: null,
    audio: null,

    startTime: null,
    initFlags: 0, // The flags passed to SDL_Init
    buttonState: 0,
    modState: 0,
    DOMButtons: [0, 0, 0],

    DOMEventToSDLEvent: {},

    TOUCH_DEFAULT_ID: 0, // Our default deviceID for touch events (we get nothing from the browser)

    eventHandler: null,
    eventHandlerContext: null,
    eventHandlerTemp: 0,

    // DOM code ==> SDL code. See
    // https://developer.mozilla.org/en/Document_Object_Model_%28DOM%29/KeyboardEvent
    // and SDL_keycode.h
    // For keys that don't have unicode value, we map DOM codes with the
    // corresponding scan codes + 1024 (using "| 1 << 10")
    keyCodes: {
      16: 225 | 1<<10, // shift
      17: 224 | 1<<10, // control (right, or left)
      18: 226 | 1<<10, // alt
      20: 57 | 1<<10, // caps lock

      33: 75 | 1<<10, // pagedup
      34: 78 | 1<<10, // pagedown
      35: 77 | 1<<10, // end
      36: 74 | 1<<10, // home
      37: 80 | 1<<10, // left arrow
      38: 82 | 1<<10, // up arrow
      39: 79 | 1<<10, // right arrow
      40: 81 | 1<<10, // down arrow
      44: 316, // print screen
      45: 73 | 1<<10, // insert
      46: 127, // SDLK_DEL == '\177'

      91: 227 | 1<<10, // windows key or super key on linux (doesn't work on Mac)
      93: 101 | 1<<10, // application

      96: 98 | 1<<10, // keypad 0
      97: 89 | 1<<10, // keypad 1
      98: 90 | 1<<10, // keypad 2
      99: 91 | 1<<10, // keypad 3
      100: 92 | 1<<10, // keypad 4
      101: 93 | 1<<10, // keypad 5
      102: 94 | 1<<10, // keypad 6
      103: 95 | 1<<10, // keypad 7
      104: 96 | 1<<10, // keypad 8
      105: 97 | 1<<10, // keypad 9
      106: 85 | 1<<10, // keypad multiply
      107: 87 | 1<<10, // keypad plus
      109: 86 | 1<<10, // keypad minus
      110: 99 | 1<<10, // keypad decimal point
      111: 84 | 1<<10, // keypad divide
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
      124: 104 | 1<<10, // F13
      125: 105 | 1<<10, // F14
      126: 106 | 1<<10, // F15
      127: 107 | 1<<10, // F16
      128: 108 | 1<<10, // F17
      129: 109 | 1<<10, // F18
      130: 110 | 1<<10, // F19
      131: 111 | 1<<10, // F20
      132: 112 | 1<<10, // F21
      133: 113 | 1<<10, // F22
      134: 114 | 1<<10, // F23
      135: 115 | 1<<10, // F24

      144: 83 | 1<<10, // keypad num lock

      160: 94, // caret
      161: 33, // exclaim
      162: 34, // double quote
      163: 35, // hash
      164: 36, // dollar
      165: 37, // percent
      166: 38, // ampersand
      167: 95, // underscore
      168: 40, // open parenthesis
      169: 41, // close parenthesis
      170: 42, // asterix
      171: 43, // plus
      172: 124, // pipe
      173: 45, // minus
      174: 123, // open curly bracket
      175: 125, // close curly bracket
      176: 126, // tilde

      181: 127, // audio mute
      182: 129, // audio volume down
      183: 128, // audio volume up

      188: 44, // comma
      190: 46, // period
      191: 47, // slash (/)
      192: 96, // backtick/backquote (`)
      219: 91, // open square bracket
      220: 92, // back slash
      221: 93, // close square bracket
      222: 39, // quote
      224: 227 | 1<<10, // meta (command/windows)
    },

    scanCodes: { // SDL keycode ==> SDL scancode. See SDL_scancode.h
      8: 42, // backspace
      9: 43, // tab
      13: 40, // return
      27: 41, // escape
      32: 44, // space
      35: 204, // hash

      39: 53, // grave

      44: 54, // comma
      46: 55, // period
      47: 56, // slash
      48: 39, // 0
      49: 30, // 1
      50: 31, // 2
      51: 32, // 3
      52: 33, // 4
      53: 34, // 5
      54: 35, // 6
      55: 36, // 7
      56: 37, // 8
      57: 38, // 9
      58: 203, // colon
      59: 51, // semicolon

      61: 46, // equals

      91: 47, // left bracket
      92: 49, // backslash
      93: 48, // right bracket

      96: 52, // apostrophe
      97: 4, // A
      98: 5, // B
      99: 6, // C
      100: 7, // D
      101: 8, // E
      102: 9, // F
      103: 10, // G
      104: 11, // H
      105: 12, // I
      106: 13, // J
      107: 14, // K
      108: 15, // L
      109: 16, // M
      110: 17, // N
      111: 18, // O
      112: 19, // P
      113: 20, // Q
      114: 21, // R
      115: 22, // S
      116: 23, // T
      117: 24, // U
      118: 25, // V
      119: 26, // W
      120: 27, // X
      121: 28, // Y
      122: 29, // Z

      127: 76, // delete

      305: 224, // ctrl

      308: 226, // alt

      316: 70, // print screen
    },
    loadRect(rect) {
      return {
        x: {{{ makeGetValue('rect', C_STRUCTS.SDL_Rect.x, 'i32') }}},
        y: {{{ makeGetValue('rect', C_STRUCTS.SDL_Rect.y, 'i32') }}},
        w: {{{ makeGetValue('rect', C_STRUCTS.SDL_Rect.w, 'i32') }}},
        h: {{{ makeGetValue('rect', C_STRUCTS.SDL_Rect.h, 'i32') }}}
      };
    },

    updateRect(rect, r) {
      {{{ makeSetValue('rect', C_STRUCTS.SDL_Rect.x, 'r.x', 'i32') }}};
      {{{ makeSetValue('rect', C_STRUCTS.SDL_Rect.y, 'r.y', 'i32') }}};
      {{{ makeSetValue('rect', C_STRUCTS.SDL_Rect.w, 'r.w', 'i32') }}};
      {{{ makeSetValue('rect', C_STRUCTS.SDL_Rect.h, 'r.h', 'i32') }}};
    },

    intersectionOfRects(first, second) {
      var leftX = Math.max(first.x, second.x);
      var leftY = Math.max(first.y, second.y);
      var rightX = Math.min(first.x + first.w, second.x + second.w);
      var rightY = Math.min(first.y + first.h, second.y + second.h);

      return {
        x: leftX,
        y: leftY,
        w: Math.max(leftX, rightX) - leftX,
        h: Math.max(leftY, rightY) - leftY
      }
    },

    checkPixelFormat(fmt) {
#if ASSERTIONS
      // Canvas screens are always RGBA.
      var format = {{{ makeGetValue('fmt', C_STRUCTS.SDL_PixelFormat.format, 'i32') }}};
      if (format != {{{ cDefs.SDL_PIXELFORMAT_RGBA8888 }}}) {
        warnOnce('Unsupported pixel format!');
      }
#endif
    },

    // Load SDL color into a CSS-style color specification
    loadColorToCSSRGB(color) {
      var rgba = {{{ makeGetValue('color', 0, 'i32') }}};
      return 'rgb(' + (rgba&255) + ',' + ((rgba >> 8)&255) + ',' + ((rgba >> 16)&255) + ')';
    },
    loadColorToCSSRGBA(color) {
      var rgba = {{{ makeGetValue('color', 0, 'i32') }}};
      return 'rgba(' + (rgba&255) + ',' + ((rgba >> 8)&255) + ',' + ((rgba >> 16)&255) + ',' + (((rgba >> 24)&255)/255) + ')';
    },

    translateColorToCSSRGBA: (rgba) =>
      'rgba(' + (rgba&0xff) + ',' + (rgba>>8 & 0xff) + ',' + (rgba>>16 & 0xff) + ',' + (rgba>>>24)/0xff + ')',

    translateRGBAToCSSRGBA: (r, g, b, a) =>
      'rgba(' + (r&0xff) + ',' + (g&0xff) + ',' + (b&0xff) + ',' + (a&0xff)/255 + ')',

    translateRGBAToColor: (r, g, b, a) => r | g << 8 | b << 16 | a << 24,

    makeSurface(width, height, flags, usePageCanvas, source, rmask, gmask, bmask, amask) {
      var is_SDL_HWSURFACE = flags & 0x00000001;
      var is_SDL_HWPALETTE = flags & 0x00200000;
      var is_SDL_OPENGL = flags & 0x04000000;

      var surf = _malloc({{{ C_STRUCTS.SDL_Surface.__size__ }}});
      var pixelFormat = _malloc({{{ C_STRUCTS.SDL_PixelFormat.__size__ }}});
      //surface with SDL_HWPALETTE flag is 8bpp surface (1 byte)
      var bpp = is_SDL_HWPALETTE ? 1 : 4;
      var buffer = 0;

      // preemptively initialize this for software surfaces,
      // otherwise it will be lazily initialized inside of SDL_LockSurface
      if (!is_SDL_HWSURFACE && !is_SDL_OPENGL) {
        buffer = _malloc(width * height * 4);
      }

      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.flags, 'flags', 'i32') }}};
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.format, 'pixelFormat', POINTER_TYPE) }}};
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.w, 'width', 'i32') }}};
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.h, 'height', 'i32') }}};
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.pitch, 'width * bpp', 'i32') }}};  // assuming RGBA or indexed for now,
                                                                                        // since that is what ImageData gives us in browsers
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.pixels, 'buffer', POINTER_TYPE) }}};

      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.clip_rect+C_STRUCTS.SDL_Rect.x, '0', 'i32') }}};
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.clip_rect+C_STRUCTS.SDL_Rect.y, '0', 'i32') }}};
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.clip_rect+C_STRUCTS.SDL_Rect.w, 'Module["canvas"].width', 'i32') }}};
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.clip_rect+C_STRUCTS.SDL_Rect.h, 'Module["canvas"].height', 'i32') }}};

      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.refcount, '1', 'i32') }}};

      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.format, cDefs.SDL_PIXELFORMAT_RGBA8888, 'i32') }}};
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.palette, '0', 'i32') }}};// TODO
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.BitsPerPixel, 'bpp * 8', 'i8') }}};
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.BytesPerPixel, 'bpp', 'i8') }}};

      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.Rmask, 'rmask || 0x000000ff', 'i32') }}};
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.Gmask, 'gmask || 0x0000ff00', 'i32') }}};
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.Bmask, 'bmask || 0x00ff0000', 'i32') }}};
      {{{ makeSetValue('pixelFormat', C_STRUCTS.SDL_PixelFormat.Amask, 'amask || 0xff000000', 'i32') }}};

      // Decide if we want to use WebGL or not
      SDL.GL = SDL.GL || is_SDL_OPENGL;
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
        stencil: (SDL.glAttributes[7 /*SDL_GL_STENCIL_SIZE*/] > 0),
        alpha: (SDL.glAttributes[3 /*SDL_GL_ALPHA_SIZE*/] > 0)
      };

#if OFFSCREEN_FRAMEBUFFER
      // TODO: Make SDL explicitly aware of whether it is being proxied or not, and set these to true only when proxying is being performed.
      GL.enableOffscreenFramebufferAttributes(webGLContextAttributes);
#endif
      var ctx = Browser.createContext(canvas, is_SDL_OPENGL, usePageCanvas, webGLContextAttributes);

      SDL.surfaces[surf] = {
        width,
        height,
        canvas,
        ctx,
        surf,
        buffer,
        pixelFormat,
        alpha: 255,
        flags,
        locked: 0,
        usePageCanvas,
        source,

        isFlagSet: (flag) => flags & flag
      };

      return surf;
    },

    // Copy data from the C++-accessible storage to the canvas backing
    // for surface with HWPALETTE flag(8bpp depth)
    copyIndexedColorData(surfData, rX, rY, rW, rH) {
      // HWPALETTE works with palette
      // set by SDL_SetColors
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

      if (!surfData.image.data32) {
        surfData.image.data32 = new Uint32Array(surfData.image.data.buffer);
      }
      var data32   = surfData.image.data32;

      var colors32 = surfData.colors32;

      for (var y = startY; y < endY; ++y) {
        var base = y * fullWidth;
        for (var x = startX; x < endX; ++x) {
          data32[base + x] = colors32[{{{ makeGetValue('buffer', 'base + x', 'u8') }}}];
        }
      }
    },

    freeSurface(surf) {
      var refcountPointer = surf + {{{ C_STRUCTS.SDL_Surface.refcount }}};
      var refcount = {{{ makeGetValue('refcountPointer', 0, 'i32') }}};
      if (refcount > 1) {
        {{{ makeSetValue('refcountPointer', 0, 'refcount - 1', 'i32') }}};
        return;
      }

      var info = SDL.surfaces[surf];
      if (!info.usePageCanvas && info.canvas) SDL.canvasPool.push(info.canvas);
      if (info.buffer) _free(info.buffer);
      _free(info.pixelFormat);
      _free(surf);
      SDL.surfaces[surf] = null;

      if (surf === SDL.screen) {
        SDL.screen = null;
      }
    },

    blitSurface(src, srcrect, dst, dstrect, scale) {
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
        dr = { x: 0, y: 0, w: srcData.width, h: srcData.height };
      }
      if (dstData.clipRect) {
        var widthScale = (!scale || sr.w === 0) ? 1 : sr.w / dr.w;
        var heightScale = (!scale || sr.h === 0) ? 1 : sr.h / dr.h;

        dr = SDL.intersectionOfRects(dstData.clipRect, dr);

        sr.w = dr.w * widthScale;
        sr.h = dr.h * heightScale;

        if (dstrect) {
          SDL.updateRect(dstrect, dr);
        }
      }
      var blitw, blith;
      if (scale) {
        blitw = dr.w; blith = dr.h;
      } else {
        blitw = sr.w; blith = sr.h;
      }
      if (sr.w === 0 || sr.h === 0 || blitw === 0 || blith === 0) {
        return 0;
      }
      var oldAlpha = dstData.ctx.globalAlpha;
      dstData.ctx.globalAlpha = srcData.alpha/255;
      dstData.ctx.drawImage(srcData.canvas, sr.x, sr.y, sr.w, sr.h, dr.x, dr.y, blitw, blith);
      dstData.ctx.globalAlpha = oldAlpha;
      if (dst != SDL.screen) {
        // XXX As in IMG_Load, for compatibility we write out |pixels|
        warnOnce('WARNING: copying canvas data to memory for compatibility');
        _SDL_LockSurface(dst);
        dstData.locked--; // The surface is not actually locked in this hack
      }
      return 0;
    },

    // the browser sends out touchstart events with the whole group of touches
    // even if we received a previous touchstart for a specific touch identifier.
    // You can test this by pressing one finger to the screen, then another. You'll
    // receive two touchstart events, the first with a touches count of 1 the second
    // with a touches count of two.
    // SDL sends out a new touchstart event for only each newly started touch so to
    // emulate this, we keep track of previously started touches.
    downFingers: {},
    savedKeydown: null,

    receiveEvent(event) {
      function unpressAllPressedKeys() {
        // Un-press all pressed keys: TODO
        for (var code in SDL.keyboardMap) {
          SDL.events.push({
            type: 'keyup',
            keyCode: SDL.keyboardMap[code]
          });
        }
      };
      switch (event.type) {
        case 'touchstart': case 'touchmove': {
          event.preventDefault();

          var touches = [];

          // Clear out any touchstart events that we've already processed
          if (event.type === 'touchstart') {
            for (var i = 0; i < event.touches.length; i++) {
              var touch = event.touches[i];
              if (SDL.downFingers[touch.identifier] != true) {
                SDL.downFingers[touch.identifier] = true;
                touches.push(touch);
              }
            }
          } else {
            touches = event.touches;
          }

          var firstTouch = touches[0];
          if (firstTouch) {
            if (event.type == 'touchstart') {
              SDL.DOMButtons[0] = 1;
            }
            var mouseEventType;
            switch (event.type) {
              case 'touchstart': mouseEventType = 'mousedown'; break;
              case 'touchmove': mouseEventType = 'mousemove'; break;
            }
            var mouseEvent = {
              type: mouseEventType,
              button: 0,
              pageX: firstTouch.clientX,
              pageY: firstTouch.clientY
            };
            SDL.events.push(mouseEvent);
          }

          for (var i = 0; i < touches.length; i++) {
            var touch = touches[i];
            SDL.events.push({
              type: event.type,
              touch
            });
          };
          break;
        }
        case 'touchend': {
          event.preventDefault();

          // Remove the entry in the SDL.downFingers hash
          // because the finger is no longer down.
          for (var i = 0; i < event.changedTouches.length; i++) {
            var touch = event.changedTouches[i];
            if (SDL.downFingers[touch.identifier] === true) {
              delete SDL.downFingers[touch.identifier];
            }
          }

          var mouseEvent = {
            type: 'mouseup',
            button: 0,
            pageX: event.changedTouches[0].clientX,
            pageY: event.changedTouches[0].clientY
          };
          SDL.DOMButtons[0] = 0;
          SDL.events.push(mouseEvent);

          for (var i = 0; i < event.changedTouches.length; i++) {
            var touch = event.changedTouches[i];
            SDL.events.push({
              type: 'touchend',
              touch
            });
          };
          break;
        }
        case 'DOMMouseScroll': case 'mousewheel': case 'wheel':
          // Flip the wheel direction to translate from browser wheel direction
          // (+:down) to SDL direction (+:up)
          var delta = -Browser.getMouseWheelDelta(event);
          // Quantize to integer so that minimum scroll is at least +/- 1.
          delta = (delta == 0) ? 0 : (delta > 0 ? Math.max(delta, 1) : Math.min(delta, -1));

          // Simulate old-style SDL events representing mouse wheel input as buttons
          // Subtract one since JS->C marshalling is defined to add one back.
          var button = delta > 0 ? 3 /*SDL_BUTTON_WHEELUP-1*/ : 4 /*SDL_BUTTON_WHEELDOWN-1*/;
          SDL.events.push({ type: 'mousedown', button, pageX: event.pageX, pageY: event.pageY });
          SDL.events.push({ type: 'mouseup', button, pageX: event.pageX, pageY: event.pageY });

          // Pass a delta motion event.
          SDL.events.push({ type: 'wheel', deltaX: 0, deltaY: delta });
          // If we don't prevent this, then 'wheel' event will be sent again by
          // the browser as 'DOMMouseScroll' and we will receive this same event
          // the second time.
          event.preventDefault();
          break;
        case 'mousemove':
          if (SDL.DOMButtons[0] === 1) {
            SDL.events.push({
              type: 'touchmove',
              touch: {
                identifier: 0,
                deviceID: {{{ cDefs.SDL_TOUCH_MOUSEID }}},
                pageX: event.pageX,
                pageY: event.pageY
              }
            });
          }
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
        case 'keydown': case 'keyup': case 'keypress': case 'mousedown': case 'mouseup':
          // If we preventDefault on keydown events, the subsequent keypress events
          // won't fire. However, it's fine (and in some cases necessary) to
          // preventDefault for keys that don't generate a character. Otherwise,
          // preventDefault is the right thing to do in general.
          if (event.type !== 'keydown' || (!SDL.unicode && !SDL.textInput) || (event.keyCode === 8 /* backspace */ || event.keyCode === 9 /* tab */)) {
            event.preventDefault();
          }

          if (event.type == 'mousedown') {
            SDL.DOMButtons[event.button] = 1;
            SDL.events.push({
              type: 'touchstart',
              touch: {
                identifier: 0,
                deviceID: {{{ cDefs.SDL_TOUCH_MOUSEID }}},
                pageX: event.pageX,
                pageY: event.pageY
              }
            });
          } else if (event.type == 'mouseup') {
            // ignore extra ups, can happen if we leave the canvas while pressing down, then return,
            // since we add a mouseup in that case
            if (!SDL.DOMButtons[event.button]) {
              return;
            }

            SDL.events.push({
              type: 'touchend',
              touch: {
                identifier: 0,
                deviceID: {{{ cDefs.SDL_TOUCH_MOUSEID }}},
                pageX: event.pageX,
                pageY: event.pageY
              }
            });
            SDL.DOMButtons[event.button] = 0;
          }

          // We can only request fullscreen as the result of user input.
          // Due to this limitation, we toggle a boolean on keydown which
          // SDL_WM_ToggleFullScreen will check and subsequently set another
          // flag indicating for us to request fullscreen on the following
          // keyup. This isn't perfect, but it enables SDL_WM_ToggleFullScreen
          // to work as the result of a keypress (which is an extremely
          // common use case).
          if (event.type === 'keydown' || event.type === 'mousedown') {
            SDL.canRequestFullscreen = true;
          } else if (event.type === 'keyup' || event.type === 'mouseup') {
            if (SDL.isRequestingFullscreen) {
              Module['requestFullscreen'](/*lockPointer=*/true, /*resizeCanvas=*/true);
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
        case 'focus':
          SDL.events.push(event);
          event.preventDefault();
          break;
        case 'blur':
          SDL.events.push(event);
          unpressAllPressedKeys();
          event.preventDefault();
          break;
        case 'visibilitychange':
          SDL.events.push({
            type: 'visibilitychange',
            visible: !document.hidden
          });
          unpressAllPressedKeys();
          event.preventDefault();
          break;
        case 'unload':
          if (MainLoop.runner) {
            SDL.events.push(event);
            // Force-run a main event loop, since otherwise this event will never be caught!
            MainLoop.runner();
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
        err('SDL event queue full, dropping events');
        SDL.events = SDL.events.slice(0, 10000);
      }
      // If we have a handler installed, this will push the events to the app
      // instead of the app polling for them.
      SDL.flushEventsToHandler();
      return;
    },

    lookupKeyCodeForEvent(event) {
      var code = event.keyCode;
      if (code >= 65 && code <= 90) {
        code += 32; // make lowercase for SDL
      } else {
        code = SDL.keyCodes[event.keyCode] || event.keyCode;
        // If this is one of the modifier keys (224 | 1<<10 - 227 | 1<<10), and the event specifies that it is
        // a right key, add 4 to get the right key SDL key code.
        if (event.location === 2 /*KeyboardEvent.DOM_KEY_LOCATION_RIGHT*/ && code >= (224 | 1<<10) && code <= (227 | 1<<10)) {
          code += 4;
        }
      }
      return code;
    },

    handleEvent(event) {
      if (event.handled) return;
      event.handled = true;

      switch (event.type) {
        case 'touchstart': case 'touchend': case 'touchmove': {
          Browser.calculateMouseEvent(event);
          break;
        }
        case 'keydown': case 'keyup': {
          var down = event.type === 'keydown';
          var code = SDL.lookupKeyCodeForEvent(event);
#if !SAFE_HEAP
          // Assigning a boolean to HEAP8, that's alright but Closure would like to warn about it.
          // TODO(https://github.com/emscripten-core/emscripten/issues/16311):
          // This is kind of ugly hack.  Perhaps we can find a better way?
          /** @suppress{checkTypes} */
#endif
          {{{ makeSetValue('SDL.keyboardState', 'code', 'down', 'i8') }}};
          // TODO: lmeta, rmeta, numlock, capslock, KMOD_MODE, KMOD_RESERVED
          SDL.modState = ({{{ makeGetValue('SDL.keyboardState', '1248', 'i8') }}} ? 0x0040 : 0) | // KMOD_LCTRL
            ({{{ makeGetValue('SDL.keyboardState', '1249', 'i8') }}} ? 0x0001 : 0) | // KMOD_LSHIFT
            ({{{ makeGetValue('SDL.keyboardState', '1250', 'i8') }}} ? 0x0100 : 0) | // KMOD_LALT
            ({{{ makeGetValue('SDL.keyboardState', '1252', 'i8') }}} ? 0x0080 : 0) | // KMOD_RCTRL
            ({{{ makeGetValue('SDL.keyboardState', '1253', 'i8') }}} ? 0x0002 : 0) | // KMOD_RSHIFT
            ({{{ makeGetValue('SDL.keyboardState', '1254', 'i8') }}} ? 0x0200 : 0); //  KMOD_RALT
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

    flushEventsToHandler() {
      if (!SDL.eventHandler) return;

      while (SDL.pollEvent(SDL.eventHandlerTemp)) {
        {{{ makeDynCall('ipp', 'SDL.eventHandler') }}}(SDL.eventHandlerContext, SDL.eventHandlerTemp);
      }
    },

    pollEvent(ptr) {
      if (SDL.initFlags & 0x200 && SDL.joystickEventState) {
        // If SDL_INIT_JOYSTICK was supplied AND the joystick system is configured
        // to automatically query for events, query for joystick events.
        SDL.queryJoysticks();
      }
      if (ptr) {
        while (SDL.events.length > 0) {
          if (SDL.makeCEvent(SDL.events.shift(), ptr) !== false) return 1;
        }
        return 0;
      }
      // XXX: somewhat risky in that we do not check if the event is real or not
      // (makeCEvent returns false) if no pointer supplied
      return SDL.events.length > 0;
    },

    // returns false if the event was determined to be irrelevant
    makeCEvent(event, ptr) {
      if (typeof event == 'number') {
        // This is a pointer to a copy of a native C event that was SDL_PushEvent'ed
        _memcpy(ptr, event, {{{ C_STRUCTS.SDL_KeyboardEvent.__size__ }}});
        _free(event); // the copy is no longer needed
        return;
      }

      SDL.handleEvent(event);

      switch (event.type) {
        case 'keydown': case 'keyup': {
          var down = event.type === 'keydown';
          //dbg('Received key event: ' + event.keyCode);
          var key = SDL.lookupKeyCodeForEvent(event);
          var scan;
          if (key >= 1024) {
            scan = key - 1024;
          } else {
            scan = SDL.scanCodes[key] || key;
          }

          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.state, 'down ? 1 : 0', 'i8') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.repeat, '0', 'i8') }}}; // TODO
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.keysym + C_STRUCTS.SDL_Keysym.scancode, 'scan', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.keysym + C_STRUCTS.SDL_Keysym.sym, 'key', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.keysym + C_STRUCTS.SDL_Keysym.mod, 'SDL.modState', 'i16') }}};
          // some non-character keys (e.g. backspace and tab) won't have keypressCharCode set, fill in with the keyCode.
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_KeyboardEvent.keysym + C_STRUCTS.SDL_Keysym.unicode, 'event.keypressCharCode || key', 'i32') }}};

          break;
        }
        case 'keypress': {
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_TextInputEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
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
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.timestamp, '0', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.windowID, '0', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.which, '0', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.button, 'event.button+1', 'i8') }}}; // DOM buttons are 0-2, SDL 1-3
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.state, 'down ? 1 : 0', 'i8') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.x, 'Browser.mouseX', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseButtonEvent.y, 'Browser.mouseY', 'i32') }}};
          } else {
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.timestamp, '0', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.windowID, '0', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.which, '0', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.state, 'SDL.buttonState', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.x, 'Browser.mouseX', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.y, 'Browser.mouseY', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.xrel, 'Browser.mouseMovementX', 'i32') }}};
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseMotionEvent.yrel, 'Browser.mouseMovementY', 'i32') }}};
          }
          break;
        }
        case 'wheel': {
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseWheelEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseWheelEvent.x, 'event.deltaX', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_MouseWheelEvent.y, 'event.deltaY', 'i32') }}};
          break;
        }
        case 'touchstart': case 'touchend': case 'touchmove': {
          var touch = event.touch;
          if (!Browser.touches[touch.identifier]) break;
          var w = Module['canvas'].width;
          var h = Module['canvas'].height;
          var x = Browser.touches[touch.identifier].x / w;
          var y = Browser.touches[touch.identifier].y / h;
          var lx = Browser.lastTouches[touch.identifier].x / w;
          var ly = Browser.lastTouches[touch.identifier].y / h;
          var dx = x - lx;
          var dy = y - ly;
          if (touch['deviceID'] === undefined) touch.deviceID = SDL.TOUCH_DEFAULT_ID;
          if (dx === 0 && dy === 0 && event.type === 'touchmove') return false; // don't send these if nothing happened
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_TouchFingerEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_TouchFingerEvent.timestamp, '_SDL_GetTicks()', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_TouchFingerEvent.touchId, 'touch.deviceID', 'i64') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_TouchFingerEvent.fingerId, 'touch.identifier', 'i64') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_TouchFingerEvent.x, 'x', 'float') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_TouchFingerEvent.y, 'y', 'float') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_TouchFingerEvent.dx, 'dx', 'float') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_TouchFingerEvent.dy, 'dy', 'float') }}};
          if (touch.force !== undefined) {
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_TouchFingerEvent.pressure, 'touch.force', 'float') }}};
          } else { // No pressure data, send a digital 0/1 pressure.
            {{{ makeSetValue('ptr', C_STRUCTS.SDL_TouchFingerEvent.pressure, 'event.type == "touchend" ? 0 : 1', 'float') }}};
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
        case 'focus': {
          var SDL_WINDOWEVENT_FOCUS_GAINED = 12 /* SDL_WINDOWEVENT_FOCUS_GAINED */;
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_WindowEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_WindowEvent.windowID, '0', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_WindowEvent.event, 'SDL_WINDOWEVENT_FOCUS_GAINED', 'i8') }}};
          break;
        }
        case 'blur': {
          var SDL_WINDOWEVENT_FOCUS_LOST = 13 /* SDL_WINDOWEVENT_FOCUS_LOST */;
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_WindowEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_WindowEvent.windowID, '0', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_WindowEvent.event, 'SDL_WINDOWEVENT_FOCUS_LOST', 'i8') }}};
          break;
        }
        case 'visibilitychange': {
          var SDL_WINDOWEVENT_SHOWN  = 1 /* SDL_WINDOWEVENT_SHOWN */;
          var SDL_WINDOWEVENT_HIDDEN = 2 /* SDL_WINDOWEVENT_HIDDEN */;
          var visibilityEventID = event.visible ? SDL_WINDOWEVENT_SHOWN : SDL_WINDOWEVENT_HIDDEN;
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_WindowEvent.type, 'SDL.DOMEventToSDLEvent[event.type]', 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_WindowEvent.windowID, 0, 'i32') }}};
          {{{ makeSetValue('ptr', C_STRUCTS.SDL_WindowEvent.event, 'visibilityEventID' , 'i8') }}};
          break;
        }
        default: throw 'Unhandled SDL event: ' + event.type;
      }
    },

    makeFontString(height, fontName) {
      if (fontName.charAt(0) != "'" && fontName.charAt(0) != '"') {
        // https://developer.mozilla.org/ru/docs/Web/CSS/font-family
        // Font family names containing whitespace should be quoted.
        // BTW, quote all font names is easier than searching spaces
        fontName = '"' + fontName + '"';
      }
      return height + 'px ' + fontName + ', serif';
    },

    estimateTextWidth(fontData, text) {
      var h = fontData.size;
      var fontString = SDL.makeFontString(h, fontData.name);
      var tempCtx = SDL.ttfContext;
#if ASSERTIONS
      assert(tempCtx, 'TTF_Init must have been called');
#endif
      tempCtx.font = fontString;
      var ret = tempCtx.measureText(text).width | 0;
      return ret;
    },

    // Sound

    // Channels are a SDL abstraction for allowing multiple sound tracks to be
    // played at the same time.  We don't need to actually implement the mixing
    // since the browser engine handles that for us.  Therefore, in JS we just
    // maintain a list of channels and return IDs for them to the SDL consumer.
    allocateChannels(num) { // called from Mix_AllocateChannels and init
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

    setGetVolume(info, volume) {
      if (!info) return 0;
      var ret = info.volume * 128; // MIX_MAX_VOLUME
      if (volume != -1) {
        info.volume = Math.min(Math.max(volume, 0), 128) / 128;
        if (info.audio) {
          try {
            info.audio.volume = info.volume; // For <audio> element
            if (info.audio.webAudioGainNode) info.audio.webAudioGainNode['gain']['value'] = info.volume; // For WebAudio playback
          } catch(e) {
            err(`setGetVolume failed to set audio volume: ${e}`);
          }
        }
      }
      return ret;
    },

    setPannerPosition(info, x, y, z) {
      if (!info) return;
      if (info.audio) {
        if (info.audio.webAudioPannerNode) {
          info.audio.webAudioPannerNode['setPosition'](x, y, z);
        }
      }
    },

    // Plays out an SDL audio resource that was loaded with the Mix_Load APIs, when using Web Audio..
    playWebAudio(audio) {
      if (!audio) return;
      if (audio.webAudioNode) return; // This instance is already playing, don't start again.
      if (!SDL.webAudioAvailable()) return;
      try {
        var webAudio = audio.resource.webAudio;
        audio.paused = false;
        if (!webAudio.decodedBuffer) {
          if (webAudio.onDecodeComplete === undefined) abort("Cannot play back audio object that was not loaded");
          webAudio.onDecodeComplete.push(() => { if (!audio.paused) SDL.playWebAudio(audio); });
          return;
        }
        audio.webAudioNode = SDL.audioContext['createBufferSource']();
        audio.webAudioNode['buffer'] = webAudio.decodedBuffer;
        audio.webAudioNode['loop'] = audio.loop;
        audio.webAudioNode['onended'] = audio['onended']; // For <media> element compatibility, route the onended signal to the instance.

        audio.webAudioPannerNode = SDL.audioContext['createPanner']();
        // avoid Chrome bug
        // If posz = 0, the sound will come from only the right.
        // By posz = -0.5 (slightly ahead), the sound will come from right and left correctly.
        audio.webAudioPannerNode["setPosition"](0, 0, -.5);
        audio.webAudioPannerNode['panningModel'] = 'equalpower';

        // Add an intermediate gain node to control volume.
        audio.webAudioGainNode = SDL.audioContext['createGain']();
        audio.webAudioGainNode['gain']['value'] = audio.volume;

        audio.webAudioNode['connect'](audio.webAudioPannerNode);
        audio.webAudioPannerNode['connect'](audio.webAudioGainNode);
        audio.webAudioGainNode['connect'](SDL.audioContext['destination']);

        audio.webAudioNode['start'](0, audio.currentPosition);
        audio.startTime = SDL.audioContext['currentTime'] - audio.currentPosition;
      } catch(e) {
        err(`playWebAudio failed: ${e}`);
      }
    },

    // Pauses an SDL audio resource that was played with Web Audio.
    pauseWebAudio(audio) {
      if (!audio) return;
      if (audio.webAudioNode) {
        try {
          // Remember where we left off, so that if/when we resume, we can restart the playback at a proper place.
          audio.currentPosition = (SDL.audioContext['currentTime'] - audio.startTime) % audio.resource.webAudio.decodedBuffer.duration;
          // Important: When we reach here, the audio playback is stopped by the user. But when calling .stop() below, the Web Audio
          // graph will send the onended signal, but we don't want to process that, since pausing should not clear/destroy the audio
          // channel.
          audio.webAudioNode['onended'] = undefined;
          audio.webAudioNode.stop(0); // 0 is a default parameter, but WebKit is confused by it #3861
          audio.webAudioNode = undefined;
        } catch(e) {
          err(`pauseWebAudio failed: ${e}`);
        }
      }
      audio.paused = true;
    },

    openAudioContext() {
      // Initialize Web Audio API if we haven't done so yet. Note: Only initialize Web Audio context ever once on the web page,
      // since initializing multiple times fails on Chrome saying 'audio resources have been exhausted'.
      if (!SDL.audioContext) {
        if (typeof AudioContext != 'undefined') SDL.audioContext = new AudioContext();
        else if (typeof webkitAudioContext != 'undefined') SDL.audioContext = new webkitAudioContext();
      }
    },

    webAudioAvailable: () => !!SDL.audioContext,

    fillWebAudioBufferFromHeap(heapPtr, sizeSamplesPerChannel, dstAudioBuffer) {
      // The input audio data is interleaved across the channels, i.e. [L, R, L, R, L, R, ...] and is either 8-bit, 16-bit or float as
      // supported by the SDL API. The output audio wave data for Web Audio API must be in planar buffers of [-1,1]-normalized Float32 data,
      // so perform a buffer conversion for the data.
      var audio = SDL.audio;
      var numChannels = audio.channels;
      for (var c = 0; c < numChannels; ++c) {
        var channelData = dstAudioBuffer['getChannelData'](c);
        if (channelData.length != sizeSamplesPerChannel) {
          throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + sizeSamplesPerChannel + ' samples!';
        }
        if (audio.format == {{{ cDefs.AUDIO_S16LSB }}}) {
          for (var j = 0; j < sizeSamplesPerChannel; ++j) {
            channelData[j] = ({{{ makeGetValue('heapPtr', '(j*numChannels + c)*2', 'i16') }}}) / 0x8000;
          }
        } else if (audio.format == {{{ cDefs.AUDIO_U8 }}}) {
          for (var j = 0; j < sizeSamplesPerChannel; ++j) {
            var v = ({{{ makeGetValue('heapPtr', 'j*numChannels + c', 'i8') }}});
            channelData[j] = ((v >= 0) ? v-128 : v+128) /128;
          }
        } else if (audio.format == {{{ cDefs.AUDIO_F32 }}}) {
          for (var j = 0; j < sizeSamplesPerChannel; ++j) {
            channelData[j] = ({{{ makeGetValue('heapPtr', '(j*numChannels + c)*4', 'float') }}});
          }
        } else {
          throw 'Invalid SDL audio format ' + audio.format + '!';
        }
      }
    },

    // Debugging

#if ASSERTIONS
    debugSurface(surfData) {
      dbg('dumping surface ' + [surfData.surf, surfData.source, surfData.width, surfData.height]);
      var image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
      var data = image.data;
      var num = Math.min(surfData.width, surfData.height);
      for (var i = 0; i < num; i++) {
        dbg('   diagonal ' + i + ':' + [data[i*surfData.width*4 + i*4 + 0], data[i*surfData.width*4 + i*4 + 1], data[i*surfData.width*4 + i*4 + 2], data[i*surfData.width*4 + i*4 + 3]]);
      }
    },
#endif

    // Joystick helper methods and state

    joystickEventState: 1, // SDL_ENABLE
    lastJoystickState: {}, // Map from SDL_Joystick* to their last known state. Required to determine if a change has occurred.
    // Maps Joystick names to pointers. Allows us to avoid reallocating memory for
    // joystick names each time this function is called.
    joystickNamePool: {},
    recordJoystickState(joystick, state) {
      // Standardize button state.
      var buttons = new Array(state.buttons.length);
      for (var i = 0; i < state.buttons.length; i++) {
        buttons[i] = SDL.getJoystickButtonState(state.buttons[i]);
      }

      SDL.lastJoystickState[joystick] = {
        buttons,
        axes: state.axes.slice(0),
        timestamp: state.timestamp,
        index: state.index,
        id: state.id
      };
    },
    // Retrieves the button state of the given gamepad button.
    // Abstracts away implementation differences.
    // Returns 'true' if pressed, 'false' otherwise.
    getJoystickButtonState(button) {
      if (typeof button == 'object') {
        // Current gamepad API editor's draft (Firefox Nightly)
        // https://dvcs.w3.org/hg/gamepad/raw-file/default/gamepad.html#idl-def-GamepadButton
        return button['pressed'];
      }
      // Current gamepad API working draft (Firefox / Chrome Stable)
      // http://www.w3.org/TR/2012/WD-gamepad-20120529/#gamepad-interface
      return button > 0;
    },
    // Queries for and inserts controller events into the SDL queue.
    queryJoysticks() {
      for (var joystick in SDL.lastJoystickState) {
        var state = SDL.getGamepad(joystick - 1);
        var prevState = SDL.lastJoystickState[joystick];
        // If joystick was removed, state returns null.
        if (typeof state == 'undefined') return;
        if (state === null) return;
        // Check only if the timestamp has differed.
        // NOTE: Timestamp is not available in Firefox.
        // NOTE: Timestamp is currently not properly set for the GearVR controller
        //       on Samsung Internet: it is always zero.
        if (typeof state.timestamp != 'number' || state.timestamp != prevState.timestamp || !state.timestamp) {
          var i;
          for (i = 0; i < state.buttons.length; i++) {
            var buttonState = SDL.getJoystickButtonState(state.buttons[i]);
            // NOTE: The previous state already has a boolean representation of
            //       its button, so no need to standardize its button state here.
            if (buttonState !== prevState.buttons[i]) {
              // Insert button-press event.
              SDL.events.push({
                type: buttonState ? 'joystick_button_down' : 'joystick_button_up',
                joystick,
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
                joystick,
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
    joystickAxisValueConversion(value) {
      // Make sure value is properly clamped
      value = Math.min(1, Math.max(value, -1));
      // Ensures that 0 is 0, 1 is 32767, and -1 is 32768.
      return Math.ceil(((value+1) * 32767.5) - 32768);
    },

    getGamepads() {
      var fcn = navigator.getGamepads || navigator.webkitGamepads || navigator.mozGamepads || navigator.gamepads || navigator.webkitGetGamepads;
      if (fcn !== undefined) {
        // The function must be applied on the navigator object.
        return fcn.apply(navigator);
      }
      return [];
    },

    // Helper function: Returns the gamepad if available, or null if not.
    getGamepad(deviceIndex) {
      var gamepads = SDL.getGamepads();
      if (gamepads.length > deviceIndex && deviceIndex >= 0) {
        return gamepads[deviceIndex];
      }
      return null;
    },
  },

  SDL_Linked_Version__proxy: 'sync',
  SDL_Linked_Version: () => {
    if (SDL.version === null) {
      SDL.version = _malloc({{{ C_STRUCTS.SDL_version.__size__ }}});
      {{{ makeSetValue('SDL.version', C_STRUCTS.SDL_version.major, '1', 'i8') }}};
      {{{ makeSetValue('SDL.version', C_STRUCTS.SDL_version.minor, '3', 'i8') }}};
      {{{ makeSetValue('SDL.version', C_STRUCTS.SDL_version.patch, '0', 'i8') }}};
    }
    return SDL.version;
  },

  SDL_Init__deps: ['calloc', 'memcpy'],
  SDL_Init__proxy: 'sync',
  SDL_Init__docs: '/** @param{number} initFlags */',
  SDL_Init: (initFlags) => {
    SDL.startTime = Date.now();
    SDL.initFlags = initFlags;

    // capture all key events. we just keep down and up, but also capture press to prevent default actions
    if (!Module['doNotCaptureKeyboard']) {
      var keyboardListeningElement = Module['keyboardListeningElement'] || document;
      keyboardListeningElement.addEventListener("keydown", SDL.receiveEvent);
      keyboardListeningElement.addEventListener("keyup", SDL.receiveEvent);
      keyboardListeningElement.addEventListener("keypress", SDL.receiveEvent);
      window.addEventListener("focus", SDL.receiveEvent);
      window.addEventListener("blur", SDL.receiveEvent);
      document.addEventListener("visibilitychange", SDL.receiveEvent);
    }

    window.addEventListener("unload", SDL.receiveEvent);
    SDL.keyboardState = _calloc(0x10000, 1); // Our SDL needs 512, but 64K is safe for older SDLs
    // Initialize this structure carefully for closure
    SDL.DOMEventToSDLEvent['keydown']    = 0x300  /* SDL_KEYDOWN */;
    SDL.DOMEventToSDLEvent['keyup']      = 0x301  /* SDL_KEYUP */;
    SDL.DOMEventToSDLEvent['keypress']   = 0x303  /* SDL_TEXTINPUT */;
    SDL.DOMEventToSDLEvent['mousedown']  = 0x401  /* SDL_MOUSEBUTTONDOWN */;
    SDL.DOMEventToSDLEvent['mouseup']    = 0x402  /* SDL_MOUSEBUTTONUP */;
    SDL.DOMEventToSDLEvent['mousemove']  = 0x400  /* SDL_MOUSEMOTION */;
    SDL.DOMEventToSDLEvent['wheel']      = 0x403  /* SDL_MOUSEWHEEL */;
    SDL.DOMEventToSDLEvent['touchstart'] = 0x700  /* SDL_FINGERDOWN */;
    SDL.DOMEventToSDLEvent['touchend']   = 0x701  /* SDL_FINGERUP */;
    SDL.DOMEventToSDLEvent['touchmove']  = 0x702  /* SDL_FINGERMOTION */;
    SDL.DOMEventToSDLEvent['unload']     = 0x100  /* SDL_QUIT */;
    SDL.DOMEventToSDLEvent['resize']     = 0x7001 /* SDL_VIDEORESIZE/SDL_EVENT_COMPAT2 */;
    SDL.DOMEventToSDLEvent['visibilitychange'] = 0x200 /* SDL_WINDOWEVENT */;
    SDL.DOMEventToSDLEvent['focus']      = 0x200 /* SDL_WINDOWEVENT */;
    SDL.DOMEventToSDLEvent['blur']       = 0x200 /* SDL_WINDOWEVENT */;

    // These are not technically DOM events; the HTML gamepad API is poll-based.
    // However, we define them here, as the rest of the SDL code assumes that
    // all SDL events originate as DOM events.
    SDL.DOMEventToSDLEvent['joystick_axis_motion'] = 0x600 /* SDL_JOYAXISMOTION */;
    SDL.DOMEventToSDLEvent['joystick_button_down'] = 0x603 /* SDL_JOYBUTTONDOWN */;
    SDL.DOMEventToSDLEvent['joystick_button_up'] = 0x604 /* SDL_JOYBUTTONUP */;
    return 0; // success
  },

  SDL_WasInit__deps: ['SDL_Init'],
  SDL_WasInit__proxy: 'sync',
  SDL_WasInit: (flags) => {
    if (SDL.startTime === null) {
      _SDL_Init(0);
    }
    return 1;
  },

  SDL_GetVideoInfo__deps: ['calloc'],
  SDL_GetVideoInfo__proxy: 'sync',
  SDL_GetVideoInfo: () => {
    var ret = _calloc({{{ C_STRUCTS.SDL_VideoInfo.__size__ }}}, 1);
    {{{ makeSetValue('ret', C_STRUCTS.SDL_VideoInfo.current_w, 'Module["canvas"].width', 'i32') }}};
    {{{ makeSetValue('ret', C_STRUCTS.SDL_VideoInfo.current_h, 'Module["canvas"].height', 'i32') }}};
    return ret;
  },

  // -1 == all modes are ok. TODO
  SDL_ListModes: (format, flags) => -1,

  // SDL_VideoModeOK returns 0 if the requested mode is not supported under any bit depth, or returns the
  // bits-per-pixel of the closest available mode with the given width, height and requested surface flags
  SDL_VideoModeOK: (width, height, depth, flags) => depth, // all modes are ok.

  SDL_AudioDriverName: 'SDL_VideoDriverName',

  SDL_VideoDriverName__proxy: 'sync',
  SDL_VideoDriverName: (buf, max_size) => {
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

  SDL_SetVideoMode__deps: ['$GL'],
  SDL_SetVideoMode__proxy: 'sync',
  SDL_SetVideoMode: (width, height, depth, flags) => {
    ['touchstart', 'touchend', 'touchmove',
     'mousedown', 'mouseup', 'mousemove',
     'mousewheel', 'wheel', 'mouseout',
     'DOMMouseScroll',
    ].forEach((e) => Module['canvas'].addEventListener(e, SDL.receiveEvent, true));

    var canvas = Module['canvas'];

    // (0,0) means 'use fullscreen' in native; in Emscripten, use the current canvas size.
    if (width == 0 && height == 0) {
      width = canvas.width;
      height = canvas.height;
    }

    if (!SDL.addedResizeListener) {
      SDL.addedResizeListener = true;
      Browser.resizeListeners.push((w, h) => {
        if (!SDL.settingVideoMode) {
          SDL.receiveEvent({
            type: 'resize',
            w,
            h
          });
        }
      });
    }

    SDL.settingVideoMode = true; // SetVideoMode itself should not trigger resize events
    Browser.setCanvasSize(width, height);
    SDL.settingVideoMode = false;

    // Free the old surface first if there is one
    if (SDL.screen) {
      SDL.freeSurface(SDL.screen);
      assert(!SDL.screen);
    }

    if (SDL.GL) flags = flags | 0x04000000; // SDL_OPENGL - if we are using GL, then later calls to SetVideoMode may not mention GL, but we do need it. Once in GL mode, we never leave it.

    SDL.screen = SDL.makeSurface(width, height, flags, true, 'screen');

    return SDL.screen;
  },

  SDL_GetVideoSurface__proxy: 'sync',
  SDL_GetVideoSurface: () => SDL.screen,

  SDL_AudioQuit__proxy: 'sync',
  SDL_AudioQuit: () => {
    for (var i = 0; i < SDL.numChannels; ++i) {
      var chan = /** @type {{ audio: (HTMLMediaElement|undefined) }} */ (SDL.channels[i]);
      if (chan.audio) {
        chan.audio.pause();
        chan.audio = undefined;
      }
    }
    var audio = /** @type {HTMLMediaElement} */ (SDL.music.audio);
    audio?.pause();
    SDL.music.audio = undefined;
  },

  SDL_VideoQuit: () => out('SDL_VideoQuit called (and ignored)'),

  SDL_QuitSubSystem: (flags) => out('SDL_QuitSubSystem called (and ignored)'),

  SDL_Quit__deps: ['SDL_AudioQuit'],
  SDL_Quit: () => {
    _SDL_AudioQuit();
    out('SDL_Quit called (and ignored)');
  },

  // Copy data from the canvas backing to a C++-accessible storage
  SDL_LockSurface__proxy: 'sync',
  SDL_LockSurface: (surf) => {
    var surfData = SDL.surfaces[surf];

    surfData.locked++;
    if (surfData.locked > 1) return 0;

    if (!surfData.buffer) {
      surfData.buffer = _malloc(surfData.width * surfData.height * 4);
      {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.pixels, 'surfData.buffer', POINTER_TYPE) }}};
    }

    // Mark in C/C++-accessible SDL structure
    // SDL_Surface has the following fields: Uint32 flags, SDL_PixelFormat *format; int w, h; Uint16 pitch; void *pixels; ...
    // So we have fields all of the same size, and 5 of them before us.
    // TODO: Use macros like in library.js
    {{{ makeSetValue('surf', C_STRUCTS.SDL_Surface.pixels, 'surfData.buffer', POINTER_TYPE) }}};

    if (surf == SDL.screen && Module.screenIsReadOnly && surfData.image) return 0;

    if (SDL.defaults.discardOnLock) {
      if (!surfData.image) {
        surfData.image = surfData.ctx.createImageData(surfData.width, surfData.height);
      }
      if (!SDL.defaults.opaqueFrontBuffer) return;
    } else {
      surfData.image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
    }

    // Emulate desktop behavior and kill alpha values on the locked surface. (very costly!) Set SDL.defaults.opaqueFrontBuffer = false
    // if you don't want this.
    if (surf == SDL.screen && SDL.defaults.opaqueFrontBuffer) {
      var data = surfData.image.data;
      var num = data.length;
      for (var i = 0; i < num/4; i++) {
        data[i*4+3] = 255; // opacity, as canvases blend alpha
      }
    }

    if (SDL.defaults.copyOnLock && !SDL.defaults.discardOnLock) {
      // Copy pixel data to somewhere accessible to 'C/C++'
      if (surfData.isFlagSet(0x00200000 /* SDL_HWPALETTE */)) {
        // If this is needed then
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
        HEAPU8.set(surfData.image.data, surfData.buffer);
      }
    }

    return 0;
  },

  // Copy data from the C++-accessible storage to the canvas backing
  SDL_UnlockSurface__proxy: 'sync',
  SDL_UnlockSurface: (surf) => {
    assert(!SDL.GL); // in GL mode we do not keep around 2D canvases and contexts

    var surfData = SDL.surfaces[surf];

    if (!surfData.locked || --surfData.locked > 0) {
      return;
    }

    // Copy pixel data to image
    if (surfData.isFlagSet(0x00200000 /* SDL_HWPALETTE */)) {
      SDL.copyIndexedColorData(surfData);
    } else if (!surfData.colors) {
      var data = surfData.image.data;
      var buffer = surfData.buffer;
      assert(buffer % 4 == 0, 'Invalid buffer offset: ' + buffer);
      var src = {{{ getHeapOffset('buffer', 'i32') }}};
      var dst = 0;
      var isScreen = surf == SDL.screen;
      var num;
      if (typeof CanvasPixelArray != 'undefined' && data instanceof CanvasPixelArray) {
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
        if (isScreen && SDL.defaults.opaqueFrontBuffer) {
          num = data32.length;
          // logically we need to do
          //      while (dst < num) {
          //          data32[dst++] = HEAP32[src++] | 0xff000000
          //      }
          // the following code is faster though, because
          // .set() is almost free - easily 10x faster due to
          // native memcpy efficiencies, and the remaining loop
          // just stores, not load + store, so it is faster
          data32.set(HEAP32.subarray(src, src + num));
          var data8 = new Uint8Array(data.buffer);
          var i = 3;
          var j = i + 4*num;
          if (num % 8 == 0) {
            // unrolling gives big speedups
            while (i < j) {
              data8[i] = 0xff;
              i = i + 4 | 0;
              data8[i] = 0xff;
              i = i + 4 | 0;
              data8[i] = 0xff;
              i = i + 4 | 0;
              data8[i] = 0xff;
              i = i + 4 | 0;
              data8[i] = 0xff;
              i = i + 4 | 0;
              data8[i] = 0xff;
              i = i + 4 | 0;
              data8[i] = 0xff;
              i = i + 4 | 0;
              data8[i] = 0xff;
              i = i + 4 | 0;
            }
           } else {
            while (i < j) {
              data8[i] = 0xff;
              i = i + 4 | 0;
            }
          }
        } else {
          data32.set(HEAP32.subarray(src, src + data32.length));
        }
      }
    } else {
      var width = Module['canvas'].width;
      var height = Module['canvas'].height;
      var s = surfData.buffer;
      var data = surfData.image.data;
      var colors = surfData.colors; // TODO: optimize using colors32
      for (var y = 0; y < height; y++) {
        var base = y*width*4;
        for (var x = 0; x < width; x++) {
          // See comment above about signs
          var val = {{{ makeGetValue('s++', 0, 'u8') }}} * 4;
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

  SDL_Flip: (surf) => {
    // We actually do this in Unlock, since the screen surface has as its canvas
    // backing the page canvas element
  },

  SDL_UpdateRect: (surf, x, y, w, h) => {
    // We actually do the whole screen in Unlock...
  },

  SDL_UpdateRects: (surf, numrects, rects) => {
    // We actually do the whole screen in Unlock...
  },

#if !ASYNCIFY
  SDL_Delay: (delay) => {
    if (!ENVIRONMENT_IS_WORKER) abort('SDL_Delay called on the main thread! Potential infinite loop, quitting. (consider building with async support like ASYNCIFY)');
    // horrible busy-wait, but in a worker it at least does not block rendering
    var now = Date.now();
    while (Date.now() - now < delay) {}
  },
#else
  SDL_Delay__deps: ['emscripten_sleep'],
  SDL_Delay__async: true,
  SDL_Delay: (delay) => _emscripten_sleep(delay),
#endif

  SDL_WM_SetCaption__proxy: 'sync',
  SDL_WM_SetCaption__deps: ['emscripten_set_window_title'],
  SDL_WM_SetCaption: (title, icon) => {
    if (title) {
      _emscripten_set_window_title(title);
    }
    icon &&= UTF8ToString(icon);
  },

  // TODO
  SDL_EnableKeyRepeat: (delay, interval) => {},

  SDL_GetKeyboardState__proxy: 'sync',
  SDL_GetKeyboardState__docs: '/** @param {number} numKeys */',
  SDL_GetKeyboardState: (numKeys) => {
    if (numKeys) {
      {{{ makeSetValue('numKeys', 0, 0x10000, 'i32') }}};
    }
    return SDL.keyboardState;
  },

  SDL_GetKeyState__deps: ['SDL_GetKeyboardState'],
  SDL_GetKeyState: () => _SDL_GetKeyboardState(0),

  SDL_GetKeyName__proxy: 'sync',
  SDL_GetKeyName__deps: ['$stringToNewUTF8'],
  SDL_GetKeyName: (key) => {
    SDL.keyName ||= stringToNewUTF8('unknown key');
    return SDL.keyName;
  },

  SDL_GetModState__proxy: 'sync',
  SDL_GetModState: () => SDL.modState,

  SDL_GetMouseState__proxy: 'sync',
  SDL_GetMouseState: (x, y) => {
    if (x) {{{ makeSetValue('x', 0, 'Browser.mouseX', 'i32') }}};
    if (y) {{{ makeSetValue('y', 0, 'Browser.mouseY', 'i32') }}};
    return SDL.buttonState;
  },

  SDL_WarpMouse__proxy: 'sync',
  SDL_WarpMouse: (x, y) => {
    return; // TODO: implement this in a non-buggy way. Need to keep relative mouse movements correct after calling this
    /*
    var rect = Module["canvas"].getBoundingClientRect();
    SDL.events.push({
      type: 'mousemove',
      pageX: x + (window.scrollX + rect.left),
      pageY: y + (window.scrollY + rect.top)
    });
    */
  },

  SDL_ShowCursor__proxy: 'sync',
  SDL_ShowCursor: (toggle) => {
    switch (toggle) {
      case 0: // SDL_DISABLE
        if (Browser.isFullscreen) { // only try to lock the pointer when in full screen mode
          Module['canvas'].requestPointerLock();
          return 0;
        }
        // else return SDL_ENABLE to indicate the failure
        return 1;
      case 1: // SDL_ENABLE
        Module['canvas'].exitPointerLock();
        return 1;
      case -1: // SDL_QUERY
        return !Browser.pointerLock;
      default:
        err(`SDL_ShowCursor called with unknown toggle parameter value: ${toggle}`);
        break;
    }
  },

  SDL_GetError__proxy: 'sync',
  SDL_GetError__deps: ['$stringToNewUTF8'],
  SDL_GetError: () => {
    SDL.errorMessage ||= stringToNewUTF8("unknown SDL-emscripten error");
    return SDL.errorMessage;
  },

  SDL_SetError: (fmt, varargs) => {},

  SDL_CreateRGBSurface__proxy: 'sync',
  SDL_CreateRGBSurface: (flags, width, height, depth, rmask, gmask, bmask, amask) => SDL.makeSurface(width, height, flags, false, 'CreateRGBSurface', rmask, gmask, bmask, amask),

  SDL_CreateRGBSurfaceFrom__proxy: 'sync',
  SDL_CreateRGBSurfaceFrom: (pixels, width, height, depth, pitch, rmask, gmask, bmask, amask) => {
    var surf = SDL.makeSurface(width, height, 0, false, 'CreateRGBSurfaceFrom', rmask, gmask, bmask, amask);

    if (depth !== 32) {
      // TODO: Actually fill pixel data to created surface.
      // TODO: Take into account depth and pitch parameters.
      err('TODO: Partially unimplemented SDL_CreateRGBSurfaceFrom called!');
      return surf;
    }

    var data = SDL.surfaces[surf];
    var image = data.ctx.createImageData(width, height);
    var pitchOfDst = width * 4;

    for (var row = 0; row < height; ++row) {
      var baseOfSrc = row * pitch;
      var baseOfDst = row * pitchOfDst;

      for (var col = 0; col < width * 4; ++col) {
        image.data[baseOfDst + col] = {{{ makeGetValue('pixels', 'baseOfDst + col', 'u8') }}};
      }
    }

    data.ctx.putImageData(image, 0, 0);

    return surf;
  },

  SDL_ConvertSurface__proxy: 'sync',
  SDL_ConvertSurface__docs: '/** @param {number} format @param {number} flags */',
  SDL_ConvertSurface: (surf, format, flags) => {
    if  (format) {
      SDL.checkPixelFormat(format);
    }

    var oldData = SDL.surfaces[surf];
    var ret = SDL.makeSurface(oldData.width, oldData.height, oldData.flags, false, 'copy:' + oldData.source);
    var newData = SDL.surfaces[ret];

    newData.ctx.globalCompositeOperation = "copy";
    newData.ctx.drawImage(oldData.canvas, 0, 0);
    newData.ctx.globalCompositeOperation = oldData.ctx.globalCompositeOperation;
    return ret;
  },

  SDL_DisplayFormatAlpha__deps: ['SDL_ConvertSurface'],
  SDL_DisplayFormatAlpha: (surf) => _SDL_ConvertSurface(surf, 0, 0),

  SDL_FreeSurface__proxy: 'sync',
  SDL_FreeSurface: (surf) => {
    if (surf) SDL.freeSurface(surf);
  },

  SDL_UpperBlit__proxy: 'sync',
  SDL_UpperBlit: (src, srcrect, dst, dstrect) =>
    SDL.blitSurface(src, srcrect, dst, dstrect, false),

  SDL_UpperBlitScaled__proxy: 'sync',
  SDL_UpperBlitScaled: (src, srcrect, dst, dstrect) =>
    SDL.blitSurface(src, srcrect, dst, dstrect, true),

  SDL_LowerBlit: 'SDL_UpperBlit',
  SDL_LowerBlitScaled: 'SDL_UpperBlitScaled',

  SDL_GetClipRect__proxy: 'sync',
  SDL_GetClipRect: (surf, rect) => {
    assert(rect);

    var surfData = SDL.surfaces[surf];
    var r = surfData.clipRect || { x: 0, y: 0, w: surfData.width, h: surfData.height };
    SDL.updateRect(rect, r);
  },

  SDL_SetClipRect__proxy: 'sync',
  SDL_SetClipRect: (surf, rect) => {
    var surfData = SDL.surfaces[surf];

    if (rect) {
      surfData.clipRect = SDL.intersectionOfRects({ x: 0, y: 0, w: surfData.width, h: surfData.height }, SDL.loadRect(rect));
    } else {
      delete surfData.clipRect;
    }
  },

  SDL_FillRect__proxy: 'sync',
  SDL_FillRect: (surf, rect, color) => {
    var surfData = SDL.surfaces[surf];
    assert(!surfData.locked); // but we could unlock and re-lock if we must..

    if (surfData.isFlagSet(0x00200000 /* SDL_HWPALETTE */)) {
      //in SDL_HWPALETTE color is index (0..255)
      //so we should translate 1 byte value to
      //32 bit canvas
      color = surfData.colors32[color];
    }

    var r = rect ? SDL.loadRect(rect) : { x: 0, y: 0, w: surfData.width, h: surfData.height };

    if (surfData.clipRect) {
      r = SDL.intersectionOfRects(surfData.clipRect, r);

      if (rect) {
        SDL.updateRect(rect, r);
      }
    }

    surfData.ctx.save();
    surfData.ctx.fillStyle = SDL.translateColorToCSSRGBA(color);
    surfData.ctx.fillRect(r.x, r.y, r.w, r.h);
    surfData.ctx.restore();
    return 0;
  },

  zoomSurface: (src, x, y, smooth) => {
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
  rotozoomSurface: (src, angle, zoom, smooth) => {
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

  SDL_SetAlpha__proxy: 'sync',
  SDL_SetAlpha: (surf, flag, alpha) => {
    var surfData = SDL.surfaces[surf];
    surfData.alpha = alpha;

    if (!(flag & 0x00010000)) { // !SDL_SRCALPHA
      surfData.alpha = 255;
    }
  },

  SDL_SetColorKey: (surf, flag, key) => {
    // SetColorKey assigns one color to be rendered as transparent. I don't
    // think the canvas API allows for anything like this, and iterating through
    // each pixel to replace that color seems prohibitively expensive.
    warnOnce('SDL_SetColorKey is a no-op for performance reasons');
    return 0;
  },

  SDL_GetTicks__proxy: 'sync',
  SDL_GetTicks: () => (Date.now() - SDL.startTime)|0,

  SDL_PollEvent__proxy: 'sync',
  SDL_PollEvent: (ptr) => SDL.pollEvent(ptr),

  SDL_PushEvent__proxy: 'sync',
  SDL_PushEvent: (ptr) => {
    var copy = _malloc({{{ C_STRUCTS.SDL_KeyboardEvent.__size__ }}});
    _memcpy(copy, ptr, {{{ C_STRUCTS.SDL_KeyboardEvent.__size__ }}});
    SDL.events.push(copy);
    return 0;
  },

  SDL_PeepEvents__proxy: 'sync',
  SDL_PeepEvents: (events, requestedEventCount, action, from, to) => {
    switch (action) {
      case 2: { // SDL_GETEVENT
        // We only handle 1 event right now
        assert(requestedEventCount == 1);

        var index = 0;
        var retrievedEventCount = 0;
        // this should look through the entire queue until it has filled up the events
        // array
        while (index < SDL.events.length && retrievedEventCount < requestedEventCount) {
          var event = SDL.events[index];
          var type = SDL.DOMEventToSDLEvent[event.type];
          if (from <= type && type <= to) {
            if (SDL.makeCEvent(event, events) === false) {
              index++;
            } else {
              SDL.events.splice(index, 1);
              retrievedEventCount++;
            }
          } else {
            index++;
          }
        }
        return retrievedEventCount;
      }
      default: throw 'SDL_PeepEvents does not yet support that action: ' + action;
    }
  },

  SDL_PumpEvents__proxy: 'sync',
  SDL_PumpEvents: () => SDL.events.forEach(SDL.handleEvent),

  // An Emscripten-specific extension to SDL: Some browser APIs require that they are called from within an event handler function.
  // Allow recording a callback that will be called for each received event.
  emscripten_SDL_SetEventHandler__proxy: 'sync',
  emscripten_SDL_SetEventHandler: (handler, userdata) => {
    SDL.eventHandler = handler;
    SDL.eventHandlerContext = userdata;

    // All SDLEvents take the same amount of memory
    SDL.eventHandlerTemp ||= _malloc({{{ C_STRUCTS.SDL_KeyboardEvent.__size__ }}});
  },

  SDL_SetColors__proxy: 'sync',
  SDL_SetColors: (surf, colors, firstColor, nColors) => {
    var surfData = SDL.surfaces[surf];

    // we should create colors array
    // only once cause client code
    // often wants to change portion
    // of palette not all palette.
    if (!surfData.colors) {
      var buffer = new ArrayBuffer(256 * 4); // RGBA, A is unused, but faster this way
      surfData.colors = new Uint8Array(buffer);
      surfData.colors32 = new Uint32Array(buffer);
    }

    for (var i = 0; i < nColors; ++i) {
      var index = (firstColor + i) * 4;
      surfData.colors[index] = {{{ makeGetValue('colors', 'i*4', 'u8') }}};
      surfData.colors[index + 1] = {{{ makeGetValue('colors', 'i*4 + 1', 'u8') }}};
      surfData.colors[index + 2] = {{{ makeGetValue('colors', 'i*4 + 2', 'u8') }}};
      surfData.colors[index + 3] = 255; // opaque
    }

    return 1;
  },

  SDL_SetPalette__deps: ['SDL_SetColors'],
  SDL_SetPalette: (surf, flags, colors, firstColor, nColors) =>
    _SDL_SetColors(surf, colors, firstColor, nColors),

  SDL_MapRGB__proxy: 'sync',
  SDL_MapRGB: (fmt, r, g, b) => {
    SDL.checkPixelFormat(fmt);
    // We assume the machine is little-endian.
    return r&0xff|(g&0xff)<<8|(b&0xff)<<16|0xff000000;
  },

  SDL_MapRGBA__proxy: 'sync',
  SDL_MapRGBA: (fmt, r, g, b, a) => {
    SDL.checkPixelFormat(fmt);
    // We assume the machine is little-endian.
    return r&0xff|(g&0xff)<<8|(b&0xff)<<16|(a&0xff)<<24;
  },

  SDL_GetRGB__proxy: 'sync',
  SDL_GetRGB: (pixel, fmt, r, g, b) => {
    SDL.checkPixelFormat(fmt);
    // We assume the machine is little-endian.
    if (r) {
      {{{ makeSetValue('r', 0, 'pixel&0xff', 'i8') }}};
    }
    if (g) {
      {{{ makeSetValue('g', 0, '(pixel>>8)&0xff', 'i8') }}};
    }
    if (b) {
      {{{ makeSetValue('b', 0, '(pixel>>16)&0xff', 'i8') }}};
    }
  },

  SDL_GetRGBA__proxy: 'sync',
  SDL_GetRGBA: (pixel, fmt, r, g, b, a) => {
    SDL.checkPixelFormat(fmt);
    // We assume the machine is little-endian.
    if (r) {
      {{{ makeSetValue('r', 0, 'pixel&0xff', 'i8') }}};
    }
    if (g) {
      {{{ makeSetValue('g', 0, '(pixel>>8)&0xff', 'i8') }}};
    }
    if (b) {
      {{{ makeSetValue('b', 0, '(pixel>>16)&0xff', 'i8') }}};
    }
    if (a) {
      {{{ makeSetValue('a', 0, '(pixel>>24)&0xff', 'i8') }}};
    }
  },

  SDL_GetAppState__proxy: 'sync',
  SDL_GetAppState: () => {
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

  SDL_WM_GrabInput: () => {},

  SDL_WM_ToggleFullScreen__proxy: 'sync',
  SDL_WM_ToggleFullScreen: (surf) => {
    if (Browser.exitFullscreen()) {
      return 1;
    }
    if (!SDL.canRequestFullscreen) {
      return 0;
    }
    SDL.isRequestingFullscreen = true;
    return 1;
  },

  // SDL_Image

  // We support JPG, PNG, TIF because browsers do
  IMG_Init: (flags) => flags,

  IMG_Load_RW__deps: ['SDL_LockSurface', 'SDL_FreeRW', '$PATH_FS', '$stackSave', '$stackRestore', '$stringToUTF8OnStack', '$stackAlloc'],
  IMG_Load_RW__proxy: 'sync',
  IMG_Load_RW: (rwopsID, freeSrc) => {
    var sp = stackSave();
    try {
      // stb_image integration support
      var cleanup = () => {
        stackRestore(sp);
        if (rwops && freeSrc) _SDL_FreeRW(rwopsID);
      }
      var addCleanup = (func) => {
        var old = cleanup;
        cleanup = () => {
          old();
          func();
        }
      }
      var callStbImage = (func, params) => {
        var x = stackAlloc({{{ getNativeTypeSize('i32') }}});
        var y = stackAlloc({{{ getNativeTypeSize('i32') }}});
        var comp = stackAlloc({{{ getNativeTypeSize('i32') }}});
        var data = Module['_' + func](...params, x, y, comp, 0);
        if (!data) return null;
        addCleanup(() => Module['_stbi_image_free'](data));
        return {
          rawData: true,
          data,
          width: {{{ makeGetValue('x', 0, 'i32') }}},
          height: {{{ makeGetValue('y', 0, 'i32') }}},
          size: {{{ makeGetValue('x', 0, 'i32') }}} * {{{ makeGetValue('y', 0, 'i32') }}} * {{{ makeGetValue('comp', 0, 'i32') }}},
          bpp: {{{ makeGetValue('comp', 0, 'i32') }}}
        };
      };

      var rwops = SDL.rwops[rwopsID];
      if (rwops === undefined) {
        return 0;
      }

      var raw;
      var filename = rwops.filename;
      if (filename === undefined) {
#if STB_IMAGE
        raw = callStbImage('stbi_load_from_memory', [rwops.bytes, rwops.count]);
        if (!raw) return 0;
#else
        warnOnce('Only file names that have been preloaded are supported for IMG_Load_RW. Consider using STB_IMAGE=1 if you want synchronous image decoding (see settings.js), or package files with --use-preload-plugins');
        return 0;
#endif
      }

      if (!raw) {
        filename = PATH_FS.resolve(filename);
        raw = preloadedImages[filename];
        if (!raw) {
          if (raw === null) err('Trying to reuse preloaded image, but freePreloadedMediaOnUse is set!');
#if STB_IMAGE
          var name = stringToUTF8OnStack(filename);
          raw = callStbImage('stbi_load', [name]);
          if (!raw) return 0;
#else
          warnOnce('Cannot find preloaded image ' + filename);
          warnOnce('Cannot find preloaded image ' + filename + '. Consider using STB_IMAGE=1 if you want synchronous image decoding (see settings.js), or package files with --use-preload-plugins');
          return 0;
#endif
        } else if (Module['freePreloadedMediaOnUse']) {
          preloadedImages[filename] = null;
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
          // rgba
          imageData.data.set({{{ makeHEAPView('U8', 'raw.data', 'raw.data+raw.size') }}});
        } else if (raw.bpp == 3) {
          // rgb
          var pixels = raw.size/3;
          var data = imageData.data;
          var sourcePtr = raw.data;
          var destPtr = 0;
          for (var i = 0; i < pixels; i++) {
            data[destPtr++] = {{{ makeGetValue('sourcePtr++', 0, 'u8') }}};
            data[destPtr++] = {{{ makeGetValue('sourcePtr++', 0, 'u8') }}};
            data[destPtr++] = {{{ makeGetValue('sourcePtr++', 0, 'u8') }}};
            data[destPtr++] = 255;
          }
        } else if (raw.bpp == 2) {
          // grayscale + alpha
          var pixels = raw.size;
          var data = imageData.data;
          var sourcePtr = raw.data;
          var destPtr = 0;
          for (var i = 0; i < pixels; i++) {
            var gray = {{{ makeGetValue('sourcePtr++', 0, 'u8') }}};
            var alpha = {{{ makeGetValue('sourcePtr++', 0, 'u8') }}};
            data[destPtr++] = gray;
            data[destPtr++] = gray;
            data[destPtr++] = gray;
            data[destPtr++] = alpha;
          }
        } else if (raw.bpp == 1) {
          // grayscale
          var pixels = raw.size;
          var data = imageData.data;
          var sourcePtr = raw.data;
          var destPtr = 0;
          for (var i = 0; i < pixels; i++) {
            var value = {{{ makeGetValue('sourcePtr++', 0, 'u8') }}};
            data[destPtr++] = value;
            data[destPtr++] = value;
            data[destPtr++] = value;
            data[destPtr++] = 255;
          }
        } else {
          err(`cannot handle bpp ${raw.bpp}`);
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
  SDL_LoadBMP_RW: 'IMG_Load_RW',

  IMG_Load__deps: ['IMG_Load_RW', 'SDL_RWFromFile'],
  IMG_Load__proxy: 'sync',
  IMG_Load: (filename) => {
    var rwops = _SDL_RWFromFile(filename, 0);
    var result = _IMG_Load_RW(rwops, 1);
    return result;
  },

  IMG_Quit: () => out('IMG_Quit called (and ignored)'),

  // SDL_Audio

  SDL_OpenAudio__deps: ['$autoResumeAudioContext', '$safeSetTimeout', '$registerPostMainLoop'],
  SDL_OpenAudio__proxy: 'sync',
  SDL_OpenAudio__postset: `
    // Queue new audio data. This is important to be right after the main loop
    // invocation, so that we will immediately be able to queue the newest
    // produced audio samples.
    registerPostMainLoop(() => SDL.audio?.queueNewAudioData?.());`,
  SDL_OpenAudio: (desired, obtained) => {
    try {
      SDL.audio = {
        freq: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.freq, 'u32') }}},
        format: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.format, 'u16') }}},
        channels: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.channels, 'u8') }}},
        samples: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.samples, 'u16') }}}, // Samples in the CB buffer per single sound channel.
        callback: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.callback, '*') }}},
        userdata: {{{ makeGetValue('desired', C_STRUCTS.SDL_AudioSpec.userdata, '*') }}},
        paused: true,
        timer: null
      };
      // The .silence field tells the constant sample value that corresponds to the safe un-skewed silence value for the wave data.
      if (SDL.audio.format == {{{ cDefs.AUDIO_U8 }}}) {
        SDL.audio.silence = 128; // Audio ranges in [0, 255], so silence is half-way in between.
      } else if (SDL.audio.format == {{{ cDefs.AUDIO_S16LSB }}}) {
        SDL.audio.silence = 0; // Signed data in range [-32768, 32767], silence is 0.
      } else if (SDL.audio.format == {{{ cDefs.AUDIO_F32 }}}) {
        SDL.audio.silence = 0.0; // Float data in range [-1.0, 1.0], silence is 0.0
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
        throw `Unsupported sound frequency ${SDL.audio.freq}!`;
      }
      if (SDL.audio.channels == 0) {
        SDL.audio.channels = 1; // In SDL both 0 and 1 mean mono.
      } else if (SDL.audio.channels < 0 || SDL.audio.channels > 32) {
        throw `Unsupported number of audio channels for SDL audio: ${SDL.audio.channels}!`;
      } else if (SDL.audio.channels != 1 && SDL.audio.channels != 2) { // Unsure what SDL audio spec supports. Web Audio spec supports up to 32 channels.
        out(`Warning: Using untested number of audio channels ${SDL.audio.channels}`);
      }
      if (SDL.audio.samples < 128 || SDL.audio.samples > 524288 /* arbitrary cap */) {
        throw `Unsupported audio callback buffer size ${SDL.audio.samples}!`;
      } else if ((SDL.audio.samples & (SDL.audio.samples-1)) != 0) {
        throw `Audio callback buffer size ${SDL.audio.samples} must be a power-of-two!`;
      }

      var totalSamples = SDL.audio.samples*SDL.audio.channels;
      if (SDL.audio.format == {{{ cDefs.AUDIO_U8 }}}) {
        SDL.audio.bytesPerSample = 1;
      } else if (SDL.audio.format == {{{ cDefs.AUDIO_S16LSB }}}) {
        SDL.audio.bytesPerSample = 2;
      } else if (SDL.audio.format == {{{ cDefs.AUDIO_F32 }}}) {
        SDL.audio.bytesPerSample = 4;
      } else {
        throw `Invalid SDL audio format ${SDL.audio.format}!`;
      }
      SDL.audio.bufferSize = totalSamples*SDL.audio.bytesPerSample;
      // Duration of a single queued buffer in seconds.
      SDL.audio.bufferDurationSecs = SDL.audio.bufferSize / SDL.audio.bytesPerSample / SDL.audio.channels / SDL.audio.freq;
      // Audio samples are played with a constant delay of this many seconds to account for browser and jitter.
      SDL.audio.bufferingDelay = 50 / 1000;
      SDL.audio.buffer = _malloc(SDL.audio.bufferSize);

      // To account for jittering in frametimes, always have multiple audio
      // buffers queued up for the audio output device.
      // This helps that we won't starve that easily if a frame takes long to complete.
      SDL.audio.numSimultaneouslyQueuedBuffers = Module['SDL_numSimultaneouslyQueuedBuffers'] || 5;

      // Pulls and queues new audio data if appropriate. This function gets
      // "over-called" in both requestAnimationFrames and setTimeouts to ensure
      // that we get the finest granularity possible and as many chances from
      // the browser to fill new audio data. This is because setTimeouts alone
      // have very poor granularity for audio streaming purposes, but also the
      // application might not be using emscripten_set_main_loop to drive the
      // main loop, so we cannot rely on that alone.
      SDL.audio.queueNewAudioData = () => {
        if (!SDL.audio) return;

        for (var i = 0; i < SDL.audio.numSimultaneouslyQueuedBuffers; ++i) {
          // Only queue new data if we don't have enough audio data already in queue. Otherwise skip this time slot
          // and wait to queue more in the next time the callback is run.
          var secsUntilNextPlayStart = SDL.audio.nextPlayTime - SDL.audioContext['currentTime'];
          if (secsUntilNextPlayStart >= SDL.audio.bufferingDelay + SDL.audio.bufferDurationSecs*SDL.audio.numSimultaneouslyQueuedBuffers) return;

          // Ask SDL audio data from the user code.
          {{{ makeDynCall('vppi', 'SDL.audio.callback') }}}(SDL.audio.userdata, SDL.audio.buffer, SDL.audio.bufferSize);
          // And queue it to be played after the currently playing audio stream.
          SDL.audio.pushAudio(SDL.audio.buffer, SDL.audio.bufferSize);
        }
      }

#if ASYNCIFY
      var sleepCallback = () => {
        SDL.audio?.queueNewAudioData?.();
      };
      Asyncify.sleepCallbacks.push(sleepCallback);
      SDL.audio.callbackRemover = () => {
        Asyncify.sleepCallbacks = Asyncify.sleepCallbacks.filter((callback) => callback !== sleepCallback);
      }
#endif

      // Create a callback function that will be routinely called to ask more audio data from the user application.
      SDL.audio.caller = () => {
        if (!SDL.audio) return;

        --SDL.audio.numAudioTimersPending;

        SDL.audio.queueNewAudioData();

        // Queue this callback function to be called again later to pull more audio data.
        var secsUntilNextPlayStart = SDL.audio.nextPlayTime - SDL.audioContext['currentTime'];

        // Queue the next audio frame push to be performed half-way when the previously queued buffer has finished playing.
        var preemptBufferFeedSecs = SDL.audio.bufferDurationSecs/2.0;

        if (SDL.audio.numAudioTimersPending < SDL.audio.numSimultaneouslyQueuedBuffers) {
          ++SDL.audio.numAudioTimersPending;
          SDL.audio.timer = safeSetTimeout(SDL.audio.caller, Math.max(0.0, 1000.0*(secsUntilNextPlayStart-preemptBufferFeedSecs)));

          // If we are risking starving, immediately queue an extra buffer.
          if (SDL.audio.numAudioTimersPending < SDL.audio.numSimultaneouslyQueuedBuffers) {
            ++SDL.audio.numAudioTimersPending;
            safeSetTimeout(SDL.audio.caller, 1.0);
          }
        }
      };

      SDL.audio.audioOutput = new Audio();

      // Initialize Web Audio API if we haven't done so yet. Note: Only initialize Web Audio context ever once on the web page,
      // since initializing multiple times fails on Chrome saying 'audio resources have been exhausted'.
      SDL.openAudioContext();
      if (!SDL.audioContext) throw 'Web Audio API is not available!';
      autoResumeAudioContext(SDL.audioContext);
      SDL.audio.nextPlayTime = 0; // Time in seconds when the next audio block is due to start.

      // The pushAudio function with a new audio buffer whenever there is new
      // audio data to schedule to be played back on the device.
      SDL.audio.pushAudio = (ptr, sizeBytes) => {
        try {
          if (SDL.audio.paused) return;

          var sizeSamples = sizeBytes / SDL.audio.bytesPerSample; // How many samples fit in the callback buffer?
          var sizeSamplesPerChannel = sizeSamples / SDL.audio.channels; // How many samples per a single channel fit in the cb buffer?
          if (sizeSamplesPerChannel != SDL.audio.samples) {
            throw 'Received mismatching audio buffer size!';
          }
          // Allocate new sound buffer to be played.
          var source = SDL.audioContext['createBufferSource']();
          var soundBuffer = SDL.audioContext['createBuffer'](SDL.audio.channels,sizeSamplesPerChannel,SDL.audio.freq);
          source['connect'](SDL.audioContext['destination']);

          SDL.fillWebAudioBufferFromHeap(ptr, sizeSamplesPerChannel, soundBuffer);
          // Workaround https://bugzilla.mozilla.org/show_bug.cgi?id=883675 by setting the buffer only after filling. The order is important here!
          source['buffer'] = soundBuffer;

          // Schedule the generated sample buffer to be played out at the correct time right after the previously scheduled
          // sample buffer has finished.
          var curtime = SDL.audioContext['currentTime'];
#if ASSERTIONS
          if (curtime > SDL.audio.nextPlayTime && SDL.audio.nextPlayTime != 0) {
            err(`warning: Audio callback had starved sending audio by ${curtime - SDL.audio.nextPlayTime} seconds`);
          }
#endif
          // Don't ever start buffer playbacks earlier from current time than a given constant 'SDL.audio.bufferingDelay', since a browser
          // may not be able to mix that audio clip in immediately, and there may be subsequent jitter that might cause the stream to starve.
          var playtime = Math.max(curtime + SDL.audio.bufferingDelay, SDL.audio.nextPlayTime);
          if (typeof source['start'] != 'undefined') {
            source['start'](playtime); // New Web Audio API: sound sources are started with a .start() call.
          } else if (typeof source['noteOn'] != 'undefined') {
            source['noteOn'](playtime); // Support old Web Audio API specification which had the .noteOn() API.
          }
          /*
          // Uncomment to debug SDL buffer feed starves.
          if (SDL.audio.curBufferEnd) {
            var thisBufferStart = Math.round(playtime * SDL.audio.freq);
            if (thisBufferStart != SDL.audio.curBufferEnd) out('SDL starved ' + (thisBufferStart - SDL.audio.curBufferEnd) + ' samples!');
          }
          SDL.audio.curBufferEnd = Math.round(playtime * SDL.audio.freq + sizeSamplesPerChannel);
          */

          SDL.audio.nextPlayTime = playtime + SDL.audio.bufferDurationSecs;
        } catch(e) {
          err(`Web Audio API error playing back audio: ${e.toString()}`);
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
      err(`Initializing SDL audio threw an exception: "${e.toString()}"! Continuing without audio`);
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

  SDL_PauseAudio__proxy: 'sync',
  SDL_PauseAudio__deps: ['$safeSetTimeout'],
  SDL_PauseAudio: (pauseOn) => {
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
      SDL.audio.timer = safeSetTimeout(SDL.audio.caller, 1);
    }
    SDL.audio.paused = pauseOn;
  },

  SDL_CloseAudio__deps: ['SDL_PauseAudio'],
  SDL_CloseAudio__proxy: 'sync',
  SDL_CloseAudio: () => {
    if (SDL.audio) {
      if (SDL.audio.callbackRemover) {
        SDL.audio.callbackRemover();
        SDL.audio.callbackRemover = null;
      }
      _SDL_PauseAudio(1);
      _free(SDL.audio.buffer);
      SDL.audio = null;
      SDL.allocateChannels(0);
    }
  },

  SDL_LockAudio: () => {},
  SDL_UnlockAudio: () => {},

  SDL_CreateMutex: () => 0,
  SDL_mutexP: (mutex) => 0,
  SDL_mutexV: (mutex) => 0,
  SDL_DestroyMutex: (mutex) => {},

  SDL_CreateCond: () => 0,
  SDL_CondSignal: (cond) => {},
  SDL_CondWait: (cond, mutex) => {},
  SDL_DestroyCond: (cond) => {},

  SDL_StartTextInput__proxy: 'sync',
  SDL_StartTextInput: () => {
    SDL.textInput = true;
  },
  SDL_StopTextInput__proxy: 'sync',
  SDL_StopTextInput: () => {
    SDL.textInput = false;
  },

  // SDL Mixer

  Mix_Init: (flags) => {
    if (!flags) return 0;
    return 8; /* MIX_INIT_OGG */
  },
  Mix_Quit: () => {},

  Mix_OpenAudio__deps: ['$autoResumeAudioContext'],
  Mix_OpenAudio__proxy: 'sync',
  Mix_OpenAudio: (frequency, format, channels, chunksize) => {
    SDL.openAudioContext();
    autoResumeAudioContext(SDL.audioContext);
    SDL.allocateChannels(32);
    // Just record the values for a later call to Mix_QuickLoad_RAW
    SDL.mixerFrequency = frequency;
    SDL.mixerFormat = format;
    SDL.mixerNumChannels = channels;
    SDL.mixerChunkSize = chunksize;
    return 0;
  },

  Mix_CloseAudio: 'SDL_CloseAudio',

  Mix_AllocateChannels__proxy: 'sync',
  Mix_AllocateChannels: (num) => {
    SDL.allocateChannels(num);
    return num;
  },

  Mix_ChannelFinished__proxy: 'sync',
  Mix_ChannelFinished: (func) => {
    SDL.channelFinished = func;
  },

  Mix_Volume__proxy: 'sync',
  Mix_Volume: (channel, volume) => {
    if (channel == -1) {
      for (var i = 0; i < SDL.numChannels-1; i++) {
        _Mix_Volume(i, volume);
      }
      return _Mix_Volume(SDL.numChannels-1, volume);
    }
    return SDL.setGetVolume(SDL.channels[channel], volume);
  },

  // Note: Mix_SetPanning requires WebAudio (file loaded from memory).
  Mix_SetPanning__proxy: 'sync',
  Mix_SetPanning: (channel, left, right) => {
    // SDL API uses [0-255], while PannerNode has an (x, y, z) position.

    // Normalizing.
    left /= 255;
    right /= 255;

    // Set the z coordinate a little forward, otherwise there won't be any
    // smooth transition between left and right.
    SDL.setPannerPosition(SDL.channels[channel], right - left, 0, 0.1);
    return 1;
  },

  Mix_LoadWAV_RW__deps: [
    '$FS',
    '$PATH_FS',
    'fileno',
  ],
  Mix_LoadWAV_RW__proxy: 'sync',
  Mix_LoadWAV_RW__docs: '/** @param {number} freesrc */',
  Mix_LoadWAV_RW: (rwopsID, freesrc) => {
    var rwops = SDL.rwops[rwopsID];

#if USE_SDL == 2
    if (rwops === undefined) {
      var type = {{{ makeGetValue('rwopsID', C_STRUCTS.SDL_RWops.type, 'i32') }}};

      if (type === 2/*SDL_RWOPS_STDFILE*/) {
        var fp = {{{ makeGetValue('rwopsID', C_STRUCTS.SDL_RWops.hidden.stdio.fp, 'i32') }}};
        var fd = _fileno(fp);
        var stream = FS.getStream(fd);
        if (stream) {
          rwops = { filename: stream.path };
        }
      }
      else if (type === 4/*SDL_RWOPS_MEMORY*/ || type === 5/*SDL_RWOPS_MEMORY_RO*/) {
        var base = {{{ makeGetValue('rwopsID', C_STRUCTS.SDL_RWops.hidden.mem.base, 'i32') }}};
        var stop = {{{ makeGetValue('rwopsID', C_STRUCTS.SDL_RWops.hidden.mem.stop, 'i32') }}};

        rwops = { bytes: base, count: stop - base };
      }
    }
#endif

    if (rwops === undefined)
      return 0;

    var filename = '';
    var audio;
    var webAudio;
    var bytes;

    if (rwops.filename !== undefined) {
      filename = PATH_FS.resolve(rwops.filename);
      var raw = preloadedAudios[filename];
      if (!raw) {
        if (raw === null) err('Trying to reuse preloaded audio, but freePreloadedMediaOnUse is set!');
        if (!Module['noAudioDecoding']) warnOnce('Cannot find preloaded audio ' + filename);

        // see if we can read the file-contents from the in-memory FS
        try {
          bytes = FS.readFile(filename);
        } catch (e) {
          err(`Couldn't find file for: ${filename}`);
          return 0;
        }
      }
      if (Module['freePreloadedMediaOnUse']) {
        preloadedAudios[filename] = null;
      }
      audio = raw;
    }
    else if (rwops.bytes !== undefined) {
      // For Web Audio context buffer decoding, we must make a clone of the audio data, but for <media> element,
      // a view to existing data is sufficient.
      if (SDL.webAudioAvailable()) bytes = HEAPU8.buffer.slice(rwops.bytes, rwops.bytes + rwops.count);
      else bytes = HEAPU8.subarray(rwops.bytes, rwops.bytes + rwops.count);
    }
    else {
      return 0;
    }

    var arrayBuffer = bytes ? bytes.buffer || bytes : bytes;

    // To allow user code to work around browser bugs with audio playback on <audio> elements an Web Audio, enable
    // the user code to hook in a callback to decide on a file basis whether each file should use Web Audio or <audio> for decoding and playback.
    // In particular, see https://bugzilla.mozilla.org/show_bug.cgi?id=654787 and ?id=1012801 for tradeoffs.
    var canPlayWithWebAudio = Module['SDL_canPlayWithWebAudio'] === undefined || Module['SDL_canPlayWithWebAudio'](filename, arrayBuffer);

    if (bytes !== undefined && SDL.webAudioAvailable() && canPlayWithWebAudio) {
      audio = undefined;
      webAudio = {};
      // The audio decoding process is asynchronous, which gives trouble if user code plays the audio data back immediately
      // after loading. Therefore prepare an array of callback handlers to run when this audio decoding is complete, which
      // will then start the playback (with some delay).
      webAudio.onDecodeComplete = []; // While this member array exists, decoding hasn't finished yet.
      var onDecodeComplete = (data) => {
        webAudio.decodedBuffer = data;
        // Call all handlers that were waiting for this decode to finish, and clear the handler list.
        webAudio.onDecodeComplete.forEach((e) => e());
        webAudio.onDecodeComplete = undefined; // Don't allow more callback handlers since audio has finished decoding.
      };
      SDL.audioContext['decodeAudioData'](arrayBuffer, onDecodeComplete);
    } else if (audio === undefined && bytes) {
      // Here, we didn't find a preloaded audio but we either were passed a filepath for
      // which we loaded bytes, or we were passed some bytes
      var blob = new Blob([bytes], {type: rwops.mimetype});
      var url = URL.createObjectURL(blob);
      audio = new Audio();
      audio.src = url;
      audio.mozAudioChannelType = 'content'; // bugzilla 910340
    }

    var id = SDL.audios.length;
    // Keep the loaded audio in the audio arrays, ready for playback
    SDL.audios.push({
      source: filename,
      audio, // Points to the <audio> element, if loaded
      webAudio // Points to a Web Audio -specific resource object, if loaded
    });
    return id;
  },

  Mix_LoadWAV__deps: ['Mix_LoadWAV_RW', 'SDL_RWFromFile', 'SDL_FreeRW'],
  Mix_LoadWAV__proxy: 'sync',
  Mix_LoadWAV: (filename) => {
    var rwops = _SDL_RWFromFile(filename, 0);
    var result = _Mix_LoadWAV_RW(rwops, 0);
    _SDL_FreeRW(rwops);
    return result;
  },

  Mix_QuickLoad_RAW__proxy: 'sync',
  Mix_QuickLoad_RAW: (mem, len) => {
    var audio;
    var webAudio;

    var numSamples = len >> 1; // len is the length in bytes, and the array contains 16-bit PCM values
    var buffer = new Float32Array(numSamples);
    for (var i = 0; i < numSamples; ++i) {
      buffer[i] = ({{{ makeGetValue('mem', 'i*2', 'i16') }}}) / 0x8000; // hardcoded 16-bit audio, signed (TODO: reSign if not ta2?)
    }

    if (SDL.webAudioAvailable()) {
      webAudio = {};
      webAudio.decodedBuffer = buffer;
    } else {
      audio = new Audio();
      audio.mozAudioChannelType = 'content'; // bugzilla 910340
      // Record the number of channels and frequency for later usage
      audio.numChannels = SDL.mixerNumChannels;
      audio.frequency = SDL.mixerFrequency;
      // FIXME: doesn't make sense to keep the audio element in the buffer
    }

    var id = SDL.audios.length;
    SDL.audios.push({
      source: '',
      audio,
      webAudio,
      buffer
    });
    return id;
  },

  Mix_FreeChunk__proxy: 'sync',
  Mix_FreeChunk: (id) => {
    SDL.audios[id] = null;
  },
  Mix_ReserveChannels__proxy: 'sync',
  Mix_ReserveChannels: (num) => {
    SDL.channelMinimumNumber = num;
  },
  Mix_PlayChannelTimed__deps: ['Mix_HaltChannel'],
  Mix_PlayChannelTimed__proxy: 'sync',
  Mix_PlayChannelTimed: (channel, id, loops, ticks) => {
    // TODO: handle fixed amount of N loops. Currently loops either 0 or infinite times.
    assert(ticks == -1);

    // Get the audio element associated with the ID
    var info = SDL.audios[id];
    if (!info) return -1;
    if (!info.audio && !info.webAudio) return -1;

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
        err(`All ${SDL.numChannels}  channels in use!`);
        return -1;
      }
    }
    var channelInfo = SDL.channels[channel];
    var audio;
    if (info.webAudio) {
      // Create an instance of the WebAudio object.
      audio = {};
      audio.resource = info; // This new object is an instance that refers to this existing resource.
      audio.paused = false;
      audio.currentPosition = 0;
      // Make our instance look similar to the instance of a <media> to make api simple.
      audio.play = function() { SDL.playWebAudio(this); }
      audio.pause = function() { SDL.pauseWebAudio(this); }
    } else {
      // We clone the audio node to utilize the preloaded audio buffer, since
      // the browser has already preloaded the audio file.
      audio = info.audio.cloneNode(true);
      audio.numChannels = info.audio.numChannels;
      audio.frequency = info.audio.frequency;
    }
    audio['onended'] = function() { // TODO: cache these
      if (channelInfo.audio === this || channelInfo.audio.webAudioNode === this) { 
        channelInfo.audio.paused = true; channelInfo.audio = null; 
      }
      if (SDL.channelFinished)  {{{ makeDynCall('vi', 'SDL.channelFinished') }}}(channel);
    }
    if (channelInfo.audio) {
      _Mix_HaltChannel(channel);
    }
    channelInfo.audio = audio;
    // TODO: handle N loops. Behavior matches Mix_PlayMusic
    audio.loop = loops != 0;
    audio.volume = channelInfo.volume;
    audio.play();
    return channel;
  },

  Mix_FadingChannel: (channel) => 0, // MIX_NO_FADING, TODO

  Mix_HaltChannel__proxy: 'sync',
  Mix_HaltChannel: (channel) => {
    function halt(channel) {
      var info = /** @type {{ audio: HTMLMediaElement }} */ (SDL.channels[channel]);
      if (info.audio) {
        info.audio.pause();
        info.audio = null;
      }
      if (SDL.channelFinished) {
        {{{ makeDynCall('vi', 'SDL.channelFinished') }}}(channel);
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
  Mix_HookMusicFinished__proxy: 'sync',
  Mix_HookMusicFinished: (func) => {
    SDL.hookMusicFinished = func;
    if (SDL.music.audio) { // ensure the callback will be called, if a music is already playing
      SDL.music.audio['onended'] = _Mix_HaltMusic;
    }
  },

  Mix_VolumeMusic__proxy: 'sync',
  Mix_VolumeMusic: (volume) => {
    return SDL.setGetVolume(SDL.music, volume);
  },

  Mix_LoadMUS_RW__deps: ['Mix_LoadWAV_RW'],
  Mix_LoadMUS_RW: (filename) => _Mix_LoadWAV_RW(filename, 0),

  Mix_LoadMUS__deps: ['Mix_LoadMUS_RW', 'SDL_RWFromFile', 'SDL_FreeRW'],
  Mix_LoadMUS__proxy: 'sync',
  Mix_LoadMUS: (filename) => {
    var rwops = _SDL_RWFromFile(filename, 0);
    var result = _Mix_LoadMUS_RW(rwops);
    _SDL_FreeRW(rwops);
    return result;
  },

  Mix_FreeMusic: 'Mix_FreeChunk',

  Mix_PlayMusic__deps: ['Mix_HaltMusic'],
  Mix_PlayMusic__proxy: 'sync',
  Mix_PlayMusic: (id, loops) => {
    // Pause old music if it exists.
    if (SDL.music.audio) {
      if (!SDL.music.audio.paused) err(`Music is already playing. ${SDL.music.source}`);
      SDL.music.audio.pause();
    }
    var info = SDL.audios[id];
    var audio;
    if (info.webAudio) { // Play via Web Audio API
      // Create an instance of the WebAudio object.
      audio = {};
      audio.resource = info; // This new webAudio object is an instance that refers to this existing resource.
      audio.paused = false;
      audio.currentPosition = 0;
      audio.play = function() { SDL.playWebAudio(this); }
      audio.pause = function() { SDL.pauseWebAudio(this); }
    } else if (info.audio) { // Play via the <audio> element
      audio = info.audio;
    }
    audio['onended'] = function() { 
      if (SDL.music.audio === this || SDL.music.audio?.webAudioNode === this) {
        _Mix_HaltMusic(); // will send callback
      }
    }
    audio.loop = loops != 0 && loops != 1; // TODO: handle N loops for finite N
    audio.volume = SDL.music.volume;
    SDL.music.audio = audio;
    audio.play();
    return 0;
  },

  Mix_PauseMusic__proxy: 'sync',
  Mix_PauseMusic: () => {
    var audio = /** @type {HTMLMediaElement} */ (SDL.music.audio);
    audio?.pause();
  },

  Mix_ResumeMusic__proxy: 'sync',
  Mix_ResumeMusic: () => {
    var audio = SDL.music.audio;
    audio?.play();
  },

  Mix_HaltMusic__proxy: 'sync',
  Mix_HaltMusic: () => {
    var audio = /** @type {HTMLMediaElement} */ (SDL.music.audio);
    if (audio) {
      audio.src = audio.src; // rewind <media> element
      audio.currentPosition = 0; // rewind Web Audio graph playback.
      audio.pause();
    }
    SDL.music.audio = null;
    if (SDL.hookMusicFinished) {
      {{{ makeDynCall('v', 'SDL.hookMusicFinished') }}}();
    }
    return 0;
  },

  Mix_FadeInMusicPos: 'Mix_PlayMusic', // XXX ignore fading in effect

  Mix_FadeOutMusic: 'Mix_HaltMusic', // XXX ignore fading out effect

  Mix_PlayingMusic__proxy: 'sync',
  Mix_PlayingMusic: () => {
    return (SDL.music.audio && !SDL.music.audio.paused) ? 1 : 0;
  },

  // http://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_38.html#SEC38
  // "Note: Does not check if the channel has been paused."
  Mix_Playing__proxy: 'sync',
  Mix_Playing: (channel) => {
    if (channel === -1) {
      var count = 0;
      for (var i = 0; i < SDL.channels.length; i++) {
        count += _Mix_Playing(i);
      }
      return count;
    }
    var info = SDL.channels[channel];
    if (info?.audio && !info.audio.paused) {
      return 1;
    }
    return 0;
  },

  Mix_Pause__proxy: 'sync',
  Mix_Pause: (channel) => {
    if (channel === -1) {
      for (var i = 0; i<SDL.channels.length;i++) {
        _Mix_Pause(i);
      }
      return;
    }
    /** @type {{ audio: HTMLMediaElement }} */
    var info = SDL.channels[channel];
    if (info?.audio) {
      info.audio.pause();
    } else {
      //err(`Mix_Pause: no sound found for channel: ${channel}`);
    }
  },

  // http://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_39.html#SEC39
  Mix_Paused__proxy: 'sync',
  Mix_Paused: (channel) => {
    if (channel === -1) {
      var pausedCount = 0;
      for (var i = 0; i<SDL.channels.length;i++) {
        pausedCount += _Mix_Paused(i);
      }
      return pausedCount;
    }
    var info = SDL.channels[channel];
    if (info?.audio?.paused) {
      return 1;
    }
    return 0;
  },

  Mix_PausedMusic__proxy: 'sync',
  Mix_PausedMusic: () => SDL.music.audio?.paused ? 1 : 0,

  // http://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer_33.html#SEC33
  Mix_Resume__proxy: 'sync',
  Mix_Resume: (channel) => {
    if (channel === -1) {
      for (var i = 0; i<SDL.channels.length;i++) {
        _Mix_Resume(i);
      }
      return;
    }
    var info = SDL.channels[channel];
    if (info?.audio) info.audio.play();
  },

  // SDL TTF

  TTF_Init__proxy: 'sync',
  TTF_Init: () => {
    // OffscreenCanvas 2D is faster than Canvas for text operations, so we use
    // it if it's available.
    try {
      var offscreenCanvas = new OffscreenCanvas(0, 0);
      SDL.ttfContext = offscreenCanvas.getContext('2d');
      // Firefox support for OffscreenCanvas is still experimental, and it seems
      // like CI might be creating a context here but one that is not entirely
      // valid. Check that explicitly and fall back to a plain Canvas if we need
      // to. See https://github.com/emscripten-core/emscripten/issues/16242
      if (typeof SDL.ttfContext.measureText != 'function') {
        throw 'bad context';
      }
    } catch (ex) {
      var canvas = /** @type {HTMLCanvasElement} */(document.createElement('canvas'));
      SDL.ttfContext = canvas.getContext('2d');
    }
#if ASSERTIONS
    // Check the final context looks valid. See
    // https://github.com/emscripten-core/emscripten/issues/16242
    assert(typeof SDL.ttfContext.measureText == 'function',
           'context ' + SDL.ttfContext + 'must provide valid methods');
#endif
    return 0;
  },

  TTF_OpenFont__proxy: 'sync',
  TTF_OpenFont: (name, size) => {
    name = PATH.normalize(UTF8ToString(name));
    var id = SDL.fonts.length;
    SDL.fonts.push({
      name, // but we don't actually do anything with it..
      size
    });
    return id;
  },

  TTF_CloseFont__proxy: 'sync',
  TTF_CloseFont: (font) => {
    SDL.fonts[font] = null;
  },

  TTF_RenderText_Solid__proxy: 'sync',
  TTF_RenderText_Solid: (font, text, color) => {
    // XXX the font and color are ignored
    text = UTF8ToString(text) || ' '; // if given an empty string, still return a valid surface
    var fontData = SDL.fonts[font];
    var w = SDL.estimateTextWidth(fontData, text);
    var h = fontData.size;
    color = SDL.loadColorToCSSRGB(color); // XXX alpha breaks fonts?
    var fontString = SDL.makeFontString(h, fontData.name);
    var surf = SDL.makeSurface(w, h, 0, false, 'text:' + text); // bogus numbers..
    var surfData = SDL.surfaces[surf];
    surfData.ctx.save();
    surfData.ctx.fillStyle = color;
    surfData.ctx.font = fontString;
    // use bottom alignment, because it works
    // same in all browsers, more info here:
    // https://bugzilla.mozilla.org/show_bug.cgi?id=737852
    surfData.ctx.textBaseline = 'bottom';
    surfData.ctx.fillText(text, 0, h|0);
    surfData.ctx.restore();
    return surf;
  },
  TTF_RenderText_Blended: 'TTF_RenderText_Solid', // XXX ignore blending vs. solid
  TTF_RenderText_Shaded: 'TTF_RenderText_Solid', // XXX ignore blending vs. solid
  TTF_RenderUTF8_Solid: 'TTF_RenderText_Solid',
  TTF_SizeUTF8: 'TTF_SizeText',

  TTF_SizeText__proxy: 'sync',
  TTF_SizeText: (font, text, w, h) => {
    var fontData = SDL.fonts[font];
    if (w) {
      {{{ makeSetValue('w', 0, 'SDL.estimateTextWidth(fontData, UTF8ToString(text))', 'i32') }}};
    }
    if (h) {
      {{{ makeSetValue('h', 0, 'fontData.size', 'i32') }}};
    }
    return 0;
  },

  TTF_GlyphMetrics__proxy: 'sync',
  TTF_GlyphMetrics: (font, ch, minx, maxx, miny, maxy, advance) => {
    var fontData = SDL.fonts[font];
    var width = SDL.estimateTextWidth(fontData,  String.fromCharCode(ch));

    if (advance) {
      {{{ makeSetValue('advance', 0, 'width', 'i32') }}};
    }
    if (minx) {
      {{{ makeSetValue('minx', 0, '0', 'i32') }}};
    }
    if (maxx) {
      {{{ makeSetValue('maxx', 0, 'width', 'i32') }}};
    }
    if (miny) {
      {{{ makeSetValue('miny', 0, '0', 'i32') }}};
    }
    if (maxy) {
      {{{ makeSetValue('maxy', 0, 'fontData.size', 'i32') }}};
    }
  },

  TTF_FontAscent__proxy: 'sync',
  TTF_FontAscent: (font) => {
    var fontData = SDL.fonts[font];
    return (fontData.size*0.98)|0; // XXX
  },

  TTF_FontDescent__proxy: 'sync',
  TTF_FontDescent: (font) => {
    var fontData = SDL.fonts[font];
    return (fontData.size*0.02)|0; // XXX
  },

  TTF_FontHeight__proxy: 'sync',
  TTF_FontHeight: (font) => {
    var fontData = SDL.fonts[font];
    return fontData.size;
  },

  TTF_FontLineSkip: 'TTF_FontHeight', // XXX

  TTF_Quit: () => out('TTF_Quit called (and ignored)'),

  // SDL gfx

  $SDL_gfx: {
    drawRectangle: (surf, x1, y1, x2, y2, action, cssColor) => {
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
    drawLine: (surf, x1, y1, x2, y2, cssColor) => {
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
    drawEllipse: (surf, x, y, rx, ry, action, cssColor) => {
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
    translateColorToCSSRGBA: (rgba) => `rgba(${rgba>>>24},${rgba>>16 & 0xff},${rgba>>8 & 0xff},${rgba&0xff})`,
  },

  boxColor__deps: ['$SDL_gfx'],
  boxColor: (surf, x1, y1, x2, y2, color) =>
    SDL_gfx.drawRectangle(surf, x1, y1, x2, y2, 'fill', SDL_gfx.translateColorToCSSRGBA(color)),

  boxRGBA__deps: ['$SDL_gfx'],
  boxRGBA: (surf, x1, y1, x2, y2, r, g, b, a) =>
    SDL_gfx.drawRectangle(surf, x1, y1, x2, y2, 'fill', SDL.translateRGBAToCSSRGBA(r, g, b, a)),

  rectangleColor__deps: ['$SDL_gfx'],
  rectangleColor: (surf, x1, y1, x2, y2, color) =>
    SDL_gfx.drawRectangle(surf, x1, y1, x2, y2, 'stroke', SDL_gfx.translateColorToCSSRGBA(color)),

  rectangleRGBA__deps: ['$SDL_gfx'],
  rectangleRGBA: (surf, x1, y1, x2, y2, r, g, b, a) =>
    SDL_gfx.drawRectangle(surf, x1, y1, x2, y2, 'stroke', SDL.translateRGBAToCSSRGBA(r, g, b, a)),

  ellipseColor__deps: ['$SDL_gfx'],
  ellipseColor: (surf, x, y, rx, ry, color) =>
    SDL_gfx.drawEllipse(surf, x, y, rx, ry, 'stroke', SDL_gfx.translateColorToCSSRGBA(color)),

  ellipseRGBA__deps: ['$SDL_gfx'],
  ellipseRGBA: (surf, x, y, rx, ry, r, g, b, a) =>
    SDL_gfx.drawEllipse(surf, x, y, rx, ry, 'stroke', SDL.translateRGBAToCSSRGBA(r, g, b, a)),

  filledEllipseColor__deps: ['$SDL_gfx'],
  filledEllipseColor: (surf, x, y, rx, ry, color) =>
    SDL_gfx.drawEllipse(surf, x, y, rx, ry, 'fill', SDL_gfx.translateColorToCSSRGBA(color)),

  filledEllipseRGBA__deps: ['$SDL_gfx'],
  filledEllipseRGBA: (surf, x, y, rx, ry, r, g, b, a) =>
    SDL_gfx.drawEllipse(surf, x, y, rx, ry, 'fill', SDL.translateRGBAToCSSRGBA(r, g, b, a)),

  lineColor__deps: ['$SDL_gfx'],
  lineColor: (surf, x1, y1, x2, y2, color) =>
    SDL_gfx.drawLine(surf, x1, y1, x2, y2, SDL_gfx.translateColorToCSSRGBA(color)),

  lineRGBA__deps: ['$SDL_gfx'],
  lineRGBA: (surf, x1, y1, x2, y2, r, g, b, a) =>
    SDL_gfx.drawLine(surf, x1, y1, x2, y2, SDL.translateRGBAToCSSRGBA(r, g, b, a)),

  pixelRGBA__deps: ['boxRGBA'],
  // This cannot be fast, to render many pixels this way!
  pixelRGBA: (surf, x1, y1, r, g, b, a) => _boxRGBA(surf, x1, y1, x1, y1, r, g, b, a),

  // GL

  SDL_GL_SetAttribute__proxy: 'sync',
  SDL_GL_SetAttribute: (attr, value) => {
    if (!(attr in SDL.glAttributes)) {
      abort('Unknown SDL GL attribute (' + attr + '). Please check if your SDL version is supported.');
    }

    SDL.glAttributes[attr] = value;
  },

  SDL_GL_GetAttribute__proxy: 'sync',
  SDL_GL_GetAttribute: (attr, value) => {
    if (!(attr in SDL.glAttributes)) {
      abort('Unknown SDL GL attribute (' + attr + '). Please check if your SDL version is supported.');
    }

    if (value) {{{ makeSetValue('value', 0, 'SDL.glAttributes[attr]', 'i32') }}};

    return 0;
  },

  SDL_GL_SwapBuffers__proxy: 'sync',
  SDL_GL_SwapBuffers: () => {
    Browser.doSwapBuffers?.(); // in workers, this is used to send out a buffered frame
  },

  // SDL 2

  SDL_GL_ExtensionSupported__proxy: 'sync',
  SDL_GL_ExtensionSupported: (extension) => Module.ctx.getExtension(extension) | 0,

  SDL_DestroyWindow: (window) => {},

  SDL_DestroyRenderer: (renderer) => {},

  SDL_GetWindowFlags__proxy: 'sync',
  SDL_GetWindowFlags: (window) => {
    if (Browser.isFullscreen) {
       return 1;
    }

    return 0;
  },

  SDL_GL_SwapWindow: (window) => {},

  SDL_GL_MakeCurrent: (window, context) => {},

  SDL_GL_DeleteContext: (context) => {},

  SDL_GL_GetSwapInterval__proxy: 'sync',
  SDL_GL_GetSwapInterval: () => {
    if (MainLoop.timingMode == {{{ cDefs.EM_TIMING_RAF }}}) return MainLoop.timingValue;
    else return 0;
  },

  SDL_GL_SetSwapInterval__deps: ['emscripten_set_main_loop_timing'],
  SDL_GL_SetSwapInterval: (state) => {
    _emscripten_set_main_loop_timing({{{ cDefs.EM_TIMING_RAF }}}, state);
  },

  SDL_SetWindowTitle__proxy: 'sync',
  SDL_SetWindowTitle: (window, title) => {
    if (title) document.title = UTF8ToString(title);
  },

  SDL_GetWindowSize__proxy: 'sync',
  SDL_GetWindowSize: (window, width, height) => {
    var w = Module['canvas'].width;
    var h = Module['canvas'].height;
    if (width) {{{ makeSetValue('width', 0, 'w', 'i32') }}};
    if (height) {{{ makeSetValue('height', 0, 'h', 'i32') }}};
  },

  SDL_LogSetOutputFunction: (callback, userdata) => {},

  SDL_SetWindowFullscreen__proxy: 'sync',
  SDL_SetWindowFullscreen: (window, fullscreen) => {
    if (Browser.isFullscreen) {
      Module['canvas'].exitFullscreen();
      return 1;
    }
    return 0;
  },

  SDL_ClearError: () => {},

  // TODO

  SDL_SetGamma: (r, g, b) => -1,

  SDL_SetGammaRamp: (redTable, greenTable, blueTable) => -1,

  // Joysticks

  SDL_NumJoysticks__proxy: 'sync',
  SDL_NumJoysticks: () => {
    var count = 0;
    var gamepads = SDL.getGamepads();
    // The length is not the number of gamepads; check which ones are defined.
    for (var i = 0; i < gamepads.length; i++) {
      if (gamepads[i] !== undefined) count++;
    }
    return count;
  },

  SDL_JoystickName__proxy: 'sync',
  SDL_JoystickName__deps: ['$stringToNewUTF8'],
  SDL_JoystickName: (deviceIndex) => {
    var gamepad = SDL.getGamepad(deviceIndex);
    if (gamepad) {
      var name = gamepad.id;
      if (SDL.joystickNamePool.hasOwnProperty(name)) {
        return SDL.joystickNamePool[name];
      }
      return SDL.joystickNamePool[name] = stringToNewUTF8(name);
    }
    return 0;
  },

  SDL_JoystickOpen__proxy: 'sync',
  SDL_JoystickOpen: (deviceIndex) => {
    var gamepad = SDL.getGamepad(deviceIndex);
    if (gamepad) {
      // Use this as a unique 'pointer' for this joystick.
      var joystick = deviceIndex+1;
      SDL.recordJoystickState(joystick, gamepad);
      return joystick;
    }
    return 0;
  },

  SDL_JoystickOpened__proxy: 'sync',
  SDL_JoystickOpened: (deviceIndex) => SDL.lastJoystickState.hasOwnProperty(deviceIndex+1) ? 1 : 0,

  // joystick pointers are simply the deviceIndex+1.
  SDL_JoystickIndex: (joystick) => joystick - 1,

  SDL_JoystickNumAxes__proxy: 'sync',
  SDL_JoystickNumAxes: (joystick) => {
    var gamepad = SDL.getGamepad(joystick - 1);
    if (gamepad) {
      return gamepad.axes.length;
    }
    return 0;
  },

  SDL_JoystickNumBalls: (joystick) => 0,

  SDL_JoystickNumHats: (joystick) => 0,

  SDL_JoystickNumButtons__proxy: 'sync',
  SDL_JoystickNumButtons: (joystick) => {
    var gamepad = SDL.getGamepad(joystick - 1);
    if (gamepad) {
      return gamepad.buttons.length;
    }
    return 0;
  },

  SDL_JoystickUpdate__proxy: 'sync',
  SDL_JoystickUpdate: () => SDL.queryJoysticks(),

  SDL_JoystickEventState__proxy: 'sync',
  SDL_JoystickEventState: (state) => {
    if (state < 0) {
      // SDL_QUERY: Return current state.
      return SDL.joystickEventState;
    }
    return SDL.joystickEventState = state;
  },

  SDL_JoystickGetAxis__proxy: 'sync',
  SDL_JoystickGetAxis: (joystick, axis) => {
    var gamepad = SDL.getGamepad(joystick - 1);
    if (gamepad && gamepad.axes.length > axis) {
      return SDL.joystickAxisValueConversion(gamepad.axes[axis]);
    }
    return 0;
  },

  SDL_JoystickGetHat: (joystick, hat) => 0,

  SDL_JoystickGetBall: (joystick, ball, dxptr, dyptr) => -1,

  SDL_JoystickGetButton__proxy: 'sync',
  SDL_JoystickGetButton: (joystick, button) => {
    var gamepad = SDL.getGamepad(joystick - 1);
    if (gamepad && gamepad.buttons.length > button) {
      return SDL.getJoystickButtonState(gamepad.buttons[button]) ? 1 : 0;
    }
    return 0;
  },

  SDL_JoystickClose__proxy: 'sync',
  SDL_JoystickClose: (joystick) => {
    delete SDL.lastJoystickState[joystick];
  },

  // Misc

  SDL_InitSubSystem: (flags) => 0,

  SDL_RWFromConstMem__proxy: 'sync',
  SDL_RWFromConstMem: (mem, size) => {
    var id = SDL.rwops.length; // TODO: recycle ids when they are null
    SDL.rwops.push({ bytes: mem, count: size });
    return id;
  },
  SDL_RWFromMem: 'SDL_RWFromConstMem',

  SDL_RWFromFile__proxy: 'sync',
  SDL_RWFromFile__docs: '/** @param {number} mode */',
  SDL_RWFromFile: (_name, mode) => {
    var id = SDL.rwops.length; // TODO: recycle ids when they are null
    var filename = UTF8ToString(_name);
    SDL.rwops.push({ filename, mimetype: Browser.getMimetype(filename) });
    return id;
  },

  SDL_FreeRW__proxy: 'sync',
  SDL_FreeRW: (rwopsID) => {
    SDL.rwops[rwopsID] = null;
    while (SDL.rwops.length > 0 && SDL.rwops[SDL.rwops.length-1] === null) {
      SDL.rwops.pop();
    }
  },

  SDL_GetNumAudioDrivers: () => 1,
  SDL_GetCurrentAudioDriver__deps: ['$stringToNewUTF8'],
  SDL_GetCurrentAudioDriver: () => stringToNewUTF8('Emscripten Audio'),
  SDL_GetScancodeFromKey: (key) => SDL.scanCodes[key],
  SDL_GetAudioDriver__deps: ['SDL_GetCurrentAudioDriver'],
  SDL_GetAudioDriver: (index) => _SDL_GetCurrentAudioDriver(),

  SDL_EnableUNICODE__proxy: 'sync',
  SDL_EnableUNICODE: (on) => {
    var ret = SDL.unicode || 0;
    SDL.unicode = on;
    return ret;
  },

  SDL_AddTimer__proxy: 'sync',
  SDL_AddTimer__deps: ['$safeSetTimeout'],
  SDL_AddTimer: (interval, callback, param) =>
    safeSetTimeout(
      () => {{{ makeDynCall('iip', 'callback') }}}(interval, param),
      interval),

  SDL_RemoveTimer__proxy: 'sync',
  SDL_RemoveTimer: (id) => {
    clearTimeout(id);
    return true;
  },

  // TODO:
  SDL_CreateThread: (fs, data, pfnBeginThread, pfnEndThread) => {
    throw 'SDL threads cannot be supported in the web platform because they assume shared state. See emscripten_create_worker etc. for a message-passing concurrency model that does let you run code in another thread.'
  },

  SDL_WaitThread: (thread, status) => { throw 'SDL_WaitThread' },
  SDL_GetThreadID: (thread) => { throw 'SDL_GetThreadID' },
  SDL_ThreadID: () => 0,
  SDL_AllocRW: () => { throw 'SDL_AllocRW: TODO' },
  SDL_CondBroadcast: (cond) => { throw 'SDL_CondBroadcast: TODO' },
  SDL_CondWaitTimeout: (cond, mutex, ms) => { throw 'SDL_CondWaitTimeout: TODO' },
  SDL_WM_IconifyWindow: () => { throw 'SDL_WM_IconifyWindow TODO' },

  Mix_SetPostMix: (func, arg) => warnOnce('Mix_SetPostMix: TODO'),

  Mix_VolumeChunk: (chunk, volume) => { throw 'Mix_VolumeChunk: TODO' },
  Mix_SetPosition: (channel, angle, distance) => { throw 'Mix_SetPosition: TODO' },
  Mix_QuerySpec: (frequency, format, channels) => { throw 'Mix_QuerySpec: TODO' },
  Mix_FadeInChannelTimed: (channel, chunk, loop, ms, ticks) => { throw 'Mix_FadeInChannelTimed' },
  Mix_FadeOutChannel: () => { throw 'Mix_FadeOutChannel' },

  Mix_Linked_Version: () => { throw 'Mix_Linked_Version: TODO' },
  SDL_SaveBMP_RW: (surface, dst, freedst) => { throw 'SDL_SaveBMP_RW: TODO' },

  /* This function would set the application window icon surface, which doesn't apply for web canvases, so a no-op. */
  SDL_WM_SetIcon: (icon, mask) => {},
  SDL_HasRDTSC: () => 0,
  SDL_HasMMX: () => 0,
  SDL_HasMMXExt: () => 0,
  SDL_Has3DNow: () => 0,
  SDL_Has3DNowExt: () => 0,
  SDL_HasSSE: () => 0,
  SDL_HasSSE2: () => 0,
  SDL_HasAltiVec: () => 0
};

autoAddDeps(LibrarySDL, '$SDL');
addToLibrary(LibrarySDL);
