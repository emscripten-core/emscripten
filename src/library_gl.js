mergeInto(Library, {
  glGetString: function(name_) {
    switch(name_) {
      case Module.contextGL.VENDOR:
      case Module.contextGL.RENDERER:
      case Module.contextGL.VERSION:
        return Pointer_make(intArrayFromString(Module.contextGL.getParameter(name_)), null, ALLOC_NORMAL);
      case 0x1F03: // Extensions
        return Pointer_make(intArrayFromString(Module.contextGL.getSupportedExtensions().join(' ')), null, ALLOC_NORMAL);
      default:
        throw 'Failure: Invalid glGetString value: ' + name_;
    }
  }

//  glGenVertexArrays: function() {},

//  glBindVertexArray: function() {},
});

