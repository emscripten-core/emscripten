// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

var LibraryXlib = {
  XOpenDisplay: function() {
    return 1; // We support 1 display, the canvas
  },

  XCreateWindow__deps: ['$Browser'],
  XCreateWindow: function(display, parent, x, y, width, height, border_width, depth, class_, visual, valuemask, attributes) {
    // All we can do is set the width and height
    Browser.setCanvasSize(width, height);
    return 2;
  },

  XChangeWindowAttributes: function(){},
  XSetWMHints: function(){},
  XMapWindow: function(){},
  XStoreName: function(){},
  XInternAtom: function(display, name_, hmm) { return 0 },
  XSendEvent: function(){},
  XPending: function(display) { return 0 },
};

mergeInto(LibraryManager.library, LibraryXlib);

