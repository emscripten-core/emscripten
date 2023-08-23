.. _api-reference-advanced-apis:

=============
Advanced APIs
=============

This section lists APIs that are not suitable for general use, but which may be
useful to developers in some circumstances. These include APIs that are
difficult or complicated to use, or which are intended primarily for developers
working on the Emscripten core.

.. contents:: Table of Contents
    :local:
    :depth: 1


.. _settings-js:

settings.js
===========

`settings.js`_ contains default values and options used in various places by the
compiler.

.. Warning:: Many **settings.js** options are highly brittle - certain
   combinations of options, and combinations of certain options used with some
   source code, can cause Emscripten to fail badly. This is intended for use by
   "advanced users", and possibly even only people developing Emscripten itself.

The options in **settings.js** are normally set as command line parameters to
*emcc*::

  emcc -sOPT=VALUE

While it is possible to edit **settings.js** manually, this is *highly
discouraged*. In general **settings.js** defines low-level options that should
not be modified. Note also that the compiler changes some options depending on
other settings. For example, ``ASSERTIONS`` is enabled by default, but disabled
in optimized builds (``-O1+``).

.. note::
  For more information on what options can be used to configure Emscripten, read
  ``src/settings.js`` or visit
  `the emsettings page <https://emsettings.surma.technology>`_.


preamble.js
===========

The following advanced APIs are documented in `preamble.js`_.

.. js:function:: allocate(slab, allocator)

  This is marked as *internal* because it is difficult to use (it has been
  optimized for multiple syntaxes to save space in generated code). Normally
  developers should instead allocate memory using ``_malloc()``, initialize it
  with :js:func:`setValue`, etc., but this function may be useful for advanced
  developers in certain cases.

  :param slab: An array of data, or a number. If a number, then the size of the
               block to allocate, in *bytes*.
  :param allocator: How to allocate memory, see ALLOC_*


Advanced File System API
========================

:ref:`Filesystem-API` covers the public API that will be relevant to most
developers. The following functions are only needed for advanced use-cases (for
example, writing a new local file system) or legacy file system compatibility.

.. js:function:: FS.hashName(parentid, name)
.. js:function:: FS.hashAddNode(node)
.. js:function:: FS.hashRemoveNode(node)
.. js:function:: FS.lookupNode(parent, name)
.. js:function:: FS.createNode(parent, name, mode, rdev)
.. js:function:: FS.destroyNode(node)
.. js:function:: FS.isRoot(node)
.. js:function:: FS.isMountpoint(node)
.. js:function:: FS.isFIFO(node)
.. js:function:: FS.nextfd()
.. js:function:: FS.getStream(fd)
.. js:function:: FS.createStream(stream, fd)
.. js:function:: FS.closeStream(fd)
.. js:function:: FS.getStreamFromPtr(ptr)
.. js:function:: FS.getPtrForStream(stream)
.. js:function:: FS.major(dev)
.. js:function:: FS.minor(dev)
.. js:function:: FS.getDevice(dev)
.. js:function:: FS.getMounts(mount)
.. js:function:: FS.lookup(parent, name)
.. js:function:: FS.mknod(path, mode, dev)
.. js:function:: FS.create(path, mode)
.. js:function:: FS.allocate(stream, offset, length)
.. js:function:: FS.mmap(stream, buffer, offset, length, position, prot, flags)
.. js:function:: FS.ioctl(stream, cmd, arg)
.. js:function:: FS.staticInit()
.. js:function:: FS.quit()
.. js:function:: FS.indexedDB()
.. js:function:: FS.DB_NAME()

  For advanced users only.

.. js:function:: FS.getMode(canRead, canWrite)
.. js:function:: FS.findObject(path, dontResolveLastLink)
.. js:function:: FS.createPath(parent, path, canRead, canWrite)
.. js:function:: FS.createFile(parent, name, properties, canRead, canWrite)
.. js:function:: FS.createDataFile(parent, name, data, canRead, canWrite, canOwn)
.. js:function:: FS.createDevice(parent, name, input, output)
.. js:function:: FS.forceLoadFile(obj)

  Legacy v1 compatibility functions.


There are also a small number of additional :ref:`flag modes <fs-read-and-write-flags>`:

- ``rs``
- ``xw``
- ``xw+``
- ``xa``
- ``xa+``

.. _settings.js: https://github.com/emscripten-core/emscripten/blob/main/src/settings.js
.. _preamble.js: https://github.com/emscripten-core/emscripten/blob/main/src/preamble.js
