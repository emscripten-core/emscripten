var asmFS = {
  $FS: {
    populate: function(path, mode) {
      var pathCString = allocate(intArrayFromString(path), 'i8', ALLOC_NORMAL);
      mode = (mode !== undefined) ? mode : 511 /* 0777 */;
      _emscripten_asmfs_populate(pathCString, mode);
      _free(pathCString);
    },

    mkdir: function(path, mode) {
      mode = (mode !== undefined) ? mode : 511 /* 0777 */;
      var pathCString = allocate(intArrayFromString(path), 'i8', ALLOC_NORMAL);
      _emscripten_asmfs_mkdir(pathCString, mode);
      _free(pathCString);
    },

    mkdirTree: function(path, mode) {
      var dirs = path.split('/');
      var d = '';
      for (var i = 0; i < dirs.length; ++i) {
        if (!dirs[i]) continue;
        d += '/' + dirs[i];
        FS.mkdir(d, mode);
      }
    },

    setRemoteUrl: function(path, remoteUrl) {
      var pathCString = allocate(intArrayFromString(path), 'i8', ALLOC_NORMAL);
      var remoteUrlCString = allocate(intArrayFromString(remoteUrl), 'i8', ALLOC_NORMAL);
      _emscripten_asmfs_set_remote_url(pathCString, remoteUrlCString);
      _free(pathCString);
      _free(remoteUrlCString);
    },

    setFileData: function(path, data) {
      var dataInHeap = _malloc(data.length);
      HEAPU8.set(data, dataInHeap);
      var pathCString = allocate(intArrayFromString(path), 'i8', ALLOC_NORMAL);
      _emscripten_asmfs_set_file_data(pathCString, dataInHeap, data.length);
      _free(pathCString);
    }
  }
};

mergeInto(LibraryManager.library, asmFS);
