.. _packaging-files:

==================================
Packaging Files (ready-for-review)
==================================

This topic shows how to package the files that will be used to populate :ref:`Emscripten's virtual file system <file-system-overview>` when the page is loaded. 

There are two alternatives for how files are packaged: *preloading* and *embedding*. Embedding puts the specified files inside the generated JavaScript, while preloading packages the files separately. Embedding files is much less efficient than preloading and should only be used when packaging small numbers of small files. Preloading also enables the option to separately host the data.
	
*Emcc* uses the *file packager* to package the files, and to generate the :ref:`File System API <Filesystem-API>` calls that create and load the file system at run time. While *Emcc* is the recommended tool for packaging, there are cases where it can make sense to run the *file packager* manually.


Packaging using emcc
====================

The easiest way to package files is to use :ref:`emcc <emccdoc>` at compile time. The :ref:`preload <emcc-preload-file>` and :ref:`embed <emcc-embed-file>` commands select their respective packaging methods.

The command below shows how to package files for preloading:

.. code-block:: bash

    ./emcc file.cpp -o file.html --preload-file asset_dir
	
The command generates **file.html**, **file.js** and **file.data**. The **.data** file contains all the files in **asset_dir/**, and is loaded by **file.js**.
	
.. note:: The :ref:`Tutorial <tutorial-files>` demonstrates preloading using the **hello_world_file.cpp** test code.


The command for embedding is shown below. In this case *emcc* generates **file.html** and **file.js** — the contents of **asset_dir/** are embedded directly into the **file.js**:

.. code-block:: bash

    ./emcc file.cpp -o file.html --embed-file asset_dir


By default, the run time path to files will be the same as you specify at compile time during packaging. For example, the files in **asset_dir** below will be available at runtime from **../../asset_dir**:

.. code-block:: bash

    ./emcc file.cpp -o file.html --preload-file ../../asset_dir

The ``@`` symbol can be used to instead map packaged files elsewhere in the virtual file system. This is discussed below in :ref:`packaging-files-packaged-file-location`.


.. _packaging-files-file-packager:

Packaging using the file packager tool 
======================================

You can also run the *file packager* manually using the instructions at the top of `file_packager.py <https://github.com/kripken/emscripten/blob/master/tools/file_packager.py>`_. 

The file packager generates a **.data** file and **.js** file. The **.js** file contains the code to use the data file, and must be loaded *before* loading your main compiled code.

.. note::

	-  Using the *file packager* allows you to run file packaging separately from compiling the code. 
	-  You can load multiple datafiles by running the file packager on each and loading the **.js** outputs. See `BananaBread <https://github.com/kripken/BananaBread>`_ for an example of this (`cube2/js/game-setup.js <https://github.com/kripken/BananaBread/blob/master/cube2/js/game-setup.js>`_).

	
.. _packaging-files-data-file-location:

Changing the data file location
===============================

By default, the **.data** file containing all the preloaded files is loaded from the same URL as your **.js** file. In some cases it may be useful to have the data file in a different location from the other files — for example if your **.html** and **.js** change a lot you may want to keep the data file is on a fast CDN somewhere else. 

This model is supported by changing the :js:attr:`Module.filePackagePrefixURL` to be the URL where the data file is stored (this is a prefix, so should include the full path before the data's file name). The attribute must be specified in a script tag before the one that loads the data file.


.. _packaging-files-packaged-file-location:

Modifying file locations in the virtual file system
===================================================

Packaged files are mapped to the virtual file system, by default, at the same relative path as was specified when they were added. The ``@`` symbol can be used in a path at build time to change the location of a resource in virtual file system at runtime. 

For example, we can map the preloaded folder **../../asset_dir** to the root of the virtual file system (**/**) using:

.. code-block:: bash

    ./emcc file.cpp -o file.html --preload-file ../../asset_dir@/

We can also map a new path and filename. For example, to make embedded file **../res/gen123.png** available as **/main.png** we might do:

.. code-block:: bash

    ./emcc file.cpp -o file.html --embed-file ../res/gen123.png@main.png


.. _packaging-files-file-usage:
	
Monitoring file usage
=====================

.. important:: Only package the files your app actually needs, in order to reduce download size and improve startup speed. 

There is an option to log which files are actually used at runtime. To use it, define the :js:attr:`Module.logReadFiles` object. The :js:attr:`Module.printErr` function will be called on each file that is read (this function must also be defined, and should log to a convenient place).

An alternative approach is to look at :js:func:`FS.readFiles` in your compiled JavaScript. This is an object with keys for all the files that were read from. It might be easier to use than logging as it records files rather than potentially multiple file accesses. 

.. note:: You can also modify the :js:func:`FS.readFiles` object or remove it entirely. This can be useful, say, in order to see which files are read between two points in time in your app.

Test code
=========

The `test suite <https://github.com/kripken/emscripten/blob/master/tests/>`_ contains many file packaging examples, and is a good place to search for working code. 

