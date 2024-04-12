.. _configuring-emscripten-settings:

==================================================================
Configuring Emscripten Settings when Manually Building from Source
==================================================================

Emscripten can be configured via a :ref:`compiler
configuration file (.emscripten) <compiler-configuration-file>`. These settings
include paths to the tools (LLVM, Clang, Binaryen, etc.) and the compiler's
temporary directory for intermediate build files.

This configuration file is optional.  By default, emscripten will search
for the tools it needs in the ``PATH``.

This article explains how to create and update the file when you are building
Emscripten :ref:`manually <installing-from-source>` from source.


Creating the compiler configuration file
========================================

A settings file may be used when running :ref:`emcc <emccdoc>` (or any of the
other Emscripten tools).  You can run ``emcc`` with ``--generate-config``
in order to generate one in the default location.

1. Navigate to the directory where you cloned the Emscripten repository.
2. Enter the command:

  ::

    ./emcc --generate-config

  You should get a ``An Emscripten settings file has been generated at:``
  message, along with the contents of the config file.

When generating this file Emscripten will make its "best guess" at the correct
locations for tools based on the current ``PATH``.

In most cases it will be necessary to edit the generated file and modify
least the ``LLVM_ROOT`` and ``BINARYEN_ROOT`` settings to point to the correct
locations for your local LLVM and Binaryen installations.


Locating the compiler configuration file (.emscripten)
======================================================

The settings file (``.emscripten``) is created by default within the emscripten
directory (alongsize ``emcc`` itself).  In cases where the emscripten directory
is read-only the user's home directory will be used:

  - On Linux and macOS this file is named **~/.emscripten**, where ~ is the
    user's home directory.

    .. note:: Files with the "." prefix are hidden by default. You may need to change your view settings to find the file.

  - On Windows the file can be found at a path like: **C:/Users/yourusername_000/.emscripten**.


Compiler configuration file-format
==================================

.. note:: While the syntax is identical, the appearance of the default **.emscripten** file created by *emcc* is quite different than that created by :ref:`emsdk <compiler-configuration-file>`. This is because *emsdk* manages multiple target environments, and where possible hard codes the locations of those tools when a new environment is activated. The default file, by contrast, is managed by the user — and is designed to make that task as easy as possible.

The file simply assigns values to a number of *variables* representing the main
tools used by Emscripten. For example, if your binaryen installation is in
**C:\\tools\\binaryen\\**, then the file might contain the line: ::

  BINARYEN_ROOT = 'C:\\tools\\binaryen\\'

You can find out the other variable names from the default *.emscripten* file or
the :ref:`example here <compiler-configuration-file>`.

Editing the compiler configuration file
=======================================

The compiler configuration file can be edited with the text editor of your
choice. If you're building manually from source, you are most likely to have to
update the variable ``LLVM_ROOT``

#. Edit the variable ``LLVM_ROOT`` to point to the directory where you built the
   LLVM binaries, such as:

    ::

      LLVM_ROOT = '/home/ubuntu/a-path/llvm/build/bin'

    .. note:: Use forward slashes!

After setting those paths, run ``emcc`` again. It should again perform the sanity checks to test the specified paths. There are further validation tests available at :ref:`verifying-the-emscripten-environment`.
