var LibraryGL = {
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
  },

  glGetIntegerv: function(name_) {
    switch(name_) {
      case Module.contextGL.MAX_TEXTURE_SIZE:
        return Module.contextGL.getParameter(name_);
      default:
        throw 'Failure: Invalid glGetIntegerv value: ' + name_;
    }
  }
};

[[0, 'shadeModel fogi fogfv'],
 [1, 'clearDepth depthFunc enable disable frontFace cullFace'],
 [4, 'viewport clearColor']].forEach(function(data) {
  var num = data[0];
  var names = data[1];
  var args = range(num).map(function(i) { return 'x' + i }).join(', ');
  var stub = '(function(' + args + ') { ' + (num > 0 ? 'Module.contextGL.NAME(' + args + ')' : '') + ' })';
  names.split(' ').forEach(function(name_) {
    var cName = 'gl' + name_[0].toUpperCase() + name_.substr(1);
    LibraryGL[cName] = eval(stub.replace('NAME', name_));
    //print(cName + ': ' + LibraryGL[cName]);
  });
});

mergeInto(Library, LibraryGL);

