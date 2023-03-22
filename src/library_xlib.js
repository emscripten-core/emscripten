/**
 * @license
 * Copyright 2012 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryXlib = {
  XOpenDisplay: function(name) {
    return 1; // We support 1 display, the canvas
  },

  XCreateWindow__deps: ['$Browser'],
  XCreateWindow: function(display, parent, x, y, width, height, border_width, depth, class_, visual, valuemask, attributes) {
    // All we can do is set the width and height
    Browser.setCanvasSize(width, height);
    return 2;
  },

  XChangeWindowAttributes: function(display, window, valuemask, attributes){},
  XSetWMHints: function(display, win, hints){},
  XMapWindow: function(display, win){},
  XStoreName: function(display, win, name){},
  XInternAtom: function(display, name_, hmm) { return 0 },
  XSendEvent: function(display, win, propagate, event_mask, even_send){},
  XPending: function(display) { return 0 },
};

mergeInto(LibraryManager.library, LibraryXlib);

