.. _Developer's-Guide:

=================
Developer's Guide
=================

This article provides information that is relevant to people who want to
contribute to Emscripten. We welcome contributions from anyone that is
interested in helping out!

.. tip:: The information will be less relevant if you're just using Emscripten, but may still be of interest.

.. _developers-guide-setting-up:

Setting up
==========

For contributing to core Emscripten code, such as ``emcc.py``, you don't need to
build any binaries as ``emcc.py`` is in Python, and the core JS generation is
in JavaScript. You do still need binaries for LLVM and Binaryen, which you can
get using the emsdk:

::

    emsdk install tot-upstream
    emsdk activate tot-upstream

That gets a "tip-of-tree" build of the very latest binaries. You can use those
binaries with a checkout of the core Emscripten repository, simply by calling
``emcc.py`` from that checkout, and it will use the binaries from the emsdk.

If you do want to contribute to LLVM or Binaryen, or to test modifications
to them, you can 
:ref:`build them from source <installing-from-source>`.

Repositories and branches of interest
=====================================

The Emscripten main repository is https://github.com/emscripten-core/emscripten.
The main branch is **incoming**, which exists for historic reasons as we did
not always have good pre-commit CI, and so we landed code on incoming first,
tested, then merged to master. Today we have extensive pre-commit CI on GitHub,
so the incoming branch functions like the master branch in most projects.

Aside from the Emscripten repo, the other codebases of interest are LLVM
and Binaryen, which Emscripten invokes, and
:ref:`have their own repos <installing-from-source>`.

.. _developers-guide-submitting-patches:

Submitting patches
==================

Patches should be submitted as *pull requests* to the **incoming** branch.

.. note:: Before submitting your first patch, add yourself to the `AUTHORS <https://github.com/emscripten-core/emscripten/blob/incoming/AUTHORS>`_ file. By doing so, you agree to license your code under the project's :ref:`open source licenses (MIT/LLVM) <emscripten-license>`.

When submitting patches, please:

- Make pull requests to **incoming**, not master.
- Add an automatic test if you add any new functionality or fix a bug. Search
  in ``tests/*.py`` for related tests, as often the simplest thing is to add to
  an existing one. If you're not sure how to test your code, feel free to ask
  for help.
- We normally squash and merge PRs, which means the PR turns into a single
  commit on the target branch. Because of that, it's ok to have merge commits
  in the PR itself, as they get removed. Please put a good description for
  the final commit in the PR description, and we'll use it when squashing.

Code reviews
============

One of the core developers will review a pull request before merging it. If
several days pass without any comments on your PR, please comment in the PR
which will ping them. (If that happens, sorry! Sometimes things get missed.)

Compiler overview
=================

The :ref:`Emscripten Compiler Frontend (emcc) <emccdoc>` is a python script that manages the entire compilation process:

- **emcc** calls :term:`Clang` to compile C++ and ``wasm-ld`` to link it. It
  builds and integrates with the Emscripten system libraries, both the
  compiled ones and the ones implemented in JS.
- **emcc** then calls `emscripten.py <https://github.com/emscripten-core/emscripten/blob/master/emscripten.py>`_
  which performs the final transformation to wasm (including invoking
  **wasm-emscripten-finalize** from Binaryen) and calls the JS compiler
  (see ``src/compiler.js`` and related files) which emits the JS.
- If optimizing wasm, **emcc** will then call **wasm-opt**, run meta-dce, and
  other useful things. It will also run JS optimizations on the JS that is
  emitted alongside the wasm.

Emscripten Test Suite
=====================

Emscripten has a :ref:`comprehensive test suite <emscripten-test-suite>`, which
covers virtually all Emscripten functionality. These tests are run on CI
automatically when you create a pull request, and they should all pass. If you
run into trouble with a test failure you can't fix, please let the developers
know.

See also
========

- :ref:`Debugging`
- :ref:`Building-Projects`


