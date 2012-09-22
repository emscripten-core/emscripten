// esUtil.js
//
//    A utility library for OpenGL ES.  This library provides
//    basic page handling for the example applications in the
//    OpenGL ES 2.0 Programming Guide.
//

var LibraryES = {
  $ES__deps:['$Browser'],
  $ES: {
  },
///
//  WinCreate()
//
//      This function initialized the native display and window for EGL (AKA Canvas)
//
  WinCreate: function(esContext, title) {
    var width  = {{{ makeGetValue('esContext+Runtime.QUANTUM_SIZE*1', '0', 'i32') }}};
    var height = {{{ makeGetValue('esContext+Runtime.QUANTUM_SIZE*2', '0', 'i32') }}};
    var canvas = Browser.getCanvas();
    Browser.setCanvasSize( canvas, width, height );
    Browser.setCanvasTitle( canvas, Pointer_stringify(title) );
      
    // Don't save reference to canvas since we currently only support one
    {{{ makeSetValue('esContext+Runtime.QUANTUM_SIZE*3', '0', '1', 'i32') }}};
    return 1; //EGL_TRUE
  }
};

autoAddDeps(LibraryES, '$ES');
mergeInto(LibraryManager.library, LibraryES);

