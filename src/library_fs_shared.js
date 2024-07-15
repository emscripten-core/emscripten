/**
 * @license
 * Copyright 2032 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $preloadPlugins: "{{{ makeModuleReceiveExpr('preloadPlugins', '[]') }}}",

#if !MINIMAL_RUNTIME
  // Tries to handle an input byteArray using preload plugins. Returns true if
  // it was handled.
  $FS_handledByPreloadPlugin__internal: true,
  $FS_handledByPreloadPlugin__deps: ['$preloadPlugins'],
  $FS_handledByPreloadPlugin: (byteArray, fullname, finish, onerror) => {
#if LibraryManager.has('library_browser.js')
    // Ensure plugins are ready.
    if (typeof Browser != 'undefined') Browser.init();
#endif

    var handled = false;
    preloadPlugins.forEach((plugin) => {
      if (handled) return;
      if (plugin['canHandle'](fullname)) {
        plugin['handle'](byteArray, fullname, finish, onerror);
        handled = true;
      }
    });
    return handled;
  },
#endif

  // Preloads a file asynchronously. You can call this before run, for example in
  // preRun. run will be delayed until this file arrives and is set up.
  // If you call it after run(), you may want to pause the main loop until it
  // completes, if so, you can use the onload parameter to be notified when
  // that happens.
  // In addition to normally creating the file, we also asynchronously preload
  // the browser-friendly versions of it: For an image, we preload an Image
  // element and for an audio, and Audio. These are necessary for SDL_Image
  // and _Mixer to find the files in preloadedImages/Audios.
  // You can also call this with a typed array instead of a url. It will then
  // do preloading for the Image/Audio part, as if the typed array were the
  // result of an XHR that you did manually.
  $FS_createPreloadedFile__deps: [
    '$asyncLoad',
    '$PATH_FS',
    '$FS_createDataFile',
#if !MINIMAL_RUNTIME
    '$FS_handledByPreloadPlugin',
#endif
  ],
  $FS_createPreloadedFile: (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) => {
    // TODO we should allow people to just pass in a complete filename instead
    // of parent and name being that we just join them anyways
    var fullname = name ? PATH_FS.resolve(PATH.join2(parent, name)) : parent;
    var dep = getUniqueRunDependency(`cp ${fullname}`); // might have several active requests for the same fullname
    function processData(byteArray) {
      function finish(byteArray) {
        preFinish?.();
        if (!dontCreateFile) {
          FS_createDataFile(parent, name, byteArray, canRead, canWrite, canOwn);
        }
        onload?.();
        removeRunDependency(dep);
      }
#if !MINIMAL_RUNTIME
      if (FS_handledByPreloadPlugin(byteArray, fullname, finish, () => {
        onerror?.();
        removeRunDependency(dep);
      })) {
        return;
      }
#endif
      finish(byteArray);
    }
    addRunDependency(dep);
    if (typeof url == 'string') {
      asyncLoad(url, processData, onerror);
    } else {
      processData(url);
    }
  },
  // convert the 'r', 'r+', etc. to it's corresponding set of O_* flags
  $FS_modeStringToFlags: (str) => {
    var flagModes = {
      'r': {{{ cDefs.O_RDONLY }}},
      'r+': {{{ cDefs.O_RDWR }}},
      'w': {{{ cDefs.O_TRUNC }}} | {{{ cDefs.O_CREAT }}} | {{{ cDefs.O_WRONLY }}},
      'w+': {{{ cDefs.O_TRUNC }}} | {{{ cDefs.O_CREAT }}} | {{{ cDefs.O_RDWR }}},
      'a': {{{ cDefs.O_APPEND }}} | {{{ cDefs.O_CREAT }}} | {{{ cDefs.O_WRONLY }}},
      'a+': {{{ cDefs.O_APPEND }}} | {{{ cDefs.O_CREAT }}} | {{{ cDefs.O_RDWR }}},
    };
    var flags = flagModes[str];
    if (typeof flags == 'undefined') {
      throw new Error(`Unknown file open mode: ${str}`);
    }
    return flags;
  },
  $FS_getMode: (canRead, canWrite) => {
    var mode = 0;
    if (canRead) mode |= {{{ cDefs.S_IRUGO }}} | {{{ cDefs.S_IXUGO }}};
    if (canWrite) mode |= {{{ cDefs.S_IWUGO }}};
    return mode;
  },

  $FS_stdin_getChar_buffer: [],

  // getChar has 3 particular return values:
  // a.) the next character represented as an integer
  // b.) undefined to signal that no data is currently available
  // c.) null to signal an EOF
  $FS_stdin_getChar__deps: [
    '$FS_stdin_getChar_buffer',
    '$intArrayFromString',
  ],
  $FS_stdin_getChar: () => {
    if (!FS_stdin_getChar_buffer.length) {
      var result = null;
#if ENVIRONMENT_MAY_BE_NODE
      if (ENVIRONMENT_IS_NODE) {
        // we will read data by chunks of BUFSIZE
        var BUFSIZE = 256;
        var buf = Buffer.alloc(BUFSIZE);
        var bytesRead = 0;

        // For some reason we must suppress a closure warning here, even though
        // fd definitely exists on process.stdin, and is even the proper way to
        // get the fd of stdin,
        // https://github.com/nodejs/help/issues/2136#issuecomment-523649904
        // This started to happen after moving this logic out of library_tty.js,
        // so it is related to the surrounding code in some unclear manner.
        /** @suppress {missingProperties} */
        var fd = process.stdin.fd;

        try {
          bytesRead = fs.readSync(fd, buf, 0, BUFSIZE);
        } catch(e) {
          // Cross-platform differences: on Windows, reading EOF throws an
          // exception, but on other OSes, reading EOF returns 0. Uniformize
          // behavior by treating the EOF exception to return 0.
          if (e.toString().includes('EOF')) bytesRead = 0;
          else throw e;
        }

        if (bytesRead > 0) {
          result = buf.slice(0, bytesRead).toString('utf-8');
        }
      } else
#endif
#if ENVIRONMENT_MAY_BE_WEB
      if (typeof window != 'undefined' &&
        typeof window.prompt == 'function') {
        // Browser.
        result = window.prompt('Input: ');  // returns null on cancel
        if (result !== null) {
          result += '\n';
        }
      } else
#endif
#if ENVIRONMENT_MAY_BE_SHELL
      if (typeof readline == 'function') {
        // Command line.
        result = readline();
        if (result) {
          result += '\n';
        }
      } else
#endif
      {}
      if (!result) {
        return null;
      }
      FS_stdin_getChar_buffer = intArrayFromString(result, true);
    }
    return FS_stdin_getChar_buffer.shift();
  },

  $FS_unlink__deps: ['$FS'],
  $FS_unlink: 'FS.unlink',

  $FS_createPath__deps: ['$FS'],
  $FS_createPath: 'FS.createPath',

  $FS_createDevice__deps: ['$FS'],
  $FS_createDevice: 'FS.createDevice',

  $FS_readFile__deps: ['$FS'],
  $FS_readFile: 'FS.readFile',
});

// Normally only the FS things that the compiler sees are needed are included.
// FORCE_FILESYSTEM makes us always include the FS object, which lets the user
// call APIs on it from JS freely.
if (FORCE_FILESYSTEM) {
  extraLibraryFuncs.push('$FS');
}
