.. _api-reference-advanced-apis:

=============
Advanced APIs
=============

This section lists APIs that are not suitable for general use, but which may be useful to developers in some circumstances. These include APIs that are difficult or complicated to use, or which are intended primarily for developers working on the Emscripten core.

.. contents:: Table of Contents
    :local:
    :depth: 1


.. _settings-js:

settings.js
============

`settings.js <https://github.com/kripken/emscripten/blob/master/src/settings.js>`_ contains default values and options used in various places by the compiler. 

.. Warning :: Many **settings.js** options are highly brittle - certain combinations of options, and combinations of certain options used with some source code, can cause Emscripten to fail badly. This is intended for use by "advanced users", and possibly even only people developing Emscripten itself.


The options in **settings.js** are normally set as command line parameters to *emcc*: ::

	emcc -s OPT=VALUE


While it is possible to edit **settings.js** manually, this is *highly discouraged*. In general **settings.js** defines low-level options that should not be modified. Note also that the compiler changes some options depending on other settings. For example, ``ASSERTIONS`` is enabled by default, but disabled in optimized builds (``-O1+``).

The small number of options that developers may have cause to change should be modified when the *emcc* tool is invoked. For example, ``EXPORTED_FUNCTIONS``: ::

	./emcc tests/hello_function.cpp -o function.html -s EXPORTED_FUNCTIONS="['_int_sqrt']"

	
preamble.js
===========

The following advanced APIs are documented in `preamble.js <https://github.com/kripken/emscripten/blob/master/src/preamble.js>`_.

.. js:function:: allocate(slab, types, allocator, ptr)

	This is marked as *internal* because it is difficult to use (it has been optimized for multiple syntaxes to save space in generated code). Normally developers should instead allocate memory using ``_malloc()``, initialize it with :js:func:`setValue`, etc., but this function may be useful for advanced developers in certain cases.
	
	:param slab: An array of data, or a number. If a number, then the size of the block to allocate, in *bytes*. 
	:param types: Either an array of types, one for each byte (or 0 if no type at that position), or a single type which is used for the entire block. This only matters if there is initial data - if ``slab`` is a number, then this value does not matter at all and is ignored.
	:param allocator: How to allocate memory, see ALLOC_*


	
Module.Runtime
================

``Module.Runtime`` gives access to low-level runtime functionality. Some of these, for example ``Runtime.stackSave()`` and ``Runtime.stackRestore()`` may be useful for advanced users.

.. todo:: **HamishW** It would be useful to expand on what is offered by ``Module.Runtime``. This has been deferred.



Advanced File System API
========================

:ref:`Filesystem-API` covers the public API that will be relevant to most developers. The following functions are only needed for advanced use-cases (for example, writing a new local file system) or legacy file system compatibility.

.. js:function:: FS.handleFSError(e)
	FS.handleFSError(e)
	FS.hashName(parentid, name)
	FS.hashAddNode(node)
	FS.hashRemoveNode(node)
	FS.lookupNode(parent, name)
	FS.createNode(parent, name, mode, rdev)
	FS.destroyNode(node)
	FS.isRoot(node)
	FS.isMountpoint(node)
	FS.isFIFO(node)
	FS.nextfd(fd_start, fd_end)
	FS.getStream(fd)
	FS.createStream(stream, fd_start, fd_end)
	FS.closeStream(fd)
	FS.getStreamFromPtr(ptr)
	FS.getPtrForStream(stream)
	FS.major(dev)
	FS.minor(dev)
	FS.getDevice(dev)
	FS.getMounts(mount)
	FS.lookup(parent, name)
	FS.mknod(path, mode, dev)
	FS.create(path, mode)
	FS.readdir(path)
	FS.allocate(stream, offset, length)
	FS.mmap(stream, buffer, offset, length, position, prot, flags)
	FS.ioctl(stream, cmd, arg)
	FS.staticInit()
	FS.quit()
	FS.indexedDB()
	FS.DB_NAME()
	FS.saveFilesToDB(paths, onload, onerror)
	FS.loadFilesFromDB(paths, onload, onerror)
	
	For advanced users only.


.. js:function:: FS.getMode(canRead, canWrite)
	FS.joinPath(parts, forceRelative)
	FS.absolutePath(relative, base)
	FS.standardizePath(path)
	FS.findObject(path, dontResolveLastLink)
	FS.analyzePath(path, dontResolveLastLink)
	FS.createFolder(parent, name, canRead, canWrite)
	FS.createPath(parent, path, canRead, canWrite)
	FS.createFile(parent, name, properties, canRead, canWrite)
	FS.createDataFile(parent, name, data, canRead, canWrite, canOwn)
	FS.createDevice(parent, name, input, output)
	FS.createLink(parent, name, target, canRead, canWrite)
	FS.forceLoadFile(obj)

	Legacy v1 compatibility functions.


There are also a small number of additional :ref:`flag modes <fs-read-and-write-flags>`:

- ``rs``
- ``xw``
- ``xw+``
- ``xa``
- ``xa+``
