.. _Browser-limitations:

=================================
Browser limitations (wiki-import)
=================================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

This page collects together some of the most common differences between
the latest versions of the major browsers and features that
emscripten-based applications and games typically depend on. Use this
page to chart a quick summary of emscripten support in different
browsers.

-  The function emscripten\_get\_now() returns a wallclock time as a
   float in milliseconds. Opera 12.16 and `on Windows Google Chrome
   28.0.1500.95 <https://code.google.com/p/chromium/issues/detail?id=158234>`__
   have a limitation that the timer precision is only in milliseconds.
   On other major browsers (IE10, Firefox 22, Chrome 28 on non-Windows),
   emscripten\_get\_now() will have sub-millisecond precision.
-  Internet Explorer 10 and older do not support WebGL, which means
   trying to initialize a GL context via EGL, GLUT, SDL or similar will
   fail. Emscripten applications that do not depend on OpenGL can still
   be compiled for this browser however. See `WebGL support in different
   browsers <http://caniuse.com/#feat=webgl>`__.
-  Internet Explorer 11 supports WebGL, but not all of WebGL 1.0, so
   certain commands, shaders etc. may not work. You may be able to limit
   your app to using the subset that is supported by IE11. Hopefully
   IE12 will support everything.
-  Opera 12.16 has limited support for the W3C File API, in particular
   it does not support `createObjectURL
   functionality <http://www.opera.com/docs/specs/presto2.12/apis/#file>`__.
   As a result, it is not possible to use the browser image codecs on
   this version of Opera to decode preloaded files in the emscripten
   virtual filesystem.
-  OpenAL and SDL audio support in Emscripten depend on the Web Audio
   API. See `Web Audio API support in different
   browsers <http://caniuse.com/#feat=audio-api>`__.

