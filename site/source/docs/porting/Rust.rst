.. Rust:

=================
Rust + Emscripten
=================

The
`Rust programming language <https://www.rust-lang.org/>`_
has direct support for compiling to WebAssembly.
In addition to that, Rust can be used with Emscripten, which allows Rust to
benefit from Emscripten's API support. For example, using Emscripten with Rust
can allow you to port a Rust codebase using :ref:`OpenGL <OpenGL-support>` or
:ref:`Pthreads <Pthreads-support>`, as Emscripten has library support code for
those APIs which handles the large differences between them and the closest Web
APIs.

============
Installation
============

The Rust toolchain will look for ``emcc`` and the other Emscripten components
in the path. You should therefore first
:ref:`download, install, and add Emscripten to your path <sdk-download-and-install>`.

It is crucial to install the proper versions of the tools. Specifically, Rust
uses a particular LLVM version, and Emscripten must use the same one, so that
object files and so forth are all compatible.

As of June 2021, stable Rust is ``1.52.1``, which uses LLVM 12, which was
branched off of the ``llvmorg-12-init`` tag. That LLVM change rolled into
Emscripten in
`56b877afe7d1b651d6b9a2ba5d5df074876172ff <https://chromium.googlesource.com/emscripten-releases/+/56b877afe7d1b651d6b9a2ba5d5df074876172ff>`_,
which means that the compatible version of Emscripten can be installed as follows:

    .. code-block:: bash

       emsdk install 56b877afe7d1b651d6b9a2ba5d5df074876172ff
       emsdk activate 56b877afe7d1b651d6b9a2ba5d5df074876172ff

(For more on how to find the proper Emscripten hash for a particular LLVM
commit, see our
`release process docs <https://github.com/emscripten-core/emscripten/blob/main/docs/process.md#release-processes>`_
which are summarized in the
:ref:`developer's guide <developers-guide-bisecting>`.)

===========
Hello World
===========

TODO

