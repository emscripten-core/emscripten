# PThreadFS

The Emscripten Pthread File System (PThreadFS) unlocks using (partly) asynchronous storage APIs such as [OPFS Access Handles](https://docs.google.com/document/d/121OZpRk7bKSF7qU3kQLqAEUVSNxqREnE98malHYwWec/edit#heading=h.gj2fudnvy982) through the Emscripten File System API. This enables C++ applications compiled through Emscripten to use persistent storage without using [Asyncify](https://emscripten.org/docs/porting/asyncify.html). PThreadFS requires only minimal modifications to the C++ code and nearly achieves feature parity with the Emscripten's classic File System API.

PThreadFS is based on Emscripten’s [SyncToAsync prototype](https://github.com/emscripten-core/emscripten/pull/13298). All file system operations are proxied through a dedicated worker thread. Although the underlying storage API is asynchronous, PThreadFS makes it appear synchronous to the C++ application.

The code is still prototype quality and **should not be used in a production environment**.

## Enable and detect OPFS in Chrome

TODO

## Getting the code

PthreadFS is available on Github in the [emscripten-pthreadfs](https://github.com/rstz/emscripten-pthreadfs) repository. All code resides in the `pthreadfs` folder. It should be usable with any up-to-date Emscripten version. 

There is **no need** to use a fork of Emscripten itself since all code operates in user-space.

## Using PThreadFS in a project

In order to use the code in a new project, you only need the three files in the `pthreadfs` folder: `pthreadfs_library.js`, `pthreadfs.cpp` and `pthreadfs.h`. The files are included as follows:

### Code changes

- Include `pthreadfs.h` in the C++ file containing `main()`. 
- Call `emscripten_init_pthreadfs();` at the top of `main()` (or before any file system syscalls).
- Files that persist between sessions must be stored in `/filesystemaccess/` or its subfolders.

### Build process changes

- Compile `pthreadfs.h` and `pthreadfs.cpp` and link the resulting object to your application.
- Include the PThreadFS Javascript code by adding the following in the linking step:
```
--js-library=library_pthreadsfs.js
```
- Enable `PROXY_TO_PTHREAD` by adding the following to the linking step:
```
-s PROXY_TO_PTHREAD
```

### Advanced Usage

If you want to modify the PThreadFS file system directly, you may use the macro `EM_PTHREADFS_ASM()` defined in `pthreadfs.h`. The macro allows you to run asynchrononous Javascript on the Pthread hosting the PThreadFS file system. For example, you may create a folder in the virtual file system by calling
```
EM_PTHREADFS_ASM(
  await PThreadFS.mkdir('mydirectory');
);
```
See `pthreadfs/examples/emscripten-tests/` for exemplary usage.


## Known Limitations

- The code is still prototype quality and **should not be used in a production environment**.
- PThreadFS requires PROXY_TO_PTHREAD to be active.

  In particular, no system calls interacting with the file system should be called from the main thread.
- Some functionality of the Emscripten File System API is missing, such as sockets, IndexedDB integration and support for XHRequests.
- PThreadFS depends on C++ libraries. `EM_PTRHEADFS_ASM()` cannot be used within C files (although initializing through `emscripten_init_pthreadfs()` is possible, see the `pthreadfs/examples/sqlite-speedtest` for an example).
- Only in-memory storage (MEMFS) and OPFS Access Handles (FSAFS) are available as backends for PThreadFS. 

  In particualar, there is no support (yet) for IDBFS. Limited support is available for the Storage Foundation API.
- Performance is good if and only if full optimizations (compiler option `-O3`) are enabled and DevTools are closed.
- Using stdout from C++ only prints to the Javascript console, not the Emscripten-generated html file.


## Examples

The examples are provided to show how projects can be transformed to use PThreadFS. To build them, navigate to the `pthreadfs/examples/` folder and run `make all`. You need to have the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) activated for the build process to succeed.

### SQLite Speedtest

This example shows how to compile and run the [speedtest1](https://www.sqlite.org/cpu.html) from the SQLite project in the browser.

The Makefile downloads the source of the speedtest and sqlite3 directly from <https://sqlite.org>.

To compile, navigate to the `pthreadfs/examples/` directory and run

```shell
make sqlite-speedtest
cd dist/sqlite-speedtest
python3 -m http.server 8888
```
Then open the following link in a Chrome instance with the
"File System Access Access Handles" enabled (see above):
[localhost:8888/sqlite-speedtest](http://localhost:8888/sqlite-speedtest). The results of the speedtest can be found in the DevTools console.

### Other tests

The folder `pthreadfs/examples/emscripten-tests` contains a number of other file system tests taken from Emscripten's standard test suite.

To compile, navigate to the `pthreadfs/examples/` directory and run

```shell
make emscripten-tests
cd dist/emscripten-tests
python3 -m http.server 8888
```
Then open the following link in a Chrome instance with the
"File System Access Access Handles" enabled (see above):
[localhost:8888/emscripten-tests](http://localhost:8888/emscripten-tests) and choose a test. The results of the test can be found in the DevTools console.

## Authors
- Richard Stotz (<rstz@chromium.org>)

This is not an official Google product.