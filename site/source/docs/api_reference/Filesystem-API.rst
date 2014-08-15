.. _Filesystem-API:

=====================================
Filesystem API (under-construction)
=====================================

File I/O in Emscripten is provided by the `FS <https://github.com/kripken/emscripten/blob/incoming/src/library_fs.js>`_ library. This same library is used internally for all of Emscripten's libc and libcxx file I/O.

Emscripten deals predominantly with synchronous file I/O, so the majority of the FS member functions offer a synchronous interface, with errors being reported by raising exceptions of type ``FS.ErrnorError``.

The file data in Emscripten is partitioned by mounted filesystems, of which several are provided to work with. By default, an instance of `MEMFS` is mounted to ``/`` and instances of `NODEFS` and `IDBFS` can be mounted to other directories if your application needs to `persist data <Files#persistence>`__.


Persistence
===========

Applications being compiled with Emscripten expect synchronous I/O, therefore, Emscripten itself must provide filesystems with completely synchronous interfaces. However, due to JavaScript's event-driven nature, most persistent storage options offer only asynchronous interfaces.

Because of this, Emscripten offers multiple filesystems that can be mounted with :js:func:`FS.mount` to help deal with persistence depending on the execution context.

MEMFS
===========

This is the default filesystem mounted at ``/`` when the runtime is initialized. All files exist strictly in-memory, and any data written to it is lost when the page is reloaded.

NODEFS
===========

NODEFS lets a program in node directly access files on the local filesystem, as if the problem were running normally. See `this test <https://github.com/kripken/emscripten/blob/master/tests/fs/test_nodefs_rw.c>`__ for an example.

Mount options
-------------

-  root ``string`` Path to persist the data to on the local filesystem.

This filesystem is only for use when running inside of node. It uses node's synchronous fs API to immediately persist any data written to emscripten's filesystem to your local disk.

IDBFS
=====

This filesystem is only for use when running inside of the browser. Due to the browser not offering any synchronous APIs for persistent storage, by default all writes exist only temporarily in-memory. However, the IDBFS filesystem implements the :js:func`FS.syncfs` interface, which once called will persist any operations to a backing IndexedDB instance.

Devices
===========

Emscripten supports registering arbitrary device drivers composed of a device id and a set of unique stream callbacks. Once a driver has been registered with :js:func:`FS.registerDevice`, a device node (acting as an interface between the device and the filesystem) can be created to reference it with :js:func:`FS.mkdev`. Any stream referencing the new node will inherit the stream callbacks registered for the device, making all of the high-level FS operations transparently interact with the device.



.. js:function:: FS.makedev(ma, mi)

	Converts a major and minor number into a single unique integer.
	
	:param ma: **HamishW**
	:param mi: **HamishW**
	:throws **HamishW**:		



.. js:function:: FS.registerDevice(dev, ops)

	Registers a device driver for the specified id / callbacks.
	
	:param dev: ``MEMFS`` ``NODEFS`` ``IDBFS``
	:param object ops: **HamishW**
	:throws **HamishW**:		
	

Setting up standard I/O devices
-------------------------------

Emscripten standard I/O works by going though the virtual ``/dev/stdin``, ``/dev/stdout`` and ``/dev/stderr`` devices. You can set them up using your own I/O functions by calling ``FS.init(input_callback, output_callback, error_callback)`` (all arguments optional). Note that all the configuration should be done before the main ``run()`` method is executed, typically by implementing ``Module.preRun``, see :ref:`Interacting-with-code`.

-  The input callback will be called with no parameters whenever the program attempts to read from ``stdin``. It should return an ASCII character code when data is available, or ``null`` when it isn't.
-  The output callback will be called with an ASCII character code whenever the program writes to ``stdout``. It may also be called with ``null`` to flush the output.
-  The error callback is similar to the output one, except it is called when data is written to ``stderr``.

If any of the callbacks throws an exception, it will be caught and handled as if the device malfunctioned.

By default:

-  ``stdin`` will read from the terminal in command line engines and use ``window.prompt()`` in browsers (in both cases, with line buffering).
-  ``stdout`` will use a ``print`` function if one such is defined, printing to the terminal in command line engines and to the browser console in browsers that have a console (again, line-buffered).
-  ``stderr`` will use the same output function as ``stdout``.


Filesystem
===========


.. js:function:: FS.mount(type, opts, mountpoint)

	Mounts the FS object specified by ``type`` to the directory specified by ``mountpoint``. The ``opts`` objects is specific to each filesystem type.

	:param type: ``MEMFS`` ``NODEFS`` ``IDBFS``
	:param object opts: **HamishW**
	:param string mountpoint: **HamishW**	
	:throws **HamishW**:	



.. js:function:: FS.syncfs(populate, callback)

	Responsible for iterating and synchronizing all mounted filesystems in an asynchronous fashion.

	The ``populate`` flag is used to control the intended direction of the underlying synchronization between Emscripten`s internal data, and the filesystem's persistent data. ``populate=true`` is used for initializing Emscripten's filesystem data with the data from the filesystem's persistent source, and ``populate=false`` is used to save emscripten's filesystem data to the filesystem's persistent source.

	For example:

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

	An actual test implementing this functionality can be seen at https://github.com/kripken/emscripten/blob/master/tests/fs/test\_idbfs\_sync.c.

	.. note:: Currently, only the `IDBFS`_ filesystem implements the interfaces needed by this. All other filesystems are completely synchronous and don't require synchronization.

	:param bool populate: ``true`` to initialize Emscripten's filesystem data with the data from the filesystem's persistent source, and ``false`` to save emscripten's filesystem data to the filesystem's persistent source.
	:param callback: **HamishW**
	:throws **HamishW**:


.. js:function:: FS.mkdir(path, mode)

	Creates a new directory node in the filesystem. For example:

	.. code:: javascript

		FS.mkdir('/data');
	
	:param string path: The path name for the new directory node.
	:param int mode: **HamishW** Link to mode values. The default is 0777.
	:throws **HamishW**:


.. js:function:: FS.mkdev(path, mode, dev)

	Creates a new device node in the filesystem referencing the device driver registered for ``dev``. For example:

	.. code:: javascript

		var id = FS.makedev(64, 0);
		FS.registerDevice(id, {});
		FS.mkdev('/dummy', id);

	:param string path: The path name for the new device node.
	:param int mode: **HamishW** Link to mode values. The default is 0777.
	:param int dev: **HamishW**.
	:throws **HamishW**:


.. js:function:: FS.symlink(oldpath, newpath)

	Creates a symlink node at ``newpath`` linking to ``oldpath``. For example:

	.. code:: javascript

		FS.writeFile('file', 'foobar');
		FS.symlink('file', 'link');

	:param string oldpath: The path name of the file to link to.
	:param string newpath: The path to the new symlink node to ``oldpath``.
	:throws **HamishW**:



.. js:function:: FS.rename(oldpath, newpath)

	Renames the node at ``oldpath`` to ``newpath``. For example:

	.. code:: javascript

		FS.writeFile('file', 'foobar');
		FS.rename('file', 'newfile');

	:param string oldpath: The old path name.
	:param string newpath: The new path name
	:throws **HamishW**:
	

.. js:function:: FS.rmdir(path)

	Removes an empty directory located at ``path``.

	Example

	.. code:: javascript

		FS.mkdir('data');
		FS.rmdir('data');

	:param string path: Path of the directory to be removed.
	:throws **HamishW**:	


.. js:function:: FS.unlink(path)

	Unlinks the node at ``path`` (this was previously called
	``deleteFile``).
	
	.. COMMENT :: **HamishW** What does unlinking actually mean?
	
	For example: 

	.. code:: javascript

		FS.writeFile('/foobar.txt', 'Hello, world');
		FS.unlink('/foobar.txt');

	:param string path: Path of the target node.
	:throws **HamishW**:
	

	
.. js:function:: FS.readlink(path)

	Gets the string value stored in the symbolic link at ``path``. For example: 

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
	
	:param string path: Path of the target file.
	:returns: The string value stored in the symbolic link at ``path``.
	:throws **HamishW**:	
	


.. js:function:: FS.stat(path)

	Gets a JavaScript object of stats for the node at ``path``. For example:

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

	:param string path: Path of the target file.
	:throws **HamishW**:	


.. js:function:: FS.lstat(path)

	Identical to :js:func:`FS.stat`, However, if ``path`` is a symbolic link then the returned stats will be for the link itself, not the file that it links to.

	:param string path: Path of the target file.
	:throws **HamishW**:


.. js:function:: FS.chmod(path, mode)

	Change the mode flags for ``path`` to ``mode``. For example:

	.. code:: javascript

		FS.writeFile('forbidden', 'can\'t touch this');
		FS.chmod('forbidden', 0000);

	:param string path: Path of the target file.
	:param int mode: **HamishW**.
	:throws **HamishW**:



.. js:function:: FS.lchmod(path, mode)

	Identical to :js:func:`FS.chmod`. However, if ``path`` is a symbolic link then the mode will be set on the link itself, not the file that it links to.

	:param string path: Path of the target file.
	:param int mode: **HamishW**.
	:throws **HamishW**:


.. js:function:: FS.fchmod(fd, mode)

	Identical to :js:func:`FS.chmod`. However, a raw file descriptor is supplied as ``fd``.

	:param int fd: Descriptor of target file.
	:param int mode: **HamishW**.
	:throws **HamishW**:




.. js:function:: FS.chown(path, uid, gid)

	Set ``uid`` and ``gid`` properties of the node at ``path``.

	:param string path: Path of the target file.
	:param int uid: **HamishW**.
	:param int gid: **HamishW**.
	:throws **HamishW**:




.. js:function:: FS.lchown(path, uid, gid)

	Identical to Identical to :js:func:`FS.chown`. However, if path is a symbolic link then the properties will be set on the link itself, not the file that it links to.

	:param string path: Path of the target file.
	:param int uid: **HamishW**.
	:param int gid: **HamishW**.
	:throws **HamishW**:



.. js:function:: FS.fchown(fd, uid, gid)

	Identical to :js:func:`FS.chown`. However, a raw file descriptor is supplied as ``fd``.

	:param int fd: Descriptor of target file.
	:param int uid: **HamishW**.
	:param int gid: **HamishW**.
	:throws **HamishW**:

	

.. js:function:: FS.truncate(path, len)

	Truncates a file to the specified length. For example:


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
	
	:param string path: Path of the file to be truncated.
	:param int len: The truncation length for the file.
	:throws ERRNO_CODES.EINVAL:
	:throws ERRNO_CODES.EPERM:
	:throws ERRNO_CODES.EISDIR:
	
	
	
.. js:function:: FS.ftruncate(fd, len)

	Truncates the file identified by the ``fd`` to the specified length (``len``).

	:param int fd: Descriptor of file to be truncated.
	:param int len: The truncation length for the file.
	:throws ERRNO_CODES.EBADF:
	:throws ERRNO_CODES.EINVAL:
	:throws ERRNO_CODES.EPERM:
	:throws ERRNO_CODES.EISDIR:


.. js:function:: FS.utime(path, atime, mtime)

	Change the timestamps of the file located at ``path``. Note that in the current implementation the stored timestamp is a single value, the maximum of ``atime`` and ``mtime``.
	
	:param string path: The path of the file to update.
	:param int atime: The file modify time.
	:param int mtime: The file access time.

	.. COMMENT :: **HamishW** what is the format of the time? Seconds since unix/posix start time in 1970?
	

.. js:function:: FS.open(path, flags [, mode])

	Opens a file with the specified flags. ``flags`` can be:

	.. _fs-read-and-write-flags:
	
	-  'r' - Open file for reading.
	-  'r+' - Open file for reading and writing.
	-  'w' - Open file for writing.
	-  'wx' - Like 'w' but fails if path exists.
	-  'w+' - Open file for reading and writing. The file is created if it does not exist or truncated if it exists.
	-  'wx+' - Like 'w+' but fails if path exists.
	-  'a' - Open file for appending. The file is created if it does not exist.
	-  'ax' - Like 'a' but fails if path exists.
	-  'a+' - Open file for reading and appending. The file is created if it does not exist.
	-  'ax+' - Like 'a+' but fails if path exists.

		
	:param string path: The path of the file to open.
	:param string flags: Read and write :ref:`flags <fs-read-and-write-flags>`.
	:param mode: Permissions for the file. This is only used if the file is created. Default is 0666.
	:returns: A stream object.	

	.. COMMENT:: **HamishW** What mode/settings does 0666 map to? We need a list to possible mode values.

	

.. js:function:: FS.close(stream)

	Closes the file stream.
	
	:param object stream: The stream to be closed.



.. js:function:: FS.llseek(stream, offset, whence)

	Repositions the offset of the stream ``offset`` bytes, relative to the ``whence`` parameter.

	:param object stream: The stream for which the offset is to be repositioned.
	:param int offset: The offset (in bytes) relative to ``whence``.
	:param int whence: SEEK\_SET (0), SEEK\_CUR(1) or SEEK\_END(2);

	.. COMMENT :: **HamishW** I don't understand the whence parameter. Need to follow up and check test code.
	

.. js:function:: FS.read(stream, buffer, offset, length [, position])

	Read ``length`` bytes from the stream, storing them into ``buffer`` starting at ``offset``. 
	
	By default, reading starts from the stream's current offset, however, a specific offset can be specified with the ``position`` argument. For example:

	.. code:: javascript

		var stream = FS.open('abinaryfile', 'r');
		var buf = new Uint8Array(4);
		FS.read(stream, buf, 0, 4, 0);
		FS.close(stream);

	:param object stream: The stream to read from.
	:param ArrayBufferView buffer: The buffer to store the read data.
	:param int offset: The offset within ``buffer`` to store the data.
	:param int length: The length of data to write in ``buffer``.
	:param int position: The offset within the stream to read. By default this is the stream's current offset.
	:throws ERRNO_CODES.EINVAL: Reading from an invalid position or length
	:throws ERRNO_CODES.EBADF:
	:throws ERRNO_CODES.ESPIPE:
	:throws ERRNO_CODES.EISDIR:
	:throws ERRNO_CODES.EINVAL:
	
	
	
.. js:function:: FS.write(stream, buffer, offset, length[, position])

	Writes ``length`` bytes from ``buffer``, starting at ``offset``. 
	
	By default, writing starts from the stream's current offset, however, a specific offset can be specified with the ``position`` argument. For example:

	.. code:: javascript

		var data = new Uint8Array(32);
		var stream = FS.open('dummy', 'w+');
		FS.write(stream, data, 0, data.length, 0);
		FS.close(stream);

	:param object stream: The stream to write to.
	:param ArrayBufferView buffer: The buffer to write.
	:param int offset: The offset within ``buffer`` to write.
	:param int length: The length of data to write.
	:param int position: The offset within the stream to write. By default this is the stream's current offset.
	:throws ERRNO_CODES.EINVAL: Reading from an invalid position or length
	:throws ERRNO_CODES.EBADF:
	:throws ERRNO_CODES.ESPIPE:
	:throws ERRNO_CODES.EISDIR:
	:throws ERRNO_CODES.EINVAL:
	
	.. COMMENT:: Need to check if Throws should be recorded, and if so, what should be said. **HamishW**


	
.. js:function:: FS.readFile(path, opts)

	Reads the entire file at ``path`` and returns it as a ``string`` (encoding is 'utf8'), or as a new ``Uint8Array`` buffer (encoding is 'binary').

	:param string path: The file to read.
	:param object opts:
	
		- **encoding** (*string*)
			Defines the encoding used to return the file contents: 'binary' | 'utf8' . The default is 'binary'		
		- **flags** (*string*)
			Read flags, as defined in :js:func:`FS.open`. The default is 'r'.
			
	:returns: The file as a ``string`` or ``Uint8Array`` buffer, depending on the encoding.



.. js:function:: FS.writeFile(path, data, opts)

	Writes the entire contents of ``data`` to the file at ``path``. 
	
	The value of ``opts`` determines whether ``data`` is treated either as a string (``encoding`` = 'utf8'), or as an ``ArrayBufferView`` (``encoding`` = 'binary'). For example:

	.. code:: javascript

		FS.writeFile('file', 'foobar');
		var contents = FS.readFile('file', { encoding: 'utf8' });
		
	:param string path: The file to which to write ``data``.
	:param ArrayBufferView data: The data to write.
	:param object opts:
	
		- **encoding** (*string*)
			'binary' | 'utf8' . The default is 'utf8'		
		- **flags** (*string*)
			Write flags, as defined in :js:func:`FS.open`. The default is 'w'.


	
.. js:function:: FS.createLazyFile(parent, name, url, canRead, canWrite)

	Creates a file that will be loaded lazily on first access from a given URL or local filesystem path, and returns a reference to it.

	.. warning:: Firefox and Chrome have recently disabled synchronous binary XHRs, which means this cannot work for JavaScript in regular HTML pages (but it works within WebWorkers).

	Example

	.. code:: javascript

		FS.createLazyFile('/', 'foo', 'other/page.htm', true, false);
		FS.createLazyFile('/', 'bar', '/get_file.php?name=baz', true, true);
	
	
	:param parent: The parent folder, either as a path (e.g. `'/usr/lib'`) or an object previously returned from a `FS.createFolder()` or `FS.createPath()` call.
	:type parent: string/object
	:param string name: The name of the new file.
	:param string url: In the browser, this is the URL whose contents will be returned when this file is accessed. In a command line engine, this will be the local (real) filesystem path from where the contents will be loaded. Note that writes to this file are virtual.
	:param bool canRead: Whether the file should have read permissions set from the program's point of view.
	:param bool canWrite: Whether the file should have write permissions set from the program's point of view.
	:returns: A reference to the new file.
	:throws ERRNO_CODES.EIO:
	:throws: if there is an invalid range or URL, or if synchronous binary XHRs have been disabled.
	


.. js:function:: FS.createPreloadedFile(parent, name, url, canRead, canWrite)

	Preloads a file asynchronously. You should call this in ``preRun``, and then ``run()`` will be delayed until all preloaded files are ready. This is how ``--preload-file`` works in *emcc*.
	
	:param parent: The parent folder, either as a path (e.g. `'/usr/lib'`) or an object previously returned from a `FS.createFolder()` or `FS.createPath()` call.
	:type parent: string/object
	:param string name: The name of the new file.
	:param string url: In the browser, this is the URL whose contents will be returned when this file is accessed. In a command line engine, this will be the local (real) filesystem path from where the contents will be loaded. Note that writes to this file are virtual.
	:param bool canRead: Whether the file should have read permissions set from the program's point of view.
	:param bool canWrite: Whether the file should have write permissions set from the program's point of view.



File types
===========

Emscripten's filesystem supports regular files, directories, symlinks, character devices, block devices and sockets. In a similar manner to most Unix systems, all of these file types are able to be operated on with the higher-level FS operations such as :js:func:`FS.read` :js:func:`FS.write`.


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
=======


.. js:function:: FS.cwd()

	Gets the current working directory.

	:returns: The current working directory.
   
   

.. js:function:: FS.lookupPath(path, opts)

	Lookups up the incoming path and returns an object containing both the resolved path and node. 
	
	The ``opts`` allow you to specify whether the object or it's parent component, and whether a symlink or the item it points to are returned. For example: ::
	
		var lookup = FS.lookupPath(path, { parent: true });
	
	:param string path: The incoming path.
	:param object opts: Options for the path:
	
		- **parent** (*bool*) 
			If true, stop resolving the path once the next to the last component is reached. 
			For example, for the path ``/foo/bar`` with ``{ parent: true }``, would return receive back an object representing ``/foo``. The default is ``false``.
		- **follow** (*bool*)
			If true, follow the last component if it is a symlink. 
			For example, consider a symlink ``/foo/symlink`` that links to ``/foo/notes.txt``. if ``{ follow: true }``, an object representing ``/foo/notes.txt`` would be returned. If ``{ follow: false }`` an object representing the symlink file would be returned. The default is ``false``.

	:returns: an object with the the format:
	
		.. code-block:: javascript

			{
			  path: resolved_path,
			  node: resolved_node
			}
	:throws ERRNO_CODES.ELOOP: Lookup caught in a loop (recursive lookup is too deep or there are too many consecutive symlinks).



.. js:function:: FS.getPath(node)

	Gets the absolute path to ``node``, accounting for mounts.
	
	:param node: The current node.
	:returns: The absolute path to ``node``.
