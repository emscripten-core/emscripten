/**
 * @license
 * Copyright 2012 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryXlib = {
  // We support 1 display, the canvas
  XOpenDisplay: (name) => 1,

  XCreateWindow__deps: ['$Browser'],
  XCreateWindow: (display, parent, x, y, width, height, border_width, depth, class_, visual, valuemask, attributes) => {
    // All we can do is set the width and height
    Browser.setCanvasSize(width, height);
    return 2;
  },

  XChangeWindowAttributes: (display, window, valuemask, attributes) => {},
  XSetWMHints: (display, win, hints) => {},
  XMapWindow: (display, win) => {},
  XStoreName: (display, win, name) => {},
  XInternAtom: (display, name_, hmm)  => 0,
  XSendEvent: (display, win, propagate, event_mask, even_send) => {},
  XPending: (display) => 0,
};

addToLibrary(LibraryXlib);
