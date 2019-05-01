.. _packaging-files:

===============
Packaging Files
===============

This topic shows how to package the files that will be used to populate :ref:`Emscripten's virtual file system <file-system-overview>` when the page is loaded.

There are two alternatives for how files are packaged: *preloading* and *embedding*. Embedding puts the specified files inside the generated JavaScript, while preloading packages the files separately. Embedding files is much less efficient than preloading and should only be used when packaging small numbers of small files. Preloading also enables the option to separately host the data.

*Emcc* uses the *file packager* to package the files and generate the :ref:`File System API <Filesystem-API>` calls that create and load the file system at run time. While *Emcc* is the recommended tool for packaging, there are cases where it can make sense to run the *file packager* manually.

With ``--use-preload-plugins``, files can be automatically decoded based on
their extension. See :ref:`preloading-files` for more information.

Packaging using emcc
====================

The easiest way to package files is to use :ref:`emcc <emccdoc>` at compile time. The :ref:`preload <emcc-preload-file>` and :ref:`embed <emcc-embed-file>` commands select their respective packaging methods.

The command below shows how to package files for preloading:

.. code-block:: bash

    emcc file.cpp -o file.html --preload-file asset_dir

The command generates **file.html**, **file.js** and **file.data**. The **.data** file contains all the files in **asset_dir/**, and is loaded by **file.js**.

.. note:: The :ref:`Tutorial <tutorial-files>` demonstrates preloading using the **hello_world_file.cpp** test code.


The command for embedding is shown below. In this case *emcc* generates **file.html** and **file.js** — the contents of **asset_dir/** are embedded directly into the **file.js**:

.. code-block:: bash

    emcc file.cpp -o file.html --embed-file asset_dir


By default, the files to be packaged should be nested in or below the compile-time command prompt directory. At runtime the same nested file structure is mapped to the virtual file system, with the root corresponding to the command prompt directory.

For example, consider a file structure **dir1/dir2/dir3/asset_dir/** where the project is compiled from **dir2**. When we package **asset_dir**, we specify its relative location **dir3/asset_dir/**:

.. code-block:: bash

    emcc file.cpp -o file.html --preload-file dir3/asset_dir

The folder is available at this same location **dir3/asset_dir** in the virtual file system at runtime. Similarly, if we packaged a file in *dir2*, it would be available in the root of the virtual file system at runtime.

The ``@`` symbol can be used to map packaged files from any location in the local file system to any location in the virtual file system. This is discussed below in :ref:`packaging-files-packaged-file-location`.


.. _packaging-files-file-packager:

Packaging using the file packager tool
======================================

You can also run the *file packager* manually using the instructions at the top of `file_packager.py <https://github.com/emscripten-core/emscripten/blob/master/tools/file_packager.py>`_.

The file packager generates a **.data** file and **.js** file. The **.js** file contains the code to use the data file, and must be loaded *before* loading your main compiled code.
(For instance, add ``<script>`` tags at the end of your ``--shell-file`` right before ``{{{ SCRIPT }}}```.)

.. note::

  -  Using the *file packager* allows you to run file packaging separately from compiling the code.
  -  You can load multiple datafiles by running the file packager on each and loading the **.js** outputs. See `BananaBread <https://github.com/kripken/BananaBread>`_ for an example of dynamic loading (`cube2/js/game-setup.js <https://github.com/kripken/BananaBread/blob/master/cube2/js/game-setup.js>`_).


.. _packaging-files-data-file-location:

Changing the data file location
===============================

By default, the **.data** file containing all the preloaded files is loaded from the same URL as your **.js** file. In some cases it may be useful to have the data file in a different location from the other files — for example if your **.html** and **.js** change a lot you may want to keep the data file on a fast CDN somewhere else.

This model is supported by specifying :js:attr:`Module.locateFile` function to return URL where the data file is stored. The function must be specified in a ``<script>`` element before the one that loads the data file.


.. _packaging-files-packaged-file-location:

Modifying file locations in the virtual file system
===================================================

The default approach for packaging is to directly map the nested file structure at compile time — relative to the compile-time command prompt directory — to the root of the virtual file system. The ``@`` symbol can be used in a path at build time to *explicitly* specify where the resource will be located in the virtual file system at runtime.

.. note:: The ``@`` symbol is needed because sometimes it is useful to package files that are *not* nested below the compile-time directory, and for which there is therefore no default mapping to a location in the virtual file system.

For example, we can map the preloaded folder **../../asset_dir** to the root of the virtual file system (**/**) using:

.. code-block:: bash

    emcc file.cpp -o file.html --preload-file ../../asset_dir@/

We can also map a new path and filename. For example, to make the embedded file **../res/gen123.png** available as **/main.png** we might do:

.. code-block:: bash

    emcc file.cpp -o file.html --embed-file ../res/gen123.png@main.png


.. _packaging-files-file-usage:

Valid Character Set
===================

The following characters may be used in filenames: ``A-Z``, ``a-z``, ``0-9``, the space character and any of the characters ``!#$%&'()+,-.;=@[]^_`{}~``. Additionally, the following characters may be used if your host filesystem supports them: ``"*<>?|`` (Windows does not allow using these in filenames). When specifying the character ``@`` on the command line, it must be escaped to the form ``@@`` to avoid triggering the ``src@dst`` mapping notation (see above). The characters ``/``, ``\`` and ``:`` cannot be used.

Monitoring file usage
=====================

.. important:: Only package the files your app actually needs, in order to reduce download size and improve startup speed.

There is an option to log which files are actually used at runtime. To use it, define the :js:attr:`Module.logReadFiles` object. Each file that is read will be logged to stderr.

An alternative approach is to look at :js:func:`FS.readFiles` in your compiled JavaScript. This is an object with keys for all the files that were read from. You may find it easier to use than logging as it records files rather than potentially multiple file accesses.

.. note:: You can also modify the :js:func:`FS.readFiles` object or remove it entirely. This can be useful, say, in order to see which files are read between two points in time in your app.

.. _preloading-files:

Preloading files
================

With ``--use-preload-plugins``, files can be automatically decoded based on
their extension. This can also be done manually by calling
:c:func:`emscripten_run_preload_plugins` on each file. The files remain stored
in their original form in the file system, but their decoded form can be used
directly.

The following formats are supported:

- **Images** (``.jpg``, ``.jpeg``, ``.png``, ``.bmp``): The files are decoded
  using the browser's image decoder, and can then be used by ``IMG_Load`` (SDL1
  and SDL2 port, which rely on :c:func:`emscripten_get_preloaded_image_data`).
  (Set ``Module.noImageDecoding`` to ``true`` to disable).

- **Audio** (``.ogg``, ``.wav``, ``.mp3``): The files are decoded using the
  browser's audio decoder, and can then by used with ``Mix_LoadWAV`` (SDL1
  only).  (Set ``Module.noAudioDecoding`` to ``true`` to disable).

- **Dynamic libraries** (``.so``): The files are precompiled and instantiated
  using ``WebAssembly.instantiate``. This is useful for browsers, such as
  Chrome, that require compiling large WebAssembly modules asynchronously, if
  you then want to load the module synchronously using ``dlopen`` later. (Set
  ``Module.noWasmDecoding`` to ``true`` to disable).

Test code
=========

The `test suite <https://github.com/emscripten-core/emscripten/blob/master/tests/>`_ contains many file packaging examples, and is a good place to search for working code.
