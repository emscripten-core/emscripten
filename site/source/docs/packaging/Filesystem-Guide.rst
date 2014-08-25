.. _Filesystem-Guide:

==============================
Filesystem Guide (wiki-import)
==============================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

Filesystem Guide
================

Emscripten allows you to set up a virtual filesystem that points to preloaded data, as well as virtual devices that can read and write data.

There are two basic ways to use the filesystem:

-  Package some files with your build. You can just tell emcc to package a directory or a set of files, and those files will be accessible from the compiled code normally, using ``fopen`` etc.
-  Manually use the FS API from JavaScript. This lets you create, modify etc. files in a more manual manner.

For concrete examples of using the filesystem API, see the automatic tests in ``tests/runner.py``. Search for ``FS.`` for find relevant tests, for example test\_files. For specific examples of how to embed files that can be read from compiled C/C++, see for example the OpenJPEG test.

The reason for the filesystem API is that JavaScript is most often run in web browsers, which sandbox content and prevent it from accessing the local filesystem. Therefore emscripten simulates a filesystem so that C/C++ code can be written normally, as if there were direct access to files. Note: if you want to run in a shell environment using node without sandboxing, then you can let code directly access the local filesystem using NODEFS, see :ref:`Filesystem-API`.

.. _packaging-files:

Packaging files
===============

The simplest thing to do is just tell emcc to package files for you,

::

    emcc file.cpp -o file.html --preload-file asset_dir

That command will generate file.html, and alongside it file.data which will contain all the files in ``asset_dir/``. You can then distribute your application with just those two files.

Assets can also be embedded directly into the html file:

::

    emcc file.cpp -o file.html --embed-file asset_dir

When this is done with a relative path, the prefixes will remain the same in the file system. For example:

::

    emcc file.cpp -o file.html --embed-file ../../asset_dir

Files will all be prefixed with ../../asset\_dir/. To change this behavior, call *emcc* as follows:

::

    emcc file.cpp -o file.html --embed-file ../../assets@/

This will package the files at the root of the file system.

You can also run the file packager manually, ``tools/file_packager.py``. See docs in that file. It generates a ``.data`` file and ``.js`` file, which contains the manual commands to utilize the data file (emcc ``--preload`` just bundles that code in your normal generated output).
You can then load that JavaScript before loading your main compiled code.

-  Note that this lets you do the file packaging without running emcc to compile your code, the two processes are separated this way.
-  Note also that you can load multiple datafiles. Just run the file packager on each and load the ``.js`` outputs. See BananaBread for an example of this (``cube2/js/game-setup.js``).

Customizing the data URL
------------------------

By default the data file (containing all the preloaded files) will be loaded from the same URL as the current file, with suffix ``.data``. You may want to put it somewhere else in some cases, e.g., if your html and js change a lot and sit on one server, while the data file is on a fast CDN somewhere else. To handle that, in your html file (or in a script tag before the one that loads the data file), change ``Module.filePackagePrefixURL`` to be the URL to the CDN. (This is a prefix, so the full filename will still be the basename with suffix ``.data``.)

@ Mapping
---------

In general the usage of ``@`` in a path (``preload-file`` or ``embed-file``) significates a mapping of a resource path (at build time) to the JS filesystem path (at run time). In the above example the path ``../../assets`` is mapped to ``/``. Other examples would be:

::

    emcc file.cpp -o file.html --embed-file ../res/gen123.png@main.png

This will make **../res/gen123.png** available as **/main.png** in Javascript.

Monitoring Read Files
---------------------

It is important to only preload the files your app actually needs, to reduce download size and improve startup speed. There is an option to log all the actually used files during runtime, which you can use to figure out which files your app actually needs. To use it, define ``logReadFiles`` on the Module object. ``Module.printErr`` will then be called on each file that is read from, so you can define that function to log to a convenient place.

You can also look at ``FS.readFiles``, which will be an object whose keys are all the files that were read from. This might be easier to use than logging. Note that you can also modify the object, even remove it entirely. This can be useful in order to see which files are read between two points in time in your app, for example.

Manually using the FS API
=========================

Check out the :ref:`Filesystem-API`.

