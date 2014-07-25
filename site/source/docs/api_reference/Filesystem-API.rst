.. _Filesystem-API:

============================
Filesystem API (wiki-import)
============================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

File I/O in emscripten is provided by the
`FS <https://github.com/kripken/emscripten/blob/incoming/src/library_fs.js>`__
library. This same library is used internally for all of emscripten's
libc and libcxx file I/O.

Emscripten deals predominantly with synchronous file I/O, so the
majority of the FS member functions offer a synchronous interface, with
errors being reported by raising exceptions of type ``FS.ErrnorError``.

The file data in emscription is partioned by mounted filesystems, of
which several are provided to work with. By default, an instance of
`MEMFS <Files#memfs>`__ is mounted to ``/`` and instances of
`NODEFS <Files#nodefs>`__ and `IDBFS <Files#idbfs>`__ can be mounted to
other directories if your application needs to `persist
data <Files#persistence>`__.

Persistence
-----------

Applications being compiled with emscripten expect synchronous I/O,
therefore, emscripten itself must provide filesystems with completely
synchronous interfaces. However, due to JavaScript's event-driven
nature, most persistent storage options offer only asynchronous
interfaces.

Because of this, emscripten offers multiple filesystems that can be
mounted with `FS.mount <Files#fsmounttype-opts-mountpoint>`__ to help
deal with persistence depending on the execution context.

MEMFS
~~~~~

This is the default filesystem mounted at ``/`` when the runtime is
initialized. All files exist strictly in-memory, and any data written to
it is lost when the page is reloaded.

NODEFS
~~~~~~

NODEFS lets a program in node directly access files on the local
filesystem, as if the problem were running normally. See `this
test <https://github.com/kripken/emscripten/blob/master/tests/fs/test_nodefs_rw.c>`__
for an example.

Mount options
^^^^^^^^^^^^^

-  root ``string`` Path to persist the data to on the local filesystem.

This filesystem is only for use when running inside of node. It uses
node's synchronous fs API to immediately persist any data written to
emscripten's filesystem to your local disk.

IDBFS
~~~~~

This filesystem is only for use when running inside of the browser. Due
to the browser not offering any synchronous APIs for persistent storage,
by default all writes exist only temporarily in-memory. However, the
IDBFS filesystem implements the
`FS.syncfs <Files#fssyncfspopulate-callback>`__ interface, which once
called will persist any operations to a backing IndexedDB instance.

Devices
-------

Emscripten supports registering arbitrary device drivers composed of a
device id and a set of unique stream callbacks. Once a driver has been
registered with `FS.registerDevice <#fsregisterdevicedev-ops>`__, a
device node (acting as an interface between the device and the
filesystem) can be created to reference it with
`FS.mkdev <#fsmkdevpath-mode-dev>`__. Any stream referencing the new
node will inherit the stream callbacks registered for the device, making
all of the high-level FS operations transparently interact with the
device.

FS.makedev(ma, mi)
^^^^^^^^^^^^^^^^^^

Converts a major and minor number into a single unique integer.

FS.registerDevice(dev, ops)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Registers a device driver for the specified id / callbacks.

Setting up standard I/O devices
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Emscripten standard I/O works by going though the virtual
``/dev/stdin``, ``/dev/stdout`` and ``/dev/stderr`` devices. You can set
them up using your own I/O functions by calling
``FS.init(input_callback, output_callback, error_callback)`` (all
arguments optional). Note that all the configuration should be done
before the main ``run()`` method is executed, typically by implementing
``Module.preRun``, see :ref:`Interacting-with-code`.

-  The input callback will be called with no parameters whenever the
   program attempts to read from ``stdin``. It should return an ASCII
   character code when data is available, or ``null`` when it isn't.
-  The output callback will be called with an ASCII character code
   whenever the program writes to ``stdout``. It may also be called with
   ``null`` to flush the output.
-  The error callback is similar to the output one, except it is called
   when data is written to ``stderr``.

If any of the callbacks throws an exception, it will be caught and
handled as if the device malfunctioned.

By default:

-  ``stdin`` will read from the terminal in command line engines and use
   ``window.prompt()`` in browsers (in both cases, with line buffering).
-  ``stdout`` will use a ``print`` function if one such is defined,
   printing to the terminal in command line engines and to the browser
   console in browsers that have a console (again, line-buffered).
-  ``stderr`` will use the same output function as ``stdout``.

Filesystem
----------

FS.mount(type, opts, mountpoint)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  type `MEMFS <Files#memfs>`__ `NODEFS <Files#nodefs>`__
   `IDBFS <Files#idbfs>`__
-  opts ``object``
-  mountpoint ``string``

Mounts the FS object specified by ``type`` to the directory specified by
``mountpoint``. The ``opts`` objects is specific to each filesystem
type.

FS.syncfs(populate, callback)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  populate ``bool``

Responsible for iterating and synchronizing all mounted filesystems in
an asynchronous fashion.

The ``populate`` flag is used to control the intended direction of the
underlying synchronization between emscripten's internal data, and the
filesystem's persistent data. ``populate=true`` is used for initializing
emscripten's filesystem data with the data from the filesystem's
persistent source, and ``populate=false`` is used to save emscripten's
filesystem data to the filesystem's persistent source.

Example
'''''''

.. code:: javascript

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

An actual test implementing this functionality can be seen at
https://github.com/kripken/emscripten/blob/master/tests/fs/test\_idbfs\_sync.c.

NOTE: Currently, only the `IDBFS <Files#idbfs>`__ filesystem implements
the interfaces needed by this. All other filesystems are completely
synchronous and don't require synchronization.

FS.mkdir(path, mode)
^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  mode ``int`` default = 0777

Creates a new directory node in the filesystem.

Example
'''''''

.. code:: javascript

    FS.mkdir('/data');

FS.mkdev(path, mode, dev)
^^^^^^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  mode ``int`` default = 0777
-  dev ``int``

Creates a new device node in the filesystem referencing the device
driver registered for ``dev``.

Example
'''''''

.. code:: javascript

    var id = FS.makedev(64, 0);
    FS.registerDevice(id, {});
    FS.mkdev('/dummy', id);

FS.symlink(oldpath, newpath)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  oldpath ``string``
-  newpath ``string``

Creates a symlink node at ``newpath`` linking to ``oldpath``.

Example
'''''''

.. code:: javascript

    FS.writeFile('file', 'foobar');
    FS.symlink('file', 'link');

FS.rename(oldpath, newpath)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  oldpath ``string``
-  newpath ``string``

Renames the node at ``oldpath`` to ``newpath``.

Example
'''''''

.. code:: javascript

    FS.writeFile('file', 'foobar');
    FS.rename('file', 'newfile');

FS.rmdir(path)
^^^^^^^^^^^^^^

-  path ``string``

Removes an empty directory located at ``path``.

Example
'''''''

.. code:: javascript

    FS.mkdir('data');
    FS.rmdir('data');

FS.unlink(path)
^^^^^^^^^^^^^^^

-  path ``string``

Unlink the node at ``path``. (This was previously called
``deleteFile``.)

Example
'''''''

.. code:: javascript

    FS.writeFile('/foobar.txt', 'Hello, world');
    FS.unlink('/foobar.txt');

FS.readlink(path)
^^^^^^^^^^^^^^^^^

-  path ``string``

Returns the string value stored in the symbolic link at ``path``.

Example
'''''''

.. code:: c

    #include <stdio.h>
    #include <emscripten.h>

    int main() {
      EM_ASM(
        FS.writeFile('file', 'foobar');
        FS.symlink('file', 'link');
        console.log(FS.readlink('link'));
      );
      return 0;
    }

outputs

::

    file

FS.stat(path)
^^^^^^^^^^^^^

-  path ``string``

Returns a JavaScript object of stats for the node at ``path``.

Example
'''''''

.. code:: c

    #include <stdio.h>
    #include <emscripten.h>

    int main() {
      EM_ASM(
        FS.writeFile('file', 'foobar');
        console.log(FS.stat('file'));
      );
      return 0;
    }

outputs

::

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

FS.lstat(path)
^^^^^^^^^^^^^^

-  path ``string``

Identical to ``FS.stat``, However, if ``path`` is a symbolic link then
the returned stats will be for the link itself, not the file that it
links to.

FS.chmod(path, mode)
^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  mode ``int``

Change the mode flags for ``path`` to ``mode``.

Example
'''''''

.. code:: javascript

    FS.writeFile('forbidden', 'can\'t touch this');
    FS.chmod('forbidden', 0000);

FS.lchmod(path, mode)
^^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  mode ``int``

Identical to ``FS.chmod``. However, if ``path`` is a symbolic link then
the mode will be set on the link itself, not the file that it links to.

FS.fchmod(fd, mode)
^^^^^^^^^^^^^^^^^^^

-  fd ``int``
-  mode ``int``

Identical to ``FS.chmod``. However, a raw file decriptor is supplied as
``fd``.

FS.chown(path, uid, gid)
^^^^^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  uid ``int``
-  gid ``int``

Set ``uid`` and ``gid`` properties of the node at ``path``.

FS.lchown(path, uid, gid)
^^^^^^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  uid ``int``
-  gid ``int``

Identical to ``FS.chown``. However, if path is a symbolic link then the
properties will be set on the link itself, not the file that it links
to.

FS.fchown(fd, uid, gid)
^^^^^^^^^^^^^^^^^^^^^^^

-  fd ``int``
-  uid ``int``
-  gid ``int``

Identical to ``FS.chown``. However, a raw file decriptor is supplied as
``fd``.

FS.truncate(path, len)
^^^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  len ``int``

Truncates a file to the specified length.

Example
'''''''

.. code:: c

    #include <stdio.h>
    #include <emscripten.h>

    int main() {
      EM_ASM(
        FS.writeFile('file', 'foobar');
        FS.truncate('file', 3);
        console.log(FS.readFile('file', { encoding: 'utf8' }));
      );
      return 0;
    }

outputs

::

    foo

FS.ftruncate(fd, len)
^^^^^^^^^^^^^^^^^^^^^

-  fd ``int``
-  len ``int``

Truncates the file identified by the file descriptor to the specified
length.

FS.utime(path, atime, mtime)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  atime ``int``
-  mtime ``int``

Change the timestamps of the file located at ``path``.

FS.open(path, flags, [mode])
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  flags ``sring``
-  mode ``int`` default = 0666

Opens a file with the specified flags. ``flags`` can be:

-  'r' - Open file for reading.
-  'r+' - Open file for reading and writing.
-  'w' - Open file for writing.
-  'wx' - Like 'w' but fails if path exists.
-  'w+' - Open file for reading and writing. The file is created if it
   does not exist or truncated if it exists.
-  'wx+' - Like 'w+' but fails if path exists.
-  'a' - Open file for appending. The file is created if it does not
   exist.
-  'ax' - Like 'a' but fails if path exists.
-  'a+' - Open file for reading and appending. The file is created if it
   does not exist.
-  'ax+' - Like 'a+' but fails if path exists.

``mode`` specifies the permissions for the file, and are only used if
the file is created.

Returns a stream object.

FS.close(stream)
^^^^^^^^^^^^^^^^

-  stream ``object``

Closes the file stream.

FS.llseek(stream, offset, whence)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  stream ``object``
-  offset ``int``
-  whence ``int``

Repositions the offset of the stream ``offset`` bytes, relative to the
``whence`` parameter.

When can be SEEK\_SET (0), SEEK\_CUR(1) or SEEK\_END(2);

FS.read(stream, buffer, offset, length, position)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  stream ``object``
-  buffer ``ArrayBufferView``
-  offset ``int``
-  length ``int``
-  position ``int``

Read ``length`` bytes from the stream, storing them into ``buffer``
starting at ``offset``. By default, reading starts from the stream's
current offset, however, a specific offset can be specified with the
``position`` argument.

Example
'''''''

.. code:: javascript

    var stream = FS.open('abinaryfile', 'r');
    var buf = new Uint8Array(4);
    FS.read(stream, buf, 0, 4, 0);
    FS.close(stream);

FS.write(stream, buffer, offset, length, position)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  stream ``object``
-  buffer ``ArrayBufferView``
-  offset ``int``
-  length ``int``
-  position ``int``

Write ``length`` bytes from ``buffer``, starting at ``offset``. By
default, writing starts from the stream's current offset, however, a
specific offset can be specified with the ``position`` argument.

Example
'''''''

.. code:: javascript

    var data = new Uint8Array(32);
    var stream = FS.open('dummy', 'w+');
    FS.write(stream, data, 0, data.length, 0);
    FS.close(stream);

FS.readFile(path, opts)
^^^^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  opts ``object``
-  flags ``string`` default = 'r'
-  encoding ``string`` default = 'binary'

Slurps the entire file at ``path`` and returns it either as a string, or
a new Uint8Array buffer (``encoding`` = 'binary').

FS.writeFile(path, data, opts)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  data ``ArrayBufferView``
-  opts ``object``
-  flags ``string`` default = 'w'
-  encoding ``string`` default = 'utf8'

Writes the entire contents of ``data`` to the file at ``path``. ``data``
is treated either as a string (``encoding`` = 'utf8'), or as an
ArrayBufferView (``encoding`` = 'binary').

Example
'''''''

.. code:: javascript

    FS.writeFile('file', 'foobar');
    var contents = FS.readFile('file', { encoding: 'utf8' });

FS.createLazyFile(parent, name, url, canRead, canWrite)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Creates a file that will be loaded lazily on first access from a given
URL or local filesystem path, and returns a reference to it. WARNING:
Firefox and Chrome have recently disabled synchronous binary XHRs, which
means this cannot work for Javascript in regular HTML pages (but it
works within WebWorkers).

::

    * `(string|object) parent`: The parent folder, either as a path (e.g. `'/usr/lib'`) or an object previously returned from a `FS.createFolder()` or `FS.createPath()` call.
    * `string name`: The name of the new file.
    * `string url`: In the browser, this is the URL whose contents will be returned when this file is accessed. In a command line engine, this will be the local (real) filesystem path from where the contents will be loaded. Note that writes to this file are virtual.
    * `bool canRead`: Whether the file should have read permissions set from the program's point of view.
    * `bool canWrite`: Whether the file should have write permissions set from the program's point of view.

Example
'''''''

.. code:: javascript

    FS.createLazyFile('/', 'foo', 'other/page.htm', true, false);
    FS.createLazyFile('/', 'bar', '/get_file.php?name=baz', true, true);

FS.createPreloadedFile(parent, name, url, canRead, canWrite)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Preloads a file asychronously. You should call this in preRun, and then
run() will be delayed until all preloaded files are ready. This is how
``--preload-file`` works in emcc.

File types
----------

Emscripten's filesystem supports regular files, directories, symlinks,
character devices, block devices and sockets. In a similar manner to
most Unix systems, all of these file types are able to be operated on
with the higher-level FS operations such as
`FS.read <#fsreadstream-buffer-offset-length-position>`__ and
`FS.write <#fswritestream-buffer-offset-length-position-canown>`__.

FS.isFile(node \|\| mode)
^^^^^^^^^^^^^^^^^^^^^^^^^

Returns true if the mode bitmask represents a file.

FS.isDir(node \|\| mode)
^^^^^^^^^^^^^^^^^^^^^^^^

Returns true if the mode bitmask represents a directory.

FS.isLink(node \|\| mode)
^^^^^^^^^^^^^^^^^^^^^^^^^

Returns true if the mode bitmask represents a symlink.

FS.isChrdev(node \|\| mode)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Returns true if the mode bitmask represents a character device.

FS.isBlkdev(node \|\| mode)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Returns true if the mode bitmask represents a block device.

FS.isSocket(node \|\| mode)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Returns true if the mode bitmask represents a socket.

Paths
-----

FS.cwd()
^^^^^^^^

Return the current working directory.

FS.lookupPath(path, opts)
^^^^^^^^^^^^^^^^^^^^^^^^^

-  path ``string``
-  opts ``object``
-  parent ``bool`` default = false
-  follow ``bool`` default = false

Lookups up the incoming path and returns an object of the format:

::

    {
      path: resolved_path,
      node: resolved_node
    }

The ``parent`` option says to stop resolving the path once the next to
the last component is reached. For example, for the path ``/foo/bar``
with ``{ parent: true }``, would return receive back an object
representing ``/foo``.

The ``follow`` option says whether to follow the last component in the
case that is is a symlink. To illustrate, let's say there exists a
symlink ``/foo/symlink`` that links to ``/foo/notes.txt``. If
``/foo/symlink`` is looked up with ``{ follow: false }`` an object
representing the symlink file itself would be returned. However if
``{ follow: true }`` is passed, an object representing
``/foo/notes.txt`` would be returned.

FS.getPath(node)
^^^^^^^^^^^^^^^^

Takes in a node object and return the absolute path to it, accounting
for mounts.
