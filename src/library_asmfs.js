var asmFS = {
#if USE_PTHREADS
  __readdir_cntptr: '; if (ENVIRONMENT_IS_PTHREAD) ___readdir_cntptr = PthreadWorkerInit.___readdir_cntptr; else PthreadWorkerInit.___readdir_cntptr = ___readdir_cntptr = {{{ makeStaticAlloc(4) }}}',
  __readdir_entptr: '; if (ENVIRONMENT_IS_PTHREAD) ___readdir_entptr = PthreadWorkerInit.___readdir_entptr; else PthreadWorkerInit.___readdir_entptr = ___readdir_entptr = {{{ makeStaticAlloc(4) }}}',
#else
  __readdir_cntptr: '{{{ makeStaticAlloc(4) }}}',
  __readdir_entptr: '{{{ makeStaticAlloc(4) }}}',
#endif

  $FS__deps: ['emscripten_asmfs_readdir', '__readdir_entptr', '__readdir_cntptr'],

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

    ErrnoError: function (errno) {
      this.errno = errno;
    },

    handleFSError: function(e) {
      if (!(e instanceof FS.ErrnoError)) throw e + ' : ' + stackTrace();
      return ___setErrNo(e.errno);
    },

    readdir: function(path) {
      var res, ents, offset, entptr, n_ents;
      var pathCString = allocate(intArrayFromString(path), 'i8', ALLOC_NORMAL);
      res = _emscripten_asmfs_readdir(pathCString,
                                      ___readdir_entptr, ___readdir_cntptr);
      _free(pathCString);
      if (res) {
        throw new FS.ErrnoError(-res);
      }

      entptr = {{{ makeGetValue('___readdir_entptr', '0', 'i32') }}};
      n_ents = {{{ makeGetValue('___readdir_cntptr', '0', 'i32') }}};

      ents = [];
      for (offset = 0; offset < n_ents * {{{ C_STRUCTS.dirent.__size__ }}}; offset += {{{ C_STRUCTS.dirent.__size__ }}}) {
        var ent = {};
        ent.id = {{{ makeGetValue('entptr + offset', C_STRUCTS.dirent.d_ino, 'i32') }}};
        ent.type = {{{ makeGetValue('entptr + offset', C_STRUCTS.dirent.d_type, 'i8') }}};
        ent.name = UTF8ToString(entptr + offset + {{{ C_STRUCTS.dirent.d_name }}});
        ents.push(ent);
      }

      _free(entptr);
      return ents;
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
