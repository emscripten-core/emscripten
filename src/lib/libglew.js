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
  $GLEW__deps: ['glGetString'],
  $GLEW: {
    isLinaroFork: 1,
    extensions: null,

    error: {
      [cDefs.GLEW_OK]: null, // GLEW_OK || GLEW_NO_ERROR
      [cDefs.GLEW_ERROR_NO_GL_VERSION]: null,
      [cDefs.GLEW_ERROR_GL_VERSION_10_ONLY]: null,
      [cDefs.GLEW_ERROR_GLX_VERSION_11_ONLY]: null,

      [cDefs.GLEW_ERROR_NOT_GLES_VERSION]: null,
      [cDefs.GLEW_ERROR_GLES_VERSION]: null,
      [cDefs.GLEW_ERROR_NO_EGL_VERSION]: null,
      [cDefs.GLEW_ERROR_EGL_VERSION_10_ONLY]: null,

      8: null, // Unknown error
    },

    version: {
      [cDefs.GLEW_VERSION]: null,
      [cDefs.GLEW_VERSION_MAJOR]: null,
      [cDefs.GLEW_VERSION_MINOR]: null,
      [cDefs.GLEW_VERSION_MICRO]: null,
    },

    errorStringConstantFromCode(error) {
      if (GLEW.isLinaroFork) {
        switch (error) {
          case GLEW_ERROR_NOT_GLES_VERSION: return 'OpenGL ES lib expected, found OpenGL lib';
          case GLEW_ERROR_GLES_VERSION: return 'OpenGL lib expected, found OpenGL ES lib';
          case GLEW_ERROR_NO_EGL_VERSION: return 'Missing EGL version';
          case GLEW_ERROR_EGL_VERSION_10_ONLY: return 'EGL 1.1 and up are supported';
          default: break;
        }
      }

      switch (error) {
        case GLEW_OK: return 'No error'; // GLEW_OK || GLEW_NO_ERROR
        case GLEW_ERROR_NO_GL_VERSION: return 'Missing GL version';
        case GLEW_ERROR_GL_VERSION_10_ONLY: return 'GL 1.1 and up are supported';
        case GLEW_ERROR_GLX_VERSION_11_ONLY: return 'GLX 1.2 and up are supported';
        default: return null;
      }
    },

    errorString(error) {
      if (!GLEW.error[error]) {
        var string = GLEW.errorStringConstantFromCode(error);
        if (!string) {
          string = 'Unknown error';
          error = 8; // prevent array from growing more than this
        }
        GLEW.error[error] = stringToNewUTF8(string);
      }
      return GLEW.error[error];
    },

    versionStringConstantFromCode(name) {
      switch (name) {
        case GLEW_VERSION: return '1.10.0';
        case GLEW_VERSION_MAJOR: return '1';
        case GLEW_VERSION_MINOR: return '10';
        case GLEW_VERSION_MICRO: return '0';
        default: return null;
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
      return (GLEW.extensions.includes('GL_' + name));
    },
  },

  glewInit: () => 0,

  glewIsSupported: (name) => {
    var exts = UTF8ToString(name).split(' ');
    for (var ext of exts) {
      if (!GLEW.extensionIsSupported(ext)) return 0;
    }
    return 1;
  },

  glewGetExtension: (name) => GLEW.extensionIsSupported(UTF8ToString(name)),

  glewGetErrorString: (error) => GLEW.errorString(error),

  glewGetString: (name) => GLEW.versionString(name),

};

addToLibrary(LibraryGLEW);
