.. _Filesystem-Guide:

===================================
File System Overview (wiki-import)
===================================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!


Emscripten allows you to set up a virtual filesystem that points to preloaded data, as well as virtual devices that can read and write data.

There are two basic ways to use the filesystem:

-  Package some files with your build. You can just tell emcc to package a directory or a set of files, and those files will be accessible from the compiled code normally, using ``fopen`` etc.
-  Manually use the FS API from JavaScript. This lets you create, modify etc. files in a more manual manner.

For concrete examples of using the filesystem API, see the automatic tests in ``tests/runner.py``. Search for ``FS.`` for find relevant tests, for example test\_files. For specific examples of how to embed files that can be read from compiled C/C++, see for example the OpenJPEG test.

The reason for the filesystem API is that JavaScript is most often run in web browsers, which sandbox content and prevent it from accessing the local filesystem. Therefore emscripten simulates a filesystem so that C/C++ code can be written normally, as if there were direct access to files. Note: if you want to run in a shell environment using node without sandboxing, then you can let code directly access the local filesystem using NODEFS, see :ref:`Filesystem-API`.

.. todo:: HamishW - make sure we explain the implications of virtual file system, particularly with respect to when file ops can be called if we want to make sure that the virtual file system has completed loading. There is a faq on this.

Packaging Files
=========================

Check out :ref:`packaging-files`.


Manually using the FS API
=========================

Check out the :ref:`Filesystem-API`.

