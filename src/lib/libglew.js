/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
 * EMSCRIPTEN GLEW 1.10.0 emulation
 *
 * What it does:
 * - Stubs init function.
 * - GL Extensions support.
 *
 * Optional:
 * - isLinaroFork variable to enable glew-es specific error strings.
 *   This is enabled by default, but should be disabled when upstream glew conflicts.
 *
 * Authors:
 * - Jari Vetoniemi <mailroxas@gmail.com>
 */

var LibraryGLEW = {
  $GLEW__deps: ['glGetString', '$stringToNewUTF8', '$UTF8ToString', '$webglGetExtensions'],
  $GLEW: {
    isLinaroFork: 1,
    extensions: null,

    error: {
      0:null, // GLEW_OK || GLEW_NO_ERROR
      1:null, // GLEW_ERROR_NO_GL_VERSION
      2:null, // GLEW_ERROR_GL_VERSION_10_ONLY
      3:null, // GLEW_ERROR_GLX_VERSION_11_ONLY

      4:null, // GLEW_ERROR_NOT_GLES_VERSION
      5:null, // GLEW_ERROR_GLES_VERSION
      6:null, // GLEW_ERROR_NO_EGL_VERSION
      7:null, // GLEW_ERROR_EGL_VERSION_10_ONLY

      8:null, // Unknown error
    },

    version: {
      1:null, // GLEW_VERSION
      2:null, // GLEW_VERSION_MAJOR
      3:null, // GLEW_VERSION_MINOR
      4:null, // GLEW_VERSION_MICRO
    },

    errorStringConstantFromCode(error) {
      if (GLEW.isLinaroFork) {
        switch (error) {
          case 4:return "OpenGL ES lib expected, found OpenGL lib"; // GLEW_ERROR_NOT_GLES_VERSION
          case 5:return "OpenGL lib expected, found OpenGL ES lib"; // GLEW_ERROR_GLES_VERSION
          case 6:return "Missing EGL version"; // GLEW_ERROR_NO_EGL_VERSION
          case 7:return "EGL 1.1 and up are supported"; // GLEW_ERROR_EGL_VERSION_10_ONLY
          default:break;
        }
      }

      switch (error) {
        case 0:return "No error"; // GLEW_OK || GLEW_NO_ERROR
        case 1:return "Missing GL version"; // GLEW_ERROR_NO_GL_VERSION
        case 2:return "GL 1.1 and up are supported"; // GLEW_ERROR_GL_VERSION_10_ONLY
        case 3:return "GLX 1.2 and up are supported"; // GLEW_ERROR_GLX_VERSION_11_ONLY
        default:return null;
      }
    },

    errorString(error) {
      if (!GLEW.error[error]) {
        var string = GLEW.errorStringConstantFromCode(error);
        if (!string) {
          string = "Unknown error";
          error = 8; // prevent array from growing more than this
        }
        GLEW.error[error] = stringToNewUTF8(string);
      }
      return GLEW.error[error];
    },

    versionStringConstantFromCode(name) {
      switch (name) {
        case 1:return "1.10.0"; // GLEW_VERSION
        case 2:return "1"; // GLEW_VERSION_MAJOR
        case 3:return "10"; // GLEW_VERSION_MINOR
        case 4:return "0"; // GLEW_VERSION_MICRO
        default:return null;
      }
    },

    versionString(name) {
      if (!GLEW.version[name]) {
        var string = GLEW.versionStringConstantFromCode(name);
        if (!string)
          return 0;
        GLEW.version[name] = stringToNewUTF8(string);
      }
      return GLEW.version[name];
    },

    extensionIsSupported(name) {
      GLEW.extensions ||= webglGetExtensions();

      if (GLEW.extensions.includes(name))
        return 1;

      // extensions from GLEmulations do not come unprefixed
      // so, try with prefix
      return (GLEW.extensions.includes("GL_" + name));
    },
  },

  glewInit: () => 0,

  glewIsSupported: (name) => {
    var exts = UTF8ToString(name).split(' ');
    for (var i = 0; i < exts.length; ++i) {
      if (!GLEW.extensionIsSupported(exts[i]))
        return 0;
    }
    return 1;
  },

  glewGetExtension: (name) => GLEW.extensionIsSupported(UTF8ToString(name)),

  glewGetErrorString: (error) => GLEW.errorString(error),

  glewGetString: (name) => GLEW.versionString(name),

};

autoAddDeps(LibraryGLEW, '$GLEW');
addToLibrary(LibraryGLEW);
