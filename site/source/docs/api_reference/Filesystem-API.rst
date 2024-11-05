.. _Filesystem-API:

===============
File System API
===============

File operations in Emscripten are provided by the `FS <https://github.com/emscripten-core/emscripten/blob/main/src/library_fs.js>`_ library. It is used internally for all of Emscripten's **libc** and **libcxx** file I/O.

.. note:: The API is *inspired* by the Linux/POSIX `File System API <http://linux.die.net/man/2/>`_, with each presenting a very similar interface.

  The underlying behaviour is also similar, except where differences between the
  native and browser environments make this unreasonable. For example, user and
  group permissions are defined but ignored in :js:func:`FS.open`.

Emscripten predominantly compiles code that uses synchronous file I/O, so the majority of the ``FS`` member functions offer a synchronous interface (with errors being reported by raising exceptions of type `FS.ErrnoError <https://github.com/emscripten-core/emscripten/blob/main/system/lib/libc/musl/arch/emscripten/bits/errno.h>`_).

File data in Emscripten is partitioned by mounted file systems. Several file systems are provided. An instance of :ref:`MEMFS <filesystem-api-memfs>` is mounted to ``/`` by default. The subdirectories `/home/web_user` and `/tmp` are also created automatically, in addition to several other special devices and streams (e.g. `/dev/null`, `/dev/random`, `/dev/stdin`, `/proc/self/fd`); see `FS.staticInit()` in the FS library for full details. Instances of :ref:`NODEFS <filesystem-api-nodefs>` and :ref:`IDBFS <filesystem-api-idbfs>` can be mounted to other directories if your application needs to :ref:`persist data <filesystem-api-persist-data>`.

The automatic tests in `test/test_core.py <https://github.com/emscripten-core/emscripten/blob/1.29.12/tests/test_core.py#L4285>`_ (search for ``test_files``) contain many examples of how to use this API. The :ref:`tutorial <tutorial-files>` also shows how to pre-load a file so that it can be read from compiled C/C++.

A high level overview of the way File Systems work in Emscripten-ported code is provided in the :ref:`file-system-overview`.

New File System: WasmFS
=======================

.. note:: Current Status: Work in Progress

WasmFS is a high-performance, fully-multithreaded, WebAssembly-based file system layer for Emscripten that will replace the existing JavaScript version.

The JavaScript-based file system was originally written before pthreads were supported and when it was more optimal to write code in JS. As a result it has overhead in pthreads builds because we must proxy to the main thread where all filesystem operations are done. WasmFS, instead, is compiled to Wasm and has full multithreading support. It also aims to be more modular and extensible.

`Design Doc Link <https://docs.google.com/document/d/1-ZxybGvz0nCqygUDuWxCcCBhCebev3EbUSYoSOlc49Q/edit?usp=sharing>`_

`GitHub Tracking Issue <https://github.com/emscripten-core/emscripten/issues/15041>`_

Differences you may notice with the original JS filesystem include:

- The original JS FS includes a lot of JS code by default, while WasmFS does
  not. As a result, if you write JS of your own, say ``FS.mkdir()``, then the
  JS FS would already have added that API support, and things would just work.
  With WasmFS you must opt-in to including the full JS API, to avoid bloating
  all builds. To do so, use ``-sFORCE_FILESYSTEM`` which forces the full
  filesystem API to be supported from JS.

- WasmFS requires malloc internally, so you cannot build with
  ``-sWASMFS -sMALLOC=none``. If you want the smallest possible malloc, use
  ``-sMALLOC=emmalloc``. (Note that the optimizer may be able to remove WasmFS
  and malloc, if your code does not actually use files in a non-trivial way.)

Including File System Support
=============================

Emscripten decides whether to include file system support automatically. Many programs don't need files, and file system support is not negligible in size, so Emscripten avoids including it when it doesn't see a reason to. That means that if your C/C++ code does not access files, then  the ``FS`` object and other file system APIs will not be included in the output. And, on the other hand, if your C/C++ code does use files, then file system support will be automatically included. So normally things will "just work" and you don't need to think about this at all.

However, if your C/C++ code doesn't use files, but you want to use them from JavaScript, then you can build with ``-sFORCE_FILESYSTEM``, which will make the compiler include file system support even though it doesn't see it being used.

On the other hand, if you want to **not** include any filesystem support code (which may be included even due to printf or iostreams, due to how musl and libc++ are structured), you can build with ``-sFILESYSTEM=0``. Very simple stdout support will be included if necessary in such a case, enough for printf and such to work, but no filesystem code will be added, which can save a significant amount of code size.


.. _filesystem-api-persist-data:

Persistent data
===============

Applications compiled with Emscripten usually expect synchronous I/O, so Emscripten itself provides file systems with completely synchronous interfaces.

However, due to JavaScript's event-driven nature, most *persistent* storage options offer only asynchronous interfaces. Emscripten offers :ref:`multiple file systems <filesystem-api-filesystems>` that can be mounted with :js:func:`FS.mount` to help deal with persistence depending on the execution context.

.. _filesystem-api-filesystems:

File systems
============

.. note:: Only the :ref:`MEMFS <filesystem-api-memfs>` filesystem is included by default. All others must be enabled explicitly, using ``-lnodefs.js`` (:ref:`NODEFS <filesystem-api-nodefs>`), ``-lidbfs.js`` (:ref:`IDBFS <filesystem-api-idbfs>`), ``-lworkerfs.js`` (:ref:`WORKERFS <filesystem-api-workerfs>`), or ``-lproxyfs.js`` (:ref:`PROXYFS <filesystem-api-proxyfs>`).

.. _filesystem-api-memfs:

MEMFS
-----

This is the default file system mounted at ``/`` when the runtime is initialized. All files exist strictly in-memory, and any data written to them is lost when the page is reloaded.

.. _filesystem-api-nodefs:

NODEFS
------

.. note:: This file system is only for use when running inside :term:`node.js`.

This file system lets a program in *node* map directories (via a mount operation) on the host filesystem to directories in Emscripten's virtual filesystem. It uses node's synchronous `FS API <http://nodejs.org/api/fs.html>`_ to immediately persist any data written to the Emscripten file system to your local disk.

See `this test <https://github.com/emscripten-core/emscripten/blob/main/test/fs/test_nodefs_rw.c>`_ for an example.

.. _filesystem-api-noderawfs:

NODERAWFS
---------

.. note:: This file system is only for use when running inside :term:`node.js`.

This is a special backend as it replaces all normal filesystem access with direct Node.js operations, without the need to do `FS.mount()`. The initial working directory will be same as process.cwd() instead of VFS root directory.  Because this mode directly uses Node.js to access the real local filesystem on your OS, the code will not necessarily be portable between OSes - it will be as portable as a Node.js program would be, which means that differences in how the underlying OS handles permissions and errors and so forth may be noticeable.  This has mostly been tested on Linux so far.

See `this <https://github.com/emscripten-core/emscripten/blob/d936e807c4d7a6163827c1fdc4a8e87abe41db44/tests/fs/test_nodefs_rw.c#L31>`_ section on NODEFS, where you can see a mount operation - this is not needed in NODERAWFS.

.. _filesystem-api-idbfs:

IDBFS
-----

.. note:: This file system is only for use when running code inside a browser.

The *IDBFS* file system implements the :js:func:`FS.syncfs` interface, which when called will persist any operations to an ``IndexedDB`` instance.

This is provided to overcome the limitation that browsers do not offer synchronous APIs for persistent storage, and so (by default) all writes exist only temporarily in-memory.

If the mount option `autoPersist: true` is passed when mounting IDBFS, then whenever any changes are made to the IDBFS directory tree, they will be automatically persisted to the IndexedDB backend. This lets users avoid needing to manually call `FS.syncfs` to persist changes to the IDBFS mounted directory tree.

.. _filesystem-api-workerfs:

WORKERFS
--------

.. note:: This file system is only for use when running code inside a worker.

This file system provides read-only access to ``File`` and ``Blob`` objects inside a worker without copying the entire data into memory and can potentially be used for huge files.

.. _filesystem-api-proxyfs:

PROXYFS
--------

This allows a module to mount another module's file system. This is useful when separate modules need to share a file system without manually syncing file contents. For example:

.. code-block:: js

  // Module 2 can use the path "/fs1" to access and modify Module 1's filesystem
  module2.FS.mkdir("/fs1");
  module2.FS.mount(module2.PROXYFS, {
      root: "/",
      fs: module1.FS
  }, "/fs1");


Devices
=======

Emscripten supports registering arbitrary device drivers composed of a device id and a set of device-specific stream callbacks. Once a driver has been registered with :js:func:`FS.registerDevice`, a device node can be created to reference it (using :js:func:`FS.mkdev`).

The device node acts as an interface between the device and the file system. Any stream referencing the new node will inherit the stream callbacks registered for the device, making all of the high-level FS operations transparently interact with the device.

.. note:: Every device is different and unique. While common file operations like ``open``, ``close``, ``read``, and ``write`` are typically supported (and inherited by file streams to provide a layer of abstraction for the equivalent *libc* functions to call), each device should implement whatever callbacks it needs based on its unique characteristics.

.. js:function:: FS.makedev(ma, mi)

  Converts a major and minor number into a single unique integer. This is used as an id to represent the device.

  :param ma: Major number.
  :param mi: Minor number.



.. js:function:: FS.registerDevice(dev, ops)

  Registers the specified device driver with a set of callbacks.

  :param dev: The specific device driver id, created using :js:func:`makedev`.
  :param object ops: The set of callbacks required by the device. For an example, see the `NODEFS default callbacks <https://github.com/emscripten-core/emscripten/blob/1.29.12/src/library_nodefs.js#L213>`_.



Setting up standard I/O devices
===============================

Emscripten standard I/O works by going though the virtual ``/dev/stdin``, ``/dev/stdout`` and ``/dev/stderr`` devices. You can set them up using your own I/O functions by calling :js:func:`FS.init`.

By default:

-  ``stdin`` will read from the terminal in command line engines and use ``window.prompt()`` in browsers (in both cases, with line buffering).
-  ``stdout`` will use a ``print`` function if one such is defined, printing to the terminal in command line engines and to the browser console in browsers that have a console (again, line-buffered).
-  ``stderr`` will use the same output function as ``stdout``.

.. note:: All the configuration should be done before the main ``run()`` method is executed, typically by implementing :js:attr:`Module.preRun`. See :ref:`Interacting-with-code` for more information.


.. js:function:: FS.init(input, output, error)

  Sets up standard I/O devices for ``stdin``, ``stdout``, and ``stderr``.

  The devices are set up using the following (optional) callbacks. If any of the callbacks throw an exception, it will be caught and handled as if the device malfunctioned.

  :param input: Input callback. This will be called with no parameters whenever the program attempts to read from ``stdin``. It should return an ASCII character code when data is available, or ``null`` when it isn't.
  :param output: Output callback. This will be called with an ASCII character code whenever the program writes to ``stdout``. It may also be called with ``null`` to flush the output.
  :param error: Error callback. This is similar to ``output``, except it is called when data is written to ``stderr``.


File system API
===============

.. note:: Functions derived from libc like ``FS.readdir()`` use all-lowercase names, whereas added functions like ``FS.readFile()`` use camelCase names.

.. js:function:: FS.mount(type, opts, mountpoint)

  Mounts the FS object specified by ``type`` to the directory specified by ``mountpoint``. The ``opts`` object is specific to each file system type.

  :param type: The :ref:`file system type <filesystem-api-filesystems>`: ``MEMFS``, ``NODEFS``, ``IDBFS`` or ``WORKERFS``.
  :param object opts: A generic settings object used by the underlying file system.

    ``NODEFS`` uses the `root` parameter to map the Emscripten directory to the physical directory. For example, to mount the current folder as a NODEFS instance:

    .. code-block:: javascript

       FS.mkdir('/working');
       FS.mount(NODEFS, { root: '.' }, '/working');

    ``WORKERFS`` accepts `files` and `blobs` parameters to map a provided flat list of files into the ``mountpoint`` directory:

    .. code-block:: javascript

       var blob = new Blob(['blob data']);
       FS.mkdir('/working');
       FS.mount(WORKERFS, {
         blobs: [{ name: 'blob.txt', data: blob }],
         files: files, // Array of File objects or FileList
       }, '/working');


    You can also pass in a package of files, created by ``tools/file_packager`` with ``--separate-metadata``. You must
    provide the metadata as a JSON object, and the data as a blob:

    .. code-block:: javascript

       // load metadata and blob using XMLHttpRequests, or IndexedDB, or from someplace else
       FS.mkdir('/working');
       FS.mount(WORKERFS, {
         packages: [{ metadata: meta, blob: blob }]
       }, '/working');


  :param string mountpoint: A path to an existing local Emscripten directory where the file system is to be mounted. It can be either an absolute path, or something relative to the current directory.


.. js:function:: FS.unmount(mountpoint)

  Unmounts the specified ``mountpoint``.

  :param string mountpoint: The directory to unmount.

.. js:function:: FS.syncfs(populate, callback)

  Responsible for iterating and synchronizing all mounted file systems in an
  asynchronous fashion.

  .. note:: Currently, only the :ref:`filesystem-api-idbfs` file system implements the
    interfaces needed for synchronization. All other file systems are completely
    synchronous and don't require synchronization.

  The ``populate`` flag is used to control the intended direction of the
  underlying synchronization between Emscripten`s internal data, and the file
  system's persistent data.

  For example:

  .. code-block:: javascript

     function myAppStartup(callback) {
       FS.mkdir('/data');
       FS.mount(IDBFS, {}, '/data');

       FS.syncfs(true, function (err) {
       // handle callback
       });
     }

     function myAppShutdown(callback) {
       FS.syncfs(function (err) {
       // handle callback
       });
     }

  A real example of this functionality can be seen in `test_idbfs_sync.c <https://github.com/emscripten-core/emscripten/blob/main/test/fs/test_idbfs_sync.c>`_.

  :param bool populate: ``true`` to initialize Emscripten's file system data with the data from the file system's persistent source, and ``false`` to save Emscripten`s file system data to the file system's persistent source.
  :param callback: A notification callback function that is invoked on completion of the synchronization. If an error occurred, it will be provided as a parameter to this function.


.. js:function:: FS.mkdir(path, mode)

  Creates a new directory node in the file system. For example:

  .. code-block:: javascript

    FS.mkdir('/data');

  .. note:: The underlying implementation does not support user or group permissions. The caller is always treated as the owner of the folder, and only permissions relevant to the owner apply.

  :param string path: The path name for the new directory node.
  :param int mode: :ref:`File permissions <fs-read-and-write-flags>` for the new node. The default setting (`in octal numeric notation <http://en.wikipedia.org/wiki/File_system_permissions#Numeric_notation>`_) is 0777.


.. js:function:: FS.mkdev(path, mode, dev)

  Creates a new device node in the file system referencing the registered device driver (:js:func:`FS.registerDevice`) for ``dev``. For example:

  .. code-block:: javascript

    var id = FS.makedev(64, 0);
    FS.registerDevice(id, {});
    FS.mkdev('/dummy', id);

  :param string path: The path name for the new device node.
  :param int mode: :ref:`File permissions <fs-read-and-write-flags>` for the new node. The default setting (`in octal numeric notation <http://en.wikipedia.org/wiki/File_system_permissions#Numeric_notation>`_) is 0777.
  :param int dev: The registered device driver.


.. js:function:: FS.symlink(oldpath, newpath)

  Creates a symlink node at ``newpath`` linking to ``oldpath``. For example:

  .. code-block:: javascript

    FS.writeFile('file', 'foobar');
    FS.symlink('file', 'link');

  :param string oldpath: The path name of the file to link to.
  :param string newpath: The path to the new symlink node, that points to ``oldpath``.



.. js:function:: FS.rename(oldpath, newpath)

  Renames the node at ``oldpath`` to ``newpath``. For example:

  .. code-block:: javascript

    FS.writeFile('file', 'foobar');
    FS.rename('file', 'newfile');

  :param string oldpath: The old path name.
  :param string newpath: The new path name


.. js:function:: FS.rmdir(path)

  Removes an empty directory located at ``path``.

  Example

  .. code-block:: javascript

    FS.mkdir('data');
    FS.rmdir('data');

  :param string path: Path of the directory to be removed.


.. js:function:: FS.unlink(path)

  Unlinks the node at ``path``.

  This removes a name from the file system. If that name was the last link to a file (and no processes have the file open) the file is deleted.

  For example:

  .. code-block:: javascript

    FS.writeFile('/foobar.txt', 'Hello, world');
    FS.unlink('/foobar.txt');

  :param string path: Path of the target node.


.. js:function:: FS.readlink(path)

  Gets the string value stored in the symbolic link at ``path``. For example:

  .. code-block:: none

    #include <stdio.h>
    #include <emscripten.h>

    int main() {
      MAIN_THREAD_EM_ASM(
      FS.writeFile('file', 'foobar');
      FS.symlink('file', 'link');
      console.log(FS.readlink('link'));
      );
      return 0;
    }

  outputs::

    file

  :param string path: Path to the target file.
  :returns: The string value stored in the symbolic link at ``path``.


.. js:function:: FS.stat(path)

  Gets a JavaScript object containing statistics about the node at ``path``. For example:

  .. code-block:: none

    #include <stdio.h>
    #include <emscripten.h>

    int main() {
      MAIN_THREAD_EM_ASM(
      FS.writeFile('file', 'foobar');
      console.log(FS.stat('file'));
      );
      return 0;
    }

  outputs::

    {
      dev: 1,
      ino: 13,
      mode: 33206,
      nlink: 1,
      uid: 0,
      gid: 0,
      rdev: 0,
      size: 6,
      atime: Mon Nov 25 2013 00:37:27 GMT-0800 (PST),
      mtime: Mon Nov 25 2013 00:37:27 GMT-0800 (PST),
      ctime: Mon Nov 25 2013 00:37:27 GMT-0800 (PST),
      blksize: 4096,
      blocks: 1
    }

  :param string path: Path to the target file.


.. js:function:: FS.lstat(path)

  Identical to :js:func:`FS.stat`, However, if ``path`` is a symbolic link then the returned stats will be for the link itself, not the file that it links to.

  :param string path: Path to the target file.


.. js:function:: FS.chmod(path, mode)

  Change the mode flags for ``path`` to ``mode``.

  .. note:: The underlying implementation does not support user or group permissions. The caller is always treated as the owner of the folder, and only permissions relevant to the owner apply.

  For example:

  .. code-block:: javascript

    FS.writeFile('forbidden', 'can\'t touch this');
    FS.chmod('forbidden', 0000);

  :param string path: Path to the target file.
  :param int mode: The new :ref:`file permissions <fs-read-and-write-flags>` for ``path``, `in octal numeric notation <http://en.wikipedia.org/wiki/File_system_permissions#Numeric_notation>`_.


.. js:function:: FS.lchmod(path, mode)

  Identical to :js:func:`FS.chmod`. However, if ``path`` is a symbolic link then the mode will be set on the link itself, not the file that it links to.

  :param string path: Path to the target file.
  :param int mode: The new :ref:`file permissions <fs-read-and-write-flags>` for ``path``, `in octal numeric notation <http://en.wikipedia.org/wiki/File_system_permissions#Numeric_notation>`_.


.. js:function:: FS.fchmod(fd, mode)

  Identical to :js:func:`FS.chmod`. However, a raw file descriptor is supplied as ``fd``.

  :param int fd: Descriptor of target file.
  :param int mode: The new :ref:`file permissions <fs-read-and-write-flags>` for ``path``, `in octal numeric notation <http://en.wikipedia.org/wiki/File_system_permissions#Numeric_notation>`_.


.. js:function:: FS.chown(path, uid, gid)

  Change the ownership of the specified file to the given user or group id.

  .. note:: |note-completeness|

  :param string path: Path to the target file.
  :param int uid: The id of the user to take ownership of the file.
  :param int gid: The id of the group to take ownership of the file.


.. js:function:: FS.lchown(path, uid, gid)

  Identical to :js:func:`FS.chown`. However, if ``path`` is a symbolic link then the properties will be set on the link itself, not the file that it links to.

  .. note:: |note-completeness|

  :param string path: Path to the target file.
  :param int uid: The id of the user to take ownership of the file.
  :param int gid: The id of the group to take ownership of the file.


.. js:function:: FS.fchown(fd, uid, gid)

  Identical to :js:func:`FS.chown`. However, a raw file descriptor is supplied as ``fd``.

  .. note:: |note-completeness|

  :param int fd: Descriptor of target file.
  :param int uid: The id of the user to take ownership of the file.
  :param int gid: The id of the group to take ownership of the file.



.. js:function:: FS.truncate(path, len)

  Truncates a file to the specified length. For example:

  .. code-block:: none

    #include <stdio.h>
    #include <emscripten.h>

    int main() {
      MAIN_THREAD_EM_ASM(
      FS.writeFile('file', 'foobar');
      FS.truncate('file', 3);
      console.log(FS.readFile('file', { encoding: 'utf8' }));
      );
      return 0;
    }

  outputs::

    foo

  :param string path: Path of the file to be truncated.
  :param int len: The truncation length for the file.


.. js:function:: FS.ftruncate(fd, len)

  Truncates the file identified by the ``fd`` to the specified length (``len``).

  :param int fd: Descriptor of file to be truncated.
  :param int len: The truncation length for the file.


.. js:function:: FS.utime(path, atime, mtime)

  Change the timestamps of the file located at ``path``. The times passed to the arguments are in *milliseconds* since January 1, 1970 (midnight UTC/GMT).

  Note that in the current implementation the stored timestamp is a single value, the maximum of ``atime`` and ``mtime``.

  :param string path: The path of the file to update.
  :param int atime: The file access time (milliseconds).
  :param int mtime: The file modify time (milliseconds).



.. js:function:: FS.open(path, flags [, mode])

  Opens a file with the specified flags. ``flags`` can be:

  .. _fs-read-and-write-flags:

  - ``r`` — Open file for reading.
  - ``r+`` — Open file for reading and writing.
  - ``w`` — Open file for writing.
  - ``wx`` — Like ``w`` but fails if path exists.
  - ``w+`` — Open file for reading and writing. The file is created if it does not exist or truncated if it exists.
  - ``wx+`` — Like ``w+`` but fails if path exists.
  - ``a`` — Open file for appending. The file is created if it does not exist.
  - ``ax`` — Like ``a`` but fails if path exists.
  - ``a+`` — Open file for reading and appending. The file is created if it does not exist.
  - ``ax+`` — Like ``a+`` but fails if path exists.

  .. note:: The underlying implementation does not support user or group permissions. The file permissions set in ``mode`` are only used if the file is created. The caller is always treated as the owner of the file, and only those permissions apply.


  :param string path: The path of the file to open.
  :param string flags: Read and write :ref:`flags <fs-read-and-write-flags>`.
  :param mode: File permission :ref:`flags <fs-read-and-write-flags>` for the file. The default setting (`in octal numeric notation <http://en.wikipedia.org/wiki/File_system_permissions#Numeric_notation>`_) is 0666.
  :returns: A stream object.



.. js:function:: FS.close(stream)

  Closes the file stream.

  :param object stream: The stream to be closed.



.. js:function:: FS.llseek(stream, offset, whence)

  Repositions the offset of the stream ``offset`` bytes relative to the beginning, current position, or end of the file, depending on the ``whence`` parameter.

  The ``_llseek()`` function repositions the ``offset`` of the open file associated with the file descriptor ``fd`` to ``(offset_high<<32) | offset_low`` bytes relative to the beginning of the file, the current position in the file, or the end of the file, depending on whether whence is ``SEEK_SET``, ``SEEK_CUR``, or ``SEEK_END``, respectively. It returns the resulting file position in the argument result.

  .. todo:: **HamishW** Above sentence does not make sense. Have requested feedback.

  :param object stream: The stream for which the offset is to be repositioned.
  :param int offset: The offset (in bytes) relative to ``whence``.
  :param int whence: Point in file (beginning, current point, end) from which to calculate the offset: ``SEEK_SET`` (0), ``SEEK_CUR`` (1) or ``SEEK_END`` (2)


.. js:function:: FS.read(stream, buffer, offset, length [, position])

  Read ``length`` bytes from the stream, storing them into ``buffer`` starting at ``offset``.

  By default, reading starts from the stream's current offset, however, a specific offset can be specified with the ``position`` argument. For example:

  .. code-block:: javascript

    var stream = FS.open('abinaryfile', 'r');
    var buf = new Uint8Array(4);
    FS.read(stream, buf, 0, 4, 0);
    FS.close(stream);

  :param object stream: The stream to read from.
  :param ArrayBufferView buffer: The buffer to store the read data.
  :param int offset: The offset within ``buffer`` to store the data.
  :param int length: The length of data to write in ``buffer``.
  :param int position: The offset within the stream to read. By default this is the stream's current offset.



.. js:function:: FS.write(stream, buffer, offset, length[, position])

  Writes ``length`` bytes from ``buffer``, starting at ``offset``.

  By default, writing starts from the stream's current offset, however, a specific offset can be specified with the ``position`` argument. For example:

  .. code-block:: javascript

    var data = new Uint8Array(32);
    var stream = FS.open('dummy', 'w+');
    FS.write(stream, data, 0, data.length, 0);
    FS.close(stream);

  :param object stream: The stream to write to.
  :param ArrayBufferView buffer: The buffer to write.
  :param int offset: The offset within ``buffer`` to write.
  :param int length: The length of data to write.
  :param int position: The offset within the stream to write. By default this is the stream's current offset.




.. js:function:: FS.readFile(path, opts)

  Reads the entire file at ``path`` and returns it as a ``string`` (encoding is ``utf8``), or as a new ``Uint8Array`` buffer (encoding is ``binary``).

  :param string path: The file to read.
  :param object opts:

    - **encoding** (*string*)
      Defines the encoding used to return the file contents: ``binary`` | ``utf8`` . The default is ``binary``
    - **flags** (*string*)
      Read flags, as defined in :js:func:`FS.open`. The default is 'r'.

  :returns: The file as a ``string`` or ``Uint8Array`` buffer, depending on the encoding.



.. js:function:: FS.writeFile(path, data, opts)

  Writes the entire contents of ``data`` to the file at ``path``. For example:

  .. code-block:: javascript

    FS.writeFile('file', 'foobar');
    var contents = FS.readFile('file', { encoding: 'utf8' });

  :param string path: The file to which to write ``data``.
  :param string|ArrayBufferView data: The data to write. A string will always be decoded as UTF-8.
  :param object opts:

    - **flags** (*string*)
      Write flags, as defined in :js:func:`FS.open`. The default is 'w'.



.. js:function:: FS.createLazyFile(parent, name, url, canRead, canWrite)

  Creates a file that will be loaded lazily on first access from a given URL or local file system path, and returns a reference to it.

  .. warning:: Firefox and Chrome have recently disabled synchronous binary XHRs, which means this cannot work for JavaScript in regular HTML pages (but it works within Web Workers).

  Example

  .. code-block:: javascript

    FS.createLazyFile('/', 'foo', 'other/page.htm', true, false);
    FS.createLazyFile('/', 'bar', '/get_file.php?name=baz', true, true);


  :param parent: The parent folder, either as a path (e.g. `'/usr/lib'`) or an object previously returned from a `FS.mkdir()` or `FS.createPath()` call.
  :type parent: string/object
  :param string name: The name of the new file.
  :param string url: In the browser, this is the URL whose contents will be returned when this file is accessed. In a command line engine like *node.js*, this will be the local (real) file system path from where the contents will be loaded. Note that writes to this file are virtual.
  :param bool canRead: Whether the file should have read permissions set from the program's point of view.
  :param bool canWrite: Whether the file should have write permissions set from the program's point of view.
  :returns: A reference to the new file.



.. js:function:: FS.createPreloadedFile(parent, name, url, canRead, canWrite)

  Preloads a file asynchronously, and uses preload plugins to prepare its content. You should call this in ``preRun``, ``run()`` will be delayed until all preloaded files are ready. This is how the :ref:`preload-file <emcc-preload-file>` option works in *emcc* when ``--use-preload-plugins`` has been specified (if you use this method by itself, you will need to build the program with that option).

  :param parent: The parent folder, either as a path (e.g. **'/usr/lib'**) or an object previously returned from a `FS.mkdir()` or `FS.createPath()` call.
  :type parent: string/object
  :param string name: The name of the new file.
  :param string url: In the browser, this is the URL whose contents will be returned when the file is accessed. In a command line engine, this will be the local (real) file system path the contents will be loaded from. Note that writes to this file are virtual.
  :param bool canRead: Whether the file should have read permissions set from the program's point of view.
  :param bool canWrite: Whether the file should have write permissions set from the program's point of view.



.. js:data:: FS.trackingDelegate[callback name]

  Users can specify callbacks to receive different filesystem events. This is useful for tracking changes in the filesystem. This requires -sFS_DEBUG.

  .. _fs-callback-names:

  - ``willMovePath`` — Indicates path is about to be moved.
  - ``onMovePath`` — Indicates path is moved.
  - ``willDeletePath`` — Indicates path is about to be deleted.
  - ``onDeletePath`` — Indicates path deleted.
  - ``onOpenFile`` — Indicates file is opened.
  - ``onWriteToFile`` — Indicates file is being written to and number of bytes written.
  - ``onReadFile`` — Indicates file is being read and number of bytes read.
  - ``onSeekFile`` — Indicates seeking within a file, position, and whence.
  - ``onCloseFile`` — Indicates a file being closed.

  :callback name: The name of the callback that indicates the filesystem event

  Example Code

  .. code-block:: javascript

    EM_ASM(
      FS.trackingDelegate['willMovePath'] = function(oldpath, newpath) {
        out('About to move "' + oldpath + '" to "' + newpath + '"');
      };
      FS.trackingDelegate['onMovePath'] = function(oldpath, newpath) {
        out('Moved "' + oldpath + '" to "' + newpath + '"');
      };
      FS.trackingDelegate['willDeletePath'] = function(path) {
        out('About to delete "' + path + '"');
      };
      FS.trackingDelegate['onDeletePath'] = function(path) {
        out('Deleted "' + path + '"');
      };
      FS.trackingDelegate['onOpenFile'] = function(path, flags) {
        out('Opened "' + path + '" with flags ' + flags);
      };
      FS.trackingDelegate['onReadFile'] = function(path, bytesRead) {
        out('Read ' + bytesRead + ' bytes from "' + path + '"');
      };
      FS.trackingDelegate['onWriteToFile'] = function(path, bytesWritten) {
        out('Wrote to file "' + path + '" with ' + bytesWritten + ' bytes written');
      };
      FS.trackingDelegate['onSeekFile'] = function(path, position, whence) {
        out('Seek on "' + path + '" with position ' + position + ' and whence ' + whence);
      };
      FS.trackingDelegate['onCloseFile'] = function(path) {
        out('Closed ' + path);
      };
      FS.trackingDelegate['onMakeDirectory'] = function(path, mode) {
        out('Created directory ' + path + ' with mode ' + mode);
      };
      FS.trackingDelegate['onMakeSymlink'] = function(oldpath, newpath) {
        out('Created symlink from ' + oldpath + ' to ' + newpath);
      };
    );

    FILE *file;
    file = fopen("/test.txt", "w");
    fputs("hello world", file);
    fclose(file);
    rename("/test.txt", "/renamed.txt");
    file = fopen("/renamed.txt", "r");
    char str[256] = {};
    fgets(str, 255, file);
    printf("File read returned '%s'\n", str);
    fclose(file);
    remove("/renamed.txt");
    mkdir("/home/test", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    symlink("/renamed.txt", "/file.txt");


  Example Output

  .. code-block:: text

    Opened "/test.txt" with flags O_CREAT O_TRUNC O_WRONLY and file size 0
    Wrote to file "/test.txt" with 11 bytes written
    Wrote to file "/test.txt" with 0 bytes written
    Closed /test.txt
    About to move "/test.txt" to "/renamed.txt"
    Moved "/test.txt" to "/renamed.txt"
    Opened "/renamed.txt" with flags O_RDONLY and file size 11
    Read 0 bytes from "/renamed.txt"
    Read 11 bytes from "/renamed.txt"
    Read 0 bytes from "/renamed.txt"
    Read 0 bytes from "/renamed.txt"
    Wrote to file "/dev/tty" with 31 bytes written
    File read returned 'hello world'
    Wrote to file "/dev/tty" with 2 bytes written
    Closed /renamed.txt
    About to delete "/renamed.txt"
    Deleted "/renamed.txt"
    Created directory "/home/test" with mode 16893
    Created symlink from "/renamed.txt" to "/file.txt"



File types
==========

Emscripten's file system supports regular files, directories, symlinks, character devices, block devices and sockets. Similarly to most Unix systems, all of these file types can be operated on using the higher-level FS operations like :js:func:`FS.read` and :js:func:`FS.write`.


.. js:function:: FS.isFile(mode)

  Tests if the ``mode`` bitmask represents a file.

  :param mode: A bitmask of possible file properties.
  :returns: ``true`` if the ``mode`` bitmask represents a file.
  :rtype: bool


.. js:function:: FS.isDir(mode)

  Tests if the ``mode`` bitmask represents a directory.

  :returns: ``true`` if the ``mode`` bitmask represents a directory.
  :rtype: bool



.. js:function:: FS.isLink(mode)

  Tests if the ``mode`` bitmask represents a symlink.

  :param mode: A bitmask of possible file properties.
  :returns: ``true`` if the ``mode`` bitmask represents a symlink.
  :rtype: bool


.. js:function:: FS.isChrdev(mode)

  Tests if the ``mode`` bitmask represents a character device.

  :param mode: A bitmask of possible file properties.
  :returns: ``true`` if the ``mode`` bitmask represents a character device.
  :rtype: bool


.. js:function:: FS.isBlkdev(mode)

  Tests if the ``mode`` bitmask represents a block device.

  :param mode: A bitmask of possible file properties.
  :returns: ``true`` if the ``mode`` bitmask represents a block device.
  :rtype: bool


.. js:function:: FS.isSocket(mode)

  Tests if the ``mode`` bitmask represents a socket.

  :param mode: A bitmask of possible file properties.
  :returns: ``true`` if the ``mode`` bitmask represents a socket.
  :rtype: bool


Paths
=====


.. js:function:: FS.cwd()

  Gets the current working directory.

  :returns: The current working directory.


.. js:function:: FS.chdir(path)

  Sets the current working directory.

  :param string path: The path to set as current working directory.


.. js:function:: FS.readdir(path)

  Reads the contents of the ``path``.

  :param string path: The incoming path.

  :returns: an array of the names of the files in the directory including ``'.'`` and ``'..'``.

.. js:function:: FS.lookupPath(path, opts)

  Looks up the incoming path and returns an object containing both the resolved path and node.

  The options (``opts``) allow you to specify whether the object, its parent component, a symlink, or the item the symlink points to are returned. For example: ::

    var lookup = FS.lookupPath(path, { parent: true });

  :param string path: The incoming path.
  :param object opts: Options for the path:

    - **parent** (*bool*)
      If true, stop resolving the path once the penultimate component is reached.
      For example, the path ``/foo/bar`` with ``{ parent: true }`` would return an object representing ``/foo``. The default is ``false``.
    - **follow** (*bool*)
      If true, follow the last component if it is a symlink.
      For example, consider a symlink ``/foo/symlink`` that links to ``/foo/notes.txt``. If ``{ follow: true }``, an object representing ``/foo/notes.txt`` would be returned. If ``{ follow: false }``, an object representing the symlink file would be returned. The default is ``false``.

  :returns: an object with the format:

    .. code-block:: javascript

      {
        path: resolved_path,
        node: resolved_node
      }


.. js:function:: FS.analyzePath(path, dontResolveLastLink)

  Looks up the incoming path and returns an object containing information about
  file stats and nodes. Built on top of ``FS.lookupPath`` and provides more
  information about given path and its parent. If any error occurs it won't
  throw but returns an ``error`` property.

  :param string path: The incoming path.
  :param boolean dontResolveLastLink: If true, don't follow the last component
    if it is a symlink.

  :returns: an object with the format:

    .. code-block:: javascript

      {
        isRoot: boolean,
        exists: boolean,
        error: Error,
        name: string,
        path: resolved_path,
        object: resolved_node,
        parentExists: boolean,
        parentPath: resolved_parent_path,
        parentObject: resolved_parent_node
      }


.. js:function:: FS.getPath(node)

  Gets the absolute path to ``node``, accounting for mounts.

  :param node: The current node.
  :returns: The absolute path to ``node``.


.. COMMENT (not rendered): Section below is automated copy and replace text. This is useful where we have boilerplate text.

.. |note-completeness| replace:: This call exists to provide a more "complete" API mapping for ported code. Values set are effectively ignored.
